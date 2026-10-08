# FINAL CODEX TASK — SteamController-PS5

**Revision:** 1.0 · **Research cutoff:** 2026-10-08 · **Primary target:** jailbroken PS5 with ProsperoOS firmware 13.60 · **Implementation language:** portable C99 for the payload/engine, other languages only where justified for host tools and UI.

**Read first:** [`AGENTS.md`](AGENTS.md), [`docs/SOURCES_AND_EVIDENCE.md`](docs/SOURCES_AND_EVIDENCE.md), [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md), [`project-memory/PROJECT_MEMORY.md`](project-memory/PROJECT_MEMORY.md).

> This is an **execution specification**, not a request for another high-level proposal. Work through the milestones, implement and test all feasible deliverables, and preserve exact evidence for things requiring a physical PS5. Never invent results, code, SDK support, commits or PR URLs. Never leave TODO/mock behavior in a deliverable described as working.

## 0. Mission and outcome

Build a maintainable standalone open-source project **SteamController-PS5**: a single native PS5 payload providing the best practical integration of **Steam Controller 2026** with PS5 games and the system UI. Support the official Valve Puck (2.4 GHz USB receiver) and wired USB-C first, then direct BLE where proven feasible. Preserve the Steam Controller's **full native capabilities** (two touchpads, independent back/grip controls and sensors where present, IMU, battery, haptics), and translate them into the widest possible verified subset of normal DualSense behavior. Provide remapping, action layers, gyro aiming, flick stick, profiles, diagnostics, clean controller hand-off, and reliable slot/user assignment.

**This is not an effort to add every gamepad to OmniPad.** The product is Steam Controller-focused. However, every added/modified **shared** module must be reusable, transport-independent, testable and extendable to future controllers **without** sacrificing performance or clarity. Do not rebuild features already solved well upstream; reuse carefully, preserve credit and licensing.

### Delivery tracks

- **Release track:** wired Steam Controller 2026 and official Puck; stable virtual DualSense, input, motion, touch, available feedback, profiles, game/UI use, safety, diagnostics.
- **Parallel research track:** native PS5 BLE feasibility, alternative external BLE central, Edge emulation, custom ShellUI display name/icon. These are **not** blockers for the USB/Puck release.
- **Contribution track:** upstream-agnostic bug fixes can be proposed back to OmniPad, Puckbridge or their origins as independently reviewable PRs; our main project's PRs are against its own development branch. Do not request maintainer write rights.

### Success definitions

1. One ELF/payload, one input/core profile model and **one authoritative slot manager**. No second competing virtual-pad stack or bridge payload.
2. On real firmware 13.60, Steam Controller can operate in at least one real PS5 game and console UI via **USB-C and Puck**, after explicit hardware validation. Before that, label as implementation/build only.
3. Controller-specific capabilities are never silently discarded by prematurely converting raw packets to ordinary six-axis/button state.
4. PS5 stock DualSense stays available; all hooks have rollback and explicit feature switches.
5. All imported and authored code has recorded provenance and compatible licensing.
6. Every test has exact command, commit, environment, status and evidence in `project-memory/TEST_LOG.md`.

## 1. Evidence policy — mandatory

Classify each relevant assertion as one of:

- `VERIFIED_CODE` — confirmed in identified source at a pinned SHA; NOT proof of runtime success.
- `VERIFIED_HOST_TEST` — executed host tests with logs and target SHA.
- `VERIFIED_PS5_BUILD` — actual PS5 cross-build produced an ELF.
- `VERIFIED_HARDWARE` — executed on actual hardware, including PS5 model, firmware, Steam Controller revision, transport, game and evidence.
- `UPSTREAM_CLAIM` — documented by project maintainer but not independently reproduced here.
- `EXTERNAL_HARDWARE` — worked on Linux/Android/another MCU/OS; not ProsperoOS proof.
- `HYPOTHESIS` — plausible untested interpretation.
- `BLOCKED` — absent hardware, access, SDK, permission or reproducible evidence.

Do not use promotional phrases such as "fully compatible" or "native BLE supported" for a feature without passing the corresponding hardware gate. Read README/release notes and then source before trusting a project claim. Where results conflict, preserve both and specify versions.

