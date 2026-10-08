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
