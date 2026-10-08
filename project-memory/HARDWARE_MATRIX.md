# Hardware and firmware compatibility matrix

No real PS5 hardware tests were performed during preparation of this pack. A source author's claimed test must not be recorded as ours.

| Test | PS5 model | FW | Steam Controller model / firmware | Transport | SDK/payload commit | Result | Evidence |
|---|---|---|---|---|---|---|---|
| Baseline USB-C | NOT_TESTED | 13.60 target | SC 2026 / UNKNOWN | USB-C | SDK v0.43 cross-build M1-15; runtime not tested | NOT_RUN / BLOCKED_HARDWARE | No physical PS5 or Steam Controller; ELF compile does not verify device support |
| Official Puck | NOT_TESTED | 13.60 target | SC 2026 / UNKNOWN | Valve Puck | SDK v0.43 cross-build M1-15; runtime not tested | NOT_RUN / BLOCKED_HARDWARE | No physical PS5 or Puck; ELF compile does not verify device support |
| Native BT coexistence | NOT_TESTED | 13.60 target | SC 2026 / UNKNOWN | Internal BLE | NOT_BUILT | BLOCKED | B0–B6 not passed |
| Edge emulation | NOT_TESTED | 13.60 target | SC 2026 / UNKNOWN | Backend | NOT_BUILT | NOT_RUN | Research only |

Add rows per actual tested firmware, PS5 hardware revision, title ID, expected behavior, pass/fail and evidence link. Do not collapse Fat/Slim/Pro, cable/Puck, game and system UI into one blanket compatibility claim.