## 2. M0 — independently choose the strongest foundation

### Required upstreams (clone and pin exact commits)

- PuckbridgePS5: https://github.com/ThisIsAkill/PuckbridgePS5 — strongly Steam-focused; README states FW 13.60 tests, wired/Puck, motion, haptics, trigger feedback, profiles, hand-off, game hooks. **Upstream claim; verify actual code.**
- OmniPad-PS5: https://github.com/diegobarbosaa/OmniPad-PS5 — USB hotplug, multiple virtual pad slots, ShellUI profile assignment, dashboard, experimental HCI code. README limits v1.0.4 to USB. **Upstream claim.**
- Ghostcontrol/PoorDS4 origins as identified by source attribution in chosen base; verify upstream remotes and licenses rather than guessing repository ownership.
- SteamlessController, SteamlessLink, SDL Triton, OpenPuck and public PS5 SDK in `docs/SOURCES_AND_EVIDENCE.md`.

#### Baseline decision matrix (mandatory deliverable `docs/BASE_SELECTION_ADR.md`)

Compare **code**, not just README, for each base:

| Criterion | Required analysis |
|---|---|
| Steam features | Exact files for USB/Puck, back buttons/grips, trackpads, motion, haptics, profile engine and game hooks |
| PS5 integration | `libScePad`, virtual pads, user assignment, ShellUI, firmware version dependencies, rollback |
| Slots/players | 1 controller hand-off versus independent simultaneous devices, reconnect and user identity |
| Latency/architecture | Polling, IO ownership, synchronization, background work, input/output buffers |
| Reliability | Build reproducibility, tests, CI, errors, cleanup, memory bounds, concurrency |
| Security | Network control, LAN debug, web endpoints, persisted profiles and credentials |
| Rights | License, inherited license, assets, copied files, GPL obligations, third-party notices |
| Integration cost | How much working code must be changed; highest risk; expected unverified portions |

Make a documented, evidence-backed choice. **Provisional recommendation: assess Puckbridge first because its claimed Steam features already overlap the product goals; use OmniPad slot/system modules only when they are demonstrably better.** This is not a directive to pick either project automatically. If neither can safely serve as base, explain why and build an independently licensed integration retaining attribution.

Record selected source and exact SHA, unmodified baseline behavior, and separate out `UPSTREAM_CLAIM` from our verification. Preserve Git ancestry when using a real GitHub fork; if creating a new derivative repository, document origin explicitly and do not misrepresent GitHub's fork relationship.

### M0 acceptance

- Base-choice ADR includes matrix, links, SHAs, risks, source/license maps and specific modules to reuse.
- Reproducible baseline host tests and PS5 build attempted; failures recorded, not hidden.
- Original behavior documented in `project-memory/PROJECT_MEMORY.md`.
- Architecture change request specifies what will be retained, extended, split or retired; no blind rewrite.

## 3. Repository/workspace bootstrap

1. Inspect user-authenticated GitHub account and repository permissions; fork the chosen upstream or create a derivative `SteamController-PS5` repo as permitted. Never overwrite someone else's repository.
2. Configure `origin` as our writable repo and `upstream` as chosen base; keep other researched bases as **read-only** remotes or source refs.
3. Pin and record source commits in `docs/PROVENANCE.md` (SHA, URL, license, purpose, code location).
4. Preserve upstream LICENSE, README attribution and copyright notices. Add the local project's README, `AGENTS.md`, this task, CI, documentation and project-memory. Review other copied assets separately.
5. Choose short branches / small PRs: `chore/bootstrap`, `fix/p0-safety`, `feat/input-core`, `feat/steam-usb-puck`, `feat/slots`, `feat/motion-touch`, `feat/feedback`, `research/ble`, `feat/web-profiles`, `research/edge-shell`.
6. Do not push secrets/keys/symbol dumps from restricted Sony SDKs, user account info, raw personal BLE MACs, console certificates or firmware material.
7. On every working session read project memory, then check current Git status, latest SHAs, CI and pending PRs before changing code.

