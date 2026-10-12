# Public repository preparation — 2026-10-12

This checkpoint prepares documentation and contributor entry points. Repository visibility was checked as **private**; this task does not change it. Public source access and a hardware-verified stable release are separate decisions.

## Reviewed state

- Authoritative pre-update main: `3a99866d63c13744c6dc3df90439f70d1c713b90`, complete tree `6f81350afedb36c1c30d9e4218e1cbada2830f94`, 282 files. Host38229733009 and SDK38229732850 completed successfully; no open PRs were returned.
- Published experimental dev.3: source/tag `9aab6dbae166b28416a9a00e6346f85bf12e524e`, workflow38225113776 passed, seven release assets containing one ELF. Dev.1/dev.2 and all release tags/assets remain unchanged.
- Downloaded dev.3 ELF: 2,029,288 bytes; SHA-256 `013e27f60091cd5de87577beb9acd40ac5ae5eeba08092b7dff65bf56847c752`. M1-238 records its independent download/source verification.
- README/build/UI/feature guidance now identifies dev.3, SDK v0.43, 220 browser assertions and intended **FW13.60, unverified**. Earlier12.00 records are historical and retained.
- Added contributor bug/feature issue forms and [SECURITY.md](../../SECURITY.md). Runtime code and default safety policy are unchanged.

## Publication and privacy review

A local inventory was compared against all282 current remote file SHAs. Only SESSION_LOG/TEST_LOG differed initially by the already-recorded local publication self-receipt; all other bytes matched. A bounded signature scan of these text files found no private-key headers, GitHub PAT patterns, AWS access-key patterns or URLs with embedded credentials. No token, environment-secret, ELF/object/archive or private-key filename was present in this snapshot.

Two MAC-shaped strings belong to deliberately artificial diagnostic-redaction fixtures (`01:23:45:67:89:ab`). The only email match is `fixture@example.invalid`. Fifteen historical local paths contain the maintainer's existing GitHub username. They are retained as command/environment evidence; they are not live console captures or credentials. The four WebPs and four documentation PNGs contain no detected EXIF/XMP/text metadata chunks. Tile JPEG/PNG metadata was inspected: resolution/orientation/dimensions/color-space/toolkit fields only, no GPS, creator/copyright or populated IPTC records found. This is a targeted review, not a guarantee that every possible identifier is absent.

At the initial M1-239 checkpoint, full history and attachment review were NOT_RUN. The follow-up M1-240 below supersedes that scope limitation for reachable Git objects and repository discussions. It does not erase the initial result or certify every historical release asset.

## Reachable-history review — M1-240

Reviewed remote main `af183a2a0ed9b98e427864c216df9d68da854363` and `fix/p0-safety` at `97097c5e715c2fdd13399895dc462c4d30fda596`, plus all three `dev/` tags. Authenticated GitHub API enumeration returned **140 distinct commits and 140 complete, untruncated trees**, ending at root `61780b03bcef6859935cac1ef288cf3eef12d3ba`. Every parent and tag target belongs to this closed inventory. The inherited local branch was not used as the history authority.

