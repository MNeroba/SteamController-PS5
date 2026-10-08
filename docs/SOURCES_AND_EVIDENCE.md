# Research source register and evidence status

**As of 2026-10-08.** These source URLs and the observed commit SHAs are **research snapshots**; Codex must re-check HEAD/repo ownership/LICENSE and pin its own reviewed SHAs before porting. Each `confirmed source fact` establishes content visible in source, **not** actual PS5 behavior unless separately marked as independent hardware test.

## Direct project sources

| ID | Source | Research-pinned SHA | Verified source observation | Application / limits |
|---|---|---|---|---|
| S01 | [PuckbridgePS5](https://github.com/ThisIsAkill/PuckbridgePS5) | `78460113ef7c48502c765a1f83ba34d53c977134` | README claims tested FW 13.60; native PS5 USB/Puck, gyro, motion, haptics, trigger feedback, profiles, Ghostcontrol-derived virtual pad and game hooks | **Best first audit candidate**; hardware claims require independent reproduction; authors say only one SC active per Puck, no portal password |
| S02 | [OmniPad-PS5](https://github.com/diegobarbosaa/OmniPad-PS5) | `b7682efce18ee0a8d7c80bbc90afbce72151ef96` | FW 7.00–13.60 claim; v1.0.4 USB hotplug and virtual slots; direct BT not active | Compare slot architecture, ShellUI handling and reuse of USB infrastructure; code has P0 issues (audit selected base before cherry-pick) |
| S03 | [SteamlessController](https://github.com/ddeverill/SteamlessController) · [pinned protocol header](https://github.com/ddeverill/SteamlessController/blob/1de3f7a40ec9f73b0683898e60c7cc8a5695e0b0/src/steam/SteamController.h#L12-L28) · [LICENSE](https://github.com/ddeverill/SteamlessController/blob/1de3f7a40ec9f73b0683898e60c7cc8a5695e0b0/LICENSE) | `1de3f7a40ec9f73b0683898e60c7cc8a5695e0b0` | The pinned source defines Valve VID `0x28DE`, wired/Puck/BLE PIDs `0x1302`/`0x1304`/`0x1303`, vendor usage page `0xFF00`, controller usage `0x0001`, and dock usage `0x0002`. Its LICENSE blob `bf6efcde7907313ebc0cace102b48fede413e2cc` is MIT. | Source-level reference only: no Steam report-descriptor bytes or device hardware were independently captured. No code was reused. |
| S04 | [SteamlessLink](https://github.com/reo101/SteamlessLink) | `85d7c4f5117d03127540c3a75b3536fe2015cc42` | Android GATT Bridge: Valve service `100f6c32-1735-4313-b402-38567131e5f3`; state chars `100f6c7c` / `100f6c7a`; output/feature reports | BLE protocol reference ONLY; not PS5 GATT proof |
| S05 | [OpenPuck](https://github.com/safijari/openpuck) · [protocol](https://github.com/safijari/openpuck/blob/main/docs/PROTOCOL.md) | `2a1e54bc77fb194c6c55ee968bc594fd5f7672f4` | Separate Puck USB interface and proprietary 2.4 GHz radio protocol, 4 bond slots described | Do not conflate OpenPuck emulated VID/PID with official Valve Puck, or radio protocol with BLE |
| S06 | [SDL HIDAPI Triton](https://github.com/libsdl-org/SDL/blob/main/src/joystick/hidapi/SDL_hidapi_steam_triton.c) · [structs](https://github.com/libsdl-org/SDL/blob/main/src/joystick/hidapi/steam/controller_structs.h) | pin during M0 | Independent protocol and report structure reference | Verify SDL zlib and individual file notices before adapting code |
| S07 | [PS5 payload SDK](https://github.com/ps5-payload-dev/sdk) | `f7fd02e6e195902a449b5e664917be8c01888b34` | Community SDK provides dynamic-linked PS5 ELF payload builds | Not official NDA PlayStation SDK; distribution license review required |
| S08 | [PS5Dev/PS5SDK](https://github.com/PS5Dev/PS5SDK) | pin during M0 | Another community homebrew SDK; WIP | Not Sony Partner SDK and no BLE Central promise |
| S09 | [BlueKitchen BTstack](https://github.com/bluekitchen/btstack) · [LICENSE](https://github.com/bluekitchen/btstack/blob/master/LICENSE) | `e38553977a25fb0b55b383c72c289be0975f422c` | Full BLE stack reference; noncommercial source terms | Research only until rights/licensing resolved; do not merge into GPL payload naively |
| S10 | [PS5 payload SDK v0.43](https://github.com/ps5-payload-dev/sdk/tree/d9c9519116944a7f1c22d262012c53b80b3520b7) · `target/include/dev/usb/usb_ioctl.h` | `d9c9519116944a7f1c22d262012c53b80b3520b7` | `struct usb_fs_complete` returns `ep_index` at lines 170–172; `USB_FS_COMPLETE` is `_IOR` at line 331. `struct usb_gen_descriptor` exposes `ugd_data`, `ugd_maxlen`, `ugd_actlen`, `ugd_iface_index`, and `ugd_altif_index` at lines 100–115; `USB_GET_REPORT_DESC` is declared `_IOWR` at line 270. | These are declarations only. They do not establish report-descriptor ioctl behavior, target error codes, descriptor contents, or Prospero runtime semantics. |
| S11 | [USB-IF Device Class Definition for HID 1.11, §§5.2–5.3 and Appendix E.2–E.5](https://www.usb.org/sites/default/files/hid1_11.pdf) | Version 1.11 (published specification; not a Git pin) | Defines HID report descriptors as item streams with short- and long-item forms and collection-scoped usage information; its example configuration, interface, HID and endpoint descriptors show the declared interface/endpoint counts and endpoint fields. | Primary format reference for bounded descriptor parsing. The specification does not identify Steam Controller descriptor contents or establish PS5 ioctl behavior. |

### Upstream path hints (not contractual)

- Puckbridge: `payload/gc_main.c`, `controller_sc2.c`, `sc2_profile.c`, `sc2_binding.c`, `sc2_gyro.c`, `sc2_flick.c`, `sc2_trigfx.c`, `sc2_haptics.c`, `ds_handoff.c`, `game_hooks.c`, `payload/bridge/`, `webui.c` — paths from README; verify at pinned commit.
- OmniPad: `src/bt_host.c`, `src/bt_hci_usb.c`, `src/usb_hotplug.c`, `src/ps5_vpad.c`, `src/shellui_inject.c`, `src/tcp_stream.c`, `src/web.c`, `Makefile` — observed in main.
- SteamlessController reference IDs at pin `1de3f7a40ec9f73b0683898e60c7cc8a5695e0b0`, `src/steam/SteamController.h:12-28`: Valve `0x28DE`; wired `0x1302`; BLE `0x1303`; Puck `0x1304`; Nereid `0x1305`; vendor usage page `0xFF00`, controller `0x0001`, dock `0x0002`; USB state report `0x42`, BLE old `0x45`. These are source constants/comments, not an independently captured descriptor. Reconfirm the relevant protocol revision before freezing logic.

## PS5 Bluetooth/BLE evidence

| ID | Source | What the evidence actually says | Not proven |
|---|---|---|---|
| B01 | [SharpProspero SceBluetoothHid](https://github.com/SvenGDK/SharpProspero/blob/main/src/SharpProspero/Interop/Bluetooth/SceBluetoothHid.cs) · [wrapper](https://github.com/SvenGDK/SharpProspero/blob/main/src/SharpProspero/Platform/BluetoothHid.cs) (observed commit `5dfdb4b0b166e83dc9ae275001afb695f12c7174`) | 17 reconstructed Bluetooth HID functions, privileged access to `/dev/bt`/`/dev/bluetooth_hid`; pointer layouts not fully documented | No confirmed arbitrary BLE/GAP/GATT Central API; no FW 13.60 runtime authorization |
| B02 | [PS5-3.20_Libs export stubs](https://github.com/DNNDHH/PS5-3.20_Libs/blob/main/libSceBluetoothHid.c) | Named exports + NIDs in historical firmware stub source | Signatures and permissions on 13.60, LE support |
| B03 | [PS5 Linux patches PR #19](https://github.com/ps5-linux/ps5-linux-patches/pull/19) | **Merged 2026-06-23**, xHCI Salina and btusb fixes; PR admits unstable Bluetooth and xHCI shutdown errors | Safe ProsperoOS radio takeover or BLE GATT |
| B04 | [PS5 Bluetooth fix](https://github.com/cZdrikjdm/ps5-dualsense-bt-fix) | Reported Marvell `1286:2059`, duplicate HCI interface under Linux and workable Bluetooth fix | Every PS5 model is same; ProsperoOS HCI ownership; BLE Central on 13.60 |
| B05 | [PS5 Linux loader](https://github.com/ps5-linux/ps5-linux-loader) | Third-party Linux can use internal Bluetooth on compatible hardware | PS5 native homebrew host/GATT API |
| B06 | [btstack-dev hardware report](https://groups.google.com/g/btstack-dev/c/N7oIyyh2Fi4) | 2025-03-09 Logitech Craft BLE-only keyboard rejected on retail PS5 with `CE-109524-1`, Classic BT keyboard worked | All BLE disabled at hardware level; diagnostic LE scan is impossible |
| B07 | [AnyPS5 emulator shim](https://github.com/boykopovar/AnyPS5/blob/main/core/libs/prx/libSceBluetoothHid/Export.cpp) | Stub definitions mimicking Bluetooth HID exports | Actual PS5 runtime functionality (emulator stubs are not real driver evidence) |
| B08 | [Stadia Pico PS5 BLE bridge](https://github.com/BjornBrorsson/Stadia_Pico_PS5) | External MCU/BTstack bridge example | Direct BLE via internal PS5 radio |
| B09 | [Bluetooth SIG HID over GATT](https://www.bluetooth.com/specifications/specs/hid-over-gatt-profile-hogp/) | External BLE HID protocol specification | PS5's supported role/permissions |

## Sony SDK and publication policy

- [PlayStation Partners](https://partners.playstation.net/) is the official program entry point. Access to full partner tooling/documentation is controlled and may require agreements. **No public official PlayStation SDK documentation has been verified as exposing generic PS5 BLE Central/GATT to games or homebrew.**
- `ps5-payload-dev/sdk`, `PS5Dev/PS5SDK`, SharpProspero and PRX stubs are **community/open research**, not Sony's restricted partner SDK.
- Do not retrieve, embed, publish or redistribute confidential SDK headers, firmware files, keys or protected documentation. A Sony Spresense BLE SDK is for an unrelated device, not PS5.
- Apply license review at file level: Puckbridge README says GPL-3.0-or-later; OmniPad lists GPLv3; BTstack has explicit noncommercial limitations; upstream artwork may have separate terms.

## Missing proof ledger

- `libSceBluetoothHid` exported entry points **resolved and tested on actual FW 13.60**: NOT_VERIFIED.
- System-managed arbitrary BLE Central scan/connect/GATT on native FW 13.60: NOT_VERIFIED.
- Real first LE Advertising Report from native PS5 payload without interfering with stock DualSense: NOT_VERIFIED.
- Steam Controller 2026 connected by internal PS5 BLE and GATT notifications delivered: NOT_VERIFIED.
- Edge virtual device with extra independent buttons visible to PS5 games: NOT_VERIFIED.
- Per-device ShellUI Steam icon/name override: NOT_VERIFIED.

Record new evidence as append-only items with exact links, date, source revision and independent test status, not retroactive replacement of prior failures.
