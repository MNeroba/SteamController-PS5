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

**Full reachable Git-history scanning, old branches, issue/PR attachments and all historical release assets were NOT_RUN in this task.** The inherited desktop `.git` is not the published repository history. Checking current main cannot prove a secret was never committed. Before switching visibility, scan an authenticated fresh full clone, including reachable branches/tags, with a locally installed reputable secret scanner and review redacted findings. Keep reports outside Git. If an actual secret is found, revoke it and arrange a separate remediation; do not silently rewrite history or alter existing releases.

## Rights and source availability

GPL code/notices, PoorDS4 attribution, community SDK provenance and archived upstream documents are retained. SDK archives, proprietary firmware and credentials are not committed. Release tags expose the corresponding source/build scripts, and release notices accompany binaries.

Controller photos and the tile image have owner-authorized repository/release publication. No separate open asset license or photographer attribution was supplied. Public access does not establish a GPL/CC0 grant for those assets; keep their [controller](../../payload/web/controllers/NOTICE.md) and [tile](../../payload/settings-tile/NOTICE.md) notices. If broader artwork reuse is desired, obtain an explicit asset license separately.

## Owner actions before visibility change

1. Review the scoped privacy findings and complete the full-history check above. Historical logs will become readable along with the source.
2. Review access to any old branches, issues/PR attachments and historic release assets; this current-tree audit does not cover them.
3. Enable and verify GitHub private vulnerability reporting if desired. SECURITY.md offers a conditional route until it is actually available; no invented contact email or response SLA is advertised.
4. Update the GitHub **About** description, which still mentions12.00 at this checkpoint. Suggested text:

   > Experimental Steam Controller 2026 payload for PS5: USB-C/Puck, remapping, gyro, trackpads and haptics. One ELF; host-tested, PS5 hardware unverified.

   Optional topics: `ps5`, `steam-controller`, `homebrew`, `usb`, `gamepad`, `experimental`. These metadata suggestions were not applied here.
5. Review README, LICENSE, asset notices, CONTRIBUTING, SECURITY and the issue forms, then change visibility yourself when ready. No source or binary claims a compatible firmware or a tested loader.

Hardware H-01–H-15 remain NOT_RUN/BLOCKED where applicable; native BLE remains research. Opening the source does not authorize console deployment, stock-radio operation or stable publication.


## Documentation publication and validation

Documentation source `e7f53f2cf1466bed0f8462bc350155b45b101eb9` published on main. Complete tree286 files retains every282 previous path;29 focused paths changed, four new guides/forms, no runtime source/asset/test/script/workflow changes. Current Host38243078629 and SDK38243078701 completed successfully, including normal, sanitizer, Chromium and cross-build checks. Targeted109 link checks, changelog5 and DEV package7 tests, issue-form YAML/ID/field validation and diff checks passed. This receipt corrects earlier workflow labels without changing results. Repository visibility/metadata/security settings remain unchanged.
