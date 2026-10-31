# Experimental DEV build: loading and configuration

**Experimental development build. Not verified on physical PS5 hardware. PS5 firmware 13.60 is the intended test environment, not a confirmed compatible firmware. Use at your own risk.**

Published experimental downloads are listed on the [GitHub Releases page](https://github.com/MNeroba/SteamController-PS5/releases). Select the exact prerelease you intend to test, then download its ELF and matching checksum from that same release. Confirm the version and source SHA in its release notes; never substitute an older or locally built ELF. Every DEV build remains experimental. Pending VDA recovery uses event correlation; actual PS5 input and native DualSense coexistence are unverified. Read the [contract and tester checklist](VDA_PENDING.md).

## Before loading

This is a native PS5 ELF, not a PKG/FPKG. A stock console cannot load it using these instructions. Independently confirm that your development environment, exact installed firmware and payload loader support this type of ELF. The project has not established a working loader/runtime combination on FW 13.60. Do not update firmware to follow an upstream example.

1. Download the ELF and its matching `.elf.sha256` from the same experimental prerelease. Read its exact source SHA, SDK version, warnings and notices.
2. Verify the files in their download directory:

   ```sh
   # Linux
   ELF=SteamController-PS5-v0.1.0-dev.N.elf # replace dev.N with the downloaded release version
   sha256sum --check "$ELF.sha256"
   # macOS
   ELF=SteamController-PS5-v0.1.0-dev.N.elf # replace dev.N with the downloaded release version
   shasum -a 256 -c "$ELF.sha256"
   ```

3. Keep the native DualSense available. Use a recoverable development environment; retain local console access. Do not enable hooks or built-in-radio experiments for a first test.
4. Start your independently confirmed compatible payload loader. If it accepts an ELF over TCP, send the verified file to its actual port:

   ```sh
   ELF=SteamController-PS5-v0.1.0-dev.N.elf # replace with the exact downloaded filename
   nc -w 8 <PS5-IP> <loader-port> < "$ELF"
   ```

   Port 9021 is an inherited convenience default, not a compatibility guarantee. This project has not tested the transfer or loading procedure on physical hardware. Do not repeatedly send the payload when a loader has failed or become unresponsive.

## First launch

1. Load the verified ELF through your existing compatible payload loader.
2. Open `http://<PS5-IP>:8090` on your PC or phone on the same trusted LAN.
3. Enter the **eight-digit pairing code** from the PS5 notification in the portal's integrated pairing screen.
4. Configure the controller; Save confirms the stored profile with a fresh readback.

No FTP, manual INI file or persistent installation is required for this flow. A missing runtime policy enables WebUI automatically. The readiness notification appears only after successful listener startup. If startup fails, the payload reports the failure and continues its controller startup. Actual PS5 notification, loader, USB and networking behavior remain unverified.

## Safe defaults and advanced recovery

Bridge probing, game hooks and DualSense handoff remain disabled. USB scanning uses only the Steam Controller allowlist and checked interfaces. Native BLE is not enabled. Keep the stock DualSense available and preserve its pairing state.

Pairing codes expire after five minutes, allow five attempts and can be redeemed once; concurrent redemption has one winner. The bearer token is never shown in a notification or logged; persistent mode stores it privately, while Session only mode keeps it only in RAM. A browser retains its credential in session storage, including reloads. In persistent mode, if the code expires or has already been used, advanced recovery accepts the owner-only `/data/ghostpad/webui.token` via the dialog. In Session only mode no filesystem recovery token exists; retain the enrolled browser to collect diagnostics before stopping. One code enrolls one browser, including a separate PS5 tile browser. A new payload start supplies a new code, but do not repeatedly reload while registration or cleanup is unresolved. Do not put either secret in URLs, screenshots, issues or committed logs.

All protected APIs require bearer authentication plus request validation. Only loopback, private RFC1918 and link-local IPv4 peers are permitted; public IPv4 and IPv6 peers are rejected. HTTP is plaintext: use a trusted isolated LAN and **never forward port 8090 to the internet**.

To explicitly disable the portal, create `/data/ghostpad/runtime-policy.ini` using trusted filesystem access:

```ini
ENABLE_BRIDGE_PROBE=0
ENABLE_GAME_HOOKS=0
ENABLE_DUALSENSE_HANDOFF=0
ENABLE_WEBUI=0
```

Malformed, duplicated, inaccessible or unsafe existing policy files fail closed with all optional services disabled. An absent file uses the first-launch defaults. An explicit valid `ENABLE_WEBUI=0` is preserved. Corrupt or unsafe token storage also prevents WebUI startup.

Profiles remain under `/data/ghostpad/profiles/`. If the stored Default profile is corrupt, the authenticated UI offers an explicit confirmed recovery; the damaged file is retained as `default.ini.recovery.*`. Valid profiles cannot be reset by that recovery API. Other profiles remain untouched.

## Stop, recover and report

The implementation handles SIGINT/SIGTERM and coordinates neutral input, USB cancellation, service shutdown and owned slot cleanup. Physical rollback and ProsperoOS timing are unverified. Use your development environment's known recovery procedure if it becomes unresponsive; do not improvise firmware/system patches.

For a test report, record the exact payload SHA, ELF checksum, SDK, measured console firmware/model, controller revision, USB-C/Puck transport and outcome. Never copy serials, tokens, Bluetooth addresses or bonding keys into the report. Follow `docs/TEST_PLAN.md` and `project-memory/HARDWARE_MATRIX.md` in the corresponding source. Stable release remains blocked until the required physical evidence is recorded and validated.

## Optional on-console settings tile (same ELF)

After pairing, open **Settings & help → PS5 tile → Create tile on PS5**, read the experimental warning and confirm. The embedded icon and metadata register only `SCPS50001` in Media. No second ELF, FTP or separate image upload is needed. Keep the payload running; the tile opens `http://127.0.0.1:8090/`, not the payload itself. The PS5 browser has its own session and needs an unused notification code or, in persistent mode, the private recovery token. A code already redeemed by the LAN browser cannot enroll the tile browser. Session only mode has no filesystem recovery token; keep the enrolled LAN browser for diagnostics. Remove the tile through the PS5 normal Delete action. This installer does not perform native uninstallation or database editing.

See [SETTINGS_TILE.md](SETTINGS_TILE.md) for failure/retry behavior and required FW 13.60 hardware checks. Do not treat an accepted registration request as a tested on-console launch.


## Download a diagnostic report

After enrollment, open Settings & help → Diagnostics → Download Diagnostic Report. The fixed-name text export includes available startup, USB, user-recovery, virtual-controller, profile/motion/haptic and build/runtime information, plus a bounded filtered application log tail. It works with incomplete initialization or no connected Steam Controller. It does not read arbitrary system files. Review profile names, USB paths and other context before attaching it to an issue; never include credentials or console identifiers. Use SECURITY.md for vulnerabilities rather than posting exploits publicly.


## Pending registration and storage fallback

The experimental pending-registration path observes one typed virtual device after one Add and continues USB input during identification. Keep other virtual-controller payloads stopped; temporal correlation is not a proven ownership ABI. Press Steam/PS and use the console's profile picker. A connected or unknown native DualSense prevents automatic local binding; no native disconnect/unbind or Shell patch is performed. If identification is unresolved, avoid repeated reloads and collect diagnostics. Native coexistence is physically unverified.

If operational credential-storage writes fail, an experimental build may start with a fresh RAM-only 256-bit token. The console notification says **Session only**; normal pairing and bearer/Origin/LAN checks remain. Pair again after restart, and download diagnostics while running. There is no persistent recovery token in this mode. Profile saves may remain unavailable; a storage fallback is not a filesystem repair. Existing malformed/foreign/insecure credentials and entropy failure still disable access. [Contract and tester checklist](VDA_PENDING.md).
