#!/usr/bin/env python3
"""Embed the portal, button icons and controller artwork in one offline page."""
import base64, glob, json, os, re
from pathlib import Path

os.chdir(Path(__file__).resolve().parent)

html = open('web/index.html', encoding='utf-8').read()
version = re.search(r'#define STEAMCONTROLLER_VERSION "([^"]+)"', Path('version.h').read_text())[1]
if html.count('/*__VERSION__*/""') != 1:
    raise ValueError('Expected one application version placeholder')
html = html.replace('/*__VERSION__*/""', json.dumps(version))

# Inline web/icons/*.png as data URIs at the /*__ICONS__*/{} placeholder.
icons = {}
for p in sorted(glob.glob('web/icons/*.png')):
    key = os.path.splitext(os.path.basename(p))[0]
    icons[key] = 'data:image/png;base64,' + base64.b64encode(open(p, 'rb').read()).decode()
html = html.replace('/*__ICONS__*/{}', json.dumps(icons, separators=(',', ':')))

# Keep the reference's logical image dimensions and viewBox, not decoded pixels.
controllers = json.loads(Path('web/controllers/layouts.json').read_text(encoding='utf-8'))
for device in controllers.values():
    for view in ('front', 'back'):
        layout = device[view]
        asset = layout['asset']
        if Path(asset).name != asset or not asset.endswith('.webp'):
            raise ValueError('Invalid controller asset name')
        layout['img'] = 'data:image/webp;base64,' + base64.b64encode(
            Path('web/controllers', asset).read_bytes()).decode('ascii')
marker = '/*__CONTROLLERS__*/{}'
if html.count(marker) != 1:
    raise ValueError('Expected exactly one controller layout placeholder')
html = html.replace(marker, json.dumps(controllers, separators=(',', ':'), ensure_ascii=True))

d = html.encode('utf-8')
out = ['/* generated from web/index.html by gen_html.py — do not edit */', 'static const unsigned char webui_html[] = {']
for i in range(0, len(d), 16):
    out.append('  ' + ','.join('0x%02x' % b for b in d[i:i+16]) + ',')
out += ['};', 'static const unsigned int webui_html_len = %d;' % len(d)]
open('webui_html.h', 'w').write('\n'.join(out) + '\n')
print('embedded %d bytes, %d icons, 4 controller images' % (len(d), len(icons)))