## 4. P0 foundation safety — do before expanding code paths

**Audit selected code, including any inherited OmniPad/Ghostcontrol/Puckbridge modules. Do not blindly apply a defect known only in a different base.** For each confirmed defect: add minimal reproducer or negative test; implement safe fix; test host with ASan/UBSan or fault injection; consider PS5 semantics; document remaining hardware gate.

### P0-A: protocol/input packet safety

- Bound-check all USB HID/HCI/ACL/L2CAP/ATT/GATT lengths **against actually received bytes**; reject malformed/truncated reports before indexing or performing memcpy.
- If OmniPad's `bt_host.c` is used: fix the `len >= 8` → access `acl[8]` flaw; validate L2CAP lengths and fix lookup where virtual `slot` is treated as Bluetooth device-array index. Do not conflate these entities.
- Validate VID/PID **and** relevant interface descriptor, usage page, endpoint and report shape before selecting a parser; do not claim any generic receiver is Steam Puck because of VID alone.
- Enforce maximum packet sizes, endian conversions, report IDs, optional fields, monotonic timestamps and strict rejection paths; no uninitialized pad state or sticky pressed buttons after failed report.

### P0-B: USB hotplug / thread/resource lifecycle

- Check pthread/thread creation and connection initialization; track whether a thread actually exists before join.
- Full rollback for USB interface claim/transfer errors; no stale /dev handles, double-close, use-after-free or unchecked `stop_thread` races.
- Device ownership: scanner, IO worker, slot manager and feedback writer must not release same resource independently.
- Rapid unplug/replug, receiver standby, reconnect and payload shutdown must be bounded and idempotent.
- If OmniPad `usb_hotplug.c` is reused, verify the unchecked `pthread_create` and clean up correctly.

### P0-C: Web/network security

- **No unauthenticated LAN control** of `press_ps`, `disconnect`, `rebind`, `exit`, profile writes, controller power, shell actions or diagnostic code execution.
- Authenticate any remote administrative operation (document enrollment/token provisioning); restrict bindings by default, validate Origin/CSRF for browsers, reject invalid tokens, avoid raw credentials in logs; no "CORS means authentication" assumption.
- Disable anonymous TCP gamepad input injection by default. Test/debug bridge must be explicit opt-in, authenticated and unable to steal a physical controller slot.
- Harden HTTP request method/path/query/body parsing; `strstr` across arbitrary HTTP data is not parsing. `atoi` without error checks is unacceptable.
- Safe JSON escaping of names/control strings, bounded builder with correct `Content-Length`, partial reads/writes, timeouts, queue limits, no loop starvation, no incorrect buffer arithmetic after truncated snprintf.
- Puckbridge specifically documents a passwordless LAN portal; treat this as a security requirement for our product, not an acceptable default.

### P0-D: virtual pad, shell and game-hook safety

- Validate pad handle/ID and slot ownership; no arbitrary reassignment, duplicate virtual devices, stale input, or lost native DualSense after stopping.
- Isolate firmware-sensitive ShellUI and game hooks; support explicit opt-in, detection of unsupported firmware/ABI, rollback or safe disable upon failure, no blind patch on unknown process/build.
- Verify that pause/stop modes, failed connection and abnormal termination neutralize output and release resources.
- Preserve native controller controls in system UI; do not replace global Sony firmware resources.

### P0-E: baseline test pipeline

- Restore/repair existing `make test`/host-test target where missing test files are referenced (document original failure).
- Add reproducible host CI and ASan/UBSan where possible; fuzz deterministic protocol parsers (seed only lawful/public fixtures).
- Cross-build PS5 ELF with actual community SDK if available; log compiler/SDK version and SHA256 of output. Cross-build is **not** proof of hardware runtime.

**P0 gate:** no confirmed high-risk memory-safety or remotely exploitable control path left in modified runtime; tests pass for exercised paths; untestable platform paths are documented and disabled/guarded as needed.

## 5. Modular architecture requirements (non-negotiable)

Use `docs/ARCHITECTURE.md` as the contract authority; prefer targeted extraction/improvement of working upstream over fresh rewrites. Required **logical** boundaries (file names and directory placement may follow selected base):

