# Steam Controller 2026: feature and evidence status

This page describes the Steam Controller 2026 work in **SteamController-PS5**. It is a feature-status page, **not an installation or compatibility guide**.

> **No controlled physical PS5 acceptance tests have been performed by this project.** External dev.4 logs show Puck reports but failed virtual-controller registration and token setup; they do not verify dev.5 or establish compatibility. The owner reports a PS5 on firmware 13.60 (upgraded from 12.00 on 2026-10-11), which is the target and has not been compatibility-tested. Source code, host tests, a PS5 SDK cross-build, and a downloadable CI artifact do not establish that the payload loads or works on a console. There is no hardware-verified stable release. Experimental DEV prereleases are software/build deliverables. Read the [README](README.md) before building; do not treat this page as a recommendation to run the payload on a stock or primary console.

## Feature status

| Area | Implemented in source | Evidence available | Still unverified |
|---|---|---|---|
| **USB-C and official Puck** | Steam 2026 VID/PID allowlist, bounded configuration-descriptor parsing, interface selection, report decoding, and reader-owned USB session/resource cleanup, session-owned frames, checked completion status, input expiration and idle-Puck lease isolation | Portable parser, scanner, and cleanup tests; PS5 SDK cross-build | Real device descriptors, Prospero USB ioctl behavior, enumeration, reconnect, and controller input on-console |
| **Virtual DualSense and slots** | One default virtual DualSense path, slot registry, generation-aware lifecycle, pending event correlation, retained cleanup ownership, and neutralization on shutdown | Slot/lifecycle and source-contract tests; PS5 SDK cross-build | `scePad` handle semantics, user assignment, duplicate prevention, and normal DualSense coexistence and correlated ownership on hardware |
| **Buttons, sticks, triggers, and profiles** | Steam report model, mapping/binding and profile persistence/selection logic | Production-module host tests, sanitizer coverage, parser property tests | End-to-end profile selection and controller behavior on PS5 |
| **Trackpads, rear controls, and motion** | Extended Steam input state, trackpad/motion mapping modules, gyro aiming and flick-stick paths | Portable mapping/motion tests and concurrency checks | Real sensor units/orientation, calibration, PS5 touch/motion event semantics, and gameplay |
| **Haptics and trigger translation** | Bounded feedback encoding, a single-SC2 session lease, queue ownership, and trigger-effect-to-haptic logic | Host tests for encoding, ownership, bounds, and concurrency | USB output reports, game-hook routing, physical rumble, and effect quality on the Steam Controller |
| **Profiles and portal** | Atomic profile patches, explicit backed-up Default recovery, first-launch notification pairing, bearer-protected APIs/icons, strict admission, four bounded clients, local IPv4 peer policy and protected session-only fallback after eligible credential-storage failures | Production-module host/concurrency tests plus 220 image-based remap/browser checks, including hostile imports, canonical Save readback, concurrent edits and geometric touch selection on the loopback fixture | Prospero filesystem/socket behavior, real-console browser integration, and on-console enrollment. Portal traffic is plain HTTP and should remain on a trusted network. |
| **Shutdown, privilege, and optional hooks** | Safe defaults, checked credential scopes, owned VDA cleanup, coordinated worker shutdown, transactional optional hook installation, refused retained-page reuse, and bounded rollback attempts | Fault-injection, source-contract, ASan/UBSan, and selected TSan host checks | Kernel/API semantics, remote-call latency, target-side leases, hook rollback, and DualSense recovery on PS5 |
| **Optional PS5 settings tile** | One ELF embeds owner-supplied icon and fixed SCPS50001 Media/deeplink metadata; installation requires authenticated explicit confirmation, owned exact files and exact-title registration | Portable transaction/native-stub tests, production HTTP protection, browser confirmation/retry and SDKv0.43 build; [setup/removal](docs/development/SETTINGS_TILE.md) | H-13: measured FW13.60 ABI, visibility, browser loopback, enrollment/navigation and console Delete; tile does not autostart payload |
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

