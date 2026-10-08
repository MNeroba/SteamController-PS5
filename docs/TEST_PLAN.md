# Verification matrix and reproducible testing

Tests are evidence, not intentions. `project-memory/TEST_LOG.md` records executed commands, exits, and environments. The matrix below distinguishes complete host verification from partial tests, unrun target work, and research blockers.

Current portable targets are in `tests/Makefile`: `make -C tests test` and `make -C tests sanitize`. The latest suite passes 873 counted assertions under ASan/UBSan: HTTP/auth/token storage 57, JSON writer 9, USB configuration parsing 19, Steam report decoding 190, mapping transforms 24, motion math 25, feedback serialization 20, profile parsing/concurrency 33, binding transitions/session isolation/reset epochs 49, atomic pause/assignment control 51, slot registry 123, slot lifecycle 85, USB resource tracking 66, USB session operation fault injection 70, game-hook safety policy 16, per-session flick isolation 13, trigger-effect isolation 11 and haptic service routing 12. Seven stress targets pass 340,000 concurrent operations/snapshots, including hand-off (60,000), telemetry (100,000), game hooks (61,000), gyro (75,000), flick (20,000), trigger effects (20,000), and haptic service (4,000). The parser property target adds 100,000 deterministic randomized Steam report inputs and 100,000 USB configuration inputs under ASan/UBSan; this is bounded coverage, not exhaustive fuzzing. The SC2 endpoint-scanner regression also runs two concurrent ioctl-stubbed scans and verifies that each classifies only its own report. Targeted ThreadSanitizer passes concurrency targets, as well as profile parsing, binding reset, pause/assignment control, slot registry, and slot lifecycle. Assignment lease tests cover 16,000 claims; registry tests cover 8,000 reserve/activate/release claims; lifecycle tests cover four concurrent workers with 4,000 startup/release cycles plus create failure, worker-start failure, ambiguous VDA handles, lazy startup, and failed-delete recovery. USB session fakes inject open, FS-init, endpoint-open, transfer-start, and transfer-completion failures and verify cleanup ownership; these test the portable adapter contract, not the real PS5 ioctl implementation. Game-hook safety tests exercise restore decisions and heartbeat-lease expiry, not remote memory operations. Stable device identity, rebind across transports, profile persistence/browser integration, menu/input-thread integration, the PS5 adapter's neutral-publication branch, live USB/Puck I/O, and physical console behavior remain unverified. See `project-memory/TEST_LOG.md` for exact commands and environments.

## Host tests

