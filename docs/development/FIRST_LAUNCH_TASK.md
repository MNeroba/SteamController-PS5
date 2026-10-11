# Codex Task — Simplify First Launch & Release v0.1.0-dev.2

**Repository:** https://github.com/MNeroba/SteamController-PS5
**Previous release:** `v0.1.0-dev.1`
**Target release:** `v0.1.0-dev.2`
**Priority:** P1 — First-run UX and experimental tester readiness

## Objective

Make SteamController-PS5 as easy to launch as typical PS5 homebrew payloads.

**Expected user experience:**

1. Download the ELF.
2. Load it through an existing PS5 payload loader.
3. WebUI starts automatically on port `8090`.
4. Open `http://PS5-IP:8090` from a PC or phone.
5. Configure the Steam Controller without manually creating configuration files or accessing the PS5 filesystem.

Remove unnecessary first-launch friction while preserving the existing security architecture.

Implement the changes, run all applicable tests, publish the verified source to `main`, and create the experimental release `v0.1.0-dev.2`.

## 1. Enable WebUI automatically

Inspect and update:

- `payload/runtime_policy.c`
- `payload/runtime_policy.h`
- `payload/gc_main.c`
- `payload/webui.c`

Change default behavior:

| Setting | Default |
|---|---|
| `ENABLE_WEBUI` | `1` |
| `ENABLE_BRIDGE_PROBE` | `0` |
| `ENABLE_GAME_HOOKS` | `0` |
| `ENABLE_DUALSENSE_HANDOFF` | `0` |

Requirements:

- When `runtime-policy.ini` is absent, automatically enable WebUI.
- Preserve explicit `ENABLE_WEBUI=0` for advanced users.
- Preserve all other safe defaults.
- Distinguish missing configuration from malformed, inaccessible or unsafe configuration.
- Invalid configuration must fail closed and must not accidentally enable dangerous functionality.
- No FTP, manual configuration or restart should be required for normal first launch.
- Do not modify the controller mapping engine, USB/Puck transport or virtual DualSense lifecycle.

## 2. Simplify first-time WebUI authorization

Currently, the user must retrieve `/data/ghostpad/webui.token` using FTP or a file manager.

This is unacceptable for the standard first-run experience.

Implement a secure, minimal first-time pairing mechanism that requires no filesystem access.

Preferred design:

- WebUI opens immediately without requiring credentials to display its interface.
- Protected API operations remain authenticated.
- Generate a short-lived pairing code using a cryptographically secure random source.
- Show the pairing code using the existing PS5 notification mechanism in `gc_main.c`.
- Allow the user to enter this code in a simple, integrated WebUI pairing screen.
- On successful pairing, issue or establish the existing strong bearer credential for that browser session.
- Reuse valid previously established credentials when possible.
- Keep `/data/ghostpad/webui.token` as a compatible advanced recovery mechanism.

Security requirements:

- Pairing code must expire, be single-use and have strict attempt limits.
- Prevent brute-force attempts and concurrent redemption.
- Never expose the permanent token through an unauthenticated endpoint, URL or console notification.
- Never log pairing secrets or bearer credentials.
- Preserve Origin/Host checks and LAN peer restrictions.
- Retain authentication for all existing protected API operations.
- Do not weaken the existing HTTP parsing, profile persistence or authorization mechanisms.
- Preserve the existing restriction against exposing WebUI to the public internet.

Reuse existing authentication and HTTP modules. Do not introduce a separate authentication framework or unnecessary dependencies.

If PS5 notifications cannot reliably support pairing, evaluate the smallest safe alternative. Do not silently replace authentication with unrestricted LAN access.

## 3. Improve startup notifications

After successfully starting the WebUI listener, show a concise PS5 notification.

Example:

`SteamController-PS5 — WebUI ready on port 8090`

Preferably include the actual local IP address if it can be obtained safely and reliably.

Important:

- Do not display a guessed IP.
- Confirm successful listener startup before announcing readiness.
- Report bind, socket and initialization failures clearly.
- Avoid repeated or excessive notifications.
- Preserve normal payload startup if the WebUI service fails, unless a security-critical failure requires stopping.

## 4. First-launch WebUI experience

Update `payload/web/index.html` as needed.

Requirements:

