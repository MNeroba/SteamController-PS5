# Changelog

DEV entries describe software and build verification. They do not establish physical PS5 compatibility. The dev.1–dev.9 and v1.0.0-experimental.1/.2 GitHub releases and tags were removed during the 1.0.0 Experimental consolidation; the entries below remain historical records and their assets are archived outside the repository.

## v1.6.3-experimental

2026-11-28 · Experimental development release. Host and SDK verification only.

### Bug Fixes

- Align Steam Controller and DualSense remapping outlines with the physical controls, including curved bumper and trigger boundaries.

### New Features / Improvements

- Open **On phone** in the PS5 WebUI to start a temporary phone session with an offline QR code, local IP and port, and an eight-digit one-time code.
- Edit and save existing profiles from the phone over the home network. The console controls a one-hour session and can revoke it at any time.
- Add a README hero banner, controller gallery and quick-start instructions for phone access.

## v1.6.2-experimental

Experimental follow-up to 1.6.1. Not a stable release and not a hardware claim; physical PS5 acceptance remains NOT_RUN and firmware 13.60 is still the intended, unverified target.

### Bug Fixes

- Haptic test requests now report enqueue failure: `/api/haptic` returns 503 when the queue is full or no Steam session owns output, instead of claiming "queued". The haptics API returns enqueue status; existing game/menu/trigger callers ignore it.
- The troubleshooting log is filtered with the same line-level redaction as the diagnostic report (private keywords, long digit/hex runs and colon-separated addresses) before it is served.
- HTTP responses now support `HEAD` (headers only) and `OPTIONS` (204 with `Allow`); `405` responses include `Allow` for the route. `GET`/`HEAD`/`OPTIONS` read routes no longer take the global dispatch lock, so a slow tile install, hook toggle or icon response cannot block status/log reads.
- `recover-default` distinguishes "already valid" (409) from a failed backup/save (500); profile deletion returns 409 for Default, 404 for a missing profile and 500 for a real delete failure.
- The WebUI gains profile **Rename** and **Duplicate** actions; duplicate creates a new validated profile from the current editor state and refuses existing IDs.

### New Features / Improvements

- Browser regression now exercises pause, hook switch, hand-off policy, bridge policy, log and per-game profile-lock mutations, including exact query keys.
- Version `v1.6.2-experimental` across `version.h`, changelog, Payload Manager metadata and the DEV workflow default.

## v1.6.1-experimental

Experimental follow-up to 1.6.0. Not a stable release and not a hardware claim; physical PS5 acceptance remains NOT_RUN and firmware 13.60 is still the intended, unverified target.

### Bug Fixes

- The game-bridge check now starts and stops from the WebUI: the checkbox persists `ENABLE_BRIDGE_PROBE` through the private policy writer, starts the read-only worker when a foreground user is known, stops it on disable, and reverts policy and worker state on failure. If no user was available at startup, it reports that a payload restart after login is required instead of pretending the worker is running.
- DualSense hand-off now has the same explicit runtime switch. Enabling starts the system-log observer when needed, re-arms the hand-off service and persists `ENABLE_DUALSENSE_HANDOFF`; disabling suspends new hand-offs immediately and reports that an already-disconnected DualSense must be restored with its PS button.
- The portal reports hand-off and bridge policy/worker state separately (`allowed`, `enabled`, `active`) so a persisted setting that cannot start a worker is visible instead of being shown as active.
- `/api/dualsense?handoff=0|1` and `/api/bridge?enable=0|1` reject malformed values, duplicate `on`+`handoff` parameters and GET mutations; direct-HID policy conflicts are refused before any worker starts.

### New Features / Improvements

- HTTP-level mutation coverage now exercises profile write/delete, pause, override, log, DualSense hand-off, bridge enable/request and the full five-key game-hook query against the production server and real dispatch path.
- Version `v1.6.1-experimental` across `version.h`, changelog, Payload Manager metadata and the DEV workflow default.

## v1.6.0-experimental

Experimental WebUI quality and completeness release. Not a stable release and not a hardware claim; physical PS5 acceptance remains NOT_RUN and firmware 13.60 is still the intended, unverified target. No radio, firmware, pairing or virtual-pad ABI change is included.

### Bug Fixes

