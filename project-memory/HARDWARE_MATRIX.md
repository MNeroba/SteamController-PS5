# Hardware and firmware compatibility matrix

No real PS5 hardware tests have been performed for this development tree. On 2026-10-11 the owner reported updating their console from firmware 12.00 to 13.60; this is the project target, not a compatibility result or a substitute for reading the console's installed version during a test. These rows are release gates, not claims. Record each transport, console model, exact installed firmware, controller firmware, tested payload commit, exact steps, result, and a corresponding `project-memory/TEST_LOG.md` entry. A source author's claim or a cross-build is not our hardware evidence.

| ID | Test | PS5 model | FW | Controller model / firmware | Transport | Payload commit / SDK | Result | Evidence |
|---|---|---|---|---|---|---|---|---|
| H-01 | Baseline console and stock DualSense before payload | NOT_TESTED | 13.60 (owner-reported; unverified) | DualSense / UNKNOWN | Native | NOT_RUN | NOT_RUN / BLOCKED_HARDWARE | User reports FW 13.60; no baseline runtime test performed |
| H-02 | Steam Controller input in system UI and a named game | NOT_TESTED | 13.60 (owner-reported; unverified) | Steam Controller 2026 / UNKNOWN | USB-C | NOT_RUN | NOT_RUN / BLOCKED_HARDWARE | User reports FW 13.60; no controller runtime test performed |
| H-03 | Steam Controller input through official Valve receiver | NOT_TESTED | 13.60 (owner-reported; unverified) | Steam Controller 2026 / UNKNOWN | Official Puck | NOT_RUN | NOT_RUN / BLOCKED_HARDWARE | User reports FW 13.60; no Puck runtime test performed |
| H-04 | Correct user assignment, no duplicate virtual pad, stock DualSense still works | NOT_TESTED | 13.60 (owner-reported; unverified) | Steam Controller 2026 / UNKNOWN; DualSense / UNKNOWN | USB-C and Puck | NOT_RUN | NOT_RUN / BLOCKED_HARDWARE | User reports FW 13.60; slot and DualSense coexistence untested |
| H-05 | Disconnect, reconnect, standby, stop/restart, and no stuck input | NOT_TESTED | 13.60 (owner-reported; unverified) | Steam Controller 2026 / UNKNOWN | USB-C and Puck | NOT_RUN | NOT_RUN / BLOCKED_HARDWARE | User reports FW 13.60; recovery runtime test not performed |
| H-06 | Motion direction and gyro aiming in a named game/profile | NOT_TESTED | 13.60 (owner-reported; unverified) | Steam Controller 2026 / UNKNOWN | USB-C or Puck | NOT_RUN | NOT_RUN / BLOCKED_HARDWARE | User reports FW 13.60; motion runtime test not performed |
| H-07 | Touchpad coordinates, contact, and clicks in a named game/screen | NOT_TESTED | 13.60 (owner-reported; unverified) | Steam Controller 2026 / UNKNOWN | USB-C or Puck | NOT_RUN | NOT_RUN / BLOCKED_HARDWARE | User reports FW 13.60; touch runtime test not performed |
| H-08 | Rumble/haptics and trigger translation in a named game | NOT_TESTED | 13.60 (owner-reported; unverified) | Steam Controller 2026 / UNKNOWN | USB-C or Puck | NOT_RUN | NOT_RUN / BLOCKED_HARDWARE | User reports FW 13.60; feedback runtime test not performed |
| H-09 | Unauthenticated remote portal calls cannot mutate state; authenticated portal behavior | NOT_TESTED | 13.60 (owner-reported; unverified) | Not applicable | Local network | NOT_RUN | NOT_RUN / BLOCKED_HARDWARE | User reports FW 13.60; no PS5/network integration test performed |
| H-10 | Native BLE capability gates B1–B6; stock DualSense unaffected | NOT_TESTED | 13.60 (owner-reported; unverified) | Steam Controller 2026 / UNKNOWN | Internal BLE | NOT_RUN | BLOCKED | No BLE/radio diagnostic performed; FW 13.60 is the unverified target |
| H-11 | Two physical Steam Controllers/Pucks do not interfere | NOT_TESTED | 13.60 (owner-reported; unverified) | Two Steam Controller 2026 / UNKNOWN | USB-C/Puck | NOT_RUN | NOT_RUN / BLOCKED_HARDWARE | Multi-controller SC2 support is not implemented or claimed |
| H-12 | Stop payload in-game; neutral input, cleanup, and no foreign hook overwrite | NOT_TESTED | 13.60 (owner-reported; unverified) | Steam Controller 2026 / UNKNOWN; DualSense / UNKNOWN | USB-C and Puck | NOT_RUN | NOT_RUN / BLOCKED_HARDWARE | User reports FW 13.60; shutdown runtime test not performed |

