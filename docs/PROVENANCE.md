# Third-party code and asset provenance

This file is a **living register**, not evidence that code has already been copied. Codex must fill exact reviewed revision, origin path, license, copyright lines and modifications **before** importing or modifying third-party code.

## M1-252 — runtime comparison and Steam field preservation (2026-11-03)

Review-only GPL-3.0 references: FGGstore/FGG-XSense `bfc5f9d324e2ca8b0a90fc498957699caa9fe5b0`, BallerIsLeet/AnyController `2eec5ec209f7e20369144de37e2358d1b2af0241`, sub0x00001/padbridge `3d97019644c8c013c377fbee4ee418f28badc60a`, Alexandre78500/xpad-ps5 `7931988466356ad0a7cc9cabeb03538dbe325256`, arwebSE/LibrePadPS5 `aab1a4e95fe8602f26573f25560ca9794f5b6dea`, and SDK `a7685ce8e97ce1f8a4f38253a41d54241c9124f1`. Root LICENSE files inspected. No implementation, firmware, assets or dependency imported. The bounded haptic-service correction is independently authored in the existing Puckbridge-derived module. Direct HID ioctls and power-event calls are research references only, not added runtime behavior.

Before decoder extension: SDL `721cb0dd494503c3a7aad1ea6f3dae46cf4a6bfc`, `src/joystick/hidapi/steam/controller_structs.h:609–650` (Valve Corporation 2020, zlib notice in file) and `SDL_hidapi_steam_triton.c:86–97, 195–203, 246–270` (Sam Lantinga 1997–2026, zlib) supply field/bit-position facts for unsigned pad pressure and left-stick touch. Independently authored field extraction/tests preserve these values in the existing canonical report; no SDL implementation copied. No normalization, PS5 ABI, button assignment or sensor scaling is inferred from this extension. Conflicting Menu/View and gyro conventions remain explicit in the comparative report.

