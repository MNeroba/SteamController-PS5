# Dev.3 review and publication — 2026-10-12

Owner approved publication on2026-10-12. [Experimental dev.3](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v0.1.0-dev.3) is published from `9aab6dbae166b28416a9a00e6346f85bf12e524e`; workflow38225113776 passed all gates. Published dev.1/dev.2 remain unchanged. The local review acknowledgement below is historical; the publication acknowledgement identifies the actual download.

## Implemented scope

- Confirmed read-only user-list logout/stable-login consensus; proven-owned virtual DualSense recovery in the existing USB worker. Preserve USB/Steam state, single active controller, pause/disconnect/release exclusions and failed deletion ownership. No automatic user assignment/native DualSense mutation.
- Preserve IN reports collected during OUT; reject buffer/index reuse while completions/cancellation ownership remain unresolved, validate status/frame/length and500ms valid-input watchdog. Failed optional OUT can leave healthy IN running.
- Authenticated Settings & help → Diagnostics text download: actual runtime firmware, version/SHA/SDK, startup/configuration/USB/VDA/Steam snapshots and latest48 filtered memory log records. No arbitrary files, credentials, serials or raw system logs.
- Rate-limited truthful connection notices, Payload Manager JSON sidecar, actual checksum/source identity and mandatory changelog/release-note matching for DEV and stable versions. Historical releases are immutable.
- Existing single ELF/optional confirmed tile/artwork/pairing/profile/gyro/touch/haptic features retained; safe defaults, /data/ghostpad and SDKv0.43 unchanged.

## Evidence and limitations

Reference LibrePadPS5 v1.1.2 commit7466317b4b356249e07770f3914c995e1a4d633a: actual source inspected and GPL obligations reviewed; no implementation copied. Upstream logout recovery is host evidence only. Storage regression warning/migration and XInput/8BitDo commands were excluded. See PROVENANCE, ADR-055, M1-236/237 and TEST_PLAN.

Local gates:44 C suites + Python normal/ASan/UBSan; Chromium220;9 artwork/generator tests;7 DEV policy/package plus5 changelog/metadata checks;45-C clean SDKv0.43/LLVM18 build, deterministic assets, optional-dependency and diff checks. Packaging creates seven files containing exactly one ELF. Actual selected source/digest are recorded after commit. Physical H-13/H-14/H-15 and all stable hardware gates remain NOT_RUN. Positive VDA receipt semantics, native account picker/coexistence, actual USB close/cancellation timing and PS5 browser/notification behavior require a console.

The metadata URL now resolves to the published dev.3 asset; authorized access is needed while the repository remains private. The sidecar is not a Payload Manager catalog wrapper. No console deployment, tile registration or radio operation was performed.

## Exact changed paths

- `.github/workflows/dev-release.yml`
- `.github/workflows/release.yml`
- `.gitignore`
- `CHANGELOG.md`
- `CODEX_TASK.md`
- `README.md`
- `STEAM_CONTROLLER_2026.md`
- `config/payload/SteamController-PS5.elf.json`
- `docs/ARCHITECTURE.md`
- `docs/PROVENANCE.md`
- `docs/README.md`
- `docs/SOURCES_AND_EVIDENCE.md`
- `docs/TEST_PLAN.md`
- `docs/development/DEV_RELEASE.md`
- `docs/development/dev-release.json`
- `payload/Makefile`
- `payload/controller_sc2.c`
- `payload/diagnostics.c`
- `payload/diagnostics.h`
- `payload/gc_main.c`
- `payload/gen_build_info.py`
- `payload/gen_html.py`
- `payload/slot_lifecycle.c`
- `payload/slot_lifecycle.h`
- `payload/slot_registry.c`
- `payload/slot_registry.h`
- `payload/usb_helpers.c`
- `payload/usb_resources.c`
- `payload/usb_resources.h`
- `payload/usb_session.c`
- `payload/usb_session.h`
- `payload/user_session.c`
- `payload/user_session.h`
- `payload/version.h`
- `payload/web/index.html`
- `payload/webui.c`
- `payload/webui_route.c`
- `payload/webui_route.h`
- `project-memory/CURRENT_STATUS.md`
- `project-memory/DECISIONS.md`
- `project-memory/FINDINGS.md`
- `project-memory/HARDWARE_MATRIX.md`
- `project-memory/ISSUES.md`
- `project-memory/PROJECT_MEMORY.md`
- `project-memory/README.md`
- `project-memory/SESSION_LOG.md`
- `project-memory/TEST_LOG.md`
- `scripts/check_browser_result.py`
- `scripts/check_changelog.py`
- `scripts/package_dev_release.py`
- `tests/Makefile`
- `tests/test_changelog.py`
- `tests/test_dev_release.py`
- `tests/test_diagnostics.c`
- `tests/test_slot_recovery.c`
- `tests/test_usb_completion.c`
- `tests/test_usb_helpers.c`
- `tests/test_user_session.c`
- `tests/test_webui_controllers.py`
- `tests/test_webui_server.c`
- `tests/webui_remap_browser.js`
- `docs/development/DEV3_REVIEW.md`


