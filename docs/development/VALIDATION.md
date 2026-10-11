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

## M1-227 — dev.2 software validation — 2026-10-11

Complete255-file reviewed source export /tmp/sc-dev2-fresh, no preexisting generated header/ELF/SDK/cache. Normal and ASan/UBSan suites PASS,38 C suites plus Python contracts. Chromium CLI0.1.22 PASS202 (original183 retained); clean-export repeat PASS202. Clean SDKv0.43LLVM18.1.8 build PASS. Header determinism SHA2562b2071a4058792dd758de531d823acb6892070814e375136592b57161f9a0166,691930-byte page,41icons/four unchanged images. Local fresh ELF SHA256d299cc90947e051c2d30dc27826f94e7f17108dd945f2f369e8a710921a81b4f; published Linux asset digest is recorded separately after release acknowledgement. Scoped diff/credential/attribution checks PASS; full41 production C units retained. Negative attempts and fixes: TEST_LOG M1-227. All hardware tests NOT_RUN; FW12.00 unverified; stable release gate remains closed. DEV2 acknowledgement pending at this entry.

Log sc-dev2-fresh-gates2.log SHA256 1735fbc59d0c73adde80c3935f24d70df746de1dbc6f2a9428534e04a45214c8.

Log sc-dev2-final-host.log SHA256 355e448e327f15d8154cea60db7251d4b74317658fbc56808ae6f166248498ed.

Log sc-dev2-fresh-browser.log SHA256 e07432da1efe8efed1ae240783c141159c2da16aaf88916003aa020e68e10ee9.

Log browser.json SHA256 2fe105a9719a6178d940bf3d65c9403fad0b25ff551cea859ac9c8a9b18752fe.

## M1-232 — dev.3 complete-source software validation

Verified main baseline2a2fb9, all255 blobs SHA-matched. Explicit40-path overlay exports266 files/all43 productionC/no deletion and no generated headers in source. Fresh normal40 C/Python suites, ASan-UBSan and Chromium214 PASS; final targeted native-adapter normal/sanitizer check and SDKv0.43/LLVM18 rebuild also PASS. Local fresh ELF SHA256d96cda4f013eaa3c159904944de8523637c1993277feaf25e04b1a1d08e677d8; exact embedded icon/metadata and checksum packaging verified, distinct from any future published Linux artifact hash. Negative results and paths remain in TEST_LOG M1-232. Source main0a3dacb453d6ea925a36888cd5c5784e68f26b6f/tree72458c460507610fd925b1368b477463e10205f2 acknowledged; push/release results recorded separately. No physical operation performed. FW13.60 and H-13 remain NOT_RUN, stable gate unchanged.
