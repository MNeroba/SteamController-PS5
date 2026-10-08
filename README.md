# SteamController-PS5

A Steam Controller 2026 focused PS5 payload project. This repository uses the pinned [PuckbridgePS5](https://github.com/ThisIsAkill/PuckbridgePS5) source tree as its implementation foundation because it contains the Steam USB/Puck report path and PS5 virtual-controller integration. The selection rationale and limits are in [ADR-001](docs/BASE_SELECTION_ADR.md).

## Development status

The imported code contains Steam Controller report handling, profiles, motion, touchpad mapping, haptics, a virtual DualSense path, and optional game integration. These are source capabilities inherited from PuckbridgePS5; they have not yet been independently validated on PS5 hardware in this project. See [the test plan](docs/TEST_PLAN.md), [project memory](project-memory/PROJECT_MEMORY.md), and the hardware matrix.

USB-C and the official Puck are the first target. Native PS5 Bluetooth LE is research-only and has no compatibility claim. Its evidence gates are recorded in [BLE decision](docs/BLE_DECISION.md).

## Portal security

The portal listens on TCP port 8090. On first startup it generates a random token in `/data/ghostpad/webui.token` with file mode `0600` and secures `/data/ghostpad` to mode `0700`. The browser asks for this token before making API requests and keeps it in tab-scoped session storage. Retrieve the file through local console file access and do not post the token in logs or public issue reports. To rotate it, stop the payload, delete the token file, and restart.

The portal currently uses plain HTTP. Use it only on a trusted local network; bearer tokens can be observed by an active network intermediary. Do not expose port 8090 to the public internet.

Game-process input merging and game-driven vibration hooks are disabled by default. Enable them explicitly in the portal; the regular virtual DualSense path remains available without game hooks. The selected title change now requests removal of the prior game's hooks, but process-exit rollback and PS5 runtime behavior still require platform validation.

## Build and host tests

PS5 payload builds use the pinned `ps5-payload-sdk` v0.43 release and LLVM 18. See [the build guide](docs/BUILDING.md) for the verified asset digest, macOS setup, and exact commands. A successful cross-build confirms compilation and linking against the SDK; it does not confirm runtime or firmware compatibility.

```sh
export PS5_PAYLOAD_SDK=/path/to/ps5-payload-sdk
make -C payload
```

Portable protocol, descriptor, portal security/JSON, profile parsing, mapping, motion-math, and feedback-encoding tests run without the PS5 SDK:

```sh
make -C tests sanitize
```

The latest ASan/UBSan run passes 685 checks, including the 123-check slot-registry suite and 85 portable slot-lifecycle checks. ThreadSanitizer passes registry contention, lifecycle startup/rollback stress, and concurrent binding-reset, profile-parser, and pause/assignment-control suites. The payload cross-builds with SDK v0.43 and produces an x86-64 PS5 ELF on macOS. Host tests and cross-compilation do not establish live USB/Puck, motion, touch, haptics, game, or console compatibility; no physical console or Steam Controller/Puck is available here. Hardware validation remains blocked, not passed or failed.

## Source pins and attribution

- PuckbridgePS5 foundation: `78460113ef7c48502c765a1f83ba34d53c977134`.
- OmniPad-PS5 comparison reference: `b7682efce18ee0a8d7c80bbc90afbce72151ef96`; no OmniPad source is imported.
- The project retains PuckbridgePS5's GPL license, source notices, and PoorDS4 attribution. See [provenance](docs/PROVENANCE.md).
- The upstream README credits its inline portal controller drawing to Akhil Moola and declares the project GPL-3.0-or-later; it has no separate artwork notice. The handoff image `steam-controller.jpg` is not used by the payload and is excluded from Git.

This repository is distributed under GPL-3.0-or-later; see [LICENSE](LICENSE). The original upstream README is retained at [README.upstream.puckbridge.md](README.upstream.puckbridge.md), and the project starter README is retained at [docs/PROJECT_STARTER_README.md](docs/PROJECT_STARTER_README.md).