| H-13 | Optional Media settings tile, icon, loopback browser, pairing/navigation and own-title Delete/retry | NOT_TESTED | 13.60 (owner-reported; unverified) | Stock DualSense retained / UNKNOWN | Local browser / app registration | NOT_RUN / SDKv0.43 | NOT_RUN / BLOCKED_HARDWARE | Single ELF; software transaction/stubs/HTTP/browser evidence only |

| H-14 | Confirmed logout/relogin owned virtual-pad recovery, native DualSense coexistence, no USB restart | NOT_TESTED | 13.60 (owner-reported; unverified) | Steam Controller2026 / UNKNOWN | USB-C and official Puck | NOT_RUN / SDKv0.43 | NOT_RUN / BLOCKED_HARDWARE | Host observer/lifecycle/race evidence only |
| H-15 | Simultaneous input/feedback, late cancellation, watchdog, diagnostics export/notifications | NOT_TESTED | 13.60 (owner-reported; unverified) | Steam Controller2026 / UNKNOWN | USB-C / Puck / enrolled WebUI | NOT_RUN / SDKv0.43 | NOT_RUN / BLOCKED_HARDWARE | Actual kernel cleanup/browser behavior unknown |

## Required for a first release

The automatic release workflow requires `VERIFIED_HARDWARE` for H-01, H-02, H-03, H-04, H-05, H-09, and H-12, with a real payload commit, actual model/firmware values, and a test-log reference for each row. Host tests and the PS5 cross-build must also pass on the release tag. The check is intentionally fail-closed; editing a status without the required recorded evidence is not a substitute for performing the test.

For each required gate, the evidence cell must link to a matching heading such as `project-memory/TEST_LOG.md#h-01`. That heading must record the same PS5 model, exact firmware, controller model/firmware, transport, full payload commit SHA, and `Result: VERIFIED_HARDWARE`. The release checker rejects generic links or test-log mentions that do not match that row.

H-06 through H-08 are feature-specific: do not claim motion, touch, or feedback as hardware-verified unless their rows pass. H-10 remains a separate, explicitly approved research track. H-11 is outside the first release scope until multi-controller sessions are implemented.

### Image remapping UI follow-up (M1-214)

A local Microsoft Edge fixture passed 99 interaction checks and seven portable embedding tests. This is host evidence only. Actual authenticated PS5 portal rendering, live button highlighting and on-console profile persistence remain NOT_RUN / BLOCKED_HARDWARE on the owner-reported FW 12.00 target. Add the image remapping scenario in `docs/WEBUI_REMAP.md` to H-09 when hardware becomes available; do not change any existing NOT_RUN row to PASS based on these browser tests.

### Remap UX calibration follow-up (M1-216)

The local fixture now passes 114 browser checks, including calibrated DualSense L1/R1 upper shoulder and L2/R2 lower trigger regions, face-button centers, the expanded coarse-pointer hit margin, Shift/import consistency and direct output staging. Eight portable layout/generator tests pass. This updates only host evidence. Actual FW 12.00 portal rendering, live highlighting and profile persistence remain NOT_RUN / BLOCKED_HARDWARE; H-09 and every other existing hardware gate are unchanged.

M1-216 source publication on main: `408900592f50b1eb39b0003c357c95a137410f2f`. Host/Chromium/normal/ASan-UBSan Actions `38067268430` and PS5 SDK cross-build Action `38067268427` pass. This does not satisfy H-09 or any console/hardware row. FW 12.00 operation, live highlighting and persisted target profiles remain NOT_RUN / BLOCKED_HARDWARE.

M1-231 target update (2026-10-11): owner reports an upgrade to FW13.60. Current unrun rows now target13.60; historical12.00 host/UI follow-ups above are preserved. No row became VERIFIED_HARDWARE.


## External failure report — 2026-10-14 (M1-242)

Owner supplied a third-party screenshot reporting dev.2/FAT/FW5.50 via Payload Manager: detected USB-C/Puck with no input, device-limit message, token-storage error and intermittent native DualSense loss at user selection. Only the comment is available; no ELF SHA/log/error code/independent reproduction. This is an unverified external report, not VERIFIED_HARDWARE or a changed result for the intended13.60 rows. Three source defects were host-fixed after dev.3, but their causal relation to this report is unproven. Follow PUBLIC_RUNTIME_REVIEW and H04/H05/H09/H12/H14/H15 before any separately authorized retest.


## External runtime logs — 2026-10-15 (M1-244)

