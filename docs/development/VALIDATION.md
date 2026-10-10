# Consolidation validation — 2026-10-10

Baseline: main79f92cb. Source changes are limited to documentation/archive, build/CI/release tools, generated-header tracking, visible branding and matching browser expectations. No driver, transport or firmware code was redesigned. All 234 baseline Git blobs matched local files before consolidation; archive hashes, licenses and stable hardware-gate bytes were verified unchanged.

| Check | Command / result |
| --- | --- |
| Fresh source | Explicit complete file manifest exported to /tmp/sc-cleanup-fresh with no generated header, ELF, SDK or caches; normal host build generated its missing header automatically. This is a verified source export, not a claimed network clone. |
| Normal host | make -C /tmp/sc-cleanup-fresh/tests clean test — PASS |
| ASan/UBSan | make -C /tmp/sc-cleanup-fresh/tests sanitize — PASS, no sanitizer error |
| Image/layout | Nine artwork/embedding/hotspot tests PASS; all four original image hashes unchanged |
| DEV gate contracts | Six Python tests PASS, including subcases for critical blockers, IDs, bad ELF, checksum/warning/source packaging, browser threshold and separate stable gate |
| Browser | Pinned Playwright CLI0.1.22, explicit Chromium155.0.8059.12, task-specific session, production C profile fixture on loopback: PASS181 assertions. Four real screenshots refreshed. |
| SDK archive | v0.43 SHA256 a9cc9929f21b2b2c5d5b309f3bab4997067c45281c0622cf4838b1aecba66fcb verified |
| PS5 clean build | LLVM18.1.8 and pinned SDK; make clean followed by make — PASS, ELF64 x86-64,1740160 bytes |
| Actual local ELF SHA256 | 8e2e819deee54ab56f4faaf12f4452319d9a7d2ff6880090a62f5e3c2887c9c0 |
| Determinism | Generated page685596 UTF-8 bytes,41icons/fourimages. Repeat header SHA256 abb2defe1c16e212db7c338e784a95d41483539818ba9ca0f59854e4faebf96c identical |
| Integrity | All seven moved documents keep baseline Git blob hashes; original specification/build/memory snapshots preserved; all41production C modules and all original tests retained |
| Review | Active documentation links, four workflow YAML parses, JS/bash syntax, reviewed workspace git diff --check and scoped secret-pattern scan PASS |
| Stable gate | Original release.yml and check_release_gate.py byte-identical; checker exits1 for missing physical evidence, as expected |
| Hardware | NOT_RUN: PS5 measured firmware/loading, USB-C/Puck, DualSense, gameplay, motion/touch/haptics, real portal and shutdown |

Log SHA256: normal fb2e88a3dbfd7e8a068d6527e96f337ff971154195aa6cb01a0f3b1e3ab87703; sanitizer d055d9830d92873aaf9d898201818a2d9cb566c0c97f309725976886cea81901; SDK c88bb9680fc8bc877dc401bed966292e2adcf27cffe6b1bae40753c3ee55b968; browser script2d2b61412bceabe5821798fc0d8161a56c0a65ed071502bc83d9d4720f52f7ca; structured browser 884299c9b763b08536bdd4ccd58c9354f8f07867c6cca3e195e44aae52acb9c8.

Negative attempts and fixes remain in TEST_LOG M1-222. Local toolchain paths/debug data can change ELF hashes; no cross-platform bit-identical ELF claim. Remote source/Actions/prerelease acknowledgement follows in current memory. No DEV release or GitHub description change is claimed before confirmation; manual dispatch/settings currently require browser login.

Fresh export index check also reported two inherited whitespace issues in unchanged payload/shellui_pad.h and tests/test_safe_mode_contract.py. Their original Git blobs were deliberately preserved; these are baseline diagnostics, not new source edits. Only the reviewed change diff is claimed clean.