| ID | Subsystem | Required test cases | Gate |
|---|---|---|---|
| T-P01 | HID/Steam decoder | report 0x42/0x45 variants, truncation, malformed IDs, signed fields and decoded controls | PARTIAL_HOST_VERIFIED |
| T-P02 | HCI/L2CAP parser | length/header bounds, invalid CID, ACL-handle/device-index mismatch | NOT_IMPLEMENTED — no HCI/L2CAP code is in the selected USB/Puck release path |
| T-P03 | Puck USB | configuration bounds and interface filters | PARTIAL_HOST_VERIFIED — no live HID usage/report-descriptor validation |
| T-P04 | Session | lizard mode, heartbeat, keepalive errors, reconnect, state neutralization | NOT_RUN — target session remains in inherited PS5 integration |
| T-P05 | Slot manager | duplicate live key, capacity, reserve/activate/release, generation staleness, virtual-handle uniqueness, startup rollback, deferred deletion and concurrent claims | PARTIAL_HOST_VERIFIED — registry and shared lifecycle tests pass under ASan/UBSan and ThreadSanitizer; manager scan-loop scheduling, stable physical identity, rebind and multi-user behavior remain unverified |
| T-P06 | Mapping/bindings | mapping bounds, toggles, long press, initial neutral frame, profile-generation reset, asynchronous invalidation, per-session binding state | PARTIAL_HOST_VERIFIED — chords, action layers, turbo timing and full adapter profile switching remain untested |
| T-P07 | Motion/touch | unit conversion, quaternion input, coordinate mapping and contact states | PARTIAL_HOST_VERIFIED — physical calibration, sensor traces and PS5 touch behavior remain untested |
| T-P08 | Feedback | report serialization and haptic bounds | PARTIAL_HOST_VERIFIED — output routing, cancellation, disconnect and physical feedback remain untested |
| T-P09 | USB lifecycle | thread creation failure, I/O failure, rapid hotplug, cleanup and blocked readers | PARTIAL_HOST_VERIFIED — `usb_session` fakes inject open/init/endpoint-open/start/complete failures and verify cleanup; the real `gc_main.c` ioctl callbacks only cross-build. Actual `pthread_create`, manager scan behavior, rapid hotplug and blocked kernel readers still need target-side injection or hardware |
| T-P10 | HTTP/API | auth policy, tokens, framing, escaping, overflow, partial requests | PARTIAL_HOST_VERIFIED — Origin/CSRF policy and live server integration remain untested |
| T-P11 | Debug transport | anonymous injection disabled or authenticated | NOT_APPLICABLE — selected release path has no raw TCP gamepad-input transport |
| T-P12 | BLE decoders | LE events, ATT/GATT notifications, truncated characteristic and negative SMP | BLOCKED_RESEARCH — BLE remains gated; no native PS5 BLE decoder/transport is implemented |
| T-P13 | Persistence | migration, atomic write/recovery, path safety, import/export | NOT_RUN — no end-to-end storage/browser integration test exists |
| T-P14 | Shared control state | atomic pause access/exchange, concurrent invalidation, coherent telemetry snapshots | PARTIAL_HOST_VERIFIED — pause/assignment controls and the new input/battery/haptic status snapshot pass host stress; portal/menu/manager integration remains target-unverified |
| T-P15 | Assignment-dialog lease | exclusive owner, generation uniqueness, stale release rejection, concurrent claim stress | PARTIAL_HOST_VERIFIED — portable lease logic passes 31 assertions and 16,000 claims under ThreadSanitizer; PS5 manager timeout and USB lifecycle integration remain cross-build-only |
| T-P16 | Slot registry/lifecycle | live-key uniqueness, four-slot capacity, lifecycle transitions, stale-generation rejection, unique VDA ownership, startup rollback, failed-delete recovery and concurrent reuse | PARTIAL_HOST_VERIFIED — 123 registry checks and 85 lifecycle checks pass under ASan/UBSan; targeted ThreadSanitizer passes both concurrency suites. The PS5 manager scan loop and USB ioctl adapter only cross-build; no physical hardware run |
| T-P17 | Game-hook rollback/lease | restore only our gateway, skip already-restored slots, preserve later owners, retry failed removals, expire audio silence after producer stop | PARTIAL_HOST_VERIFIED — 16 pure policy/heartbeat checks pass under ASan/UBSan and the PS5 SDK cross-build passes. Remote page reads/writes, races with another hook, process exit and payload stop need a real target or injected adapter |
| T-P18 | DualSense hand-off state | concurrent enable/session/release/JSON access | PARTIAL_HOST_VERIFIED — after fixing three ThreadSanitizer reports for unlocked `g_enabled` and `g_release`, 60,000 concurrent snapshots pass without reports; real MBus and UI behavior remain hardware-blocked |
| T-P19 | Runtime telemetry | coherent input/connection, battery, haptic configuration and counters under concurrent readers/writers | PARTIAL_HOST_VERIFIED — 100,000 coherent snapshots pass ASan/UBSan and ThreadSanitizer; integration semantics with simultaneous Steam Controller sessions remain unverified |
| T-P20 | Game-hook shared state | concurrent configuration updates, active/owner snapshots, pad feed, JSON reads and worker install/read loop | PARTIAL_HOST_VERIFIED — 61,000 concurrent operations pass ASan/UBSan and ThreadSanitizer against `game_hooks.c` with mocked PS5/game bridge calls; remote-memory ownership and real process lifecycle remain target-unverified |
| T-P21 | Gyro shared state | concurrent IMU apply/motion, button edge, reset, moving flag and status reads | PARTIAL_HOST_VERIFIED — 75,000 operations pass ASan/UBSan and ThreadSanitizer against `sc2_gyro.c`; USB feature ioctl and physical IMU behavior remain unverified |
| T-P22 | Steam per-session input context | independent flick accumulator, frame clock, QAM long-hold, and menu action-edge state; reset lifecycle; two concurrent report streams | PARTIAL_HOST_VERIFIED — 13 assertions and 20,000 concurrent report calls pass ASan/UBSan and ThreadSanitizer. The test executes `sc2_flick.c`; USB packet adapter integration and other process-level menu/gyro/haptic/profile semantics remain unverified |
| T-P23 | Adaptive-trigger per-session runtime | independent threshold crossing history and vibration expiry for two readers; concurrent reports; state reset when disabled or cleared | PARTIAL_HOST_VERIFIED — 11 assertions and 20,000 concurrent report calls pass ASan/UBSan and ThreadSanitizer. PS5 trigger-effect hook routing, physical Steam Controller haptics, and the shared output queue remain hardware/target-unverified |
| T-P24 | Haptic USB service concurrency | concurrent service calls with different extra-OUT counts, count bounds, queue access and logging state | PARTIAL_HOST_VERIFIED — production haptic service is host-compiled with only the USB sender stubbed; 12 assertions and 4,000 concurrent service reports pass ASan/UBSan and ThreadSanitizer. No PS5 ioctl, physical haptic, session-specific queue routing or device teardown was exercised |
| T-P25 | Steam USB/report parser robustness | deterministic randomized valid, truncated and malformed packets/descriptors; documented return ranges; report-size and Puck-interface output invariants | PARTIAL_HOST_VERIFIED — fixed-seed property test checks 100,000 Steam reports plus 100,000 USB configuration inputs under ASan/UBSan. This is bounded deterministic sampling, not exhaustive fuzzing or a live descriptor test |
| T-P26 | Concurrent SC2 USB endpoint discovery | simultaneous Puck and wired endpoint scans keep report buffers independent and classify only each call's completion | PARTIAL_HOST_VERIFIED — deterministic host ioctl stubs force overlapping report writes; the regression failed with shared static storage and passes with per-call buffers under normal, sanitizer, and ThreadSanitizer builds. Actual PS5 USB ioctls and device reports remain untested |

