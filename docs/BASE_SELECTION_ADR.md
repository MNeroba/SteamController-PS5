# ADR-001 — Foundation selection for SteamController-PS5

Status: **ACCEPTED — 2026-10-08**

Date: 2026-10-08

## Context

PuckbridgePS5 already claims the primary SC 2026 feature set and has native PS5 game integration; OmniPad offers multiple virtual slots/USB hotplug/ShellUI integration but its direct BT remains experimental. We need one coherent, secure payload rather than two independently running hooks/slot systems.

## Options and criteria

1. **Puckbridge base** + selective OmniPad improvements: likely least reimplementation of Steam features; verify safety, single-player architecture and future multi-slot costs.
2. **OmniPad base** + careful transfer of Puckbridge SC modules: likely stronger slot system but may require extensive haptics/game hook porting.
3. **Clean common-core derivative** using attributed proven modules only: only if both fail critical maintainability/licensing constraints; larger execution cost.

Score after inspecting pinned source and baseline test, for: PS5 13.60 actual evidence (20%), existing SC extended features (25%), slot/handoff architecture (15%), P0 safety (15%), ease of host validation/build (10%), license compatibility (10%), maintenance complexity (5%). Weights are decision aids, not proof; provide specific observations rather than arbitrary numeric confidence.

## M0 audit result

The comparison used the exact public `main` snapshots below. These are source observations; the authors' hardware claims remain claims and were not reproduced here.

| Criterion | PuckbridgePS5 | OmniPad-PS5 | Result |
|---|---|---|---|
| Pinned source | `ThisIsAkill/PuckbridgePS5` at `78460113ef7c48502c765a1f83ba34d53c977134` | `diegobarbosaa/OmniPad-PS5` at `b7682efce18ee0a8d7c80bbc90afbce72151ef96` | Both snapshots cloned and inspected; exact SHAs are fixed for this decision. |
| Steam Controller 2026 | `payload/controller_sc2.c` parses input/battery reports; `gc_main.c` discovers USB interfaces/endpoints; `sc2_profile.c`, `sc2_binding.c`, `sc2_gyro.c`, `sc2_flick.c`, `sc2_haptics.c`, `sc2_trigfx.c` implement the advertised features. | No Steam Controller 2026 parser or Valve product-ID path in the v1.0.4 source tree. | Puckbridge is the only viable base for the product's defining controller support. |
| PS5 virtual pad | Existing Ghostcontrol-derived `ScePadData`/virtual-device and handoff path in `gc_main.c`, `shellui_pad.c`, `ds_handoff.c`. | `src/ps5_vpad.c` manages three virtual slots and USB reader lifecycle. | OmniPad's slot lifecycle is a design reference; adopting it wholesale would replace a working Steam-to-pad path. |
| Input architecture | Functional pipeline exists, but `controller_sc2.c` combines wire parsing, profile processing, and `ScePadData` output. | Generic input normalization and slot modules are more separated, but do not implement this controller. | Keep Puckbridge behavior; split pure Steam parsing/state from PS5 mapping in staged refactors with golden vectors. |
| Security / P0 | `payload/webui.c:288-315` binds `INADDR_ANY`; `:175-281` exposes mutations and diagnostics without authentication. | `src/web.c` and `src/tcp_stream.c` expose unauthenticated controls/injection; Bluetooth ACL parsing has unsafe short-length handling. | Both need security work. Do not import OmniPad web, TCP, or Bluetooth code. Puck portal authentication is the first selected-base P0. |
| USB lifecycle | `gc_main.c:1073-1103` checks reader-thread creation and rolls back the virtual slot on failure. | `src/usb_hotplug.c` ignores `pthread_create` and then joins the thread handle. | Puckbridge is safer for the needed hotplug reader path at the pinned source. |
| Host validation | No host test suite or test files found in the pinned tree; PS5 build requires unavailable `PS5_PAYLOAD_SDK`. | `make test` passes parser, vpad ABI and flow simulation, then fails because `tests/test_normalization.c` is missing. | Neither baseline gives complete validation; Puckbridge needs new portable tests. |
| Licensing | Root GPL license and README say GPL-3.0-or-later; embedded PoorDS4 notice records upstream revision `476b0b667ce82c6e51ffd9e96d7ef1f5bfc74d4b`; root README credits Ghostcontrol and other sources. Portal controller illustration's specific license is unclear. | Root GPLv3; no OmniPad source is copied into this project. | Puckbridge is compatible with the intended GPL family subject to completing notices and excluding the uncleared illustration. |