EXTERNAL_HARDWARE adverse report, not a gate pass. Supplied comment: Payload Manager v0.5.1; earlier FAT/FW5.50 report has not been remeasured for these logs. Dev.4 banner but no ELF SHA: Puck decoded input and gyro/battery, one unattributable803b0006 VDA result, WebUI token failure, no owned virtual-ready/VDI progress. Old unidentified log:11975 Adds/14 VDA identifiers; native UI disruption cause remains unproven. Full hashes and sanitized counts in EXTERNAL_RUNTIME_REPORT. No existing13.60 NOT_RUN row changes. Do not request another uncontrolled run; retain native DualSense and only collect already-existing metadata/logs.


M1-245 dev.5: protected-storage/logging changes have host/build evidence only. The actual tester storage errno and pending VDA receipt ABI remain unknown. No H-row changes, no compatibility assertion; candidate/source prepared without publication. H09 must inspect precise storage stage/error and verify privacy in raw/diagnostic logs; H14 must establish owned pending receipt/assignment before any release.


## M1-246 dev.5 controlled-test preparation — 2026-10-16

Owner authorizes experimental dev.5 publication for the external log author's test; no hardware run occurred here. P0 Add return/event/low32 VDI/MBus contract, user picker/native coexistence, P1 actual storage-stage/errno and USB shutdown/reconnect on USB-C/Puck remain NOT_RUN. Owner console13.60 and tester's earlier5.50 are reports, not measured compatibility. Follow docs/development/VDA_PENDING.md with exact selected source/ELF digest and measured console/loader/transport; no account IDs/serials. Host-injected ENOSPC28 and synthetic events are not console results.


## M1-249 — dev.6 release checkpoint (2026-11-01)

