# Image-based button remapping

**Evidence:** `VERIFIED_HOST_TEST` (181 Microsoft Edge browser checks + nine portable asset/geometry/embedding tests); `VERIFIED_PS5_BUILD`. Actual portal operation on PS5 FW 12.00 remains **NOT_RUN / BLOCKED_HARDWARE**.

![Steam Controller → DualSense](images/remap-desktop.png)

## Use

1. Open **Remap → Buttons** in the authenticated portal and choose the profile to edit. WebUI starts automatically when the runtime policy is absent; enter the short-lived code from the console notification. Advanced token recovery is available in the pairing dialog. Use only a trusted isolated LAN.
2. Click/tap a Steam Controller button, or focus it and press Enter/Space. Use **Back / top** for trigger pulls and the four rear buttons. The image is viewed from behind: physical L is on the right, R on the left.
3. Choose one or more DualSense outputs in the editor's grouped picker or preview image. Green outlines and badges show the draft assignment for the active activator. On the main page, first select a Steam input, then click/tap a DualSense output: it is staged for the **Press** action, and **Done** applies it while **Cancel** discards it. Coarse-pointer hit areas extend around small output hotspots.
4. **Done** updates the draft profile. **Cancel** or Escape preserves its previous assignment. **Save changes** writes through the authenticated `/api/profile` API, then rereads the canonical profile before reporting Saved. Server-normalized values replace the submitted draft; newer local edits remain unsaved. Failed or incomplete readback keeps a “Save not confirmed” state; changing a game profile uses the existing load/save flow.

Press, Long press, Double press, Shift layer, trigger Full pull, Turbo and Toggle use the shared binding engine. The configured Shift input is shown as a **Shift modifier** and all of its button outputs, including trigger Full pull, are suppressed and neither highlighted nor editable while it is the modifier. Its stored assignments remain available when a different Shift input is selected. Analog trigger axis behavior is unchanged. The assignment list expands to show all 24 inputs. QAM is visible and reserved. L3/R3 are stick clicks, and Touchpad is explicitly labeled **Touchpad click**. Gyro, stick axes, trackpad surface modes and adaptive trigger feedback stay in their dedicated sections. Internal Puckbridge actions appear in their own picker group; they are not fictitious physical DualSense controls. Grip Sense, USB-C, the magnetic charging connector and Mute are informational.

The **Import** action validates input/output IDs and known setting keys before changing anything; malformed identifiers, reserved QAM assignments, invalid flags and unknown keys reject the entire file. Assignment rows use DOM nodes and output identifiers are allowlisted even for corrupt local state. Turbo and Toggle are mutually exclusive across editor, import, save and C parsing; Toggle wins an ambiguous imported pair, independent of line order.

The **Import** action merges settings: values present in the file replace the current values, while omitted settings are preserved. An empty or `NONE` activator value clears that activator. `NONE`/empty activators do not appear as phantom “Nothing” actions. Face-button assignments use the PlayStation glyphs (✕ ○ □ △) in visible summaries; their accessible labels retain the spoken button names.

[Back / top screenshot](images/remap-desktop-back.png) · [Editor screenshot](images/remap-editor.png) · [Mobile screenshot](images/remap-mobile.png)

## Implementation and build

- `payload/web/index.html`: production HTML/CSS/JS; one logical key per input across both views; transient editor state is separate from profile state. No PS5 ABI or new output protocol IDs.
- `payload/web/controllers/layouts.json`: keeps the Steam reference dimensions and records the manually checked DualSense button centers. DualSense front/back L1/R1 regions target the upper bumper buttons; L2/R2 target the lower triggers. Triangle and circle centers are aligned to the supplied source artwork. The four user-provided transparent WebPs and rights notice are unchanged.
- Keep Steam logical `iw`/`ih` and `vb` values: front decodes at 1500×1091 but its geometry is 2566×1867; back is 1400×1007 with 2136×1536 geometry. SVG scales the image into that coordinate system.
- `payload/gen_html.py`: embeds the four WebPs and geometry alongside the existing icons in the generated header. The generator works from any current directory. Never edit `webui_html.h` manually.
- `payload/Makefile`: artwork, layout JSON and button icons are dependencies of `webui_html.h`. No CDN, remote font or framework is needed. The generated page is 685,503 UTF-8 bytes. On coarse pointers, destination hotspots accept taps up to 16 CSS pixels outside the visible shape; selection prioritizes actual SVG fill containment, then distance to the transformed shape boundary (curves sampled with segments no longer than one CSS pixel), with center distance as a tie breaker. Front/back shoulder fills and neighboring D-pad/triangle zones are disjoint.

