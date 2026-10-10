# Image-based button remapping

**Evidence:** `VERIFIED_HOST_TEST` (99 browser checks + seven portable asset/embedding tests); `VERIFIED_PS5_BUILD`. Actual portal operation on PS5 FW 12.00 remains **NOT_RUN / BLOCKED_HARDWARE**.

![Steam Controller → DualSense](images/remap-desktop.png)

## Use

1. Open **Remap → Buttons** in the authenticated portal and choose the profile to edit. WebUI remains disabled by default; follow the safe opt-in and enrollment steps in the root README.
2. Click/tap a Steam Controller button, or focus it and press Enter/Space. Use **Back / top** for trigger pulls and the four rear buttons. The image is viewed from behind: physical L is on the right, R on the left.
3. The existing editor opens. Select one or several DualSense outputs in the picker or preview image. Green outlines and badges show the draft assignment for the active activator.
4. **Done** updates the draft profile. **Cancel** or Escape preserves its previous assignment. **Save changes** persists it through the existing authenticated `/api/profile` API; changing a game profile uses the existing load/save flow.

Press, Long press, Double press, Shift layer, trigger Full pull, Turbo and Toggle retain their original behavior. The assignment list expands to show all 24 inputs. QAM is visible and reserved. L3/R3 are stick clicks, and Touchpad is explicitly labeled **Touchpad click**. Gyro, stick axes, trackpad surface modes and trigger effects stay in their dedicated sections. Internal Puckbridge actions appear as badges; they are not fictitious physical DualSense controls. Grip Sense, USB-C, the magnetic charging connector and Mute are informational.

[Back / top screenshot](images/remap-desktop-back.png) · [Editor screenshot](images/remap-editor.png) · [Mobile screenshot](images/remap-mobile.png)

## Implementation and build

- `payload/web/index.html`: production HTML/CSS/JS; one logical key per input across both views; transient editor state is separate from profile state. No PS5 ABI or new output protocol IDs.
- `payload/web/controllers/`: four unchanged user-supplied transparent WebPs, preserved reference geometry and rights notice. Keep logical `iw`/`ih` and `vb` values: Steam front decodes at 1500×1091 but its geometry is 2566×1867; back is 1400×1007 with 2136×1536 geometry. SVG scales the image into that coordinate system.
- `payload/gen_html.py`: embeds the four WebPs and geometry alongside the existing icons in the generated header. The generator works from any current directory. Never edit `webui_html.h` manually.
- `payload/Makefile`: artwork, layout JSON and button icons are dependencies of `webui_html.h`. No CDN, remote font or framework is needed. The generated page is 672,184 UTF-8 bytes.

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

Require the returned object to contain `result: PASS` and `checks: 99`; some CLI versions return shell status 0 even for a displayed tool error. The scenario creates its own clean tab, resets only the loopback fixture profile via HTTP, and checks all physical input/output hotspots, output combinations, activators, QAM reservation, Cancel/Done, token-bearing save, reload, profile switching, Enter/Space/Escape/Tab, resizing at 768/390 pixels, actual touch emulation, and Games/Settings navigation. Four screenshots are written to `output/playwright/`. Machine-readable results from this execution are in [webui-remap-host-results.json](webui-remap-host-results.json).

The fixture binds only 127.0.0.1:8765, uses the deliberately public `preview-only` token and RAM-only profiles. It does not simulate PS5 device services or establish console HTTP, filesystem, browser, USB, controller, firmware or rendering compatibility. H-09 on the real portal remains required; verify both transports, profile persistence and live highlighting there. Asset publication was authorized by the owner; see [provenance](PROVENANCE.md) and the artwork notice.
