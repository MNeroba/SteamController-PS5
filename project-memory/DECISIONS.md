# Architecture decisions

| ADR | Status | Subject | Current decision |
|---|---|---|---|
| ADR-001 | ACCEPTED 2026-10-08 | Choose Puckbridge vs OmniPad vs derivative | Puckbridge pinned at `78460113ef7c48502c765a1f83ba34d53c977134` is the executable foundation; OmniPad pinned at `b7682efce18ee0a8d7c80bbc90afbce72151ef96` is reference only. See `docs/BASE_SELECTION_ADR.md`. |
| ADR-002 | ACCEPTED (design constraint) | Normal virtual DualSense as production backend | Keeps PS5 compatibility; Edge research separate |
| ADR-003 | ACCEPTED (design constraint) | One input/protocol/session/slot architecture | Steam focus with reusable shared subsystems; no duplicate payloads |
| ADR-004 | ACCEPTED (safety constraint) | BLE hardware path | Passive first; native BLE only after B0–B6 evidence and operator-approved active tests |
| ADR-005 | ACCEPTED (release constraint) | USB/Puck release independent from BLE | No requirement to solve native BLE before shipping tested wired/Puck build |
| ADR-006 | ACCEPTED (licensing constraint) | Restricted and incompatible code | No confidential Sony SDK or unapproved BTstack reuse in public GPL build |

For any new significant design decision: date, status `PROPOSED/ACCEPTED/SUPERSEDED`, options, source evidence and SHA, decision/negative consequences, test gate and reconsideration trigger. Do not silently rewrite an accepted ADR; add a superseding entry.

## New decisions

| ADR | Date | Status | Decision |
|---|---|---|---|
| ADR-001 | 2026-10-08 | ACCEPTED | Use pinned Puckbridge as foundation because it has the actual Steam Controller 2026 USB/Puck parser and PS5 mapping route. OmniPad's slot and USB lifecycle remain reference material only; its web, TCP and Bluetooth code are not imported. Reconsider only on the source/rights/behavior triggers in `docs/BASE_SELECTION_ADR.md`. |
| ADR-007 | 2026-10-08 | ACCEPTED | Keep one portable `slot_registry` as the authoritative owner of live session keys, lifecycle, generation and VDA handles; use `slot_lifecycle` for shared startup/rollback/release and pass generation tokens into the PS5 USB worker. Require explicit `RELEASING`, perform backend deletion outside the lock, restore ownership on failure, and retry cleanup before reusing a slot. Do not import OmniPad's registry or keep a second `gc_main.c` slot table. The USB path is an ephemeral key; no stable-device or cross-transport rebind claim. Evidence: lifecycle/registry tests M1-33–35 and SDK v0.43 cross-build M1-36. Reconsider if target ABI integration requires a different synchronization primitive, or if verified persistent USB identity becomes available. |
| ADR-008 | 2026-10-08 | ACCEPTED | Keep USB reader resource ownership in portable `usb_resources`/`usb_session` modules with a narrow injected operation table used by `gc_main.c`. Track descriptor, successful FS init, endpoint indexes, and possibly active transfers; mark START as possibly active before invoking the platform callback so failures still trigger STOP. On failed FS_INIT, do not issue UNINIT without confirmed initialization; close the descriptor once. Host tests inject adapter operation failures, while PS5 ioctl behavior remains cross-build-only. Reconsider only if documented platform semantics establish safe cleanup for partially failed init or if more USB paths are moved under the same seam. Evidence: F038, M1-40–42. |
| ADR-009 | 2026-10-08 | ACCEPTED | Keep game hooks opt-in and resident only inside the game-owned mapping so an identical payload layout can resume them after restart. Store the expected gateway per import slot; restore only when a fresh read still equals that gateway, leave another hook's value untouched, and retry unreadable/unrestored owned slots. Old v5 pages must have input/audio disabled and known gateway slots restored before installing v6; abort migration if safe restoration cannot be established. Audio muting is a producer heartbeat lease and expires after 120 stale output callbacks; input already expires after 60 reads. This avoids stale behavior on payload death without unmapping code from another process. The read-then-write ownership check is not atomic, and cross-process/hardware shutdown remains unverified; no race-free or complete rollback claim. Evidence: F039 and 16 host policy/heartbeat checks in M1-45–47; implementation published in `2e24ac196f52cb850ba3df6de328a5c0c198e452`. Reconsider if a documented remote compare-and-swap or cooperative payload shutdown API becomes available. |
| ADR-010 | 2026-10-08 | ACCEPTED | Protect shared game-hook configuration/owner/active values and gyro input/motion/reset/status with mutex-backed APIs. Each hook-worker pass uses a coherent immutable settings copy; settings file writes serialize and then read the newest settings to avoid stale last-writer persistence. The gyro USB-service retry counters use a separate mutex and do not hold the IMU state mutex across ioctls. Host TSan regressions cover actual compiled modules with the target bridge/USB calls stubbed. Keep kernel/USB/process calls outside shared state locks where practical; target responsiveness and PS5 behavior remain a build/hardware gate. Evidence: F041–F042, M1-52–55. Reconsider if measured target latency or a verified scheduler/API contract requires finer-grained state. |
| ADR-011 | 2026-10-08 | ACCEPTED | Keep Steam packet-history state caller-owned per USB session. `sc2_input_state_t` groups binding activators, flick-stick turn/timing state, QAM hold timing/latch, and menu-action edge history; initialize/reset it with the reader lifetime and pass it through the existing adapter. This fixes actual multi-reader state contamination without a shared lock or duplicating transport/mapping code. It does not make inherited profile, menu, gyro, haptic, or telemetry semantics multi-controller-safe. Evidence: failing interleaved-session regression M1-58, fixed ASan/UBSan + TSan suite M1-59–60, F043, implementation commit `1682e7806ceff3dc5d0b43b8f1927ddf499a0818`. Reconsider group boundaries only if more per-session state is moved from the adapter. |
| ADR-012 | 2026-10-08 | ACCEPTED | Store adaptive-trigger pull-zone and vibration-expiry history in the same per-reader `sc2_input_state_t` used by the USB session. Keep effect commands and active enable process-scoped because the game hook owns one active DualSense effect configuration; snapshot that configuration under its existing mutex, then apply it without holding the lock across the haptic callback. Reset the reader context at startup, disconnect, malformed-input neutralization, and when effects are inactive. This prevents separate readers from generating false trigger edges or suppressing one another's vibration without making the shared game-hook config or haptic queue multi-controller-safe. Evidence: M1-64 expected-red test, M1-65–67 fixed ASan/UBSan, TSan, and SDK build; F044. Reconsider the configuration scope only after verifying multi-pad game-hook semantics on PS5. |
| ADR-013 | 2026-10-08 | ACCEPTED | Keep each haptic service invocation's extra-OUT endpoint count local to its report send; clamp it to `SC2_HAP_MAX_OUT`. Serialize shared report/service log-throttling state separately from the haptic queue lock. This removes cross-reader route-count contamination and races without holding the queue lock during USB I/O or logging. The process-wide pulse queue and rumble target remain unchanged and do not establish independent multi-controller haptic routing. Evidence: M1-69 expected-red behavior, M1-70 TSan race, M1-71–74 fixed regression/sanitizer/build results, F045; published in draft PR #1 commit `a902be57084fd90e70672c3fbf84db8a7a297f1c`. Reconsider queue scope only after producer APIs carry a verified session/slot owner and hardware requirements are known. |
| ADR-014 | 2026-10-08 | ACCEPTED | Do not add an implicit global "active haptic session" pointer or retarget the queue based on whichever USB reader happens to run. Current manager code can host multiple lazy SC2 readers, while menu/game/portal producers have no session identity. Any routing fix must define the active-controller policy and carry an explicit generation-tagged session/slot owner to enqueue and service APIs, or explicitly gate the product to one SC2 session. Evidence: F046. Until selected, preserve P1-HAPTIC-26 as open and do not claim multi-session haptic correctness. |
| ADR-015 | 2026-10-08 | ACCEPTED | Every concurrent `sc2_find_active_ep()` call owns its report buffers for the whole USB scan; do not use function-static storage for endpoint completion data. The manager may run wired and Puck discovery concurrently, and each call classifies mutable completion buffers after returning from ioctl. A deterministic host stub reproduced cross-call contamination with static storage; per-call stack buffers pass the regression. This changes only scanner scratch ownership, not the USB session/resource contract. Evidence: F047 and M1-78–84; scanner fix published in PR commit `b6f5bae3f6cb2c45be0e798c10b6cf2345ce2edf`. Reconsider only if the bounded stack footprint becomes unsafe on the PS5 worker stack; the current allocation is `SC2_MAX_EPS * 64` bytes. |
| ADR-016 | 2026-10-08 | ACCEPTED | `json_title()` must own a bounded input buffer per call. The profile watcher can resolve the active title while the portal lists games; a 64 KiB function-static buffer made both calls parse whichever file was read last. Allocate one 64 KiB buffer, parse through a pure helper, and free it on return. This adds a small bounded allocation per metadata lookup. Evidence: F048 and M1-86–90; published in commit `dfd8cc9baaf4b49849dc27451d0fffca956c88ed`. Reconsider if target memory pressure is measured; do not replace it with process-global mutable scratch. |
| ADR-017 | 2026-10-08 | ACCEPTED | One-time diagnostic throttles used by concurrent workers must use an explicit synchronized claim. `inject_pad()`'s local static error flag allowed simultaneous failures to race and duplicate the supposedly once-only log. `gc_diag_once_t` now serializes the claim and leaves the PS5 log I/O outside the helper lock. Evidence: F049 and M1-92–97. Reconsider only if console latency measurements show the short mutex is material. |
| ADR-018 | 2026-10-08 | ACCEPTED | Legacy wired Steam left-stick history belongs to one USB reader session. The report's bytes 16–19 alternate between left-stick and left-pad data, so the adapter must retain the last real stick sample; use a small caller-owned `steam_legacy_input_state_t` inside `usb_session_state_t`, initialized with that session, rather than function-static process state or a global lock. Evidence: F050 and M1-99–105; implementation published in PR commit `9b22caa2d633af4bc66b0d5e3c93772ca5e80ce3`. This isolates this legacy decoder's stick history only; it does not change Steam Controller 2026's `sc2_input_state_t` or resolve process-wide profile, menu, gyro, status, or haptic queue ownership. Reconsider only if the adapter's session ownership model changes. |
| ADR-019 | 2026-10-08 | ACCEPTED | Constrain active Steam Controller 2026 haptic service to one USB/Puck reader until producer and shared-state APIs have a verified multi-session contract. The manager claims an exclusive `slot_registry_token_t`; a second SC2 reader is rejected, service calls must present the owner generation, and teardown clears the queue/rumble state and best-effort sends a zero-rumble report before closing USB. Targetless producer APIs are safe only under this enforced singleton policy. Evidence: F051, T-P30, M1-106–110. Tradeoff: simultaneous wired and Puck SC2 use is unavailable. Manager/USB runtime and hardware remain unverified; revisit only with explicit routing identity through all producers and tested lifecycle semantics. |
| ADR-020 | 2026-10-08 | ACCEPTED | Compute profile selection outside `g_sel_lock`, but commit the loaded profile and selected ID only if a protected selection generation is unchanged. Advance the generation for override, explicit reload, title and title-name mutations; hold `g_sel_lock` across generation revalidation and `sc2_active_set()` so a completed newer request cannot be overwritten by an older load. Evidence: F052/T-P31/M1-113–117. This avoids keeping the UI/input state mutex held over storage I/O. Revisit if selection moves into a session object or storage operations become asynchronous. |
| ADR-021 | 2026-10-08 | ACCEPTED | Serialize profile save/delete mutations with a storage mutex around the fixed temporary file, close and atomic rename; keep loads outside the mutation lock so readers can observe either complete version. Evidence: F053/T-P32/M1-120–128; 22 host checks and targeted TSan. Concurrent stale snapshots remain last-writer-wins. |
| ADR-022 | 2026-10-08 | ACCEPTED | The on-console remap path must propagate storage failure to the notification UI and stop before profile reload/success feedback. Evidence: F054/T-P33/M1-130–136; production menu and store tests. |
| ADR-023 | 2026-10-08 | ACCEPTED | The sequential portal request reader uses one absolute monotonic deadline per request, checks it before and after every receive, and bounds each socket wait. An idle timeout alone can be reset indefinitely by a slow client. Evidence: F055/T-P34/M1-139–147; eight deterministic host checks and full ASan/UBSan suite. |
| ADR-024 | 2026-10-08 | ACCEPTED | Parse persisted game-hook settings with checked `strtol`, require full-value conversion, accept booleans only as 0/1 and hook mode only in range. Malformed fields retain safe defaults; numeric out-of-range mode preserves its prior fallback to 1. Evidence: F056/T-P35/M1-148–154; four production-code settings checks and the existing concurrency test. |
| ADR-025 | 2026-10-08 | ACCEPTED | Bound portal response writes with one five-second absolute monotonic deadline spanning all header/body chunks; cap each blocking socket send wait at two seconds. A per-call send timeout does not bound the loop when each partial write makes progress. Evidence: F057/T-P36/M1-156–161; seven production framing checks plus full ASan/UBSan suite. |
| ADR-026 | 2026-10-08 | ACCEPTED | Match complete portal routes and query keys exactly, with allowlists, bounded copies, explicit methods and strict action values. Evidence: F058/T-P37/M1-163–168; 54 route checks. |
| ADR-027 | 2026-10-08 | ACCEPTED | Treat a supplied Origin as defense in depth: require it to match the unique HTTP Host while keeping it optional for non-browser clients and preserving bearer authorization as the `/api/` gate. Require profile POST to include a nonzero Content-Length, nonblank body and declared `text/plain;charset=UTF-8`; other no-body POST actions are unchanged. Evidence: F059/T-P38/M1-172–176; 27 new HTTP checks, 1030 full checks and SDK cross-build. |
| ADR-028 | 2026-10-08 | ACCEPTED | Keep the portal listener on IPv4 for current compatibility, but obtain each accepted peer address and close public, CGNAT, reserved, multicast, unspecified and other nonlocal ranges before reading a request. Allow loopback, RFC1918 and IPv4 link-local ranges for local use. Retain bearer auth for API mutations and optional same-host Origin validation. Evidence: F060/T-P39/M1-180–182; 18 policy checks, 1048 full checks and SDK v0.43 cross-build. Tradeoffs: a routed client using a public source address is rejected, while the listener still binds all IPv4 interfaces; address-scope filtering is not TLS, a firewall, or proof against NAT rewriting. Reconsider if a documented Prospero interface-binding API can preserve the intended LAN use safely. |