- The off-screen save bar is no longer keyboard/screen-reader reachable before a profile is loaded; Save and Undo are inert while hidden, and a profile save is refused until a canonical profile has been loaded. This removes the path that could POST an empty activator set and clear existing long/double/shift/turbo/toggle bindings.
- Save-state handling no longer reports a confirmed save as failed when the follow-up profile list refresh fails; Undo and the unsaved-changes Discard path clear the "Save not confirmed" state; an in-flight save cannot be cancelled by Discard.
- Core controls now surface failures instead of failing silently: the per-game profile lock (`#mode`), pause, log show/copy/previous, DualSense hand-off, game-hook switches and "Use recommended settings" report an error and revert the control when the request fails.
- Polling backs off to 1.2 s while the backend is offline, pauses while the page is hidden, resumes immediately on visibility, and clears the stale battery/paused indicators. Icon hydration is sequential instead of firing one request per installed game at once, reducing pressure on the four-client serialized server.
- The remap sheet keeps keyboard focus after activator/picker re-renders, marks the background inert while open, restores focus on close, and the unload guard now covers a staged sheet draft.
- The game list distinguishes a read failure from an empty library and offers Retry; a settings-only profile file is accepted by the import heuristic; a malformed `#profile=` hash cannot abort startup.
- DualSense hand-off and the game-bridge check now report when their runtime-policy flags are off, disable the affected controls, and explain how to enable them instead of appearing functional and failing with 403 or doing nothing.
- The route parser accepts a fully populated `/api/gamehooks` query (all five allowed keys); the status JSON buffer has more room and non-finite gyro samples are written as 0; `/api/log` and `/api/icon` open regular files with `O_NOFOLLOW` instead of following symlinks.

### New Features / Improvements

- Version `v1.6.0-experimental` across `version.h`, changelog, Payload Manager metadata and the DEV workflow default.
- `/api/dualsense` and `/api/bridge` now expose their runtime-policy availability; the WebUI uses it to label disabled features honestly.
- Tab lists expose `aria-controls`/`tabpanel` relationships and support arrow/Home/End navigation; picker and sheet focus handling is documented by host/browser regressions.

## v1.5.2-experimental

Experimental follow-up to 1.5.1. Not a stable release and not a hardware claim; physical PS5 acceptance remains NOT_RUN and firmware 13.60 is still the intended, unverified target.

### Bug Fixes

- The "In games" section now has an explicit **Enable game hooks (Experimental)** checkbox. It persists `ENABLE_GAME_HOOKS` through the existing private policy writer, starts or stops the game-hook worker immediately, requires a confirmation dialog, refuses to combine hooks with `ENABLE_DIRECT_HID`, reverts the policy if the worker cannot start, and reports a retained rollback. This removes the manual runtime-policy file step for PS5 vibration/haptics while keeping the default off.
- A disabled or failed toggle shows an accurate status instead of implying hooks will attach; the previous "runtime policy only" message is gone.

### New Features / Improvements

- Version `v1.5.2-experimental` across `version.h`, changelog, Payload Manager metadata and the DEV workflow default.
- ADR-070 records the owner-authorized local UI toggle, its confirmation/persistence/conflict rules and the unchanged trusted-loopback limitation.
- README/LOADING/SECURITY/ARCHITECTURE explain that PS5-native haptics still require this experimental path and that classic PS4-style rumble remains automatic (v1.5.1).

## v1.5.1-experimental

Experimental follow-up to 1.5.0. Not a stable release and not a hardware claim; physical PS5 acceptance remains NOT_RUN and firmware 13.60 is still the intended, unverified target.

### Bug Fixes

- The payload now reads classic DualShock-4-style rumble back from the owned virtual pad with `scePadVirtualDeviceGetRemoteSetting` (large motor at byte 3, small motor at byte 4, polled every 16 ms) and forwards it to the Steam Controller when game hooks are not active. PS4 games fill these fields; PS5-native haptics are converted to DualSense haptic audio inside the system and leave them at zero, so this does not replace the opt-in game hooks for PS5 games. The previous "no reverse channel" statement was too broad and is corrected in the docs.
- Rumble from this path stops when the motors return to zero, when the virtual pad is detached, when game hooks take over, and at session teardown.

### New Features / Improvements

- Version `v1.5.1-experimental` across `version.h`, changelog, Payload Manager metadata and the DEV workflow default.
- Corrected README, LOADING and ARCHITECTURE feedback descriptions and added the octet/padbridge reference for the remote-setting ABI facts.

## v1.5.0-experimental

Source-release candidate for the mechanism-audit fixes. Not a stable release and not a hardware claim: physical PS5 acceptance remains NOT_RUN and firmware 13.60 is still the intended, unverified target.

