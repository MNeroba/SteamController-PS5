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

## M1-233 — release candidate refinement

Initial tile source0a3dacb is superseded by lazy installer loading: missing AppInstUtil/API cannot impose a new controller-payload startup dependency. Full fresh normal40 C, sanitizer, Chromium214, SDK build and ELF dependency/package checks PASS; DEV contract suite now7. Feature/status/navigation updated. Select only the follow-up full source SHA for dev.3 after its own Host/PS5 push checks pass. Previous queued jobs had runner_id0/no steps for over10min. No dev.3 release/tag/assets are claimed yet. H-13 remains NOT_RUN.

## M1-234 — final permissions check

Final new-directory umask077 regression fixed and verified without changing existing directories. Normal/sanitizer40 C/Python7, SDK/dependency checks PASS; Chromium214 remains valid for unchanged browser/UI source. Use the final permission-fix main SHA (not0a3dacb or b214da9) for dev.3 after its own green push gates. One ELF remains experimental; on-console FW13.60 icon/registration/browser use is NOT_RUN. GitHub's previous jobs remained queued/unassigned; no dev.3 release or download is claimed yet.

## M1-234 final publication checkpoint — 2026-10-11 20:05 UTC

Implementation source ACK: f480e58b953c75a87d7f1b929789ad023fc05fb1; tree 91184e1a2c60c599356a8752a42433473fc47896; all267 blobs/43 productionC retained, no deletions/unscoped edits. Exact fresh-source export verified. Final local one-ELF digest14447b40d07a1ce9713a808bb2f42d0307a9d1dd8cd318e628b93a111da3755e; workspace payload/ghost-control-ps5.elf and its checksum now match. Local6-file package /tmp/sc-dev3-reviewed-assets has exactly one ELF, correct source SHA, embedded PNG/metadata and matching checksum. Normal/sanitizer40 C +7 DEV contracts, Chromium214 and SDK/dependency gates PASS; on-console FW13.60/H-13 NOT_RUN.

External release blocker: exact-source Host38170245378/job114561164646 and SDK38170245381/job114561164692 still queued, runner_id0, steps0 at20:05:58 UTC. Earlier candidate jobs had the same unassigned state for over16min; obsolete0a3dacb cancellations requested. No compiler failure, global incident or billing cause is proven. dev.3 tag/release/assets NOT_PUBLISHED; do not represent local packaging as a GitHub download. Leave current candidate checks queued; do not bypass the gate.

Next exact action: query latest push runs for f480e58b953c75a87d7f1b929789ad023fc05fb1; only after both succeed, dispatch dev-release.yml on main with source_sha=f480e58b953c75a87d7f1b929789ad023fc05fb1, version=v0.1.0-dev.3 (authenticated browser form is prepared). Then wait repeated gates, verify tag points to selected source, prerelease=true, six assets/one ELF, actual downloaded SHA256 and source; append acknowledgement. Existing dev.2/tag/assets and historical FW12.00 records unchanged. No physical deployment, app registration or radio/firmware operation performed.
