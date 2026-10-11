<div align="center">

# SteamController-PS5

**Steam Controller 2026 → virtual DualSense on PS5**<br>
USB-C and official Valve Puck · remapping · motion · touch · haptics

[Build](docs/BUILDING.md) · [DEV downloads](https://github.com/MNeroba/SteamController-PS5/releases) · [Feature status](STEAM_CONTROLLER_2026.md) · [Contribute](CONTRIBUTING.md)

[![Host tests](https://github.com/MNeroba/SteamController-PS5/actions/workflows/host-tests.yml/badge.svg)](https://github.com/MNeroba/SteamController-PS5/actions/workflows/host-tests.yml)
[![PS5 build](https://github.com/MNeroba/SteamController-PS5/actions/workflows/ps5-cross-build.yml/badge.svg)](https://github.com/MNeroba/SteamController-PS5/actions/workflows/ps5-cross-build.yml)

![Real image-based remapping UI](docs/images/remap-desktop.png)

</div>

> **Experimental development project.** Host tests and PS5 SDK builds pass; no physical PS5 test has been performed. **FW 13.60 is the owner-reported intended test environment, not confirmed compatible firmware.** Native BLE remains research. Keep your stock DualSense available.

## What it does

One native ELF translates Steam Controller input into a virtual DualSense while retaining Steam-specific controls in a reusable input model. Profiles and the local portal configure mappings, timing, sticks, trackpads and motion.

| Implemented in code / covered on the host | Physical validation still needed |
| --- | --- |
| USB-C/Puck identification, checked interfaces, session/resource ownership and reconnect cleanup | Real descriptors, Prospero USB ioctls, hotplug and transport switching |
| Buttons, L3/R3, four back buttons, analog/Shift trigger routing; Long, Double, Turbo and Toggle | Gameplay, console UI, slot/user assignment and output pressure |
| Trackpad-to-stick/touch mapping, gyro/motion, Flick Stick and calibration state | Sensor axes, feel, latency and PS5 touch/motion behavior |
| Haptic ownership/queue, rumble and trigger-effect feedback paths | Actual USB feedback and controller response; effect routing may require optional hooks |
| Per-game/default profiles, complete atomic persistence, image remapping and guarded editor navigation | Real console filesystem, game switching and authenticated portal |
| Safe defaults, credential rollback, owned virtual-device cleanup, bounded shutdown and LAN controls | PS5 ABI, syscall failures and recovery on real hardware |

**One active Steam Controller 2026 session.** Multi-SC2 feedback, native PS5 BLE, Edge emulation and custom ShellUI identity are not supported features. See the [detailed feature table](STEAM_CONTROLLER_2026.md) for evidence and limitations.

## Architecture

```text
Steam Controller 2026
   USB-C / official Valve Puck
              │
      checked USB session
              │
  Steam protocol → extended input model
              │
  profiles / bindings / motion / touch
              │
  slot lifecycle → PS5 adapter → virtual DualSense
              └── owned feedback queue → controller
```

The transport owns I/O; Steam decoding owns device details; mapping stays independent of PS5 ABI. Firmware-sensitive hooks are isolated. [Architecture and contracts →](docs/ARCHITECTURE.md)

## Remap visually

Select a physical input, choose DualSense outputs, and stage changes before saving. Front/back views include L3/R3, grip buttons and trackpad clicks. Back-view physical **L is on the right**, **R on the left**.

| Back controls | Binding editor |
| --- | --- |
| ![Back mappings](docs/images/remap-desktop-back.png) | ![Binding editor](docs/images/remap-editor.png) |

Screenshots are from the real host fixture; they do not show a PS5 hardware session. [UI guide and browser checks →](docs/WEBUI_REMAP.md)

## Connections and DEV build

- **USB-C:** wired Steam Controller 2026 path.
- **Valve Puck:** the official USB receiver path, including checked interface/endpoint selection.
- **Bluetooth LE:** evidence-gated research; the payload does not enable native PS5 radio experiments.

Both USB paths are implemented and host-tested, **physically unverified**.

**Published: [v0.1.0-dev.2](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v0.1.0-dev.2)** — ELF, SHA256, loading instructions and notices; source `6ed81c2`. Repeated Host/ASan-UBSan/Chromium/SDK gates passed. Physical PS5 validation remains pending.

[Experimental prereleases](https://github.com/MNeroba/SteamController-PS5/releases) contain a real SDK-built ELF, checksum, source SHA, loading instructions and notices **when published**. Choose a `dev/v…-dev.…` prerelease; this is not a stable firmware compatibility promise. If no prerelease is listed, no DEV download has been published yet. Successful [PS5 Actions runs](https://github.com/MNeroba/SteamController-PS5/actions/workflows/ps5-cross-build.yml) also provide temporary build artifacts.

> **Experimental development build. Not verified on physical PS5 hardware. PS5 firmware 13.60 is the intended test environment, not a confirmed compatible firmware. Use at your own risk.**

## Build from source

Requirements: Python 3, Clang for host tests, LLVM **18**, and community **PS5 Payload SDK v0.43**. The SDK archive is checksum-pinned; it is not bundled here.

```sh
git clone https://github.com/MNeroba/SteamController-PS5.git
cd SteamController-PS5
# Install and export the pinned SDK/LLVM as documented below.
make -C tests clean test
make -C tests sanitize
make -C payload clean
make -C payload
```

Output: `payload/ghost-control-ps5.elf` (legacy internal filename). WebUI HTML, icons and all four controller images are embedded automatically; no separate web server/assets installation is needed. **[Full fresh-checkout setup for Linux/macOS →](docs/BUILDING.md)**

## Load and configure

This project produces **one ELF**: no PKG/FPKG. The optional settings tile installer and controller icon are embedded in this same payload. Use an independently confirmed compatible development payload loader for your exact firmware. Neither the loader nor FW 13.60 runtime compatibility has been established by this project.

[Loading, checksum verification, trusted WebUI setup and recovery →](docs/development/LOADING.md)

1. Load the ELF through your existing compatible loader.
2. Open **`http://<PS5-IP>:8090`** on your PC or phone.
3. Enter the **eight-digit code from the PS5 notification** in the integrated pairing screen.
4. Configure and Save; the portal confirms stored profile readback.

WebUI starts automatically when the policy file is absent; no FTP or manual INI is needed. Explicit `ENABLE_WEBUI=0` is respected, and invalid existing policy fails closed. **Game hooks, bridge probing and DualSense handoff stay disabled.** Codes expire in five minutes, allow five attempts and one redemption; protected APIs retain strong bearer authentication. Advanced token recovery remains available. HTTP is plaintext: use only a trusted isolated LAN, never public port forwarding. Internal `/data/ghostpad` paths remain for compatibility.

## Progress and next steps

- **Complete on the host:** upstream foundation/provenance audit, runtime hardening, reusable modules, profile integrity and remap/browser regressions, sanitizer coverage and SDK cross-build.
- **DEV channel:** manual, exact-source prerelease workflow with repeated host/sanitizer/Chromium/asset/build/checksum gates. Separate from stable publication.
- **Next hardware work:** measured FW 13.60, USB-C/Puck, gameplay/system UI, native DualSense coexistence, motion/touch/haptics, portal persistence and shutdown. Stable release waits for recorded physical evidence.
- **Later research:** passive BLE feasibility, stable transport identity, advanced platform integration. Active stock-radio experiments require separate explicit approval.

[Current status](project-memory/CURRENT_STATUS.md) · [Hardware test plan](docs/TEST_PLAN.md) · [Documentation index](docs/README.md) · [Repository audit](docs/development/REPOSITORY_AUDIT.md)

## Contributing

Read [CONTRIBUTING.md](CONTRIBUTING.md) before an external PR. Keep changes focused, include a reproducible regression and actual test commands/results, mark unavailable checks `NOT_RUN`, and record provenance before borrowing code. Preserve notices; do not submit credentials, console identifiers, firmware or proprietary SDK material. Discuss significant scope changes in an issue first.

## Credits and license

Built on [PuckbridgePS5](https://github.com/ThisIsAkill/PuckbridgePS5) by ThisIsAkill, with [Ghostcontrol](https://github.com/srbraboo/Ghostcontrol-PS5-USB-Controller-Patcher) by StonedModder and [PoorDS4](https://github.com/ItsBlurf/PoorDS4) by ItsBlurf. Thanks to their authors and the [community PS5 SDK](https://github.com/ps5-payload-dev/sdk) contributors. [OmniPad-PS5](https://github.com/diegobarbosaa/OmniPad-PS5) was audited as a reference; no OmniPad code was imported. Protocol research also draws on the public work identified in [sources](docs/SOURCES_AND_EVIDENCE.md).

Code is **GPL-3.0-or-later**, with existing notices retained. Xelu controller prompts are CC0; supplied controller photos have separate owner-authorized publication terms, not an inferred GPL/CC0 license. Read [LICENSE](LICENSE), [provenance](docs/PROVENANCE.md), [asset notices](payload/web/controllers/NOTICE.md) and [preserved upstream documents](docs/archive/upstream/README.md).

Not affiliated with Valve or Sony. Steam, PlayStation and DualSense are their respective owners' trademarks.

## Settings tile — dev.3

One ELF includes the supplied controller icon and an optional Media tile installer. After pairing, open **Settings & help → PS5 tile → Create tile on PS5**, then confirm. The tile opens `http://127.0.0.1:8090/` while the payload runs. Pair again in the console browser using the current local code or recovery token. It does not launch the payload after reboot. See [tile setup and removal](docs/development/SETTINGS_TILE.md). FW 13.60 registration, browser launch and controller navigation remain **NOT_RUN** on hardware.