## ADR-015 — Per-invocation SC2 endpoint scan buffers

- Date/status: 2026-10-08, ACCEPTED.
- Context: the inherited `sc2_find_active_ep()` used a function-static completion buffer shared across independent scanner threads.
- Evidence: pinned Puckbridge base `78460113ef7c48502c765a1f83ba34d53c977134`, original `payload/controller_sc2.c`; host regression `tests/test_sc2_usb_scan.c` deterministically orders Puck and wired report writes. The expected-red run classified neither endpoint after the other call overwrote the shared report.
- Decision: keep `bufs` automatic per call; preserve separate completion and cleanup behavior.
- Tradeoff: each scan reserves `SC2_MAX_EPS * 64` bytes on its thread stack.
- Verification gate: normal test, ASan/UBSan suite, targeted TSan and SDK cross-build pass; live USB ioctl and stack-budget behavior on PS5 need target hardware.
- Reconsider if target stack measurements show the bounded scratch allocation is unsafe; then use explicit per-session heap/resource ownership, never shared static scratch.

## ADR-016 — Per-call installed-title JSON input

- Date/status: 2026-10-08, ACCEPTED.
- Context: `json_title()` used one mutable static 64 KiB buffer; profile watching and portal game listing can call it from different threads.
- Evidence: host regression `tests/test_game_list_concurrency.c` synchronizes two calls after reading different fixture files. The expected-red result gave both IDs the `Wired Title` string.
- Decision: allocate a bounded 64 KiB buffer per invocation, pass it to a pure JSON-title parser, and free it after parsing.
- Tradeoff: a temporary heap allocation is made per title read; read size remains capped at 65,535 bytes.
- Verification gate: normal, ASan/UBSan and targeted TSan host tests, plus PS5 SDK cross-build. Actual appmeta filesystem access and memory behavior remain target-unverified.
- Reconsider only if measured target memory pressure warrants a different per-call ownership strategy.

## ADR-017 — Synchronized one-time diagnostics

- Date/status: 2026-10-08, ACCEPTED.
- Context: each USB session calls `inject_pad()` independently; the process-static VDA error log flag used an unlocked check-then-set.
- Evidence: a deterministic two-thread reproducer seeded from that pattern returned two logging claims; `tests/test_diag_once.c` validates the reusable synchronized claim against contention.
- Decision: use `gc_diag_once_t` for an atomic-in-intent, mutex-backed one-time claim. Call logging after the claim returns so the helper lock is never held across logging.
- Tradeoff: a short mutex acquisition on the error path only.
- Verification gate: 80,000 concurrent host claims under normal, ASan/UBSan and TSan; PS5 VDA errors were not induced.
- Reconsider only if a supported platform atomic or observed contention justifies a change.

## ADR-018 — Legacy wired Steam state is session-owned

