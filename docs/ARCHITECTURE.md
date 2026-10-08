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

`usb_resources.c` owns the USB reader's descriptor, FS-init state, opened endpoint indexes, and possibly active transfers. `usb_session.c` routes reader open/init/endpoint-open/start/complete calls through a small injected operation table, then records successful ownership transitions; teardown stops active transfers, closes endpoints in reverse order, uninitializes only a successfully initialized FS session, and closes the descriptor once. Failed START and COMPLETE operations remain eligible for STOP during cleanup. Host callbacks inject open, init, endpoint-open, start and completion failures; the real PS5 ioctl adapter is cross-build-only. The separate SC2 candidate scanner still owns its temporary endpoint operations, while `usb_send_out` and SC2 haptic helpers perform their own best-effort STOP on completion error/timeout. None of those device paths has been run on hardware.

Small platform-independent operations are factored into `steam_mapping.c` (axis/trigger, d-pad and trackpad transforms), `steam_motion.c` (unit conversion, bias handling and quaternion integration), and `steam_feedback.c` (bounded pulse/rumble report serialization). `webui_json.c` supplies bounded JSON string/number output to the authenticated portal. `slot_registry.c` is the one authoritative owner of live slot metadata: session key, VID/PID, lifecycle state, generation and virtual-handle ownership. The manager claims a session and rejects duplicate live keys atomically; its generation token travels into the USB reader, which snapshots, attaches, detaches and releases only that generation. Registered VDA handles are unique across slots. The backend deletion callback runs outside the registry lock; on failure the registry restores ownership, moves the slot to `RELEASING`, and the manager retries cleanup on later scans. `slot_lifecycle.c` owns the reusable startup/rollback/release transaction now called by `gc_main.c`: reserve tokens must be active before worker startup, failed worker creation removes only registry-owned VDA handles, and delete failure leaves the generation unavailable for a later retry. Host fault-injection covers VDA-create failure, worker-start failure, ambiguous handles, lazy/eager startup and delete retry; a four-thread stress test exercises 4,000 lifecycle cycles. The target scan loop, PS5 callbacks and USB I/O remain cross-build-only. Profile and `ScePadData` mapping state, USB I/O and VDA calls remain in the inherited PS5 adapter; this is not yet the canonical transport/session architecture. A `/dev/ugen*` path is only a live-session key, not a stable physical identity, and no USB↔Puck↔BLE rebind continuity is claimed. Do not infer PS5 behavior or hardware support from host/build tests.

Numeric profile fields use checked `strtol` parsing with full-input and `int`-range validation; malformed or overflowing values leave the prior/default field intact. Combo parsing uses a call-local tokenizer so the profile watcher and authenticated portal can parse concurrently without sharing `strtok` state. Binding activator state is caller-owned per slot; initialization, reset epochs, and active-profile generation changes clear pending state, seed physically held inputs, and ask the adapter to publish one fully neutral pad report. Pause state uses atomic load/exchange, while external menu-close callers set a request that the input thread applies to menu-owned state. Host ASan/UBSan tests cover numeric bounds, concurrent parsing, profile-generation changes, long press, asynchronous reset requests, session isolation, and atomic pause-state access; ThreadSanitizer passes the profile-parser, binding-reset, and pause-state concurrency targets. They do not validate full profile persistence/browser integration, menu/input-thread integration, or the PS5 adapter's `ScePadData` publication.

The assignment dialog gate remains a separate atomic lease token containing a slot and generation. Only the exact token that acquired the gate can release it, so a delayed USB reader cannot clear a newer dialog after its previous lease timed out. Portable tests cover that lease, the slot registry, the shared lifecycle transaction's create/start rollback, ambiguous-handle protection, failed-delete recovery, concurrent startup/release, and USB session operation failures. The registry supersedes the inherited volatile slot table. The actual PS5 scan loop and ioctl adapter are not host-executed; stable physical identity, cross-transport rebind, kernel reader interruption and target runtime behavior still need validation.

`SteamSession`: controlled state machine `DISCOVERED → INITIALIZING → READY → STANDBY → RECONNECTING → CLOSED` plus failure transitions; lizard suppression, heartbeat, mode, battery, capability flags, cleanup. A session lease binds transport and physical identity without owning the PS5 virtual slot directly.

`ControllerState`: monotonic time, independent raw and normalized axes/buttons, independent rear controls, per-trackpad contacts+click and calibration, gyro[3], accel[3] with units and reference frames, battery, charging, optional flags. Keep unknown/missing fields marked absent; avoid overwriting good state with malformed input.

`Mapping`: stateless evaluation + bounded per-profile transition state (hold/double/chords/layers/gyro/gesture). Pure deterministic processing given timestamps and previous state. State transitions must release prior mapped buttons upon profile change/disconnect.

`SlotManager`: exactly one authoritative owner of live-session-to-virtual-slot metadata; transactional claim, activate, handle attach/delete, and release; generation tokens reject delayed operations after slot reuse. `slot_registry.c` enforces duplicate session-key and virtual-handle rejection, requires an explicit releasing state, preserves handles after backend delete errors, and is covered by portable concurrency/failure tests. USB path keys are ephemeral, so identity continuity across disconnect or transport switch, multi-user behavior, and zero duplicates on physical hardware remain unverified. VDA creation is serialized because its identity is correlated through one shared klog queue; the assignment-dialog lease is a separate concern.

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
