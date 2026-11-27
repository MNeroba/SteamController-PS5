<div align="center">

# SteamController-PS5

**Steam Controller 2026 → virtual DualSense on PS5**<br>
USB-C & Valve Puck · visual remapping · gyro · trackpads · haptics

[Download 1.6.1 Experimental](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v1.6.1-experimental) · [Get started](docs/development/LOADING.md) · [Build](docs/BUILDING.md) · [Changelog](CHANGELOG.md) · [Contribute](CONTRIBUTING.md)

[![Host tests](https://github.com/MNeroba/SteamController-PS5/actions/workflows/host-tests.yml/badge.svg)](https://github.com/MNeroba/SteamController-PS5/actions/workflows/host-tests.yml)
[![PS5 build](https://github.com/MNeroba/SteamController-PS5/actions/workflows/ps5-cross-build.yml/badge.svg)](https://github.com/MNeroba/SteamController-PS5/actions/workflows/ps5-cross-build.yml)

![Image-based controller remapping in the host preview](docs/images/remap-desktop.png)

</div>

> **Experimental prerelease.** Host tests and SDK builds pass; this release has **no maintainer-run physical PS5 acceptance result**. FW **13.60** is the intended, unverified test target. A stock console cannot load this ELF. Use an independently confirmed compatible development loader and keep your native DualSense available.

> **1.6.1 Experimental:** the WebUI quality release plus runtime policy switches for DualSense hand-off and the game-bridge check, and HTTP-level mutation tests. The save bar is inert until a profile is loaded, mutation failures are reported and reverted, polling backs off offline and pauses while hidden, icon loading is sequential, the remap sheet keeps keyboard focus and marks the background inert, and the unload guard covers staged edits. DualSense hand-off and the game-bridge check now say when their runtime-policy flags are off instead of appearing functional. It also accepts the full five-key game-hook query, enlarges the status JSON, guards non-finite gyro values and opens log/icon files with `O_NOFOLLOW`. Hand-off and bridge can now be enabled from the page instead of editing runtime-policy by hand. Physical console behavior remains unverified.

> **External hardware evidence:** [TheBrokenG0d reports](https://github.com/MNeroba/SteamController-PS5/pull/8) WebUI and Puck input in system menus on one fat PS5/FW 13.20 at original PR source `c3a7ddb`. That build predates the revised filesystem adapter in dev.7; native DualSense coexistence and games were not systematically tested. It does not establish compatibility for this selected ELF or FW 13.60. Please [report reproducible results](.github/ISSUE_TEMPLATE/bug_report.yml) without tokens, account identifiers or serials.

**Controller transport:** [direct HID, rest/wake, safety limits and hardware test plan](docs/development/DEV8_CANDIDATE.md).

## Overview

One native ELF connects **one Steam Controller 2026** over USB-C or the official Valve Puck and translates its input into a virtual DualSense. The embedded WebUI lets you edit profiles and mappings without installing a separate web server or copying image assets.

The project builds on audited PuckbridgePS5 code. Steam-specific input stays in a reusable core; USB ownership, virtual-controller lifecycle and firmware-sensitive operations have separate responsibilities.

## Latest release — v1.6.1-experimental

[Release notes and downloads](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v1.6.1-experimental) · [Loading guide](docs/development/LOADING.md) · [Audit report](docs/development/MECHANISM_AUDIT_2026-11-27.md)

- **[Download the only ELF](https://github.com/MNeroba/SteamController-PS5/releases/download/dev/v1.6.1-experimental/SteamController-PS5-v1.6.1-experimental.elf)** — 2,124,944 bytes.
- **[SHA-256 file](https://github.com/MNeroba/SteamController-PS5/releases/download/dev/v1.6.1-experimental/SteamController-PS5-v1.6.1-experimental.elf.sha256)** — `33834d70f26d275a92c9a0e89eeb307559dd907a1209bb6e436d58a6069b52ab`.
- Payload Manager metadata, loading instructions, notices and license accompany the ELF.

Published 2026-11-27T22:53:06Z from [`c02977510aabc57fd081c0b28a0950447350b4c2`](https://github.com/MNeroba/SteamController-PS5/tree/c02977510aabc57fd081c0b28a0950447350b4c2) by [release workflow 46078999218](https://github.com/MNeroba/SteamController-PS5/actions/runs/46078999218). Seven assets contain one ELF; the downloaded digest, embedded source SHA/version and metadata were independently verified. The workflow repeated the normal host, ASan/UBSan, Chromium, deterministic-asset, SDK v0.43 and checksum gates. Physical PS5 acceptance remains outstanding.

Previous experimental tags remain published: `dev/v1.6.0-experimental` (`731138d`, WebUI hardening), `dev/v1.5.2-experimental` (`ef6e2ae`, game-hook checkbox), `dev/v1.5.1-experimental` (`d4b3a36`, PS4-style rumble), `dev/v1.5.0-experimental` (`65ba503`) and `dev/v1.0.0-experimental` (`f5208ad`).

## Features and verification

| Capability | Software evidence | PS5 hardware status |
| --- | --- | --- |
| USB-C and official Valve Puck | Checked device/interface selection, protocol parsing, transfer ownership and cleanup tests | Not tested |
| Virtual DualSense | Generation-aware lifecycle, default pending correlation, opt-in direct receipts with native/user checks, cleanup and logout/login regressions | Creation ABI, event ownership and user assignment unverified |
| Rest/wake recovery | Read-only power observation, owned cleanup, worker join and fresh-session consensus tests | Active-game suspend/resume and timing unverified |
| Buttons, sticks, triggers and four back buttons | Remapping, Shift, Long/Double press, Turbo/Toggle and profile tests | Gameplay unverified |
| Gyro, motion, trackpads and Flick Stick | Portable mapping, calibration and motion tests | Sensor orientation, feel and game behavior unverified |
| Haptics and trigger-effect translation | Feedback encoding, queue/ownership/recovery tests, a dropped-pulse counter and PS4-style remote-setting decoding (v1.5.1) | **Classic PS4 rumble** is read from the virtual pad and forwarded without hooks (v1.5.1). **PS5-native haptics** become DualSense haptic audio inside the system and need the Experimental game-hook toggle. Manual WebUI haptics work. Output routing and physical feedback unverified |
| Default/per-game profiles and visual editor | Atomic persistence, import/export and browser tests | Console storage and live integration unverified |
| Diagnostics | Loopback-only, bounded text export with filtered application logs | PS5/browser download unverified |
| Automatic Media tile | Embedded icon, owned atomic first install, firmware-NID resolution and retry-safe installer tests (v1.5) | Registration and launch unverified; FW 13.60 still NOT_RUN |

**Scope:** one active Steam Controller 2026, regular virtual DualSense. Native PS5 Bluetooth LE, Edge emulation, custom ShellUI controller identity and multiple active Steam Controllers remain research or unsupported. USB/Puck is independent of those research tracks. Haptic translation does not provide physical adaptive-trigger resistance. Classic PS4-style rumble is forwarded from the virtual pad since v1.5.1; PS5-native haptics remain opt-in and require the game-hook runtime policy described above. A manual haptics test succeeding does not establish in-game feedback.

[Detailed feature status](STEAM_CONTROLLER_2026.md) · [Hardware matrix](project-memory/HARDWARE_MATRIX.md) · [Test plan](docs/TEST_PLAN.md)

## Load and configure

The current 1.6.1 Experimental release is a single experimental ELF. Load it with an independently confirmed compatible loader. On first startup a background worker attempts to register **Steam Controller Settings** in PS5 Media; open that tile while the payload is running. It opens `http://127.0.0.1:8090/` without a pairing code. The WebUI is available only on the PS5, not from a PC, phone or LAN. Existing title paths are never overwritten automatically. If registration fails, consult the [loading guide](docs/development/LOADING.md) and use an existing PS5 browser to inspect the failure and retry.

Loopback Host and Origin checks protect browser requests. Other software running on the console with loopback access can control the WebUI. Registration, browser behavior and FW13.60 are physically unverified. The tile does not start the payload after a reboot. `ENABLE_WEBUI=0` disables both WebUI and its first-run tile attempt; hooks, bridge probing and DualSense handoff stay disabled by default.

### Troubleshooting

In **Settings & help → Diagnostics**, select **Download Diagnostic Report**. The report is also shown inside the page (v1.5), because the PS5 browser does not reliably handle file downloads; select and copy the text if no download starts. It includes build identity, runtime firmware when available, startup stages, USB/virtual-controller state and a bounded filtered log tail, and remains useful with no controller connected.

**Controller works but a game does not vibrate:** classic PS4-style rumble is forwarded automatically (v1.5.1) when the virtual pad exposes it. PS5-native haptics (including GTA VI) are converted by the system into DualSense haptic audio and never appear as motor values. To capture them, open the WebUI → **In games**, turn on **Enable game hooks (Experimental)**, accept the confirmation, enable the vibration/audio switches, and start or restart the game. The checkbox writes and starts the protected runtime policy for you; no manual file editing is needed. It stays off by default and can be turned off at any time. The WebUI **Vibration test** checks the controller's own haptics independently; a passing test does not make game feedback work.

Review the report before sharing it: profile names or USB paths may need further redaction. Never post credentials from older releases, console identifiers or bonding keys. Use the [bug-report template](https://github.com/MNeroba/SteamController-PS5/issues/new?template=bug_report.yml) for ordinary failures and [SECURITY.md](SECURITY.md) for vulnerabilities.

## Visual remapping

Select a Steam input, choose DualSense outputs, and stage changes before saving. Front/back views cover stick clicks, grip buttons and trackpad clicks. In the back view, physical **L is on the right**, **R on the left**.

| Back controls | Binding editor |
| --- | --- |
| ![Back mappings](docs/images/remap-desktop-back.png) | ![Binding editor](docs/images/remap-editor.png) |

These screenshots come from the real **host fixture**, not a PS5 session; they illustrate remapping rather than current release Settings screens. [Editor behavior and verification →](docs/WEBUI_REMAP.md)

## Build from source

Requirements: Python 3, make, host Clang, **LLVM 18** and community **PS5 Payload SDK v0.43**. Node 24 and Playwright are needed only for browser checks. No proprietary Sony SDK is used.

```sh
git clone https://github.com/MNeroba/SteamController-PS5.git
cd SteamController-PS5
# To rebuild the published version:
git checkout dev/v1.6.1-experimental
# Install the pinned SDK and export PS5_PAYLOAD_SDK/LLVM_CONFIG
# using docs/BUILDING.md, then:
make -C tests clean test
make -C tests sanitize
make -C payload clean
make -C payload
```

Output: `payload/ghost-control-ps5.elf` (retained internal filename). WebUI, controller images and optional tile assets are embedded automatically. Generated headers and build output are not committed. [Linux/macOS setup, browser checks and source identity →](docs/BUILDING.md)

## Project layout

```text
payload/          PS5 adapter, Steam core, USB, profiles, embedded WebUI
  web/            Browser UI, controller artwork and button prompts
  settings-tile/  Embedded optional launcher metadata and icon
config/payload/   Payload Manager metadata source
scripts/          Release, build and validation helpers
tests/            Portable C, fault-injection and browser regressions
docs/             Architecture, setup, evidence and release procedures
project-memory/   Current status, decisions and append-only test history
.github/          CI, release workflows and contribution templates
```

Transport owns I/O; the Steam decoder owns device details; mapping is independent of the PS5 ABI. Firmware-sensitive hooks are isolated. [Architecture →](docs/ARCHITECTURE.md)

## Progress and roadmap

- **Delivered:** audited foundation, host runtime hardening, reusable protocol/session/slot modules, mapping/profiles, embedded WebUI, the experimental release line through **1.0.0 Experimental**, the published **v1.5.x-experimental** reliability fixes (tile NID install, VDA/haptics/profile/shutdown recovery, inline diagnostics, CI hardening, PS4 rumble and the experimental game-hook checkbox), and **v1.6.0-experimental** WebUI hardening and **v1.6.1-experimental** runtime hand-off/bridge toggles.
- **Next:** controlled physical testing of v1.6.1 on FW 13.60 — USB-C/Puck, native DualSense coexistence, user recovery, game input, the opt-in game-feedback path, motion/touch, storage and shutdown.
- **Stable release:** blocked until the required hardware evidence matches the tested source. A passing SDK build does not establish firmware compatibility.
- **Research:** [passive native BLE feasibility, stable cross-transport identity and advanced platform integration](docs/research/BLE_IDENTITY_PLATFORM_2026-11-28.md). Active stock-radio experiments require separate approval. The 2026-11-28 plan finds no public passive native BLE path and no ready safe/stable Puck-free option; it proposes an external BLE-to-USB bridge plus handle-preserving identity, with active stock-radio work behind a separate approval gate.

[Current status](project-memory/CURRENT_STATUS.md) · [Documentation index](docs/README.md) · [Changelog](CHANGELOG.md)

The [3 November runtime comparison](docs/development/UPSTREAM_RUNTIME_COMPARISON.md) reviews current analogs and SDK sources. The published **1.6.1 Experimental** build includes the audit fixes, the earlier bounded haptic-queue work, preserved Steam touch/pressure fields, opt-in direct HID and rest/wake handling, plus the WebUI hardening described above. Source/build evidence and remaining physical checks are documented separately.

## Contributing and security

External contributions use focused pull requests against `main`. Read [CONTRIBUTING.md](CONTRIBUTING.md), include a reproducible regression and actual checks, mark unavailable tests `NOT_RUN`, and record provenance before importing code or assets. Significant protocol, architecture or platform changes should be discussed in an issue first.

[Report a bug](https://github.com/MNeroba/SteamController-PS5/issues/new?template=bug_report.yml) · [Propose an improvement](https://github.com/MNeroba/SteamController-PS5/issues/new?template=feature_request.yml) · [Security policy](SECURITY.md)

## Credits and license

Thanks to the authors whose work made this project possible:

- **[PuckbridgePS5](https://github.com/ThisIsAkill/PuckbridgePS5)** — the audited implementation foundation, by ThisIsAkill.
- **[Ghostcontrol](https://github.com/srbraboo/Ghostcontrol-PS5-USB-Controller-Patcher)** and **[PoorDS4](https://github.com/ItsBlurf/PoorDS4)** — inherited PS5 integration and bridge work, credited to StonedModder and ItsBlurf.
- **[TheBrokenG0d / PR #8](https://github.com/MNeroba/SteamController-PS5/pull/8)** — PS5 hardware investigation, WebUI worker-stack and klog-backlog fixes.
- **[PS5 Payload SDK](https://github.com/ps5-payload-dev/sdk)** — community toolchain and API references.
- **[OmniPad-PS5](https://github.com/diegobarbosaa/OmniPad-PS5)** and **[LibrePadPS5](https://github.com/arwebSE/LibrePadPS5)** — reviewed engineering references; no implementation copied from these projects.
- **Xelu** — CC0 controller prompts; **Akhil Moola** — the inherited controller drawing credit. Protocol and platform references are listed with exact pins in [sources](docs/SOURCES_AND_EVIDENCE.md).

Code is **GPL-3.0-or-later**, with upstream notices retained. Supplied controller photos and the tile icon have owner-authorized publication terms; **no separate open artwork license was supplied**. The code license does not grant a new license for those assets. See [LICENSE](LICENSE), [provenance](docs/PROVENANCE.md), [controller artwork notice](payload/web/controllers/NOTICE.md), [tile artwork notice](payload/settings-tile/NOTICE.md) and [upstream documents](docs/archive/upstream/README.md).

Not affiliated with Valve or Sony. Steam, PlayStation and DualSense are their respective owners' trademarks.
