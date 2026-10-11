# Current status — 2026-10-11

## Published dev.2

- [Experimental prerelease dev/v0.1.0-dev.2](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v0.1.0-dev.2) is published, prerelease=true/draft=false, six assets plus source archives. [DEV workflow38141611911](https://github.com/MNeroba/SteamController-PS5/actions/runs/38141611911) completed/success, all three jobs PASS. Evidence M1-229.
- Exact source/tag: `6ed81c2ba477285e932bd4593aa029554cadf757`. Verified source tree50f3f5dd7b26cbddd446de9f0d09498b6c6bb3b6:255 blobs/all41 production C units/all251 previous files retained/zero deletions. Later documentation acknowledgement commits do not change this implementation.
- Both exact-source push checks PASS: Host38125160695 and SDK38125160699. Repeated DEV normal/ASan-UBSan/Chromium202+/asset/build/checksum gates PASS with SDKv0.43/LLVM18. All16 confirmed static findings have scoped remediation; see STATIC_AUDIT.md, ADR-053, M1-227/228.
- Actual downloaded ELF1782328 bytes, ELF64/little-endian/x86_64. SHA256 `bab7c92d8b49b56356a8d616104fc99ef69c49084adb49a76b574e92cb07dfbf` matches downloaded checksum, release notes and GitHub asset metadata. This is the published Linux digest; earlier local build digest is historical.

## First launch

Missing policy starts authenticated WebUI8090. Pair with the eight-digit local notification code (five minutes/five attempts/one redemption); valid session restoration and advanced token recovery supported. Explicit disable and invalid-file fail-closed behavior preserved. Hooks/probe/DualSense handoff stay off. Notification delivery and actual portal remain physically unverified.

One active Steam Controller2026 session. PS5 FW13.60 is owner-reported/unverified (owner reports upgrading from12.00 on2026-10-11). BLE/Edge/custom ShellUI identity research only. No physical console operation performed.

## Remaining gates and continuation

The earlier external runner queue blocker is resolved; its negative evidence remains in logs. No DEV publication blocker remains for this source. Stable H-01/H-02/H-03/H-04/H-05/H-09/H-12 remain NOT_RUN. Real USB cancellation/watchdog timing, VDA ownership/neutralization, credentials, native hooks/unmap/rollback, motion/touch/haptics and native DualSense coexistence require hardware.

Next safely executable hardware stage: read docs/TEST_PLAN.md and HARDWARE_MATRIX.md, record measured firmware/read-only console baseline before an explicitly authorized controlled runtime test. Use only an independently confirmed compatible development loader; retain stock DualSense. Do not update firmware or operate the stock radio. No console access is available in this session.

## M1-231 — updated console target

Owner now reports FW13.60. This supersedes the earlier refusal to update; it is not independently measured firmware or a runtime compatibility result. Current hardware matrix targets13.60 with all rows still NOT_RUN. SDKv0.43, USB/Puck and safety defaults remain unchanged. Published dev.2/tag/assets/release notes retain their historical12.00 target and exact checksum. Before any new DEV release, adapt scripts/package_dev_release.py:13-20, tests/test_dev_release.py:30 and dev-release.json together (they currently pin12.00), and refresh current README/loading/BLE/test-plan text without relabeling historical results. No new release, deployment or system-resource write performed. Launcher M1-230 requires fresh FW13.60 hardware checks.

## dev.3 in progress — M1-232

Single ELF now embeds the supplied icon and optional Media settings tile installer. Pair, choose Settings & help → PS5 tile, confirm installation; tile opens loopback8090 while payload runs. Local40 C/normal/sanitizer/Chromium214/SDK build gates PASS. Current firmware policy13.60 owner-reported/unverified, SDKv0.43 unchanged. Fresh complete-source validation, remote push checks and dev.3 publication are next; no new release is claimed yet. H-13 tile registration/browser/navigation/removal remains NOT_RUN, alongside existing console gates.
