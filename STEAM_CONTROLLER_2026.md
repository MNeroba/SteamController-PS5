# Steam Controller 2026: feature and evidence status

This page describes the Steam Controller 2026 work in **SteamController-PS5**. It is a feature-status page, **not an installation or compatibility guide**.

> **No physical PS5, Steam Controller, DualSense, or Valve Puck has been tested in this project.** Source code, host tests, a PS5 SDK cross-build, and a downloadable CI artifact do not establish that the payload loads or works on a console. There is no verified release. Read the [README](README.md) before building; do not treat this page as a recommendation to run the payload on a stock or primary console.

## Feature status

| Area | Implemented in source | Evidence available | Still unverified |
|---|---|---|---|
| **USB-C and official Puck** | Steam 2026 VID/PID allowlist, bounded configuration-descriptor parsing, interface selection, report decoding, and reader-owned USB session/resource cleanup | Portable parser, scanner, and cleanup tests; PS5 SDK cross-build | Real device descriptors, Prospero USB ioctl behavior, enumeration, reconnect, and controller input on-console |
| **Virtual DualSense and slots** | One default virtual DualSense path, slot registry, generation-aware lifecycle, owner-checked cleanup, and neutralization on shutdown | Slot/lifecycle and source-contract tests; PS5 SDK cross-build | `scePad` handle semantics, user assignment, duplicate prevention, and normal DualSense coexistence on hardware |
| **Buttons, sticks, triggers, and profiles** | Steam report model, mapping/binding and profile persistence/selection logic | Production-module host tests, sanitizer coverage, parser property tests | End-to-end profile selection and controller behavior on PS5 |
| **Trackpads, rear controls, and motion** | Extended Steam input state, trackpad/motion mapping modules, gyro aiming and flick-stick paths | Portable mapping/motion tests and concurrency checks | Real sensor units/orientation, calibration, PS5 touch/motion event semantics, and gameplay |
| **Haptics and trigger translation** | Bounded feedback encoding, a single-SC2 session lease, queue ownership, and trigger-effect-to-haptic logic | Host tests for encoding, ownership, bounds, and concurrency | USB output reports, game-hook routing, physical rumble, and effect quality on the Steam Controller |
| **Profiles and portal** | Profile import/export and persistence paths; bearer-token API policy, strict route/request parsing, deadlines, and local IPv4 peer policy | Host tests for production portable modules and storage concurrency | Prospero filesystem/socket behavior, browser integration, and on-console enrollment. Portal traffic is plain HTTP and should remain on a trusted network. |
| **Shutdown, privilege, and optional hooks** | Safe defaults, checked credential scopes, owned VDA cleanup, coordinated worker shutdown, and bounded hook rollback attempts | Fault-injection, source-contract, ASan/UBSan, and selected TSan host checks | Kernel/API semantics, remote-call latency, target-side leases, hook rollback, and DualSense recovery on PS5 |
| **Native PS5 Bluetooth LE** | No native PS5 BLE transport is implemented | Public-source/API and licensing review only; no radio operation was run | Every native PS5 BLE gate, starting with passive console diagnostics |
| **DualSense Edge / ShellUI identity** | Not part of the production path | Research notes only | Registration ABI, game-visible behavior, and reversible per-device UI identity |
| **Two SC2 controllers through one Puck** | Not supported; the runtime deliberately admits one active SC2 haptics owner | Host policy tests | A separate multi-controller architecture and hardware validation would be required |

## How to interpret the evidence

- `VERIFIED_CODE` means a source path exists; it does not mean the device or console works.
- `VERIFIED_HOST_TEST` means the named portable production modules passed the recorded host checks and fakes.
- `VERIFIED_PS5_BUILD` means the PS5-target ELF compiled and linked against the pinned community SDK.
- `VERIFIED_HARDWARE` requires an actual console, exact firmware, controller/receiver, transport, and recorded steps. **No feature currently has this evidence.**

The current readiness matrix and test records are in [README](README.md), the [test plan](docs/TEST_PLAN.md), [hardware matrix](project-memory/HARDWARE_MATRIX.md), and [test log](project-memory/TEST_LOG.md).

## Build outputs

The [PS5 cross-build workflow](https://github.com/MNeroba/SteamController-PS5/actions/workflows/ps5-cross-build.yml) uploads `ghost-control-ps5-cross-build` with an ELF and SHA-256 file after a successful build. Actions artifacts are temporary and expire under GitHub's retention policy. The repository is private, so access is required. An artifact is a build output, not an install package or a verified release.

Use the [build instructions](docs/BUILDING.md) for portable tests and the SDK cross-build. A development-console deploy target is documented in the README, but this project's deploy/runtime path has not been tested on hardware. There is no supported stock-console installation procedure.

## Current scope

The project is focused on one Steam Controller 2026 connected by USB-C or the official Valve Puck, feeding the normal virtual DualSense route. BLE, Edge emulation, ShellUI name/icon changes, and multi-controller use are not prerequisites for the USB/Puck implementation and must not be described as supported. The USB/Puck and extended-input paths remain development work until the matching hardware rows pass.