## Decision

Adopt **Option 1: PuckbridgePS5 as the executable foundation**, pinned to `78460113ef7c48502c765a1f83ba34d53c977134`. The direct Steam-controller support and existing PS5 lifecycle outweigh OmniPad's stronger-looking generic slot separation. OmniPad remains a source-level design reference only; no OmniPad source is imported.

The source snapshot and pinned Git metadata are present in this workspace. The local `HEAD` remains the selected Puckbridge SHA; local `fix/p0-safety` changes are published through the connected GitHub app to the user's derivative repository and draft PR #1. The local Git author identity and GitHub CLI credentials are not configured, so the local branch cannot create those commits; GitHub app publication has been confirmed by the host. `puckbridge-upstream` and `omnipad-reference` are fetch-only remotes; the user-supplied `MNeroba/SteamController-PS5` URL is configured as `origin`. The pristine comparison trees remain available at `/private/tmp/SteamController-PS5-audit/` for this session.

### Keep and stage for refactor

- Keep `payload/gc_main.c`, `usb_helpers.*`, `controller_sc2.*`, profile/binding/motion/haptics modules and the existing virtual DualSense backend as the first functional route.
- Keep the `payload/bridge/NOTICE.md` and source-level attributions. Keep the game bridge optional and capability-gated; treat its game-process hooks as high-risk, firmware/game-specific code pending stronger rollback tests.
- The SC report decoder is now a host-buildable pure module returning typed input, battery, and link state; preserve report-to-profile behavior behind the existing adapter and extend golden vectors before further changes.
- USB configuration descriptor parsing is now a pure bounded module shared by Puck and wired endpoint maps. Keep parser behavior fail-closed and expand fixtures before changing interface selection.
- Build a single authoritative session/slot layer around the existing slot table before adding another transport. Do not duplicate OmniPad's virtual-pad manager.
- Replace unauthenticated web mutations with provisioned bearer-token authorization and regressions. Keep the raw TCP/debug input service out of the release path.
- Serialize VDA creation while it is correlated through the shared klog queue; classify malformed known input as neutral and prevent stale controls; keep game-process hooks opt-in and remove hooks when the selected title changes.
- Exclude the handoff-only `steam-controller.jpg` from Git; it is not used by the payload. Preserve the portal drawing attribution and root GPL notice; the upstream README does not give that inline drawing a separate license.

### Reconsideration / rollback triggers

Reopen this ADR only if Puckbridge Steam parsing fails valid golden vectors, its USB/Puck path cannot build or recover under a licensed SDK, or the Ghostcontrol/PoorDS4 rights chain cannot be documented. Any refactor is rolled back if valid pinned-source report fixtures produce changed button/axis/motion/haptic output, or disconnect/reconnect tests violate slot cleanup. No OmniPad module is to be imported as a shortcut around a Puckbridge defect; evaluate the exact module, license, tests, and security boundary first.

### Limits of this decision

Puckbridge's README claims firmware 13.60 and several gameplay features, but this audit confirms only the source paths. No PS5, controller, Puck, game, firmware, or SDK was available for independent validation. It does not establish that the imported payload builds, that the hooks are safe for arbitrary games, that all advertised features work, or that two Steam Controllers can use one Puck.

Decision owner: Codex, based on pinned source audit. Reopen if the triggers above occur.
