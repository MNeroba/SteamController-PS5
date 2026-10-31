# DEV prerelease procedure

DEV builds are experimental software/build deliverables. They do not satisfy the stable hardware gate or establish PS5 firmware compatibility. The owner explicitly authorized this separate channel on 2026-10-10. Target: owner-reported FW 13.60 (unmeasured); SDK v0.43.

## Mandatory changelog and approval

Before publication, update CHANGELOG.md relative to the previous published version, with exactly **Bug Fixes** and **New Features / Improvements** (write `None.` when empty). Match payload/version.h, metadata and selected workflow version; never rewrite historical releases/tags. Both DEV packaging and stable tagged release workflows fail closed on a missing/mismatched entry. Notes include those two sections, actual source SHA and development/hardware status.

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