- Replace the current `window.prompt()` token dialog with a clean, integrated pairing UI.
- Match the existing professional WebUI design.
- Support desktop and mobile browsers.
- Display clear connection, pairing, authentication and backend-offline states.
- Preserve all controller artwork, SVG hotspot coordinates and existing navigation.
- Display a useful disconnected-controller state without breaking the WebUI.
- Preserve profile editing, Save/readback, import/export and unsaved-change protections.

Do not redesign the whole WebUI.

## 5. Tests and regression protection

Extend existing tests rather than creating redundant test infrastructure.

Cover:

- Fresh first launch with no configuration files.
- WebUI enabled by default.
- Explicit WebUI disable.
- Invalid/malformed configuration.
- Missing, corrupted or inaccessible token storage.
- Pairing success, expiration, reuse rejection and brute-force protection.
- Existing authorized API behavior.
- Rejected unauthorized mutations.
- Browser reload, reconnect and session restoration.
- Concurrent pairing and controller profile operations.
- WebUI startup/bind failure and clean shutdown.

Run:

```bash
make -C tests clean test
make -C tests sanitize
bash scripts/run_webui_browser.sh
make -C payload clean
make -C payload
git diff --check
```

Use the pinned PS5 Payload SDK v0.43 and LLVM 18.

Preserve all existing Chromium assertions and add regression coverage for the new first-run flow.

A successful host test or cross-build is not proof of physical PS5 compatibility.

## 6. Documentation

Update relevant documentation:

- `README.md`
- `docs/development/LOADING.md`
- `docs/development/DEV_RELEASE.md`
- `docs/WEBUI_REMAP.md`, if applicable
- Relevant project-memory files
- `scripts/package_dev_release.py`, where release notes describe outdated WebUI defaults

The normal installation guide should now be extremely simple:

1. Load the ELF.
2. Open `http://PS5-IP:8090`.
3. Complete first-time pairing if requested.
4. Configure the controller.

Keep advanced security and recovery instructions in a separate section.

Remove obsolete statements claiming that WebUI is disabled by default.

## 7. Publish v0.1.0-dev.2

After successful implementation and verification:

1. Review all changes, check provenance and ensure no unrelated changes.
2. Commit the complete reviewed implementation.
3. Push to `main` without force push.
4. Wait for successful Host tests and PS5 cross-build GitHub Actions on the exact source SHA.
5. Follow the existing `docs/development/DEV_RELEASE.md` procedure.
6. Trigger `.github/workflows/dev-release.yml` using:
   - Version: `v0.1.0-dev.2`
   - Source SHA: exact validated 40-character commit SHA.
7. Create GitHub prerelease with tag `dev/v0.1.0-dev.2`.
8. Verify the actual published ELF, SHA-256 and release assets.

Expected release assets:

- `SteamController-PS5-v0.1.0-dev.2.elf`
- `SteamController-PS5-v0.1.0-dev.2.elf.sha256`
- `RELEASE_NOTES.md`
- `LOADING.md`
- `NOTICES.md`
- `LICENSE`

Do not manually rename an older ELF or reuse the `dev.1` binary.

Do not bypass release gates, weaken critical-blocker checks or claim physical hardware verification.

If GitHub Actions dispatch or publication is unavailable, finish the validated implementation and report the exact remaining manual release step. Do not claim that `dev.2` was published unless GitHub confirms it.

## 8. Acceptance criteria

The task is complete when:

- A tester can load the ELF and access the WebUI without FTP or manual INI configuration.
- First-time authorization requires no access to internal PS5 files.
- Existing authenticated API protection remains effective.
- Default dangerous functionality remains disabled.
- Existing controller mapping and profile behavior have no regressions.
- Host, sanitizer, browser and PS5 cross-build gates pass.
- `v0.1.0-dev.2` is published with a verified downloadable ELF and checksum.
- Release documentation accurately describes the new behavior.

## Final report

Provide:

- Source commit SHA.
- Files changed and key implementation decisions.
- Exact first-run user experience.
- Authentication and security changes.
- Tests executed and results.
- Remaining hardware-dependent risks.
- GitHub Actions status.
- Release URL and ELF SHA-256.

**Do the full implementation and release workflow. Do not stop after analysis or planning. Keep the scope focused on first-launch simplification and release v0.1.0-dev.2.**