## Source and local artifact acknowledgement

Implementation: `9aab6dbae166b28416a9a00e6346f85bf12e524e`; tree `1512f3fcb9e694d54e55a4d5c8afb982e2d0ca65`. Main update acknowledged,282 blobs/all267 previous files retained/45 C/zero deletions. Fresh complete export normal host and exact-SHA SDK build PASS. Local review artifacts: `output/dev3-review/` (ignored, not GitHub release assets). One ELF2028864 bytes, SHA-256 `fc71b6acbf9ac857e3fe13259b37bdfecc0cb150a313db355222d3b4258188af`; actual checksum and matching metadata/source/version/tile bytes verified. Linux workflow hashes may differ by build path/toolchain; publication will verify its own actual artifact. SDK push38223580419 PASS; Host38223580325 PASS. No dev.3 tag/release; waiting owner review/approval.


## Publication acknowledgement

VERIFIED_HOST_TEST / VERIFIED_PS5_BUILD: owner-approved workflow38225113776 completed/success; select-source114730599939, validate-build114730632562 and publish-prerelease114731524283 all PASS. Repeated normal44 C/Python, ASan/UBSan, Chromium220, deterministic embedded assets, SDKv0.43/LLVM18 clean build, mandatory changelog/metadata/package/checksum gates passed. Publication job downloaded its tested artifact and rechecked ELF checksum OK.

GitHub release410387546 acknowledged published2026-10-12T08:44:17Z at https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v0.1.0-dev.3; draft=false/prerelease=true. Tagdev/v0.1.0-dev.3 and target both exactly9aab6dbae166b28416a9a00e6346f85bf12e524e. Seven uploaded/nonempty assets: one ELF, checksum, Payload Manager JSON, RELEASE_NOTES, LOADING, NOTICES and LICENSE; source archives also available. Notes contain exactly Bug Fixes and New Features / Improvements, exact source and explicit experimental/FW13.60-unverified warning. Historical dev.1/dev.2 source/updated timestamps remain unchanged.

Actual browser-downloaded ELF2029288 bytes, ELF64/little-endian/x86_64; SHA256013e27f60091cd5de87577beb9acd40ac5ae5eeba08092b7dff65bf56847c752 matches release body and GitHub asset digest. Exact source SHA and version strings verified in downloaded bytes; local copy output/dev3-published/SteamController-PS5-v0.1.0-dev.3.elf. This published Linux digest supersedes local Darwin/path hash for download verification. Sidecar checksum verified in release workflow; sidecar JSON/notes packaging gates passed, not independently downloaded here. Browser download call unexpectedly blocked about35min before returning the actual path; publication itself finished within3min. Browser screenshot APIs unavailable despite two attempts, so no screenshot artifact is claimed. No credential/console data exposed.

Physical PS5 testing, deployment, Settings tile installation and radio operations NOT_RUN. H13/H14/H15 and stable hardware gates remain blocked. Next: owner can download the single ELF and read LOADING/NOTICES; before an explicitly authorized runtime experiment, follow TEST_PLAN/HARDWARE_MATRIX and record measured firmware, native DualSense baseline, owned VDA lifecycle, logout recovery, cancellation/watchdog and optional-tile browser behavior. Do not infer console compatibility from host/build evidence.