- Read and scanned **all 1,413 distinct UTF-8 blobs** from those remote trees. Targeted rules covered private keys, GitHub/AWS/Slack/Google/Stripe credentials, JWTs, credential-bearing URLs and quoted credential assignments; additional checks covered email, MAC addresses, private IPs and local user paths. No credential-pattern matches were found. This was a custom bounded signature review, not a Gitleaks/TruffleHog run or a guarantee against every secret format.
- Resolved **all 63 binary image blobs** to matching current bytes or SHA-verified local Git objects. PNG/WebP metadata inspection found no text/EXIF/XMP chunks except the already-reviewed tile PNG; tile JPEG/PNG technical metadata remains unchanged. No new metadata privacy finding. Historical screenshots were structurally inspected, not independently OCR-reviewed in this follow-up.
- Text findings are the synthetic diagnostic MAC, `fixture@example.invalid`, documented/test private-network addresses and `/Users/mneroba` paths in historical execution records. The inherited Makefile default `192.168.69.44` is a private-network address, not a credential or verified console address. Commit authors use the owner's GitHub noreply address. No personal email, real controller MAC, credential filename, committed ELF/SDK archive or private key was found by these checks.
- One closed PR (#1), no standalone issues, and zero issue comments, PR review comments or reviews were returned. Its head is included above. The only attachment reference found in Git documentation is an inherited upstream controller demonstration URL; its remote media content was not independently inspected. No project issue/PR attachment was returned.
- Reviewed all three release bodies and inventories: **19 assets**, three experimental prereleases with corresponding tagged sources and warnings. Re-scanned the already-downloaded dev.2/dev.3 ELF bytes after confirming their current GitHub SHA-256 digests; no private-key/PAT/AWS/MAC signature matches, only `/home/runner` build paths. Dev.1 ELF and every historical sidecar were not independently downloaded/re-scanned in this follow-up; earlier publication evidence is retained. Do not call this an exhaustive binary-forensics audit.
- Exact reviewed main CI is green: Host **38243782521**, PS5 cross-build **38243782563**. No physical PS5 validation was performed.

**No confirmed public-publication blocker was identified in the reviewed scope.** The repository can be presented as experimental source with the documented artwork rights and hardware limitations. Maintainer username paths and historical negative results will be public. Deleted/unreachable GitHub objects, unseen discussions outside this repository and arbitrary secret formats are outside this review. If an actual credential is later identified, revoke it and propose separate history remediation; no history or release was rewritten here.

## Rights and source availability

GPL code/notices, PoorDS4 attribution, community SDK provenance and archived upstream documents are retained. SDK archives, proprietary firmware and credentials are not committed. Release tags expose the corresponding source/build scripts, and release notices accompany binaries.

Controller photos and the tile image have owner-authorized repository/release publication. No separate open asset license or photographer attribution was supplied. Public access does not establish a GPL/CC0 grant for those assets; keep their [controller](../../payload/web/controllers/NOTICE.md) and [tile](../../payload/settings-tile/NOTICE.md) notices. If broader artwork reuse is desired, obtain an explicit asset license separately.

## Owner actions before visibility change

1. Review the M1-240 scope and residual limitations above. Reachable branch/tag text history is covered; historical logs and maintainer username paths will become public.
2. Retain all existing source/license/asset notices and experimental release warnings. An additional independent secret-scanner or exhaustive historical binary review may provide extra assurance; neither was claimed here.
3. Enable and verify GitHub private vulnerability reporting if desired. SECURITY.md offers a conditional route until it is actually available; no invented contact email or response SLA is advertised.
4. Update the GitHub **About** description, which still mentions12.00 at this checkpoint. Suggested text:

   > Experimental Steam Controller 2026 payload for PS5: USB-C/Puck, remapping, gyro, trackpads and haptics. One ELF; host-tested, PS5 hardware unverified.

   Optional topics: `ps5`, `steam-controller`, `homebrew`, `usb`, `gamepad`, `experimental`. These metadata suggestions were not applied here.
5. Review README, LICENSE, asset notices, CONTRIBUTING, SECURITY and the issue forms, then change visibility yourself when ready. No source or binary claims a compatible firmware or a tested loader.

Hardware H-01–H-15 remain NOT_RUN/BLOCKED where applicable; native BLE remains research. Opening the source does not authorize console deployment, stock-radio operation or stable publication.


## Documentation publication and validation

Documentation source `e7f53f2cf1466bed0f8462bc350155b45b101eb9` published on main. Complete tree286 files retains every282 previous path;29 focused paths changed, four new guides/forms, no runtime source/asset/test/script/workflow changes. Current Host38243078629 and SDK38243078701 completed successfully, including normal, sanitizer, Chromium and cross-build checks. Targeted109 link checks, changelog5 and DEV package7 tests, issue-form YAML/ID/field validation and diff checks passed. This receipt corrects earlier workflow labels without changing results. Repository visibility/metadata/security settings remain unchanged.
