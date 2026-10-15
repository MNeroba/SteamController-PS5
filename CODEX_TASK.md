# SteamController-PS5 — active execution specification

Revision 1.4 · 2026-10-12 · Target: owner-reported **PS5 FW 13.60**, hardware-unverified · **SDK v0.43**

The [full original specification](docs/archive/CODEX_TASK-1.1.md) is preserved verbatim and remains authoritative for detailed contracts, safety rules, evidence labels and acceptance criteria. Read it in full before substantial implementation. This file consolidates the current execution order; it does not revoke uncompleted requirements. Historical setup/PR instructions are superseded by the owner's direct-main solo-development policy and separate experimental DEV authorization.

## Mission and evidence

One maintainable native payload specialized for Steam Controller 2026 via USB-C and official Puck. Keep full Steam input capabilities in a reusable core with one slot/session authority and virtual DualSense. Reuse audited modules before rewriting. Do not conflate implemented code, host tests, SDK build or physical runtime success.

Use VERIFIED_CODE, VERIFIED_HOST_TEST, VERIFIED_PS5_BUILD, VERIFIED_HARDWARE, UPSTREAM_CLAIM, EXTERNAL_HARDWARE, HYPOTHESIS and BLOCKED. No compatibility claim for 13.60 before a measured real run; earlier 12.00 records remain historical. Preserve legal notices and exact pins. No stock-radio/BLE/HCI, firmware writes or forced DualSense disconnect without the original hardware/approval gates.

## Completed software milestones

- M0: PuckbridgePS5/OmniPad source comparison, pins, license/provenance and foundation ADR. Selected Puckbridge `78460113ef7c48502c765a1f83ba34d53c977134`; OmniPad `b7682efce18ee0a8d7c80bbc90afbce72151ef96` remains reference-only.
- P0 host hardening: owned VDA cleanup, coordinated stop, checked credential rollback, Safe Mode, Steam/interface-restricted USB, authenticated optional portal and bounded hook rollback. Physical PS5 consequences remain open.
- M1–M3 portable protocol/session/slot/profile/motion/touch/feedback modules, USB/Puck adapter integration and one-SC2 feedback lease; broad host fault/concurrency coverage. All original physical acceptance criteria remain outstanding.
- Image remap UI, canonical complete profile persistence, Shift/analog provenance, Long flags, navigation guards and complete HTTP delivery have regression coverage through M1-220/221.

Details: [current status](project-memory/CURRENT_STATUS.md), [feature table](STEAM_CONTROLLER_2026.md), [architecture](docs/ARCHITECTURE.md), [issues](project-memory/ISSUES.md).

## Current execution order

- **Current dev.5 preparation — M1-245:** owner requested VDA/storage/account-log fixes, explicitly WITHOUT publication. Storage diagnostics/private permission regression and log filtering are implemented/host-tested; observed pending VDA ownership is still blocked. Keep critical gate closed, prepare exact-source build for review, do not create a release/tag or dispatch DEV workflow. See docs/development/DEV5_CANDIDATE.md.
- **Published dev.4 runtime safety update** — ambiguous Add creation latch/global serialization, live-fd USB identity check and bounded feedback status. Owner explicitly approved; sourcea5c20346392eac0eb9fa04cc201cd214e2957363, workflow38588689667, seven assets/one ELF. Host/sanitizer/Chromium220/SDKv0.43 gates pass; external dev.2/FW5.50 report and physical acceptance remain unresolved. No radio/storage migration or safety-policy relaxation.
- **Published dev.3: LibrePad v1.1.2 findings** — confirmed user-list consensus and owned virtual-pad recovery, USB completion ownership, authenticated bounded diagnostics, concise notifications, Payload Manager metadata and mandatory two-category changelogs. Preserve the single ELF/optional tile and all dev.2 features; no storage migration or general-controller protocol import. See ADR-055 and M1-235/236. Owner approved publication; dev/v0.1.0-dev.3 is published from9aab6dbae166b28416a9a00e6346f85bf12e524e via successful workflow38225113776. Future DEV publication requires separate explicit approval.
- First launch/dev.2 is published; [full historical owner task](docs/development/FIRST_LAUNCH_TASK.md) and M1-229 retain its evidence.
- **Audit remediation P0–P3** — [confirmed findings](docs/development/STATIC_AUDIT.md). Resolve the listed code defects before DEV publication; preserve critical-blocker checks. New first-launch work does not establish hardware compatibility.


