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

DEV2 publication and actual downloaded checksum are verified (M1-229). Release dev/v0.1.0-dev.2 points to6ed81c2ba477285e932bd4593aa029554cadf757; workflow38141611911 PASS, six assets. Published ELF SHA256bab7c92d8b49b56356a8d616104fc99ef69c49084adb49a76b574e92cb07dfbf. The previous runner queue blocker is resolved. Read CURRENT_STATUS.md, TEST_LOG.md M1-229 and docs/TEST_PLAN.md before continuation. Remaining runtime acceptance requires measured FW13.60 (owner-reported upgrade2026-10-11) and explicitly authorized controlled physical tests; all hardware rows remain NOT_RUN. No firmware update, radio operation or stable release is authorized by DEV publication.

## M1-226 — Audit remediation and first-launch task intake — 2026-10-11

Main ffff5e08905d394b67a7da070fcf4657a250f070 unchanged; no open PRs. DEV workflow 38075764498 succeeded and GitHub confirms dev/v0.1.0-dev.1 prerelease with six assets. Published ELF SHA-256 c908a67cfbe3fb0596b94ef472bc578062ee0a0bb024e683d3242e5f78354aaa. New audit: eight P1/eight P2 confirmed static findings registered in docs/development/STATIC_AUDIT.md, critical code defects added to DEV gate. New owner task FIRST_LAUNCH_TASK.md authorizes automatic authenticated WebUI and dev.2 after all gates. Physical firmware/notification/USB/PS5 remains NOT_RUN. Implementation and new gates pending; no dev.2 published.

M1-227: automatic authenticated WebUI, local notification pairing and 16 static audit findings remediated. Normal, ASan/UBSan and SDK gates PASS on full 255-file source export; Chromium202 PASS on the matching portal, clean-export browser repeat PASS. DEV policy blocks newly discovered critical defects; no hardware or release claim before acknowledgement.

M1-229: exact source push gates and all repeated DEV gates PASS; prerelease dev/v0.1.0-dev.2 and six assets acknowledged, actual downloaded ELF/checksum verified. See CURRENT_STATUS.md for digest and remaining hardware gates.

M1-231: owner reports upgrading the console from12.00 to13.60 on2026-10-11; current target is13.60, unmeasured/unverified. Earlier12.00 records and dev.2 metadata remain historical. SDKv0.43 and safety policy unchanged. New DEV packaging currently pins12.00 and must be adapted with its regression before another release; see CURRENT_STATUS.md. No on-console action was taken.

## M1-232 — dev.3 implementation continuation

Current intended target13.60 (owner report, firmware_verified=false); SDKv0.43. Exactly one ELF, with optional authenticated Settings & help → PS5 tile installation and embedded owner-supplied PNG/metadata. ADR-054/F105/M1-232 document ownership, retry and failure contracts; H-13 is physical-only. Local normal/sanitizer40 C suites, Chromium214 and SDK build PASS; publish only after fresh complete-source gates and remote acknowledgement. Current next commands are in SESSION_LOG M1-232. Existing dev.2 tag/assets/history remain immutable.

M1-233 continuation: select the lazy-loading follow-up source, superseding initial0a3dacb for release. Single ELF no longer imports AppInstUtil at startup; readelf gate protects this. Fresh40 C/normal/sanitizer/Chromium214/SDK checks pass, DEV unit contracts now7. Current feature list/docs include FW13.60 and H-13 NOT_RUN. Remote runner allocation is the current external publication gate; no DEV3 tag/assets exist yet.
