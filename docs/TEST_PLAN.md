# Verification matrix and reproducible testing

Tests are evidence, not intentions. `project-memory/TEST_LOG.md` records executed commands, exits, and environments. The matrix below distinguishes complete host verification from partial tests, unrun target work, and research blockers.

Current portable targets are in `tests/Makefile`: `make -C tests test` and `make -C tests sanitize`. The latest suite passes 821 checks under ASan/UBSan: HTTP/auth/token storage 57, JSON writer 9, USB configuration parsing 19, Steam report decoding 190, mapping transforms 24, motion math 25, feedback serialization 20, profile parsing/concurrency 33, binding transitions/session isolation/reset epochs 49, atomic pause/assignment control 51, slot registry 123, slot lifecycle 85, USB resource tracking 66 and USB session operation fault injection 70. Separate ThreadSanitizer runs pass for the profile parser, binding-reset, pause/assignment-control, slot-registry concurrency, and slot-lifecycle startup/release concurrency tests. Assignment lease tests cover a 16,000-claim stress run; registry tests cover 8,000 reserve/activate/release claims; lifecycle tests cover four concurrent workers with 4,000 startup/release cycles plus create failure, worker-start failure, ambiguous VDA handles, lazy startup, and failed-delete recovery. USB session fakes inject open, FS-init, endpoint-open, transfer-start, and transfer-completion failures and verify cleanup ownership; these test the portable adapter contract, not the real PS5 ioctl implementation. Stable device identity, rebind across transports, profile persistence/browser integration, menu/input-thread integration, the PS5 adapter's neutral-publication branch, live USB/Puck I/O, and physical console behavior remain unverified. See `project-memory/TEST_LOG.md` for exact commands and environments.

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
| T-P14 | Shared control state | atomic pause access/exchange, concurrent invalidation | PARTIAL_HOST_VERIFIED — pause and assignment control functions are host-tested; portal/menu/manager integration and other shared telemetry remain target-unverified |
| T-P15 | Assignment-dialog lease | exclusive owner, generation uniqueness, stale release rejection, concurrent claim stress | PARTIAL_HOST_VERIFIED — portable lease logic passes 31 assertions and 16,000 claims under ThreadSanitizer; PS5 manager timeout and USB lifecycle integration remain cross-build-only |
| T-P16 | Slot registry/lifecycle | live-key uniqueness, four-slot capacity, lifecycle transitions, stale-generation rejection, unique VDA ownership, startup rollback, failed-delete recovery and concurrent reuse | PARTIAL_HOST_VERIFIED — 123 registry checks and 85 lifecycle checks pass under ASan/UBSan; targeted ThreadSanitizer passes both concurrency suites. The PS5 manager scan loop and USB ioctl adapter only cross-build; no physical hardware run |

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
| H-12 | Shutdown | Payload stops with all own hooks reverted and existing physical PS5 controller usable |

Record actual iterations and scenarios, not planned numbers. If no physical console: `NOT_RUN / BLOCKED_HARDWARE`; do not assert failure or pass.

## Security/ABI/build gates

- PS5 cross ELF build: use the pinned SDK v0.43 asset and verify its SHA-256, build the payload, record the ELF SHA-256, and inspect target dependencies/imports. M1-41 passes those compile/link checks for the USB session integration. This is build-only evidence and does not verify firmware/runtime or USB/Puck behavior; keep those hardware gates separate.
- Native radio: **no active HCI experiment** without approval and safe plan. Strong evidence required for LE scan, not Bluetooth UI visibility.
- Game hooks: per-game/process build fingerprint, rollback, basic input unaffected when hook disabled/fails.
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
