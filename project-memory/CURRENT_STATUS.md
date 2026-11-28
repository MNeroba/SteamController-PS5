# Current — 1.6.2 Experimental (2026-11-28)

## v1.6.3 phone profile editing — 2026-11-28

Owner selected the header popup and explicitly approved the next experimental publication. The default console listener stays on loopback. **On phone** starts a temporary same-subnet listener, displays an offline QR and address, and generates an eight-digit one-use code. Phone access edits existing profiles and can be revoked on the console. It uses local HTTP. PS5/phone runtime and FW 13.60 remain NOT_RUN. Source and release gates are recorded in TEST_LOG and SESSION_LOG.

## WebUI controller contours — 2026-11-28

The source diagrams now follow the supplied controller photos. DualSense uses curved rear bumper and trigger outlines and realigned front controls. Steam uses the same SVG renderer for its cross-shaped D-pad, trackpads, rear controls and other calibrated regions. The README gallery shows all four views with the button outlines visible. Local Chromium passed 246 checks, including clicks, edge probes and touch at 390 and 768 px. Normal host, ASan/UBSan and the SDK v0.43 cross-build passed. Hardware remains `NOT_RUN`. The published v1.6.2 experimental ELF predates the update.

To verify the update again, generate the page with `python3 payload/gen_html.py` and run `bash scripts/run_webui_browser.sh` with Playwright CLI 0.1.22. Use `gh run list --branch main` for the exact-source GitHub checks.

## M1-266 — v1.6.2-experimental final WebUI audit fixes (2026-11-28)

