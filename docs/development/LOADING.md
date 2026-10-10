# Experimental DEV build: loading and configuration

**Experimental development build. Not verified on physical PS5 hardware. PS5 firmware 12.00 is the intended test environment, not a confirmed compatible firmware. Use at your own risk.**

## Before loading

This is a native PS5 ELF, not a PKG/FPKG or persistent installer. A stock console cannot load it using these instructions. Independently confirm that your development environment, exact installed firmware and payload loader support this type of ELF. The project has not established a working loader/runtime combination on FW 12.00. Do not update firmware to follow an upstream example.

1. Download the ELF and its matching `.elf.sha256` from the same experimental prerelease. Read its exact source SHA, SDK version, warnings and notices.
2. Verify the files in their download directory:

   ```sh
   # Linux
   sha256sum --check SteamController-PS5-v0.1.0-dev.1.elf.sha256
   # macOS
   shasum -a 256 -c SteamController-PS5-v0.1.0-dev.1.elf.sha256
   ```

3. Keep the native DualSense available. Use a recoverable development environment; retain local console access. Do not enable hooks or built-in-radio experiments for a first test.
4. Start your independently confirmed compatible payload loader. If it accepts an ELF over TCP, send the verified file to its actual port:

   ```sh
   nc -w 8 <PS5-IP> <loader-port> < SteamController-PS5-v0.1.0-dev.1.elf
   ```

   Port 9021 is an inherited convenience default, not a compatibility guarantee. This project has not tested the transfer or loading procedure on physical hardware. Do not repeatedly send the payload when a loader has failed or become unresponsive.

## Safe defaults

With no valid `/data/ghostpad/runtime-policy.ini`, bridge probing, game hooks, DualSense handoff and WebUI are disabled. USB scanning is restricted to the Steam Controller allowlist and checked interfaces. Native BLE is not enabled. Do not disconnect the stock DualSense or alter its pairing state.

Existing internal `/data/ghostpad` paths remain for compatibility. Profiles are stored under `/data/ghostpad/profiles/`. There is no automatic persistent installation or startup service.

## Optional WebUI in a trusted environment

Only after a safe baseline test, use your existing trusted console filesystem access to create `/data/ghostpad/runtime-policy.ini`:

```ini
ENABLE_BRIDGE_PROBE=0
ENABLE_GAME_HOOKS=0
ENABLE_DUALSENSE_HANDOFF=0
ENABLE_WEBUI=1
```

Restart through your established loader procedure. If startup succeeds, open `http://<PS5-IP>:8090`. Obtain the owner-only token from `/data/ghostpad/webui.token` using trusted local access and enter it in the portal. Do not put it in a URL, screenshot, issue or committed log. API access requires bearer authentication; allowed IPv4 peer ranges are loopback, private RFC1918 and link-local. IPv6/public IPv4 peers are rejected. Transport is plain HTTP: use a trusted isolated LAN and do not forward the port to the internet.

Missing, malformed or duplicated policy keys fail closed. This file enables service startup; it does not establish that PS5 sockets, filesystems, browser rendering or controller I/O work.

## Stop, recover and report

The implementation handles SIGINT/SIGTERM and coordinates neutral input, USB cancellation, service shutdown and owned slot cleanup. Physical rollback and ProsperoOS timing are unverified. Use your development environment's known recovery procedure if it becomes unresponsive; do not improvise firmware/system patches.

For a test report, record the exact payload SHA, ELF checksum, SDK, measured console firmware/model, controller revision, USB-C/Puck transport and outcome. Never copy serials, tokens, Bluetooth addresses or bonding keys into the report. Follow `docs/TEST_PLAN.md` and `project-memory/HARDWARE_MATRIX.md` in the corresponding source. Stable release remains blocked until the required physical evidence is recorded and validated.