```sh
python3 payload/gen_html.py
make -C tests clean test
make -C tests clean sanitize
# Set SDK v0.43 / LLVM_CONFIG as documented in BUILDING.md first:
make -C payload clean
make -C payload
```

## Reproduce browser checks

Use an installed Playwright CLI and Chromium-family browser; they are development tools, not product dependencies. In one terminal:

```sh
python3 payload/gen_html.py
python3 tests/webui_preview.py
```

In another terminal (recorded browser: Microsoft Edge 152.0.4191.53; Playwright CLI 0.1.22):

```sh
mkdir -p output/playwright
playwright-cli open about:blank --browser msedge
playwright-cli run-code --filename tests/webui_remap_browser.js
playwright-cli close
```

Require the returned object to contain `result: PASS` and at least `checks: 181`; the workflow also parses the JSON result so a displayed CLI error cannot pass on shell status alone. The scenario creates its own clean tab, resets only the loopback fixture profile via HTTP, and checks calibrated front/back L1/R1 targets and face centers, Shift semantics, symbol labels, grouped outputs, staged DualSense assignment and Cancel, Merge import and `NONE` normalization, all physical input/output hotspots, activators, QAM reservation, token-bearing save/reload, profile switching, keyboard behavior, responsive layout, expanded coarse-pointer hit targets and Games/Settings navigation. Four screenshots are written to `output/playwright/`. The GitHub Host workflow requires 181 checks on every push; [run 38070068676](https://github.com/MNeroba/SteamController-PS5/actions/runs/38070068676) passed the normal suite, ASan/UBSan and Chromium browser regression for the published security follow-up. Machine-readable results from this execution are in [webui-remap-host-results.json](webui-remap-host-results.json).

The fixture binds only 127.0.0.1:8765, uses the deliberately public `preview-only` token and RAM-only profiles. On startup it builds `tests/webui_profile_fixture.c`, which delegates normalization to the production C parser/formatter. Host regression also checks hostile file rejection, failed/incomplete readback, concurrent editing/profile switching during Save, all SVG output fills at a two-source-pixel sampling interval, touch boundaries on front/back at 390/768 pixels, and access to the final output group in the mobile dialog. It does not simulate PS5 device services or establish console HTTP, filesystem, browser, USB, controller, firmware or rendering compatibility. H-09 on the real portal remains required; verify both transports, profile persistence and live highlighting there. Asset publication was authorized by the owner; see [provenance](PROVENANCE.md) and the artwork notice.

A full editor Save explicitly transmits `NONE`/`0` for cleared optional activators/flags because the existing C API merges updates. Canonical readback therefore confirms clears as well as added bindings. Merge import remains distinct: omission preserves current draft values, while a supplied empty/`NONE` activator clears that field.

## Profile integrity and operation ordering

- Disk snapshots require every base input and scalar setting, valid known values, final newline and complete EOF. Oversized, truncated, duplicate-key, unknown-token and NUL-corrupt files fail without changing the caller's profile. Invalid existing files remain available for recovery; startup does not overwrite them and POST refuses to merge into them. Partial authenticated updates remain supported after strict validation. Profile names must fit in 47 UTF-8 bytes.
- Serialization returns bytes excluding NUL or -1, never successful truncation. `SC2_PROFILE_CAPACITY` is 24 KiB including NUL; all supported combinations fit. The store checks formatting/validation before opening its temporary file and keeps the shared mutation mutex and atomic rename. Failure preserves the old snapshot.
- Leaving a dirty/unconfirmed profile offers **Save / Discard / Cancel**, including game-list creation, Now Playing and deletion. Save must receive complete canonical POST→GET confirmation. Cancel creates no profile; failed Save preserves the draft. Finish or cancel an open button editor first.
- Load/Create/Delete use request generations and a draft snapshot; stale completions cannot change the editor. Save additionally uses an editor generation so returning to the same profile ID cannot attach an old confirmation. List refreshes reject stale results. Back/Forward retains a pending destination in history state while the displayed hash reflects the committed profile. A newer draft always wins.

### Activator compatibility

Long + Turbo and Long + Toggle are **supported**. A short tap pulses ordinary Press for 90 ms, without Turbo or Toggle. At LONG_MS, Turbo starts the Long destination in its on phase and alternates every TURBO_MS until release; Toggle changes its Long latch once per qualifying hold (another qualifying hold clears it). Releasing a long hold sends no ordinary tap. A short tap while Long is latched may pulse Press alongside it. Double on the second press takes precedence over that hold's Long/Turbo/Toggle activation; an already latched output remains active. Shift selects its effective Press destination and bypasses Long/Double for the current hold; ordinary Turbo/Toggle act on that destination. Toggle wins an ambiguous Turbo+Toggle import independently of line order. Assignments are retained by editor, import and C parsing.

Ordinary trigger destinations retain proportional pressure, including Shift, cross mappings and combinations. Effective digital Long/Double/tap/Turbo/Toggle/Full Pull sources contribute 255 only while active. Multiple analog sources targeting one trigger use their maximum pressure. A Shift modifier contributes no ordinary pressure or Full Pull. The binding engine provides this provenance; the PS5 adapter contains no second activator timer.

### Complete page delivery

The production HTTP writer is tested against the actual embedded page, byte-for-byte, with short writes, transient EINTR/EAGAIN, deadline expiration, EPIPE and zero writes. EAGAIN retries within the original absolute deadline; the 5-second response timeout is unchanged. Failures are logged and the client is closed. Content-Length describes the complete intended body; a failed response is incomplete and must not be treated as successful by the client. ProsperoOS socket semantics, latency and real FW 12.00 browser delivery still require H-09 measurements.

## Binding, persistence and navigation audit — M1-220 (2026-10-10)

`VERIFIED_HOST_TEST`: full normal and ASan/UBSan suites PASS; profile 52 assertions, atomic storage 71, binding/proportional-pressure 165 and HTTP/auth 110. Nine portable artwork/generator contracts and 181 Edge/Playwright browser assertions PASS (all original 162 retained). The maximum-complexity stored profile is 14,573 bytes; complete 24-input/22-output/activator round-trip succeeds within the 24 KiB capacity. Final generated page: 685,503 UTF-8 bytes, 41 icons/four unchanged controller images. Clean SDK v0.43/LLVM 18.1.8 cross-build PASS, ELF SHA-256 `5bdd18153301213e7a4b82e3038dff24563fdc7e0e474ce41546d3c66ffc7dbc`. This is local source/build evidence; publication and Actions acknowledgement follow separately. Physical FW 12.00 remains NOT_RUN.

## First launch and recovery

The public interface initializes before authentication. Enter the eight-digit local notification code in the integrated desktop/mobile dialog; browser reload retains the session credential. Rejected credentials clear authentication while preserving unsaved edits. Backend-offline state keeps valid credentials for reconnect. Protected game icons use authenticated fetch and local blob URLs.

A corrupt stored Default offers confirmed, authenticated recovery with a backup of its exact previous bytes. The profile store serializes load/patch/save for both menu and portal. See [loading and advanced recovery](development/LOADING.md). These flows have host/Chromium coverage; console notification, browser and filesystem behavior require H-09 hardware validation.