1. **Transport (USB/Puck/BLE)** — connect, enumerate, read/write packet, hotplug, per-channel identity, timestamps, reliability; **no Steam button mapping** or PS5 API.
2. **Steam wire protocol** — parse/serialize USB/Puck/BLE reports and feature/output commands, strict bounds, protocol versioning, report ID mapping; **no sockets, UI, kernel, game hooks**.
3. **Steam session engine** — device lifecycle, lizard-mode disabling, heartbeat/keepalive, power/battery, reconnect, capability negotiation and per-transport ownership.
4. **Input domain model** — raw + normalized controls, independent back/grip buttons and capacitive signals, both trackpads (contact/position/click), IMU (3-axis gyro + 3-axis acceleration), timestamps, optional telemetry. Explicit validity/capability bits; don't pretend unavailable sensors exist.
5. **Mapping/profile engine** — deterministic transforms: deadzone, tap/hold/double, chords, shift/action layers, turbo with safe bounds, calibration, remappable gyro aiming and flick stick, profile selection/versioned import/export. No PS5 APIs.
6. **Slot/user/session manager** — single authoritative mapping of physical identity and transport session to virtual pad; user association, multiple pads, hand-off and atomic teardown. Exactly one active slot per physical controller identity; no slot sharing unless explicitly defined hand-off mode.
7. **PS5 backend** — default `VirtualDualSense` translating normalized data to `libScePad`; callbacks/output, system user assignment, supported FW adapters and optional safe game hooks. Ordinary DualSense is production default.
8. **Feedback engine** — typed game audio/haptics/rumble/trigger intents into physically possible Steam haptics and output reports; nonblocking queue, mixing, caps, rate limits and safety fallback.
9. **Web/API/diagnostics** — profiles, status, live input metrics, pairing/transport diagnostics and authenticated management. Does not bypass shared state engine.
10. **Platform HAL/test adapters** — mockable clock, USB, HCI/GATT, sockets, file persistence, threads, PS5 ABI; host tests use realistic fixtures, not replacement production logic.

Every changed module requires: stated responsibility, input/output/error contracts, ownership/lifetimes, thread model, extension point, tests, observability, performance and fault-handling notes. New abstraction only where there are real variants or an evidence-backed future consumer. Avoid speculative "enterprise framework" layers and duplicate wrappers.

## 6. M1 — USB-C and Valve Puck (release-critical)

### Authoritative technical references

- Puckbridge `payload/controller_sc2.c`, build and PS5 integration; actual paths must be revalidated against pinned revision.
- SteamlessController `src/steam/SteamController.h`: published VID `0x28DE`, wired PID `0x1302`, BLE PID `0x1303`, official Puck PID `0x1304`, possible receiver PID `0x1305`, vendor usage `0xFF00`, controller usage `0x0001`, dock usage `0x0002`, USB report `0x42`, BLE `0x45`, battery/status report IDs. **Treat IDs as reference evidence, not universal constants for every firmware/device.**
- SDL HIDAPI Triton driver and structures; Linux `hid-steam` as independent consistency checks.
- OpenPuck: **Puck USB + proprietary 2.4GHz radio, not normal BLE GATT**; its alternative firmware may expose different VID/PID than Valve's official receiver. Do not conflate.

### Implementation

- Enumerate multi-interface USB devices correctly; find game input interface by descriptors and validate report type before attaching.
- Support direct wired USB and official Puck via **one protocol decoder** with transport-specific frame adaptation. Separate USB physical device from logical controller identity; support multiple interfaces cleanly.
- Receive input reliably: buttons, analog axes/triggers, touchpad contacts+clicks, independent extra buttons/capacitive grips where proven, IMU, battery/power state, status messages.
- Implement protocol init, suppress lizard-mode keyboard/mouse emissions when needed, maintain refresh/heartbeat per actual transport behavior, handle asleep/awake, power off, reconnect and firmware variance.
- Avoid writing undocumented feature/output reports without testing; unsafe outputs gated off by default.
- Map common controls to virtual DualSense and preserve raw extended controls for the mapping engine.
- Do not break official DualSense, do not assign two controllers to same slot, and neutralize all controls on timeout/disconnect.
- Integrate controlled multi-Puck/multi-device support **only** after interface semantics and virtual slot semantics are validated. One-Puck/multi-controller is an independent submilestone, not guaranteed in first RC.