### Bug Fixes

- The Media tile installer resolves `sceAppInstUtilInitialize` (`540lotO7oHE`), `sceAppInstUtilTerminate` (`kLLazhNh6d4`) and `sceAppInstUtilAppInstallTitleDir` (`Wudg3Xe3heE`) by firmware NID with a symbol-name fallback, instead of requiring the initialize/terminate C names that an NID-only system SPRX does not export. A failed probe, initialization or unload no longer latches a process-lifetime retry block, and an accepted registration with failed cleanup is reported as success-with-warning.
- First-run tile installation builds an owned `SCPS50001.partial` directory and renames it into place, so an interrupted install cannot leave an unclaimed title directory that later runs refuse. Owned or empty partial directories are recovered; foreign ones still fail safe.
- A virtual-device creation refused before `AddDevice` (bounded log/window unavailable or a rest/wake race) now clears its reservation instead of setting the registry's lifetime ambiguity latch, which previously disabled every later registration and suppressed input after logout recovery.
- A retired haptic OUT endpoint now restarts the owned USB session instead of leaving haptics permanently busy while advertised as available. The portal-selected haptic test method is reset on every new session, and dropped pulses (no owner or full queue) are counted and exposed as `drop` in the status JSON.
- A stale `<id>.ini.tmp` left by a crash no longer blocks profile saves: the store removes only a private owned regular temp and retries once, and fsyncs the directory after the atomic rename. The shared private-text writer performs the same stale-temp recovery.
- Shutdown is bounded: the main loop retries cleanup for at most 30 seconds before forcing credential restore and PID-lock removal; the DualSense hand-off, WebUI clients and first-run tile worker use completion waits instead of unbounded joins.
- The status log is rotated and opened only after this process owns the runtime PID lock, so a duplicate launch cannot destroy the live instance's diagnostics. It now uses the private no-follow directory/file policy.
- A held long-press binding no longer re-arms and fires again at a profile-generation boundary; a latched toggle keeps the output mask it latched while Shift changes.
- Bridge-probe output is a whitelisted private status file (no game `pid`/account `user_id`) instead of a raw copy of the PoorDS4 report.
- The sanitizer gate fails on undefined behaviour: `make -C tests sanitize` now builds with `-fno-sanitize-recover=all` and `UBSAN_OPTIONS=halt_on_error=1`.
- GitHub Actions are pinned to full commit SHAs, and stable `v*` tags must point at an ancestor of `main` before the release jobs may publish.

### New Features / Improvements

- Version `v1.5.0-experimental` across `version.h`, changelog, Payload Manager metadata and the DEV workflow default.
- Documented that in-game vibration/trigger feedback requires the opt-in `ENABLE_GAME_HOOKS=1` runtime policy; the default virtual-DualSense backend has no reverse feedback channel. The console UI now shows when hooks are unavailable instead of implying they will attach.
- Corrected architecture, tile, loading and security documentation for NID resolution, unreachable legacy adapter/dialog scaffolding, the portless literal `Host` form, private install locking and the lifetime kernel-credential scope.
- The Diagnostics section now renders the bounded report inline before attempting a download, because the PS5 browser does not reliably handle file downloads.
- Verified issue #10 against its attached FW13.60 GTA VI log: hooks were disabled, the virtual pad and interrupt-OUT haptics worked, and the missing in-game vibration is the documented opt-in feedback limitation rather than a transport defect. Startup now logs the disabled state.

## v1.0.0-experimental

Cumulative 1.0.0 Experimental build. Not a stable release; physical PS5 acceptance remains NOT_RUN and firmware 13.60 is the intended, unverified target.

### Bug Fixes

- The pause and DualSense-takeover release report keeps the documented neutral gravity vector `(0, 1, 0)` instead of reporting zero acceleration.
- Direct-HID input insertion now retries transient `EAGAIN`/`EBUSY`/`EINTR` instead of stopping the payload, and a brief controller link loss holds the owned virtual pad for three seconds before detaching it.
- Media-tile registration resolves `sceAppInstUtilAppInstallTitleDir` by the firmware NID `Wudg3Xe3heE` with a symbol-name fallback instead of relying on name lookup alone.
- Automatic tile installation skips only after a post-success marker and exact embedded-asset match, so a staged but unregistered directory is retried.
- Reject a loopback `Host` value whose explicit port is not the port the WebUI actually bound, so another local service cannot borrow the listener identity.
- Return an explicit error instead of a truncated PNG when a game icon is empty, unreadable or larger than the response buffer.
- Return `404` for unknown routes instead of reporting `405 Method Not Allowed`.

