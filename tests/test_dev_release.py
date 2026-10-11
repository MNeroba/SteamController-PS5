#!/usr/bin/env python3
"""Host-only release policy/package regressions; synthetic ELF stays in a tempdir."""
import hashlib
import contextlib
import io
from unittest.mock import patch
import json
import struct
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'scripts'))
from package_dev_release import package, validate_policy, validate_elf, validate_identifiers, WARNING
from check_browser_result import validate as validate_browser


class DevRelease(unittest.TestCase):
    def setUp(self):
        self.policy = json.loads((ROOT / 'docs/development/dev-release.json').read_text())

    def test_policy(self):
        if self.policy['critical_code_blockers']:
            with self.assertRaises(ValueError):
                validate_policy(self.policy)
        self.policy = dict(self.policy, critical_code_blockers=[])
        validate_policy(self.policy)
        for key, invalid in [('schema', None), ('channel', 'stable'), ('target_firmware', '12.00'),
                             ('firmware_verified', True), ('sdk', 'v0.44'),
                             ('critical_code_blockers', ['unfixed corruption']), ('review', '')]:
            with self.subTest(key=key), self.assertRaises(ValueError):
                validate_policy(dict(self.policy, **{key: invalid}))

    def test_identifiers(self):
        validate_identifiers('a' * 40, 'v0.1.0-dev.1')
        for sha, version in [('main', 'v0.1.0-dev.1'), ('A' * 40, 'v0.1.0-dev.1'),
                             ('a' * 40, 'v1.0.0'), ('a' * 40, 'dev/v0.1.0-dev.1'),
                             ('a' * 40, 'v0.1.0-dev.1\n'), ('$(id)', 'v0.1.0-dev.1')]:
            with self.subTest(sha=sha, version=version), self.assertRaises(ValueError):
                validate_identifiers(sha, version)

    def test_bad_elf(self):
        for data in [b'', b'PKG', b'\x7fELF\x02\x01\x01' + b'\0' * 57]:
            with self.assertRaises(ValueError):
                validate_elf(data)

    def test_packaged_checksum_warning_and_source(self):
        # Tests packaging only; this fixture must never be called a real PS5 build.
        data = bytearray(128)
        data[:7] = b'\x7fELF\x02\x01\x01'
        struct.pack_into('<HH', data, 16, 3, 62)
        data += (ROOT / 'payload/settings-tile/icon0.png').read_bytes()
        data += (ROOT / 'payload/settings-tile/param.json').read_bytes()
        with tempfile.TemporaryDirectory() as temp:
            elf = Path(temp) / 'test-fixture.elf'
            elf.write_bytes(data)
            out = Path(temp) / 'assets'
            # Test the packaging function using a synthetic policy in an
            # isolated root. Never clear the real project's blocker manifest.
            fixture_root = Path(temp) / 'source'; fixture_root.mkdir()
            for name in ['LICENSE', 'docs/development/LOADING.md', 'payload/bridge/NOTICE.md',
                         'payload/web/icons/LICENSE.txt', 'payload/web/controllers/NOTICE.md', 'payload/settings-tile/NOTICE.md', 'payload/settings-tile/icon0.png', 'payload/settings-tile/param.json']:
                target = fixture_root / name; target.parent.mkdir(parents=True, exist_ok=True)
                target.write_bytes((ROOT / name).read_bytes())
            (fixture_root / 'docs/development/dev-release.json').write_text(json.dumps(dict(self.policy, critical_code_blockers=[])))
            with patch('package_dev_release.ROOT', fixture_root), contextlib.redirect_stdout(io.StringIO()):
                incomplete = Path(temp) / 'missing-icon.elf'; incomplete.write_bytes(data[:128])
                with self.assertRaisesRegex(ValueError, 'Missing embedded'):
                    package('a' * 40, 'v0.1.0-dev.1', incomplete, out)
                package('a' * 40, 'v0.1.0-dev.1', elf, out)
            notes = (out / 'RELEASE_NOTES.md').read_text()
            self.assertIn(WARNING, notes)
            self.assertIn('a' * 40, notes)
            self.assertIn(hashlib.sha256(data).hexdigest(), notes)
            asset = out / 'SteamController-PS5-v0.1.0-dev.1.elf'
            self.assertEqual(asset.read_bytes(), data)
            self.assertEqual((out / (asset.name + '.sha256')).read_text(),
                             f'{hashlib.sha256(data).hexdigest()}  {asset.name}\n')
            self.assertEqual(len(list(out.glob('*.elf'))), 1)
            self.assertIn('One ELF', notes)
            for name in ['LOADING.md', 'NOTICES.md', 'LICENSE']:
                self.assertTrue((out / name).stat().st_size)

    def test_browser_gate(self):
        config = json.loads((ROOT / 'tests/playwright-cli.json').read_text())
        self.assertEqual(config['browser']['browserName'], 'chromium')
        self.assertEqual(config['browser']['launchOptions']['channel'], 'chromium')
        self.assertIn('--config tests/playwright-cli.json', (ROOT / 'scripts/run_webui_browser.sh').read_text())
        self.assertIn('cat "$log_dir/browser.json"', (ROOT / 'scripts/run_webui_browser.sh').read_text())
        validate_browser({'result': 'PASS', 'checks': 202})
        validate_browser({'output': json.dumps({'result': 'PASS', 'checks': 202})})
        for value in [{}, {'result': 'FAIL', 'checks': 202},
                      {'result': 'PASS', 'checks': 201}, {'result': 'PASS', 'checks': True}]:
            with self.assertRaises(ValueError):
                validate_browser(value)

    def test_stable_gate_separate(self):
        workflow = (ROOT / '.github/workflows/dev-release.yml').read_text()
        self.assertIn('workflow_dispatch:', workflow)
        self.assertNotIn('\n  push:', workflow)
        for requirement in ['prerelease: true', 'make_latest: false', 'dev/${{',
                            'needs: [select-source, validate-build]', 'fail_on_unmatched_files: true',
                            'if-no-files-found: error', 'head_sha: sha', 'make -C tests sanitize',
                            'bash scripts/run_webui_browser.sh', 'sha256sum --check']:
            self.assertIn(requirement, workflow)
        stable = (ROOT / '.github/workflows/release.yml').read_text()
        self.assertIn('needs: [host-tests, cross-build, hardware-gate]', stable)
        self.assertIn('python3 scripts/check_release_gate.py', stable)
        for requirement in ['make -C tests clean test','make -C tests sanitize',
                            'bash scripts/run_webui_browser.sh','python3 tests/test_webui_controllers.py',
                            'fetch-depth: 0','sha256sum --check ghost-control-ps5.elf.sha256']:
            self.assertIn(requirement,stable)


if __name__ == '__main__':
    unittest.main()
