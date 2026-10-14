# Current status — 2026-10-14



## M1-243 — Published dev.4, 2026-10-14

Owner explicitly approved creation/publication. Tag dev/v0.1.0-dev.4 points to a5c20346392eac0eb9fa04cc201cd214e2957363; DEV workflow38588689667 completed/success and all three jobs passed. GitHub release412878689, prerelease=true/draft=false, published 2026-10-14T15:29:58Z; seven assets contain exactly one ELF (2029632 bytes, SHA25634ae9fa0d55f8fdadbec4384d4171b029243e2fc6d0eb94a3d0120a6165f9ea4). Selected source has287 blobs/45 production C; prior releases/tags/assets remain unchanged. Exact-source push Host38587348600/SDK38587348499 and repeated DEV normal44 C/Python, ASan/UBSan, Chromium220, deterministic assets, SDKv0.43 build/dependency/package/checksum gates passed. Earlier local browser fixture version mismatch and checksum command wrong-directory negatives retained; corrected reruns passed. Fresh local exact-source normal/sanitizer/build/package PASS; local ELFc236bf1aff8cd842eea092f06568f9b45a9d48fb5404028c9dd84e8414607650 is separate from published Linux output. Three M1-242 safety fixes included; full dev.3 functionality retained. Firmware13.60 unmeasured, physical hardware NOT_RUN, external dev.2/FW5.50 cause/token-storage/native-input problem unconfirmed. Repo visibility unchanged; no console/radio/firmware/tile action. Latest setup/download/changelog/security/status documents updated after publication acknowledgement. Next: review existing tester gc_status.log/.prev or authenticated diagnostic report when supplied; do not ask for secrets or rerun the failing old ELF. Tester reply prepared; no Reddit destination URL known and no external comment posted.

Downloaded dev.4 ELF independently verified:2029632 bytes, SHA25634ae9fa0d55f8fdadbec4384d4171b029243e2fc6d0eb94a3d0120a6165f9ea4 and selected SHA/version/tile embedded. Separate sidecar download declined; no workaround. GitHub notes/digest/build log and workflow checksum gates agree. Earlier checkpoints below are historical and superseded by this release receipt.

## dev.4 approved release preparation — M1-243

Owner explicitly requested create/publish dev.4 before updating the tester reply. Authoritative main310a1f60dccb8ac18cd97661e83cdcee37fbc274/tree9b07c06ce20cae265b91b4727070e8c8fb45a40d has287 files. Runtime c204524 Host38581146662/SDK38581146350 and receipt310a1f Host38581734371/SDK38581734544 are completed/success; this supersedes earlier queued observations. No dev.4 release/tag yet. Version/metadata/changelog and version-aware package tests updated; preserve all existing functionality, SDKv0.43 and /data/ghostpad. Local normal44 C/Python, ASan/UBSan, Chromium220 repeat (old version fixture failure retained), deterministic embedding, SDK45 C/dependency and release/link/diff checks PASS. Next: complete reviewed main commit, own green push gates, manual DEV dispatch, seven assets/one ELF/source/checksum verification, publication receipts and tester reply. No Public visibility change or console operation authorized here.

## Final runtime safety review — M1-242 (unreleased source)

Three confirmed post-dev.3 defects fixed: unattributable Add retry/global reservation gap, reopened USB identity before detach, invalid OUT status overflow. Full normal44 C/Python, ASan/UBSan, Chromium220, clean SDKv0.43/dependency and link114/changelog/DEV/diff gates PASS locally. Sourcec204524cbd14cc232c11289677e7bb6607e59a0e published with all287 blobs retained. Fresh exact export repeats normal/sanitizer/SDK/dependency PASS; five-module static analysis clean. Host38581146662/SDK38581146350 queued at14:32 UTC; remote success not claimed. No new binary/tag/version; published dev.3/earlier assets lack these fixes and README says so. See docs/development/PUBLIC_RUNTIME_REVIEW.md and CHANGELOG Unreleased.

Owner-supplied dev.2/FAT5.50 failure comment lacks logs/digest/error code and remains unverified; no cause or successful hardware correction claimed. Intended13.60 and all physical gates still NOT_RUN. Private source visibility unchanged. Next: verify focused main push Host/SDK CI; owner Public decision and separately approved new DEV binary. Keep stock DualSense, no console/radio/tile operation.

## Published dev.3 — M1-238

**PUBLISHED** experimental prerelease [dev/v0.1.0-dev.3](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v0.1.0-dev.3) from `9aab6dbae166b28416a9a00e6346f85bf12e524e`. Workflow38225113776 completed/success; all three jobs PASS. Dev.1/dev.2 releases are unchanged.

