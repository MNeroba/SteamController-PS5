#!/usr/bin/env python3
"""Portable asset/embedding contracts; browser interactions are tested separately."""
import base64
import hashlib
import json
import re
import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
WEB = ROOT / 'payload/web'
HTML = (WEB / 'index.html').read_text()
LAYOUTS = json.loads((WEB / 'controllers/layouts.json').read_text())
HASHES = {
    'steam-front.webp': '5b7354a941223f0b5293eeaa012c55897e9aebb7941cd6aa75eb31650dec8376',
    'steam-back.webp': '909e7e5c52916da5d60ec72cf9d3280e85824d08aecbc32b531165780bafb4a6',
    'dual-front.webp': '888a3b62a3c093f2ffa7b203986fe6b53a427628624942d782922ec010d0151a',
    'dual-back.webp': 'bc7af34550853da4f303f7143b2d0beebbd12679bf7258dfe853cc2a49d74e5c',
}


def mapping(name):
    raw = re.search(r'const ' + name + r'=(\{.*?\});', HTML).group(1)
    return json.loads(re.sub(r'(\w+):', r'"\1":', raw))


class Controllers(unittest.TestCase):
    def test_exact_four_webp_assets(self):
        for name, digest in HASHES.items():
            blob = (WEB / 'controllers' / name).read_bytes()
            self.assertEqual(blob[:4], b'RIFF')
            self.assertEqual(blob[8:12], b'WEBP')
            self.assertEqual(hashlib.sha256(blob).hexdigest(), digest)

    def test_source_coverage_and_shared_keys(self):
        keys = mapping('SOURCE_KEYS')
        bits = json.loads(re.search(r'const BITS=(\[.*?\]);', HTML).group(1))
        ids = {s['id'] for v in ('front', 'back') for s in LAYOUTS['steam'][v]['shapes']}
        self.assertEqual(set(keys.values()), set(bits))
        self.assertEqual(len(keys), 24)
        self.assertTrue(set(keys).issubset(ids))
        self.assertEqual(keys['l1'], 'LB')
        self.assertEqual(keys['l2'], 'LT')
        self.assertEqual(keys['tpl'], 'LPAD_CLICK')

    def test_output_ids_preserved(self):
        keys = mapping('OUTPUT_KEYS')
        outs = json.loads(re.search(r'const OUTS=(\[.*?\]);', HTML).group(1))
        ids = {s['id'] for v in ('front', 'back') for s in LAYOUTS['dual'][v]['shapes']}
        self.assertEqual(len(keys), 18)
        self.assertEqual(set(keys.values()), {o[0] for o in outs if not o[0].startswith('PB_')})
        self.assertTrue(set(keys).issubset(ids))
        self.assertNotIn('mute', keys)
        self.assertEqual(keys['tp'], 'TOUCHPAD')

    def test_geometry_and_back_orientation(self):
        self.assertEqual(LAYOUTS['steam']['front']['iw'], 2566)
        self.assertEqual(LAYOUTS['steam']['front']['vb'], '170 120 2230 1640')
        for device in ('steam', 'dual'):
            shapes = {s['id']: s for s in LAYOUTS[device]['back']['shapes']}
            for left, right in [('l1', 'r1'), ('l2', 'r2')]:
                self.assertGreater(shapes[left]['pts'][0][0], shapes[right]['pts'][0][0])
        shapes = {s['id']: s for s in LAYOUTS['steam']['back']['shapes']}
        self.assertGreater(shapes['l4']['cx'], shapes['r4']['cx'])

    def test_informational_controls_not_mappings(self):
        for key in ('grl', 'grr', 'usbc', 'puck'):
            self.assertNotIn(key, mapping('SOURCE_KEYS'))
        self.assertIn('const RESERVED="QAM"', HTML)
        self.assertIn('if(k===RESERVED)', HTML)

    def test_self_contained_generator_roundtrip(self):
        with tempfile.TemporaryDirectory() as tmp:
            target = Path(tmp)
            shutil.copytree(WEB, target / 'web')
            shutil.copy(ROOT / 'payload/gen_html.py', target)
            subprocess.run(['python3', str(target / 'gen_html.py')], cwd=ROOT, check=True)
            header = (target / 'webui_html.h').read_text()
            page = bytes(int(v, 16) for v in re.findall(r'0x([0-9a-f]{2})', header))
            declared = int(re.search(r'webui_html_len = (\d+)', header).group(1))
            self.assertEqual(len(page), declared)
            text = page.decode('utf-8')
            self.assertNotIn('/*__CONTROLLERS__*/', text)
            self.assertNotIn('/*__ICONS__*/', text)
            data = json.loads(re.search(r'const SHAPES=(\{.*?\});', text).group(1))
            for device in ('steam', 'dual'):
                for view in ('front', 'back'):
                    v = data[device][view]
                    self.assertEqual(base64.b64decode(v['img'].split(',')[1]),
                                     (WEB / 'controllers' / v['asset']).read_bytes())
                    self.assertEqual(v['vb'], LAYOUTS[device][view]['vb'])
            self.assertNotRegex(text, r'<(?:script|link)[^>]+(?:src|href)="https?://')

    def test_makefile_tracks_asset_changes(self):
        makefile = (ROOT / 'payload/Makefile').read_text()
        self.assertIn('web/icons/*.png web/controllers/*.webp', makefile)
        self.assertIn('web/controllers/layouts.json', makefile)
        self.assertIn('webui_html.h: web/index.html gen_html.py $(WEB_ASSETS)', makefile)


if __name__ == '__main__':
    unittest.main()
