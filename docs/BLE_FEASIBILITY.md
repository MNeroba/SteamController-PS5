# PS5 FW 13.60 — native Bluetooth/BLE feasibility protocol

This is a **platform research procedure**, not a blanket authorization to run raw HCI commands against the console's live internal radio.

## Known source-level facts and boundaries

- Sony OS exposes a named `libSceBluetoothHid` module. Two open-source projects independently list many exports: `DNNDHH/PS5-3.20_Libs` NID stubs and `SvenGDK/SharpProspero` reconstructed declarations. SharpProspero's functions include Init, RegisterCallback/Device, GetDeviceInfo/Name/ReportDescriptor/InputReport/FeatureReport, SetOutput/Feature and InterruptOutput. This is **HID**, not proven GAP Central/GATT. The register-device function must **not** be presumed to perform pairing.
- SharpProspero describes privileged device nodes `/dev/bt` and `/dev/bluetooth_hid`. This is source interpretation; actual FW 13.60 accessibility and ABI remain untested.
- OmniPad's `src/bt_hci_usb.c` scans `/dev/ugen` nodes and attempts HCI USB commands. Existence of code is not proof that native LE scanning works; its receive path is incomplete and direct interface ownership is unresolved.
- PS5 Linux independently operates internal BT hardware with patched xHCI/btusb, but OS and hardware revision differ. Linux patch PR #19 warns about BT instability.
- Real 2025 experiment on PS5 rejected BLE keyboard with `CE-109524-1`, while Classic HID keyboard worked. This **does not** prove host BLE is physically impossible.
- SteamlessLink demonstrates Steam Controller 2026 BLE/GATT on **Android**, not PS5; Puck uses separate receiver and proprietary 2.4GHz radio.

## Investigate passive-first

**Phase B0, host only:** pin sources; collect NIDs/ABI hypothesis + valid host parser fixtures; review licenses, privilege requirements, device ownership. Find any public Sony-compatible native GAP/GATT export (with actual parameters, revision and source).

**Phase B1, safe hardware diagnostic:** opt-in run on user-owned FW 13.60 console: report model/build, module availability, symbol lookup, safe metadata of `/dev/bt`, `/dev/bluetooth_hid`, `/dev/ugen*`; record exact `errno`, result, timestamps, read-only code path. Do not call undocumented functions with opaque pointer layouts. Evaluate physical DualSense before/during/after; default no radio commands.

**Phase B2, separate approved active study:** only after a safety/recovery protocol and explicit operator approval, try documented OS-managed metadata/listening interfaces; do not seize USB/HCI driver interfaces from stock OS. Must establish whether OS can coexist with native DualSense connections. Abort on unexpected dropouts/privilege changes; restore state.

**Phase B3:** prove LE advertisement discovery with actual timestamped `LE Meta Event (0x3E)` or equivalent trustworthy system API output. PS5 Bluetooth settings dialog or generic HCI version is **insufficient**.

**Phase B4:** prove LE link, secure pairing/bonding where needed, GATT discovery, service and characteristic listing; log negative responses safely. Preserve stock DualSense.

**Phase B5:** Steam Controller Valve GATT service and notifications; BLE report reconstruction, lizard-mode and output commands guarded by actual verified UUID/reports.

**Phase B6:** input is interpreted by the common core and feeds virtual DualSense in a PS5 game; safe reconnect and cleanup.

### Gates and fallback

Stop at first failed/unproven gate. Do not take over internal HCI to brute force around privilege refusal; record exact barrier. Prefer shipping USB/Puck; then investigate a **separately owned** USB Bluetooth controller (with verified driver support) or optional Android bridge as a temporary diagnostic transport. Bridge/network input requires authentication and explicit slot ownership.

Do not copy the noncommercial-licensed BTstack into GPL distribution without compatible rights. Android/Windows implementations are useful for BLE protocol, not valid code for ProsperoOS stack itself.

## Output format

Create `docs/BLE_DECISION.md` with: `target`, `hardware`, `fw`, `source pins`, `system API evidence`, `device ownership`, `B0...B6 gate statuses`, `commands/log hashes`, `stock DualSense coexistence`, `risk`, `next safe step`. Each experiment enters `project-memory/TEST_LOG.md` and negative results remain visible.