- Implementation/review source **9aab6dbae166b28416a9a00e6346f85bf12e524e** (documentation sourcee7f53f2cf1466bed0f8462bc350155b45b101eb9 has green Host38243078629 and SDK38243078701), tree1512f3fcb9e694d54e55a4d5c8afb982e2d0ca65;282 blobs/45 C,267 previous files retained. Baseline6dc554633b133c0dd8c865168010306e38e66384 was complete267 blobs; old f480e58 push gates both now PASS (queue resolved).
- Implemented portable user-list consensus and existing-worker owned VDA recovery, serialized Add/release ownership, IN/OUT completion retention and cancellation retirement,500ms watchdog, authenticated bounded/redacted diagnostic download, truthful limited connection notices, matching Payload Manager metadata and mandatory two-section changelogs.
- Existing single ELF/tile/artwork/pairing/profiles/gyro/touch/haptics preserved. Tile remains explicitly confirmed. SDKv0.43, /data/ghostpad, safe defaults retained. No automatic account binding/native DualSense mutation, storage migration, new generic controller support or radio operation added.
- Local normal/sanitizer44 C suites plus existing Python contracts/9 artwork/7 DEV/5 changelog checks; Chromium220; clean45-C SDKv0.43 and optional-dependency checks recorded in TEST_LOG. Fresh282-file export matches remote; repeat normal host + exact-SHA SDK build and seven-file/one-ELF local package PASS. ELF2028864 bytes, SHA256fc71b6acbf9ac857e3fe13259b37bdfecc0cb150a313db355222d3b4258188af at output/dev3-review (not a GitHub release download). Push SDK38223580419 PASS; Host38223580325 PASS.
- H-13 Settings tile, H-14 user recovery/native coexistence and H-15 actual USB cancellation/watchdog/diagnostic browser/notification behavior remain NOT_RUN/BLOCKED_HARDWARE. Positive Add receipt semantics and terminal USB close require physical confirmation. Stable gate still fails closed.

Published download:2029288-byte ELF; SHA256 `013e27f60091cd5de87577beb9acd40ac5ae5eeba08092b7dff65bf56847c752`, independently downloaded and verified against GitHub asset digest/release notes, exact SHA/version embedded. Seven assets/oneELF plus source archives; tag/source and prerelease flags verified. Next: follow hardware TEST_PLAN before an explicitly authorized console experiment; H13/H14/H15 remain NOT_RUN. No physical deployment, tile installation or radio operation performed.

## Public-facing preparation — M1-239

Documentation update published as e7f53f2cf1466bed0f8462bc350155b45b101eb9; Host38243078629 and SDK38243078701 completed/success. README and active build/feature/UI/specification guidance reconciled with published dev.3. Contributor issue forms and SECURITY.md added; all109 active relative links and targeted changelog/package/form checks pass. No runtime source/build/test/workflow change; historic results/releases retained.

Repository remains private. M1-240 follow-up covers both remote branches, all three tags,140 commits/trees, all1,413 text blobs and63 SHA-matching image blobs; targeted signatures found no credentials. PR#1 is closed with no comments/reviews; no project discussion attachments returned. Release bodies/inventories and matching downloaded dev.2/dev.3 ELFs checked. Full historical release-binary forensics, upstream linked media and independent secret-scanner run remain outside scope. Historical local paths contain the maintainer username; artwork publication permission is separate from an open asset license. No confirmed public-publication blocker identified. Follow [PUBLIC_READINESS](../docs/development/PUBLIC_READINESS.md), refresh About and verify desired private vulnerability reporting. No compatibility/stable claim or visibility change.

Reviewed mainaf183a2a0ed9b98e427864c216df9d68da854363 Host38243782521 and SDK38243782563 both PASS. Follow-up audit receipt55365f6164bff4475c4b038aee157833e26aa89c Host38284898787/SDK38284898822 also both PASS. M1-241 owner requested the metadata update: About now describes experimental USB/Puck, remapping/gyro/trackpads/haptics, one ELF and hardware-unverified status; UI/API verified. Private vulnerability reporting is unavailable while private per authenticated UI and GitHub's public-repository documentation; enable/verify after Public. Visibility remains private. Next: owner visibility decision after reading the scoped audit, then Security reporting enablement. No hardware operation.

---

## Historical status (previous checkpoint, retained)

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

## M1-235 — LibrePad v1.1.2 task intake, 2026-10-12

Owner requests integration in existing unpublished dev.3 and explicitly withholds release/tag approval. This supersedes earlier dispatch next actions: DO NOT DISPATCH or publish dev.3 until owner approval. Main6dc554633b133c0dd8c865168010306e38e66384 has267 blobs; all match local bytes. Original dev.3 sourcef480e58b953c75a87d7f1b929789ad023fc05fb1 Host38170245378/SDK38170245381 both completed/success; old runner queue is resolved. No open PR; only dev.1/dev.2 published, unchanged. Reference tagv1.1.2 annotatedbb60defad681563742448faaefa29b440ea83a0e resolves7466317b4b356249e07770f3914c995e1a4d633a. RootGPL3, no code copied yet. Release now warns of rejected symlink storage migration and recommends1.1.3; do not adopt migration. Logout recovery is upstream host evidence only. Preserve single ELF/tile, USB/Puck/security/defaults and /data/ghostpad. Next: audit pinned virtual/USB/diagnostics/metadata source, add regressions, implement smallest owned-worker recovery and bounded authenticated report, changelog/metadata gates, full local validation and reviewed main push; stop before DEV publication. No hardware available; all physical rows NOT_RUN.