Published prerelease `dev/v0.1.0-dev.6`, source `4c8b89bc789bf1dfd064264ff432d3919b5a1899`, [workflow 41246199948](https://github.com/MNeroba/SteamController-PS5/actions/runs/41246199948). Repeated host/ASan/UBSan/Chromium/asset/SDK v0.43/checksum gates passed. Release contains one ELF (2,087,296 bytes; SHA-256 `ef52b279be433c1d445ec4a616e6294c647fb87555c3bca34fddddb68da7df9b`). This does **not** change any H-row: measured console firmware, VDA ownership/assignment, native DualSense coexistence, USB-C/Puck, portal and physical feature behavior remain NOT_RUN / BLOCKED_HARDWARE. Repository visibility is Public; no hardware compatibility is asserted.


## M1-250 — external PR8 report and dev.7 boundary (2026-11-01)

TheBrokenG0d reports one fat PS5, FW13.20, official Puck VID28de/PID1304, original PR8 source c3a7ddb, WebUI HTTP/auth/profile operations and system-menu input after profile-picker selection. This is EXTERNAL_HARDWARE, not independently reproduced here; native DualSense coexistence, games and motion/haptic fidelity were not systematically tested. It does not update the selected dev.7 H-rows or establish FW13.60 compatibility. Published experimental [v0.1.0-dev.7](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v0.1.0-dev.7) from `f4ec9ae9e7e287c957a5602493a5b7d46449aed3` by [DEV workflow 41300890370](https://github.com/MNeroba/SteamController-PS5/actions/runs/41300890370) on 2026-11-01 11:15:23 UTC (2026-11-01 13:15:23 EET). Seven assets contain exactly one ELF, `SteamController-PS5-v0.1.0-dev.7.elf`, 2,091,960 bytes, SHA-256 `9bc6e1672bc8d20c8e9fcff85359c976523bdf6296a64e1e92dae34e6d7edb73`. PR8 was merged with contributor commits preserved; both descriptor P1s and the post-drain observation-window issue are resolved in code/host regressions. Exact PR, merged-main and repeated DEV Host/ASan/UBSan/Chromium220/asset/SDKv0.43/LLVM18/checksum gates passed. This is VERIFIED_HOST_TEST and VERIFIED_PS5_BUILD only. Contributor FW13.20/Puck observations at original c3a7ddb are EXTERNAL_HARDWARE; this revised ELF, owner target FW13.60, native DualSense coexistence, gameplay, motion/haptics, console cleanup and tile launch remain physically unverified. No historical result/tag/release was changed.


## M1-252 — comparison evidence is not our hardware acceptance (2026-11-03)

No H-row changed. FGG source/README reportsFW11.60; AnyController README reportsFW13.60, while issue1 reports Insert EAGAIN onFW4.00; padbridge1.3.1 reportsSlimCFI-2114/FW13.60 rest/wake, xpad1.1.0 reportsSlim/FW13.40. These are upstream/external statements for different controllers, payloads and source revisions, not SteamController-PS5 tests. Our direct HID, rest/wake, new pressure/touch fields and haptic timing remain physically NOT_RUN. Issue2 still awaits an exact published dev.7 ELF test; no console was accessed. Pins/limits/required contracts are in UPSTREAM_RUNTIME_COMPARISON.


## M1-255 — dev.8 publication verified (2026-11-03)

Published experimental [v0.1.0-dev.8](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v0.1.0-dev.8) from `1cf624f2ccaf869eecd8c29dd29e834529f8a3d9` by [DEV workflow 41688254764](https://github.com/MNeroba/SteamController-PS5/actions/runs/41688254764) on **3 November 2026, 17:54:11 UTC**. All three release jobs completed successfully. Seven assets contain exactly one ELF, `SteamController-PS5-v0.1.0-dev.8.elf`, 2,121,120 bytes, SHA-256 `7a37126cd223259f06deb9b420c74fb7b9aaee7d353495e5eb81b69e93be0d2a`. The remote tag points to the selected source; every downloaded asset matched GitHub's digest, and the ELF source/version, sidecar checksum and metadata were independently checked.

Exact-source push Host41687756285, SDK41687756162, full-history secret scan41687756071 and CI smoke41687756186 passed. The DEV workflow repeated normal53 C/11 Python, ASan/UBSan, Chromium220, image/embedding/deterministic-asset, clean SDKv0.43/LLVM18, dependency, metadata/changelog/package and checksum gates. This is VERIFIED_HOST_TEST/VERIFIED_PS5_BUILD, not physical acceptance. Native DualSense coexistence/account assignment, FW13.60 acceptance, actual game motion/touch/quaternion, USB suspend/reconnect, active-game rest/wake and tile behavior remain NOT_RUN. Direct HID remains opt-in and waits for one confirmed foreground user plus a readable disconnected native pad; no native disconnect/unbind/Open, Shell patch or radio operation is enabled. Historical tags/releases and the stable hardware gate remain unchanged.

Downloaded release data and independently checked assets are saved in desktop output/dev8-published (release.json plus seven assets). The Linux release hash is distinct from the earlier local macOS candidate and is not substituted into historical test records. The source tree retains163 payload blobs; no history rewrite, source loss, native controller operation or console deployment. README, current build/loading/tile/security/feature/changelog/DEV docs are refreshed only after GitHub acknowledgement. Previous release receipts remain historical. Next: publish this documentation receipt, observe its Actions, and await controlled dev.8 tester evidence with exact release checksum/firmware/loader/transport. Issue2 remains open; do not claim hardware compatibility or post a tester message without an explicit communication request.


## M1-256 — dev.9 publication verified (2026-11-15)

Published experimental [v0.1.0-dev.9](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v0.1.0-dev.9) from `d6d51af1ce1d0cf14438fb73efae3cd2652b1d25` by [DEV workflow 43703160706](https://github.com/MNeroba/SteamController-PS5/actions/runs/43703160706) on 2026-11-15T10:18:02Z. Three release jobs succeeded. Seven assets contain one ELF, `SteamController-PS5-v0.1.0-dev.9.elf`, 2,068,880 bytes, SHA-256 `e4c1ad438f4f3861e846338528f4139f9e8e7dce3695839c96b31c53190cfc42`. The tag points to the selected source, every downloaded asset matched GitHub's digest, and the ELF embedded source, version, metadata and sidecar checksum were verified.

Owner selected no pairing code and PS5-only access, then requested automatic Media tile registration on first launch. WebUI binds IPv4 loopback only; other console processes can access it. Literal-loopback Host, same-host Origin, mandatory POST Origin, no-sniff and frame denial reduce browser-origin attacks. A joinable worker attempts registration only for an absent exact title path; it does not overwrite an existing or inaccessible path. An explicit local retry remains available. The tile does not start the payload. Physical PS5 FW13.60/browser/Origin, automatic installer privilege/visibility, registration recovery and shutdown remain NOT_RUN.

Exact-source push Host43702935960, SDK43702935974 and secret scan43702935930 succeeded. The release workflow repeated normal Host, ASan/UBSan, Chromium, deterministic assets, SDKv0.43/LLVM18, package/dependency and checksum gates. The earlier DEV workflow43702614078 was deliberately cancelled before creating a tag to correct misleading tile-status wording. Historical releases and the stable hardware gate remain unchanged.

Downloaded assets and GitHub metadata are retained in output/dev9-published. Release digest is from the published Linux artifact and differs from local SDK builds. Next: controlled physical test with exact release SHA, console firmware/loader, native DualSense and USB/Puck status; do not claim compatibility or post to others without an explicit request. Previous dev.8 and earlier tags/assets are unchanged.
