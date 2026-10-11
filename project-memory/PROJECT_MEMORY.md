# Project memory — current continuation entry

Read [CURRENT_STATUS](CURRENT_STATUS.md) first, then the append-only evidence below. Historical pre-consolidation summary is preserved verbatim at [PROJECT_MEMORY-before-dev](../docs/archive/PROJECT_MEMORY-before-dev.md); it contains earlier/intermediate statements, not current publication status.

## Identity and foundation

SteamController-PS5, private repository MNeroba/SteamController-PS5, main. Owner uses direct non-force main updates while developing solo; external contributors follow CONTRIBUTING/PR rules.

M0 is complete: PuckbridgePS5 `78460113ef7c48502c765a1f83ba34d53c977134` selected from source comparison; OmniPad `b7682efce18ee0a8d7c80bbc90afbce72151ef96` reference-only, no imported code. PoorDS4/Ghostcontrol/SDK/asset notices retained in PROVENANCE. Payload code is GPL-3.0-or-later; photos have separate owner publication permission.

## Evidence and safety

Host hardening and reusable protocol/USB/slot/profile/motion/touch/feedback/UI components are implemented and covered. Main sourceffff5e0 has green normal/ASan-UBSan/183Chromium and SDK Actions (M1-225). SDK v0.43 archive SHA-256 a9cc9929f21b2b2c5d5b309f3bab4997067c45281c0622cf4838b1aecba66fcb; LLVM18. Physical PS5, controller/Puck, Prospero ioctls/ABI and FW12.00 behavior have never been tested here.

Safe defaults disable bridge probe, game hooks, DualSense handoff and WebUI. Restricted USB policy, owned devices, credential rollback and coordinated cleanup have host tests. Stock DualSense must remain usable; no built-in Bluetooth/HCI or active firmware/system experiment without separate physical evidence/approval.

The owner explicitly authorized an experimental DEV prerelease on2026-10-10. Follow DEV_RELEASE.md and separate manual dev/ workflow with exact source/full software gates and warnings. Stable v* checker remains hardware-gated. DEV authorization does not authorize deployment or claim compatibility.

## Publication and workspace

Desktop checkout: old fix/p0-safety at upstream78460113 with inherited dirty/untracked overlay. GitHub main/full tree is authoritative. All234 local baseline files matched main79f92cb before cleanup. Never reset/stage/push the whole desktop overlay. Publish reviewed allowlisted blobs on complete parent tree, verify all retained hashes and intentional moves/deletion, update expected parent force=false, read back source/CI.

GitHub connector works for Git objects and read-only Actions, not manual workflow dispatch. Edge permission was rejected; authenticated in-app GitHub access subsequently resolved the blocker. Description update and manual DEV dispatch acknowledged; no credentials copied.

## Next safe action

First-launch and 16-item static audit remediation implemented (ADR-053/M1-227). Fresh-export Chromium202 and scoped diff/secret checks PASS; publish complete reviewed tree on verified main with expected_sha/force=false, then require Host/SDK push Actions for exact SHA. Dispatch dev-release.yml on main with version v0.1.0-dev.2 and that tested SHA; verify prerelease/tag/six assets/ELF digest. Update current status and append acknowledgement. No physical FW12.00 result exists.

## M1-226 — Audit remediation and first-launch task intake — 2026-10-11

Main ffff5e08905d394b67a7da070fcf4657a250f070 unchanged; no open PRs. DEV workflow 38075764498 succeeded and GitHub confirms dev/v0.1.0-dev.1 prerelease with six assets. Published ELF SHA-256 c908a67cfbe3fb0596b94ef472bc578062ee0a0bb024e683d3242e5f78354aaa. New audit: eight P1/eight P2 confirmed static findings registered in docs/development/STATIC_AUDIT.md, critical code defects added to DEV gate. New owner task FIRST_LAUNCH_TASK.md authorizes automatic authenticated WebUI and dev.2 after all gates. Physical firmware/notification/USB/PS5 remains NOT_RUN. Implementation and new gates pending; no dev.2 published.

M1-227: automatic authenticated WebUI, local notification pairing and 16 static audit findings remediated. Normal, ASan/UBSan and SDK gates PASS on full 255-file source export; Chromium202 PASS on the matching portal, clean-export browser repeat PASS. DEV policy blocks newly discovered critical defects; no hardware or release claim before acknowledgement.
