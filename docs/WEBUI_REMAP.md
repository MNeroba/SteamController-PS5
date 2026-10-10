# Image-based button remapping

**Evidence:** `VERIFIED_HOST_TEST` (114 Microsoft Edge browser checks + eight portable asset/geometry/embedding tests); `VERIFIED_PS5_BUILD`. Actual portal operation on PS5 FW 12.00 remains **NOT_RUN / BLOCKED_HARDWARE**.

![Steam Controller → DualSense](images/remap-desktop.png)

## Use

1. Open **Remap → Buttons** in the authenticated portal and choose the profile to edit. WebUI remains disabled by default; follow the safe opt-in and enrollment steps in the root README.
2. Click/tap a Steam Controller button, or focus it and press Enter/Space. Use **Back / top** for trigger pulls and the four rear buttons. The image is viewed from behind: physical L is on the right, R on the left.
3. Choose one or more DualSense outputs in the editor's grouped picker or preview image. Green outlines and badges show the draft assignment for the active activator. On the main page, first select a Steam input, then click/tap a DualSense output: it is staged for the **Press** action, and **Done** applies it while **Cancel** discards it. Coarse-pointer hit areas extend around small output hotspots.
4. **Done** updates the draft profile. **Cancel** or Escape preserves its previous assignment. **Save changes** persists it through the existing authenticated `/api/profile` API; changing a game profile uses the existing load/save flow.

Press, Long press, Double press, Shift layer, trigger Full pull, Turbo and Toggle retain their original behavior. The configured Shift input is shown as a **Shift modifier** and its ordinary output is neither displayed as active nor editable while it is the modifier, matching the binding engine. The assignment list expands to show all 24 inputs. QAM is visible and reserved. L3/R3 are stick clicks, and Touchpad is explicitly labeled **Touchpad click**. Gyro, stick axes, trackpad surface modes and adaptive trigger feedback stay in their dedicated sections. Internal Puckbridge actions appear in their own picker group; they are not fictitious physical DualSense controls. Grip Sense, USB-C, the magnetic charging connector and Mute are informational.

The **Import** action merges settings: values present in the file replace the current values, while omitted settings are preserved. An empty or `NONE` activator value clears that activator. `NONE`/empty activators do not appear as phantom “Nothing” actions. Face-button assignments use the PlayStation glyphs (✕ ○ □ △) in visible summaries; their accessible labels retain the spoken button names.

[Back / top screenshot](images/remap-desktop-back.png) · [Editor screenshot](images/remap-editor.png) · [Mobile screenshot](images/remap-mobile.png)

## Implementation and build

- `payload/web/index.html`: production HTML/CSS/JS; one logical key per input across both views; transient editor state is separate from profile state. No PS5 ABI or new output protocol IDs.
- `payload/web/controllers/layouts.json`: keeps the Steam reference dimensions and records the manually checked DualSense button centers. DualSense front/back L1/R1 regions target the upper bumper buttons; L2/R2 target the lower triggers. Triangle and circle centers are aligned to the supplied source artwork. The four user-provided transparent WebPs and rights notice are unchanged.
- Keep Steam logical `iw`/`ih` and `vb` values: front decodes at 1500×1091 but its geometry is 2566×1867; back is 1400×1007 with 2136×1536 geometry. SVG scales the image into that coordinate system.
- `payload/gen_html.py`: embeds the four WebPs and geometry alongside the existing icons in the generated header. The generator works from any current directory. Never edit `webui_html.h` manually.
- `payload/Makefile`: artwork, layout JSON and button icons are dependencies of `webui_html.h`. No CDN, remote font or framework is needed. The generated page is 674,696 UTF-8 bytes. On coarse pointers, destination hotspots accept taps up to 16 CSS pixels outside the visible shape; closest-hotspot selection keeps nearby controls distinct.

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

Require the returned object to contain `result: PASS` and at least `checks: 114`; the workflow also parses the JSON result so a displayed CLI error cannot pass on shell status alone. The scenario creates its own clean tab, resets only the loopback fixture profile via HTTP, and checks calibrated front/back L1/R1 targets and face centers, Shift semantics, symbol labels, grouped outputs, staged DualSense assignment and Cancel, Merge import and `NONE` normalization, all physical input/output hotspots, activators, QAM reservation, token-bearing save/reload, profile switching, keyboard behavior, responsive layout, expanded coarse-pointer hit targets and Games/Settings navigation. Four screenshots are written to `output/playwright/`. The GitHub Host workflow runs this browser scenario on every push. Machine-readable results from this execution are in [webui-remap-host-results.json](webui-remap-host-results.json).

The fixture binds only 127.0.0.1:8765, uses the deliberately public `preview-only` token and RAM-only profiles. It does not simulate PS5 device services or establish console HTTP, filesystem, browser, USB, controller, firmware or rendering compatibility. H-09 on the real portal remains required; verify both transports, profile persistence and live highlighting there. Asset publication was authorized by the owner; see [provenance](PROVENANCE.md) and the artwork notice.
