# Hardware and firmware compatibility matrix

No real PS5 hardware tests have been performed for this development tree. The owner reports that their console runs firmware 12.00 and will not be updated; this is the project target, not a compatibility result or a substitute for reading the console's installed version during a test. These rows are release gates, not claims. Record each transport, console model, exact installed firmware, controller firmware, tested payload commit, exact steps, result, and a corresponding `project-memory/TEST_LOG.md` entry. A source author's claim or a cross-build is not our hardware evidence.

| ID | Test | PS5 model | FW | Controller model / firmware | Transport | Payload commit / SDK | Result | Evidence |
|---|---|---|---|---|---|---|---|---|
| H-01 | Baseline console and stock DualSense before payload | NOT_TESTED | 12.00 (owner-reported; unverified) | DualSense / UNKNOWN | Native | NOT_RUN | NOT_RUN / BLOCKED_HARDWARE | User reports FW 12.00; no baseline runtime test performed |
| H-02 | Steam Controller input in system UI and a named game | NOT_TESTED | 12.00 (owner-reported; unverified) | Steam Controller 2026 / UNKNOWN | USB-C | NOT_RUN | NOT_RUN / BLOCKED_HARDWARE | User reports FW 12.00; no controller runtime test performed |
| H-03 | Steam Controller input through official Valve receiver | NOT_TESTED | 12.00 (owner-reported; unverified) | Steam Controller 2026 / UNKNOWN | Official Puck | NOT_RUN | NOT_RUN / BLOCKED_HARDWARE | User reports FW 12.00; no Puck runtime test performed |
| H-04 | Correct user assignment, no duplicate virtual pad, stock DualSense still works | NOT_TESTED | 12.00 (owner-reported; unverified) | Steam Controller 2026 / UNKNOWN; DualSense / UNKNOWN | USB-C and Puck | NOT_RUN | NOT_RUN / BLOCKED_HARDWARE | User reports FW 12.00; slot and DualSense coexistence untested |
| H-05 | Disconnect, reconnect, standby, stop/restart, and no stuck input | NOT_TESTED | 12.00 (owner-reported; unverified) | Steam Controller 2026 / UNKNOWN | USB-C and Puck | NOT_RUN | NOT_RUN / BLOCKED_HARDWARE | User reports FW 12.00; recovery runtime test not performed |
| H-06 | Motion direction and gyro aiming in a named game/profile | NOT_TESTED | 12.00 (owner-reported; unverified) | Steam Controller 2026 / UNKNOWN | USB-C or Puck | NOT_RUN | NOT_RUN / BLOCKED_HARDWARE | User reports FW 12.00; motion runtime test not performed |
| H-07 | Touchpad coordinates, contact, and clicks in a named game/screen | NOT_TESTED | 12.00 (owner-reported; unverified) | Steam Controller 2026 / UNKNOWN | USB-C or Puck | NOT_RUN | NOT_RUN / BLOCKED_HARDWARE | User reports FW 12.00; touch runtime test not performed |
| H-08 | Rumble/haptics and trigger translation in a named game | NOT_TESTED | 12.00 (owner-reported; unverified) | Steam Controller 2026 / UNKNOWN | USB-C or Puck | NOT_RUN | NOT_RUN / BLOCKED_HARDWARE | User reports FW 12.00; feedback runtime test not performed |
| H-09 | Unauthenticated remote portal calls cannot mutate state; authenticated portal behavior | NOT_TESTED | 12.00 (owner-reported; unverified) | Not applicable | Local network | NOT_RUN | NOT_RUN / BLOCKED_HARDWARE | User reports FW 12.00; no PS5/network integration test performed |
| H-10 | Native BLE capability gates B1–B6; stock DualSense unaffected | NOT_TESTED | 12.00 (owner-reported; unverified) | Steam Controller 2026 / UNKNOWN | Internal BLE | NOT_RUN | BLOCKED | No BLE/radio diagnostic performed; FW 12.00 is the unverified target |
| H-11 | Two physical Steam Controllers/Pucks do not interfere | NOT_TESTED | 12.00 (owner-reported; unverified) | Two Steam Controller 2026 / UNKNOWN | USB-C/Puck | NOT_RUN | NOT_RUN / BLOCKED_HARDWARE | Multi-controller SC2 support is not implemented or claimed |
| H-12 | Stop payload in-game; neutral input, cleanup, and no foreign hook overwrite | NOT_TESTED | 12.00 (owner-reported; unverified) | Steam Controller 2026 / UNKNOWN; DualSense / UNKNOWN | USB-C and Puck | NOT_RUN | NOT_RUN / BLOCKED_HARDWARE | User reports FW 12.00; shutdown runtime test not performed |

## Required for a first release

The automatic release workflow requires `VERIFIED_HARDWARE` for H-01, H-02, H-03, H-04, H-05, H-09, and H-12, with a real payload commit, actual model/firmware values, and a test-log reference for each row. Host tests and the PS5 cross-build must also pass on the release tag. The check is intentionally fail-closed; editing a status without the required recorded evidence is not a substitute for performing the test.

For each required gate, the evidence cell must link to a matching heading such as `project-memory/TEST_LOG.md#h-01`. That heading must record the same PS5 model, exact firmware, controller model/firmware, transport, full payload commit SHA, and `Result: VERIFIED_HARDWARE`. The release checker rejects generic links or test-log mentions that do not match that row.

H-06 through H-08 are feature-specific: do not claim motion, touch, or feedback as hardware-verified unless their rows pass. H-10 remains a separate, explicitly approved research track. H-11 is outside the first release scope until multi-controller sessions are implemented.

### Image remapping UI follow-up (M1-214)

A local Microsoft Edge fixture passed 99 interaction checks and seven portable embedding tests. This is host evidence only. Actual authenticated PS5 portal rendering, live button highlighting and on-console profile persistence remain NOT_RUN / BLOCKED_HARDWARE on the owner-reported FW 12.00 target. Add the image remapping scenario in `docs/WEBUI_REMAP.md` to H-09 when hardware becomes available; do not change any existing NOT_RUN row to PASS based on these browser tests.

### Remap UX calibration follow-up (M1-216)

The local fixture now passes 114 browser checks, including calibrated DualSense L1/R1 upper shoulder and L2/R2 lower trigger regions, face-button centers, the expanded coarse-pointer hit margin, Shift/import consistency and direct output staging. Eight portable layout/generator tests pass. This updates only host evidence. Actual FW 12.00 portal rendering, live highlighting and profile persistence remain NOT_RUN / BLOCKED_HARDWARE; H-09 and every other existing hardware gate are unchanged.