1. Finish file-by-file consolidation without deleting production modules/tests/notices. Keep a complete tree and [audit](docs/development/REPOSITORY_AUDIT.md); generated assets must build from a clean checkout.
2. Keep the concise landing page, documentation index, contributor rules and current memory accurate. Preserve full historical evidence and negative results.
3. For the explicitly authorized **experimental DEV prerelease**, follow [DEV_RELEASE](docs/development/DEV_RELEASE.md). Exact tested SHA, pinned SDK, normal host, ASan/UBSan, Chromium, artwork/determinism, fresh SDK build and checksum are mandatory. Block known critical code defects. After explicit owner approval use the selected new `dev/` version, experimental warning, source/SDK/checksum/notices and real ELF only. No hardware claim.
4. Stable `v*` publication still requires H-01/H-02/H-03/H-04/H-05/H-09/H-12, exact measured firmware and matching TEST_LOG evidence. Do not weaken its checker/workflow. DEV authorization does not authorize console deployment or active platform experiments.
5. When real PS5/controller/Puck access exists: execute the [test plan](docs/TEST_PLAN.md) and record each transport, model, firmware, SHA and outcome. Confirm VDA handle semantics, privilege rollback, cancellation/neutralization, native DualSense, gameplay, motion/touch/haptics and portal. Keep unrun rows NOT_RUN.
6. Native BLE stays passive/evidence-gated; Edge/custom ShellUI identity and multi-SC2 feedback are separate research. Complete feasibility gates before adding a BLE stack. Keep safe USB/Puck work independent of research.

## Session/publication contract

Read AGENTS, full original specification, source/evidence, architecture, BLE, test plan and all project memory. Reconcile live main/CI/refs. Write failing regressions before bug fixes and run risk-appropriate tests. Append actual commands, hashes, issues, decisions and negative attempts immediately.

Publish only reviewed paths on the verified complete main tree with expected parent and force=false. The desktop checkout is an inherited dirty overlay, not an authoritative Git snapshot. Never stage/publish it wholesale. External contributors use PRs per CONTRIBUTING; the owner uses direct main updates until the first successful release. Do not rewrite history without a separate backup/proposal and explicit force-push approval.

At end: inspect diff, run diff-check/tests/sanitizers/build/browser as relevant, verify publication/Actions and actual downloadable assets, update persistent current status and exact next commands, report only acknowledged commits/releases. Hardware-dependent work remains explicit BLOCKED, not simulated success.

### Owner target update — 2026-10-11 (M1-231)

Owner reports upgrading from FW12.00 to13.60. Use13.60 for future measured hardware/launcher/BLE checks; no update was performed by this agent. Keep SDKv0.43 and safety gates. Published dev.2 metadata remains unchanged. Current packaging policy and warning now target13.60 unverified; preserve historical12.00 results and published dev.2 records. Never relabel old test results.

### Current dev.3 task — M1-232

Owner requests supplied icon and on-console settings launcher in **one ELF**. Implemented optional authenticated confirmation, embedded icon/metadata and owned exact-title registration; local software/build evidence only. Fresh-source and remote gates passed; experimental dev/v0.1.0-dev.3 is published and recorded in M1-238. Current console target owner-reported FW13.60, unmeasured; SDKv0.43 and safety defaults retained. H-13 is hardware-gated. Historical12.00/dev.2 records remain unchanged. See ADR-054 and SETTINGS_TILE.md.


## Dev.3 acceptance additions — 2026-10-12

Pin LibrePadPS5 v1.1.2 source7466317b4b356249e07770f3914c995e1a4d633a and inspect actual virtual_controller/usb_device/http_server/metadata implementation before adopting general ideas. Do not import storage migration, XInput or 8BitDo commands. Use three consistent spaced empty user-list reads after prior login, two same-list observations for relogin, retain owned deletion failures and pending creation reservations, and preserve paused/released/disconnected exclusions. No automatic account selection or native DualSense mutation.

Host coverage includes late IN/OUT completion, invalid frames/status/lengths, failed START/COMPLETE/STOP/CLOSE/UNINIT, watchdog neutralization and fresh generations. Download reports require existing auth/Origin/LAN admission, bounded output and redaction, no filesystem path parameter; incomplete startup/no controller is a supported diagnostic state. Payload Manager JSON validates version/filename/URL/checksum without changing the single ELF. CHANGELOG.md has exactly Bug Fixes and New Features / Improvements for every version and matching release notes; historical releases/tags remain unchanged. Repeat normal, sanitizer, Chromium220+, deterministic assets, SDKv0.43 clean build, dependency/checksum and diff checks. All physical H-13/H-14/H-15 tests remain NOT_RUN until actual console access. Prepare/push source; publication is a separate explicit owner approval.