Gate labels describe the cases actually executed, not the intended target coverage. `PARTIAL_HOST_VERIFIED` means the pure module has host checks but at least one listed integration or edge case is still open.

### Host build / CI

- Use existing native build/test targets after inspection; repair missing Makefile referenced tests first where applicable.
- Enable strict C warnings and ASan/UBSan in an isolated host-test build. Use fuzz/property testing for parsers and HTTP/profile inputs when useful and reproducible.
- A sanitizer build passing does **not** validate PS5-only memory/hook behavior. No crossbuild fake shims substituted for runtime result.
- Store compiler flags, SDK commit, OS/arch, command line, test count, exit code and log or GitHub Actions URL.

## Hardware test IDs

| ID | Transport / feature | Required observed behavior |
|---|---|---|
| H-01 | Baseline PS5 | Model, FW 13.60, loader, native DualSense works before payload |
| H-02 | USB | Actual wired Steam Controller connects, pads/sticks/triggers and PS/Home visible in UI and game |
| H-03 | Puck | Official receiver recognized via correct HID interface and actual SC state received |
| H-04 | Slot | User profile correct; no duplicate virtual pad; native DualSense still works |
| H-05 | Recovery | unplug/replug, power-off, standby, reconnect, stop/restart, no stuck input |
| H-06 | Motion | Verified tilt/gyro behavior in a named game and profile with observed axis direction |
| H-07 | Touch | Contact/coordinates/clicks in named game or verifiable system screen |
| H-08 | Feedback | Actual rumble/haptics, trigger-click translation in named game; verified fallback |
| H-09 | Security | Remote unauthenticated calls do not mutate pad/user state; portal authenticated |
| H-10 | BLE | Per `docs/BLE_FEASIBILITY.md` B1-B6; stock DualSense unaffected |
| H-11 | Multi-instance | Two physical SCs/pucks/slots only when explicitly implemented; no claim from one-controller handoff |
| H-12 | Shutdown | Stop/restart payload while a game is active; verify native input remains usable, merged input neutralizes, audio mute expires, and no foreign hook is overwritten |

Record actual iterations and scenarios, not planned numbers. If no physical console: `NOT_RUN / BLOCKED_HARDWARE`; do not assert failure or pass.

## Security/ABI/build gates

- PS5 cross ELF build: use the pinned SDK v0.43 asset and verify its SHA-256, build the payload, record the ELF SHA-256, and inspect target dependencies/imports. M1-41 passes those compile/link checks for the USB session integration. This is build-only evidence and does not verify firmware/runtime or USB/Puck behavior; keep those hardware gates separate.
- Native radio: **no active HCI experiment** without approval and safe plan. Strong evidence required for LE scan, not Bluetooth UI visibility.
- Game hooks: per-game/process build fingerprint, slot-owner checked rollback, liveness lease expiry, and basic input unaffected when hook disabled/fails. Current removal checks are not remote atomic compare-and-swap; require hardware validation before claiming race-free rollback.
- Licenses: scanned code and assets, required notices, no BTstack copying without review, no official NDA SDK redistribution.
- PR: `git diff --check`, host tests, sanitizer report, SDK build result/hardware status, scope/attribution and rollback in description.

## Template for logs

```
- Test ID:
  Date (UTC):
  Git commit / branch:
  Environment / SDK / FW / hardware:
  Exact command or steps:
  Input fixture checksum:
  Expected:
  Actual:
  Exit code / logs:
  Result: PASS | FAIL | NOT_RUN | BLOCKED
  Evidence link / log filename:
  New issue or follow-up:
```
