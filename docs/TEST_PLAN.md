# Verification matrix and reproducible testing

Tests are evidence, not intentions. Codex must create/verify actual runnable commands in selected source tree and update `project-memory/TEST_LOG.md` with exit status and exact environment. Commands below are **targets/examples**, not claims that the current unmodified repo provides these targets.

Current portable targets are in `tests/Makefile`: `make -C tests test` and `make -C tests sanitize`. The latest suite passes 370 checks under ASan/UBSan: HTTP/auth/token storage 57, JSON writer 9, USB configuration parsing 19, Steam report decoding 190, mapping transforms 24, motion math 25, feedback serialization 20, and profile parsing 26. It covers malformed and overflowing profile integers but does not validate session/slot lifecycle, profile transitions, the PS5 adapter's neutral-publication branch, live USB/Puck I/O, or physical console behavior. See the latest entry in `project-memory/TEST_LOG.md` for environment and command.

## Host tests

| ID | Subsystem | Required test cases | Gate |
|---|---|---|---|
| T-P01 | HID/Steam decoder | report 0x42/0x45/0x47 variations, truncation 0..N bytes, oversized, malformed ID, valid vectors, endian/sign | HOST_VERIFIED |
| T-P02 | HCI/L2CAP parser | length shorter than header, len==8, claims > received, invalid CID, ACL-handle-to-device index mismatch | HOST_VERIFIED |
| T-P03 | Puck USB | multiple HID interfaces, dock vs game input usage, wrong PID, conflicting receiver, wake/sleep, no double attach | HOST_VERIFIED |
| T-P04 | Session | lizard mode state, heartbeat timing, keepalive errors, reconnect, state neutralization | HOST_VERIFIED |
| T-P05 | Slot manager | duplicate same device across transports, atomic attach, stale generation, conflicts, rebind, multiple users | HOST_VERIFIED |
| T-P06 | Mapping | hold/tap/double, toggle, chord, action layers, turbo rate, profile change during hold, invalid profile | HOST_VERIFIED |
| T-P07 | Motion/touch | 3 axes, units, calibration, mirrored coordinates, missing contact, invalid reports, timestamp drift | HOST_VERIFIED |
| T-P08 | Feedback | output report validation, haptics bounds, cancellation, disconnect, device sleep, feedback routing | HOST_VERIFIED |
| T-P09 | USB lifecycle | failure-injected pthread_create, USB read/write fail, rapid hotplug, double cleanup, blocked readers | HOST_VERIFIED |
| T-P10 | HTTP/API | auth required, unauthorized mutation denied, bad tokens/Origin, invalid slots, escaping, overflow, partial reads/writes | HOST_VERIFIED |
| T-P11 | Debug transport | disabled by default, authorized opt-in, exact framing, partial TCP, occupied slot cannot be stolen | HOST_VERIFIED |
| T-P12 | BLE decoders | LE events, ATT/GATT notifications, reconnect, truncated characteristic, reconstructed report ID, negative SMP tests | HOST_VERIFIED |
| T-P13 | Persistence | version migration, atomic write/recovery, invalid paths, truncated files, profile undo/export/import | HOST_VERIFIED |

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

- PS5 cross ELF build: actual SDK installed, exact binary SHA256, dynamic symbol availability inspected, no proprietary objects included. `PS5_BUILD_VERIFIED` only after success.
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
