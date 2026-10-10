# Project memory — current continuation entry

Read [CURRENT_STATUS](CURRENT_STATUS.md) first, then the append-only evidence below. Historical pre-consolidation summary is preserved verbatim at [PROJECT_MEMORY-before-dev](../docs/archive/PROJECT_MEMORY-before-dev.md); it contains earlier/intermediate statements, not current publication status.

## Identity and foundation

SteamController-PS5, private repository MNeroba/SteamController-PS5, main. Owner uses direct non-force main updates while developing solo; external contributors follow CONTRIBUTING/PR rules.

M0 is complete: PuckbridgePS5 `78460113ef7c48502c765a1f83ba34d53c977134` selected from source comparison; OmniPad `b7682efce18ee0a8d7c80bbc90afbce72151ef96` reference-only, no imported code. PoorDS4/Ghostcontrol/SDK/asset notices retained in PROVENANCE. Payload code is GPL-3.0-or-later; photos have separate owner publication permission.

## Evidence and safety

Host hardening and reusable protocol/USB/slot/profile/motion/touch/feedback/UI components are implemented and covered. Main runtime commit79f92cb has green Host/Chromium and SDK Actions (M1-221). SDK v0.43 archive SHA-256 a9cc9929f21b2b2c5d5b309f3bab4997067c45281c0622cf4838b1aecba66fcb; LLVM18. Physical PS5, controller/Puck, Prospero ioctls/ABI and FW12.00 behavior have never been tested here.

Safe defaults disable bridge probe, game hooks, DualSense handoff and WebUI. Restricted USB policy, owned devices, credential rollback and coordinated cleanup have host tests. Stock DualSense must remain usable; no built-in Bluetooth/HCI or active firmware/system experiment without separate physical evidence/approval.

The owner explicitly authorized an experimental DEV prerelease on2026-10-10. Follow DEV_RELEASE.md and separate manual dev/ workflow with exact source/full software gates and warnings. Stable v* checker remains hardware-gated. DEV authorization does not authorize deployment or claim compatibility.

## Publication and workspace

Desktop checkout: old fix/p0-safety at upstream78460113 with inherited dirty/untracked overlay. GitHub main/full tree is authoritative. All234 local baseline files matched main79f92cb before cleanup. Never reset/stage/push the whole desktop overlay. Publish reviewed allowlisted blobs on complete parent tree, verify all retained hashes and intentional moves/deletion, update expected parent force=false, read back source/CI.

GitHub connector works for Git objects and read-only Actions, not manual workflow dispatch. Edge permission was rejected; in-app GitHub browser needs login. Do not harvest credentials. Keep this access blocker distinct from hardware gates.

## Next safe action

Finish consolidation validation from an explicit fresh source export without generated header, publish reviewed source/docs/workflows on main, verify Actions, manually dispatch the tested SHA for experimental prerelease if authentication becomes available. Update CURRENT_STATUS/TEST_LOG/SESSION_LOG with actual acknowledgement and checksum. Preserve history; no rewrite is proposed.
