# Dev.3 review candidate — 2026-10-12

v0.1.0-dev.3 remains unpublished. No tag or experimental workflow dispatch is authorized until owner review/approval. Published dev.1/dev.2 releases remain unchanged. The acknowledgement section below will identify the actual selected source SHA and local artifact digest.

## Implemented scope

- Confirmed read-only user-list logout/stable-login consensus; proven-owned virtual DualSense recovery in the existing USB worker. Preserve USB/Steam state, single active controller, pause/disconnect/release exclusions and failed deletion ownership. No automatic user assignment/native DualSense mutation.
- Preserve IN reports collected during OUT; reject buffer/index reuse while completions/cancellation ownership remain unresolved, validate status/frame/length and500ms valid-input watchdog. Failed optional OUT can leave healthy IN running.
- Authenticated Settings & help → Diagnostics text download: actual runtime firmware, version/SHA/SDK, startup/configuration/USB/VDA/Steam snapshots and latest48 filtered memory log records. No arbitrary files, credentials, serials or raw system logs.
- Rate-limited truthful connection notices, Payload Manager JSON sidecar, actual checksum/source identity and mandatory changelog/release-note matching for DEV and stable versions. Historical releases are immutable.
- Existing single ELF/optional confirmed tile/artwork/pairing/profile/gyro/touch/haptic features retained; safe defaults, /data/ghostpad and SDKv0.43 unchanged.

## Evidence and limitations

Reference LibrePadPS5 v1.1.2 commit7466317b4b356249e07770f3914c995e1a4d633a: actual source inspected and GPL obligations reviewed; no implementation copied. Upstream logout recovery is host evidence only. Storage regression warning/migration and XInput/8BitDo commands were excluded. See PROVENANCE, ADR-055, M1-236/237 and TEST_PLAN.

Local gates:44 C suites + Python normal/ASan/UBSan; Chromium220;9 artwork/generator tests;7 DEV policy/package plus5 changelog/metadata checks;45-C clean SDKv0.43/LLVM18 build, deterministic assets, optional-dependency and diff checks. Packaging creates seven files containing exactly one ELF. Actual selected source/digest are recorded after commit. Physical H-13/H-14/H-15 and all stable hardware gates remain NOT_RUN. Positive VDA receipt semantics, native account picker/coexistence, actual USB close/cancellation timing and PS5 browser/notification behavior require a console.

Prepared metadata URL is a future dev.3 download, unavailable before publication; this private repository also requires authorized access. The sidecar is not a Payload Manager catalog wrapper. No release publication, console deployment, tile registration or radio operation was performed.

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
