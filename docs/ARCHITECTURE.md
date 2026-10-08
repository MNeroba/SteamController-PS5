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

`usb_resources.c` owns the USB reader's descriptor, FS-init state, opened endpoint indexes, and possibly active transfers. `usb_session.c` routes reader open/init/endpoint-open/start/complete calls through a small injected operation table, then records successful ownership transitions; teardown stops active transfers, closes endpoints in reverse order, uninitializes only a successfully initialized FS session, and closes the descriptor once. Failed START and COMPLETE operations remain eligible for STOP during cleanup. Host callbacks inject open, init, endpoint-open, start and completion failures; the real PS5 ioctl adapter is cross-build-only. The separate SC2 candidate scanner still owns its temporary endpoint operations, but `sc2_find_active_ep()` now keeps report buffers on each invocation's stack so simultaneous Puck/wired scans cannot overwrite each other's reports. A host ioctl-stub regression reproduces the old contamination and checks independent classification; it does not exercise the PS5 USB stack. `usb_send_out` and SC2 haptic helpers perform their own best-effort STOP on completion error/timeout. None of those device paths has been run on hardware.

The optional game hooks stay in the game process's own mapped memory so a payload restart can reuse an identical layout. Each new layout records its gateway address; rollback restores a slot only when it still points at that gateway, treats an already-restored slot idempotently, and leaves a later owner's hook untouched. Read/write failures retain the old target ownership and retry; unknown/conflicting slot owners are not overwritten. The payload refreshes an audio-silence heartbeat with each hook-manager update, and game-side callbacks expire that mute lease after 120 stale output callbacks; merged input already ages out after 60 pad reads. A game process exit releases its own mapping. This is source/host-policy and SDK-build evidence only: no live game, cross-module hook race, explicit payload-stop behavior, or PS5 rollback has been exercised. In particular, a pointer check followed by a remote write is not an atomic compare-and-swap, so target concurrency still needs a safe console test.

Small platform-independent operations are factored into `steam_mapping.c` (axis/trigger, d-pad and trackpad transforms), `steam_motion.c` (unit conversion, bias handling and quaternion integration), and `steam_feedback.c` (bounded pulse/rumble report serialization). `webui_json.c` supplies bounded JSON string/number output to the authenticated portal. `slot_registry.c` is the one authoritative owner of live slot metadata: session key, VID/PID, lifecycle state, generation and virtual-handle ownership. The manager claims a session and rejects duplicate live keys atomically; its generation token travels into the USB reader, which snapshots, attaches, detaches and releases only that generation. Registered VDA handles are unique across slots. The backend deletion callback runs outside the registry lock; on failure the registry restores ownership, moves the slot to `RELEASING`, and the manager retries cleanup on later scans. `slot_lifecycle.c` owns the reusable startup/rollback/release transaction now called by `gc_main.c`: reserve tokens must be active before worker startup, failed worker creation removes only registry-owned VDA handles, and delete failure leaves the generation unavailable for a later retry. Host fault-injection covers VDA-create failure, worker-start failure, ambiguous handles, lazy/eager startup and delete retry; a four-thread stress test exercises 4,000 lifecycle cycles. The target scan loop, PS5 callbacks and USB I/O remain cross-build-only. Profile and `ScePadData` mapping state, USB I/O and VDA calls remain in the inherited PS5 adapter; this is not yet the canonical transport/session architecture. A `/dev/ugen*` path is only a live-session key, not a stable physical identity, and no USB↔Puck↔BLE rebind continuity is claimed. Do not infer PS5 behavior or hardware support from host/build tests.

Numeric profile fields use checked `strtol` parsing with full-input and `int`-range validation; malformed or overflowing values leave the prior/default field intact. Combo parsing uses a call-local tokenizer so the profile watcher and authenticated portal can parse concurrently without sharing `strtok` state. Binding activator state is caller-owned per slot; initialization, reset epochs, and active-profile generation changes clear pending state, seed physically held inputs, and ask the adapter to publish one fully neutral pad report. Pause state uses atomic load/exchange, while external menu-close callers set a request that the input thread applies to menu-owned state. Host ASan/UBSan tests cover numeric bounds, concurrent parsing, profile-generation changes, long press, asynchronous reset requests, session isolation, and atomic pause-state access; ThreadSanitizer passes the profile-parser, binding-reset, and pause-state concurrency targets. They do not validate full profile persistence/browser integration, menu/input-thread integration, or the PS5 adapter's `ScePadData` publication.

`sc2_status.c` owns the cross-thread diagnostics snapshot shared by USB/input workers, the manager, and the portal. Input mask/connection, battery level/state and warning thresholds, haptic availability/method/endpoint, and haptic counters are read or updated under one mutex; readers receive a coherent value copy. This replaces the former shared `volatile` telemetry globals and keeps warning hysteresis in the same critical section as battery updates. Host stress validates 100,000 concurrent snapshots under ThreadSanitizer and ASan/UBSan. The snapshot represents the current active Steam Controller path; it does not establish simultaneous multi-controller telemetry semantics.

`sc2_input_state_t` is owned by one USB reader session and carries that session's binding activators, flick-stick accumulator/frame timing, adaptive-trigger zone and buzz-expiry history, QAM hold latch, and menu-action edge history. The previous flick-stick, trigger-effect and button-edge fields were function statics, so two SC reader threads could consume or overwrite one another's input history. `sc2_input_state_reset()` initializes and clears the complete context at reader start, malformed-input neutralization, disconnect, and exit. Regressions reproduced both cross-session flick and trigger-effect contamination, then validated independent interleaved state and concurrent reports under ASan/UBSan and ThreadSanitizer. Trigger effect commands and enable state remain process-wide configuration guarded by a mutex; each reader snapshots the configuration and mutates only its own runtime state. The haptic service now passes each caller's extra-OUT count directly to that send and clamps it to the endpoint-array bound; its log counters are serialized. The pulse queue and rumble target remain process-wide, so independent simultaneous-controller feedback delivery is not established. The profile selector, menu, gyro integration, and status snapshot also retain process-level semantics.

The DualSense hand-off settings/session/release flags also use their existing mutex on every access, including hot-path checks. The hand-off test reproducer previously reported ThreadSanitizer races for enable and release state; the corrected path passes 60,000 concurrent JSON/session snapshots. The bridge-probe force request now uses its state mutex, and adaptive-trigger enable state is read/written under the effect mutex. These source-level race fixes have not been exercised on a PS5.

`game_hooks.c` protects settings, active state, owner selection, pad input and JSON-visible status with its shared mutex. The worker takes one coherent settings snapshot per iteration; settings persistence is serialized separately so concurrent UI changes cannot leave an older snapshot on disk. Its 61,000-operation host worker test passes ThreadSanitizer with mocked process/bridge calls. `sc2_gyro.c` protects the IMU/calibration/motion/reset/status state with one mutex, and serializes its USB-service retry state separately; callers use synchronized moving-state accessors rather than a shared `volatile` flag. The gyro state stress passes 75,000 operations under ThreadSanitizer. These tests do not execute PS5 USB ioctls, game process memory access, or console behavior.

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
