#!/usr/bin/env python3
"""Version/changelog/metadata/source identity fail-closed regressions."""
import importlib.util
import json
import re
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'scripts'))
from check_changelog import entry,validate
from package_dev_release import validate_metadata
VERSION = re.search(r'STEAMCONTROLLER_VERSION "([^"]+)"',(ROOT/'payload/version.h').read_text()).group(1)
class Changelog(unittest.TestCase):
    def test_current_and_history(self):
        text=(ROOT/'CHANGELOG.md').read_text()
        self.assertIn('### Bug Fixes',validate(VERSION))
        for v in ['v0.1.0-dev.1','v0.1.0-dev.2','v0.1.0-dev.3','v0.1.0-dev.4']:
            self.assertEqual(entry(text,v).count('### '),2)
    def test_bad_format(self):
        good='## v0.1.0-dev.3\n\n### Bug Fixes\n\nNone.\n\n### New Features / Improvements\n\n- Reports.\n'
        for bad in [good.replace('Bug Fixes','Fixes'),good+good,good.replace('None.','- Reports.'),good.replace('None.','Maybe.'),good+'### Other\n- Other.\n']:
            with self.subTest(bad=bad),self.assertRaises(ValueError):entry(bad,'v0.1.0-dev.3')
        with self.assertRaises(ValueError):entry(good,'v0.1.0-dev.4')
        with self.assertRaises(ValueError):validate('v0.1.0-dev.2')
    def test_metadata(self):
        m=json.loads((ROOT/'config/payload/SteamController-PS5.elf.json').read_text())
        v=VERSION;asset='SteamController-PS5-'+v+'.elf';validate_metadata(m,v,asset)
        for key,value in [('version','v0.0.0-dev.0'),('filename','other.elf'),('url','https://example.com/payload.elf'),('category','Game'),('description','Works on all consoles'),('checksum','x')]:
            with self.subTest(key=key),self.assertRaises(ValueError):validate_metadata(dict(m,**{key:value}),v,asset)
    def test_default_build_target(self):
        self.assertIn(".DEFAULT_GOAL := all",(ROOT/"payload/Makefile").read_text())
    def test_workflow_changelog_gates(self):
        for name in ['dev-release.yml','release.yml']:
            s=(ROOT/'.github/workflows'/name).read_text()
            self.assertIn('scripts/check_changelog.py',s)
            self.assertIn('body_path:',s)
        self.assertIn('*.elf.json',(ROOT/'.github/workflows/dev-release.yml').read_text())
if __name__=='__main__':unittest.main()
