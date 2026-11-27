# Passive native BLE, cross-transport identity and platform integration

**Status:** research and plan only. No console, radio, pairing, firmware or USB experiment was run for this document. Every hardware claim below is labelled and remains unverified on the project target.

**Date:** 2026-11-28
**Target context:** owner-reported PS5 firmware 13.60 (unmeasured), Steam Controller 2026 ("Ibex"/Triton), payload `v1.5.2-experimental` (`ef6e2ae`) plus docs `5367733`.
**Method:** five desk-research workstreams (PS5 Bluetooth surface, Steam Controller BLE protocol, virtual-pad identity, platform features, repo gap analysis), source pins fetched directly, then reconciled against this repository's code. `web_search` was unavailable (provider HTTP 402); all evidence was fetched from direct URLs and the GitHub REST API, so this is a source review, not a literature sweep.

## 1. Verdict

| Question | Verdict | Confidence | Gate |
|---|---|---|---|
| Can the payload use the **stock PS5 radio as a passive BLE Central** (receive advertisements / GATT without taking over HCI)? | **No public path exists.** `libSceBluetoothHid` is HID-device plumbing with no scan/connect/pair/GATT verb, and a reported retail test rejected a BLE-only HID keyboard. Any remaining route is raw HCI ownership, which is **active**, not passive. | High for the public surface; `UNKNOWN` only for unpublished 13.60 exports | B0 closed negative; B1 not run |
| Is the **Steam Controller 2026 BLE link itself usable by a non-Steam central**? | **Yes.** Three independent non-Valve centrals (ESP32-S3/NimBLE, Android, macOS) read it; pairing is Just Works. | High (`VERIFIED_CODE` + one `EXTERNAL_HARDWARE` ESP32 report) | Controller-side B5 evidence exists externally; PS5-side B4/B5 still unproven |
| Can one logical controller survive **USB-C ↔ Puck ↔ BLE** without the game seeing a new pad? | **Only by preserving the virtual-pad handle.** The Add API exposes no caller-controlled identity (no name/MAC/serial); delete+recreate is observably a new device. The design is feasible but needs console validation. | Medium | Depends on B1/identity hardware test |
| Are **advanced DualSense features** reachable? | **Partly.** Motion and two touch contacts are already in the ABI and unpublished; haptics work but are quality-limited; trigger effects are haptic only; lightbar, player LEDs, speaker audio and physical trigger resistance are **not** available through the virtual pad. | High for current ABI | Platform work can start host-side now |

**Bottom line:** "passive native BLE" is not implementable as worded. Wireless is still achievable without touching the stock radio by moving the BLE central into a **separate bridge** (ESP32-S3) that presents a normal USB HID device to the PS5. That path also gives the project something the native-radio path cannot: it keeps the console's Bluetooth stack, the stock DualSense and the existing USB architecture untouched.

### 1.1 Wireless without the Puck or a cable

"Is there a safe and stable way to connect the Steam Controller 2026 without the Puck or a cable?" — **not today.** There is no ready, hardware-verified option; the only realistic path is the external BLE-to-USB bridge, and its PS5 stability is still unproven.

| Option | Console safety | Stability | Status |
|---|---|---|---|
| Native PS5 BLE (payload owns the radio) | Low — HCI takeover can drop the stock DualSense | Unknown; no public central API | **Rejected** (see 2.1) |
| Generic USB Bluetooth dongle | — | — | No generic path: the PS5 exposes no Bluetooth host stack to the payload, and a dongle would still have to emulate an approved USB gamepad |
| **External BLE-to-USB bridge (ESP32-S3)** | **High** — no radio operation on the PS5; ordinary USB HID | Controller link and rumble proven externally; PS5 binding, latency and hot-unplug **unproven** | **Only recommended path** (Phase 2) |
| Custom ESB receiver dongle (OpenPuck-style) | Medium — different radio personality, pairing workflow | Unproven without Steam; more reverse engineering | Not planned |
| Phone/PC network bridge | Medium — needs a third device and authentication | Poor for gameplay latency | Not planned |

Two usability facts belong with the answer: BLE, ESB (Puck) and USB are **mutually exclusive controller personalities**, so switching to BLE drops the Puck link and returning requires re-pairing; and a bridge is still a dongle, just one the project controls. For a user who wants no Puck and no cable *and* no dongle, the honest answer is that the stock PS5 cannot do it. Keep the Puck for now; if an external bridge is acceptable, Phase 2 is the path, and Phase 1 can start immediately without hardware.

