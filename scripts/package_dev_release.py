#!/usr/bin/env python3
"""Validate experimental policy and package an actual SDK ELF; no hardware claims."""
import argparse
import hashlib
import json
import re
import shutil
import struct
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
WARNING = ('Experimental development build. Not verified on physical PS5 hardware. '
           'PS5 firmware 12.00 is the intended test environment, not a confirmed '
           'compatible firmware. Use at your own risk.')
SDK_SHA = 'a9cc9929f21b2b2c5d5b309f3bab4997067c45281c0622cf4838b1aecba66fcb'


def validate_policy(policy):
    if (policy.get('schema') != 1 or policy.get('channel') != 'experimental-dev' or
            policy.get('target_firmware') != '12.00' or
            policy.get('firmware_verified') is not False or policy.get('sdk') != 'v0.43' or
            policy.get('critical_code_blockers') != [] or not policy.get('review')):
        raise ValueError('DEV policy missing, altered, or has critical code blockers')


def validate_identifiers(source_sha, version):
    if not re.fullmatch(r'[0-9a-f]{40}', source_sha):
        raise ValueError('Select a full lowercase source commit SHA')
    if not re.fullmatch(r'v\d+\.\d+\.\d+-dev\.\d+', version):
        raise ValueError('Version must have the form v0.1.0-dev.1')


def validate_elf(data):
    # ELF64 little-endian x86-64 ET_DYN/ET_EXEC; SDK output is ET_DYN.
    if (len(data) < 64 or data[:7] != b'\x7fELF\x02\x01\x01' or
            struct.unpack_from('<H', data, 16)[0] not in (2, 3) or
            struct.unpack_from('<H', data, 18)[0] != 62):
        raise ValueError('Missing or invalid ELF64 x86-64 payload')


def package(source_sha, version, elf, output):
    validate_identifiers(source_sha, version)
    validate_policy(json.loads((ROOT / 'docs/development/dev-release.json').read_text()))
    data = elf.read_bytes()
    validate_elf(data)
    output.mkdir(parents=True, exist_ok=True)
    asset = f'SteamController-PS5-{version}.elf'
    (output / asset).write_bytes(data)
    digest = hashlib.sha256(data).hexdigest()
    (output / f'{asset}.sha256').write_text(f'{digest}  {asset}\n')
    shutil.copyfile(ROOT / 'LICENSE', output / 'LICENSE')
    shutil.copyfile(ROOT / 'docs/development/LOADING.md', output / 'LOADING.md')
    source = f'https://github.com/MNeroba/SteamController-PS5/tree/{source_sha}'
    notices = ('# Notices and corresponding source\n\n'
               f'Payload source: [{source_sha}]({source}). The release tag points to this exact revision. '
               'GitHub provides the matching source archive alongside these assets.\n\n'
               'Code: GPL-3.0-or-later; full text in LICENSE. Preserve inherited notices. '
               'PuckbridgePS5 / Ghostcontrol / PoorDS4 provenance and pins: docs/PROVENANCE.md in that source.\n\n'
               'Community SDK v0.43 corresponding source: '
               'https://github.com/ps5-payload-dev/sdk/tree/d9c9519116944a7f1c22d262012c53b80b3520b7. '
               'SDK headers retain their individual GPL/BSD exceptions. SDK archive checksum: '
               f'`{SDK_SHA}`. No proprietary Sony SDK is included.\n\n')
    for file in ('payload/bridge/NOTICE.md', 'payload/web/icons/LICENSE.txt',
                 'payload/web/controllers/NOTICE.md'):
        notices += f'## {file}\n\n' + (ROOT / file).read_text() + '\n'
    (output / 'NOTICES.md').write_text(notices)
    notes = (f'# SteamController-PS5 {version} — experimental DEV\n\n**{WARNING}**\n\n'
             f'- Source: [`{source_sha}`]({source})\n'
             '- SDK: PS5 Payload SDK v0.43 / LLVM 18\n'
             f'- SDK archive SHA-256: `{SDK_SHA}`\n'
             f'- Payload: `{asset}`\n- ELF SHA-256: `{digest}`\n\n'
             'Required workflow gates: normal host, ASan/UBSan, 181+ Chromium assertions, '
             'nine image/embedding contracts, deterministic asset generation, clean PS5 cross-build '
             'and checksum verification. These establish software/build evidence only.\n\n'
             'USB-C/Puck, virtual DualSense, motion/touch/haptics and shutdown remain physically unverified. '
             'One active Steam Controller 2026 session. Native BLE and Edge identity are research only. '
             'Game hooks, bridge probing, DualSense handoff and WebUI are disabled by default.\n\n'
             'Read LOADING.md and NOTICES.md before testing. Verify the checksum before sending '
             'the ELF through an independently confirmed compatible development loader. '
             'There is no PKG/FPKG or persistent installer. The stable hardware release gate remains blocked.\n')
    (output / 'RELEASE_NOTES.md').write_text(notes)
    print(f'{asset}: SHA-256 {digest}')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source-sha', required=True)
    parser.add_argument('--version', required=True)
    parser.add_argument('--elf', type=Path, default=ROOT / 'payload/ghost-control-ps5.elf')
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    package(args.source_sha, args.version, args.elf, args.output)
