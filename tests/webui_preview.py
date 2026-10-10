#!/usr/bin/env python3
"""Loopback-only browser fixture. Exercises generated UI, never a PS5 backend.

Run `python3 payload/gen_html.py` first, then this script. Browser tests enroll
the public fixture token `preview-only` via sessionStorage. State is RAM-only.
"""
import json
import re
import subprocess
import argparse
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import parse_qs, urlsplit

ROOT = Path(__file__).resolve().parents[1]
def embedded_page():
    header = (ROOT / 'payload/webui_html.h').read_text()
    return bytes(int(v, 16) for v in re.findall(r'0x([0-9a-f]{2})', header))


PAGE = embedded_page()
defaults = re.search(r'const DEFAULTS=(\{.*?\});', PAGE.decode()).group(1)
defaults = json.loads(re.sub(r'(\w+):', r'"\1":', defaults))
BASE = ''.join(f'{k}={v}\n' for k, v in defaults.items()) + 'SHIFT=L4\n'
PROFILES = {'default': 'name=Default\n' + BASE,
            'PPSA00001': 'name=Browser fixture\n' + BASE + 'A=SQUARE\n'}
NORMALIZER = ROOT / 'tests/build/webui_profile_fixture'


class Preview(BaseHTTPRequestHandler):
    def log_message(self, *_):
        pass

    def respond(self, body, kind='application/json', code=200):
        if not isinstance(body, bytes):
            body = body.encode()
        self.send_response(code)
        self.send_header('Content-Type', kind)
        self.send_header('Content-Length', str(len(body)))
        self.send_header('Cache-Control', 'no-store')
        self.end_headers()
        self.wfile.write(body)

    def do_GET(self):
        url = urlsplit(self.path)
        if url.path == '/':
            return self.respond(embedded_page(), 'text/html; charset=utf-8')
        if url.path == '/favicon.ico':
            return self.respond(b'', 'image/x-icon', 204)
        if not self.authorized():
            return
        if url.path == '/api/status':
            value = {'connected': False, 'active': 'default', 'inputs': 0}
        elif url.path == '/api/profiles':
            value = [{'id': k, 'name': v.splitlines()[0][5:]}
                     for k, v in PROFILES.items()]
        elif url.path == '/api/profile':
            key = parse_qs(url.query).get('id', ['default'])[0]
            return self.respond(PROFILES.get(key, ''), 'text/plain')
        elif url.path == '/api/games':
            value = [{'id': 'PPSA00001', 'name': 'Browser fixture', 'icon': False}]
        elif url.path in ('/api/gamehooks', '/api/dualsense', '/api/bridge'):
            value = {}  # No hardware services are simulated.
        else:
            return self.respond('{}', code=404)
        self.respond(json.dumps(value))

    def authorized(self):
        if self.headers.get('Authorization') == 'Bearer preview-only':
            return True
        self.respond('Fixture token required', 'text/plain', 401)
        return False

    def do_POST(self):
        if not self.authorized():
            return
        url = urlsplit(self.path)
        if url.path != '/api/profile':
            return self.respond('Fixture only supports profile writes', code=404)
        length = int(self.headers.get('Content-Length', '0'))
        if not 0 < length <= 32768:
            return self.respond('Invalid fixture body', code=400)
        key = parse_qs(url.query).get('id', ['default'])[0]
        if key not in PROFILES:
            return self.respond('Unknown fixture profile', code=404)
        body = self.rfile.read(length)
        # Same load-existing -> parse-update -> format flow as the PS5 route,
        # with production C and RAM-only storage. This is not socket/PS5 proof.
        try:
            result = subprocess.run([str(NORMALIZER)],
                input=PROFILES[key].encode() + b'\0' + body,
                capture_output=True, check=True, timeout=5)
            PROFILES[key] = result.stdout.decode()
        except (subprocess.SubprocessError, OSError, UnicodeError):
            return self.respond('Host profile normalization failed', code=500)
        self.respond('OK', 'text/plain')


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--port', type=int, default=8765)
    args = parser.parse_args()
    subprocess.run(['make', '-C', str(ROOT / 'tests'), 'browser-fixture'], check=True)
    print(f'Browser fixture: http://127.0.0.1:{args.port} (production parser, no hardware)', flush=True)
    ThreadingHTTPServer(('127.0.0.1', args.port), Preview).serve_forever()
