# Project memory — current status

**Snapshot:** 2026-10-10 11:16 UTC.
**Repository:** private GitHub repository MNeroba/SteamController-PS5; default branch main.
**Latest source commit:** 0b00c987d4c4e8ed80d52c9e2bbe723a1cd68b79. **Last CI-verified docs publication:** d2107fe1b0719e5878d468b0f0120580ce4f8a1e (tree fb5d06aa2357346ada46c64da5078f19ae0d6f64). The current memory synchronization is a documentation-only descendant and preserves the full source tree.

## Current readiness

- M0 source comparison and provenance are complete. Foundation: PuckbridgePS5 commit 78460113ef7c48502c765a1f83ba34d53c977134. OmniPad-PS5 commit b7682efce18ee0a8d7c80bbc90afbce72151ef96 is a read-only comparison; no OmniPad code was imported. See docs/BASE_SELECTION_ADR.md and docs/PROVENANCE.md.
- P0 host-side safe ownership, coordinated shutdown, credential rollback, safe defaults, USB allowlisting, peer authentication, and bounded hook shutdown are implemented and covered by production-module host tests. ProsperoOS ABI/runtime effects remain unverified.
- M1–M3 portable protocol, USB/session, mapping, motion/touch, profile, feedback, haptics, slot, lifecycle, and portal modules have broad host coverage; USB/Puck enumeration, gameplay, real haptics, and controller behavior remain unverified.
- Native PS5 BLE is research-only. No active HCI/radio operation was performed. Edge/ShellUI identity and multiple SC2 sessions are not supported features.
- No PS5, Steam Controller, Puck, or DualSense hardware test has been performed. The release gate correctly blocks on H-01/H-02/H-03/H-04/H-05/H-09/H-12. No Release or installer is published. The Actions ELF artifact is temporary and is not a verified runtime binary.
- The published main tree has 232 recursive entries / 218 blobs, not truncated; the local candidate file blobs were previously compared against all 218 main entries. The desktop checkout is an older dirty overlay (branch fix/p0-safety at 78460113ef7c48502c765a1f83ba34d53c977134). Never reset it or publish its entire working tree; publish only explicitly reviewed paths on top of the current GitHub main tree with an expected-head, fast-forward update.

## Verification evidence

- GitHub Actions on finalization commit d2107fe1b0719e5878d468b0f0120580ce4f8a1e: Host tests run 38047541353 PASS; PS5 cross-build run 38047541326 PASS. Cross-build artifact 11668855395 (ZIP SHA-256 24004f11c987e8b89d1a4b379dcfab51484bde88179b72e7de231e9b93a9fc67) expires 2027-01-08; it is not a release.
- Local strict-C11 host suite: make -C tests clean test PASS.
- Local AddressSanitizer/UndefinedBehaviorSanitizer suite: make -C tests clean sanitize PASS.
- Local PS5 SDK v0.43 / LLVM 18.1.8 cross-build: PASS when LLVM_CONFIG and PS5_PAYLOAD_SDK are explicitly exported. ELF SHA-256 b78a4a5967921f463adbc54afecf866f6561f2d96108a1aeba475631c79249f7. Dependencies include libSceLibcInternal.sprx, libSceNet.sprx, libScePad.sprx, libSceSystemService.sprx, libSceUserService.sprx, and libkernel_web.sprx.
- SDK archive SHA-256: a9cc9929f21b2b2c5d5b309f3bab4997067c45281c0622cf4838b1aecba66fcb.
- Release gate exit 1 is expected while required hardware rows are unverified; this is a release blocker, not a passing release gate.

## Important open limits

- payload/steam_usb.c and scanner tests establish host policy only. The actual Prospero USB descriptor/ioctl behavior, device IDs/descriptors, driver detach, endpoint completion and cancellation require a real device and PS5.
- payload/slot_registry.c validates ownership and lifecycle in host tests; the actual scePad handle semantics, assignment and native DualSense coexistence require PS5.
- Credential and game-hook tests cover injected host behavior only. Remote kernel calls, game memory races, target leases, system UI, shutdown latency and stock DualSense recovery require a PS5 test setup.
- Portal authorization and HTTP parsing are host-tested. Prospero sockets/filesystem/browser behavior is not; HTTP remains plaintext and must stay on a trusted local network.
- BLE gates B1–B6 remain blocked. Do not run active Bluetooth/HCI operations or change pairing/radio state without passive diagnostics and explicit approval.
- The shell could not resolve github.com for git clone; GitHub connector publication and readback were used instead.

## Continuation protocol

1. Read CODEX_TASK.md, docs/, this file, DECISIONS.md, ISSUES.md, and recent TEST_LOG.md / SESSION_LOG.md.
2. Re-fetch main and its complete recursive Git tree. Compare any candidate file hashes and preserve every parent path; update the ref only with expected_sha and force=false.
3. Run host tests, ASan/UBSan, and SDK cross-build for relevant changes. Keep compile/build evidence separate from runtime/hardware evidence.
4. Append test, session, finding, and decision records. Preserve negative results.
5. Do not deploy to a stock console, touch Bluetooth, publish a Release, or mark hardware rows verified without their actual evidence.

## Exact next action

Verify the final documentation/memory commit on GitHub main, confirm its tree retains all source files, and read the Actions runs for that commit. Then stop at the hardware gates: no further PS5-dependent claim or release can be completed without the physical console/controller/Puck setup.