### New Features / Improvements

- Consolidated the project into one publication commit and one cumulative 1.0.0 Experimental release.
- Removed the unused pairing/token and peer-policy modules and the permanently forbidden `/pair` and `/api/pair-refresh` routes from the shipped payload; the modules remain host-test-only.
- Corrected architecture, security, test-plan, VDA/tester and audit documentation that still described bearer or pairing authentication as active.

## v1.0.0-experimental.2

Experimental hardening build. Not a stable release; physical PS5 acceptance remains NOT_RUN.

### Bug Fixes

- Reject a loopback `Host` value whose explicit port is not the port the WebUI actually bound, so another local service cannot borrow the listener identity.
- Return an explicit error instead of a truncated PNG when a game icon is empty, unreadable or larger than the response buffer.
- Return `404` for unknown routes instead of reporting `405 Method Not Allowed`.

### New Features / Improvements

- Removed the unused pairing/token and peer-policy modules and the permanently forbidden `/pair` and `/api/pair-refresh` routes from the shipped payload; the modules remain host-test-only.
- Corrected architecture, security, test-plan and tester documentation that still described bearer/pairing authentication as active.

## v1.0.0-experimental.1

Experimental consolidation build. It is not a stable release; physical PS5 acceptance remains NOT_RUN and firmware 13.60 is the intended, unverified target.

### Bug Fixes

- The pause and DualSense-takeover release report now keeps the documented neutral gravity vector `(0, 1, 0)` instead of reporting zero acceleration.

### New Features / Improvements

- Consolidated the project history into one publication commit and moved the experimental channel to the `v1.0.0-experimental.N` version series.

## v0.1.0-dev.9

Published 2026-11-15T10:18:02Z from `d6d51af1ce1d0cf14438fb73efae3cd2652b1d25`. PS5 runtime and automatic tile registration are unverified.

### Bug Fixes

- Removed the one-time pairing code and storage dependency that could prevent console WebUI access.

### New Features / Improvements

- Restricted WebUI access to the PS5 itself and added browser-origin protection.
- Added automatic first-run Media tile registration with conservative handling of existing files.

## v0.1.0-dev.8

Published 3 November 2026 from `1cf624f2ccaf869eecd8c29dd29e834529f8a3d9`. SDK v0.43; physical PS5 testing NOT_RUN. Direct HID remains opt-in with strict native-controller protection; see docs/development/DEV8_CANDIDATE.md.

### Bug Fixes

- Corrected acceleration/angular-velocity ordering in the native input structure and separated Create from the PS button (source-reviewed and host-tested; console pending).
- Prevented a continuously refilled haptic queue from monopolizing the input reader; pending effects remain ordered and rumble-stop processing continues (host-tested).

### New Features / Improvements

- Preserved left-stick touch and both raw touchpad pressures in the shared Steam input model, with source-derived boundary tests; no new PS5 output or editor control is claimed.
- Added an opt-in direct HID backend with explicit handle receipts, retained failed-deletion ownership, full touch/Create/motion translation and validation/neutralization of invalid reports (host-tested; hardware pending).
- Added rest/wake session quiescence and rediscovery after confirmed working-state observations (host-tested policy; hardware pending).
- Added a pinned comparison of controller projects and SDK versions, including direct-HID registration and rest/wake follow-up requirements.

## v0.1.0-dev.7

Published 1 November 2026 (UTC) from `f4ec9ae9e7e287c957a5602493a5b7d46449aed3`. [Release and downloads](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v0.1.0-dev.7). Contributor-reported results on a single FW 13.20/Puck console apply to the original PR #8 build; this revised release has no physical PS5 acceptance result.

### Bug Fixes

- Prevented WebUI request handling from overflowing the default worker stack by allocating an explicit bounded stack (host-tested; contributor reported the original fix on FW 13.20).
- Allowed virtual-controller startup to drain larger existing kernel-log backlogs safely and retain a full observation window before accepting a pending registration (host-tested; ownership and user assignment remain experimental).
- Corrected PS5 filesystem error handling for credentials, profiles and optional tile files, preserving exclusive creation and descriptor ownership during failures and file replacement (host-tested; revised adapter awaits console testing).

### New Features / Improvements

