# Third-party code and asset provenance

This file is a **living register**, not evidence that code has already been copied. Codex must fill exact reviewed revision, origin path, license, copyright lines and modifications **before** importing or modifying third-party code.

| Source | Candidate reuse | Reported license / validation | Source SHA | Destination | Copy vs reimplementation / modifications | Status |
|---|---|---|---|---|---|---|
| ThisIsAkill/PuckbridgePS5 | Selected foundation: SC2 protocol, motion, profiles, haptics, PS5 integration | README says GPL-3.0-or-later; root `LICENSE` is GNU GPL v3 text. README credits Ghostcontrol, SDL, Linux `hid-steam`, PoorDS4, Manba V2 and SDK. `payload/bridge/NOTICE.md` identifies PoorDS4 GPL-3.0-or-later revision `476b0b667ce82c6e51ffd9e96d7ef1f5bfc74d4d`. Audit every inherited file and retain notices. | `78460113ef7c48502c765a1f83ba34d53c977134` | Workspace root; source and Git metadata copied from pinned checkout | Whole source tree copied as foundation; portal HTTP/auth, Steam protocol/USB parsers, numeric/reentrant profile parser, mapping/motion/feedback helpers, and project docs modified. Original upstream README is preserved at `README.upstream.puckbridge.md`; project starter README at `docs/PROJECT_STARTER_README.md`. | IMPORTED_SOURCE_BASE; active branch `fix/p0-safety`; upstream push disabled |
| diegobarbosaa/OmniPad-PS5 | Slot/USB lifecycle design reference only | Root GPLv3; no source imported. `make test` baseline incomplete. | `b7682efce18ee0a8d7c80bbc90afbce72151ef96` | N/A | Read-only comparison; do not copy its web, TCP or experimental Bluetooth code. | REFERENCE_ONLY |
| ddeverill/SteamlessController | protocol reference | VERIFY LICENSE before copying | TBD | TBD | reference first | NOT_IMPORTED |
| libsdl-org/SDL | Triton decoder/struct reference | file-specific notice and project license to verify | TBD | TBD | reference first | NOT_IMPORTED |
| safijari/openpuck | Puck descriptor/radio reference | verify file-level license | `2a1e54b...` snapshot | TBD | TBD | NOT_IMPORTED |
| reo101/SteamlessLink | BLE UUID/report mapping | VERIFY LICENSE before copying | `85d7c4f...` snapshot | TBD | TBD | NOT_IMPORTED |
| bluekitchen/btstack | BLE architecture only | Noncommercial restrictions; do not incorporate without rights compatibility | `e385539...` snapshot | N/A | documentation only | REFERENCE_ONLY |
| ps5-payload-dev/sdk | cross-compilation | GPL-3.0+ with certain BSD headers; verify packaging | `f7fd02...` snapshot | build-only | no SDK bundle by default | REFERENCE_ONLY |

License provenance also applies to images, icons, web fonts, binary blobs, test captures and generated content. Do not distribute private Sony symbols/firmware or SDK files subject to confidential access terms. Separate code attribution from scientific citation.

## GitHub publication record

The user-specified private repository `MNeroba/SteamController-PS5` was empty at publication time; it is a derivative repository, not a GitHub fork. `main` was initialized with project README commit `61780b03bcef6859935cac1ef288cf3eef12d3ba`. The pinned Puckbridge-based project snapshot was published to `fix/p0-safety` as commit `4596c7fc4a9876e56cb8194961f15de0fd1e7066`, opening [draft PR #1](https://github.com/MNeroba/SteamController-PS5/pull/1). Follow-ups `7721f18cde11498c92fe9f6c91aa2a3f7dfbd269`, `04cc833e99c4e1e53eaaa93ae01658daa2cc69c0`, `cf225f48f0f38e86101f4f6d2993edbe47347c58`, and `afeb258a6ddb5c607b78dc9a481edccf1b3ff254` record memory, lifecycle, profile-parser and provenance work. The remote commits do not retain Puckbridge Git ancestry; the source SHA, upstream license, notices, modifications, and comparison reference are preserved in this register and `docs/BASE_SELECTION_ADR.md`.

## Source-base audit and unresolved rights

- The pinned Puckbridge `LICENSE` is the unmodified GNU GPL version 3 license text; upstream README and source SPDX headers describe the code as GPL-3.0-or-later. Preserve the file, source notices, and GPL corresponding-source obligations in any distributed build.
- Puckbridge's README describes the project as built on Ghostcontrol and credits StonedModder. The specific upstream ancestry is present in source and should remain attributed; no separate Ghostcontrol snapshot was imported for this audit.
- The vendored `payload/bridge/` files are attributed to PoorDS4 and its exact source revision in `payload/bridge/NOTICE.md`; retain and update that notice for any modifications.
- `payload/web/icons/LICENSE.txt` identifies button prompt icons as CC0 1.0. Upstream README credits the inline portal controller drawing to Akhil Moola and declares the project GPL-3.0-or-later, but gives the drawing no separate license. Treat the root license declaration as the project-wide grant and retain the credit; do not describe the drawing as CC0. The separate workspace handoff image `steam-controller.jpg` is not used by the payload and is excluded from Git.
- SDL and Linux `hid-steam` are protocol references credited by upstream; no source from either was separately copied by this task. Verify each source file's license before adapting code.
- OmniPad is not copied into this tree; its GPLv3 license does not resolve the separate security/quality review needed for any future selective port.
- No Sony proprietary SDK or firmware, BTstack code, or test capture was imported. `PS5_PAYLOAD_SDK` remains unset, so build packaging and SDK licensing have not been verified.