### Verification

- Unit tests with **real source-derived/consented input fixtures** and synthetic edge cases; verify CRC/checksum where applicable, malformed length, replay, dropped/reordered packets, sign/axis conversions, report variants, per-interface routing.
- USB/Puck smoke matrix: first connect, profile assignment, 20+ reconnect cycles (target, not claimed), suspend/resume/standby, two controllers not interfering, no hidden mouse/keyboard mode.
- Host parser quality may be `HOST_VERIFIED`; claim `HARDWARE_VERIFIED` only after actual gameplay on FW 13.60.

## 7. M2 — rich Steam input, profiles and advanced gameplay

### Extended motion

- Decode 3-axis gyro and acceleration with explicit units, handedness, scale, orientation, per-device calibration and timestamps. Validate against SDL + Puckbridge + fixtures; no arbitrary axis swap.
- Preserve native motion suitable for DualSense games where backend supports it. Separately provide remapped gyro aiming, gyro-to-stick or gyro-to-mouse semantics when appropriate. Neutralize/cap stale and extreme values, prevent drift and spikes; no motion faking if missing.

### Trackpads, extra controls

- Track two surfaces separately with contact state, coordinates, pressure/click where actually provided. Map to DualSense touch event(s) only after verifying PS5's event model. Support alternate per-game modes: touchpad, right/left stick, D-pad, radial menu, gestures, off.
- Preserve independent back buttons and capacitive grip/sense inputs in an internal namespace; configurable hold/activation instead of prematurely remapping to face buttons.
- Support configurable action layers, press/hold/double/chords, remap profiles, trigger threshold/full pull and safe turbo; deterministic timing, no delayed "stuck" inputs.
- Provide profiles scoped to game IDs with a reliable fallback to manual selection when title detection on firmware varies; versioned import/export, migration and atomic write.
- Reuse mature Puckbridge profile/mapping modules when actually superior; decompose into pure functions for host tests rather than rewriting everything.

### Verification

- Recorded exact-report golden vectors; coordinate/range/calibration tests, high-rate input, edge-triggering, touch IDs, profile switching during hold and disconnect, fallback profile safety.
- Motion/touch game behavior requires specific **real PS5 titles/tests** before declaring fully working.

## 8. M3 — haptics, adaptive trigger translation and bidirectional feedback

- Audit how Puckbridge currently captures DualSense output and game-driven audio/rumble via hooks, the provenance of its PoorDS4 bridge, and actual FW 13.60 limitations. Keep functioning code where evidence exists.
- Define `FeedbackEvent` (source, target slot, type, intensity, duration, timestamp, priority) and a transport-independent output interface. Map standard rumble, grip motors/haptics and supported trigger events to physically possible sensations; do **not** claim real adaptive resistance on Steam hardware that lacks resistance motors.
- Serialize output reports per transport, throttle/coalesce, guard battery/thermal power, stop safely upon transport shutdown. Fail closed for unsupported effects.
- Game hooks must be isolated, version/fingerprint aware and opt-in where invasive; always preserve basic input on hook failure. Never port unexplained offsets from one firmware to another.
- Test valid/invalid command encoding, bounds, mixing, cancellation, priorities, feedback when profile/handoff changes. Confirm in real games before marking `HARDWARE_VERIFIED`.

## 9. M4 — BLE feasibility and staged implementation (research/high risk)

**Critical confirmed distinction:** internal PS5 Bluetooth hardware exists; PS5 Linux has `btusb`/xHCI work, but this is **not** proof of arbitrary LE/GATT from ProsperoOS FW 13.60. `libSceBluetoothHid` exports describe HID reports/devices, not a proven GATT Central client. The retail console rejected a tested BLE-only Logitech Craft keyboard while Classic HID worked (specific report; not universal LE impossibility). Do not claim BLE support until hardware gates pass.

