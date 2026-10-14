# Runtime safety review before public source access

2026-10-14 · M1-242 · SDK v0.43 · intended FW13.60, hardware unverified

## Outcome

Three reproducible source defects were fixed and regression-tested. This review supports publishing the **experimental source**. It does not certify that an ELF cannot cause Kernel Panic: virtual-pad ABI, kernel/loader behavior and USB cancellation still need a physical console.

Reviewed baseline: main `1da839a6fa6059716181c9a3009adbef5a5b67e6`, tree `9480e78c8a3ad833d7dd41b165a304b74ec64647`,286 files. Its Host38287206901 and PS5 cross-build38287206908 passed. All production bytes matched the local workspace; only the two append-only logs had known local publication receipts. Final fix commit and remote checks are recorded in project memory after publication.

**Published dev.3 and earlier ELFs do not contain these new fixes.** No old tag or release asset is replaced. A new experimental binary needs its own tested source and explicit owner publication approval. See [unreleased changes](../../CHANGELOG.md).

## Confirmed findings and fixes

| Finding | Before | Smallest safe correction | Evidence |
| --- | --- | --- | --- |
| P0-VDA-UNATTRIBUTED-03 | A nonpositive Add receipt cleared the creation reservation, allowing another attempt every second. Raw SDK-call serialization also left a gap before its result reached the registry. A backend that creates a device without returning our accepted handle could exhaust the system device pool. | Reserve creation across the whole registry until its result is recorded. Any nonpositive/aliased receipt latches off future Add calls for this registry lifetime, including hotplug/new generations. Never scan/delete guessed handles. Positive owned receipts retain normal behavior and cleanup. | Expected failing host tests for32 repeated attempts,0/-1/pending-style results and two different pending slots; regressions pass after fix. No PS5 Add ABI or device-pool exhaustion was reproduced. |
| P1-USB-REOPEN-03 | The manager identified a USB path, but the worker reopened it without rereading its identity. Rapid replacement could authorize detach using the old device's VID/PID and a different device's configuration. | Reread identity from the live opened descriptor before configuration/driver claim. Require the same exact Steam VID/PID/transport; missing/changed identity closes without detach. Standard device descriptors require their complete18-byte format and actual response length. | Expected failing adapter source-order guard,77 portable USB checks including truncated/wrong descriptors, changed transport, native DualSense ID and unknown IDs. Actual hotplug/ioctl behavior remains untested. |
| P1-USB-STATUS-03 | Encoding an invalid large OUT status as `-1000-status` overflowed signed int. | Bound the conversion; out-of-range status becomes a protocol error. Preserve useful ordinary status values and session cleanup. | UBSan reproduced signed overflow with INT_MAX before the fix;69 OUT checks and full ASan/UBSan pass afterwards. No kernel crash claim. |

## Analogous implementations actually reviewed

