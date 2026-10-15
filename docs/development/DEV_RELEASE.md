# DEV prerelease procedure

DEV builds are experimental software/build deliverables. They do not satisfy the stable hardware gate or establish PS5 firmware compatibility. The owner explicitly authorized this separate channel on 2026-10-10. Target: owner-reported FW 13.60 (unmeasured); SDK v0.43.

## Mandatory changelog and approval

Before publication, update CHANGELOG.md relative to the previous published version, with exactly **Bug Fixes** and **New Features / Improvements** (write `None.` when empty). Match payload/version.h, metadata and selected workflow version; never rewrite historical releases/tags. Both DEV packaging and stable tagged release workflows fail closed on a missing/mismatched entry. Notes include those two sections, actual source SHA and development/hardware status.

**dev.3 was explicitly approved and published on 2026-10-12 from `9aab6dbae166b28416a9a00e6346f85bf12e524e` via workflow38225113776. Future candidates still require explicit owner approval before publication; a green push does not grant approval.**

## Manual publication

1. Review `ISSUES.md`, this task's full file audit, safety defaults and `dev-release.json`. A known unresolved critical **code** defect must be added to `critical_code_blockers`; the workflow then fails closed. Hardware-dependent risks stay explicit in the notes and matrix.
2. Publish a complete reviewed main tree using a non-force update. Wait for both Host tests and PS5 cross-build push runs on that exact commit to succeed.
3. Open **Actions → Experimental DEV prerelease → Run workflow**, use branch **main**, paste the full lowercase 40-character tested source SHA and a new version such as `v0.1.0-dev.4`.
4. The workflow requires main ancestry, the latest successful Host/PS5 push gates for that SHA and an unused `dev/` tag. It then checks out that exact revision, reruns normal host, ASan/UBSan and Chromium tests, verifies deterministic embedding, downloads checksum-pinned SDK v0.43, cleans/builds and packages the real ELF. Missing files, checksum mismatch or failed tests prevent publication.
5. Publication downloads the tested artifact and verifies its checksum again. It creates only a prerelease tagged `dev/v0.1.0-dev.4`, points the tag to the selected source SHA and leaves GitHub's latest stable release unchanged. Stable `v*` publication also requires normal/sanitizer/Chromium/assets/checksum gates and hardware evidence bound to the released source. Only a descendant with unchanged payload, scripts, tests and workflow inputs may reuse a hardware-tested SHA; documentation-only evidence commits are permitted.
6. Verify the remote tag commit, `prerelease: true`, source SHA, all seven assets, checksum and Actions conclusion; append the actual acknowledgement to TEST_LOG/SESSION_LOG/current status. Do not claim a download exists before GitHub acknowledges it.

## Assets and source

- `SteamController-PS5-v0.1.0-dev.4.elf` — real SDK output, copied from the validated build.
- Its `.elf.sha256` — digest with the download filename.
- Its `.elf.json` — Payload Manager metadata with matching filename/version/release URL and actual SHA-256. The source metadata URL identifies the selected dev.4 asset after publication; authorized access is needed while the repository remains private.
- `RELEASE_NOTES.md`, `LOADING.md`, `NOTICES.md`, `LICENSE` — warning, evidence, configuration and attribution.
- GitHub's matching source archives at the exact release tag. Full source, asset notices, SDK source pin and generator remain available for rebuilds.

No PKG/FPKG, firmware file, token or SDK archive is uploaded. Local ELF hashes may differ between toolchains or checkout paths due to debug/build metadata; reproducible means a documented fresh-source build with pinned SDK and deterministic embedded assets, not a claim of cross-platform bit-identical ELF output.

## Automation limits

The currently connected GitHub tools can publish Git objects and inspect Actions; they do not expose workflow dispatch or Release creation. An authenticated browser or GitHub CLI is needed for the one manual dispatch. No credentials should be embedded in project files. With an authenticated CLI, the equivalent is:

```sh
gh workflow run dev-release.yml --repo MNeroba/SteamController-PS5 --ref main \
  -f source_sha=<tested-full-commit-sha> -f version=v0.1.0-dev.4
```

Retain history: earlier SHA references are evidence, provenance and regression context. A cosmetic squash gives no build benefit. There is no proposed ref rewrite or force push.

## dev.2 first launch

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


## Next publication blocked — M1-244 (2026-10-15)

New external dev.4 logs demonstrate unresolved pending VDA ownership and token initialization failures. dev-release.json lists P0-VDA-PENDING-04 and packaging must reject future publication while it remains unresolved. Do not clear it based on compilation/host suites alone: justify the actual receipt/ownership contract and add meaningful regressions. Existing dev.4/tag/assets stay unchanged. See [external runtime report](EXTERNAL_RUNTIME_REPORT.md). No new release approval was given with the log link.


## dev.5 preparation only — M1-245

Owner explicitly withheld publication. Version/metadata/changelog and one-ELF SDK build are prepared for source review, with storage/logging regressions covered. P0-VDA-PENDING-04 remains open and real packaging rejects the candidate. Do not dispatch the manual DEV workflow or create dev/v0.1.0-dev.5. See [candidate status](DEV5_CANDIDATE.md); existing published assets/tags are unchanged.