### 9.1 Source audit and static diagnostics

- Read `docs/BLE_FEASIBILITY.md` and primary links. Cross-check `libSceBluetoothHid` names/NIDs with `DNNDHH/PS5-3.20_Libs` and `SvenGDK/SharpProspero`, recording exact SHAs and firmware of reference. SharpProspero declares 17 exports, discusses privileged `/dev/bt` and `/dev/bluetooth_hid`, and unknown pointer structure layouts — **not** confirmed on 13.60.
- Search public community SDK for OS-managed BLE GAP Central/GATT APIs; differentiate accepted OS symbols, emulator stubs, Sony Partner SDK (restricted), PS5 Linux drivers and USB HCI experimentation.
- Investigate Marvell `1286:2059` evidence; identify actual model/chip on target rather than assume all PS5 units match. Review xHCI/BTUSB Linux patch PR #19 and its instability warning.
- Build **read-only opt-in diagnostic ELF**: OS model/FW, module availability, symbol resolution, safe device-node metadata, privileges, error codes; redact private identifiers; no unknown-ABI calls.
- No firmware writes, HCI reset/scan, USB interface claim/rebind or driver takeover on the stock radio in this passive stage. A call named `Init` is not necessarily side-effect free. Require distinct manual approval for active radio experiments.

### 9.2 Capability gates (do not skip)

- **B0:** Host research + source/ABI matrix (all assertions pinned and categorized).
- **B1:** Read-only FW 13.60 probe with normal DualSense functioning before/during/after.
- **B2:** Approved low-impact OS-managed BLE availability test, with exact API arguments/contracts and reversible side effects.
- **B3:** Real LE scan: timestamped valid LE advertising report `LE Meta Event 0x3E` or equivalent reliable system API. Merely seeing the PS5 Bluetooth pairing menu **does not count**.
- **B4:** LE connect + controlled SMP/bond + GATT service discovery; paired native DualSense stays functional.
- **B5:** Steam Controller 2026 GATT notifications (e.g. Valve service from SteamlessLink), verified report content and feature/output writes.
- **B6:** Input → Steam protocol core → virtual DualSense → PS5 game, including reconnect and recovery tests.

Each gate must include source/revision, test ID, hardware model, FW, before/after DualSense test, exact logs/errors and abort/rollback. If a gate fails, STOP native BLE probing and retain USB/Puck release track.

### 9.3 BLE transport design

- Reuse the one Steam input/session core. Adapt full BLE input (`0x45` and newer characteristic-specific `0x47` according to SteamlessLink), reconstruct report IDs only when transport requires, validate GATT notifications and output/feature report channels.
- Generic BLE central API must support discovery, service/characteristic mapping, notifications, verified secure pairing and key storage, timeouts, disconnection, clean session/slot ownership.
- Investigate ownership-safe external USB Bluetooth dongle if native system API is not proven, or optional Android `SteamlessLink` bridge for **prototype only**; network bridge must not be anonymous or direct unsafe input injection.
- BTstack may be used as architectural reference; its noncommercial license is not automatically GPL-compatible. Do not import source without documented rights approval.
- No BLE work may interfere with PS5's internal radio/controller service without approved safe experiments.

## 10. M5 — optional DualSense Edge and controller identity research

- Default production backend is **regular DualSense**; Steam Controller rear buttons and extra sensors remain independent inside the input core.
- Research Edge-specific HID fields (two paddles and Fn controls) separately from **PS5 game API exposure**. Linux HID bits do not prove games receive unique Edge actions. Do not set Edge VID/PID and call that real Edge emulation.
- Edge proof-of-concept only after finding registration ABI, virtual pad class, event handling, game/API visibility, user assignment and rollback on 13.60. If benefit is nil or unverified, remain normal DualSense and remap extra buttons via profiles.
- Optional ShellUI cosmetic alias `Steam Controller` and unique icon must be per-virtual-device, reversible and opt-in; first research source of display names/assets. Never globally replace the real Edge icon or write Sony system resource files.
- Both features must be isolated so release can ship without them.

## 11. M6 — web configuration, diagnostics and usability

