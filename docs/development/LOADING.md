# Experimental DEV build: loading and configuration

Current prerelease: [v1.0.0-experimental](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v1.0.0-experimental). Use its one ELF and matching checksum. The contributor's earlier FW13.20/Puck report applies to original PR8 source, while this revised ELF still needs controlled console verification.


**Experimental development build. Not verified on physical PS5 hardware. PS5 firmware 13.60 is the intended test environment, not a confirmed compatible firmware. Use at your own risk.**

Published experimental downloads are listed on the [GitHub Releases page](https://github.com/MNeroba/SteamController-PS5/releases). Select the exact prerelease you intend to test, then download its ELF and matching checksum from that same release. Confirm the version and source SHA in its release notes; never substitute an older or locally built ELF. Every DEV build remains experimental. Pending VDA recovery uses event correlation; actual PS5 input and native DualSense coexistence are unverified. Read the [contract and tester checklist](VDA_PENDING.md).

## Before loading

This is a native PS5 ELF, not a PKG/FPKG. A stock console cannot load it using these instructions. Independently confirm that your development environment, exact installed firmware and payload loader support this type of ELF. The project has not established a working loader/runtime combination on FW 13.60. Do not update firmware to follow an upstream example.

1. Download the ELF and its matching `.elf.sha256` from the same experimental prerelease. Read its exact source SHA, SDK version, warnings and notices.
2. Verify the files in their download directory:

   ```sh
   # Linux
   ELF=SteamController-PS5-v1.0.0-experimental.N.elf # replace experimental.N with the downloaded release version
   sha256sum --check "$ELF.sha256"
   # macOS
   ELF=SteamController-PS5-v1.0.0-experimental.N.elf # replace experimental.N with the downloaded release version
   shasum -a 256 -c "$ELF.sha256"
   ```

3. Keep the native DualSense available. Use a recoverable development environment; retain local console access. Do not enable hooks or built-in-radio experiments for a first test.
4. Start your independently confirmed compatible payload loader. If it accepts an ELF over TCP, send the verified file to its actual port:

   ```sh
   ELF=SteamController-PS5-v1.0.0-experimental.N.elf # replace with the exact downloaded filename
   nc -w 8 <PS5-IP> <loader-port> < "$ELF"
   ```

   Port 9021 is an inherited convenience default, not a compatibility guarantee. This project has not tested the transfer or loading procedure on physical hardware. Do not repeatedly send the payload when a loader has failed or become unresponsive.

## First launch — v1.0.0-experimental

1. Load the verified ELF through an independently confirmed compatible payload loader.
2. The payload starts a background attempt to register **Steam Controller Settings** in the PS5 Media section, using its embedded icon and metadata. Registration is retried when the post-success marker or embedded assets are missing; a staged-but-unregistered directory is not treated as installed. Existing foreign or inaccessible paths are not overwritten. Watch for the console notification and check Media.
3. Open the tile on the PS5. It opens `http://127.0.0.1:8090/` while the payload is running. No code or password is requested. This WebUI is reachable only from the console itself, not from a phone, PC or LAN.
4. Configure the controller; Save confirms the stored profile with a fresh readback. If the tile cannot be registered, use an existing PS5 browser/launcher to reach the loopback URL and inspect the error before using the explicit retry in Settings & help. Do not repeatedly reload the payload while VDA ownership is unresolved.

The listener binds IPv4 loopback, checks literal loopback Host and browser Origin, and requires Origin for POST requests. These checks reduce browser-origin abuse; other processes on the PS5 that can access loopback can control the WebUI. No TLS or user authentication is present. Tile registration, notifications, PS5 browser Origin behavior and firmware 13.60 have not been physically tested.

## Safe defaults

Bridge probing, game hooks and DualSense handoff remain disabled. USB scanning is restricted to known Steam Controller identities. `ENABLE_WEBUI=0` in the protected runtime policy disables WebUI startup and its automatic tile attempt. Existing profiles remain under `/data/ghostpad/profiles/`; a corrupt Default profile can be explicitly recovered with a backup.

## Stop, recover and report

The implementation handles SIGINT/SIGTERM and coordinates neutral input, USB cancellation, service shutdown and owned slot cleanup. Physical rollback and ProsperoOS timing are unverified. Use your development environment's known recovery procedure if it becomes unresponsive; do not improvise firmware/system patches.

For a test report, record the exact payload SHA, ELF checksum, SDK, measured console firmware/model, controller revision, USB-C/Puck transport and outcome. Never copy serials, tokens, Bluetooth addresses or bonding keys into the report. Follow `docs/TEST_PLAN.md` and `project-memory/HARDWARE_MATRIX.md` in the corresponding source. Stable release remains blocked until the required physical evidence is recorded and validated.

## Media tile, diagnostics and pending registration

The same ELF contains the icon and fixed Media/deeplink metadata. The first-run worker attempts registration automatically without touching a pre-existing title directory. A successful installer return is not proof that the tile appears or opens on a particular firmware. See [SETTINGS_TILE.md](SETTINGS_TILE.md) for errors and retry. The tile opens settings; it does not start the payload. Remove it using the normal console Delete action.

From the on-console WebUI, Settings & help → Diagnostics → Download Diagnostic Report exports bounded, filtered application details. Review the report before sharing it. Native PS5 download behavior is unverified.

Pending virtual-controller identification remains experimental; keep other virtual-controller payloads stopped. Press Steam/PS and follow the console picker. Native DualSense coexistence and USB/rest-wake cleanup remain unverified. Avoid repeated reloads when ownership is unresolved.

## Dev.8 direct HID and rest/wake

The default backend remains libScePad. The experimental direct backend is selected only at startup by `ENABLE_DIRECT_HID=1` in the protected `/data/ghostpad/runtime-policy.ini`; other existing policy flags and file protections still apply. Do not combine it with hooks, bridge probing or DualSense handoff. It requires one confirmed foreground account and a successfully read disconnected native pad; connected/unknown DualSense state or no readable native handle leaves registration waiting. No native controller is opened, disconnected or unbound. A raced or ambiguous Add may inhibit creation until the process is safely shut down; do not repeatedly reload it.

The full168-byte raw report includes Create, two touch contacts, quaternion, gyro(rad/s) and acceleration(g). It is source-reviewed and host-tested, not validated on our target console. Automatic rest/wake quiescence uses read-only state observations and existing owned cleanup; active-game suspend/resume timing is NOT_RUN. Before any controlled console test, read [candidate limitations](DEV8_CANDIDATE.md) and [source evidence](RAW_HID_REPORT.md). Dev.8 is published as an experimental prerelease; the physical test requirements still apply.
