# Project memory — current status

**Snapshot:** 2026-10-10 12:53 UTC; FW 12.00 target adaptation validated locally, direct-main publication pending.
**Repository:** private GitHub repository MNeroba/SteamController-PS5; default branch main.
**Latest source commit:** 0b00c987d4c4e8ed80d52c9e2bbe723a1cd68b79. **Latest finalized docs/status commit:** 6b727b6502585c90878944a00d3fc29d6ef2bc36 (tree 90cbdb9684484fb0e852e60b0c255f6491047de5), a documentation-only descendant of d2107fe1b0719e5878d468b0f0120580ce4f8a1e. It preserves the complete source tree.

## Current readiness

- M0 source comparison and provenance are complete. Foundation: PuckbridgePS5 commit 78460113ef7c48502c765a1f83ba34d53c977134. OmniPad-PS5 commit b7682efce18ee0a8d7c80bbc90afbce72151ef96 is a read-only comparison; no OmniPad code was imported. See docs/BASE_SELECTION_ADR.md and docs/PROVENANCE.md.
- P0 host-side safe ownership, coordinated shutdown, credential rollback, safe defaults, USB allowlisting, peer authentication, and bounded hook shutdown are implemented and covered by production-module host tests. ProsperoOS ABI/runtime effects remain unverified.
- M1–M3 portable protocol, USB/session, mapping, motion/touch, profile, feedback, haptics, slot, lifecycle, and portal modules have broad host coverage; USB/Puck enumeration, gameplay, real haptics, and controller behavior remain unverified.
- Native PS5 BLE is research-only. No active HCI/radio operation was performed. Edge/ShellUI identity and multiple SC2 sessions are not supported features.
- The owner reports a PS5 on FW 12.00 and will not update it. This is the target, not a verified compatibility result. No PS5, Steam Controller, Puck, or DualSense runtime test has been performed. The release gate correctly blocks on H-01/H-02/H-03/H-04/H-05/H-09/H-12. No Release or installer is published. The Actions ELF artifact is temporary and is not a verified runtime binary.
- The published main tree has 232 recursive entries / 218 blobs, not truncated; the local candidate file blobs were previously compared against all 218 main entries. The desktop checkout is an older dirty overlay (branch fix/p0-safety at 78460113ef7c48502c765a1f83ba34d53c977134). Never reset it or publish its entire working tree; publish only explicitly reviewed paths on top of the current GitHub main tree with an expected-head, fast-forward update.

## Verification evidence

- GitHub Actions on latest finalized docs/status commit 6b727b6502585c90878944a00d3fc29d6ef2bc36: Host tests run 38047930840 PASS; PS5 cross-build run 38047930852 PASS. Cross-build artifact 11668467159 (ZIP SHA-256 8701025029d3fa43c77c8a2eeafea3a7526cf71aefcac6e52db92455dfcceceb) expires 2027-01-08T11:18:17Z; it is not a release.
- Local strict-C11 host suite: make -C tests clean test PASS.
- Local AddressSanitizer/UndefinedBehaviorSanitizer suite: make -C tests clean sanitize PASS.
- Local PS5 SDK v0.43 / LLVM 18.1.8 cross-build: PASS when LLVM_CONFIG and PS5_PAYLOAD_SDK are explicitly exported. ELF SHA-256 b78a4a5967921f463adbc54afecf866f6561f2d96108a1aeba475631c79249f7. Dependencies include libSceLibcInternal.sprx, libSceNet.sprx, libScePad.sprx, libSceSystemService.sprx, libSceUserService.sprx, and libkernel_web.sprx.
- SDK archive SHA-256: a9cc9929f21b2b2c5d5b309f3bab4997067c45281c0622cf4838b1aecba66fcb.
- After target adaptation, `python3 -m unittest tests.test_release_gate` passes 9 checks, including exact FW 12.00 matching, proof that no specific firmware is pinned, and rejection of substring-only matches. The real release gate still exits 1 as expected: required hardware evidence is absent and the owner-reported FW is explicitly marked unverified in the matrix.
- Repeated after the FW 12.00 adaptation: `make -C tests clean test` PASS; `make -C tests clean sanitize` PASS; SDK v0.43 / LLVM 18.1.8 clean cross-build PASS. Resulting ELF SHA-256 remains `b78a4a5967921f463adbc54afecf866f6561f2d96108a1aeba475631c79249f7`; dependencies remain the same six expected PS5 SDK libraries. These are host and compile/link checks only.

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

Publish only the reviewed firmware-target, checker, test, documentation, and memory paths as a fast-forward child of GitHub main `c49df86ac2b8dd177aac5af28aa330aa2ce450ba`; verify the full tree has no deletions, then inspect Host and PS5 cross-build Actions. Do not update the PS5 or claim FW 12.00 compatibility without actual device/runtime evidence.