- Build authenticated, LAN-usable UI for controller/transport/slot state, battery, raw and normalized live input, motion/touch calibration, remaps, action layers, feedback settings, game profiles, error logs, pairing and rollback controls.
- Keep text/status truthful: show `USB`, `Puck`, `BLE experimental`, `connected`, `paired`, `hardware verified` separately; never infer pairing success from scan alone.
- Validate persistent profile schema/version; atomic writes, backup/export/import, corrupted profile recovery, filenames/path traversal defense, bounded storage and safe default configuration.
- No unauthenticated mutation endpoints; safe reload/reboot handling; browser should not hang 250Hz input loop.
- If reusing Puckbridge existing portal, harden rather than replacing its UI gratuitously.

## 12. Engineering and test requirements

### Performance/robustness

- Preserve baseline loop cadence; do not claim measured latency from `usleep(4000)` or stated 250Hz; measure on target when available.
- No unbounded allocations, queues, CPU spin, blocking sockets in hot path or UI/HTTP processing in realtime input loop. Enforce no stale input after disconnect; bounded buffer/queue limits.
- Explicit lock hierarchy or single-thread-owned state with message passing; no unguarded data races. No logging raw secrets, addresses, serials, keys, profile passwords.
- Fail-safe on malformed packets, controller power loss, game restart, existing shell-hook failure and sleep/wake. Correct behavior more important than maximum advertised feature set.

### Mandatory host CI

- `clang` C99 strict warnings, `-fsanitize=address,undefined` where supported, tests for parsers/mapping/session/slots/output.
- Deterministic fixtures from source-backed reports and synthetic adversarial frames; consent/sanitization for any captures. Property/fuzz targets for HID/GATT/HTTP/profile decoders when beneficial.
- Fault injection: short read/write, USB init/thread creation failure, descriptor truncation, reconnect collision, status read timeout, profile corruption, HTTP overflow, token bypass, double release, contradictory transport notifications.
- CI must not falsely mark runtime PASS when PS5 hardware cannot run. Add cross-build job if actual PS5 SDK available/legally redistributable.

### Mandatory hardware matrix (when hardware is available)

- FW 13.60 with console type (Fat/Slim/Pro), loader/SDK versions, actual SC 2026 firmware revision, official Puck version, USB-C cable type.
- Native DualSense untouched baseline; PS5 home UI; profile selection; one native PS5 game and one secondary title, repeatable menu/navigation; other controllers do not interfere.
- Wired and Puck connect/disconnect/standby/reconnect, profile change under active input, button release after timeout, no phantom duplicates and no uncontrolled game hooks.
- Motion/trackpad game scenarios, audio/haptics, trigger feedback only when supported by hardware, save/restart/cleanup. Log actual repeat count and results, not expected counts.
- BLE gated as in section 9; no default invasive experiments.

### Evidence levels for release table

`DESIGNED` → `IMPLEMENTED` → `HOST_VERIFIED` → `PS5_BUILD_VERIFIED` → `HARDWARE_VERIFIED` → `RELEASE_CANDIDATE`. Levels are **not automatically cumulative** where host/PS5 work differs; record each test separately.

## 13. Deliverables and roadmap

| Milestone | Concrete deliverable | Release gate |
|---|---|---|
| M0 | Source comparisons, ADR, license/provenance matrix, pristine build | Code pinned and selection justified |
| M1 | P0 hardening, deterministic host CI, baseline PS5 ELF | Tests passed or blockers explicit |
| M2 | USB wired SC2026 + official Puck → single virtual DualSense | Host verified; runtime claims only if PS5 tested |
| M3 | Extended input, motion/touch, action layers, gyro aiming/flick, profiles | Host + hardware feature matrix |
| M4 | Feedback/haptics/trigger intent and safe optional game hook | Hardware game test and rollback |
| M5 | BLE research gates B0–B6, implementation if proven | No stock BT disruption; separate status |
| M6 | Web UI, recovery, docs, release packaging and PRs | Security, license and release matrix passed |
| R | Optional Edge/UI identity R&D | Only if evidence supports, no release block |