## 2. Evidence

### 2.1 PS5 Bluetooth surface (native, stock radio)

| Claim | Label | Source | Implication |
|---|---|---|---|
| `libSceBluetoothHid` exports 17 functions: `Init`, `ParamInitialize`, `ThreadParamInitialize`, `Register/UnregisterCallback`, `Register/UnregisterDevice`, `GetDeviceInfo/Name/ReportDescriptor/InputReport/FeatureReport`, `SetOutputReport/FeatureReport`, `InterruptOutput`, `DisconnectDevice`, `DebugGetVersion`. | `VERIFIED_CODE` (names/NIDs) + `UPSTREAM_CLAIM` (arities/layouts) | [SharpProspero `SceBluetoothHid.cs` @ `5dfdb4b`](https://github.com/SvenGDK/SharpProspero/blob/5dfdb4b0b166e83dc9ae275001afb695f12c7174/src/SharpProspero/Interop/Bluetooth/SceBluetoothHid.cs); [PS5-3.20_Libs @ `b2a77c0`](https://github.com/DNNDHH/PS5-3.20_Libs/blob/main/libSceBluetoothHid.c) | Every verb is keyed by an **existing** device handle/ID. There is no scan, connect, pair, bond, SMP or GATT-client call. |
| `sceBluetoothHidRegisterDevice(ushort, ushort)` is a 4-byte ioctl (`0x8004622b`); the public signature cannot carry a 48-bit address, keys or pairing material. | `VERIFIED_CODE` (arity) + `HYPOTHESIS` (semantics) | SharpProspero above; FreeBSD `_IOC_IN` encoding | "Register device" is not pairing. Treating it as a pairing API would be an unverified ABI call with side effects. |
| The community SDK v0.43 has **no** `libSceBluetoothHid` stub and no PS5 pad/BT header; `include/ps5/` holds only `iommu.h, kernel.h, klog.h, mdbg.h, nid.h, payload.h`. The only Bluetooth headers are imported FreeBSD netgraph ones (`ng_btsocket_hci_raw.h`, `ng_hci.h`, `ng_l2cap.h`). | `VERIFIED_CODE` | [ps5-payload-dev/sdk @ `d9c9519`](https://github.com/ps5-payload-dev/sdk/tree/d9c9519116944a7f1c22d262012c53b80b3520b7) | No supported LE API ships with the SDK. Netgraph headers describe a kernel interface, not a Sony GAP API. |
| OmniPad's `bt_host.c` sends raw HCI Inquiry and parses `0x02/0x2F` events; `bt_hci_usb.c` opens `/dev/ugenB.D` `O_RDWR` and issues `USB_DO_REQUEST`. At the pinned SHA neither file is in `ps5.mk`/`Makefile`, and its README says direct wireless Bluetooth is "NOT active in v1.0.4". | `VERIFIED_CODE` + upstream self-disclaimer | [OmniPad @ `b7682ef`](https://github.com/diegobarbosaa/OmniPad-PS5/blob/b7682efce18ee0a8d7c80bbc90afbce72151ef96/src/bt_hci_usb.c) | The only public "native BLE" attempt is (a) an **HCI takeover**, not passive, and (b) not built or proven. |
| A BLE-only Logitech Craft keyboard was rejected by a retail PS5 with `CE-109524-1`; the same BTstack demo over Classic Bluetooth worked. | `EXTERNAL_HARDWARE` / `UPSTREAM_CLAIM` (single community test) | [btstack-dev thread, 2025-03-09](https://groups.google.com/g/btstack-dev/c/N7oIyyh2Fi4) | That retail test found no host-visible LE HID path. It does not prove the radio lacks LE silicon. |
| The PS5's own Linux port drives the internal radio with patched xHCI/btusb and still reports instability. | `EXTERNAL_HARDWARE` | ps5-linux patches PR #19 (see `docs/SOURCES_AND_EVIDENCE.md` B03) | A different OS taking ownership is not evidence that ProsperoOS permits coexistence. |

**What "passive" can actually mean here** (important for any future approval request):

| Mode | Radio commands | Available? | Assessment |
|---|---|---|---|
| Metadata only: resolve `libSceBluetoothHid`, `stat` `/dev/bt`, `/dev/bluetooth_hid`, `/dev/ugen*`, read USB descriptors of the BT controller | none | Yes, but **unmeasured** | Safe enough for a future read-only diagnostic; module constructors and node opens must still be treated as side effects until measured. |
| OS-managed BLE scan/observe | none from us | **No API found** | No observer, filter or advertisement-callback export exists publicly. |
| Raw HCI socket (`AF_BLUETOOTH`/netgraph) or `/dev/ugen` HCI ownership | yes (reset, scan, connect) | Headers exist; runtime unproven | **Active.** Can drop the stock DualSense, lose bonds and leave the radio in an unknown state. Hard stop under `AGENTS.md` without separate approval. |
| BLE central in an external bridge | none on PS5 | Yes | Only PS5-side work is ordinary USB HID. Recommended path. |

### 2.2 Steam Controller 2026 BLE (external central)

The controller side is no longer the unknown. Public, non-Valve centrals work:

- **ESP32-S3 / NimBLE**: `inonomori/steamcontroller32` @ `2b20075`, MIT. Basic input, reconnect, pairing persistence and two-channel rumble exercised on one unit (2026-11-15).
- **Android `BluetoothGatt`**: `reo101/SteamlessLink` @ `1e66096` (no licence file; behaviour reference only).
- **macOS CoreBluetooth**: ALVR's `triton_probe.swift` harness.
- **Linux**: `hid-steam.c` treats it as an ordinary HOGP device.

| Fact | Value | Label |
|---|---|---|
| GAP name | `Steam Ctrl (BT) <id>` in the **scan response**; vendor service UUID is **not** in the advertisement | `VERIFIED_CODE` (CFW reproduction) + `EXTERNAL_HARDWARE` (observer) |
| Valve service | `100F6C32-1735-4313-B402-38567131E5F3` | `VERIFIED_CODE` + `EXTERNAL_HARDWARE` |
| Input characteristics | `…100F6C7C…` → report `0x47`, `…100F6C7A…` → `0x45`, `…100F6C33…` → legacy `0x03`; sources disagree on which of 7C/7A is preferred | `VERIFIED_CODE` (three sources); `0x45` path `EXTERNAL_HARDWARE` |
| Notification payload | 45 bytes without a report-ID prefix; a 46-byte capture with a matching prefix is also accepted; 0x45 and 0x47 share the first 17 bytes | `EXTERNAL_HARDWARE` (MTU 67) / `VERIFIED_CODE` |
| Haptic output | `100F6CB5-…` → report `0x80`, **write-without-response only**; 9-byte payload `type:u8, intensity:u16le, left.speed:u16le, left.gain:i8, right.speed:u16le, right.gain:i8`; refresh ≈40 ms because the controller stops after ≈50 ms | `VERIFIED_CODE` + `EXTERNAL_HARDWARE` (rumble felt) |
| Pairing | SMP LE Secure Connections, bonding, **NoInputNoOutput / Just Works**, ENC+ID keys; no passkey | `EXTERNAL_HARDWARE` (ESP32) |
| Pairing mode | hold **RB + B + Steam** ≈7 s until the LED is steady; unconnected controller sleeps after ≈1 min | `UPSTREAM_CLAIM` (one tested unit) |
| Reconnect | A bonded controller may advertise without a name **or** the Valve UUID; the central must match a stored peer identity | `EXTERNAL_HARDWARE` |
| Connection parameters | controller asks 7.5–15 ms interval, latency 0, 4 s supervision; ATT MTU 67 negotiated (central may request 185/517) | `VERIFIED_CODE` + `EXTERNAL_HARDWARE` |
| Radio personality | BLE, ESB (Puck) and USB are **mutually exclusive**; the controller reboots into one personality. A Puck-paired controller is not simultaneously BLE-connectable. | `VERIFIED_CODE` (CFW `radio_personality.c`) + `UPSTREAM_CLAIM` |

**Minimal central sequence** (what a bridge or an approved native experiment must implement): scan by name/remembered identity → connect → MTU exchange → satisfy an `INSUFFICIENT_AUTHEN`/`INSUFFICIENT_ENCRYPTION` by bonding (Just Works) → discover `100F6C32…` → subscribe to `7C` and `7A` (preference varies by source) → decode 45/46-byte notifications → optionally write `B5` for rumble → on disconnect zero input, stop rumble, re-arm scan and match the bond.

**Licensing:** `steamcontroller32` MIT; SDL zlib; `hid-steam.c` GPL-2.0; `SteamlessLink` no licence (do not copy); `sc26re` and `pair-raught` AGPL-3.0 (facts only); Valve firmware blobs must not be vendored. BTstack remains noncommercial and is not usable here.

### 2.3 Cross-transport identity

| Claim | Label | Source |
|---|---|---|
| `scePadVirtualDeviceAddDevice` receives a 32-byte block: `size=32`, `creation_word=1`, six opaque in/out words; the kernel writes back candidate IDs. There is **no** name, MAC, serial, UUID or user field. | `VERIFIED_CODE` (repo) + `UPSTREAM_CLAIM` (ABI) | `payload/gc_main.c:700,760-762`; `docs/development/VDA_PENDING.md:16` |
| The SDK ships **no** `ScePad` struct or prototype; `sce_stubs/libScePad.c` declares bare symbols only. `ScePadData` is repo-local (`sizeof == 120` assert). | `VERIFIED_CODE` | SDK `sce_stubs/libScePad.c`; `payload/bridge/pad_types.h` |
| The only durable identity the console gives us is the **Add receipt/handle**. A non-positive Add that the klog classifies as `0x803b0006` is an assignment-pending event, not an owned handle. | `VERIFIED_CODE` + `UPSTREAM_CLAIM` | `payload/gc_main.c:766-777`; `docs/development/EXTERNAL_RUNTIME_REPORT.md` |
| Today a slot is keyed by the devfs path (`/dev/ugen*`), and the repo explicitly says that is only a live-session key. A transport change produces a new path, worker exit runs `slot_release` → `DeleteDevice`, and the game sees a new controller. | `VERIFIED_CODE` | `payload/slot_registry.h:53-56`; `payload/gc_main.c:1209-1212,1703-1712`; `docs/ARCHITECTURE.md:78` |
| A 3 s link-loss grace already exists, but only for the same path/session; it does not survive a transport swap. | `VERIFIED_CODE` | `payload/gc_main.c:170-172,1627-1629` |
| LibrePadPS5, OmniPad and any other public virtual-pad project all **delete and re-Add** on session loss; none implements handle-preserving hot-swap. | `VERIFIED_CODE` | LibrePad `virtual_controller.c:405-429`; OmniPad `ps5_vpad.c:130-153` |

The API gives us no way to *name* a device, so identity continuity is equivalent to **handle continuity**. That yields four options:

| Option | Game sees | Input gap | Main failure | Cost |
|---|---|---|---|---|
| **A. Keep handle, hot-swap source** | same pad | ~rebind only | racy source swap injects to stale handle | M |
| B. Delete + recreate (today) | new controller, possibly a pause/dialog | Add + user confirmation | dialog re-prompt, ambiguous-Add latch | 0 |
| C. One pad per transport | two pads; user must choose | 2× Add | wrong pad, duplicate dialog | L |
| **D. A + neutral keep-alive (recommended)** | same pad, neutral while switching | 0 | phantom pad if the hold is unbounded | M |

Recommendation: **D**. Keep the virtual handle for the life of the logical controller, inject neutral frames while no transport is bound, cap the hold (start at 10 s, configurable), and force a full release on logout, power rest, payload stop or hold expiry. This is the only design that preserves identity without inventing an ABI field the console does not accept.

### 2.4 Platform integration

| Feature | Direction | Current status | Verdict |
|---|---|---|---|
| Buttons/sticks/touchpads (as sticks/buttons) | pad → game | shipped | works today |
| Motion (gyro/accel) | pad → game | fields exist in `pad_types.h:26-28`, **publication unproven** | feasible, cheap, needs one console test |
| Two touch contacts + click | pad → game | fields exist (`pad_types.h:16-20`), publication NOT_RUN | feasible, cheap |
| Classic rumble (DS4-style remote setting) | game → pad | shipped v1.5.1 (16 ms poll) | works for PS4-style consumers |
| PS5 haptic audio / adaptive-trigger feedback | game → pad | only through experimental game hooks (`v1.5.2`) | works when hooks attach; not a virtual-pad output |
| Adaptive-trigger **resistance** | game → pad | intentionally translated to haptic pulses | **physically impossible** (SC2 triggers have no actuator) |
| Lightbar / player LEDs | game → pad | no code, no ABI | **not available via the virtual pad** |
| Speaker audio | game → pad | only muted/rerouted by hooks | **not available** |
| Mic-mute button/LED | both | unmapped | unknown bit, `HYPOTHESIS` |
| Battery | pad → UI | own telemetry only | not exposed to games |

The virtual pad's only settable surface is an input report (`InsertData`) plus remote-setting reads (`GetRemoteSetting`). Everything else is either captured inside the game process by hooks or unavailable. This matches the v1.5.1 finding: PS4 games fill the classic motor fields; PS5-native haptics are converted to haptic audio and never appear there.

Platform-level items sit outside that surface. The Media tile is shipped and works through the application installer, not the pad ABI. Edge identity and a per-device ShellUI Steam icon/name remain separate, unverified experiments: changing either risks the normal DualSense path, and no public evidence shows games accepting an Edge-capability virtual pad. PS5 activities/cards, the controller-assignment UI and Remote Play expose no public payload API, so they are not integration targets. Mic mute would ride a `buttons` bit if a real DualSense capture confirms the index; until then it stays `HYPOTHESIS`.

### 2.5 Repository gap

The portable core (`steam_protocol`, `sc2_binding`, `steam_mapping`, `slot_registry`, `slot_lifecycle`, `usb_input_watchdog`) is reusable. The hard part is `usb_hid_thread()` — a 530-line function in `payload/gc_main.c:1190-1721` that mixes discovery, descriptor validation, endpoint scan, streaming, decode, mapping, VDA injection, haptics and teardown. There is a good seam below it (`usb_session_ops_t`, already fake-injectable) but none above it.

Three concrete defects that a transport abstraction would otherwise inherit:

1. **Double decode.** `gc_main.c:1505` decodes a report for validation/watchdog, then `sc2_handle_packet()` decodes the same bytes again at `controller_sc2.c:375`. A BLE envelope would make these two decisions disagree.
2. **Blocking read vs shutdown.** Reader threads are joined at stop; a blocking BLE read cannot observe `runtime_is_stopping()`/`power_session_paused()` and would hang join, skipping slot release and leaving a VDA handle owned.
3. **Feedback lease is token-only.** `sc2_haptic_session_claim()` clears state only for a fresh token; a reconnect into the same slot could serve queued USB-encoded pulses to a different transport.

## 3. Plan

The plan deliberately separates what can be built **without radio or console** from what needs hardware approval. Nothing in Phase 1 touches Bluetooth, firmware or a virtual device.

### Phase 0 — this document (done)

- Record the verdict, pins and gates.
- Update `docs/BLE_DECISION.md` (B0 closed negative), `docs/SOURCES_AND_EVIDENCE.md` (new pins), project memory and ADRs.
- **No code, no radio, no release.**

### Phase 1 — host-only foundations (no hardware, no radio, shippable after normal gates)

| ID | Task | Files | Size | Acceptance |
|---|---|---|---|---|
| R1.1 | Split decode from decision: replace `sc2_handle_packet(buf,len,…)` with `sc2_handle_report(const steam_report_t*,…)`; delete the second decode in the reader | `controller_sc2.c/.h`, `gc_main.c`, new `tests/test_sc2_report_input.c` | S | one decode per frame; truncated→neutral; disconnect→neutral; battery/unknown→no inject; parity test green |
| R1.2 | Add `transport_ops_t` with `read_frame/write_report/identity/cancel/close`; implement a USB adapter over the existing `usb_session`; add a fake transport and byte-for-byte parity test | new `payload/transport.h`, `gc_main.c`, `tests/transport_fixture.h`, new test | M | USB behavior unchanged; fake and USB produce identical frame/sequence streams |
| R1.3 | Identity v2: `logical_id`, `source_epoch`, new `IDLE` state, bind/unbind, bounded neutral keep-alive, haptics lease reset on transport change | `slot_registry.c/.h`, `slot_lifecycle.c/.h`, `gc_main.c` | M | handle preserved across unbind/rebind; zero DeleteDevice while parked; expiry injects neutral then deletes once; stale token rejected |
| R1.4 | Define the bridge wire protocol and implement a fake-bridge transport (no USB IDs, no hardware) | `transport.h`, `docs/research/BRIDGE_PROTOCOL.md`, tests | S/M | malformed/truncated/duplicate frames rejected; input neutralized on link loss; rumble write ordering tested |

**First PR:** R1.1 alone. It removes the duplication a BLE path would otherwise create, is fully host-testable, and adds no unused API.

### Phase 2 — external BLE bridge (hardware; no PS5 radio)

Target: a user-owned ESP32-S3 (XIAO ESP32S3 class, 8 MB flash/PSRAM) running NimBLE as BLE Central, presenting a **vendor-specific USB HID interface** to the PS5. The PS5 sees a normal USB device; the payload sees a framed input source.

| ID | Task | Acceptance |
|---|---|---|
| R2.1 | Bridge firmware: scan by name/bond, Just Works bonding, store bond in NVS, subscribe `7C`/`7A`, decode 0x45/0x47, forward rumble to `B5` at ≥25 Hz, zero on disconnect | bench: PC sees HID input; controller reconnect after power cycle; rumble felt; no bond loss on adapter power cycle |
| R2.2 | Payload bridge transport: dedicated VID/PID (not Valve's), framed `HELLO/INPUT/STATUS/RUMBLE/RESET` with CRC and sequence, feed `transport_ops_t`, never parse BLE inside `steam_protocol` | host: replay fixtures; malformed frames rejected; link loss neutral; rumble round-trip |
| R2.3 | Bench validation on Linux/Windows first, then PS5 | input, motion, touch, rumble, hot-unplug; logs without MACs/keys |
| R2.4 | Security/provenance: bond stays on the bridge; payload logs a redacted peer tag; document Just Works (no MITM protection) and the dedicated USB ID | `SECURITY.md`, `PROVENANCE.md`, `docs/BLE_DECISION.md` updated |

The dedicated ID must be legitimately obtained (an open-hardware allocation such as pid.codes, or an owner-owned ID); do not reuse Valve's. Why not simply impersonate `0x28de:0x1302`? It would need zero payload changes, but it misrepresents a Valve device, complicates provenance and makes the PS5's own HID driver behavior part of the contract. A dedicated ID plus one small adapter is cleaner and testable. Keep impersonation as a throwaway lab option only, not a shipped path.

### Phase 3 — console validation (owner + console; read-only or USB-only)

| ID | Task | Requires |
|---|---|---|
| R3.1 | Passive inventory: resolve the 17 `libSceBluetoothHid` NIDs, `stat` `/dev/bt` + `/dev/bluetooth_hid` + `/dev/ugen*`, read the BT controller's USB descriptors through read-only descriptor ioctls, call `DebugGetVersion`, sample DualSense before/during/after. No HCI command, no interface claim/detach, no write, no scan; a plain node open is itself a side effect to justify before use. | explicit console run approval; exact ELF/checksum; recovery plan |
| R3.2 | Identity handover: start on USB-C, park in `IDLE`, switch to Puck, confirm the same virtual handle and no game-visible new controller; then reverse. Record pause/dialog/LED behavior per game. | R1.3 + console |
| R3.3 | Bridge on PS5: USB only; verify input, motion, touch, rumble, hot-unplug, native DualSense coexistence. | R2 + console |
| R3.4 | Passive finding review: if R3.1 finds no GAP/GATT export, close B1–B6 as **negative** and do not schedule active work. | R3.1 |

R3.1 must be a *separate* approved run with the exact build and checksum, not a side effect of installing 1.5.2. A failed/unreadable node is a valid `BLOCKED` result, not a reason to retry with `O_RDWR`.

### Phase 4 — active native BLE (not requested; separate written approval)

Only if R3.1 produces a positive, documented OS-managed LE interface. If it does not, the correct decision is **NO-GO**, not "try raw HCI anyway".

If ever approved, the minimum protocol is: sacrificial console + explicit recovery, one HCI interface, no bond deletion, DualSense sampled continuously, abort on the first dropout or privilege change, and a written report even on failure. Expected outcome based on current evidence: the stock stack either refuses the interface or loses the radio; a self-written L2CAP/ATT/SMP central would be required, with no coexistence proof. This is a research dead end relative to the bridge, not a shortcut.

### Phase 5 — platform features (parallel, no radio)

| Order | Feature | Why | Gate |
|---|---|---|---|
| 1 | Publish motion (`quat/velocity/acceleration`) | biggest gap vs a real DualSense; fields already exist | one game test + calibration check |
| 2 | Publish two trackpad contacts + click | native touch instead of stick emulation | one game/UI test |
| 3 | Haptics quality: coalescing, 50 ms refresh, trigger-feel preview | haptics are the entire felt difference; resistance is impossible | host + console |
| 4 | Rename "adaptive triggers" to "trigger feel / haptic detents" in UI/docs | prevents a false capability claim | docs only |
| 5 | Mic-mute mapping | small, but bit index is unconfirmed | capture a real DS report first |

Lightbar, player LEDs, speaker audio and force-feedback triggers stay out of scope: the virtual pad has no setter for them, and adding one would require either real DualSense HID emulation or ShellUI patching, both of which conflict with the "preserve native DualSense" rule.

## 4. Interfaces

### 4.1 Transport contract

```c
typedef enum { STEAM_TRANSPORT_USB, STEAM_TRANSPORT_PUCK,
               STEAM_TRANSPORT_BLE_BRIDGE } steam_transport_t;

typedef struct {
    steam_transport_t kind;
    uint16_t vid, pid;
    char logical_id[32];   /* stable across transports */
    char source_id[32];    /* current path/peer, diagnostic only */
} transport_identity_t;

typedef struct {
    /* 1 = one complete wire frame; 0 = none within wait_ms; <0 = failure. */
    int  (*read_frame)(void *ctx, uint8_t *buf, size_t cap,
                       size_t *len, unsigned wait_ms);
    int  (*write_report)(void *ctx, const uint8_t *buf, size_t len);
    int  (*identity)(void *ctx, transport_identity_t *out);
    void (*cancel)(void *ctx);   /* idempotent; unblocks read_frame */
    void (*close)(void *ctx);
} transport_ops_t;
```

`usb_session` stays USB-specific (endpoint multiplexing, COMPLETE-any-endpoint accounting, EAGAIN/EBUSY). The adapter above it produces whole frames; BLE/bridge reassembly lives in its adapter, never in `steam_protocol`.

### 4.2 Identity state machine

```text
DISCOVERED -> CLAIMED -> CREATED(handle) -> ACTIVE(source)
                                   |            |
                                   |            v
                                   +--------> IDLE(neutral, bounded)
                                                |
                                                +--> RELEASING -> FREED
```

Rules: one logical controller → at most one slot; transport is an attribute, not part of the key; a second source for the same `logical_id` is refused while `ACTIVE`; `IDLE` injects neutral frames and deletes once on expiry; logout, power rest and payload stop force `RELEASING`; the existing `virtual_blocked` latch is never bypassed.

### 4.3 Bridge protocol (draft)

USB vendor interface, 64-byte max packet, little-endian:

| Message | Direction | Payload |
|---|---|---|
| `HELLO` | bridge → host | version, capabilities, bridge serial hash (no MAC) |
| `INPUT` | bridge → host | seq, timestamp ms, report id, report bytes |
| `STATUS` | bridge → host | link state, battery, last error |
| `RUMBLE` | host → bridge | type, intensity, left/right speed+gain |
| `RESET` | host → bridge | stop output, drop input, re-arm scan |

CRC-16/CCITT over header+payload; duplicate `seq` dropped; a gap publishes one neutral frame; a 500 ms silence is a link loss. The bridge owns pairing material; the payload never sees keys or addresses.

## 5. Test plan

| Layer | Host | Console |
|---|---|---|
| Decode/decision split | golden 0x42/0x45/0x47 frames, truncation, battery, wireless events, parity | one game, one controller |
| Transport adapter | fake frames, partial/duplicate/oversized, cancel during read, write failure | USB unplug/replug |
| Identity | keep handle across unbind/rebind, expiry neutral+delete, stale token, ambiguous Add latch | USB-C↔Puck mid-game; pause/dialog/LED observation |
| Bridge | frame replay, CRC/seq faults, rumble ordering, link-loss neutral | PC bench, then PS5 |
| Platform | motion/touch encoding vectors | one game that reads DS motion/touch |
| Passive B1 | n/a | symbol/node/descriptor inventory, DualSense before/after |

All host tests run under ASan/UBSan; lifecycle and state tables get TSan. Hardware rows stay `NOT_RUN` until executed.

## 6. Risks

| Risk | Severity | Mitigation |
|---|---|---|
| Holding a virtual pad while no source is bound becomes a phantom player or blocks assignment | High | bounded hold (10 s start), neutral frames, forced release on logout/power/stop, per-game test |
| Game pauses or re-prompts anyway on transport change | High | R3.2 measures it; fall back to delete+recreate with a clear UI message, never silently |
| Bridge USB ID/driver behavior on PS5 is unknown | Medium | dedicated vendor ID, bench on PC first, PS5 test is a separate gate |
| BLE Just Works has no MITM protection | Medium | document proximity threat; no secrets in payload; bond on bridge only |
| BLE and Puck personalities are exclusive | Medium | UI must say "wireless mode"; no claim of simultaneous links |
| Blocking bridge read hangs shutdown | High | `cancel()` contract + bounded `read_frame`; join test |
| Report 0x47 vs 0x45 differences (timestamps) break decode | Medium | accept both; strip only an exact matching prefix; fixture tests |
| Scope creep into LEDs/speaker | Medium | explicit non-goals; no ABI invention |
| AGPL/MIT contamination | Medium | MIT `steamcontroller32` facts only; no code from AGPL/no-licence repos |

## 7. Decisions requested

1. **Approve Phase 1** (host-only refactor + identity + fake bridge). No radio, no console, normal CI gates.
2. **Approve or defer Phase 2** (external ESP32-S3 bridge prototype). Requires hardware and a bench host, not a PS5.
3. **Approve Phase 3.1/3.2** as a later, separately scheduled console run with an exact ELF/checksum and recovery plan.
4. **Do not approve Phase 4 now.** Active HCI/radio work has no positive evidence to justify it and a clear path to losing the stock DualSense.

## 8. Sources

| Source | Pin | Use |
|---|---|---|
| ps5-payload-dev/sdk | `d9c9519116944a7f1c22d262012c53b80b3520b7` | no BT stub/header; FreeBSD netgraph headers; `libScePad.c` bare symbols |
| SvenGDK/SharpProspero | `5dfdb4b0b166e83dc9ae275001afb695f12c7174` | 17 BluetoothHid exports, HID-only |
| DNNDHH/PS5-3.20_Libs | `b2a77c09cd71c01f34c790cfbe98c751a13262ad` | NID list |
| diegobarbosaa/OmniPad-PS5 | `b7682efce18ee0a8d7c80bbc90afbce72151ef96` | raw-HCI code, not built |
| btstack-dev thread | 2025-03-09 | BLE-only keyboard rejected (`CE-109524-1`) |
| inonomori/steamcontroller32 | `2b20075265d8c5736d64c5754583edb889726b8f` | SC2 BLE protocol, Just Works, rumble, ESP32 hardware report (MIT) |
| reo101/SteamlessLink | `1e66096d41820baff6600c468d67dbba6f177ced` | Android GATT behaviour (no licence) |
| mwdmwd/sc26re | `89d4d00410774d34908038f2c34c327b4884e359` | CFW GAP name, personality switching (AGPL, facts only) |
| libsdl-org/SDL | `8ea563bce3cc852967ed5a713d853e1de2b6d0cc` | Triton report/output structures |
| torvalds/linux | fetched 2026-11-27 | HOGP handling, report sizes (GPL-2.0, reference only) |

## 9. Negative results to preserve

- No public GAP/GATT Central export for ProsperoOS was found.
- No public project has scanned, connected or read GATT through `libSceBluetoothHid`.
- No SDK header or stub for `libSceBluetoothHid` exists at the pinned SDK.
- No caller-controlled virtual-pad identity (name/MAC/serial) exists in the public ABI.
- No public virtual-pad project preserves a handle across a transport change.
- No virtual-pad setter exists for lightbar, player LEDs or speaker audio.
- No hardware test of any phase in this document has been run.

These are evidence about the public surface, not proof about 13.60 internals. Only R3.1 can narrow that gap, and only with explicit approval.
