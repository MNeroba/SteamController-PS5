# Current status — 2026-10-31

## M1-249 — v0.1.0-dev.6 candidate preparation (2026-10-31)

The owner requested dev.6 preparation. Latest published release remains dev.5; no dev.6 tag, ELF or release exists. The tester's latest logs still came from published dev.5: USB-C/Puck input works, WebUI storage reports `file-inspect/EINVAL`, and VDA log setup reports `EILSEQ` before AddDevice. No exact tested ELF SHA or measured tester firmware was established. These logs do not exercise the source changes now on main.

The candidate contains the M1-248 fixes: determine an absent protected token from `openat(O_NOFOLLOW) == ENOENT`, with exclusive/no-follow creation and existing validation preserved; retry the bounded VDA observer once through a fresh loopback stream only when the selected `/dev/klog` reader returns `EILSEQ`. Add remains blocked unless the new stream drains to `EAGAIN`. No Add ABI, event ownership, account binding, firmware or native DualSense policy is changed.

Dev.6 release metadata is being aligned: version header, two-section changelog, Payload Manager sidecar, manual workflow default and packaged loading guide. Source/Host/SDK CI on the dev.6 commit is pending. The immediately preceding main source `e03f2c4` passed Host/SDK/secret scan; its docs descendant `4bd783c` passed Host/SDK. Local sandbox could not bind loopback, and the inherited desktop checkout is not a safe publication tree. No PS5 hardware claim. Do not dispatch the DEV workflow until the exact dev.6 source gates pass and separate publication approval is given.

## M1-248 — external dev.5 failure follow-up; source committed, release unpublished

The tester updated the shared Gist with logs from dev.5. Sanitized facts: Steam Controller via USB-C/Puck is detected and reports input; WebUI authentication setup fails at `file-inspect / EINVAL`; VDA event-log setup fails with `EILSEQ` before AddDevice, so this run says nothing about the VDA Add result. No user IDs, tokens, pairing codes or raw system log text are retained here. The run's measured firmware and exact ELF SHA are not established by the inspected excerpt.

The reviewed source follow-up was pushed directly to current `main` as `e03f2c428e98c8e76334539f33e1f336d3b6ae49`. It is not included in published dev.5 and does not modify its tag or assets:

- `payload/webui_auth.c` now recognizes an absent token from `openat(..., O_NOFOLLOW) == ENOENT`, avoiding the subsequent `fstatat(..., AT_SYMLINK_NOFOLLOW)` that plausibly produced `EINVAL` on the tester's filesystem. Create remains exclusive/no-follow; a file that appears/disappears in the race fails safely. The filesystem hypothesis is not hardware-confirmed.
- `payload/vda_pending.c`/`payload/gc_main.c` allow one loopback-reader retry only when `/dev/klog` is the selected source and its bounded initial read returns `EILSEQ`. The loopback stream must independently drain to `EAGAIN`; otherwise Add is not called. Other failures remain fail-closed, and source/error logs are more specific. The tester's old log did not identify its reader source.
- Regression-first storage test failed on injected `fstatat` `EINVAL`; after the fix, storage tests pass. VDA test injects `EILSEQ`, verifies the reader is discarded and the bounded alternate stream is used; unrelated errors do not retry, and a fallback-open failure without errno is normalized to `EIO`.

Validation: normal host C/Python tests pass except `test_webui_server`, whose loopback `bind()` is denied with `EPERM` in this sandbox. All other host binaries passed in the targeted sweep. ASan/UBSan sweep also passed for every runnable binary except the same listener test. Browser preview cannot bind localhost (`EPERM`); PS5 SDK v0.43 is not configured/found, so cross-build is NOT_RUN. Physical PS5 validation is NOT_RUN. The published dev.5 source/tag/assets remain unchanged.

Checkout constraint: local Git is `fix/p0-safety` based on `7846011` with a large inherited mixed/dirty tree, not an authoritative current-main snapshot. Do not commit or publish the whole checkout. Before any replacement release, transplant only reviewed files onto a freshly verified complete `main` tree, run green exact-source CI/SDK/browser gates, and get separate owner approval.