**Published experimental dev.5:** [dev/v0.1.0-dev.5](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v0.1.0-dev.5), source `382e4ecef4937356b72f9e1786917ae7147c0415`. [DEV workflow38810046681](https://github.com/MNeroba/SteamController-PS5/actions/runs/38810046681) completed successfully; release414294352 is a prerelease, not a draft, published **2026-10-15 21:30:53 UTC** (16 October in Riga). Seven assets contain exactly one ELF, **2,084,968 bytes**, SHA-256 `01a73f3f676121061f58b52a833c642d8cd94b048fbbc524182b8756eb642bd0`. Host48 C/Python, ASan/UBSan, Chromium220, deterministic assets and clean SDKv0.43/LLVM18 gates passed on that selected source. Physical Add/VDI/MBus, native DualSense coexistence, firmware compatibility and actual storage errno remain unverified. Event correlation is an owner-approved temporary experimental mechanism, not proof of exclusive ownership. Previous releases/tags/assets are unchanged.

The [PS5 cross-build workflow](https://github.com/MNeroba/SteamController-PS5/actions/workflows/ps5-cross-build.yml) uploads `SteamController-PS5-cross-build` with an ELF and SHA-256 file after a successful build. Actions artifacts are temporary and expire under GitHub's retention policy. Authorized access is required while the repository is private; this restriction disappears for public downloads after visibility changes. An artifact is a build output, not an install package or a verified release.

Use the [build instructions](docs/BUILDING.md) for portable tests and the SDK cross-build. The ELF loading procedure is documented in docs/development/LOADING.md, but this project's deploy/runtime path has not been tested on hardware. There is no supported stock-console installation procedure.

## Current scope

The project is focused on one Steam Controller 2026 connected by USB-C or the official Valve Puck, feeding the normal virtual DualSense route. BLE, Edge emulation, ShellUI name/icon changes, and multi-controller use are not prerequisites for the USB/Puck implementation and must not be described as supported. The USB/Puck and extended-input paths remain development work until the matching hardware rows pass.

## Binding, persistence and navigation audit — M1-220 (2026-10-10)

`VERIFIED_HOST_TEST`: full normal and ASan/UBSan suites PASS; profile 52 assertions, atomic storage 71, binding/proportional-pressure 165 and HTTP/auth 110. Nine portable artwork/generator contracts and 181 Edge/Playwright browser assertions PASS (all original 162 retained). The maximum-complexity stored profile is 14,573 bytes; complete 24-input/22-output/activator round-trip succeeds within the 24 KiB capacity. Final generated page: 685,503 UTF-8 bytes, 41 icons/four unchanged controller images. Clean SDK v0.43/LLVM 18.1.8 cross-build PASS, ELF SHA-256 `5bdd18153301213e7a4b82e3038dff24563fdc7e0e474ce41546d3c66ffc7dbc`. This is local source/build evidence; publication and Actions acknowledgement follow separately. Physical FW 12.00 remains NOT_RUN.


## Features introduced in dev.3, retained in dev.5

| Capability | Software status | Physical PS5 status |
|---|---|---|
| Owned virtual-pad logout/relogin recovery; USB preserved | Host consensus/lifecycle/race tests | NOT_RUN H-14 |
| IN/OUT ordering, late-cancel retirement and watchdog | Host fault/concurrency tests | NOT_RUN H-15 |
| Authenticated diagnostic text export, bounded/redacted log | Host/report/HTTP/Chromium tests | NOT_RUN H-15 |
| Payload Manager sidecar and mandatory versioned changelog | Packaging/format tests | Published metadata/checksum; manager installation unverified |
| Embedded optional Settings tile, one ELF | Existing host/native-stub/build tests retained | NOT_RUN H-13 |

No automatic tile installation/account reassignment, new controller protocols or storage migration. SDKv0.43, /data/ghostpad, artwork, pairing/profile/motion/touch/haptic contracts and safe defaults retained.