Published **v1.6.2-experimental** from [`9afa9f6fa05dffe3f2308ee3b94f670fcc40e394`](https://github.com/MNeroba/SteamController-PS5/tree/9afa9f6fa05dffe3f2308ee3b94f670fcc40e394) by [DEV workflow 46081216835](https://github.com/MNeroba/SteamController-PS5/actions/runs/46081216835) at 2026-11-27T23:15:29Z. Seven assets, one ELF `SteamController-PS5-v1.6.2-experimental.elf`, 2,127,824 bytes, SHA-256 `da9663f1c39e7d40e4932b892ea0dff381111f527f02a99e90cf7f77dd80ba86`; downloaded sidecar matched.

Completes the remaining audit list: `/api/haptic` returns 503 when enqueue is rejected; `sc2_haptic_pulse/tick/rumble_for` return status; `/api/log` applies the diagnostic report's line-level redaction; HTTP supports `HEAD`/`OPTIONS` with `Allow` and returns `Allow` on 405; read-only routes bypass the global dispatch lock; `recover-default` distinguishes already-valid (409) and failure (500), delete returns 409/404/500; profile Rename/Duplicate actions use the existing validated update API. Browser regression now covers pause, hooks, hand-off, bridge, log and profile-lock mutations.

Local normal/sanitizer suites pass; push gates Host 46080984292, PS5 46080984319, CI Actions smoke 46080984293 and secret scan 46080984349 succeeded; DEV workflow repeated Chromium/SDK/package/checksum gates. Physical PS5 and all hardware rows remain `NOT_RUN`.


## M1-265 — v1.6.1-experimental runtime toggles and mutation tests (2026-11-28)

Published **v1.6.1-experimental** from [`c02977510aabc57fd081c0b28a0950447350b4c2`](https://github.com/MNeroba/SteamController-PS5/tree/c02977510aabc57fd081c0b28a0950447350b4c2) by [DEV workflow 46078999218](https://github.com/MNeroba/SteamController-PS5/actions/runs/46078999218) at 2026-11-27T22:53:06Z. Seven assets, one ELF `SteamController-PS5-v1.6.1-experimental.elf`, 2,124,944 bytes, SHA-256 `33834d70f26d275a92c9a0e89eeb307559dd907a1209bb6e436d58a6069b52ab`; the downloaded sidecar matched.

Adds `runtime_feature_set_handoff/bridge` (gc_main) that persist the complete runtime policy through the private writer, start/stop the hand-off observer/service or the read-only bridge worker, revert on failure and reject `ENABLE_DIRECT_HID` conflicts. `/api/dualsense?handoff=0|1` and `/api/bridge?enable=0|1` expose them; `/api/dualsense` and `/api/bridge` now report policy and worker state separately (`allowed`, `enabled`, `active`). The WebUI adds confirmation-gated hand-off and bridge switches and disables dependent controls when the policy is off. Bridge enable without a foreground user persists the policy and asks for a payload restart.

`tests/test_webui_server.c` now exercises profile write/delete, pause, override, log, hand-off policy, bridge enable/request and the five-key gamehook query through the production HTTP dispatch. Local normal and sanitizer suites passed; push gates Host 46078747565, PS5 46078747671, CI Actions smoke 46078747567 and secret scan 46078747554 succeeded; DEV workflow repeated Chromium/SDK/package/checksum gates. Physical PS5 and all hardware rows remain `NOT_RUN`.


## M1-264 — v1.6.0-experimental WebUI quality release (2026-11-28)

Published **v1.6.0-experimental** from [`731138df0a94a9e434d422a165d346d4b9f7684e`](https://github.com/MNeroba/SteamController-PS5/tree/731138df0a94a9e434d422a165d346d4b9f7684e) by [DEV workflow 46076758638](https://github.com/MNeroba/SteamController-PS5/actions/runs/46076758638) at 2026-11-27T22:30:24Z. Release [dev/v1.6.0-experimental](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v1.6.0-experimental) has seven assets and one ELF, `SteamController-PS5-v1.6.0-experimental.elf`, 2,119,208 bytes, SHA-256 `588e415204cf023b7bfccf504f2857a3e1738cbddb2cee6c9f411b9c5a64a267`. The downloaded sidecar matched the ELF.

Scope was the 2026-11-28 four-way WebUI audit. Fixes: load-gated inert save bar; save/readback and Undo/Discard state; visible errors and control rollback for profile lock, pause, log, hand-off, hooks and recommended settings; 1.2 s offline backoff, hidden-page pause and immediate resume; sequential icon hydration; remap-sheet focus retention, background `inert` and draft-aware unload guard; honest hand-off/bridge policy states; five-key `/api/gamehooks`; 2048-byte finite status JSON; `O_NOFOLLOW` log/icon reads; tab `aria-controls`/`tabpanel` and arrow navigation; game-list error/retry; settings-only import; malformed-hash tolerance.

Verification: local normal and sanitizer suites pass; exact-source push gates Host 46076463781, PS5 46076463853, secret scan 46076463783 and CI Actions smoke 46076463820 succeeded; the DEV workflow repeated normal/ASan/UBSan/Chromium/SDK v0.43/deterministic-asset/package/checksum gates. Physical PS5, FW13.60, native DualSense coexistence and all hardware rows remain `NOT_RUN`; no radio, firmware or virtual-pad ABI change. The stable `v*` gate is unchanged.


## M1-263 — passive BLE / identity / platform research (2026-11-28)

Five desk-research workstreams produced [docs/research/BLE_IDENTITY_PLATFORM_2026-11-28.md](../docs/research/BLE_IDENTITY_PLATFORM_2026-11-28.md). No console, radio, pairing, USB or firmware experiment was run; every hardware row remains `NOT_RUN`.

Findings:
- **Native passive BLE is not available on the public surface.** `libSceBluetoothHid` is HID-device plumbing (17 exports, all keyed by an existing handle; `RegisterDevice(ushort,ushort)` cannot carry an address or keys). SDK v0.43 ships no Bluetooth stub/header. The only public native attempt (OmniPad `bt_hci_usb.c`) seizes `/dev/ugen` with raw HCI and is not built; that is active takeover. A retail PS5 rejected a BLE-only keyboard (`CE-109524-1`) while Classic HID worked. B0's public portion is complete-negative; B1–B6 remain unrun.
- **The controller's BLE link is usable by non-Steam centrals.** ESP32-S3/NimBLE (`inonomori/steamcontroller32` @ `2b20075`, MIT), Android and macOS implementations connect with Just Works bonding, Valve service `100F6C32…`, `7C`→0x47 / `7A`→0x45 input and `B5`→0x80 rumble. The blocker is PS5-side BLE, not the controller.
- **Identity is handle identity.** `scePadVirtualDeviceAddDevice` takes a 32-byte opaque block with no name/MAC/serial; today a slot is keyed by the ephemeral `/dev/ugen*` path, so a transport change deletes and re-adds the pad. Recommended design D: keep the handle for the logical controller, inject neutral frames while unbound, cap the hold, force release on logout/power/stop.
- **Advanced features are partly reachable.** Motion and two touch contacts are already in the ABI but unpublished; haptics work with hook/haptic-audio limits; trigger effects are haptic only; lightbar, player LEDs, speaker audio and physical trigger resistance have no virtual-pad setter.
- **Repo gap:** `usb_hid_thread()` is a 530-line function mixing discovery/stream/mapping/injection, and frames are decoded twice (`gc_main.c:1505`, `controller_sc2.c:375`). The first host-only PR should split decode from decision before any transport abstraction.

Plan: Phase 1 host-only (single decode, `transport_ops_t`, identity keep-alive, fake bridge), Phase 2 external ESP32-S3 BLE-to-USB bridge, Phase 3 separately approved console validation of the passive inventory and identity handover, Phase 4 active HCI **not requested** (no positive evidence). Motion/touch publication is a parallel platform quick win.

Next safe step: implement Phase 1 R1.1 (`sc2_handle_report` split) with host tests, or wait for owner direction on the bridge. Physical gates unchanged.

Research documentation is pushed as `74c7124`; exact-source gates Host 46072301189, PS5 46072300763 and secret scan 46072300781 succeeded. This is documentation evidence only — no code or hardware gate changed.

## M1-262 — v1.5.1 / v1.5.2 experimental published (2026-11-27)

Current published source is **v1.5.2-experimental**: `ef6e2ae0af87b393a090ec477ddc6668cddbeeb7`, [release](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v1.5.2-experimental), [DEV workflow 46069613276](https://github.com/MNeroba/SteamController-PS5/actions/runs/46069613276), published 2026-11-27T21:23:12Z. Seven assets contain one ELF, `SteamController-PS5-v1.5.2-experimental.elf`, 2,099,840 bytes, SHA-256 `b48aa18ad5bed2e267b177319adfb8439e6efb9b6986f05bc8d00c5922bde744`; sidecar, embedded source/version and metadata verified. Push gates Host 46069328761, PS5 46069328676 and secret scan 46069328700 succeeded.

Two feedback changes since 1.5.0:
- **v1.5.1** (`d4b3a36`): reads classic DualShock-4-style rumble from the owned virtual pad with `scePadVirtualDeviceGetRemoteSetting` (large byte 3, small byte 4) every 16 ms while hooks are inactive, and forwards it to `sc2_haptics`. PS4 games fill these fields; PS5-native haptics are converted to DualSense haptic audio and leave them at zero.
- **v1.5.2** (`ef6e2ae`): adds the **Enable game hooks (Experimental)** checkbox. It persists the complete runtime policy through the private writer, starts/stops the worker immediately, rejects `ENABLE_DIRECT_HID` combinations, requires UI confirmation, reverts the policy on worker-start failure, and reports a retained rollback. ADR-070 records the owner authorization and the unchanged trusted-loopback limitation. A worker stop flag makes runtime disable joinable.

Documentation now states the PS4-vs-PS5 feedback split precisely instead of the earlier "write-only" simplification. Issue #10 is being updated with the correction and the checkbox instructions. Hardware remains NOT_RUN for both builds; the stable gate is unchanged. Next: controlled FW13.60 testing of `v1.5.2` and a report on whether the game-hook checkbox attaches for GTA VI.

## M1-261 — v1.5.0-experimental published (2026-11-27)

Current published source is **v1.5.0-experimental**: `65ba503f28a6dcb0a34bd544b2581a51473728ea`, [release](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v1.5.0-experimental), [DEV workflow 46064596992](https://github.com/MNeroba/SteamController-PS5/actions/runs/46064596992), published 2026-11-27T20:37:45Z. Seven assets contain one ELF, `SteamController-PS5-v1.5.0-experimental.elf`, 2,096,232 bytes, SHA-256 `49bbd70d53c7734a85136cf6b8827a97469e22d5a4ded1b4c08f874521eb832b`; the downloaded sidecar, embedded source/version and Payload Manager metadata were independently verified. History was squashed to one root commit after local backups (`backup/pre-1.5-state`, `backup/pre-squash-v1.5`, `backup/pre-1.5-20261127-221021`) and pushed with force-with-lease; push gates Host 46064305214, PS5 46064305101 and secret scan 46064305150 succeeded. The previous 1.0.0 release/tag remains published and unchanged.

The build applies every host-fixable finding from the [2026-11-27 audit](../docs/development/MECHANISM_AUDIT_2026-11-27.md): NID-first Media-tile resolution with a retry-safe installer and atomic `.partial` first install; pre-Add VDA refusal no longer latches creation; retired haptic OUT restarts the session, the test method resets per session and dropped pulses are counted; stale-temp profile recovery with directory fsync; bounded shutdown (30 s retry then forced credential/PID rollback) and bounded WebUI/hand-off joins; private status log rotated after the PID lock; held long-press no longer re-fires at a profile boundary and latched toggles keep their mask; sanitized private bridge-probe report; fail-fast UBSan; SHA-pinned Actions and an ancestor-of-main tag guard.

Issue #10 was verified against its attached FW13.60 GTA VI log: hooks were disabled (`hooks=0`, zero hook lines), the virtual pad was active (five sessions, 1,580 VDI progress lines), manual interrupt-OUT haptics returned 0 and only the `SET_REPORT` fallback failed EIO. Missing in-game vibration is the documented opt-in feedback limitation, not a transport defect; the same log reproduced AUD-01. The diagnostic report is now rendered inline because the PS5 browser could not download it. The reporter received the log analysis, 1.5 checksum and post-upgrade instructions in [issue #10 comment 7227721020](https://github.com/MNeroba/SteamController-PS5/issues/10#issuecomment-7227721020); the publication receipt is in docs commit `c923653`.

Hardware remains NOT_RUN: PS5/FW13.60 execution, native DualSense coexistence, game feedback, motion/touch quality, storage and shutdown. The stable `v*` gate is unchanged. Next: controlled physical testing of this exact ELF and checksum; any further published code change requires a new `v1.5.0-experimental.N` version.

## M1-259 — consolidated 1.0.0 Experimental publication (2026-11-18)

Owner consolidated every change into one 1.0.0 Experimental release: the full dev.9 feature set, the neutral-gravity release-report fix, loopback `Host`-port validation against the real listener port, bounded/validated game-icon reads, `404` for unknown routes, removal of the unused auth/peer modules and dead `/pair` routes from the shipped payload, and corrected authentication documentation.

M1-260 pre-publication review (2026-11-19): compared the Media-tile mechanism with working `ps5-payload-dev/ftpsrv` (`92072f2`), `ps5-payload-dev/websrv` (`7748df9`), BFpilot (`0eeb6f4`), Payload Manager and ps5-web-file-manager. The tile now resolves `sceAppInstUtilAppInstallTitleDir` by NID `Wudg3Xe3heE` with a symbol-name fallback, skips automatic registration only after a post-success marker plus exact embedded assets, and retries staged-but-unregistered directories. Direct-HID insertion now retries transient `EAGAIN`/`EBUSY`/`EINTR`, and a three-second link-loss grace holds the owned virtual pad before detaching. No implementation was copied; source facts and pins are recorded in PROVENANCE.

Published 2026-11-18T22:27:39Z from `f5208adb9608a589708f3f9d2f987ff0857e07bc` by [DEV workflow 44410095266](https://github.com/MNeroba/SteamController-PS5/actions/runs/44410095266). Seven assets contain one ELF, `SteamController-PS5-v1.0.0-experimental.elf`, 2,069,488 bytes, SHA-256 `3f2ed1f28122391bfda161db9d2221a645daa216993ae6c4edf1a44e69660c4c`; the downloaded sidecar, embedded source/version and metadata were verified. The workflow repeats normal host, ASan/UBSan, Chromium, deterministic-asset, SDK v0.43 and checksum gates. Host and sanitizer suites pass locally, `git diff --check` passes, and the full-history gitleaks scan reports zero findings.

The earlier 1.0.0 artifact (`3bad173`, SHA-256 `79ab4f…`) was archived as `v1.0.0-experimental-pre-tile` and replaced by this rebuilt source. The superseded dev.1–dev.9, v1.0.0-experimental.1 and v1.0.0-experimental.2 releases/tags were archived under `~/GitHub/_archive/SteamController-PS5-releases-2026-11-18` and deleted. Only `dev/v1.0.0-experimental` remains. Physical PS5/FW13.60, browser/Origin, native DualSense coexistence, motion/haptics and tile registration remain NOT_RUN; the stable hardware gate is unchanged.

Next: controlled physical PS5 validation of the published 1.0.0 Experimental ELF. Any further published code change requires a new experimental version.

# Historical — v1.0.0-experimental.2 published (2026-11-18)

## M1-258 — v1.0.0-experimental.2 hardening publication (2026-11-18)

Published experimental [v1.0.0-experimental.2](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v1.0.0-experimental.2) from `3e75e03a3fcee80549838a26ce3675c9b2b8dff6` by [DEV workflow 44397526550](https://github.com/MNeroba/SteamController-PS5/actions/runs/44397526550) on 2026-11-18T21:14:36Z. Three release jobs succeeded. Seven assets contain one ELF, `SteamController-PS5-v1.0.0-experimental.2.elf`, 2,067,376 bytes, SHA-256 `523c6ac784250abf36c464b7edffeb5c13eed685249a40ad034ec9d8cc493326`. The tag points to the selected source; the downloaded ELF, sidecar, embedded source/version and metadata were independently verified.

Owner asked to fix the confidently diagnosed problems. Changes: `payload/webui.c` now ties an explicit loopback `Host` port to the port returned by `getsockname` after bind, rejects empty/unreadable/oversized icons instead of serving a truncated PNG, and returns `404` for unknown routes. The unused `webui_auth.c` and `webui_peer_policy.c` modules and the permanently forbidden `/pair` and `/api/pair-refresh` routes were removed from the shipped payload; both modules and their tests remain host-test-only. Active architecture, security, test-plan, VDA/tester and audit documentation was corrected so it no longer describes bearer or pairing authentication as active.

Verification: normal `make -C tests clean test` and `make -C tests sanitize` pass (53 C targets plus Python contracts), `check_changelog`/package/policy/metadata regressions pass, `git diff --check` passes and a local full-history gitleaks scan reports zero findings. Exact-source push gates Host44397042663, SDK44397042952 and secret scan44397042725 succeeded.

The superseded v1.0.0-experimental.1 release and tag were archived under `~/GitHub/_archive/SteamController-PS5-releases-2026-11-18` and deleted, leaving only `dev/v1.0.0-experimental.2`. Physical PS5/FW13.60, browser/Origin, native DualSense coexistence, motion/haptics and tile registration remain NOT_RUN; the stable hardware gate is unchanged.

Next: controlled physical PS5 validation of the published `.2` ELF; any further code change requires `v1.0.0-experimental.3`.

# Historical — v1.0.0-experimental.1 published (2026-11-18)

## M1-257 — v1.0.0-experimental.1 publication verified (2026-11-18)

Published experimental [v1.0.0-experimental.1](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v1.0.0-experimental.1) from `884d8e9f77b32771f84adc00075670664b92bd1f` by [DEV workflow 44389398512](https://github.com/MNeroba/SteamController-PS5/actions/runs/44389398512) on 2026-11-18T20:20:18Z. Three release jobs succeeded. Seven assets contain one ELF, `SteamController-PS5-v1.0.0-experimental.1.elf`, 2,068,904 bytes, SHA-256 `2780abe6ad56d67216c2ed6b8e9ca824a7409a57fb21331d437ee7b7eea6c5d2`. The tag points to the selected source; the downloaded ELF, sidecar checksum, embedded source/version and metadata were independently verified.

Owner-approved scope: a defect pass limited to 95%-confidence changes, consolidation of all prior commits into one publication commit, deletion of the older release artifacts, and a v1.00 experimental publication. The desktop checkout was aligned from an older dev.5-era snapshot to published `main` `08b1b97e0dce49bc51af9c66927a0fd810c25fa4` after a full refs bundle and worktree archive. The only code defect fixed is the pause / DualSense-takeover release report in `payload/gc_main.c`, which injected zero acceleration instead of the documented neutral gravity `(0,1,0)`; the staging pad was aligned at the same time. Version, changelog, Payload Manager metadata, packaging tooling, workflow version gate and its regression now accept the `v1.0.0-experimental.N` series; the stable `v*` hardware gate is unchanged.

Verification: normal `make -C tests clean test` and `make -C tests sanitize` pass (53 C targets plus Python contracts). Three commit-keyed gitleaks false positives (a synthetic test token and two prose matches) were removed at the source; a local full-history scan then reported zero findings. Squashed-source push gates Host44388876807, SDK44388876981 and secret scan44388876943 all succeeded.

The nine dev.1–dev.9 GitHub releases and tags were deleted after archiving their ELF, sidecar, metadata, notices and release notes outside the repository under `~/GitHub/_archive/SteamController-PS5-releases-2026-11-18`; archived ELF digests match the historical receipts. Only `dev/v1.0.0-experimental.1` remains. The pre-squash main commit is retained locally as branch `archive/pre-squash-main` (`08b1b97`) and in the pre-alignment bundle; it is no longer reachable from GitHub `main` or tags.

NOT_RUN: physical PS5/FW13.60 execution, browser/Origin behavior, native DualSense coexistence, motion/haptics, tile registration and USB suspend/rest-wake. The SDK cross-build ran in CI, not on this host. This is an experimental prerelease; no stable release or firmware-compatibility claim exists, and the stable hardware gate is unchanged.

Next: obtain controlled physical PS5 evidence for the published ELF and firmware 13.60; append any follow-up receipt without rewriting the immutable `dev/v1.0.0-experimental.1` tag or its assets. Any further published defect fix requires a new `v1.0.0-experimental.N` version.

# Historical — dev.9 published (2026-11-15)# Historical — dev.9 published (2026-11-15)


## M1-256 — dev.9 publication verified (2026-11-15)

Published experimental [v0.1.0-dev.9](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v0.1.0-dev.9) from `d6d51af1ce1d0cf14438fb73efae3cd2652b1d25` by [DEV workflow 43703160706](https://github.com/MNeroba/SteamController-PS5/actions/runs/43703160706) on 2026-11-15T10:18:02Z. Three release jobs succeeded. Seven assets contain one ELF, `SteamController-PS5-v0.1.0-dev.9.elf`, 2,068,880 bytes, SHA-256 `e4c1ad438f4f3861e846338528f4139f9e8e7dce3695839c96b31c53190cfc42`. The tag points to the selected source, every downloaded asset matched GitHub's digest, and the ELF embedded source, version, metadata and sidecar checksum were verified.

Owner selected no pairing code and PS5-only access, then requested automatic Media tile registration on first launch. WebUI binds IPv4 loopback only; other console processes can access it. Literal-loopback Host, same-host Origin, mandatory POST Origin, no-sniff and frame denial reduce browser-origin attacks. A joinable worker attempts registration only for an absent exact title path; it does not overwrite an existing or inaccessible path. An explicit local retry remains available. The tile does not start the payload. Physical PS5 FW13.60/browser/Origin, automatic installer privilege/visibility, registration recovery and shutdown remain NOT_RUN.

Exact-source push Host43702935960, SDK43702935974 and secret scan43702935930 succeeded. The release workflow repeated normal Host, ASan/UBSan, Chromium, deterministic assets, SDKv0.43/LLVM18, package/dependency and checksum gates. The earlier DEV workflow43702614078 was deliberately cancelled before creating a tag to correct misleading tile-status wording. Historical releases and the stable hardware gate remain unchanged.

Downloaded assets and GitHub metadata are retained in output/dev9-published. Release digest is from the published Linux artifact and differs from local SDK builds. Next: controlled physical test with exact release SHA, console firmware/loader, native DualSense and USB/Puck status; do not claim compatibility or post to others without an explicit request. Previous dev.8 and earlier tags/assets are unchanged.

---

Historical records follow unchanged.

# Current — dev.9 publication preparation (2026-11-15)


## M1-256 — owner-approved code-free console WebUI and automatic tile (2026-11-15)

Owner requests disabling the code completely, publishing a new DEV release after verification, selects **no code only on the PS5** instead of LAN access, then explicitly requests automatic first-run tile registration without a manual button. This supersedes the earlier pairing refresh and opt-in tile assumptions for this release only. Explicit choice is loopback-only, not anonymous LAN.

VERIFIED_CODE: WebUI binds IPv4 127.0.0.1, rejects non-loopback peers and non-literal-loopback Host headers, validates Origin against Host and requires Origin on POST; no active pairing, bearer, random token or credential-storage initialization. `/pair` and `/api/pair-refresh` are refused. Other local console processes can reach the service; browser-origin checks are not process isolation. PS5 browser handling of Origin is not physically verified. The old auth module remains as historical portable test code but is removed from the payload build.

The WebUI starts an owned joinable background worker which checks the exact SCPS50001 title path. Only when absent does it call the existing ownership-protected installer. Existing title, symlink, inaccessible path or unknown state triggers no automatic overwrite/re-registration. Registration errors notify/log and do not block controller startup; shutdown joins the worker. User explicitly authorizes this persistent PS5 Media registration attempt on first launch. No manual confirmation is needed for the automatic attempt; the local UI retains explicit manual retry. If registration fails after creating files, automatic later launches skip the existing path; use a separately available PS5 browser to inspect and retry. No console operation was performed. Native installer, privileges, visible tile, browser loopback/Origin, teardown timing and firmware13.60 remain NOT_RUN on PS5.

Tests: expected-red unauthenticated local status was recorded in /private/tmp/scps5-loopback-red.log. Host53 C/11 Python and full ASan/UBSan passed on the runtime code; final additional existing-path, access-error, installer-failure and DNS-rebinding/origin tests passed under separately built ASan/UBSan. Chromium222 checks passed, including no prompt and local-only request behavior. Clean SDKv0.43/LLVM18 cross-build and optional installer dependency guard passed on the candidate; exact committed-source and GitHub repeated gates remain to be run. No physical hardware claim, no publication acknowledgement yet.

---

Historical release and candidate records follow.

# Current status — dev.8 published (2026-11-03)


## M1-255 — dev.8 publication verified (2026-11-03)

Published experimental [v0.1.0-dev.8](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v0.1.0-dev.8) from `1cf624f2ccaf869eecd8c29dd29e834529f8a3d9` by [DEV workflow 41688254764](https://github.com/MNeroba/SteamController-PS5/actions/runs/41688254764) on **3 November 2026, 17:54:11 UTC**. All three release jobs completed successfully. Seven assets contain exactly one ELF, `SteamController-PS5-v0.1.0-dev.8.elf`, 2,121,120 bytes, SHA-256 `7a37126cd223259f06deb9b420c74fb7b9aaee7d353495e5eb81b69e93be0d2a`. The remote tag points to the selected source; every downloaded asset matched GitHub's digest, and the ELF source/version, sidecar checksum and metadata were independently checked.

Exact-source push Host41687756285, SDK41687756162, full-history secret scan41687756071 and CI smoke41687756186 passed. The DEV workflow repeated normal53 C/11 Python, ASan/UBSan, Chromium220, image/embedding/deterministic-asset, clean SDKv0.43/LLVM18, dependency, metadata/changelog/package and checksum gates. This is VERIFIED_HOST_TEST/VERIFIED_PS5_BUILD, not physical acceptance. Native DualSense coexistence/account assignment, FW13.60 acceptance, actual game motion/touch/quaternion, USB suspend/reconnect, active-game rest/wake and tile behavior remain NOT_RUN. Direct HID remains opt-in and waits for one confirmed foreground user plus a readable disconnected native pad; no native disconnect/unbind/Open, Shell patch or radio operation is enabled. Historical tags/releases and the stable hardware gate remain unchanged.

Downloaded release data and independently checked assets are saved in desktop output/dev8-published (release.json plus seven assets). The Linux release hash is distinct from the earlier local macOS candidate and is not substituted into historical test records. The source tree retains163 payload blobs; no history rewrite, source loss, native controller operation or console deployment. README, current build/loading/tile/security/feature/changelog/DEV docs are refreshed only after GitHub acknowledgement. Previous release receipts remain historical. Next: publish this documentation receipt, observe its Actions, and await controlled dev.8 tester evidence with exact release checksum/firmware/loader/transport. Issue2 remains open; do not claim hardware compatibility or post a tester message without an explicit communication request.

---

Historical approval/preparation checkpoints follow unchanged.

# Current publication — dev.8 / M1-255 (2026-11-03)


## M1-255 — dev.8 publication authorized (2026-11-03)

Owner explicitly requests sending dev.8 changes to main, running Actions and publishing only after success. This supersedes the M1-253/254 publication hold; no console deployment/hardware approval is implied. Source candidate0d1d1a65cb023a53e7338bc312fb33ad57614eb3 contains verified runtime6882512 plus notes/package regression. Normal53 C/11 Python, sanitizer, Chromium220, clean-clone SDKv0.43/LLVM18 and metadata/changelog/tile gates passed locally. Prior local candidate digest remains separate from future Linux release output.

GitHub read confirms mainb83a4b642c35dbf86d95331137c2fccfa69708c4 and unused dev/v0.1.0-dev.8 tag; authenticated CLI has push permission. Use explicit owner URL, since inherited origin points to the contributor fork. Preserve complete tree/history; non-force HEAD:main only, exact selected SHA must pass Host, SDK and secret scan. Then dispatch dev-release.yml from main with full SHA/versionv0.1.0-dev.8; workflow repeats normal/sanitizer/Chromium/assets/SDK/checksum gates. Verify remote tag/source/prerelease/seven assets/one ELF and all downloaded hashes before claiming publication.

Physical PS5 execution, targetFW13.60, native coexistence/account binding, game motion/touch/quaternion, rest/wake remain NOT_RUN. Default library path retained; direct HID remains explicit ENABLE_DIRECT_HID=1 and refuses connected/unknown native state. No approval to mutate native controllers/radio/Shell. Inherited dirty desktop overlay remains untouched; source lives in isolated worktree.

---

Historical preparation checkpoints below are retained.

# Current status — 2026-11-03

## M1-254 — full raw-HID contract (2026-11-03, unpublished)

Owner asks to resolve the extended-report blocker and finish dev.8 without publication. Public main checked through GitHub API remains b83a4b642c35dbf86d95331137c2fccfa69708c4; no open PR. Work remains local codex/dev8-hid-rest in /private/tmp/steamcontroller-runtime-audit; inherited dirty desktop overlay preserved.

VERIFIED_CODE: pinned ProsperoSunshine3574d60989efe7d0b2410e2ff137cd1c4d490c60 full168-byte report, ps5upload efcd6fd4a2ce416b226c9a28ef16464ad7fbcb73 same-lineage corroboration; native acceleration/velocity order cross-checked against OpenOrbis0a1aaf9dd4a92695538bdeb09fb056d06dd11725. Independently authored adapter/test; provenance recorded before adaptation. See docs/development/RAW_HID_REPORT.md for exact paths/lines/table/license/evidence. Upstream FW13.00 readback is UPSTREAM_CLAIM only, not our FW13.60 validation.

Regression-first: test_hid_report_full failed old HID_REPORT_SIZE160 assertion, log /private/tmp/dev8-hid-full-red.log. Fixed168-byte translation for Create, two contacts with stable IDs/release, quaternion, gyro(rad/s), acceleration(g); all10 floats and bounds validated. Corrected native ScePadData accel0x1c/vel0x28 and erroneous Create=PS alias; native ABI static assertions. Neutral gravity is(0,1,0), actual supplied acceleration remains intact.

Upstream reports a native-controller disconnection on same-account registration. Do not adopt its account-directory scanning/automatic user fallback. Direct descriptor uses actual supported-API confirmed foreground ID; shared one-user/native-disconnected policy before reservation and authorizer rechecks before/after open. Connected/unknown native/missing user waits, with rate-limited identifier-free diagnostics. Recovery deletes only old owned receipt and defers re-create while unsafe. Last-read/ioctl race is not atomic; post-reservation failure conservatively inhibits creation. Missing readable disconnected handle can leave mode waiting. Direct remains opt-in, no native disconnect/unbind/Open, Shell patches, radio operations or backend fallback.

This supersedes M1-253's unresolved touch/Create/motion code finding; earlier negative results are retained. Remove only DEV8-HID-EXTENDED after source review and targeted host regressions. Firmware/game/native coexistence/quaternion/rest-wake hardware tests remain NOT_RUN and experimental release policy remains unchanged. No push/PR/tag/release/deployment authorized.



## M1-253 — dev.8 preparation (unpublished, 2026-11-03)

Owner authorizes implementing direct HID and rest/wake, testing and preparing dev.8 without publication. Worktree /private/tmp/steamcontroller-runtime-audit, branch codex/dev8-hid-rest, base b83a4b642c35dbf86d95331137c2fccfa69708c4. Desktop dirty overlay preserved. No remote push, PR, tag, release, console deployment or active hardware experiment.

Implemented hid_backend ABI assertions, zeroed request/descriptor/report encoding and injected I/O seam; integrate through existing create/delete/inject callbacks and registry. Exact-handle checks, global registry creation reservation, poison on ambiguous/invalid Add, no fallback/native binding, retained failed Delete and one-time FD close. Direct is explicit ENABLE_DIRECT_HID=1, default library path unchanged; combining direct with hooks/handoff/bridge fails policy validation. Unsupported Create/unknown bits/touch/accel/gyro encode attempts neutral and stops payload through normal owned cleanup. Public sources do not establish those extended offsets: preserve canonical state/default backend and block release rather than silently regress them. No native DualSense open/disconnect/unbind, Shell patch, radio operation or storage migration. P0 DEV8-HID-EXTENDED is recorded in critical_code_blockers. Dev.8 is a partial experimental implementation candidate, not a feature-complete publishable release.

Rest/wake: manager read-only polls SceSystemStateMgrInfo through SDK symbols; suspend/standby latch atomic pause. Scanner/reader cancellation uses existing resource cleanup/neutralization/release. Manager joins every previous USB worker before processing resume; two spaced working observations then permit fresh-generation discovery. Unknown/error never resumes a paused session. Failed VDA cleanup remains registry-owned; no forced thread cancellation, blind deletion or auto payload restart. User pause/config remains; device-local gyro/mapping reset follows existing disconnect behavior. H-rows unchanged, physical NOT_RUN. Blocking kernel ioctl/join timing remains unverified; no latency bound asserted.

New host regression covers ABI/encoding, finite quaternion, unknown bits, open/Add/Insert/Delete failure, ambiguous receipts, exact handle refusal, lifecycle cleanup retention and power unknown/error/rapid/suspend/resume consensus; existing suites retained. Version/metadata/manual workflow default/changelog now dev.8, latest published dev.7 remains unchanged. New candidate guide docs/development/DEV8_CANDIDATE.md lists limitations and physical gates. Full normal/sanitizer/browser/SDK results and exact local commit/artifact receipt follow in TEST_LOG/SESSION_LOG. Next: finish checks, local commit/build only; resolve extended HID ABI before publication, then measured user assignment/native coexistence/rest-wake tests under separately controlled hardware procedure.


## M1-252 — comparative runtime/SDK review; source changes unreleased

Reviewed current public pins and critical paths in Puckbridge, OmniPad, LibrePad1.1.6, padbridge1.3.1, xpad1.1.0, FGG-XSense, AnyController, SDL and SDKv0.43/v0.45. The [comparison](../docs/development/UPSTREAM_RUNTIME_COMPARISON.md) records evidence quality, licenses, negative hardware reports and decisions. Direct HID receipt is the strongest backend research candidate; its full report/assignment/cleanup must be integrated with our one registry before a controlled console experiment. Rest/wake and asynchronous feedback remain follow-ups. No new ioctl/radio/Shell/power operation is enabled.

Two unreleased source improvements extend existing modules: one queued haptic pulse per reader cycle plus due rumble refresh/stop; left-stick touch and both unsigned raw pad pressures retained by the canonical decoder. Haptic regression fails baseline5052f15 with64 sends in one call, passes with one and preserved FIFO/rumble-stop/generation behavior. Haptics39 checks/4,000 concurrent iterations and protocol276 checks pass; full validation/publication receipts are in TEST_LOG/SESSION_LOG. No measured latency or new PS5 output claim. SDK staysv0.43; seven commits to0.45 contain no USB/VDA/*at() fix. Conflicting SDL/Steamless Menu/View and gyro conventions remain unchanged pending identified fixtures.

Source published on main as `12fcd7ff1548ad6a1ffc0f5c61f3f2f04d9972fa` after exact-candidate Host41681210387 (normal/ASan/UBSan/Chromium), SDK41681210474 and secret41681210935 all completed/success. GitHub read-back confirms main; all313 baseline paths retained,314 files including the report, no deletions. The documentation receipt reruns automatic main checks; it changes no payload/tests/workflow bytes.

Published dev.7/tag/assets remain unchanged, independently rechecked: sourcef4ec9ae, seven assets, singleELF2091960 bytes/digest9bc6e1672bc8d20c8e9fcff85359c976523bdf6296a64e1e92dae34e6d7edb73. Issue2 is OPEN/needs-hardware with only the maintainer's retest request; no new physical result. Worktree: /private/tmp/steamcontroller-runtime-audit, branch codex/upstream-runtime-audit, baseline5052f15158e73735e056cfdeddbf594250b961cb. Do not stage the inherited desktop overlay. Next: direct-HID ABI/ownership tests through existing lifecycle; hardware tests and future release publication remain separate gates.

## M1-251 — verified CI Actions integration (2026-11-01)

All five Dependabot heads from PR3–7 are preserved in the combined ancestry on complete dev.7-era main. Candidate 44b9426a18e69b8d384cf505581903612e8ffd97 passed fresh push checks: [Host/ASan/UBSan/Chromium](https://github.com/MNeroba/SteamController-PS5/actions/runs/41305639575), [SDK v0.43](https://github.com/MNeroba/SteamController-PS5/actions/runs/41305639619), [secret scan](https://github.com/MNeroba/SteamController-PS5/actions/runs/41305639576), and [CI Actions smoke](https://github.com/MNeroba/SteamController-PS5/actions/runs/41305639653), all completed/success.

The smoke log confirms the reviewed checkout/upload/download/github-script revisions were executed; both upload and download jobs passed, every fixture checksum and exact LICENSE/source identity matched. All five upstream action licenses were reviewed as MIT at the referenced revisions (github-script uses LICENSE.md). Hosted Node24, explicit archive:true and digest-mismatch:error are compatible. No privileged-event fork-checkout bypass or write permission was added to the smoke.

Payload, tests, scripts, config, SDK/version/policy and released dev.7 bytes are unchanged. This is CI maintenance, not a new payload release or hardware result. Issue2 now requests exact dev.7 evidence and remains open; its dev.5 text is preserved. PR1 stays closed/superseded. Future workflow updates should run this smoke check; future DEV publication still requires its separate manual full-gate procedure and owner approval. Next product task remains controlled physical dev.7 verification.


## Current — M1-250 / dev.7 published

Published experimental [v0.1.0-dev.7](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v0.1.0-dev.7) from `f4ec9ae9e7e287c957a5602493a5b7d46449aed3` by [DEV workflow 41300890370](https://github.com/MNeroba/SteamController-PS5/actions/runs/41300890370) on 2026-11-01 11:15:23 UTC (2026-11-01 13:15:23 EET). Seven assets contain exactly one ELF, `SteamController-PS5-v0.1.0-dev.7.elf`, 2,091,960 bytes, SHA-256 `9bc6e1672bc8d20c8e9fcff85359c976523bdf6296a64e1e92dae34e6d7edb73`. PR8 was merged with contributor commits preserved; both descriptor P1s and the post-drain observation-window issue are resolved in code/host regressions. Exact PR, merged-main and repeated DEV Host/ASan/UBSan/Chromium220/asset/SDKv0.43/LLVM18/checksum gates passed. This is VERIFIED_HOST_TEST and VERIFIED_PS5_BUILD only. Contributor FW13.20/Puck observations at original c3a7ddb are EXTERNAL_HARDWARE; this revised ELF, owner target FW13.60, native DualSense coexistence, gameplay, motion/haptics, console cleanup and tile launch remain physically unverified. No historical result/tag/release was changed.

Next: request a sanitized result from the exact dev.7 ELF with measured firmware, loader and transport. Verify native DualSense before/during/after and use the existing hardware checklist; do not promote the earlier contributor build to acceptance of this release. Further publication requires separate owner approval.


## Historical M1-249 — dev.6 published; repository Public (2026-11-01)

Experimental prerelease [v0.1.0-dev.6](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v0.1.0-dev.6) was published by [DEV workflow 41246199948](https://github.com/MNeroba/SteamController-PS5/actions/runs/41246199948). Tag `dev/v0.1.0-dev.6` points to exact source `4c8b89bc789bf1dfd064264ff432d3919b5a1899`. The release has seven assets and exactly one ELF: **2,087,296 bytes**, SHA-256 `ef52b279be433c1d445ec4a616e6294c647fb87555c3bca34fddddb68da7df9b`. Repeated Host, ASan/UBSan, Chromium, deterministic asset and clean PS5 SDK v0.43 gates passed. This is VERIFIED_HOST_TEST and VERIFIED_PS5_BUILD only. Physical console, USB-C/Puck, VDA ownership/user binding, native DualSense coexistence and runtime storage remain NOT_RUN.

The repository is now Public. GitHub private vulnerability reporting, dependency graph and Dependabot alerts are enabled. No historical test result was changed. The M1-248 fixes in dev.6 remain host/build evidence: the tester's earlier logs identify dev.5 and do not exercise them. Target filesystem behavior, VDA Add/receipt ABI and the EILSEQ workaround remain unconfirmed on Prospero.

Next: provide the tester the published dev.6 ELF and checksum, request sanitized results with measured firmware/loader details, and update hardware evidence only from a real run. No stable release or PS5 compatibility claim.


## Historical M1-249 — dev.6 candidate preparation (2026-10-31, before publication)

The owner requested dev.6 preparation. Latest published release remains dev.5; no dev.6 tag, ELF or release exists. The tester's latest logs still came from published dev.5: USB-C/Puck input works, WebUI storage reports `file-inspect/EINVAL`, and VDA log setup reports `EILSEQ` before AddDevice. No exact tested ELF SHA or measured tester firmware was established. These logs do not exercise the source changes now on main.

The candidate contains the M1-248 fixes: determine an absent protected token from `openat(O_NOFOLLOW) == ENOENT`, with exclusive/no-follow creation and existing validation preserved; retry the bounded VDA observer once through a fresh loopback stream only when the selected `/dev/klog` reader returns `EILSEQ`. Add remains blocked unless the new stream drains to `EAGAIN`. No Add ABI, event ownership, account binding, firmware or native DualSense policy is changed.

Dev.6 release metadata is aligned: version header, two-section changelog, Payload Manager sidecar, manual workflow default and version-neutral packaged loading guide. The first versioned source draft `617fdbc4537cf3362439181c0d7d748941b79bdf` is superseded because review found it replaced two historical memory files. Those original histories are restored in the follow-up; do not select the first draft for a release. The corrected candidate `4c8b89bc789bf1dfd064264ff432d3919b5a1899` passed exact-SHA push gates: Host [41245762239](https://github.com/MNeroba/SteamController-PS5/actions/runs/41245762239) (normal host, ASan/UBSan and Chromium regression), PS5 SDK cross-build [41245762158](https://github.com/MNeroba/SteamController-PS5/actions/runs/41245762158), and full-history secret scan [41245762162](https://github.com/MNeroba/SteamController-PS5/actions/runs/41245762162), all completed successfully. This is VERIFIED_HOST_TEST and VERIFIED_PS5_BUILD only; no physical PS5 test. No dev.6 tag/release/ELF publication has occurred. The immediately preceding main source `e03f2c4` passed Host/SDK/secret scan; its docs descendant `4bd783c` passed Host/SDK. Local sandbox could not bind loopback, and the inherited desktop checkout is not a safe publication tree. No PS5 hardware claim. The exact source gates are green. Do not dispatch the DEV workflow until separate publication approval is given.

## M1-248 — external dev.5 failure follow-up; source committed, release unpublished

The tester updated the shared Gist with logs from dev.5. Sanitized facts: Steam Controller via USB-C/Puck is detected and reports input; WebUI authentication setup fails at `file-inspect / EINVAL`; VDA event-log setup fails with `EILSEQ` before AddDevice, so this run says nothing about the VDA Add result. No user IDs, tokens, pairing codes or raw system log text are retained here. The run's measured firmware and exact ELF SHA are not established by the inspected excerpt.

The reviewed source follow-up was pushed directly to current `main` as `e03f2c428e98c8e76334539f33e1f336d3b6ae49`. It is not included in published dev.5 and does not modify its tag or assets:

- `payload/webui_auth.c` now recognizes an absent token from `openat(..., O_NOFOLLOW) == ENOENT`, avoiding the subsequent `fstatat(..., AT_SYMLINK_NOFOLLOW)` that plausibly produced `EINVAL` on the tester's filesystem. Create remains exclusive/no-follow; a file that appears/disappears in the race fails safely. The filesystem hypothesis is not hardware-confirmed.
- `payload/vda_pending.c`/`payload/gc_main.c` allow one loopback-reader retry only when `/dev/klog` is the selected source and its bounded initial read returns `EILSEQ`. The loopback stream must independently drain to `EAGAIN`; otherwise Add is not called. Other failures remain fail-closed, and source/error logs are more specific. The tester's old log did not identify its reader source.
- Regression-first storage test failed on injected `fstatat` `EINVAL`; after the fix, storage tests pass. VDA test injects `EILSEQ`, verifies the reader is discarded and the bounded alternate stream is used; unrelated errors do not retry, and a fallback-open failure without errno is normalized to `EIO`.

Validation: normal host C/Python tests pass except `test_webui_server`, whose loopback `bind()` is denied with `EPERM` in this sandbox. All other host binaries passed in the targeted sweep. ASan/UBSan sweep also passed for every runnable binary except the same listener test. Browser preview cannot bind localhost (`EPERM`); PS5 SDK v0.43 is not configured/found, so cross-build is NOT_RUN. Physical PS5 validation is NOT_RUN. The published dev.5 source/tag/assets remain unchanged.

Checkout constraint: local Git is `fix/p0-safety` based on `7846011` with a large inherited mixed/dirty tree, not an authoritative current-main snapshot. Do not commit or publish the whole checkout. Before any replacement release, transplant only reviewed files onto a freshly verified complete `main` tree, run green exact-source CI/SDK/browser gates, and get separate owner approval.

Source fix `e03f2c428e98c8e76334539f33e1f336d3b6ae49` is an ancestor of current main `4bd783c1b8dc1082942b85a7fcee72a0e0c45632`. Exact-source Host [41245027045](https://github.com/MNeroba/SteamController-PS5/actions/runs/41245027045), PS5 SDK [41245027061](https://github.com/MNeroba/SteamController-PS5/actions/runs/41245027061), and secret scan [41245027077](https://github.com/MNeroba/SteamController-PS5/actions/runs/41245027077) passed; the docs descendant also passed Host [41245148448](https://github.com/MNeroba/SteamController-PS5/actions/runs/41245148448) and SDK [41245148430](https://github.com/MNeroba/SteamController-PS5/actions/runs/41245148430). The versioned dev.6 commit still needs its own exact-SHA Host/SDK checks. The published dev.5 ELF/tag/assets remain unchanged. Next: complete the versioned dev.6 source commit, wait for its exact-SHA Host/SDK runs, then request separate publication approval. A tester must receive the actual release ELF and SHA-256 only after publication. Never describe this workaround as PS5-proven. Never describe this workaround as PS5-proven.

---

# Historical current status — 2026-10-16

## M1-247 — published dev.5 and documentation reconciliation

**Published experimental dev.5:** [dev/v0.1.0-dev.5](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v0.1.0-dev.5), source `382e4ecef4937356b72f9e1786917ae7147c0415`. [DEV workflow38810046681](https://github.com/MNeroba/SteamController-PS5/actions/runs/38810046681) completed successfully; release414294352 is a prerelease, not a draft, published **2026-10-15 21:30:53 UTC** (16 October in Riga). Seven assets contain exactly one ELF, **2,084,968 bytes**, SHA-256 `01a73f3f676121061f58b52a833c642d8cd94b048fbbc524182b8756eb642bd0`. Host48 C/Python, ASan/UBSan, Chromium220, deterministic assets and clean SDKv0.43/LLVM18 gates passed on that selected source. Physical Add/VDI/MBus, native DualSense coexistence, firmware compatibility and actual storage errno remain unverified. Event correlation is an owner-approved temporary experimental mechanism, not proof of exclusive ownership. Previous releases/tags/assets are unchanged.

Documentation-only main commit `5f91c44f87d6615dba625d28d063ba342ce37bc1` is published after the source release. Its exact-source PS5 cross-build [38913781983](https://github.com/MNeroba/SteamController-PS5/actions/runs/38913781983) and Host suite [38913781945](https://github.com/MNeroba/SteamController-PS5/actions/runs/38913781945) both completed successfully. Documentation checks are summarized in TEST_LOG/SESSION_LOG.

Current user/build/tile instructions now target dev.5. Persistent and session-only enrollment are distinguished; a redeemed code cannot enroll a second tile browser. Do not repeatedly reload an unresolved VDA to get another code. No code/assets/workflows/release bytes or repository visibility are changed by this documentation pass.

Next: obtain the tester's sanitized dev.5 diagnostic report with exact ELF SHA-256, measured firmware/loader, VDA status, storage stage/errno, native DualSense behavior and USB-C/Puck result. Follow docs/development/VDA_PENDING.md; no physical test is claimed. Future version publication requires new approval.

## Historical checkpoints — retained evidence

Earlier candidate holds and queued CI statements below are superseded by the receipt above.

## M1-246 — current dev.5 execution, 2026-10-16

Owner explicitly authorizes the analogous event-correlation approach temporarily and publishing dev.5 for the log author's controlled test. Baseline main6e1e82fefe3b9ae4408fb5d4540cc485a1ca4b03 has294 files; Host38805657403/SDK38805657360 succeeded, no open PR. Complete-tree source publication and manual DEV workflow must follow final validation, with no historical asset changes.

Implemented: one asynchronous Add/reservation, bounded fresh typed virtual-event reader, receipt provenance, cancelled/retiring cleanup and global failed-cleanup barrier; native exclusion and Steam/PS-only guarded local MBus/picker fallback, queued150ms PS action; protected RAM-only auth after operational storage failure; expanded secret-free stage/errno/count diagnostics and first-IN zero initialization. Existing USB/Steam/profile/tile security and SDKv0.43 retained. See docs/development/VDA_PENDING.md and ADR061/062.

Priority status: P0 actual Add/VDI/MBus contract and native coexistence BLOCKED_HARDWARE; operational correlation implemented under explicit owner acceptance, not proven exclusive ownership. P1 exact tester storage errno still unknown; reporting/session-only fallback host-verified. P1 USB shutdown/reconnect host fault suites pass, physical ioctl/cleanup timing NOT_RUN. No PS5/radio/firmware/tile operation performed. No release acknowledgement yet; local/exact-source final checks and release receipt follow in TEST_LOG/SESSION_LOG.





M1-245 exact-source CI ACK: 05587f209d8171d0e9492ce87bb6224ece3c8b63 Host38805088725/job116663752470 and SDK38805088852 completed/success. All normal46 C/Python, sanitizer and Chromium220 steps pass remotely. Exact local export repeats normal/sanitizer/SDK build; selected SHA/version and exact tile PNG/param embedded. Local candidate2047056 bytes/SHA25672e8bc941809ece3f7a693e4a3ea2a39c5a82d47951b4122ce4e09e910dd9841, not a published Linux release. Exact host logbc16b88f536629ccef3439906016d62295fc29d14fc86e218f54bb3e0b5313c2; sanitizecb6f4c0edd51c05cf09ff5df46367cbeda4704ea553024355c57c49d470f8359; buildda9a848417ca12011b6b324ff9add71a16f30ddaa1128f2dec530303cddf5bae. GitHub releases remain dev.1–dev.4 and matching tag enumeration contains no dev.5. No manual dispatch/publication/physical action. Gate remains blocked for VDA ownership; actual PS5 storage cause unverified. Next: source-proven receipt/assignment ABI or separately approved controlled measured hardware capture, then repeat full gates before asking for release approval.

## dev.5 candidate — M1-245, 2026-10-15

Owner requested fixes and preparation WITHOUT publication. Source baseline910f90371971fba08b2b1f375c7c61144617dfdf/tree d96fe30f296039025d4c82d16d8222dbc824bbde has288 files; all production bytes matched. Storage diagnostics and redundant chmod regression fixed; known account fields filtered before raw sinks, Safe Mode klog observer disabled, optional bridge text accounts redacted. Version/metadata/changelog/workflow defaultdev.5. Normal46 C/Python, full ASan/UBSan, Chromium220, SDKv0.43/45 C/dependency/determinism and targeted static analysis pass locally. Real DEV packager correctly rejects the unchanged P0 critical blocker before assets.

P0-VDA-PENDING-04 remains BLOCKED: classification/changed-word mask and containment implemented, not a functioning pending registration path. No source-proven safe receipt/assignment ABI found in pinned analogs; AGENTS forbids guessed ownership/unknown side effects. Existing positive-return ownership/global reservation/lifetime latch and native isolation retained. No PS5/hardware/firmware/radio/tile operation, no dev.5 publication approval. Next: reviewed full-tree source commit/push, exact-source build identity/CI and local candidate checksums; then proven VDA receipt contract or separately approved controlled hardware evidence. Do not dispatch publication or clear the critical gate on compilation alone. See docs/development/DEV5_CANDIDATE.md.

## External dev.4 logs — M1-244, 2026-10-15

Main remains aa9d4fa881b13c101edd1965baac7cbeb8b79655 at intake; Host38590488943/SDK38590488466 completed/success, no open PR. Published dev.4/sourcea5c2034 is unchanged. Complete public Gist snapshot reviewed: new log7572 bytes, previous1990922 bytes/36188 lines. EXTERNAL_HARDWARE: Puck input/battery/buttons/gyro decoded; one Add returns803b0006 and latch inhibits creation, no owned VDA/input injection. Older unidentified Ghost-Control v5 log has11975 Adds (14 pending-style/11961 subsequent803b0003) and14 VDA log identifiers. Temporal events do not prove ownership; device exhaustion/native UI cause remains HYPOTHESIS. Token initialization fails before listener; exact operation/errno unavailable. Default profile also rejected; no confirmed storage cause. Raw klog samples expose account IDs even with hand-off disabled; no identifier values/log bytes committed.

Future DEV gate blocked by P0-VDA-PENDING-04; P1-STORAGE-DIAG-04 and P2-KLOG-PRIVACY-04 recorded. No guessed handles, new ABI calls, MBus writes, storage deletion/permission relaxation, physical test or new release. Next: establish proven pending Add receipt contract, add secret-free per-stage storage diagnostics/fault tests and raw-klog gating/redaction; request already-used ELF SHA and confirmed FW/loader only. Do not ask for another uncontrolled failing run. Detailed evidence/hashes: docs/development/EXTERNAL_RUNTIME_REPORT.md. Existing FW13.60 hardware rows remain NOT_RUN; prior FW5.50 report is not a measured current run.

## M1-243 — Published dev.4, 2026-10-14

Owner explicitly approved creation/publication. Tag dev/v0.1.0-dev.4 points to a5c20346392eac0eb9fa04cc201cd214e2957363; DEV workflow38588689667 completed/success and all three jobs passed. GitHub release412878689, prerelease=true/draft=false, published 2026-10-14T15:29:58Z; seven assets contain exactly one ELF (2029632 bytes, SHA25634ae9fa0d55f8fdadbec4384d4171b029243e2fc6d0eb94a3d0120a6165f9ea4). Selected source has287 blobs/45 production C; prior releases/tags/assets remain unchanged. Exact-source push Host38587348600/SDK38587348499 and repeated DEV normal44 C/Python, ASan/UBSan, Chromium220, deterministic assets, SDKv0.43 build/dependency/package/checksum gates passed. Earlier local browser fixture version mismatch and checksum command wrong-directory negatives retained; corrected reruns passed. Fresh local exact-source normal/sanitizer/build/package PASS; local ELFc236bf1aff8cd842eea092f06568f9b45a9d48fb5404028c9dd84e8414607650 is separate from published Linux output. Three M1-242 safety fixes included; full dev.3 functionality retained. Firmware13.60 unmeasured, physical hardware NOT_RUN, external dev.2/FW5.50 cause/token-storage/native-input problem unconfirmed. Repo visibility unchanged; no console/radio/firmware/tile action. Latest setup/download/changelog/security/status documents updated after publication acknowledgement. Next: review existing tester gc_status.log/.prev or authenticated diagnostic report when supplied; do not ask for secrets or rerun the failing old ELF. Tester reply prepared; no Reddit destination URL known and no external comment posted.

Downloaded dev.4 ELF independently verified:2029632 bytes, SHA25634ae9fa0d55f8fdadbec4384d4171b029243e2fc6d0eb94a3d0120a6165f9ea4 and selected SHA/version/tile embedded. Separate sidecar download declined; no workaround. GitHub notes/digest/build log and workflow checksum gates agree. Earlier checkpoints below are historical and superseded by this release receipt.

## dev.4 approved release preparation — M1-243

Owner explicitly requested create/publish dev.4 before updating the tester reply. Authoritative main310a1f60dccb8ac18cd97661e83cdcee37fbc274/tree9b07c06ce20cae265b91b4727070e8c8fb45a40d has287 files. Runtime c204524 Host38581146662/SDK38581146350 and receipt310a1f Host38581734371/SDK38581734544 are completed/success; this supersedes earlier queued observations. No dev.4 release/tag yet. Version/metadata/changelog and version-aware package tests updated; preserve all existing functionality, SDKv0.43 and /data/ghostpad. Local normal44 C/Python, ASan/UBSan, Chromium220 repeat (old version fixture failure retained), deterministic embedding, SDK45 C/dependency and release/link/diff checks PASS. Next: complete reviewed main commit, own green push gates, manual DEV dispatch, seven assets/one ELF/source/checksum verification, publication receipts and tester reply. No Public visibility change or console operation authorized here.

## Final runtime safety review — M1-242 (unreleased source)

Three confirmed post-dev.3 defects fixed: unattributable Add retry/global reservation gap, reopened USB identity before detach, invalid OUT status overflow. Full normal44 C/Python, ASan/UBSan, Chromium220, clean SDKv0.43/dependency and link114/changelog/DEV/diff gates PASS locally. Sourcec204524cbd14cc232c11289677e7bb6607e59a0e published with all287 blobs retained. Fresh exact export repeats normal/sanitizer/SDK/dependency PASS; five-module static analysis clean. Host38581146662/SDK38581146350 queued at14:32 UTC; remote success not claimed. No new binary/tag/version; published dev.3/earlier assets lack these fixes and README says so. See docs/development/PUBLIC_RUNTIME_REVIEW.md and CHANGELOG Unreleased.

Owner-supplied dev.2/FAT5.50 failure comment lacks logs/digest/error code and remains unverified; no cause or successful hardware correction claimed. Intended13.60 and all physical gates still NOT_RUN. Private source visibility unchanged. Next: verify focused main push Host/SDK CI; owner Public decision and separately approved new DEV binary. Keep stock DualSense, no console/radio/tile operation.

## Published dev.3 — M1-238

**PUBLISHED** experimental prerelease [dev/v0.1.0-dev.3](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v0.1.0-dev.3) from `9aab6dbae166b28416a9a00e6346f85bf12e524e`. Workflow38225113776 completed/success; all three jobs PASS. Dev.1/dev.2 releases are unchanged.

- Implementation/review source **9aab6dbae166b28416a9a00e6346f85bf12e524e** (documentation sourcee7f53f2cf1466bed0f8462bc350155b45b101eb9 has green Host38243078629 and SDK38243078701), tree1512f3fcb9e694d54e55a4d5c8afb982e2d0ca65;282 blobs/45 C,267 previous files retained. Baseline6dc554633b133c0dd8c865168010306e38e66384 was complete267 blobs; old f480e58 push gates both now PASS (queue resolved).
- Implemented portable user-list consensus and existing-worker owned VDA recovery, serialized Add/release ownership, IN/OUT completion retention and cancellation retirement,500ms watchdog, authenticated bounded/redacted diagnostic download, truthful limited connection notices, matching Payload Manager metadata and mandatory two-section changelogs.
- Existing single ELF/tile/artwork/pairing/profiles/gyro/touch/haptics preserved. Tile remains explicitly confirmed. SDKv0.43, /data/ghostpad, safe defaults retained. No automatic account binding/native DualSense mutation, storage migration, new generic controller support or radio operation added.
- Local normal/sanitizer44 C suites plus existing Python contracts/9 artwork/7 DEV/5 changelog checks; Chromium220; clean45-C SDKv0.43 and optional-dependency checks recorded in TEST_LOG. Fresh282-file export matches remote; repeat normal host + exact-SHA SDK build and seven-file/one-ELF local package PASS. ELF2028864 bytes, SHA256fc71b6acbf9ac857e3fe13259b37bdfecc0cb150a313db355222d3b4258188af at output/dev3-review (not a GitHub release download). Push SDK38223580419 PASS; Host38223580325 PASS.
- H-13 Settings tile, H-14 user recovery/native coexistence and H-15 actual USB cancellation/watchdog/diagnostic browser/notification behavior remain NOT_RUN/BLOCKED_HARDWARE. Positive Add receipt semantics and terminal USB close require physical confirmation. Stable gate still fails closed.

Published download:2029288-byte ELF; SHA256 `013e27f60091cd5de87577beb9acd40ac5ae5eeba08092b7dff65bf56847c752`, independently downloaded and verified against GitHub asset digest/release notes, exact SHA/version embedded. Seven assets/oneELF plus source archives; tag/source and prerelease flags verified. Next: follow hardware TEST_PLAN before an explicitly authorized console experiment; H13/H14/H15 remain NOT_RUN. No physical deployment, tile installation or radio operation performed.

## Public-facing preparation — M1-239

Documentation update published as e7f53f2cf1466bed0f8462bc350155b45b101eb9; Host38243078629 and SDK38243078701 completed/success. README and active build/feature/UI/specification guidance reconciled with published dev.3. Contributor issue forms and SECURITY.md added; all109 active relative links and targeted changelog/package/form checks pass. No runtime source/build/test/workflow change; historic results/releases retained.

Repository remains private. M1-240 follow-up covers both remote branches, all three tags,140 commits/trees, all1,413 text blobs and63 SHA-matching image blobs; targeted signatures found no credentials. PR#1 is closed with no comments/reviews; no project discussion attachments returned. Release bodies/inventories and matching downloaded dev.2/dev.3 ELFs checked. Full historical release-binary forensics, upstream linked media and independent secret-scanner run remain outside scope. Historical local paths contain the maintainer username; artwork publication permission is separate from an open asset license. No confirmed public-publication blocker identified. Follow [PUBLIC_READINESS](../docs/development/PUBLIC_READINESS.md), refresh About and verify desired private vulnerability reporting. No compatibility/stable claim or visibility change.

Reviewed mainaf183a2a0ed9b98e427864c216df9d68da854363 Host38243782521 and SDK38243782563 both PASS. Follow-up audit receipt55365f6164bff4475c4b038aee157833e26aa89c Host38284898787/SDK38284898822 also both PASS. M1-241 owner requested the metadata update: About now describes experimental USB/Puck, remapping/gyro/trackpads/haptics, one ELF and hardware-unverified status; UI/API verified. Private vulnerability reporting is unavailable while private per authenticated UI and GitHub's public-repository documentation; enable/verify after Public. Visibility remains private. Next: owner visibility decision after reading the scoped audit, then Security reporting enablement. No hardware operation.

---

## Historical status (previous checkpoint, retained)

# Current status — 2026-10-11

## Published dev.2

- [Experimental prerelease dev/v0.1.0-dev.2](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v0.1.0-dev.2) is published, prerelease=true/draft=false, six assets plus source archives. [DEV workflow38141611911](https://github.com/MNeroba/SteamController-PS5/actions/runs/38141611911) completed/success, all three jobs PASS. Evidence M1-229.
- Exact source/tag: `6ed81c2ba477285e932bd4593aa029554cadf757`. Verified source tree50f3f5dd7b26cbddd446de9f0d09498b6c6bb3b6:255 blobs/all41 production C units/all251 previous files retained/zero deletions. Later documentation acknowledgement commits do not change this implementation.
- Both exact-source push checks PASS: Host38125160695 and SDK38125160699. Repeated DEV normal/ASan-UBSan/Chromium202+/asset/build/checksum gates PASS with SDKv0.43/LLVM18. All16 confirmed static findings have scoped remediation; see STATIC_AUDIT.md, ADR-053, M1-227/228.
- Actual downloaded ELF1782328 bytes, ELF64/little-endian/x86_64. SHA256 `bab7c92d8b49b56356a8d616104fc99ef69c49084adb49a76b574e92cb07dfbf` matches downloaded checksum, release notes and GitHub asset metadata. This is the published Linux digest; earlier local build digest is historical.

## First launch

Missing policy starts authenticated WebUI8090. Pair with the eight-digit local notification code (five minutes/five attempts/one redemption); valid session restoration and advanced token recovery supported. Explicit disable and invalid-file fail-closed behavior preserved. Hooks/probe/DualSense handoff stay off. Notification delivery and actual portal remain physically unverified.

One active Steam Controller2026 session. PS5 FW13.60 is owner-reported/unverified (owner reports upgrading from12.00 on2026-10-11). BLE/Edge/custom ShellUI identity research only. No physical console operation performed.

## Remaining gates and continuation

The earlier external runner queue blocker is resolved; its negative evidence remains in logs. No DEV publication blocker remains for this source. Stable H-01/H-02/H-03/H-04/H-05/H-09/H-12 remain NOT_RUN. Real USB cancellation/watchdog timing, VDA ownership/neutralization, credentials, native hooks/unmap/rollback, motion/touch/haptics and native DualSense coexistence require hardware.

Next safely executable hardware stage: read docs/TEST_PLAN.md and HARDWARE_MATRIX.md, record measured firmware/read-only console baseline before an explicitly authorized controlled runtime test. Use only an independently confirmed compatible development loader; retain stock DualSense. Do not update firmware or operate the stock radio. No console access is available in this session.

## M1-231 — updated console target

Owner now reports FW13.60. This supersedes the earlier refusal to update; it is not independently measured firmware or a runtime compatibility result. Current hardware matrix targets13.60 with all rows still NOT_RUN. SDKv0.43, USB/Puck and safety defaults remain unchanged. Published dev.2/tag/assets/release notes retain their historical12.00 target and exact checksum. Before any new DEV release, adapt scripts/package_dev_release.py:13-20, tests/test_dev_release.py:30 and dev-release.json together (they currently pin12.00), and refresh current README/loading/BLE/test-plan text without relabeling historical results. No new release, deployment or system-resource write performed. Launcher M1-230 requires fresh FW13.60 hardware checks.

## dev.3 in progress — M1-232

Single ELF now embeds the supplied icon and optional Media settings tile installer. Pair, choose Settings & help → PS5 tile, confirm installation; tile opens loopback8090 while payload runs. Local40 C/normal/sanitizer/Chromium214/SDK build gates PASS. Current firmware policy13.60 owner-reported/unverified, SDKv0.43 unchanged. Fresh complete-source validation, remote push checks and dev.3 publication are next; no new release is claimed yet. H-13 tile registration/browser/navigation/removal remains NOT_RUN, alongside existing console gates.

## M1-233 — release candidate refinement

Initial tile source0a3dacb is superseded by lazy installer loading: missing AppInstUtil/API cannot impose a new controller-payload startup dependency. Full fresh normal40 C, sanitizer, Chromium214, SDK build and ELF dependency/package checks PASS; DEV contract suite now7. Feature/status/navigation updated. Select only the follow-up full source SHA for dev.3 after its own Host/PS5 push checks pass. Previous queued jobs had runner_id0/no steps for over10min. No dev.3 release/tag/assets are claimed yet. H-13 remains NOT_RUN.

## M1-234 — final permissions check

Final new-directory umask077 regression fixed and verified without changing existing directories. Normal/sanitizer40 C/Python7, SDK/dependency checks PASS; Chromium214 remains valid for unchanged browser/UI source. Use the final permission-fix main SHA (not0a3dacb or b214da9) for dev.3 after its own green push gates. One ELF remains experimental; on-console FW13.60 icon/registration/browser use is NOT_RUN. GitHub's previous jobs remained queued/unassigned; no dev.3 release or download is claimed yet.

## M1-234 final publication checkpoint — 2026-10-11 20:05 UTC

Implementation source ACK: f480e58b953c75a87d7f1b929789ad023fc05fb1; tree 91184e1a2c60c599356a8752a42433473fc47896; all267 blobs/43 productionC retained, no deletions/unscoped edits. Exact fresh-source export verified. Final local one-ELF digest14447b40d07a1ce9713a808bb2f42d0307a9d1dd8cd318e628b93a111da3755e; workspace payload/ghost-control-ps5.elf and its checksum now match. Local6-file package /tmp/sc-dev3-reviewed-assets has exactly one ELF, correct source SHA, embedded PNG/metadata and matching checksum. Normal/sanitizer40 C +7 DEV contracts, Chromium214 and SDK/dependency gates PASS; on-console FW13.60/H-13 NOT_RUN.

External release blocker: exact-source Host38170245378/job114561164646 and SDK38170245381/job114561164692 still queued, runner_id0, steps0 at20:05:58 UTC. Earlier candidate jobs had the same unassigned state for over16min; obsolete0a3dacb cancellations requested. No compiler failure, global incident or billing cause is proven. dev.3 tag/release/assets NOT_PUBLISHED; do not represent local packaging as a GitHub download. Leave current candidate checks queued; do not bypass the gate.

Next exact action: query latest push runs for f480e58b953c75a87d7f1b929789ad023fc05fb1; only after both succeed, dispatch dev-release.yml on main with source_sha=f480e58b953c75a87d7f1b929789ad023fc05fb1, version=v0.1.0-dev.3 (authenticated browser form is prepared). Then wait repeated gates, verify tag points to selected source, prerelease=true, six assets/one ELF, actual downloaded SHA256 and source; append acknowledgement. Existing dev.2/tag/assets and historical FW12.00 records unchanged. No physical deployment, app registration or radio/firmware operation performed.

## M1-235 — LibrePad v1.1.2 task intake, 2026-10-12

Owner requests integration in existing unpublished dev.3 and explicitly withholds release/tag approval. This supersedes earlier dispatch next actions: DO NOT DISPATCH or publish dev.3 until owner approval. Main6dc554633b133c0dd8c865168010306e38e66384 has267 blobs; all match local bytes. Original dev.3 sourcef480e58b953c75a87d7f1b929789ad023fc05fb1 Host38170245378/SDK38170245381 both completed/success; old runner queue is resolved. No open PR; only dev.1/dev.2 published, unchanged. Reference tagv1.1.2 annotatedbb60defad681563742448faaefa29b440ea83a0e resolves7466317b4b356249e07770f3914c995e1a4d633a. RootGPL3, no code copied yet. Release now warns of rejected symlink storage migration and recommends1.1.3; do not adopt migration. Logout recovery is upstream host evidence only. Preserve single ELF/tile, USB/Puck/security/defaults and /data/ghostpad. Next: audit pinned virtual/USB/diagnostics/metadata source, add regressions, implement smallest owned-worker recovery and bounded authenticated report, changelog/metadata gates, full local validation and reviewed main push; stop before DEV publication. No hardware available; all physical rows NOT_RUN.


## M1-250 — PR8 corrective follow-up / dev.7 (2026-11-01)

Owner authorizes fixes, validated PR8 merge and a new experimental release. Both P1 defects reproduced against c3a7ddb, then resolved using explicit syscall error status rather than fd/inode inference. Contributor's 1 MiB WebUI stack and bounded 4 MiB klog drain retained; 51 host targets, ASan/UBSan, Chromium220, amd64 CF/register trap and clean SDKv0.43/LLVM18 build pass locally. Exact-head GitHub CI/merge/publication pending. External contributor FW13.20/Puck results are not a test of the revised adapter or FW13.60. See [PR8 review and correction](PR8_REVIEW.md).


## M1-251 — PR3–7 Actions maintenance preparation (2026-11-01)

Owner requests necessary follow-up for PR1–7. PR1 stays closed/superseded. Issue2 updated to exact dev.7 source f4ec9ae9e7e287c957a5602493a5b7d46449aed3, ELF digest9bc6e1672bc8d20c8e9fcff85359c976523bdf6296a64e1e92dae34e6d7edb73 and hardware/native coexistence/storage checklist; dev.5 body preserved. Five original Dependabot heads merged locally onto complete main f44da276c6672bb283d2b715f3a8729c2e598df8; one adjacent checkout/setup-node conflict resolved to both v7. Added read-only CI Actions smoke for real multi-file transfer/checksums and github-script APIs, explicit archive:true/digest-mismatch:error. Actual candidate/main runs pending; no CI success or merge into main claimed yet. No payload/SDK/version/release change. Next: publish codex/actions-maintenance, require smoke plus Host/sanitizer/Chromium/SDK/secret checks, then non-force main integration preserving all PR ancestry and verify GitHub closure. See docs/development/CI_ACTIONS.md.

M1-254 validation: full normal53 C/11 Python, ASan/UBSan, Chromium220, targeted full-report/backend regressions, Clang analyzer and initial SDKv0.43 build PASS; final committed-source receipt follows in desktop local package. DEV8-HID-EXTENDED is resolved in code/host evidence, physical gates remain NOT_RUN. No publication. Exact commands and negative environment result retained in TEST_LOG.


M1-255 remote receipt (2026-11-03): non-force push acknowledged mainb83a4b6→1cf624f2ccaf869eecd8c29dd29e834529f8a3d9. Complete source tree includes163 payload blobs; critical source/tests/workflow/license files verified. Exact-source push checks all succeeded: Host41687756285 (normal/sanitizer/Chromium), SDK41687756162, full-history secret scan41687756071 and CI smoke41687756186. Manual DEV dispatch accepted as workflow41688254764 using source1cf624f2ccaf869eecd8c29dd29e834529f8a3d9/versionv0.1.0-dev.8. Release not yet acknowledged; wait for all three release jobs and verify actual tag/asset hashes. No console deployment or hardware claim.


## 2026-11-15 — local WebUI pairing/tile preparation (unpublished)

Owner requests simpler first WebUI launch and PS5 Media tile setup, no publication. Base main395463adc1d4267ca4bb48db054c861b421da31a is restored from the verified source bundle in /private/tmp/scps5-web-review-1115; desktop dirty overlay is untouched. Existing tile remains explicit, authenticated and exact-title only.

VERIFIED_CODE: authenticated POST /api/pair-refresh rotates the existing one-time CSPRNG pairing code under its mutex, resets expiry/attempt limit, and notifies only the console; no code or bearer token is returned or logged. Notification and rotation are serialized with initial startup. An explicit confirmed successful tile registration requests a fresh console notification; a new-code button allows later retry from an already paired browser even in session-only mode. Old code expires, each new code still has five attempts, five-minute expiry and single redemption. Existing peer, bearer and Origin gates remain. Entropy/clock/notification availability fails closed. If no authenticated browser remains after expiration and protected storage is unavailable, no unauthenticated reset is added; controlled recovery/restart remains an outstanding limitation. Actual PS5 notification, browser interaction, tile registration privilege/ABI and FW13.60 are NOT_RUN.

Expected-red regressions: prior code lacked webui_pairing_refresh (/private/tmp/scps5-pair-red.log); successful tile had no new-code notification (/private/tmp/scps5-tile-red.log); unexpected body received a server error rather than 400 (/private/tmp/scps5-refresh-body-red.log). Verified host regressions cover one-use reissue, unauthenticated/method/cross-origin/body denial, no secrets in response, actual notification and session-only server lifecycle. See follow-up final test receipt. No console deployment or external publication.


2026-11-15 verification: VERIFIED_HOST_TEST `make -C tests clean test` PASS (53 C targets/11 Python modules), `make -C tests sanitize` PASS, then additional Session-only second-browser regression passed under normal and separately built ASan/UBSan. Chromium `scripts/run_webui_browser.sh` PASS (223 checks); the final post-suite change added only the Session-only C test, not runtime/browser code. Expected-red regressions and exact logs /private/tmp/scps5-pair-{red,host-verified,san-verified,session-san,browser}.log plus /private/tmp/scps5-{tile-red,refresh-body-red}.log. `git diff --check` PASS. SDKv0.43 final clean committed-source build follows; physical PS5 NOT_RUN, no publication.


### M1-256 follow-up — loopback browser response hardening

VERIFIED_CODE: before publishing the code-free listener, review identified that a cross-origin classic script or framed page could interact with local unauthenticated responses if MIME sniffing or clickjacking were possible. Added `X-Content-Type-Options: nosniff` on general/PNG responses and `X-Frame-Options: DENY` plus CSP frame-ancestors on general responses. Existing strict literal-loopback Host and same-host Origin (required for POST) remain. This mitigates browser-origin attacks; local PS5 processes retain loopback access. Expected-red header regression /private/tmp/scps5-dev9-headers-red.log; real host HTTP fixture now checks HTML, status and PNG. An initial icon fixture used an invalid test ID and failed; corrected valid fixture passes /private/tmp/scps5-dev9-headers-final.log. Physical PS5 browser behavior remains NOT_RUN.

The original dev.9 push 1029a608ce0517fa413ca780811457813f7e7a4e passed Host43702035447, SDK43702035486, secret scan43702035382 and CI smoke43702035484, but is superseded by this uncommitted hardening; do not dispatch publication until its new exact-source checks pass.


M1-256 release validation 2026-11-15: source df2247d48c947dfa0d6d912606a975e6979f6e11 passed exact-source Host43702372100, SDK43702372010 and secret scan43702372002. Manual DEV workflow43702614078 was deliberately cancelled during validate-build, before publication and before any dev.9 tag existed, after finding UI tile status said registration happened rather than was attempted. Expected-red wording assertion /private/tmp/scps5-dev9-tile-wording-red.log; UI and browser regression now require accurate attempt wording. Repeat browser/SDK and exact-source GitHub push/DEV gates on the follow-up source. No console test or compatibility claim; no historical tag or asset changed.

M1-256 wording follow-up: static expected-red assertion confirmed that the UI falsely stated the tile was registered. The status now says registration is attempted and the Chromium regression asserts that wording. The first browser rerun failed because the generated embedded page was stale; regenerating payload/webui_html.h and rerunning passed223 checks (/private/tmp/scps5-dev9-wording-browser-fixed.log). Full prior Host/ASan and SDK checks passed on df2247d runtime code; only web page text and its browser assertion change here, and exact-source push/release gates must repeat. Workflow43702614078 was cancelled before publication and no dev.9 tag existed.
