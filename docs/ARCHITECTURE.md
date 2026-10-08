# Architecture contracts — specialization without monolith

## Target dependency direction

```text
Official Steam Controller 2026 (USB-C / Valve Puck / BLE)
     |
     v
transport adapters (USB transport | Puck adapter | BLE central adapter)
     |
     v
Steam wire protocol decoder/encoder  <- transport-specific packet envelope
     |
     v
Steam session and physical identity model
     |
     v
Extended canonical controller state (raw + normalized, timestamped)
     |
     +--> profile/action layers / motion / gyro aim / touch / flick stick
     |                                  |
     |                                  v
     +----------------------------> slot + user manager
                                        |
                                        v
                              PS5 virtual DualSense backend ----> game/UI
                                        ^
                                        |
 game output/feedback source --> typed feedback engine --> Steam output --> transport

Web UI ---> authenticated application service ---> profile/session/slot APIs
Diagnostic recorder ---> safe snapshots of each layer, no bypass
```

## Contracts / API principles

`Transport`: own connection/hotplug; send/receive bounded immutable binary frames, timestamp and errors; no Steam protocol semantics. `DeviceIdentity` cannot be only USB path or transient BLE address. Persist pairing material securely and never log secrets.

`SteamProtocol`: parse versioned reports into typed `SteamRawState`, validate frame/report ID/length/endianness/signed motion fields. Emit `SteamOutputCommand`; pure decoding and host golden-vector tests. Supports USB/Puck/BLE without duplicating core parsing.

The first pure decoder, `payload/steam_protocol.c`, now returns typed SC input, battery, and wireless-link reports. Truncated known input reports retain their input classification so the adapter can publish a neutral state instead of leaving stale buttons/touch held. `payload/steam_usb.c` validates complete USB configuration descriptors for Puck and wired endpoint discovery. Active endpoint selection requires a complete decoded input report; the HID usage page is not yet independently checked because no target/controller descriptor capture or compiled control-transfer implementation is available.

Small platform-independent operations are factored into `steam_mapping.c` (axis/trigger, d-pad and trackpad transforms), `steam_motion.c` (unit conversion, bias handling and quaternion integration), and `steam_feedback.c` (bounded pulse/rumble report serialization). `webui_json.c` supplies bounded JSON string/number output to the authenticated portal. These pure modules have host tests; the current PS5 adapter still owns inherited profile and `ScePadData` mapping state, so a complete canonical transport/session/slot architecture is not yet implemented. Do not infer PS5 behavior or hardware support from helper tests.

Numeric profile fields use checked `strtol` parsing with full-input and `int`-range validation; malformed or overflowing values leave the prior/default field intact. Combo parsing uses a call-local tokenizer so the profile watcher and authenticated portal can parse concurrently without sharing `strtok` state. Host tests cover numeric bounds and concurrent parsing. This protects the parser used by saved profile files and authenticated profile writes, without claiming profile-transition or browser integration tests.

`SteamSession`: controlled state machine `DISCOVERED → INITIALIZING → READY → STANDBY → RECONNECTING → CLOSED` plus failure transitions; lizard suppression, heartbeat, mode, battery, capability flags, cleanup. A session lease binds transport and physical identity without owning the PS5 virtual slot directly.

`ControllerState`: monotonic time, independent raw and normalized axes/buttons, independent rear controls, per-trackpad contacts+click and calibration, gyro[3], accel[3] with units and reference frames, battery, charging, optional flags. Keep unknown/missing fields marked absent; avoid overwriting good state with malformed input.

`Mapping`: stateless evaluation + bounded per-profile transition state (hold/double/chords/layers/gyro/gesture). Pure deterministic processing given timestamps and previous state. State transitions must release prior mapped buttons upon profile change/disconnect.

`SlotManager`: exactly one authoritative owner of physical-to-virtual identities; transactional claim/rebind/release, no duplicate virtual pad when switching USB↔Puck↔BLE, multi-user safety, handle generations to reject delayed packets after reattach; hand-off is explicit mode rather than implicit collision. Coexistence with native DualSense. The current source still uses the inherited slot table and lacks generation IDs; VDA creation is serialized because its identity is correlated through one shared klog queue.

`PS5Backend`: isolated `VirtualDualSense` default; firmware-specific ShellUI bridges/hooks behind opt-in capability checks. Never let generic input modules call `libScePad` or ptrace. Backend can expose `CAP_TOUCH`, `CAP_IMU`, `CAP_FEEDBACK` only if implemented and tested.

`Feedback`: typed intensity/time/effect events, routing by current slot generation, safe intensity bounds, haptic/rumble fallbacks, output coalescing and cancellation. No feedback loop calling transport under a slot lock. No physical resistance on Steam triggers; can translate signals to haptic events.

`WebControl`: nonblocking, authenticated mutations through application layer; documented schema with version, error codes, limits, profile data migrations. No raw input socket enabled in release. Avoid conflicting web ports if selected base already has them.

`HAL`: dependency injection or light function-table wrappers only where a real test/platform seam exists (clock, USB, BLE transport, socket, storage, OS virtual pad). Avoid pointless generic factories or unused backend interfaces.

## Invariants to test

1. One physical controller -> at most one active virtual device generation; every disconnect frees all outputs.
2. Every packet validated before state update; no stale pressed input after transport timeout.
3. One normalized extended input format across transports with consistent coordinate and time units.
4. Output feedback never delivered to a disconnected/rebound controller.
5. No HTTP/TCP action bypasses authorization, ownership or domain validation.
6. If motion/haptics/hooks are absent, basic buttons/sticks work with safe fallback.
7. User profile/slot operations are atomic relative to incoming input and USB disconnect.
8. Shutdown is idempotent and preserves the native DualSense path.

## Extension requirement

A new controller in the future should require a new parser+capability adapter (and perhaps transport profile), **not** another slot manager, PS5 backend, web server, haptics router or pairing stack. A new transport should require a transport adapter, not a copy of the Steam report decoder or gyro engine. Prefer extending the best existing module rather than creating duplicate `*_v2` implementations.