| Source | Candidate reuse | Reported license / validation | Source SHA | Destination | Copy vs reimplementation / modifications | Status |
|---|---|---|---|---|---|---|
| ThisIsAkill/PuckbridgePS5 | Selected foundation: SC2 protocol, motion, profiles, haptics, PS5 integration | README says GPL-3.0-or-later; root `LICENSE` is GNU GPL v3 text. README credits Ghostcontrol, SDL, Linux `hid-steam`, PoorDS4, Manba V2 and SDK. `payload/bridge/NOTICE.md` identifies PoorDS4 GPL-3.0-or-later revision `476b0b667ce82c6e51ffd9e96d7ef1f5bfc74d4b`. Audit every inherited file and retain notices. | `78460113ef7c48502c765a1f83ba34d53c977134` | Workspace root; source and Git metadata copied from pinned checkout | Whole source tree copied as foundation; portal HTTP/auth, Steam protocol/USB parsers, numeric/reentrant profile parser, per-session binding/flick/QAM/menu-edge state and full-neutral transitions, atomic pause/menu control, profile generations and async reset epochs, mapping/motion/feedback helpers, a portable generation-tagged slot registry, startup/rollback lifecycle coordinator, USB resource/session seam integrated into `gc_main.c`, owner-checked game-hook rollback/audio-heartbeat policy, mutex-protected hand-off and telemetry state, and synchronized game-hook/gyro state with host stress tests. No new third-party implementation code was imported for these modules. Original upstream README is preserved at `docs/archive/upstream/README.upstream.puckbridge.md`; project starter README at `docs/archive/PROJECT_STARTER_README.md`. | IMPORTED_SOURCE_BASE; active branch `fix/p0-safety`; upstream push disabled |
| diegobarbosaa/OmniPad-PS5 | Slot/USB lifecycle design reference only | Root GPLv3; no source imported. `make test` baseline incomplete. | `b7682efce18ee0a8d7c80bbc90afbce72151ef96` | N/A | Read-only comparison; do not copy its web, TCP or experimental Bluetooth code. | REFERENCE_ONLY |
| ddeverill/SteamlessController | protocol reference | MIT at the reviewed pin | `1de3f7a40ec9f73b0683898e60c7cc8a5695e0b0` | N/A | Reference only; Menu/View disagreement with SDL retained for hardware investigation | REFERENCE_ONLY; no implementation imported |
| libsdl-org/SDL | Triton decoder/struct reference | zlib notices inspected in driver and Valve structures; details above | `721cb0dd494503c3a7aad1ea6f3dae46cf4a6bfc` | `payload/steam_protocol.*`, source-derived tests | Independently extract raw pressure/touch facts; no copied implementation | REFERENCE_ONLY; no SDL implementation imported |
| safijari/openpuck | Puck descriptor/radio reference | verify file-level license | `2a1e54b...` snapshot | TBD | TBD | NOT_IMPORTED |
| reo101/SteamlessLink | BLE UUID/report mapping | VERIFY LICENSE before copying | `85d7c4f...` snapshot | TBD | TBD | NOT_IMPORTED |
| bluekitchen/btstack | BLE architecture only | Noncommercial restrictions; do not incorporate without rights compatibility | `e385539...` snapshot | N/A | documentation only | REFERENCE_ONLY |
| sub0x00001/octet (formerly padbridge) | virtual-pad remote-setting ABI facts only | GPL-3.0; no source copied | `f7383ff5e9e827a3c732ce21e90ce5ed27370efc` | `payload/sc2_haptics.c`, `payload/gc_main.c` | Independent decoder and polling loop written here; the buffer offsets (3/4 motors, 56 user flag) and the PS4-vs-PS5 haptics finding come from octet's `src/ps5_vpad.c` and CHANGELOG | REFERENCE_ONLY; no implementation imported |
| xdCloudy/PS5-13.60-Function-Stub-Library | firmware export NID facts only | No license file detected at the reviewed revision; only interface names/NIDs used as facts, no code copied | `0a2789c582b5bd973a68fb919a07779006ac3dd2` | `payload/settings_tile_ps5.c` NID constants | Independent implementation; NIDs also corroborated by `ps5-payload-dev/ftpsrv` linking the same library | REFERENCE_ONLY; no implementation imported |
| ps5-payload-dev/sdk | cross-compilation | Upstream README says GPL-3.0+ except files with their own notices, including BSD-licensed FreeBSD headers | v0.43 commit `d9c9519116944a7f1c22d262012c53b80b3520b7`; release archive SHA-256 `a9cc9929f21b2b2c5d5b309f3bab4997067c45281c0622cf4838b1aecba66fcb` | local ignored build cache; no SDK files imported into source | BUILD_TOOLCHAIN_ONLY; commit is unsigned per GitHub metadata; archive digest is pinned in the build workflow |

License provenance also applies to images, icons, web fonts, binary blobs, test captures and generated content. Do not distribute private Sony symbols/firmware or SDK files subject to confidential access terms. Separate code attribution from scientific citation.

## Recent source and documentation changes

Commit `9b22caa2d633af4bc66b0d5e3c93772ca5e80ce3` publishes the caller-owned legacy wired Steam input state, its host regression, and architecture/test/memory updates. The change is derived from this project's pinned Puckbridge foundation; no additional external source was imported.

The project README credits PuckbridgePS5 as the implementation foundation; Ghostcontrol and PoorDS4 through the retained upstream files and notices; and SteamlessController, SDL, Linux `hid-steam`, OpenPuck, SteamlessLink, and OmniPad as protocol/design references only. OmniPad and the protocol-reference repositories did not contribute copied code in this work. Akhil Moola and Xelu's Free Controller Prompts remain credited as noted by the upstream README; consult each asset's original notice before reuse. The new README, contribution guide, and PR template use links to official GitHub contribution documentation as process references and add no third-party source or assets.

The README, contribution guide, PR template, provenance update, and project-memory synchronization were published in commit `5656b9fc2b54d8f0313b25924a072315e08a799b`, parent `9b22caa2d633af4bc66b0d5e3c93772ca5e80ce3`, tree `2f83bc089fb62993a222fb26565ca33d87d1d258`. All seven uploaded blob SHAs matched local Git IDs. PR #1 remains open/draft/mergeable; the PR description was refreshed. At 17:41 UTC Host and PS5 cross-build Actions were queued, with no combined status. No CI pass is claimed.