Source fix `e03f2c428e98c8e76334539f33e1f336d3b6ae49` is an ancestor of current main `4bd783c1b8dc1082942b85a7fcee72a0e0c45632`. Exact-source Host [41245027045](https://github.com/MNeroba/SteamController-PS5/actions/runs/41245027045), PS5 SDK [41245027061](https://github.com/MNeroba/SteamController-PS5/actions/runs/41245027061), and secret scan [41245027077](https://github.com/MNeroba/SteamController-PS5/actions/runs/41245027077) passed; the docs descendant also passed Host [41245148448](https://github.com/MNeroba/SteamController-PS5/actions/runs/41245148448) and SDK [41245148430](https://github.com/MNeroba/SteamController-PS5/actions/runs/41245148430). The versioned dev.6 commit still needs its own exact-SHA Host/SDK checks. The published dev.5 ELF/tag/assets remain unchanged. Next: complete the versioned dev.6 source commit, wait for its exact-SHA Host/SDK runs, then request separate publication approval. A tester must receive the actual release ELF and SHA-256 only after publication. Never describe this workaround as PS5-proven. Never describe this workaround as PS5-proven.

---

# Historical current status — 2026-10-16

## M1-247 — published dev.5 and documentation reconciliation

**Published experimental dev.5:** [dev/v0.1.0-dev.5](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v0.1.0-dev.5), source `382e4ecef4937356b72f9e1786917ae7147c0415`. [DEV workflow38810046681](https://github.com/MNeroba/SteamController-PS5/actions/runs/38810046681) completed successfully; release414294352 is a prerelease, not a draft, published **2026-10-15 21:30:53 UTC** (16 October in Riga). Seven assets contain exactly one ELF, **2,084,968 bytes**, SHA-256 `01a73f3f676121061f58b52a833c642d8cd94b048fbbc524182b8756eb642bd0`. Host48 C/Python, ASan/UBSan, Chromium220, deterministic assets and clean SDKv0.43/LLVM18 gates passed on that selected source. Physical Add/VDI/MBus, native DualSense coexistence, firmware compatibility and actual storage errno remain unverified. Event correlation is an owner-approved temporary experimental mechanism, not proof of exclusive ownership. Previous releases/tags/assets are unchanged.

Documentation-only main commit `5f91c44f87d6615dba625d28d063ba342ce37bc1` is published after the source release. Its exact-source PS5 cross-build [38913781983](https://github.com/MNeroba/SteamController-PS5/actions/runs/38913781983) and Host suite [38913781945](https://github.com/MNeroba/SteamController-PS5/actions/runs/38913781945) both completed successfully. Documentation checks are summarized in TEST_LOG/SESSION_LOG.

Current user/build/tile instructions now target dev.5. Persistent and session-only enrollment are distinguished; a redeemed code cannot enroll a second tile browser. Do not repeatedly reload an unresolved VDA to get another code. No code/assets/workflows/release bytes or repository visibility are changed by this documentation pass.

Next: obtain the tester's sanitized dev.5 diagnostic report with exact ELF SHA-256, measured firmware/loader, VDA status, storage stage/errno, native DualSense behavior and USB-C/Puck result. Follow docs/development/VDA_PENDING.md; no physical test is claimed. Future version publication requires new approval.

## Historical checkpoints — retained evidence

Earlier candidate holds and queued CI statements below are superseded by the receipt above.

## M1-246 — current dev.5 execution, 2026-10-16

Owner explicitly authorizes the analogous event-correlation approach temporarily and publishing dev.5 for the log author's controlled test. Baseline main6e1e82fefe3b9ae4408fb5d4540cc485a1ca4b03 has294 files; Host38805657403/SDK38805657360 succeeded, no open PR. Complete-tree source publication and manual DEV workflow must follow final validation, with no historical asset changes.

Implemented: one asynchronous Add/reservation, bounded fresh typed virtual-event reader, receipt provenance, cancelled/retiring cleanup and global failed-cleanup barrier; native exclusion and Steam/PS-only guarded local MBus/picker fallback, queued150ms PS action; protected RAM-only auth after operational storage failure; expanded secret-free stage/errno/count diagnostics and first-IN zero initialization. Existing USB/Steam/profile/tile security and SDKv0.43 retained. See docs/development/VDA_PENDING.md and ADR061/062.

Priority status: P0 actual Add/VDI/MBus contract and native coexistence BLOCKED_HARDWARE; operational correlation implemented under explicit owner acceptance, not proven exclusive ownership. P1 exact tester storage errno still unknown; reporting/session-only fallback host-verified. P1 USB shutdown/reconnect host fault suites pass, physical ioctl/cleanup timing NOT_RUN. No PS5/radio/firmware/tile operation performed. No release acknowledgement yet; local/exact-source final checks and release receipt follow in TEST_LOG/SESSION_LOG.





M1-245 exact-source CI ACK: 05587f209d8171d0e9492ce87bb6224ece3c8b63 Host38805088725/job116663752470 and SDK38805088852 completed/success. All normal46 C/Python, sanitizer and Chromium220 steps pass remotely. Exact local export repeats normal/sanitizer/SDK build; selected SHA/version and exact tile PNG/param embedded. Local candidate2047056 bytes/SHA25672e8bc941809ece3f7a693e4a3ea2a39c5a82d47951b4122ce4e09e910dd9841, not a published Linux release. Exact host logbc16b88f536629ccef3439906016d62295fc29d14fc86e218f54bb3e0b5313c2; sanitizecb6f4c0edd51c05cf09ff5df46367cbeda4704ea553024355c57c49d470f8359; buildda9a848417ca12011b6b324ff9add71a16f30ddaa1128f2dec530303cddf5bae. GitHub releases remain dev.1–dev.4 and matching tag enumeration contains no dev.5. No manual dispatch/publication/physical action. Gate remains blocked for VDA ownership; actual PS5 storage cause unverified. Next: source-proven receipt/assignment ABI or separately approved controlled measured hardware capture, then repeat full gates before asking for release approval.

## dev.5 candidate — M1-245, 2026-10-15

Owner requested fixes and preparation WITHOUT publication. Source baseline910f90371971fba08b2b1f375c7c61144617dfdf/tree d96fe30f296039025d4c82d16d8222dbc824bbde has288 files; all production bytes matched. Storage diagnostics and redundant chmod regression fixed; known account fields filtered before raw sinks, Safe Mode klog observer disabled, optional bridge text accounts redacted. Version/metadata/changelog/workflow defaultdev.5. Normal46 C/Python, full ASan/UBSan, Chromium220, SDKv0.43/45 C/dependency/determinism and targeted static analysis pass locally. Real DEV packager correctly rejects the unchanged P0 critical blocker before assets.

P0-VDA-PENDING-04 remains BLOCKED: classification/changed-word mask and containment implemented, not a functioning pending registration path. No source-proven safe receipt/assignment ABI found in pinned analogs; AGENTS forbids guessed ownership/unknown side effects. Existing positive-return ownership/global reservation/lifetime latch and native isolation retained. No PS5/hardware/firmware/radio/tile operation, no dev.5 publication approval. Next: reviewed full-tree source commit/push, exact-source build identity/CI and local candidate checksums; then proven VDA receipt contract or separately approved controlled hardware evidence. Do not dispatch publication or clear the critical gate on compilation alone. See docs/development/DEV5_CANDIDATE.md.

## External dev.4 logs — M1-244, 2026-10-15

Main remains aa9d4fa881b13c101edd1965baac7cbeb8b79655 at intake; Host38590488943/SDK38590488466 completed/success, no open PR. Published dev.4/sourcea5c2034 is unchanged. Complete public Gist snapshot reviewed: new log7572 bytes, previous1990922 bytes/36188 lines. EXTERNAL_HARDWARE: Puck input/battery/buttons/gyro decoded; one Add returns803b0006 and latch inhibits creation, no owned VDA/input injection. Older unidentified Ghost-Control v5 log has11975 Adds (14 pending-style/11961 subsequent803b0003) and14 VDA log identifiers. Temporal events do not prove ownership; device exhaustion/native UI cause remains HYPOTHESIS. Token initialization fails before listener; exact operation/errno unavailable. Default profile also rejected; no confirmed storage cause. Raw klog samples expose account IDs even with hand-off disabled; no identifier values/log bytes committed.

Future DEV gate blocked by P0-VDA-PENDING-04; P1-STORAGE-DIAG-04 and P2-KLOG-PRIVACY-04 recorded. No guessed handles, new ABI calls, MBus writes, storage deletion/permission relaxation, physical test or new release. Next: establish proven pending Add receipt contract, add secret-free per-stage storage diagnostics/fault tests and raw-klog gating/redaction; request already-used ELF SHA and confirmed FW/loader only. Do not ask for another uncontrolled failing run. Detailed evidence/hashes: docs/development/EXTERNAL_RUNTIME_REPORT.md. Existing FW13.60 hardware rows remain NOT_RUN; prior FW5.50 report is not a measured current run.

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
