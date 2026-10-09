#!/usr/bin/env python3
"""Static guard against unscoped payload credential mutations."""
from pathlib import Path
import re
import sys

root = Path(__file__).resolve().parents[1]
raw_credential_api = re.compile(
    r"\bkernel_(?:get|set)_ucred_(?:authid|caps)\s*\("
)
violations = []
for source in (root / "payload").rglob("*.c"):
    if source.name == "credential_kernel.c":
        continue
    for number, line in enumerate(source.read_text().splitlines(), 1):
        if raw_credential_api.search(line):
            violations.append(f"{source.relative_to(root)}:{number}: {line.strip()}")

if violations:
    print("Unscoped kernel credential API calls remain:", file=sys.stderr)
    print("\n".join(violations), file=sys.stderr)
    raise SystemExit(1)

main = (root / "payload/gc_main.c").read_text()
if "credential_kernel_restore(&g_runtime_credentials)" not in main:
    raise SystemExit("main shutdown does not restore runtime credentials")
if "if (ret != 0)" not in main:
    raise SystemExit("privileged startup result is not checked")
kernel = (root / "payload/credential_kernel.c").read_text()
if "runtime_request_stop();" not in kernel:
    raise SystemExit("persistent temporary-scope restore failure does not stop runtime")
if "pthread_mutex_lock(&g_credential_scope_lock)" not in kernel:
    raise SystemExit("temporary credential scopes are not serialized")

print("privilege contract checks passed")