- Date/status: 2026-10-08, ACCEPTED.
- Context: legacy `steam_handle_packet()` retains the last real left-stick sample because the same report bytes also carry left-pad coordinates. USB readers can run concurrently, but the baseline used process-static `g_lx/g_ly`.
- Evidence: pinned Puckbridge base `78460113ef7c48502c765a1f83ba34d53c977134`, baseline blob `7131ad17e158b7892ea0deecf6f6de016d8d375c`; `tests/test_steam_legacy_session.c` expected-red result contaminated a fresh reader with x=206 instead of neutral 128.
- Decision: pass `steam_legacy_input_state_t` to the adapter and store it in the corresponding `usb_session_state_t`; the existing per-reader zero initialization clears it at session start.
- Tradeoff: the legacy adapter signature now requires the caller's state object, making ownership explicit.
- Verification gate: normal and ASan/UBSan full host suites, targeted TSan with two concurrent 10,000-report sessions, and SDK v0.43 compile/link pass. Implementation and regression were published in PR #1 commit `9b22caa2d633af4bc66b0d5e3c93772ca5e80ce3`. Physical controller frames and PS5 runtime remain unverified.
- Reconsider only if legacy session ownership moves behind a reusable device-session object.

## ADR-019 — Constrain Steam Controller haptics to one active session

- Date/status: 2026-10-08, ACCEPTED.
- Context: menu, game-hook, adaptive-trigger and portal producers call process-wide haptic APIs, while multiple lazy Steam Controller USB readers could otherwise start and drain the same queue.
- Evidence: baseline architecture in F046; expected-red M1-106 showed a non-owner service draining an anonymous pulse. Fixed T-P30 passes 28 checks, 4,000 concurrent service calls under ASan/UBSan and targeted TSan; the selected SDK v0.43 cross-build passes in M1-107–110. Exact implementation paths: `payload/sc2_haptics.c:58-90,213-275` and `payload/gc_main.c:700-703,1184-1199,1308-1347`.
- Decision: the manager claims one exclusive generation-tagged slot token before an SC2 USB/Puck worker starts and refuses another active SC2 session. Only that token can drain queued output. Teardown invalidates ownership, clears pending pulse/rumble state, and best-effort sends a zero-rumble report before USB cleanup. Stale release/abandon cannot change a newer owner's generation.
- Tradeoff: two Steam Controller 2026 USB/Puck sessions cannot be active together. Targetless producers depend on the singleton manager rule; this is not a multi-controller routing architecture.
- Verification gate: manager and USB paths cross-build only. No PS5 runtime, controller or physical rumble behavior has been tested.
- Reconsider only after session identity reaches every producer and multi-session behavior is validated across the manager, USB output, profiles, menu, gyro, telemetry and feedback paths.

## ADR-020 — Revalidate profile selection before applying it

- Date/status: 2026-10-08, ACCEPTED.
- Context: the profile watcher, menu, and authenticated portal can concurrently call selection APIs. `g_sel_lock` prevented data races, but the operation released it for disk I/O before setting `g_active` and `g_selected`; an old operation could therefore finish after and overwrite a newer override.
- Evidence: T-P31 expected-red delayed the old reload before commit and let a new override complete first; the old mapping then became active. Fixed T-P31 passes 10 checks under normal and ASan/UBSan; targeted TSan passes both selection generation (10 checks) and profile parsing (33 checks). SDK v0.43 cross-build passes. See F052 and M1-113–117; published in PR #1 commit `9afe1f7a306daa029026602ce897404359d16942`.
- Decision: bump `g_selection_generation` for override, explicit reload, title, and title-name changes. Each `apply_selection()` snapshots that generation, performs storage work without the lock, then takes `g_sel_lock`, rejects stale results, and commits the active profile plus selected ID while selection mutations are excluded.
- Tradeoff: stale work is discarded and relies on the newer caller/watcher to complete the current choice. No lock is held across filesystem operations. This fixes logical ordering, not PS5 runtime behavior.
- Verification gate: deterministic production-module host regression, normal/sanitizer/targeted TSan, and SDK compile/link. Portal, watcher and menu integration on a PS5 remains untested.
- Reconsider if the selection state becomes session-owned or the selection backend gains asynchronous storage semantics.

## ADR-021 — Serialize profile file mutations

- Date/status: 2026-10-08, ACCEPTED.
- Context: `sc2_store_save()` used the same `<id>.ini.tmp` for every caller, while the title watcher, authenticated portal and input/menu path can write profiles concurrently. An overlapping save can truncate a shared temporary inode and make one rename fail; a save racing a delete can recreate a profile after deletion.
- Evidence: pinned Puckbridge base `78460113ef7c48502c765a1f83ba34d53c977134`; derivative `payload/sc2_profile.c:323-377`; `tests/test_sc2_profile_store.c`; F053/T-P32/M1-120–128. The expected-red test recorded concurrent entry into the previously unguarded mutation region. The fixed production module passes 22 checks under normal and ASan/UBSan, plus targeted TSan. Published in PR #1 commit `085357422014aa658dcba47298d90a3b1a96b978`.
- Decision: hold one process-wide mutex across `sc2_store_save()` replacement and `sc2_store_delete()`. Save checks complete write and `fclose` before rename; failed rename cleans up the temp path. Reads remain outside the mutex and rely on rename atomicity to see one complete file version.
- Tradeoff: storage mutations for different IDs are also serialized; profile files are small and writes are infrequent. Independently edited read-modify-write snapshots are still last-writer-wins; this is not conflict detection or merge semantics.
- Verification gate: host tests cover two saves and save/delete overlap against the production module, sanitizer and TSan. PS5 filesystem behavior, crash recovery, and menu/portal runtime integration remain unverified.
- Reconsider if profile writes become frequent enough for measured contention, or if an API adds explicit versioned updates/merge behavior.

## ADR-022 — Do not report a failed remap as saved

- Date/status: 2026-10-08, ACCEPTED.
- Context: `do_remap()` ignored the profile-store result, then reloaded and displayed a success notification even when the storage operation failed.
- Evidence: `tests/test_sc2_menu_storage.c` calls the production menu action and forces `fopen()` failure by placing a directory at the temporary path. The old path still sent “Saved”; PR #1 commit `a3b373c281d3a3abc4d7333b7accaf555855647a` fixes it and passes nine checks under normal and ASan/UBSan. See F054/T-P33/M1-130–136.
- Decision: report a clear save failure and return before reloading the profile or sending success feedback. Retain the normal save/reload/success sequence only when `sc2_store_save()` returns success.
- Tradeoff: the menu closes after the action as before, but now the final notification identifies the failed persistence. Hardware notification display remains unverified.
- Reconsider only if the menu gains a recoverable editor state that can retain unsaved changes and offer retry.

## ADR-023 — Bound total portal request time

