# Native PS5 Bluetooth LE evidence decision

**dev.6 status — 2026-11-01:** native BLE remains unimplemented and B1–B6 remain unrun. Future hardware work targets measured owner-console FW13.60, not the historical12.00 assessment below; USB/Puck and the new VDA path provide no native-BLE evidence.

**Current target checkpoint — 2026-10-12:** intended FW13.60 is owner-reported and unverified. Native BLE B1–B6 remain NOT_RUN; no radio experiment was performed. The original12.00 assessment below is historical evidence, not the current hardware target. Follow [BLE_FEASIBILITY.md](BLE_FEASIBILITY.md) before any future passive study.

## Historical source assessment

**Status:** Research only; no active native-radio operation performed.

**Decision date:** 2026-10-08

## Target and hardware

- Target: native PS5 system Bluetooth interface on owner-reported firmware 12.00. The version is a target supplied by the owner, not a compatibility result.
- Hardware report: the owner has a PS5 on FW 12.00. Model, Bluetooth module revision, DualSense and Steam Controller firmware are not recorded; no diagnostic or runtime test was performed.
- Safety requirement: preserve a stock DualSense connection; no internal-radio ownership changes or undocumented calls.

## Pinned source evidence

- PuckbridgePS5 `78460113ef7c48502c765a1f83ba34d53c977134`: USB/Puck support. It does not implement native PS5 BLE central transport.
- OmniPad-PS5 `b7682efce18ee0a8d7c80bbc90afbce72151ef96`: source contains experimental `src/bt_host.c` and `src/bt_hci_usb.c`, but the v1.0.4 PS5 build excludes them; their presence is not proof of scan or GATT support.
- SharpProspero `5dfdb4b0b166e83dc9ae275001afb695f12c7174` and historical PS5-3.20_Libs stubs document Bluetooth HID exports, not a verified arbitrary GAP/GATT central API on the 12.00 target.
- SteamlessLink `85d7c4f5117d03127540c3a75b3536fe2015cc42` documents Android-side Valve GATT service/report mapping only.

## Gate statuses

| Gate | Status | Evidence / stop reason |
|---|---|---|
| B0 — host source/API/licensing review | PARTIAL | Public HID declarations and Android protocol references reviewed; no confirmed generic Prospero GAP/GATT central API. No code or BTstack imported. |
| B1 — passive FW 12.00 metadata diagnostic | NOT_RUN | The owner reports a FW 12.00 console, but this task was limited to host/build validation. No model/build, device nodes, modules, symbols, errno values, or stock-pad coexistence were inspected. |
| B2 — documented OS-managed discovery | NOT_STARTED | Requires B1 evidence and a console-specific safety/recovery plan. |
| B3 — first real LE advertising report | NOT_PROVEN | No native PS5 LE report observed. |
| B4 — link, secure pairing, GATT discovery | NOT_PROVEN | No native PS5 connection attempt performed. |
| B5 — Steam Controller GATT reports | NOT_PROVEN | Android reference is not PS5 runtime evidence. |
| B6 — common-core/game input and cleanup | NOT_PROVEN | No native BLE transport exists in selected release source. |

## Conclusion and next safe step

Native PS5 BLE remains an evidence-gated research track and is not a release dependency. This audit ran no Bluetooth/HCI commands and did not interact with the stock DualSense. Continue with USB-C and the official Puck. A future passive, read-only metadata inventory must first confirm the reported 12.00 build and record the model/build, module and node metadata, exact safe command, errno, timestamp, and before/after DualSense state. Do not call opaque HID/GAP functions or seize `/dev/ugen`/internal HCI. Stop if the next gate requires active radio ownership or could interrupt the DualSense.

## Risk and distribution boundary

No firmware compatibility claim is made. Linux Bluetooth patches, Android GATT behavior, emulator stubs, and PS5 Bluetooth settings do not prove native PS5 LE Central/GATT. BTstack is reference-only because its current source terms have noncommercial restrictions and are not cleared for this GPL payload. Any later experiment must be logged in `project-memory/TEST_LOG.md`, with negative results preserved.

## 2026-11-28 research update — B0 closed negative, bridge path recommended

Desk research (five parallel workstreams, no console/radio experiment) is recorded in [research/BLE_IDENTITY_PLATFORM_2026-11-28.md](research/BLE_IDENTITY_PLATFORM_2026-11-28.md). It closes the public part of B0 with a negative result:

- `libSceBluetoothHid` remains HID-device plumbing: 17 exports, all keyed by an existing handle/device ID, with no scan/connect/pair/bond/SMP/GATT verb. `RegisterDevice(ushort,ushort)` is a 4-byte ioctl and cannot carry a 48-bit address or pairing material. SDK v0.43 ships no `libSceBluetoothHid` stub or PS5 Bluetooth header.
- The only public "native BLE" code (OmniPad `bt_host.c`/`bt_hci_usb.c`) seizes `/dev/ugen` with `O_RDWR` and sends raw HCI commands; it is not built into its release and is explicitly inactive. That is active radio ownership, not a passive path.
- A BLE-only keyboard was rejected by a retail PS5 (`CE-109524-1`) while Classic HID worked; that test found no host-visible LE HID path.
- The Steam Controller 2026 BLE link itself **is** usable by non-Steam centrals: ESP32-S3/NimBLE, Android and macOS implementations exist; Just Works bonding, `100F6C32…` service, `7C`→0x47 / `7A`→0x45 input and `B5`→0x80 rumble. The blocker is PS5-side BLE, not the controller.

**Decision:** native passive BLE is not implementable on the public surface; B1–B6 remain `NOT_RUN` and no radio work is scheduled. Wireless is pursued through an **external BLE bridge** (Phase 2) that presents normal USB HID, with the payload's host-only transport/identity work (Phase 1) first. The full B0–B6 status table above is unchanged; B0's public portion is now complete-negative, while the 13.60 runtime probe (B1) still needs a separately approved console run. Active HCI takeover (Phase 4) is not requested and remains a hard stop.

Cross-transport identity and platform findings are in the same report: the public AddDevice ABI exposes no caller-controlled identity, so continuity requires preserving the Add receipt/handle (recommended design D: bounded neutral keep-alive); motion/touch are publishable, while lightbar/LED/speaker/physical trigger resistance are not available through the virtual pad.
