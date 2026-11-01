# SteamController-PS5 — active execution specification

Revision 1.8 · 2026-11-01 · Target: owner-reported **PS5 FW 13.60**, hardware-unverified · **SDK v0.43**

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

- **M1-250 dev.7 published:** Published experimental [v0.1.0-dev.7](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v0.1.0-dev.7) from `f4ec9ae9e7e287c957a5602493a5b7d46449aed3` by [DEV workflow 41300890370](https://github.com/MNeroba/SteamController-PS5/actions/runs/41300890370) on 2026-11-01 11:15:23 UTC (2026-11-01 13:15:23 EET). Seven assets contain exactly one ELF, `SteamController-PS5-v0.1.0-dev.7.elf`, 2,091,960 bytes, SHA-256 `9bc6e1672bc8d20c8e9fcff85359c976523bdf6296a64e1e92dae34e6d7edb73`. PR8 was merged with contributor commits preserved; both descriptor P1s and the post-drain observation-window issue are resolved in code/host regressions. Exact PR, merged-main and repeated DEV Host/ASan/UBSan/Chromium220/asset/SDKv0.43/LLVM18/checksum gates passed. This is VERIFIED_HOST_TEST and VERIFIED_PS5_BUILD only. Contributor FW13.20/Puck observations at original c3a7ddb are EXTERNAL_HARDWARE; this revised ELF, owner target FW13.60, native DualSense coexistence, gameplay, motion/haptics, console cleanup and tile launch remain physically unverified. No historical result/tag/release was changed. See project-memory/PR8_REVIEW.md.


- **Published dev.5 — M1-246 (2026-10-16):** owner explicitly authorizes the analogous event-correlation approach temporarily, then publication for the log author's tests. Implement one pending Add transaction in the existing registry, bounded typed virtual-event identification, local MBus only on Steam/PS with native protection, no Shell patches. Preserve distinction from a proven Add ABI, native coexistence and actual storage errno remain hardware gates. All software/build gates passed on `382e4ecef4937356b72f9e1786917ae7147c0415`; dev/v0.1.0-dev.5 is published via successful workflow38810046681. See docs/development/VDA_PENDING.md and current status for the receipt. Next: controlled tester evidence and remaining hardware gates; future publication needs separate approval.

- **M1-248 tester follow-up (2026-10-31):** protected token initialization and a single bounded VDA log-reader retry were pushed to `main` at `e03f2c4`; exact-source Host and SDK workflows passed. The tester's existing dev.5 log is from the old ELF and does not test these fixes. No VDA Add/ownership, PS5 filesystem or firmware claim is made.
- **Published dev.6 — M1-249 (2026-11-01):** token initialization and bounded VDA observer retry fixes were published from `4c8b89bc789bf1dfd064264ff432d3919b5a1899` via successful workflow [41246199948](https://github.com/MNeroba/SteamController-PS5/actions/runs/41246199948). Repeated Host/ASan/UBSan/Chromium/asset and SDK v0.43/checksum gates passed. The repository is Public; hardware claims remain blocked pending exact-console tests. See current status and release receipt. Future DEV releases require separate owner approval.

- **Historical dev.5 preparation — M1-245:** owner requested VDA/storage/account-log fixes, explicitly WITHOUT publication. Storage diagnostics/private permission regression and log filtering are implemented/host-tested; observed pending VDA ownership is still blocked. Keep critical gate closed, prepare exact-source build for review, do not create a release/tag or dispatch DEV workflow. See docs/development/DEV5_CANDIDATE.md.
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

### Historical dev.3 task — M1-232

Owner requests supplied icon and on-console settings launcher in **one ELF**. Implemented optional authenticated confirmation, embedded icon/metadata and owned exact-title registration; local software/build evidence only. Fresh-source and remote gates passed; experimental dev/v0.1.0-dev.3 is published and recorded in M1-238. Current console target owner-reported FW13.60, unmeasured; SDKv0.43 and safety defaults retained. H-13 is hardware-gated. Historical12.00/dev.2 records remain unchanged. See ADR-054 and SETTINGS_TILE.md.


## Dev.3 acceptance additions — 2026-10-12

Pin LibrePadPS5 v1.1.2 source7466317b4b356249e07770f3914c995e1a4d633a and inspect actual virtual_controller/usb_device/http_server/metadata implementation before adopting general ideas. Do not import storage migration, XInput or 8BitDo commands. Use three consistent spaced empty user-list reads after prior login, two same-list observations for relogin, retain owned deletion failures and pending creation reservations, and preserve paused/released/disconnected exclusions. No automatic account selection or native DualSense mutation.

Host coverage includes late IN/OUT completion, invalid frames/status/lengths, failed START/COMPLETE/STOP/CLOSE/UNINIT, watchdog neutralization and fresh generations. Download reports require existing auth/Origin/LAN admission, bounded output and redaction, no filesystem path parameter; incomplete startup/no controller is a supported diagnostic state. Payload Manager JSON validates version/filename/URL/checksum without changing the single ELF. CHANGELOG.md has exactly Bug Fixes and New Features / Improvements for every version and matching release notes; historical releases/tags remain unchanged. Repeat normal, sanitizer, Chromium220+, deterministic assets, SDKv0.43 clean build, dependency/checksum and diff checks. All physical H-13/H-14/H-15 tests remain NOT_RUN until actual console access. Prepare/push source; publication is a separate explicit owner approval.


### CI maintenance checkpoint — M1-251 (2026-11-01)

PR3–7 Actions updates are integrated with fresh combined-candidate Host/sanitizer/Chromium/SDK/secret and non-publishing artifact/API smoke evidence. See docs/development/CI_ACTIONS.md. No payload/SDK/version/ELF or hardware-gate change. Issue2 tracks controlled verification of the published dev.7 source/ELF; the original dev.5 task remains historical.