| Project and exact pin | Useful evidence | Decision |
| --- | --- | --- |
| [PuckbridgePS5](https://github.com/ThisIsAkill/PuckbridgePS5/tree/78460113ef7c48502c765a1f83ba34d53c977134), unchanged HEAD | `payload/gc_main.c:457` Add, `:411–413` probe detach, `:1141–1142` credentials, `:1188` anonymous deletion show why our existing hardening matters. Steam protocol foundation remains valuable. | Preserve audited Steam implementation. Do not reintroduce blanket detach/deletion or unchecked credential writes. |
| [OmniPad](https://github.com/diegobarbosaa/OmniPad-PS5/tree/b7682efce18ee0a8d7c80bbc90afbce72151ef96), unchanged HEAD | `src/ps5_vpad.c:108–124` sweeps handles obtained from logged-in users; later paths use GetHandle/remote system-process fallbacks. `src/usb_hotplug.c:620` does not check pthread_create. | No safer replacement for our ownership/shutdown model found. Do not copy native-handle lookup, Shell injection or broad cleanup. |
| [LibrePadPS5 current source](https://github.com/arwebSE/LibrePadPS5/tree/28a3bfefe19cbcdbacb94a9ba911cb01ebc6e0fc) | `src/input/usb_device.c:242–318` preserves IN while OUT completes and stops reusing cancelled OUT; our existing resource/session modules already cover this. `src/ps5/virtual_controller.c:519–590` also handles output-param/klog identity and MBus binding. Its current main differs from the previously reviewedv1.1.2 by storage/DS3/diagnostic work; virtual-controller source is unchanged in that comparison. | Keep our completion retirement and explicit ownership. Do not infer klog events establish exclusive ownership or import MBus/user binding. No DS3/XInput/storage migration copied. |
| [LibrePad v1.1.3 release evidence](https://github.com/arwebSE/LibrePadPS5/releases/tag/v1.1.3) | Maintainer reports storage/startup/diagnostic tests on FW12.60, and explicitly says physical input was not retested and logout recovery/FW13.60 need confirmation. | Useful warning about storage assumptions, not proof of our Steam transport or FW13.60 runtime safety. |
| [SDK v0.43](https://github.com/ps5-payload-dev/sdk/blob/d9c9519116944a7f1c22d262012c53b80b3520b7/crt/kernel.c) | `:257–274` reads runtime version from libc metadata; `:301–568` selects kernel layouts, including5.50 and13.60, and returns ENOSYS for unknown layouts. `:1149–1256` credential accessors check address/read/write failures. | Keep the pinned community SDK and checked credential guard. SDK layout declarations are not a tested VDA contract. Do not copy another project's arbitrary firmware range or introduce new kernel offsets. |

All reference implementations were read only; no new third-party code was copied. GPL/SDK/upstream notices are retained in [PROVENANCE](../PROVENANCE.md).

## Reviewed default execution boundaries

- Credentials: snapshot before elevation, rollback armed before setters, partial failures restored/retried, startup stops on error. Workers are joined before persistent credential restoration. Kernel setter/runtime behavior remains unverified.
- Virtual pad: positive exact-call ownership only; no0–63 sweep or guessed/native-handle deletion. Failed owned deletion prevents replacement. Confirmed user-list consensus and one active Steam session remain intact.
- USB: exact Steam IDs, parsed HID interfaces/endpoints, reopened-descriptor identity, no arbitrary detach; active/completed/retired buffer ownership and valid-input watchdog retained.
- System processes/radio: game hooks, bridge probe and DualSense handoff stay disabled by default. No new Shell patches, MBus calls, HCI command or firmware write. Inherited optional research code is not hardware-certified.
- Settings tile: authenticated explicit confirmation, exact owned title, lazy optional API loading; no automatic installation or new required ELF dependency. Actual registration/termination ABI still needs H-13.
- WebUI/storage: existing bearer/Origin/LAN checks, bounded clients/reports, private token files and safe failure behavior preserved. Storage failure disables WebUI rather than allowing anonymous controls. No permission relaxation or migration introduced to guess around a report.
- Shutdown: owned neutralization, reader-owned cancellation/cleanup, checked joins and conservative retained VDA/hook state. Repeated failed deletion may keep shutdown pending; a bound on userspace retries cannot bound a hung kernel call.

## External FW5.50 report supplied by the owner

A screenshot reports dev.2 on PS5 FAT/FW5.50 with Payload Manager: Puck/USB-C detection but no input; a maximum-Bluetooth-devices error; WebUI token-storage error; sometimes the native DualSense stops responding at the user picker, requiring a console restart. The owner confirms **only the comment is available**: no log, ELF checksum/source SHA, exact error code or independent reproduction.

Record this as an **unverified external failure report**, not VERIFIED_HARDWARE and not a measured pass/fail on our intended13.60 target. Screenshot SHA-256: `f0b722dd67f931928213664bbbad18b7dbcfe5dd87ac63192019e7c0c83fa070`; screenshot/user identity is not committed. Unattributable Add retries are a plausible contributor to device-pool exhaustion (**HYPOTHESIS**), not an established explanation of that console's failure. The token-storage cause is unresolved. SDK5.50 layout support does not establish SteamController-PS5 compatibility there.

Until actual logs/approved console access exist: do not ask the tester to repeat the failing old build, relax token permissions, guess handles, enable handoff/hooks or install the optional tile to diagnose it. H-04/H-05/H-09/H-12/H-14/H-15 must establish user-picker/native DualSense coexistence, device counts, storage and cleanup on measured hardware.

## Validation

`VERIFIED_HOST_TEST`: normal44 C/Python suites, full ASan/UBSan, targeted expected-failure regressions then fixes, Chromium220. `VERIFIED_PS5_BUILD`: clean SDKv0.43/LLVM18 build and optional-dependency check. Link/changelog/package/diff checks and exact source/remote CI receipts are recorded in M1-242 TEST_LOG. Initial sandbox HTTP bind failure was environmental; authorized loopback rerun passed. The initial dependency-check command could not find Linux-named llvm-readelf-18; rerun with the installed Homebrew LLVM18 readelf passed. These negatives remain in memory.

`NOT_RUN`: every physical-console scenario. No Kernel Panic immunity, firmware compatibility or successful native DualSense coexistence is claimed. Public source access can proceed with these limits; a hardware-verified stable release still cannot.


## Source publication receipt

Fix source [c204524cbd14cc232c11289677e7bb6607e59a0e](https://github.com/MNeroba/SteamController-PS5/commit/c204524cbd14cc232c11289677e7bb6607e59a0e) published on main:287 blobs, all286 previous files and45 production C units retained. Fresh SHA-verified export repeats normal/sanitizer/SDK/dependency checks successfully; exact source identity embedded. Clang static analysis of credential guard, slot registry, USB resources/session and Steam USB returned no diagnostics. Chromium220 tests apply to unchanged browser/UI bytes. Host38581146662 and SDK38581146350 are queued at2026-10-14 14:32 UTC; this is not a remote CI pass. The experimental release workflow was not run.
