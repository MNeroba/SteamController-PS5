# Native PS5 Bluetooth LE evidence decision

**Status:** Research only; no active native-radio operation performed.

**Decision date:** 2026-10-08

## Target and hardware

- Target: native PS5 system Bluetooth interface on firmware 13.60.
- Hardware available for this audit: none. PS5 model, Bluetooth module revision, DualSense, and Steam Controller firmware are unknown.
- Safety requirement: preserve a stock DualSense connection; no internal-radio ownership changes or undocumented calls.

## Pinned source evidence

- PuckbridgePS5 `78460113ef7c48502c765a1f83ba34d53c977134`: USB/Puck support. It does not implement native PS5 BLE central transport.
- OmniPad-PS5 `b7682efce18ee0a8d7c80bbc90afbce72151ef96`: source contains experimental `src/bt_host.c` and `src/bt_hci_usb.c`, but the v1.0.4 PS5 build excludes them; their presence is not proof of scan or GATT support.
- SharpProspero `5dfdb4b0b166e83dc9ae275001afb695f12c7174` and historical PS5-3.20_Libs stubs document Bluetooth HID exports, not a verified FW 13.60 arbitrary GAP/GATT central API.
- SteamlessLink `85d7c4f5117d03127540c3a75b3536fe2015cc42` documents Android-side Valve GATT service/report mapping only.

## Gate statuses

| Gate | Status | Evidence / stop reason |
|---|---|---|
| B0 — host source/API/licensing review | PARTIAL | Public HID declarations and Android protocol references reviewed; no confirmed generic Prospero GAP/GATT central API. No code or BTstack imported. |
| B1 — passive FW 13.60 metadata diagnostic | BLOCKED_HARDWARE | No console is available. No device nodes, modules, symbols, errno values, or stock-pad coexistence were inspected. |
| B2 — documented OS-managed discovery | NOT_STARTED | Requires B1 evidence and a console-specific safety/recovery plan. |
| B3 — first real LE advertising report | NOT_PROVEN | No native PS5 LE report observed. |
| B4 — link, secure pairing, GATT discovery | NOT_PROVEN | No native PS5 connection attempt performed. |
| B5 — Steam Controller GATT reports | NOT_PROVEN | Android reference is not PS5 runtime evidence. |
| B6 — common-core/game input and cleanup | NOT_PROVEN | No native BLE transport exists in selected release source. |

## Conclusion and next safe step

Native PS5 BLE remains an evidence-gated research track and is not a release dependency. This audit ran no Bluetooth/HCI commands and did not interact with the stock DualSense. Continue with USB-C and the official Puck. The next BLE step is a passive, read-only FW 13.60 metadata inventory performed on an explicitly available console, recording the model/build, module and node metadata, exact safe command, errno, timestamp, and before/after DualSense state. Do not call opaque HID/GAP functions or seize `/dev/ugen`/internal HCI. Stop if the next gate requires active radio ownership or could interrupt the DualSense.

## Risk and distribution boundary

No firmware compatibility claim is made. Linux Bluetooth patches, Android GATT behavior, emulator stubs, and PS5 Bluetooth settings do not prove native PS5 LE Central/GATT. BTstack is reference-only because its current source terms have noncommercial restrictions and are not cleared for this GPL payload. Any later experiment must be logged in `project-memory/TEST_LOG.md`, with negative results preserved.