## GitHub publication record

The user-specified private repository `MNeroba/SteamController-PS5` was empty at publication time; it is a derivative repository, not a GitHub fork. `main` was initialized with project README commit `61780b03bcef6859935cac1ef288cf3eef12d3ba`. The pinned Puckbridge-based project snapshot was published to `fix/p0-safety` as commit `4596c7fc4a9876e56cb8194961f15de0fd1e7066`, opening [draft PR #1](https://github.com/MNeroba/SteamController-PS5/pull/1). Follow-ups `7721f18cde11498c92fe9f6c91aa2a3f7dfbd269`, `04cc833e99c4e1e53eaaa93ae01658daa2cc69c0`, `cf225f48f0f38e86101f4f6d2993edbe47347c58`, `afeb258a6ddb5c607b78dc9a481edccf1b3ff254`, and `ade3a334d4af408102c7b149e01dd3cc087fbc1a` record memory, lifecycle, profile parsing, provenance, and the thread-safe combo parser. Memory/provenance synchronization followed in `2088cd639087b135940c7fc318223c6d3adeeca1`. Commit `65cd4f918f56fb2c547217e008f6b81c17853c04` publishes the SDK cross-build, state hardening, digest-pinned workflow, and project-memory records; commits `78bcf3d840dd53b7f5c2c92e0c0b40612d3a6d6c` and `014f418ade3c7d09655ed302b09cf9a4d0e508fc` synchronize that publication and CI queue records; `7ded26db0465992d9ce4c3f03163470727ec5ae6` publishes generation-tagged assignment leases and updated tests, followed by memory/provenance sync `0a3ef29edcce29ea78efd6f1e80c67a68b38c426`. Commit `8808f969cff62d0e129a1bbd3c391fed36e863c6` publishes the authoritative slot registry and deferred VDA-delete retry; commit `13c64c5439d82e81c31ec78dd030dbcf8d30bfe7` publishes the shared slot lifecycle coordinator and fault-injection tests; commit `28acb8a8b0d4c979ecfc23a81e36e53c3e236e59` publishes the USB session/resource owner, USB failure-path tests and corresponding docs/memory; memory synchronization followed in `096a4daf7d4fbcb997aa1f3314a44a46a0c693d4`. Commit `2e24ac196f52cb850ba3df6de328a5c0c198e452` publishes owner-checked game-hook rollback, legacy migration safety, the audio heartbeat lease, host tests and source/docs changes. At 15:45 UTC PR #1 remained open/draft/mergeable at that head; Host run `37803237618` and PS5 cross-build run `37803237655` were queued with empty combined status, so no CI pass is claimed. The remote commits do not retain Puckbridge Git ancestry; the source SHA, upstream license, notices, modifications, and comparison reference are preserved in this register and `docs/BASE_SELECTION_ADR.md`.

Commit `e01055e13601e9451b241b221f0cc19a1a8618e5` adds mutex-protected DualSense hand-off, coherent status telemetry, game-hook shared state and gyro state. The per-session input-state follow-up is published in `1682e7806ceff3dc5d0b43b8f1927ddf499a0818` and imports no third-party code. The local checkout is an upstream-ancestry snapshot with uncommitted edits; GitHub publication uses the connected GitHub app rather than the local Git author's identity.

Commit `1682e7806ceff3dc5d0b43b8f1927ddf499a0818` publishes per-reader Steam input state, flick-stick regression coverage, and the corresponding architecture/provenance/test-plan/project-memory records. The 16 local blob SHA values matched GitHub before a non-force fast-forward from `e01055e13601e9451b241b221f0cc19a1a8618e5`; no third-party source was imported. PR #1 is open, draft, and mergeable at that head. At 16:23 UTC the GitHub connector returned no PR-triggered workflow runs and the combined status was empty; this is not a CI pass.

## Source-base audit and unresolved rights

- The pinned Puckbridge `LICENSE` is the unmodified GNU GPL version 3 license text; upstream README and source SPDX headers describe the code as GPL-3.0-or-later. Preserve the file, source notices, and GPL corresponding-source obligations in any distributed build.
- Puckbridge's README describes the project as built on Ghostcontrol and credits StonedModder. The specific upstream ancestry is present in source and should remain attributed; no separate Ghostcontrol snapshot was imported for this audit.
- The vendored `payload/bridge/` files are attributed to PoorDS4 and its exact source revision in `payload/bridge/NOTICE.md`; retain and update that notice for any modifications.
- `payload/web/icons/LICENSE.txt` identifies button prompt icons as CC0 1.0. Upstream README credits the inline portal controller drawing to Akhil Moola and declares the project GPL-3.0-or-later, but gives the drawing no separate license. Treat the root license declaration as the project-wide grant and retain the credit; do not describe the drawing as CC0. The separate workspace handoff image `steam-controller.jpg` is not used by the payload and is excluded from Git.
- SDL and Linux `hid-steam` are protocol references credited by upstream; no source from either was separately copied by this task. Verify each source file's license before adapting code.
- OmniPad is not copied into this tree; its GPLv3 license does not resolve the separate security/quality review needed for any future selective port.
- No Sony proprietary SDK or firmware, BTstack code, or test capture was imported. The community SDK v0.43 is used only as a local ignored cross-build toolchain; its downloaded release asset matched the SHA-256 digest published in GitHub release metadata. The v0.43 source commit is unsigned per GitHub's verification metadata. The SDK is not copied into source or the project repository; review distribution obligations before publishing built payload binaries.


The portal Origin/profile-body policy and its host tests were published on 2026-10-08 in commit `7f1bb0ac71115c170190264802fab0a70321a8a7` (parent `1ac94ecdb903cae2d38c5b8ad8b8830eb6e94803`, tree `761768f9646c7be252e0e0e156b64d454e8b0a88`). Eight source/document blobs matched local Git object IDs before the expected-head branch update (`force=false`). This change imports no third-party implementation or assets. PR #1 remains open/draft/mergeable; its description was updated with 1030 local checks and ELF SHA-256 `1084457bba0e35a479b6bed5e0578027567decc199d0d3ae6724c301e74cd6a1`. At 19:35 UTC GitHub returned no Actions runs or combined statuses for the source commit; this is not a CI pass.

M1-194 reuses the existing `usb_session_ops_t` contract and resource tracker for SC2 endpoint scans. It adds no third-party implementation code, protocol capture, generated asset, or new external dependency; no new provenance entry is required. The derivative's draft PR #1 was later closed unmerged at the owner's request. Current publication policy is direct, non-force fast-forward commits to the project's own `main` until the first successful release; Puckbridge and OmniPad remain attribution/source references with their original pins and rights notices preserved.

## P1 controller artwork and hotspot geometry (2026-10-10)

The owner explicitly requested reuse of four transparent WebP images and the SVG hotspot geometry from the supplied `Steam Controller и DualSense — интерактивные кнопки.html`. Reference SHA-256: `82078e359df5258839f6cfdba2c352ac977adc81491c6daad205801c2da4f28c`. No upstream asset license or photographer attribution was included; do not describe these images as GPL or CC0. The owner confirmed they have the right to authorize repository publication on 2026-10-10. This confirmation does not supply a separate blanket asset license. Implementation code for the production adapter is authored in this project; reference demo code is not copied wholesale.

| Local file | Source | Changes / license evidence |
|---|---|---|
| `payload/web/controllers/steam-front.webp` | Owner-supplied HTML `DATA.steam.front.img` | Exact Base64 decoded bytes; SHA-256 `5b7354a941223f0b5293eeaa012c55897e9aebb7941cd6aa75eb31650dec8376`; repository publication authorized by owner (2026-10-10) |
| `payload/web/controllers/steam-back.webp` | `DATA.steam.back.img` | Exact bytes; SHA-256 `909e7e5c52916da5d60ec72cf9d3280e85824d08aecbc32b531165780bafb4a6`; repository publication authorized by owner (2026-10-10) |
| `payload/web/controllers/dual-front.webp` | `DATA.dual.front.img` | Exact bytes; SHA-256 `888a3b62a3c093f2ffa7b203986fe6b53a427628624942d782922ec010d0151a`; repository publication authorized by owner (2026-10-10) |
| `payload/web/controllers/dual-back.webp` | `DATA.dual.back.img` | Exact bytes; SHA-256 `bc7af34550853da4f303f7143b2d0beebbd12679bf7258dfe853cc2a49d74e5c`; repository publication authorized by owner (2026-10-10) |
| `payload/web/controllers/layouts.json` | Same HTML `DATA` | Preserved circle/rect/ellipse/polygon geometry, rotations, `iw`, `ih`, `vb`; removed data URIs, named local assets; repository publication authorized by owner (2026-10-10) |

Remap audit follow-up (ADR-050/M1-218, 2026-10-10): local authored UI/geometry/regressions retain the existing artwork rights notice. `sc2_binding.c` and `sc2_profile.c` retain their Puckbridge-derived GPL provenance at the foundation pin above; changes suppress Shift-trigger Full pull and canonicalize conflicting Turbo/Toggle. The authored host-only `tests/webui_profile_fixture.c` reuses the production parser/formatter through a bounded stdin adapter and stubs unused platform lookup functions; no external implementation or proprietary SDK code was imported.

Consolidation provenance review (2026-10-10): corrected a one-character PoorDS4 SHA typo in the foundation row to 476b0b667ce82c6e51ffd9e96d7ef1f5bfc74d4b, matching the preserved bridge NOTICE and GitHub commit readback. No source/license notice changed. All seven relocated upstream/starter files retain exact baseline blob hashes; new build/release tools are authored here, with no new imported implementation.

## 2026-10-11 local hardening

Pairing/HTTP admission, session-owned USB frames/watchdog, bounded metadata parsing and hook installation transaction are authored in this repository under GPL-3.0-or-later. Hook adapter changes derive from the already attributed Puckbridge/PoorDS4 pinned files above; no additional upstream code, assets or dependencies are imported. All inherited notices remain.

## dev.3 settings tile (2026-10-11, before implementation)

| Local paths | Pinned reference | License / reuse / changes |
|---|---|---|
| `payload/settings_tile_ps5.c`, `payload/settings-tile/param.json` | SDK `d9c9519116944a7f1c22d262012c53b80b3520b7`, `samples/install_app/payload.c:20–34`; owendswang/ps5-web-file-manager `3659cbbc40022e32229f9919f045018a988f2ef5`, `assets/param.json`, `src/app_installer.c` | SDK example GPL-3.0-or-later, John Törnblom (2025); Web File Manager root GPL-3.0. API declarations and Media/deeplink schema reference only; independently authored installer with exclusive, owned files, explicit authenticated opt-in, exact-title registration and no global-install fallback. No upstream implementation/assets copied. |
| `payload/settings_tile_ps5.c` termination declaration | itsPLK/ps5-payload-manager `e4e72dc9d831dd09ee8fabc222ad1e7f5139050e`, `src/app_installer.c:34` | API signature reference only; no implementation copied. SDK v0.43 exports the termination symbol. |
| `payload/settings-tile/source.jpg`, `icon0.png` | Owner attachment, requested for dev.3 publication on 2026-10-11 | Original JPEG SHA256 `fec522611c33371f416e6df59dbfb9656937470c0382f74e1151999db15de69c`; PNG `b5bbe4c1a321435f1b51ca64ebda9234e58f4a64d00972e05563c4950781bb55`. PNG is a deterministic stored 512×512 build asset converted with macOS sips; preserve original photo. Owner authorization does not establish a separate GPL/CC0 license or photographer attribution. |

M1-233 lazy installer loading: SDK v0.43 pin `d9c9519116944a7f1c22d262012c53b80b3520b7`, `samples/hello_dlfcn/main.c:53–70` (John Törnblom, GPL-3.0-or-later) demonstrates dlopen/dlsym/dlclose. API usage reference only. The authored adapter resolves the three installer functions only after authenticated tile confirmation, with checked cleanup and no new mandatory AppInstUtil ELF dependency; no sample implementation copied.

## LibrePadPS5 v1.1.2 engineering review (2026-10-12, before changes)

| Planned local paths | Pinned reference | Rights and adaptation |
|---|---|---|
| user-session/slot lifecycle, USB completion, diagnostics, release metadata | arwebSE/LibrePadPS5 `7466317b4b356249e07770f3914c995e1a4d633a` (v1.1.2; root GPL-3.0), `src/ps5/virtual_controller.c:375–435`, `src/input/usb_device.c:184–252`, `src/dashboard/http_server.c:315–365`, `config/payload/LibrePadPS5.elf.json` | General engineering/API/schema reference only; independently authored extensions of our existing registry/session/HTTP stack under GPL-3.0-or-later. No upstream implementation copied. No XInput/8BitDo commands, storage migration, guessed native handles, MBus binding or BT code imported. Upstream recovery has host tests but no physical confirmation; hardware reports for unrelated devices do not establish Steam compatibility. |

FreeBSD 12b769ed36a115da3b5013c92cdf294a3a5de98d sys/dev/usb/usb_generic.c / usb_dev.c, BSD-2-Clause-FreeBSD: read-only engineering evidence for USBFS cancellation/close; zero copied implementation. Prospero behavior remains unverified.

## Final runtime comparison — M1-242 (2026-10-14)

Reviewed current LibrePadPS5 `28a3bfefe19cbcdbacb94a9ba911cb01ebc6e0fc`, `src/ps5/virtual_controller.c`, `src/input/usb_device.c`, `src/app/main.c`, `tests/test_usb_transport.c`, root GPL3 and v1.1.3 release notes. Rechecked unchanged Puckbridge `78460113ef7c48502c765a1f83ba34d53c977134` gc_main and OmniPad `b7682efce18ee0a8d7c80bbc90afbce72151ef96` ps5_vpad/usb_hotplug/GPL3. SDK `d9c9519116944a7f1c22d262012c53b80b3520b7` crt/kernel.c is API/error/layout evidence only. No implementation copied. The local registry latch/reservation, live-fd USB identity check and bounded status arithmetic extend existing project modules independently. No MBus binding, klog ownership, storage migration, DS3/XInput code, kernel offset or new asset imported. Exact relevant lines/limits: PUBLIC_RUNTIME_REVIEW.md.


## dev.5 review before changes — M1-245 (2026-10-15)

Rechecked unchanged source pins: LibrePad28a3bfefe19cbcdbacb94a9ba911cb01ebc6e0fc virtual_controller.c:519–590 and storage v1.1.3 warning, OmniPadb7682efce18ee0a8d7c80bbc90afbce72151ef96 ps5_vpad.c and FIRMWARE_1360_GUIDE, Puckbridge78460113ef7c48502c765a1f83ba34d53c977134 gc_main. General status/API reference only; no implementation copied. Their klog/MBus/Shell fallback does not prove exclusive ownership and is not enabled here. Independent extensions to the existing private-file and WebUI sign-in modules, raw-log diagnostics and VDA result classification remain GPL-3.0-or-later. No upstream assets, new kernel offsets, storage migration or confidential SDK imported. Public OpenOrbis Pad.h marks VDA unreversed; export stubs alone do not establish a safe receipt ABI.

## M1-246 — pending VDA observer and session authentication (2026-10-16)

LibrePadPS5 `28a3bfefe19cbcdbacb94a9ba911cb01ebc6e0fc`, GPL-3.0, `src/ps5/virtual_controller.c:165–282, 519–590`: reference for bounded observation of typed virtual-device events. Independently authored read-only `vda_pending` parser/observer; no implementation copied, no IDs promoted to handles, no MBus/Shell fallback or native-controller operation. Unique temporal correlation still does not prove ownership. Session-only WebUI credentials extend our existing CSPRNG/bearer/pairing implementation; no upstream storage migration, new protocol, dependency or artwork.

M1-246 owner clarification authorizes the analogous event-correlation approach for this stage, with a stronger ownership mechanism deferred. The resulting registration extends the existing slot registry/lifecycle; the LibrePad MBus prototype `int32_t(uint64_t,int32_t)` and read-only native-pad checks at :878–891 are API references. No code copied. Direct local bind is limited to an explicit Steam/PS press, one current foreground user and confirmed disconnected native state. Unknown/connected native state falls back to the console's user picker; no disconnect/unbind, handle scan, Shell injection or privilege modification is imported. Temporal attribution across unrelated payloads remains an accepted experimental limitation, not an established AddDevice ABI.


## PR #8 / dev.7 — 2026-11-01, before adaptation

| Local paths | Source / exact revision | License / reuse |
|---|---|---|
| payload/webui.c worker stack, payload/vda_pending.c backlog bound, initial payload/ps5_at_compat.h and payload/Makefile | TheBrokenG0d/SteamController-PS5 PR8 head c3a7ddb4f721b512e294dea23ed67e4d5cd1ef1a (three contributor commits) | Contributor's GPL-3.0-or-later changes; preserve author commits and SPDX. Retain 1 MiB client stacks and 4096-read nonblocking drain; replace ambiguous fd/inode inference after reproducing two P1 defects. Contributor-reported FW13.20/Puck results are EXTERNAL_HARDWARE only. |
| payload/ps5_at_compat.c, tests/test_ps5_at_compat.c and ABI tests | ps5-payload-dev/sdk d9c9519116944a7f1c22d262012c53b80b3520b7 libc/syscalls.c:1841–1950 and include/freebsd/sys/errno.h:163–165; freebsd/freebsd-src d106002967eff712894ab8b5337bbbff106ce5e0 lib/libc/amd64/SYS.h | SDK GPL-3.0-or-later (John Törnblom), FreeBSD file BSD-4-Clause notices retained upstream. Public ABI/numeric definitions and carry-flag error convention used as reference only; independently authored typed wrappers and tests, no implementation copied. No new kernel offsets, memory patches, filesystem migration or native-controller operations. |


## M1-251 — CI Actions updates (2026-11-01)

Dependabot PR3–7 update references to existing official actions/upload-artifact, checkout, setup-node, download-artifact and github-script dependencies. Exact reviewed upstream revisions and compatibility decisions are in development/CI_ACTIONS.md. No upstream implementation is copied into the payload; the small non-publishing artifact/API smoke workflow is authored here. Existing licenses and release notices remain unchanged.

## Dev.8 direct-HID and power adapter (before implementation)

FGG-XSense bfc5f9d324e2ca8b0a90fc498957699caa9fe5b0 src/vpad.c and AnyController2eec5ec209f7e20369144de37e2358d1b2af0241 include/vpad.h (GPL3) supply request/descriptor/report layout facts. padbridge3d97019644c8c013c377fbee4ee418f28badc60a src/ps5_power.c (GPL3) supplies named event-flag observation/signature/state values. Independently authored adapters/tests; no copied implementation. Raw touch/linear/angular-motion layout remains unproven: direct mode must explicitly refuse unsupported fields, remain opt-in, and never change the existing default backend or silently drop those capabilities. No physical invocation authorized by host preparation.

## Dev.8 extended report investigation — before adaptation (2026-11-03)

Reviewed Darkatek7/ProsperoSunshine3574d60989efe7d0b2410e2ff137cd1c4d490c60 src/platform/ps5/pad.c, pad.h, docs/re/pad-input.md and STATUS.md; GPL-3.0-only in source. Reviewed phantomptr/ps5uploadefcd6fd4a2ce416b226c9a28ef16464ad7fbcb73 payload/src/vpad.c (file GPL-3.0-only), include/vpad.h and tests/vpad_selftest.c. Both share the ProsperoSunshine/ps5-mcp lineage, not independent ABI derivations. Interface/field facts only; independently authored translation/tests, no implementation copied, no Sony binary/header/disassembly fetched or redistributed. Prospero's published live FW13.00 readback/units/Create observations remain UPSTREAM_CLAIM, not our firmware validation. MIT ps5-mcp84c0f7b316772b13cd62e0121c154afc3ed2482c docs/vpad-abi.md only has the old partial layout; jefflouisma/SunshinePS55bb544695d88342d8dfde92e823865ffc66635dd remains explicitly provisional and cannot establish full fields. Acceleration/velocity native-structure order cross-checked against public OpenOrbis _types/pad.h; pin0a1aaf9dd4a92695538bdeb09fb056d06dd11725, include/orbis/_types/pad.h; PS4 corroboration only. Full field table, source links and limitations: development/RAW_HID_REPORT.md.

## M1-260 — Media-tile API facts and virtual-pad behavior review (2026-11-19)

Reviewed for API/behavior facts only; no implementation code was copied.

- `ps5-payload-dev/ftpsrv` `92072f2694fdd804718cebfe0cc49fe7fe4aab82`, GPL-3.0: `install-ps5.c` initializes AppInstUtil, stages `/user/app/<id>/sce_sys/{param.json,icon0.png}`, and resolves `sceAppInstUtilAppInstallTitleDir` by NID `Wudg3Xe3heE` via `kernel_dynlib_handle/resolve`.
- `ps5-payload-dev/websrv` `7748df9d2a43822a5ebfda149f43d4f2696d640b`, GPL-3.0: same NID resolution and staged-title layout for a Media tile.
- `ItsBlurf/BFpilot` `0eeb6f4` (SDK 0.45 preparation), GPL-3.0: post-success marker `.bfpilot-registered`, exact embedded-asset comparison before skipping AppInst, bounded initialization, and TitleDir NID resolution. Only the marker/skip and NID facts informed our independently written code; the global `AppInstallAll` fallback was deliberately not adopted.
- `BallerIsLeet/AnyController` `2eec5ec`, GPL-3.0: direct `/dev/hid` virtual-pad Add/Insert/Delete behavior, transient insert-error tolerance and short USB-loss grace. No code copied; our direct-HID adapter and lifecycle remain independently implemented.
- `itsPLK/ps5-payload-manager` and `owendswang/ps5-web-file-manager`: `param.json` category `65536`, `deeplinkUri` and 512×512 `icon0.png` schema confirmation.

These are engineering references; their licenses and notices remain their own.

## Controller outline gallery — 2026-11-28

The SVG curves in `payload/web/controllers/layouts.json` were drawn against the four owner-authorized WebPs listed above. The images in `docs/images/remap/` are new Chromium screenshots of the generated WebUI served by the loopback host fixture. The same artwork rights and attribution apply.

The README hero at `docs/images/remap/hero.png` composes the same Steam and DualSense front photos into an original HTML/CSS layout rendered in Chromium. It uses their existing logical crops and artwork notice.

## Phone pairing QR encoder

| Local path | Exact upstream | Scope |
|---|---|---|
| `payload/web/vendor/qrcodegen.js` | [Project Nayuki QR Code generator v1.8.0](https://github.com/nayuki/QR-Code-generator/releases/tag/v1.8.0), commit `720f62bddb7226106071d4728c292cb1df519ceb`, asset `qrcodegen-v1.8.0-es6.js`, MIT | Upstream asset SHA-256 `6a1116192ed1dd67fa1bf31e77f5817103d71c23bbac24c382e698b7668bdd01`. Removed trailing spaces only. Vendored SHA-256 `5d65f10cbda9bc5f61df20e4dfb716ef3203b19f7daedb6cc7c0ae51193f033f`. Embedded for offline QR encoding of the LAN URL. |

The README's `docs/images/remap/webui-main.png` and `docs/images/remap/phone-setup.png` are Chromium captures of the v1.6.3 host preview. The address and eight-digit code are demonstration values. The QR was decoded independently and matches the displayed example.