- Added regressions for target syscall arguments and error flags, descriptor exhaustion, concurrent file replacement and credential cleanup after failed initialization.

## v0.1.0-dev.6

Published 31 October 2026 (UTC; 1 November in Riga) from `4c8b89bc789bf1dfd064264ff432d3919b5a1899`. [Release and downloads](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v0.1.0-dev.6).

### Bug Fixes

- Fixed protected WebUI token initialization so a missing token is detected by the existing no-follow open, without a second metadata lookup; exclusive creation and validation remain in place (host-tested; target filesystem unverified).
- Improved pending VDA log setup with one fresh loopback retry when the selected `/dev/klog` reader returns `EILSEQ`; registration remains blocked if the fallback cannot drain safely (host-tested; PS5 behavior unverified).

### New Features / Improvements

None.

## v0.1.0-dev.5

Published on 2026-10-15 (UTC) from `382e4ecef4937356b72f9e1786917ae7147c0415`. Pending registration uses event correlation; the actual PS5 ABI and native-controller coexistence remain unverified.

### Bug Fixes

- Handled pending virtual-controller registration without repeating device creation or restarting USB input; cancelled registration retains cleanup ownership (host-tested; PS5 verification pending).
- Prevented reconnect from creating another virtual controller while earlier cleanup is unresolved (host-tested).
- Avoided unnecessary permission changes when the settings directory and new credential file already have the required private permissions (host-tested).
- Removed account identifiers from raw application logs and disabled the optional handoff log observer in Safe Mode (host-tested).
- Preserved the original storage error through cleanup and cleared partial credentials after failed initialization (host-tested).

### New Features / Improvements

- Added bounded registration diagnostics and precise storage-stage/errno reporting without exposing credentials or account IDs.
- Added protected session-only WebUI access when persistent credential storage fails; pairing and authentication remain required.

## v0.1.0-dev.4

Published on 2026-10-14 from `a5c20346392eac0eb9fa04cc201cd214e2957363`; physical PS5 testing was not performed.

### Bug Fixes

- Stopped repeated virtual-controller creation after an ambiguous result, including concurrent registration attempts (host-tested).
- Rechecked an opened USB device before claiming its interfaces after hotplug (host-tested; PS5 ioctl validation pending).
- Fixed an overflow while handling an invalid USB feedback status (UBSan-tested).

### New Features / Improvements

None.

## v0.1.0-dev.3

Published on 2026-10-12 from `9aab6dbae166b28416a9a00e6346f85bf12e524e`; physical PS5 testing was not performed.

### Bug Fixes

- Prevented cancelled USB requests from reusing buffers while late completions remain possible; preserved input received during haptic output (host-tested).
- Added serialized virtual-device creation and recovery that retains ownership when deletion fails (host-tested).

### New Features / Improvements

- Added virtual DualSense recovery after confirmed logout and stable login, keeping the Steam USB connection intact (host-tested; PS5 validation pending).
- Added authenticated diagnostic downloads with bounded, filtered application logs.
- Added clearer connection, initialization and recovery messages, plus Payload Manager metadata.
- Embedded the supplied icon and optional PS5 Settings tile installer in the existing single ELF; installation requires explicit confirmation.
- Made a two-section changelog mandatory for future release notes and packaging.

## v0.1.0-dev.2

Published from `6ed81c2ba477285e932bd4593aa029554cadf757`; physical PS5 testing was not performed.

### Bug Fixes

- Kept USB transfer buffers alive through cleanup, rejected invalid completed frames and added stale-input neutralization (host-tested).
- Made profile writes safer during concurrent edits and added explicit recovery for a corrupt Default profile.
- Hardened optional game-hook installation and rollback so failed cleanup retains ownership (host-tested).

### New Features / Improvements

- Enabled the authenticated WebUI on first launch, with a short-lived console pairing code and browser enrollment; manual FTP setup is no longer required.
- Expanded HTTP admission, storage, browser and release-evidence checks.

## v0.1.0-dev.1

First published experimental version, source `ffff5e08905d394b67a7da070fcf4657a250f070`; physical PS5 testing was not performed.

### Bug Fixes

None.

### New Features / Improvements

- Introduced the experimental single-ELF Steam Controller 2026 USB-C/Puck implementation, with virtual DualSense, profiles, motion, trackpads and haptic translation.
- Included an authenticated optional WebUI with image-based remapping and profile import/export.
- Added reproducible Host, sanitizer, Chromium and SDK v0.43 build gates, checksums, source references and safety documentation.
