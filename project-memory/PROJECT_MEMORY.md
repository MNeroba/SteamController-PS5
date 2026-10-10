# Project memory — current status

**Snapshot:** 2026-10-10; repeat Remap audit hardening is published and Host/Chromium/ASan-UBSan/PS5 Actions pass. FW 12.00 remains an unverified target.
**Repository:** private GitHub repository MNeroba/SteamController-PS5; default branch main.
**Latest source commit:** 6914442f6b813f90d1156f93d18f532919ff13e8 (tree 4bff80b5c83f088da2fef77b826e22c9314c5607; 234 blobs). Firmware target adaptation remains the previously verified source 81501afa53e2acd329db0298ffccdc533161e749; historical records below preserve earlier results.

## Current readiness

- M0 source comparison and provenance are complete. Foundation: PuckbridgePS5 commit 78460113ef7c48502c765a1f83ba34d53c977134. OmniPad-PS5 commit b7682efce18ee0a8d7c80bbc90afbce72151ef96 is a read-only comparison; no OmniPad code was imported. See docs/BASE_SELECTION_ADR.md and docs/PROVENANCE.md.
- P0 host-side safe ownership, coordinated shutdown, credential rollback, safe defaults, USB allowlisting, peer authentication, and bounded hook shutdown are implemented and covered by production-module host tests. ProsperoOS ABI/runtime effects remain unverified.
- M1–M3 portable protocol, USB/session, mapping, motion/touch, profile, feedback, haptics, slot, lifecycle, and portal modules have broad host coverage; USB/Puck enumeration, gameplay, real haptics, and controller behavior remain unverified.
- Native PS5 BLE is research-only. No active HCI/radio operation was performed. Edge/ShellUI identity and multiple SC2 sessions are not supported features.
- The owner reports a PS5 on FW 12.00 and will not update it. This is the target, not a verified compatibility result. No PS5, Steam Controller, Puck, or DualSense runtime test has been performed. The release gate correctly blocks on H-01/H-02/H-03/H-04/H-05/H-09/H-12. No Release or installer is published. The Actions ELF artifact is temporary and is not a verified runtime binary.
- The published image-remapping main tree has 249 recursive entries / 233 blobs, not truncated; commit 4065ff4 changed exactly 18 existing reviewed paths and added exactly 15 intended files, with no deletions. The earlier FW target commit 81501af preserved its full 218-blob tree. The desktop checkout is an older dirty overlay (branch fix/p0-safety at 78460113ef7c48502c765a1f83ba34d53c977134). Never reset it or publish its entire working tree; publish only explicitly reviewed paths on top of the current GitHub main tree with an expected-head, fast-forward update.

## Verification evidence

- GitHub Actions on earlier finalized docs/status commit 6b727b6502585c90878944a00d3fc29d6ef2bc36: Host tests run 38047930840 PASS; PS5 cross-build run 38047930852 PASS. Cross-build artifact 11668467159 (ZIP SHA-256 8701025029d3fa43c77c8a2eeafea3a7526cf71aefcac6e52db92455dfcceceb) expires 2027-01-08T11:18:17Z; it is not a release.
- Local strict-C11 host suite: make -C tests clean test PASS.
- Local AddressSanitizer/UndefinedBehaviorSanitizer suite: make -C tests clean sanitize PASS.
- Local PS5 SDK v0.43 / LLVM 18.1.8 cross-build: PASS when LLVM_CONFIG and PS5_PAYLOAD_SDK are explicitly exported. ELF SHA-256 667db8369ebe58163b9afe677d2873c62cd56df56ade62f8e6b334d978ed2b07 after image remapping. Dependencies include libSceLibcInternal.sprx, libSceNet.sprx, libScePad.sprx, libSceSystemService.sprx, libSceUserService.sprx, and libkernel_web.sprx.
- SDK archive SHA-256: a9cc9929f21b2b2c5d5b309f3bab4997067c45281c0622cf4838b1aecba66fcb.
- After target adaptation, `python3 -m unittest tests.test_release_gate` passes 9 checks, including exact FW 12.00 matching, proof that no specific firmware is pinned, and rejection of substring-only matches. The real release gate still exits 1 as expected: required hardware evidence is absent and the owner-reported FW is explicitly marked unverified in the matrix.
- Repeated after the FW 12.00 adaptation: `make -C tests clean test` PASS; `make -C tests clean sanitize` PASS; SDK v0.43 / LLVM 18.1.8 clean cross-build PASS. Resulting ELF SHA-256 remains `b78a4a5967921f463adbc54afecf866f6561f2d96108a1aeba475631c79249f7`; dependencies remain the same six expected PS5 SDK libraries. These are host and compile/link checks only.
- GitHub Actions on published target-validation commit 81501afa53e2acd329db0298ffccdc533161e749: Host tests run 38053866134 PASS; PS5 cross-build run 38053866148 PASS. Temporary artifact 11670507757 (ZIP SHA-256 `7259c645662dcdd0e664c0d95d6ccde17fa453992fc81e032eb6d9b822b622ba`) expires 2027-01-08T12:57:22Z; it is not a release or runtime proof.

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

