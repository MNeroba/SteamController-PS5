# Dev.8 implementation and validation

Published experimental [v0.1.0-dev.8](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v0.1.0-dev.8) from `1cf624f2ccaf869eecd8c29dd29e834529f8a3d9` by [DEV workflow 41688254764](https://github.com/MNeroba/SteamController-PS5/actions/runs/41688254764) on **3 November 2026, 17:54:11 UTC**. All three release jobs completed successfully. Seven assets contain exactly one ELF, `SteamController-PS5-v0.1.0-dev.8.elf`, 2,121,120 bytes, SHA-256 `7a37126cd223259f06deb9b420c74fb7b9aaee7d353495e5eb81b69e93be0d2a`. The remote tag points to the selected source; every downloaded asset matched GitHub's digest, and the ELF source/version, sidecar checksum and metadata were independently checked.

Exact-source push Host41687756285, SDK41687756162, full-history secret scan41687756071 and CI smoke41687756186 passed. The DEV workflow repeated normal53 C/11 Python, ASan/UBSan, Chromium220, image/embedding/deterministic-asset, clean SDKv0.43/LLVM18, dependency, metadata/changelog/package and checksum gates. This is VERIFIED_HOST_TEST/VERIFIED_PS5_BUILD, not physical acceptance. Native DualSense coexistence/account assignment, FW13.60 acceptance, actual game motion/touch/quaternion, USB suspend/reconnect, active-game rest/wake and tile behavior remain NOT_RUN. Direct HID remains opt-in and waits for one confirmed foreground user plus a readable disconnected native pad; no native disconnect/unbind/Open, Shell patch or radio operation is enabled. Historical tags/releases and the stable hardware gate remain unchanged.

Original implementation branch `codex/dev8-hid-rest`, based on main `b83a4b642c35dbf86d95331137c2fccfa69708c4`. Prepared and published through the existing experimental DEV gates. One ELF, SDK v0.43; no console deployment.

## Implemented

Direct backend is selected at startup by `ENABLE_DIRECT_HID=1` in the existing protected `/data/ghostpad/runtime-policy.ini`. Default remains libScePad. Unknown/duplicate settings fail closed; direct mode cannot be combined with hooks, handoff or bridge probing. No automatic fallback or handle mixing. No native DualSense open/disconnect/unbind or Shell memory patch.

The existing registry/lifecycle reserves creation and owns the direct positive receipt. Backend serializes FD/report operations, checks exact handles, zero-initializes the168-byte descriptor/report,24-byte Add/Insert headers and16-byte Delete header, and translates buttons including Create, sticks/triggers, two contacts, quaternion, gyro and acceleration. Invalid fields attempt neutral then stop for owned cleanup. Add failures/invalid output poison creation and retain the unresolved FD. Failed Delete retains ownership; successful removal closes FD once. [Exact ABI, source pins, licensing and evidence limits](RAW_HID_REPORT.md).

Registration requires a real confirmed foreground account and a successfully read disconnected native pad. Connected/unknown native state or missing user defers before reservation; it never disconnects a DualSense. Selection is rechecked before Add. A race after reservation fails conservatively and can inhibit creation for this process. This opt-in mode can therefore wait indefinitely if no readable disconnected native handle exists. Read-only API checks cannot promise atomic exclusion of OS account/controller changes.

Read-only power-event observation runs in the manager. Confirmed suspend/standby latches pause; readers/scanners cancel via existing cleanup, neutralize and release owned virtual devices. Manager joins old workers before two spaced working observations permit ordinary fresh-generation rediscovery. Errors/unknown states cannot resume a paused session. Failed join or owned deletion blocks unsafe replacement. No blanket device deletion, radio command, forced thread cancellation or payload restart. User pause/configuration persist; device-local gyro/mapping state follows existing disconnect reset.

The extended-report code blocker is resolved by pinned public source comparison, regression and build checks. This does not prove FW13.60 acceptance, game motion/touch behavior or rest/wake reliability. Upstream FW13.00 readback is an external claim, not our hardware result.

## Validation and next steps

Host tests cover the complete raw report, finite floats/contact limits/release, user authorization, open/Add/Insert/Delete failures, registry retention and power consensus. Full normal, sanitizer, browser, asset, metadata/changelog and SDK gates are recorded in project-memory/TEST_LOG.md. The final local source SHA and ELF digest accompany the local output package.

Physical NOT_RUN: controlled Add/neutral/readback, actual Create/touch/quaternion/gyro/acceleration in a suitable game, account changes, native DualSense before/during/after, owned deletion, pending/failed cleanup, USB-C/Puck disconnect/reconnect, user pause/shutdown, and rest/wake with active gameplay on measured firmware/model/loader. Synchronous ioctl/join timing has no measured bound. Do not run another controller payload concurrently or repeatedly reload after unresolved creation.

The owner-approved publication completed with the selected mainline SHA and successful repeated experimental-release gates. Future releases require separate approval. Stable hardware criteria remain unchanged. No firmware compatibility assertion is made.
