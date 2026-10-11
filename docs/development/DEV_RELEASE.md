# DEV prerelease procedure

DEV builds are experimental software/build deliverables. They do not satisfy the stable hardware gate or establish PS5 firmware compatibility. The owner explicitly authorized this separate channel on 2026-10-10. Target: owner-reported FW 13.60 (unmeasured); SDK v0.43.

## Manual publication

1. Review `ISSUES.md`, this task's full file audit, safety defaults and `dev-release.json`. A known unresolved critical **code** defect must be added to `critical_code_blockers`; the workflow then fails closed. Hardware-dependent risks stay explicit in the notes and matrix.
2. Publish a complete reviewed main tree using a non-force update. Wait for both Host tests and PS5 cross-build push runs on that exact commit to succeed.
3. Open **Actions → Experimental DEV prerelease → Run workflow**, use branch **main**, paste the full lowercase 40-character tested source SHA and a new version such as `v0.1.0-dev.3`.
4. The workflow requires main ancestry, the latest successful Host/PS5 push gates for that SHA and an unused `dev/` tag. It then checks out that exact revision, reruns normal host, ASan/UBSan and Chromium tests, verifies deterministic embedding, downloads checksum-pinned SDK v0.43, cleans/builds and packages the real ELF. Missing files, checksum mismatch or failed tests prevent publication.
5. Publication downloads the tested artifact and verifies its checksum again. It creates only a prerelease tagged `dev/v0.1.0-dev.3`, points the tag to the selected source SHA and leaves GitHub's latest stable release unchanged. Stable `v*` publication also requires normal/sanitizer/Chromium/assets/checksum gates and hardware evidence bound to the released source. Only a descendant with unchanged payload, scripts, tests and workflow inputs may reuse a hardware-tested SHA; documentation-only evidence commits are permitted.
6. Verify the remote tag commit, `prerelease: true`, source SHA, all six assets, checksum and Actions conclusion; append the actual acknowledgement to TEST_LOG/SESSION_LOG/current status. Do not claim a download exists before GitHub acknowledges it.

## Assets and source

- `SteamController-PS5-v0.1.0-dev.3.elf` — real SDK output, copied from the validated build.
- Its `.elf.sha256` — digest with the download filename.
- `RELEASE_NOTES.md`, `LOADING.md`, `NOTICES.md`, `LICENSE` — warning, evidence, configuration and attribution.
- GitHub's matching source archives at the exact release tag. Full source, asset notices, SDK source pin and generator remain available for rebuilds.

No PKG/FPKG, firmware file, token or SDK archive is uploaded. Local ELF hashes may differ between toolchains or checkout paths due to debug/build metadata; reproducible means a documented fresh-source build with pinned SDK and deterministic embedded assets, not a claim of cross-platform bit-identical ELF output.

## Automation limits

The currently connected GitHub tools can publish Git objects and inspect Actions; they do not expose workflow dispatch or Release creation. An authenticated browser or GitHub CLI is needed for the one manual dispatch. No credentials should be embedded in project files. With an authenticated CLI, the equivalent is:

```sh
gh workflow run dev-release.yml --repo MNeroba/SteamController-PS5 --ref main \
  -f source_sha=<tested-full-commit-sha> -f version=v0.1.0-dev.3
```

Retain history: earlier SHA references are evidence, provenance and regression context. A cosmetic squash gives no build benefit. There is no proposed ref rewrite or force push.

## dev.2 first launch

Missing policy enables only WebUI on port 8090. Pair using the short-lived console notification code; no FTP is needed. Unsafe or malformed configuration fails closed; hooks, bridge probe and DualSense handoff remain opt-in. The audit remediation register is [STATIC_AUDIT.md](STATIC_AUDIT.md). No PS5 runtime result is inferred from this prerelease.

## dev.3 optional tile

Still one ELF/six assets. The PNG and Media/deeplink metadata are embedded in that ELF; generation/checksum gates include both headers. Authenticated explicit confirmation is required before app writes. On-console registration and browser behavior remain H-13 NOT_RUN; it does not alter stable firmware/evidence gates. Historical dev.2 notes/assets retain their FW12.00 target.
