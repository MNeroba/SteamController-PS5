#!/usr/bin/env bash
# Host-only RAM fixture. Never run this script against a console.
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p output/playwright
log_dir="${WEBUI_LOG_DIR:-output/playwright}"
mkdir -p "$log_dir"
python3 - <<'PY'
import socket
with socket.socket() as probe:
    if probe.connect_ex(('127.0.0.1', 8765)) == 0:
        raise SystemExit('Port 8765 already in use; stop your existing fixture first')
PY
python3 tests/webui_preview.py > "$log_dir/preview.log" 2>&1 &
preview_pid=$!
trap 'playwright-cli --session steamcontroller-regression close >/dev/null 2>&1 || true; kill "$preview_pid" 2>/dev/null || true' EXIT
for attempt in $(seq 1 30); do
  kill -0 "$preview_pid" # A pre-existing listener must not masquerade as our fixture.
  if curl --fail --silent http://127.0.0.1:8765/ >/dev/null; then break; fi
  sleep 1
done
curl --fail --silent http://127.0.0.1:8765/ >/dev/null
playwright-cli --session steamcontroller-regression open about:blank --config tests/playwright-cli.json
if ! playwright-cli --session steamcontroller-regression --json run-code --filename tests/webui_remap_browser.js > "$log_dir/browser.json" 2>&1; then
  cat "$log_dir/browser.json"
  cat "$log_dir/preview.log"
  exit 1
fi
python3 scripts/check_browser_result.py "$log_dir/browser.json"