Milestone ordering may be adjusted when dependency tests justify it. **Do not block USB/Puck release on native BLE, Edge, ShellUI icon or multi-Puck multi-player.** Ship an honestly scoped release candidate if those remain unverified.

## 14. Project memory (must be maintained continuously)

Persistent files in `project-memory/` are the **source of truth for cross-session continuity**:

- `PROJECT_MEMORY.md` — short current state, latest verified milestone, active blockers, precise next action.
- `FINDINGS.md` — each finding ID, evidence status, exact source URL/SHA/file/line, impact, follow-up.
- `ISSUES.md` — P0/P1/P2 problem register, reproducer, branch/commit, owner, state, risk.
- `TEST_LOG.md` — append-only commands, environment, result, logs/hash, negative tests; record NOT_RUN explicitly.
- `DECISIONS.md` — architecture decisions/ADR references, options, rationale, change trigger.
- `SESSION_LOG.md` — timestamp, branch, commits, changes, tests, next commands, blockers.
- `HARDWARE_MATRIX.md` — exact observed configurations and feature states.
- `PROVENANCE.md` (in `docs/`) — code/library source attribution, license, original commit, modifications.

At the start of each Codex session, reread all memory and reconcile it with actual Git history/CI. After **each meaningful fix or test**, update the relevant files. Before ending, leave exact next steps in `SESSION_LOG.md`, not vague summaries. Never overwrite a failed test; append a superseding rerun.

## 15. GitHub workflow and PR publication

- Confirm repository permissions and authentication. Reuse suitable fork if present. Don't publish until baseline and scope are understood.
- Commit reviewable changes with clear subject prefixes `fix`, `feat`, `test`, `ci`, `docs`, `research`; use small branches and PRs. First: safety + test foundation; then USB/Puck; then rich input; feedback; BLE feasibility; UI, followed by Edge experiments as separate drafts.
- PR body must contain purpose, source SHA/attributions, defects and reproducible before/after, modifications, exact CI/host and PS5 build commands, manual hardware matrix, safety/security implications, rollout/rollback, and outstanding blockers.
- If PS5 hardware test is absent, use **Draft PR** and label hardware status accurately; this may be merged for host-safe isolated modules per project policy, but never assert hardware success.
- Upstream improvements in independent common modules may be offered to the original author separately **only** after maintaining our tested mainline. Never force push to upstream or rewrite upstream history.
- Create a release only when required release gates pass, clearly mark experimental and unsupported features, include changelog, license notices, source SHA, ELF SHA256, compatibility and recovery instructions.

## 16. Execution instructions and final report

**Start now**: read docs/memory, inspect live GitHub source, pin SHAs, choose base with ADR, establish baseline, then implement P0 and begin USB/Puck. Continue through all feasible milestones autonomously. If a capability needs physical hardware and none is connected, finish host-verified portions, provide exact manual test instructions and mark remaining work BLOCKED. Do not request repeat confirmation for choices already specified; only seek approval before potentially disruptive Bluetooth commands, irreversible platform changes, release/publication requiring explicit permissions, or other truly necessary actions.

When concluding a development iteration, report:

1. Current target repo, branch, upstream pinned commits and base choice rationale.
2. Actual files changed, commit hashes, PR URLs and statuses (if actually created).
3. Fixed defects and actual reproduction evidence.
4. Exact commands and results for host tests, sanitizers, PS5 cross-build and hardware, or clear NOT_RUN reason.
5. Function-level status for USB, Puck, input, gyro, touch, haptics, profiles, native BLE and Edge — no conflation of plans with verification.
6. Open P0/P1/P2 with next actions and links to memory documents.
7. Artifacts with checksum and licensing attributions, only when created.
8. Updated memory snapshot and exact next safe action.

**Definition of done for initial release:** a secure, reproducible single payload with Steam Controller 2026 USB and official Puck support verified on actual FW 13.60 hardware, basic DualSense functions and reliable cleanup; high-value extended functions enabled only where tested; no invented BLE/Edge/firmware claims; clean source/license/test/memory/PR trail. If no PS5 is available, state "host development complete; hardware release gate outstanding" rather than calling the initial release done.