- Date/status: 2026-10-08, ACCEPTED.
- Context: the portal has one sequential request thread. Its inherited 2-second `SO_RCVTIMEO` limited each idle gap, so a client could hold that thread by sending a byte before each timeout. A complete read that returned after the deadline also needed rejection.
- Evidence: pinned Puckbridge `78460113ef7c48502c765a1f83ba34d53c977134`, baseline `payload/webui.c:101-115,288-307`; current framing `payload/webui_http.c:173-241`, connected in `payload/webui.c:114-135,147-160`; regressions `tests/test_webui_http.c:71-127`; F055/T-P34/M1-139–147 and commits `98adfdf56f3288c549abb746b95d53b57cf11081`/`4734ffbe869c54222f5b3e20b5c78c598253faa3`.
- Decision: start a 5-second absolute deadline from a monotonic clock when request reading begins. Before each `recv`, pass only the remaining budget (capped at the socket's 2-second idle wait); after each successful receive, sample the clock again and reject data returned at or beyond the deadline.
- Tradeoff: a slow but otherwise valid request exceeding five seconds is rejected. This bounds request framing time, not response-generation or complete portal availability.
- Verification gate: expected-red deterministic slow-drip and delayed-completion cases; eight framing assertions (including slow body delivery) plus full normal and ASan/UBSan suites; SDK v0.43 compile/link. Target socket timeout precision and PS5 runtime behavior remain unverified.
- Reconsider only if target networking needs a different documented total request budget and it can be shown not to permit indefinite thread occupancy.

## ADR-024 — Validate persisted game-hook settings

- Date/status: 2026-10-08, ACCEPTED.
- Context: `game_hooks.c::load_settings()` accepted arbitrary text through `atoi`. A nonzero boolean like `2` enabled input/vibration, trailing junk such as `1junk` was treated as mode 1, and very large decimal strings were not checked for conversion errors.
- Evidence: pinned Puckbridge `78460113ef7c48502c765a1f83ba34d53c977134`, baseline `payload/game_hooks.c:39-46`; current parser `payload/game_hooks.c:67-100`; regression `tests/test_game_hooks.c:65-80,120-136`; F056/T-P35/M1-148–154.
- Decision: parse a complete decimal with `strtol`, reject `ERANGE`, trailing non-whitespace and values outside the setting's permitted domain. Malformed booleans retain default safe values; a syntactically valid but out-of-range mode keeps the existing fallback of 1.
- Tradeoff: malformed historical settings may now fall back to defaults instead of being partly interpreted. Existing saved files write exact decimal 0/1/mode values and remain compatible.
- Verification gate: expected-red host regression, four settings assertions and concurrent game-hook worker test under normal, ASan/UBSan and targeted TSan, plus the PS5 SDK cross-build. Real filesystem/configuration on a PS5 remains unverified.
- Reconsider only if the persisted settings format gains a versioned schema or documented non-decimal values.


## ADR-025 — Bound total portal response time

- Date/status: 2026-10-08, ACCEPTED.
- Context: the inherited send_all() loop made repeated blocking send() calls while the accepted socket had only a two-second per-call send timeout. A client could keep the sole portal thread busy by accepting some bytes before each timeout. The HTTP response header, body and large icon path all used this helper.
- Evidence: pinned Puckbridge 78460113ef7c48502c765a1f83ba34d53c977134, baseline payload/webui.c:41-47,303-307; current portable implementation payload/webui_http.c:173-214; adapter payload/webui.c:42-66,110-149,226-233; tests/test_webui_http.c; F057/T-P36/M1-156–161.
- Decision: use one five-second absolute monotonic deadline for each complete response, including all header/body chunks, and pass only the remaining budget (capped at two seconds per socket call) to the send callback. Reject writes that finish at or after the deadline.
- Tradeoff: a slow-reading client may receive a truncated response after five seconds. Server runtime remains serialized, but a single response can no longer occupy the thread indefinitely at the send layer.
- Verification gate: expected-red simulated drip sender, successful partial-write case, seven HTTP write checks, full normal and ASan/UBSan suites, and SDK v0.43 compile/link. PS5 socket timeout scheduling and live portal operation remain unverified.
- Reconsider only if target measurements show a legitimate portal response needs a larger documented deadline.


## ADR-026 — Match portal routes and parameters exactly

- Date/status: 2026-10-08, ACCEPTED.
- Context: pinned Puckbridge's portal used prefix route checks and searched the complete target with `strstr`. A suffix route could enter a state-changing handler, and a query-key substring could be mistaken for an exact `id=`/`on=` parameter. The derived portal already requires a bearer token for `/api/`, so these cases were dispatch ambiguity and input validation defects, not an authentication bypass.
- Evidence: pinned Puckbridge `78460113ef7c48502c765a1f83ba34d53c977134`, baseline `payload/webui.c:61-70,175-182,238-258,269-272`; current `payload/webui_route.c`, handler `payload/webui.c`; regression `tests/test_webui_route.c`; F058/T-P37/M1-163–168. The initial production-wired route extraction failed four of 17 expected checks before strict parsing was implemented.
- Decision: register complete path strings and allowed query-key sets per route; reject unknown, duplicate, malformed and percent-encoded parameters; copy values into caller-sized buffers; enforce explicit GET/POST policy and strict values before action handlers mutate state. Keep URL values to the restricted ASCII alphabet already used for profile IDs and UI controls.
- Tradeoff: adding API routes or parameter values now requires updating the route schema and tests. Percent-encoded or future arbitrary-text query values are rejected until their decoding and validation rules are deliberately specified.
- Verification gate: 54 route assertions and full 1003-check normal and ASan/UBSan suites pass; SDK v0.43 compile/link succeeds. The host tests exercise the production route module, not the live socket-to-handler path. Browser behavior, PS5 socket integration and hardware remain unverified.
- Reconsider only when introducing a route that needs a documented encoding/value schema, and retain exact path/key matching.


## ADR-027 — Validate portal Origin and profile request bodies

- Date/status: 2026-10-08, ACCEPTED.
- Context: the pinned HTTP parser treated a POST without Content-Length as a zero-byte body and ignored Content-Type and Origin. The profile route then parsed an empty string into the existing or default profile, saved it and returned success. The derivative already requires a private bearer token for `/api/`, so this was request-contract correctness and defense-in-depth work, not an authentication bypass.
- Evidence: Puckbridge pin `78460113ef7c48502c765a1f83ba34d53c977134`, `payload/webui.c:194-202`; derivative `payload/webui_http.c:157-243`, `payload/webui.c:161-187,284-297`; `tests/test_webui_http.c`; F059/T-P38/M1-172–176.
- Decision: parse bounded unique Host, Origin and Content-Type fields. If Origin is present, require `http://` plus the request Host, comparing ASCII case-insensitively; allow no Origin for non-browser clients and keep bearer authorization as the primary API gate. Require `/api/profile` POST to include explicit nonzero Content-Length, nonblank content and declared `text/plain;charset=UTF-8`.
- Tradeoff: clients behind a host-rewriting proxy or using another profile media type will be rejected. The portal remains plain HTTP; this change does not encrypt the bearer token or provide browser/console runtime evidence.
- Verification gate: 27 new request-policy checks pass in normal and ASan/UBSan suites; full suite passes 1030 checks; SDK v0.43/LLVM 18.1.8 compile/link passes. Live browser/socket-to-handler and physical PS5 validation remain required.
- Reconsider if the portal gains a documented HTTPS/reverse-proxy mode or needs a different profile upload media type.


## ADR-029 — Publish directly to main while the owner works solo

- Date/status: 2026-10-08, ACCEPTED at the product owner's explicit request.
- Context: the owner is the sole developer and asked that work be published directly to `main`, without pull requests, until the first successful release. The current draft PR is incomplete and its description overstated what a clean clone contains.
- Decision: after verifying the current `main` head and full source tree, publish complete working snapshots as direct, fast-forward commits to `main`; close the obsolete draft PR after its content is restored to `main`. Do not rewrite or delete history. Keep the hardware, firmware, and BLE gates explicitly blocked/unverified and do not release an unverified binary.
- Tradeoff: direct commits remove the PR review/checkpoint flow; compensate with exact tree/blob verification, clean-checkout builds, and append-only project memory.
- Reconsider after the first successful release or when the owner asks for a review workflow.


## ADR-030 — Track SC2 endpoint discovery in the USB session

- Date/status: 2026-10-08, ACCEPTED.
- Context: `sc2_find_active_ep()` opened and started temporary candidate endpoints with direct ioctls, but the session resource state did not know those operations occurred. STOP/CLOSE return values were ignored before `gc_main.c` uninitialized or reinitialized the USB FS session.
- Evidence: pinned Puckbridge source `78460113ef7c48502c765a1f83ba34d53c977134`; derivative `payload/controller_sc2.c:109-225`, `payload/usb_resources.c`, `payload/gc_main.c:808-839`; fail-once CLOSE regression in `tests/test_sc2_usb_scan.c`; F064/T-P40/M1-191.
- Decision: have the scanner record opened endpoint, possible transfer, completion, stop, and close transitions in the reader's existing `usb_resources_t`. Successful CLOSE clears the endpoint and transfer bits. Failed STOP/CLOSE retains its ownership state and returns the first error; `gc_main.c` routes retained resources through `usb_session_cleanup` instead of proceeding with ordinary scan reinitialization. This keeps one authoritative resource registry and reuses the existing cleanup path.
- Tradeoff: scanner ioctl operations are still direct and cannot yet inject each platform operation through the `usb_session_ops_t` table. Host tests verify state transitions and retries but not Prospero kernel semantics.
- Verification gate: concurrent scanner test plus fail-once CLOSE/retry under normal, ASan/UBSan, and TSan; full SDK v0.43 compile/link. PS5 USB ioctl and physical Puck tests remain required.
- Reconsider if the target's documented USB FS behavior requires scanner-specific operation callbacks or different partial-close semantics.


## ADR-031 — Account USB COMPLETE by the returned endpoint

- Date/status: 2026-10-08, ACCEPTED.
- Context: SDK v0.43 declares `USB_FS_COMPLETE` as `_IOR` and its output structure contains the completed endpoint index. The generic session API instead cleared the endpoint index supplied by the caller. With overlapping input/output work, a different endpoint can complete first. Separate Xbox and SC2 OUT helpers also managed START/COMPLETE/STOP outside the shared resource owner and discarded STOP failures.
- Evidence: SDK commit `d9c9519116944a7f1c22d262012c53b80b3520b7`, `target/include/dev/usb/usb_ioctl.h:170-172,331`; expected-red two-active-endpoint host regression in `tests/test_usb_session.c`; fail-once COMPLETE/STOP regression in `tests/test_usb_helpers.c`; F065/T-P41/M1-193.
- Decision: make the injected COMPLETE callback return the actual completed endpoint, clear that endpoint's transfer ownership, and distinguish an unrelated completion so a caller continues polling. Reject a new START while that endpoint remains active. Route Xbox handshake input, Xbox/Nintendo OUT commands, and SC2 haptics through the session/resource helper. If STOP fails, retain the active bit and retry through common session cleanup.
- Tradeoff: a caller may observe and drain completions for other active endpoints while waiting on its own transfer. The SC2 candidate scanner still handles multiplexed completions through its direct but tracked scan loop.
- Verification gate: expected-red then fixed endpoint-accounting test, fail-once STOP cleanup retry, full normal/ASan/UBSan host suites, and pinned SDK cross-build. Real PS5 ioctl scheduling, controller/Puck behavior, device removal, and kernel STOP semantics remain unverified.
- Reconsider if target measurements reveal different COMPLETE semantics or if scanner operations are moved to a transport-specific multiplexing interface.


## ADR-032 — Reuse the USB session operation table for SC2 endpoint scans

- Date/status: 2026-10-08, ACCEPTED.
- Context: after M1-193, `sc2_find_active_ep()` updated the shared resource bitmap but still issued raw endpoint ioctls directly. That made operation failures difficult to inject and left scanner lifecycle behavior outside the existing `usb_session_ops_t` abstraction. USB FS COMPLETE is multiplexed across active endpoints, so any seam must report and account the endpoint the kernel actually returns.
- Evidence: `payload/controller_sc2.c::sc2_find_active_ep()`, `payload/usb_session.c::usb_session_complete_any_transfer()` and `usb_session_close_endpoint()`, `tests/test_sc2_usb_scan.c`, T-P42/M1-194; SDK v0.43 header evidence retained in ADR-031.
- Decision: pass the owning `usb_session_ops_t` and context into the SC2 scanner. Reuse shared open/start/complete/stop/close lifecycle helpers; do not create a scanner-specific duplicate ops table. Add a general COMPLETE-any-active-transfer helper, which accounts the returned endpoint before scanner report classification. Reject a scan while any pre-existing transfer is active, because COMPLETE consumes from the whole FS session queue and must not drain another owner's completion.
- Tradeoff: the scanner now depends explicitly on the session operation interface, and a session with an unrelated active transfer must finish or clean up before discovery starts. `gc_main.c` performs discovery in that exclusive pre-reader phase. The fake table validates portable ownership/dispatch only; Prospero scheduling, unplug semantics and actual USB behavior remain unverified.
- Verification gate: endpoint 1 completes before endpoint 0 and the scanner selects the valid endpoint; active-transfer conflict leaves state unchanged; fail-once STOP/CLOSE remains owned and cleanup retries; full normal and ASan/UBSan suites, targeted scanner TSan, and pinned SDK v0.43 compile/link.
- Reconsider if hardware evidence shows the Prospero USB FS completion queue can safely be consumed concurrently by independent endpoint owners.


## ADR-033 — Fail closed on invalid or empty SC2 configuration maps

- Date/status: 2026-10-08, ACCEPTED.
- Context: `sc2_read_usb_map()` collapsed ioctl failure and malformed configuration into the same negative result, while `sc2_list_in_eps()` converted that and a valid zero-candidate map into `0`. `gc_main.c` treated `n_eps == 0` as permission to probe all eight guessed endpoint addresses. A removed device (`ENXIO`) therefore also reached the fallback, and the configuration was read twice for IN and OUT discovery.
- Evidence: baseline GitHub main at memory-sync commit `d4bf2acd02d46542a9b927d004be920bedd2fac3`; expected-red regressions in `tests/test_sc2_usb_scan.c` first observed `GET_FULL_DESC` with `EIO` returning `0` instead of an unavailable status and `ENXIO` returning unavailable instead of device-gone; T-P43/M1-195.
- Decision: return separate statuses for unavailable ioctl, malformed map, and device disappearance. Use the bounded endpoint compatibility list only for a non-ENXIO `USB_GET_FULL_DESC` failure; reject malformed descriptors and valid maps with no eligible HID IN endpoint. Parse once and derive candidates and haptic interface pairing from that same map.
- Tradeoff: an unexpected but recoverable descriptor parsing incompatibility will now fail discovery rather than guess. The target kernel's exact error codes and live configuration/report descriptors remain unverified.
- Verification gate: expected-red then fixed status checks for unavailable, malformed, ENXIO, valid-empty Puck filter, and valid candidate map; full host/ASan/UBSan, targeted scanner TSan, SDK v0.43 cross-build. No real USB ioctl or hardware behavior is claimed.
- Reconsider if PS5 hardware evidence shows a non-ENXIO ioctl failure that returns usable descriptor data or if the Puck's valid interface topology differs from the pinned parser contract.


## ADR-034 — Validate declared USB configuration and interface counts

- Date/status: 2026-10-08, ACCEPTED.
- Context: the bounded configuration parser validated descriptor byte lengths but ignored `bNumInterfaces` and each interface/alternate setting's `bNumEndpoints`; duplicate altsetting-zero records could inflate the map, while more selected interfaces than the fixed map capacity were silently dropped.
- Evidence: `payload/steam_usb.c::steam_usb_parse_config()`, regression `tests/test_steam_usb.c::test_declared_descriptor_counts()` and `test_interface_map_capacity()`, USB-IF HID 1.11 Appendix E.2–E.5 (S11), F069/T-P45/M1-197. Before the fix, the three initial count/capacity regressions failed against the production parser.
- Decision: count distinct altsetting-zero interfaces and require that count to equal configuration `bNumInterfaces`; reject duplicate default interfaces; count endpoint descriptors separately for every interface/alternate setting and require each to match its declaration; reject selected maps that exceed `STEAM_USB_MAX_INTERFACES` rather than truncating them.
- Tradeoff: malformed or unusual descriptors now fail discovery. The count checks do not validate a device's HID report-descriptor contents, PS5 ioctl semantics, or physical hardware compatibility.
- Verification gate: 24 production parser checks, the full normal and ASan/UBSan suites, and the pinned SDK v0.43 cross-build pass. No PS5, Steam Controller, Puck, live descriptor, or USB ioctl was tested. See T-P45.
- Reconsider only if a captured, standards-conforming target descriptor demonstrates a legitimate topology that this bounded map cannot represent; preserve strict count validation.


## ADR-035 — Require fixed sizes for standard USB descriptors

- Date/status: 2026-10-08, ACCEPTED.
- Context: `steam_usb_parse_config()` checked only minimum lengths for standard configuration, interface, and endpoint descriptor types. It therefore accepted extra bytes inside those records even though USB 2.0 defines fixed standard lengths: 9, 9, and 7 bytes respectively. A descriptor extension has its own descriptor type and must not be folded into a standard record.
- Evidence: USB-IF USB 2.0 Specification §§9.6.2, 9.6.3 and 9.6.6 (S12); expected-red `test_fixed_standard_descriptor_lengths()` in `tests/test_steam_usb.c`; F070/T-P46/M1-198.
- Decision: reject configuration descriptor headers unless bLength is exactly 9, interface descriptors unless bLength is exactly 9, and endpoint descriptors unless bLength is exactly 7. Keep unknown descriptor types length-bounded and skippable.
- Tradeoff: malformed streams and nonstandard vendor encodings of these standard descriptor types are rejected. This does not validate HID report descriptors or actual PS5 ioctl/device behavior.
- Verification gate: three expected-red regressions fail before and pass after the fix; 27 USB parser checks, full normal and ASan/UBSan suites, and SDK v0.43 compile/link pass. Physical USB/PS5 remains untested.
- Reconsider only if a standards-backed target ABI explicitly defines a different descriptor type/length; do not relax standard descriptor sizes based on an undocumented capture.


## ADR-036 — Request declarations required by strict-C11 token storage builds

- Date/status: 2026-10-09, ACCEPTED.
- Context: GitHub Ubuntu host CI compiled `webui_auth.c` with `-std=c11` and failed because the system headers hid `O_DIRECTORY`, `O_NOFOLLOW`, `openat()`, `fchmod()`, `unlinkat()`, and `fstatat()`. The PS5 SDK build had passed, and the previous local Darwin build did not reproduce the Linux feature-visibility behavior.
- Evidence: source at GitHub main `4c947486fd41371d9a96238d39e8b7a826c1c34c`, `payload/webui_auth.c:74-164`; Host Actions `37848828922`, attempts 1 and 2; expected-red CI compile logs; T-P47/M1-199/F071.
- Decision: define `_POSIX_C_SOURCE=200809L` at the top of `webui_auth.c`, before every include, only when compiling on Linux and only if the build has not already defined it. This requests POSIX.1-2008 declarations in the Ubuntu strict-C11 build without changing global flags or the Darwin/PS5 SDK feature environment.
- Tradeoff: Linux consumers of this translation unit get POSIX.1-2008 declarations; runtime logic and token-storage behavior are unchanged. Darwin and PS5 compile paths do not define or override the feature macro here.
- Verification gate: strict local syntax compile, full normal and ASan/UBSan host suites, and SDK v0.43 compile/link pass. GitHub Ubuntu Actions on the fix commit remain the final Linux CI gate; no physical PS5 behavior is implied.
- Reconsider if a supported host libc cannot expose the required interfaces with this feature request; then add a narrowly scoped portability wrapper rather than weakening global C flags.

## ADR-037 — Delete only VDA handles attributable to the current payload call

- Date/status: 2026-10-09, ACCEPTED WITH TARGET ABI LIMITS.
- Context: startup deleted every device ID in a guessed numeric range. The create path also promoted VDA output tokens, asynchronous klog DeviceIds, and handles found by `scePadGetHandle()` scans into registry-owned values. ShellUI stubs could later delete injected or recovered handles that belonged to another process/session.
- Evidence: pre-fix expected-red `tests/test_vda_ownership.py`; `payload/gc_main.c::main()`, `create_vda_for_slot()`, `delete_vda_handle()`; `payload/shellui_pad.c::shellui_stub()` and `shellui_stub_force_vda()`; Puckbridge-derived source pin `78460113ef7c48502c765a1f83ba34d53c977134`; T-P48/F072/M1-200.
- Decision: remove global ID-range cleanup. The managed slot registry may own only a positive handle returned immediately from that session's `scePadVirtualDeviceAddDevice()` call. Async klog IDs, output tokens, and `GetHandle`/open results are diagnostic/lookup data, never deletion authority. The standalone ShellUI stubs delete only a positive direct return from their own AddDevice call, and only while that same handle remains selected.
- Tradeoff: if the target API creates a device but does not return a usable positive handle, this implementation fails closed and may leave an unmanaged device until a safe target-specific cleanup contract is established. The installed SDK exposes no public contract header for this call; positive-return semantics and all physical cleanup remain unverified.
- Verification gate: three source-invariant regressions pass; full normal and ASan/UBSan suites pass; SDK v0.43 compile/link passes. No PS5, DualSense, Puck, controller, or USB/Bluetooth I/O was run.
- Reconsider only with primary ABI evidence or a reversible physical-console test proving a safe way to map the exact AddDevice-created device to a deletion handle. Never restore an ID-range sweep or delete a looked-up controller handle.

## ADR-038 — Stop runtime workers through their owners before releasing shared state

- Date/status: 2026-10-09, ACCEPTED WITH TARGET LIMITS.
- Context: the payload had multiple long-lived worker threads, USB reader sessions, profile/game-hook state and virtual DualSense devices. Process exit or detached workers could leave USB transfers active, input latched, hook changes unrolled, or shared memory in use during teardown.
- Evidence: `payload/gc_main.c::runtime_shutdown()`, `join_all_usb_workers()`, `neutralize_vda_handle()`, signal handling and USB manager; `payload/runtime_control.c/.h`; joinable stop APIs in `webui.c`, `bridge_probe.c`, `game_hooks.c`, `ds_handoff.c` and `sc2_profile.c`; cancellation path in `controller_sc2.c::sc2_find_active_ep()`; `tests/test_runtime_control.c`, `tests/test_shutdown_contract.py`, `tests/test_sc2_usb_scan.c`; T-P49/M1-201/F073; published on main as `115341056cb18eafa500eb1d3bd1020934db7106`.
- Decision: signal handlers only set a signal-safe flag; main-context shutdown first gates and joins the DualSense hand-off worker, requests a shared stop, then stops and joins service workers. USB readers observe cancellation and stop/close transfers through their existing session owner. Registered VDA input is neutralized before deletion. Do not destroy shared state or exit while a worker or registry rollback remains unresolved; retry cleanup while keeping the manager process alive. Do not use `pthread_cancel` or close another thread's active USB descriptor.
- Tradeoff: shutdown can remain pending indefinitely if the target refuses USB cleanup or game-hook rollback. Retaining ownership is safer than releasing shared state while operations may still run. The current SDK has no verified API for reconnecting a physical DualSense after an in-flight hand-off; P0-4 Safe Mode will disable hand-off by default.
- Verification gate: 12-worker shared-stop test, four source-contract tests, injected SC2 cancel/STOP/CLOSE regression, full normal and ASan/UBSan suites, `git diff --check`, and SDK v0.43 compile/link pass. No on-console signal, USB cancellation, VDA neutralization, or rollback failure was exercised; those remain hardware/ABI gates.
- Reconsider only with target evidence showing a safe owner-side interruption or cleanup contract. Do not free owner state or abandon rollback after a timeout.

## ADR-039 — Guard every payload credential elevation and restore the startup scope

- Date/status: 2026-10-09, ACCEPTED WITH TARGET ABI LIMITS.
- Context: shellui_pad.c could return after setting authid when a capability setter failed; gc_main.c ignored runtime credential and pad-privilege setup results. Direct ucred API calls were spread across shell and wireless bridge code.
- Evidence: payload/credential_guard.c/.h, credential_kernel.c/.h; guarded startup and teardown in payload/gc_main.c; temporary scopes in shellui_pad.c and bridge/wireless_ds4.c; injected regression tests/test_credential_guard.c, static source contract tests/test_privilege_contract.py, T-P50/M1-202/F074.
- Decision: centralize raw kernel credential APIs in one adapter. Snapshot before mutation, mark rollback active before setters, restore caps before authid, retry transient restoration failures, preserve operation result/errno after successful restore, and refuse to run the privileged operation unless elevation succeeds. Serialize temporary scopes. Hold a persistent scope only around runtime lifetime, check pad initialization/privilege setup, and attempt restoration after workers stop.
- Limits: credential getter/setter return semantics, zero-authid sentinel, scePadSetProcessPrivilege(1) lifetime, and process-exit cleanup are not verified on Prospero. Do not guess a privilege-zero call without ABI evidence. Persistent restore failure remains a stop-and-report condition; target recovery needs hardware.

## ADR-040 — Keep optional runtime services disabled until explicit local policy opt-in

- Date/status: 2026-10-09, ACCEPTED; HOST VERIFIED, TARGET UNVERIFIED.
- Context: startup previously launched bridge probing, game hooks, DualSense hand-off and WebUI unconditionally, and endpoint-guessing detached interfaces on unknown external USB devices.
- Evidence: payload/runtime_policy.c/.h; guarded startup and Steam-only probe fallback in payload/gc_main.c; DualSense setter gate in payload/ds_handoff.c/.h and payload/webui.c; tests/test_runtime_policy.c and tests/test_safe_mode_contract.py; T-P51/M1-203/F079.
- Decision: a missing, malformed, oversized, non-regular, or symlink policy file resolves to all optional services off. The bounded file at /data/ghostpad/runtime-policy.ini accepts only unique, exact boolean opt-ins. Bridge probe, game hooks, DualSense hand-off and WebUI require separate keys. Legacy endpoint-guessing requires a separate opt-in and is disabled by default. A WebUI call cannot bypass the DualSense policy gate.
- Limit: safe USB enumeration still identifies exact Steam Controller/Puck IDs and existing handlers contain fixed interface detach operations. Those interfaces are not yet validated; P0-5 remains required before recommending a physical console. Published directly to main as a366f2d6ca3923a3645f52500d9000cd89a9c9dd (tree 4833da3c35d8debcb08984d98e8a3688b5f6c450); all 13 local blob hashes matched, recursive tree has 224 entries/211 blobs with zero deleted base blobs. Actions 37900071213/37900071175 are queued. No target policy-file or USB runtime test occurred.

## ADR-041 — Gate every Steam USB driver detach with an exact device and descriptor map

- Date/status: 2026-10-09, ACCEPTED; HOST VERIFIED, TARGET UNVERIFIED.
- Context: safe mode stopped unknown-device endpoint guessing, but recognized Steam paths still detached interfaces before validating exact device/interface ownership, and one fallback guessed interrupt endpoints.
- Evidence: main implementation commit `afc35de59206df8a479db031d320e0d31740dff2`; `payload/gc_main.c:411,741-763,853`; `payload/steam_usb.c`; `tests/test_steam_usb.c`, `tests/test_usb_detach_contract.py`; T-P52/M1-204/F080.
- Decision: permit detach only for the exact Steam Controller 2026 wired/Puck VID/PID allowlist and interfaces in a unique parsed map with HID class and a valid interrupt-IN endpoint; restrict Puck interfaces to 2–5. Missing, malformed, unavailable, or ineligible maps fail closed. Keep one raw ioctl behind the checked helper and never guess endpoints or detach unknown devices.
- Verification/limit: expected-red duplicate-map checks, 45 USB policy/parser checks, five source contracts, full normal and ASan/UBSan suites, and SDK v0.43 cross-build pass. No PS5 USB ioctl, live descriptor, controller, or Puck was tested. Device IDs and driver-detach behavior remain hardware/ABI assumptions to verify on an isolated development console.

## ADR-042 — Use an owned process lock; never signal a PID loaded from disk

- Date/status: 2026-10-09, ACCEPTED; HOST VERIFIED, TARGET UNVERIFIED.
- Context: a persisted PID can be reused for an unrelated process, so startup cannot safely send it a termination signal based only on the numeric value.
- Evidence: `payload/runtime_instance.c:91-155`, `payload/gc_main.c:1469-1474,1582-1586`, `tests/test_runtime_instance.c`, `tests/test_pid_file_contract.py`; T-P54/M1-204/F081; published in `afc35de59206df8a479db031d320e0d31740dff2`.
- Decision: acquire a no-follow regular PID file and an exclusive nonblocking lock before runtime setup. A live/indeterminate PID causes startup refusal; a dead PID may be replaced. Never signal the recorded PID. On release, remove only the file that still matches the locked inode and this process's PID.
- Verification/limit: 41 module checks, three source contracts, full normal and ASan/UBSan suites and SDK v0.43 cross-build pass. Prospero `flock`, PID namespace, permission and shutdown semantics have not been tested on-console.

## ADR-043 — Bind first-release hardware gates to matching test-log records

- Date/status: 2026-10-09, ACCEPTED; HOST VERIFIED, RELEASE BLOCKED BY HARDWARE.
- Context: a generic test-log mention of a gate ID is insufficient evidence for a hardware result, and no physical PS5 is available in this development environment.
- Evidence: `scripts/check_release_gate.py`, `tests/test_release_gate.py`, `.github/workflows/release.yml`, `project-memory/HARDWARE_MATRIX.md`; T-P53/M1-204/F082; source commit `afc35de59206df8a479db031d320e0d31740dff2`.
- Decision: a required hardware row must link to its own `#h-xx` heading whose log entry repeats the gate ID, PS5 model, exact firmware, controller model/firmware, transport, full payload commit SHA and `Result: VERIFIED_HARDWARE`. Release tags also require passing host/sanitizer and PS5 cross-build jobs. Keep the current matrix blocked; do not release a binary based on synthetic test fixtures or a successful cross-build alone.
- Verification/limit: seven synthetic checker tests pass; the real matrix intentionally fails closed because all required hardware rows are unverified. The tag workflow and hardware gate have not yet run on a real release tag.

## ADR-044 — Use portable POSIX delay calls and host linker flags in tests

- Date/status: 2026-10-09, ACCEPTED; LOCAL HOST/SDK VERIFIED, LINUX ACTIONS PENDING.
- Context: Ubuntu Host Actions found `usleep()` hidden under strict C11, Apple's `-Wl,-dead_strip` unsupported by GNU `ld`, and a missing `<stdlib.h>` declaration for test use of `mkdtemp()`; Darwin had masked these issues.
- Evidence: runs `37900289677`, `37980962018`, `37981319013`, and `37982325945`; `payload/controller_sc2.c`, `payload/sc2_gyro.c`, `payload/runtime_control.c`, `tests/Makefile`, `tests/test_runtime_policy.c`; T-P55/T-P56/T-P57/M1-205/M1-207.
- Decision: use shared `runtime_sleep_ms()` for SC2 scanner/gyro waits, linking `runtime_control.c` into the corresponding host targets. Select `-Wl,-dead_strip` on Darwin and GNU-compatible `-Wl,--gc-sections` elsewhere.
- Verification/limit: targeted scanner, gyro and runtime-policy tests, full normal and ASan/UBSan suites, and SDK v0.43 cross-build pass locally. The POSIX wait/test-link fix is published as `451b10f03200f7bf3b95806324286c33709bb498`; the test header fix is published as `d727f28af5f7e94452da2a017270db368266ac59`. GitHub Host `37982788726` and PS5 cross-build `37982788729` both pass on `d727f28...`. No PS5 runtime or USB hardware behavior is proven.

## ADR-045 — Bound shutdown retries for game hooks and retain lease-protected target stubs

- Date/status: 2026-10-09, ACCEPTED; host behavior and SDK build verified locally; PS5 target behavior unverified.
- Context: if remote hook removal kept failing, the hook worker retried forever and central shutdown waited forever in `pthread_join()`. Arbitrary ShellUI unpatching or freeing game-resident code would be unsafe.
- Evidence: `payload/game_hooks.c::hooks_thread()`, `payload/game_hook_safety.h`, `payload/gc_main.c::runtime_shutdown()`, `payload/bridge/wireless_ds4.c::pb_merge()` and `pb_audio_silence_active()`; expected-red timeout plus T-P17/T-P58/M1-208 tests. Repository call-site audit found no production use of the legacy ShellUI patch/unpatch APIs; DualSense disconnect remains under its explicit runtime-policy gate.
- Decision: attempt a neutral frame and unsilence, retry removal at most five times, then join and return a distinct rollback-pending state. Do not unmap or overwrite target memory on failure. Complete local cleanup, emit a nonzero exit status, and rely on existing stale-read/audio heartbeat fallbacks until the target exits or later safe cleanup succeeds.
- Verification/limit: mocked persistent-write-failure regression, pure retry-policy assertions, normal tests, ASan/UBSan, shutdown/safe-mode source contracts, and SDK v0.43 cross-build pass locally. Each actual remote syscall may still have target-dependent latency; the in-memory gateway, fallback callback timing, races, process exit and stock DualSense behavior require the documented PS5/H-12 gate. No physical hardware or system-memory operation was used.

## ADR-046 — Separate source/CI readiness from console release readiness

- Date/status: 2026-10-10, ACCEPTED.
- Context: the source tree and host/cross-build checks are complete enough for reproducible software review, while no physical PS5/controller/Puck test has been performed. A CI artifact exists but expires and does not demonstrate console runtime.
- Decision: describe each capability with its evidence level; treat GitHub Actions artifacts as temporary build outputs only; keep hardware-dependent rows NOT_RUN/BLOCKED_HARDWARE; keep the release workflow fail-closed; do not publish a Release or claim firmware compatibility without actual required hardware records.
- Verification/limit: latest main source Actions 37986069382 and 37986069353 pass, local host and ASan/UBSan suites pass, and SDK v0.43 cross-build passes. No console, USB/Puck runtime, radio, firmware write or install path was exercised.

## ADR-047 — Target owner-reported PS5 firmware 12.00 without a fixed release pin

- Date/status: 2026-10-10, ACCEPTED; HOST TESTS AND SDK BUILD VERIFIED; HARDWARE COMPATIBILITY UNVERIFIED.
- Context: the owner reports that their PS5 runs firmware 12.00 and will not update. Prior target text and the release checker incorrectly made 13.60 mandatory. Upstream Puckbridge's 13.60 statement is a historical upstream claim and is not evidence for this target.
- Decision: use 12.00 as the development/test target, explicitly labeled owner-reported and unverified until H-01 reads the console. Preserve historical test logs and upstream claims. The release gate accepts any exact numeric firmware version only when a required matrix row records it and its gate-specific TEST_LOG section contains exactly one identical `Firmware:` value; it does not pin 13.60 or treat the owner's report as hardware evidence. Keep required hardware rows and release workflow fail-closed.
- Verification/limit: synthetic checker tests pass for the reported 12.00 target, an alternate exactly evidenced version, malformed/unverified firmware strings, mismatches and substring-only mismatches. Full normal host tests, ASan/UBSan, and SDK v0.43 cross-build pass. Published directly to main as `81501afa53e2acd329db0298ffccdc533161e749`; Host Actions `38053866134` and PS5 cross-build `38053866148` pass. No console test/deploy or BLE/radio operation was run, so FW 12.00 compatibility remains unverified.

## ADR-048 — Image-based remapping through the existing profile transaction

Date: 2026-10-10 14:45 UTC. Preserve the production `BITS`, `OUTS`, INI API and `openSheet` draft boundary. Reuse the owner's four unchanged WebPs and SVG geometry with explicit source/output shape-key tables; shared front/back controls use one logical key. Preserve logical image sizes/viewBoxes rather than replacing them with decoded pixels. `gen_html.py` embeds data alongside icons, and Make tracks all resources. Cancel discards draft state; Done updates the profile; existing authenticated Save performs persistence. DualSense face symbols use the existing PlayStation colors. QAM stays reserved; sensors/connectors/Mute are informational. No device parser, virtual-pad backend, firmware or security policy changes. Owner authorized asset publication; no blanket asset license is inferred. Seven portable and 99 real-browser fixture checks pass; target portal H-09 remains NOT_RUN.

ADR-048 publication acknowledgement: source `4065ff41d51b86fdf3a60f972f72b2b24855c647` is on main with its complete 233-blob tree, no deletions and both Host/cross-build Actions passing. Subsequent documentation-only acknowledgement commits do not change the validated payload or artwork.


## ADR-049 — Calibrate DualSense hotspots and align Remap UI intent

- Date/status: 2026-10-10; local implementation and host/browser/build verification complete; publication/Actions pending at record time; physical portal remains unverified.
- Context: owner screenshots showed DualSense L1/R1 outlines below visible bumper controls and face-symbol overlays offset. The attached audit also found Shift inputs appeared to have ordinary assignments despite the binding engine skipping them, imported `NONE` activators looked like actions, tapping an output did not stage that output, and “Replace” overstated partial-INI import behavior.
- Evidence: review baseline GitHub main `78cc6f027e7a43a805d47e7bc3183e4274ee5dba`; `payload/web/controllers/layouts.json` DualSense front/back `l1/r1/l2/r2`, `tri`, `ci`; `payload/web/index.html::buildController()`, coarse-pointer delegated click path, picker/activator rendering and profile import; `payload/sc2_binding.c` Shift-input skip; regressions `tests/test_webui_controllers.py::test_dualsense_trigger_and_face_button_calibration` and `tests/webui_remap_browser.js`; M1-216/T-P59/F097.
- Decision: map L1/R1 to the upper bumper shapes and L2/R2 to lower triggers in both image views; center triangle/circle over the source artwork. Keep the authored mapping as a draft until Done, make Shift-only state explicit, normalize empty/`NONE` activators, label import as Merge, group outputs and use PlayStation glyphs in visible face-button summaries. On coarse pointers only, select the nearest output whose shape is within 16 CSS pixels of the tap, then stage it for confirmation.
- Verification/limit: expected-red geometry test failed before calibration; eight portable WebP/geometry/generator tests, 114 Microsoft Edge fixture checks, full normal host suite, ASan/UBSan suite and SDK v0.43 cross-build pass. Browser workflow added to Host Actions. This is not console/PS5 browser or FW 12.00 evidence; no hardware integration was run.


ADR-049 publication acknowledgement: commit `408900592f50b1eb39b0003c357c95a137410f2f`, parent `78cc6f027e7a43a805d47e7bc3183e4274ee5dba`, tree `3fbd9e72cd5ffed4180112a28b0cf03dd3793404`; expected-head non-force update acknowledged. The complete tree retains 249 entries/233 blobs, all 23 intended blob hashes match and no paths were added or removed. Host/Chromium/normal/ASan-UBSan Actions `38067268430` and SDK cross-build `38067268427` pass.

## ADR-050 — WebUI Remap security and edge-case hardening

- Date/status: 2026-10-10; ACCEPTED; local host/browser/build verified, publication pending, PS5 unverified.
- Baseline: main `799d301b8d8936244fe5ea50ab9967b21cccee2d` / tree `0f7768a27832e12adf6d08b30b7d65b3f5ba6836` / 233 blobs. User repeat audit confirmed five issues through source and expected-red regressions.
- Decision: reject an invalid imported profile atomically using BITS/OUTS and setting allowlists; render assignment rows through DOM nodes and filter badge IDs defensively. Shift is button-modifier-only, including trigger Full pull; retain stored bindings and analog trigger axis behavior. Toggle wins Turbo conflicts in JS and C independent of order. Save rereads a complete canonical server snapshot, keeps later edits dirty and isolates results by profile ID. HTTP success without confirmed readback is not Saved. Coarse taps prioritize actual SVG fill then transformed boundary distance; remove intersecting shoulder/D-pad/triangle zones. Show a mobile scroll cue.
- Reuse: production C parser/formatter in bounded host stdin fixture; no second parser/server/payload architecture. SDK v0.43, safe policy, USB/Puck unchanged.
- Evidence: M1-218/T-P60/F099; nine portable and 158 browser assertions, full normal/ASan/UBSan suites and SDK build pass. Target H-09 remains required. Replace Profile remains optional; current documented import contract is Merge.

ADR-050 addendum: production C fixture proved editor-cleared activators were resurrected by the merge-based POST when omitted. Serialize explicit NONE/0 tombstones for a full editor Save without changing the established API or partial Merge-import contract. Require uncached, complete canonical GET; reject control characters in profile names and empty suffixes. Regression now passes162 assertions; clean final ELF `247a25a7962336ddc4b31c36ef8c251502b2c27cd7d158e35c69cd52c7a15564`.

ADR-050 publication acknowledgement: `6914442f6b813f90d1156f93d18f532919ff13e8` / tree `4bff80b5c83f088da2fef77b826e22c9314c5607`; exact-head non-force main update verified, full 234-blob tree retained (one intentional fixture addition, no deletion). Host/Chromium/ASan-UBSan `38070068676` and PS5 cross-build `38070068771` PASS. Real H-09 remains unverified.

## ADR-051 — Complete snapshots, evaluated trigger provenance and ordered editor operations

Context: baseline main 45a8bda6c53efd50099e1f727ea9cc2b3fa30f0b, owner remaining P0/P1 audit. Decision: a 24 KiB bounded format/error contract and checked complete disk snapshots reuse the existing parser/store mutex/rename. Invalid files are retained for recovery and cannot be overwritten by implicit default+partial POST. Reuse the binding engine for actual active source provenance and proportional effective trigger routing; no duplicate timing in controller_sc2. Support Long+Turbo and Long+Toggle with threshold-based Long output, quick regular tap, once-per-hold latch and existing Toggle precedence. Browser generations, immutable draft comparisons, editor identity, canonical readback and explicit Save/Discard/Cancel prevent stale operations and cancelled creation. Preserve pending history destinations for Forward. Retry transient HTTP EAGAIN/EINTR under the original deadline; log failure and protect socket writes against SIGPIPE.

Tradeoffs: incomplete/invalid legacy disk snapshots need explicit repair; names are limited to the existing C field's 47 UTF-8 bytes. Unknown data is rejected rather than silently normalized away. Native PS5 browser/dialog/socket behavior and input thresholds are still H-09/H-01/H-02 hardware gates. SDK v0.43, firmware target 12.00, artwork/geometry, USB/Puck, authentication, safe defaults and release gates remain unchanged. Evidence: M1-220 local normal/sanitizer/build/browser results; no console operation.

## ADR-052 — Consolidation and explicitly authorized experimental prereleases (2026-10-10)

Preserve all production source/modules, tests and Git history. Audit all 234 parent files before deletion/move. Untrack only deterministic webui_html.h after fresh-tree regression and Makefile fixes. Archive historical docs byte-for-byte; retain logs at stable paths and add concise navigation/current status. Existing internal ABI/data/ELF names remain; public identity and DEV filename use SteamController-PS5.

The owner requests a downloadable experimental DEV ELF without claiming physical readiness. Separate workflow_dispatch uses dev/ tags, explicitly selected tested main SHA, reruns host/ASan-UBSan/Chromium/assets/SDK/checksum and publishes prerelease only. Existing stable v* workflow/hardware checker remain unchanged. Critical-code-blocker manifest blocks known unresolved code defects; hardware risks remain explicit. No console deployment/BLE/system experiment or history rewrite is authorized by this channel.

## ADR-053 — First-launch pairing and audit remediation — 2026-10-11 — ACCEPTED

Owner explicitly authorized critical USB remediation before dev.2 despite the initial first-launch transport scope restriction. Missing policy now enables only WebUI; malformed or unsafe existing files retain all-off behavior. Reuse existing HTTP/auth modules: CSPRNG eight-digit code, five minutes, five attempts, mutex-enforced single redemption, strong existing bearer, local notification only after listener readiness. Four tracked clients and pre-body admission avoid one unpaired slow body blocking the portal; API dispatch stays serialized.

Session-owned USB frame arrays survive failed STOP/CLOSE until session cleanup; busy endpoints cannot mutate metadata. Only successful single-frame input refreshes the 500 ms watchdog; expiration neutralizes owned input and clears the active Steam state. Idle Puck discovery takes no active feedback lease. Real ioctl timing/cancellation remains a hardware gate.

Optional hook installation persists rollback metadata before import writes. Partial writes keep retry ownership; unused mappings are unmapped or retained for cleanup retry. Retained old hook pages are refused without modification until game restart. In-flight-call safety requires keeping any previously referenced gateway mapped until process exit.

Settings use checked atomic replacement and apply candidate RAM values only after persistence succeeds. Menu/portal profile patches hold the same store lock throughout load/patch/save; explicit Default recovery keeps exact previous bytes. Authenticated icon fetch uses bounded local blob caching. Stable release evidence permits an exact tested SHA or descendant with unchanged payload/scripts/tests/workflows; full software gates remain mandatory. No radio/firmware experiment or hardware claim is authorized by DEV publication.
