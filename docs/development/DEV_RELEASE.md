# DEV prerelease procedure

Publication is complete for 1.0.0 Experimental. The dev.1–dev.9 sections below are retained as historical records; their GitHub releases and tags were removed during the consolidation and their assets are archived outside the repository, so their download links are no longer live. The [implementation/hardware limits](DEV8_CANDIDATE.md) and [raw-report contract](RAW_HID_REPORT.md) remain applicable; future publications require separate owner approval.


DEV builds are experimental software/build deliverables. They do not satisfy the stable hardware gate or establish PS5 firmware compatibility. The owner explicitly authorized this separate channel on 2026-10-10. Target: owner-reported FW 13.60 (unmeasured); SDK v0.43.

## Mandatory changelog and approval

Before publication, update CHANGELOG.md relative to the previous published version, with exactly **Bug Fixes** and **New Features / Improvements** (write `None.` when empty). Match payload/version.h, metadata and selected workflow version; never rewrite historical releases/tags. Both DEV packaging and stable tagged release workflows fail closed on a missing/mismatched entry. Notes include those two sections, actual source SHA and development/hardware status.

## Published experimental 1.0.0 Experimental — M1-259 (2026-11-18)

Published [1.0.0 Experimental](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v1.0.0-experimental) from `3bad173a6593dcc94d47f1e7f7da4e1d7372496f` by [DEV workflow 44403927035](https://github.com/MNeroba/SteamController-PS5/actions/runs/44403927035) on 2026-11-18T21:43:48Z. Seven assets contain exactly one ELF, `SteamController-PS5-v1.0.0-experimental.elf`, 2,067,360 bytes, SHA-256 `79ab4f11176a76687a6e987ab1a8addd401618b04e1d5ca4fb7771901d986855`. The tag points to the selected source; downloaded assets, embedded source/version and sidecar checksum were independently verified. The workflow repeats normal host, ASan/UBSan, Chromium, deterministic-asset, SDK v0.43 and checksum gates.

Scope: cumulative dev.9 feature set, neutral-gravity release-report fix, loopback `Host`-port validation, bounded icon reads, `404` for unknown routes, removal of unused auth/peer modules and dead pairing routes, and corrected authentication documentation. Physical PS5/FW13.60 remains NOT_RUN; the stable gate is unchanged. The superseded .1 and .2 releases were archived and removed.

## Published experimental v1.0.0-experimental.2 — M1-258 (2026-11-18)

Published experimental [v1.0.0-experimental.2](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v1.0.0-experimental.2) from `3e75e03a3fcee80549838a26ce3675c9b2b8dff6` by [DEV workflow 44397526550](https://github.com/MNeroba/SteamController-PS5/actions/runs/44397526550) on 2026-11-18T21:14:36Z. All three jobs succeeded. Seven assets contain exactly one ELF, `SteamController-PS5-v1.0.0-experimental.2.elf`, 2,067,376 bytes, SHA-256 `523c6ac784250abf36c464b7edffeb5c13eed685249a40ad034ec9d8cc493326`. The tag points to the selected source; the downloaded ELF, sidecar checksum, embedded source/version and metadata were independently verified.

Scope: bind explicit `Host` ports to the actual listener port, reject truncated/oversized icons, return `404` for unknown routes, remove the unused auth/peer modules and dead pairing routes from the shipped payload, and correct stale authentication documentation. Exact-source push gates Host44397042663, SDK44397042952 and secret scan44397042725 succeeded; the local full-history scan reported zero findings. Normal and sanitizer host suites pass. Physical PS5/FW13.60 remains NOT_RUN and the stable gate is unchanged. The previous v1.0.0-experimental.1 release was archived and removed.

## Published experimental v1.0.0-experimental.1 — M1-257 (2026-11-18)

Published experimental [v1.0.0-experimental.1](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v1.0.0-experimental.1) from `884d8e9f77b32771f84adc00075670664b92bd1f` by [DEV workflow 44389398512](https://github.com/MNeroba/SteamController-PS5/actions/runs/44389398512) on 2026-11-18T20:20:18Z. All three jobs succeeded. Seven assets contain exactly one ELF, `SteamController-PS5-v1.0.0-experimental.1.elf`, 2,068,904 bytes, SHA-256 `2780abe6ad56d67216c2ed6b8e9ca824a7409a57fb21331d437ee7b7eea6c5d2`. The tag points to the selected source; the downloaded ELF, sidecar checksum, embedded source/version, metadata and experimental warning were independently verified.

The owner asked for a high-confidence-only defect pass and a consolidated v1.00 experimental publication. One code defect was fixed: the pause / DualSense-takeover release report in `payload/gc_main.c` injected zero acceleration instead of the documented neutral gravity `(0,1,0)`. Version, changelog, Payload Manager metadata, packaging tooling and the workflow version gate now accept the `v1.0.0-experimental.N` series; the stable `v*` hardware gate is unchanged.

The owner also approved consolidating all prior commits into this single publication commit and deleting the older releases and tags. The nine dev.1–dev.9 release asset sets were archived locally outside the repository before deletion; digests match the historical receipts. Historical receipts and negative evidence remain in the project-memory logs.

Exact-source push Host44388876807, SDK44388876981 and secret scan44388876943 succeeded on the squashed commit. Three commit-keyed gitleaks false positives were fixed at the source (synthetic test token and two prose matches), and a local full-history scan reported zero findings. Physical PS5 FW13.60, browser/Origin behavior, native DualSense coexistence, motion/haptics and tile registration remain NOT_RUN. The stable hardware gate is unchanged.

## Published experimental dev.9 — M1-256

Published experimental [v0.1.0-dev.9](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v0.1.0-dev.9) from `d6d51af1ce1d0cf14438fb73efae3cd2652b1d25` by [DEV workflow 43703160706](https://github.com/MNeroba/SteamController-PS5/actions/runs/43703160706) on 2026-11-15T10:18:02Z. Three release jobs succeeded. Seven assets contain one ELF, `SteamController-PS5-v0.1.0-dev.9.elf`, 2,068,880 bytes, SHA-256 `e4c1ad438f4f3861e846338528f4139f9e8e7dce3695839c96b31c53190cfc42`. The tag points to the selected source, every downloaded asset matched GitHub's digest, and the ELF embedded source, version, metadata and sidecar checksum were verified.

Owner selected no pairing code and PS5-only access, then requested automatic Media tile registration on first launch. WebUI binds IPv4 loopback only; other console processes can access it. Literal-loopback Host, same-host Origin, mandatory POST Origin, no-sniff and frame denial reduce browser-origin attacks. A joinable worker attempts registration only for an absent exact title path; it does not overwrite an existing or inaccessible path. An explicit local retry remains available. The tile does not start the payload. Physical PS5 FW13.60/browser/Origin, automatic installer privilege/visibility, registration recovery and shutdown remain NOT_RUN.

Exact-source push Host43702935960, SDK43702935974 and secret scan43702935930 succeeded. The release workflow repeated normal Host, ASan/UBSan, Chromium, deterministic assets, SDKv0.43/LLVM18, package/dependency and checksum gates. The earlier DEV workflow43702614078 was deliberately cancelled before creating a tag to correct misleading tile-status wording. Historical releases and the stable hardware gate remain unchanged.

## Published experimental dev.8 — M1-255

Published experimental [v0.1.0-dev.8](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v0.1.0-dev.8) from `1cf624f2ccaf869eecd8c29dd29e834529f8a3d9` by [DEV workflow 41688254764](https://github.com/MNeroba/SteamController-PS5/actions/runs/41688254764) on **3 November 2026, 17:54:11 UTC**. All three release jobs completed successfully. Seven assets contain exactly one ELF, `SteamController-PS5-v0.1.0-dev.8.elf`, 2,121,120 bytes, SHA-256 `7a37126cd223259f06deb9b420c74fb7b9aaee7d353495e5eb81b69e93be0d2a`. The remote tag points to the selected source; every downloaded asset matched GitHub's digest, and the ELF source/version, sidecar checksum and metadata were independently checked.

Exact-source push Host41687756285, SDK41687756162, full-history secret scan41687756071 and CI smoke41687756186 passed. The DEV workflow repeated normal53 C/11 Python, ASan/UBSan, Chromium220, image/embedding/deterministic-asset, clean SDKv0.43/LLVM18, dependency, metadata/changelog/package and checksum gates. This is VERIFIED_HOST_TEST/VERIFIED_PS5_BUILD, not physical acceptance. Native DualSense coexistence/account assignment, FW13.60 acceptance, actual game motion/touch/quaternion, USB suspend/reconnect, active-game rest/wake and tile behavior remain NOT_RUN. Direct HID remains opt-in and waits for one confirmed foreground user plus a readable disconnected native pad; no native disconnect/unbind/Open, Shell patch or radio operation is enabled. Historical tags/releases and the stable hardware gate remain unchanged.

## Published experimental dev.7 — M1-250

Published experimental [v0.1.0-dev.7](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v0.1.0-dev.7) from `f4ec9ae9e7e287c957a5602493a5b7d46449aed3` by [DEV workflow 41300890370](https://github.com/MNeroba/SteamController-PS5/actions/runs/41300890370) on 2026-11-01 11:15:23 UTC (2026-11-01 13:15:23 EET). Seven assets contain exactly one ELF, `SteamController-PS5-v0.1.0-dev.7.elf`, 2,091,960 bytes, SHA-256 `9bc6e1672bc8d20c8e9fcff85359c976523bdf6296a64e1e92dae34e6d7edb73`. PR8 was merged with contributor commits preserved; both descriptor P1s and the post-drain observation-window issue are resolved in code/host regressions. Exact PR, merged-main and repeated DEV Host/ASan/UBSan/Chromium220/asset/SDKv0.43/LLVM18/checksum gates passed. This is VERIFIED_HOST_TEST and VERIFIED_PS5_BUILD only. Contributor FW13.20/Puck observations at original c3a7ddb are EXTERNAL_HARDWARE; this revised ELF, owner target FW13.60, native DualSense coexistence, gameplay, motion/haptics, console cleanup and tile launch remain physically unverified. No historical result/tag/release was changed.

## Historical dev.7 preparation — M1-250 (2026-11-01)

The owner explicitly authorized correction of PR8, merge after successful checks and a new release. Select **v0.1.0-dev.7**, preserving dev.6 and all historical tags/assets. Contributor commits retain their authorship; the maintainer replaces the fd/inode error heuristic with explicit amd64 syscall error status and adds host/ABI/storage/backlog regressions. See [PR8 review and correction](../../project-memory/PR8_REVIEW.md). A standalone dev.N fits the existing version gate; do not relax it for a fix.1 suffix.

Local normal/sanitizer/Chromium220 and SDKv0.43/LLVM18 checks passed before versioning. The new selected exact source still needs green remote checks and the manual DEV workflow's repeated checks before publication. Contributor-reported FW13.20/Puck results apply only to original c3a7ddb; revised adapter, FW13.60, native DualSense coexistence, games and cleanup remain hardware-unverified. No new kernel/Shell/radio behavior or storage migration.

## Published experimental dev.6 — M1-249 (2026-11-01 Riga / 2026-10-31 UTC)

[dev/v0.1.0-dev.6](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v0.1.0-dev.6), source `4c8b89bc789bf1dfd064264ff432d3919b5a1899`, published as release 430212834 by [manual DEV workflow 41246199948](https://github.com/MNeroba/SteamController-PS5/actions/runs/41246199948). The workflow completed successfully in all three jobs and reran the normal host suite, ASan/UBSan, Chromium regression, nine image/embedding contracts, deterministic asset generation, clean SDK v0.43/LLVM 18 cross-build, packaging/notice checks and checksum verification. The release contains seven assets and exactly one ELF: `SteamController-PS5-v0.1.0-dev.6.elf`, 2,087,296 bytes, SHA-256 `ef52b279be433c1d445ec4a616e6294c647fb87555c3bca34fddddb68da7df9b`. The annotated tag points to the selected source SHA above.

The repository is Public. GitHub private vulnerability reporting, dependency graph and Dependabot alerts are enabled. This release remains experimental: no physical PS5/USB/Puck/VDA/native DualSense/storage test or firmware-compatibility claim is established.

**Published experimental dev.5:** [dev/v0.1.0-dev.5](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v0.1.0-dev.5), source `382e4ecef4937356b72f9e1786917ae7147c0415`. [DEV workflow38810046681](https://github.com/MNeroba/SteamController-PS5/actions/runs/38810046681) completed successfully; release414294352 is a prerelease, not a draft, published **2026-10-15 21:30:53 UTC** (16 October in Riga). Seven assets contain exactly one ELF, **2,084,968 bytes**, SHA-256 `01a73f3f676121061f58b52a833c642d8cd94b048fbbc524182b8756eb642bd0`. Host48 C/Python, ASan/UBSan, Chromium220, deterministic assets and clean SDKv0.43/LLVM18 gates passed on that selected source. Physical Add/VDI/MBus, native DualSense coexistence, firmware compatibility and actual storage errno remain unverified. Event correlation is an owner-approved temporary experimental mechanism, not proof of exclusive ownership. Previous releases/tags/assets are unchanged.

Future versions still require explicit owner approval. A green push does not authorize publication; never reuse an existing release tag.

## dev.6 preparation record — M1-249 (pre-publication checkpoint, 2026-10-31)

The owner requested preparation of v0.1.0-dev.6. Its scope is the M1-248 tester follow-up: protected token initialization without the redundant no-follow metadata probe, plus one bounded VDA log-reader retry for `/dev/klog/EILSEQ`. No VDA Add result, firmware compatibility or physical fix is claimed. The tester's shown logs came from the published dev.5 ELF and do not exercise the new source.

The version header, changelog, Payload Manager metadata, release workflow default and bundled loading guide are aligned to dev.6. Corrected candidate source `4c8b89bc789bf1dfd064264ff432d3919b5a1899` passed exact push gates: Host [41245762239](https://github.com/MNeroba/SteamController-PS5/actions/runs/41245762239), SDK [41245762158](https://github.com/MNeroba/SteamController-PS5/actions/runs/41245762158), and secret scan [41245762162](https://github.com/MNeroba/SteamController-PS5/actions/runs/41245762162), all success. At this checkpoint the release had not yet been published; the subsequent owner-approved publication is recorded above.

## Manual publication

1. Review `ISSUES.md`, this task's full file audit, safety defaults and `dev-release.json`. A known unresolved critical **code** defect must be added to `critical_code_blockers`; the workflow then fails closed. Hardware-dependent risks stay explicit in the notes and matrix.
2. Publish a complete reviewed main tree using a non-force update. Wait for both Host tests and PS5 cross-build push runs on that exact commit to succeed.
3. Open **Actions → Experimental DEV prerelease → Run workflow**, use branch **main**, paste the full lowercase 40-character tested source SHA and a new, unused version matching the source manifest (substitute for `v0.1.0-dev.N`).
4. The workflow requires main ancestry, the latest successful Host/PS5 push gates for that SHA and an unused `dev/` tag. It then checks out that exact revision, reruns normal host, ASan/UBSan and Chromium tests, verifies deterministic embedding, downloads checksum-pinned SDK v0.43, cleans/builds and packages the real ELF. Missing files, checksum mismatch or failed tests prevent publication.
5. Publication downloads the tested artifact and verifies its checksum again. It creates only a prerelease tagged `dev/<selected-version>`, points the tag to the selected source SHA and leaves GitHub's latest stable release unchanged. Stable `v*` publication also requires normal/sanitizer/Chromium/assets/checksum gates and hardware evidence bound to the released source. Only a descendant with unchanged payload, scripts, tests and workflow inputs may reuse a hardware-tested SHA; documentation-only evidence commits are permitted.
6. Verify the remote tag commit, `prerelease: true`, source SHA, all seven assets, checksum and Actions conclusion; append the actual acknowledgement to TEST_LOG/SESSION_LOG/current status. Do not claim a download exists before GitHub acknowledges it.

## Published dev.5 assets and source

- `SteamController-PS5-v0.1.0-dev.5.elf` — real SDK output, copied from the validated build.
- Its `.elf.sha256` — digest with the download filename.
- Its `.elf.json` — Payload Manager metadata with matching filename/version/release URL and actual SHA-256. The source metadata URL identifies the selected dev.5 asset after publication; authorized access is needed while the repository remains private.
- `RELEASE_NOTES.md`, `LOADING.md`, `NOTICES.md`, `LICENSE` — warning, evidence, configuration and attribution.
- GitHub's matching source archives at the exact release tag. Full source, asset notices, SDK source pin and generator remain available for rebuilds.

No PKG/FPKG, firmware file, token or SDK archive is uploaded. Local ELF hashes may differ between toolchains or checkout paths due to debug/build metadata; reproducible means a documented fresh-source build with pinned SDK and deterministic embedded assets, not a claim of cross-platform bit-identical ELF output.

## Automation limits

The currently connected GitHub tools can publish Git objects and inspect Actions; they do not expose workflow dispatch or Release creation. An authenticated browser or GitHub CLI is needed for the one manual dispatch. No credentials should be embedded in project files. With an authenticated CLI, the equivalent is:

```sh
gh workflow run dev-release.yml --repo MNeroba/SteamController-PS5 --ref main \
  -f source_sha=<tested-full-commit-sha> -f version='v0.1.0-dev.N'
```

Retain history: earlier SHA references are evidence, provenance and regression context. A cosmetic squash gives no build benefit. There is no proposed ref rewrite or force push.

## Release history — dated records

The entries below preserve earlier approvals, holds and results. The published dev.5 receipt above supersedes earlier publication blocks; hardware gates remain open.

### dev.2 first launch

Missing policy enables only WebUI on port 8090. Pair using the short-lived console notification code; no FTP is needed. Unsafe or malformed configuration fails closed; hooks, bridge probe and DualSense handoff remain opt-in. The audit remediation register is [STATIC_AUDIT.md](STATIC_AUDIT.md). No PS5 runtime result is inferred from this prerelease.

## dev.3 optional tile

Still one ELF/seven assets. The PNG and Media/deeplink metadata are embedded in that ELF; generation/checksum gates include both headers. Authenticated explicit confirmation is required before app writes. On-console registration and browser behavior remain H-13 NOT_RUN; it does not alter stable firmware/evidence gates. Historical dev.2 notes/assets retain their FW12.00 target.


## Metadata and diagnostic support

Metadata uses the documented Payload Manager fields at itsPLK/ps5-payload-manager e4e72dc9d831dd09ee8fabc222ad1e7f5139050e CUSTOM_REPOSITORIES.md. The sidecar is not a repository catalog; a catalog wrapper can be added separately by the owner after approval. It adds no second ELF or installer binary. Packaging rejects a nonempty output directory and mismatched ELF embedded source/version.

Settings & help → Diagnostics → Download Diagnostic Report requires browser enrollment and existing protected API admission. Reports contain actual firmware (or unavailable), build source, startup/USB/virtual/Steam snapshots and the latest48 filtered memory log records. Review profile names/USB paths before public sharing. No unrelated files, credentials, serials, bonding keys or raw system logs are read. Physical console/browser download support remains H-15 NOT_RUN.

## dev.4 safety update

Owner explicitly approved creation and publication on 2026-10-14. This candidate includes the three host-verified fixes in PUBLIC_RUNTIME_REVIEW.md; preserve the previous release assets/tags. Select the full new versioned main SHA only after its own successful Host/PS5 push checks. Existing dev.3 functionality and safety defaults are retained. No physical FW5.50 or FW13.60 result is inferred; the external dev.2 token-storage/native-input report remains unresolved.


## M1-243 — Published dev.4, 2026-10-14

Owner explicitly approved creation/publication. Tag dev/v0.1.0-dev.4 points to a5c20346392eac0eb9fa04cc201cd214e2957363; DEV workflow38588689667 completed/success and all three jobs passed. GitHub release412878689, prerelease=true/draft=false, published 2026-10-14T15:29:58Z; seven assets contain exactly one ELF (2029632 bytes, SHA25634ae9fa0d55f8fdadbec4384d4171b029243e2fc6d0eb94a3d0120a6165f9ea4). Selected source has287 blobs/45 production C; prior releases/tags/assets remain unchanged. Exact-source push Host38587348600/SDK38587348499 and repeated DEV normal44 C/Python, ASan/UBSan, Chromium220, deterministic assets, SDKv0.43 build/dependency/package/checksum gates passed. Earlier local browser fixture version mismatch and checksum command wrong-directory negatives retained; corrected reruns passed. Fresh local exact-source normal/sanitizer/build/package PASS; local ELFc236bf1aff8cd842eea092f06568f9b45a9d48fb5404028c9dd84e8414607650 is separate from published Linux output. Three M1-242 safety fixes included; full dev.3 functionality retained. Firmware13.60 unmeasured, physical hardware NOT_RUN, external dev.2/FW5.50 cause/token-storage/native-input problem unconfirmed. Repo visibility unchanged; no console/radio/firmware/tile action. Latest setup/download/changelog/security/status documents updated after publication acknowledgement. Next: review existing tester gc_status.log/.prev or authenticated diagnostic report when supplied; do not ask for secrets or rerun the failing old ELF. Tester reply prepared; no Reddit destination URL known and no external comment posted.


## Historical publication block — M1-244 (2026-10-15)

New external dev.4 logs demonstrate unresolved pending VDA ownership and token initialization failures. dev-release.json lists P0-VDA-PENDING-04 and packaging must reject future publication while it remains unresolved. Do not clear it based on compilation/host suites alone: justify the actual receipt/ownership contract and add meaningful regressions. Existing dev.4/tag/assets stay unchanged. See [external runtime report](EXTERNAL_RUNTIME_REPORT.md). No new release approval was given with the log link.


## Historical dev.5 preparation hold — M1-245

Owner explicitly withheld publication. Version/metadata/changelog and one-ELF SDK build are prepared for source review, with storage/logging regressions covered. P0-VDA-PENDING-04 remains open and real packaging rejects the candidate. Do not dispatch the manual DEV workflow or create dev/v0.1.0-dev.5. See [candidate status](DEV5_CANDIDATE.md); existing published assets/tags are unchanged.


## M1-246 — dev.5 owner approval (2026-10-16)

Owner explicitly authorized the analogous event-correlation mechanism for this stage, then requested publishing dev.5 for the log author's test. This supersedes M1-245's publication hold. No actual Add/VDI/MBus ABI, firmware compatibility, native coexistence or storage cause is claimed. The missing pending software transition is implemented with regression coverage; correlation limitations are accepted for this experimental DEV channel and stated in release notes. Clear the critical code blocker only after final source review and complete local/exact-source CI gates; retain the stable hardware gate and owner-approved manual workflow, versiondev.5, SDKv0.43, one ELF/seven assets and downloaded-checksum verification. No old tag/release/asset is overwritten.


## Actions maintenance verification

Before merging workflow dependency changes, run [CI Actions smoke](CI_ACTIONS.md) together with normal Host/SDK checks. The smoke workflow exercises the same github-script API families and multi-file upload/download Actions without creating a release, tag or ELF. Its success does not replace the full manual DEV gates or publication approval. Multi-file uploads retain archive:true; artifact digest mismatches fail closed.