The firmware retarget and image-remapping implementation are complete for host/browser/build validation. M1-216 passes 114 local browser checks; [Host Actions](https://github.com/MNeroba/SteamController-PS5/actions/runs/38067268430) also passes its normal suite, ASan/UBSan and Chromium browser regression, and [PS5 cross-build](https://github.com/MNeroba/SteamController-PS5/actions/runs/38067268427) passes. Reproduce locally with `python3 payload/gen_html.py`, `python3 tests/webui_preview.py`, `playwright-cli open about:blank --browser msedge`, and `playwright-cli run-code --filename tests/webui_remap_browser.js`; see `docs/WEBUI_REMAP.md`. Actual portal H-09 must include front/back hotspots, live highlighting, authentication and persisted profile reload on the PS5. Keep the first-release gate blocked until real hardware records replace the owner-reported placeholders and pass H-01/H-02/H-03/H-04/H-05/H-09/H-12. Do not update the console, deploy the payload, or claim FW 12.00 compatibility without actual device/runtime evidence.

## Active P1 image remapping update (M1-214)

Paired Steam Controller / DualSense artwork UI is implemented and locally verified: four exact user-provided WebPs, preserved SVG coordinates/front/back orientation, all 24 inputs and 18 digital outputs, transactional activators/profiles, combo highlights, keyboard/touch and responsive layout. Seven portable embedding tests and 99 Microsoft Edge fixture checks pass; full host and ASan/UBSan suites and SDK v0.43 build pass. Current local ELF SHA-256 `667db8369ebe58163b9afe677d2873c62cd56df56ade62f8e6b334d978ed2b07`. No firmware/security/USB/PS5 backend changes. Publication rights confirmed by owner; no new asset license invented. Source publication acknowledged on main at `4065ff41d51b86fdf3a60f972f72b2b24855c647`; both Host and cross-build Actions pass (M1-215). Actual FW 12.00 portal rendering/persistence/live input and all existing console gates remain NOT_RUN. See ADR-048/F095/P1-REMAP-UI-01 and `docs/WEBUI_REMAP.md`.

### Latest source Actions (M1-215)

Source `4065ff41d51b86fdf3a60f972f72b2b24855c647`: [Host tests](https://github.com/MNeroba/SteamController-PS5/actions/runs/38061016300) and [PS5 cross-build](https://github.com/MNeroba/SteamController-PS5/actions/runs/38061016345) both PASS. Temporary `ghost-control-ps5-cross-build` artifact ID 11673116887, ZIP SHA-256 `bf4acba960df78e5baa70c46a17dcaa2d7b0824ceeed33cb0dedb192c8d951ca`, expires 2027-01-08T14:47:32Z. The ELF is available via that run's artifact, not as a Release. Target runtime/firmware remains unverified.


## Active Remap UX follow-up (M1-216)

DualSense screenshot calibration and the attached Shift/import/output-picker audit were addressed and published as `408900592f50b1eb39b0003c357c95a137410f2f`. Both views map L1/R1 to upper shoulders and L2/R2 to lower triggers; △/○ are aligned. Shift, `NONE`, Merge import, direct output staging, grouped actions, glyph labels and coarse-pointer targets are covered by eight portable tests and 114 local browser checks. Full normal and ASan/UBSan suites pass; SDK v0.43 build ELF SHA-256 `3f19ca8a85af5583840ccc9542aa91796740b6b166d6ae553723e7b588565fbf`. Host/Chromium Actions `38067268430` and cross-build Actions `38067268427` pass. Source publication was a verified no-delete 23-path fast-forward. Hardware remains untested. See ADR-049/F097/F098/P1-REMAP-UX-01/M1-216/M1-217 and `docs/WEBUI_REMAP.md`.

## Exact next action

Run H-09 on an actual PS5 using its measured firmware, including image rendering/hotspots, live button highlighting, authenticated portal, and profile save/reload. Preserve all other hardware rows as NOT_RUN until their specified setups are available. Do not deploy or claim PS5 FW 12.00 compatibility before that evidence.

## Active repeat audit — M1-218 / ADR-050 (supersedes older current snapshots)

Baseline main `799d301b8d8936244fe5ea50ab9967b21cccee2d`. Five confirmed Remap audit issues fixed: imported identifier/HTML safety, Shift/full-trigger consistency, geometric touch selection and disjoint outputs, JS/C Turbo/Toggle policy, canonical snapshot-aware Save readback. Mobile scroll cue added; fixture reuses production C parse/format. Local nine portable/162 browser assertions, full Host/ASan-UBSan and SDK v0.43 clean build pass (ELF `247a25a7962336ddc4b31c36ef8c251502b2c27cd7d158e35c69cd52c7a15564`). Publish the exact reviewed overlay and verify Actions next. History above retains prior results; physical H-09 and all firmware/USB/controller gates remain NOT_RUN. Optional Replace Profile remains P2; current supported import is Merge.

## Latest acknowledged source and exact next action (M1-219)

`6914442f6b813f90d1156f93d18f532919ff13e8` is on main, complete 234-blob tree, no removed paths. Host `38070068676` and PS5 cross-build `38070068771` PASS. Temporary ELF build artifact `11676088176` is available through the cross-build run; no Release. Local 162 browser assertions, nine portable contracts, normal/ASan-UBSan and SDK v0.43 clean build pass. P0 imported HTML safety and all audited Shift/touch/Turbo/Save/persistence issues are fixed at the host/build evidence level. Physical H-09 must repeat authenticated rendering, hostile rejection, canonical saved/cleared state after restart, Shift-trigger and boundary touch behavior on the exact installed FW. All PS5/USB/Puck/haptics/coexistence gates remain NOT_RUN. Optional Replace Profile is deferred P2; Merge is supported.
