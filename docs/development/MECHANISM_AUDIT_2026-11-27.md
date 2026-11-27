# Mechanism audit — 2026-11-27

Scope: full static review of the shipped payload mechanisms plus the release/build gates, cross-checked
against the pinned community SDK, upstream projects and one external FW 13.60 console log.
No code was changed by this audit and no hardware was operated.

Baseline: `main` = `254191ca4910823981be4306e21748147c91f483` (1.0.0 Experimental receipt), clean tree.
Host evidence on the audit machine (macOS, Apple clang 21): `make -C tests clean test` and
`make -C tests sanitize` both exit 0 (53 C targets plus Python contracts).

Evidence labels: `VERIFIED_CODE` (source read here), `VERIFIED_HOST_TEST` (executed here),
`EXTERNAL_HARDWARE` (reported console log), `HYPOTHESIS` (requires hardware or external confirmation).

## Findings

| ID | Sev | Status | Finding | Key evidence |
| --- | --- | --- | --- | --- |
| AUD-01 | high | defect | Media tile installer can never resolve its API on an NID-only SPRX | `payload/settings_tile_ps5.c:75-85`; FW13.60 export inventory |
| AUD-02 | high | defect/UX | In-game vibration is unreachable without policy-gated game hooks; docs/UI do not say so | `payload/gc_main.c:2155`, `payload/runtime_policy.c:29`, `payload/game_hooks.c:48` |
| AUD-03 | medium | defect | A pre-Add refusal is latched as an ambiguous receipt and disables VDA creation for the process lifetime | `payload/slot_lifecycle.c:88-92`, `payload/slot_registry.c:339-345`, `payload/gc_main.c:686-708` |
| AUD-04 | medium | defect | One failed haptic OUT transfer permanently wedges the OUT endpoint for the session | `payload/gc_main.c:1121,1392`, `payload/usb_helpers.c:44`, `payload/usb_resources.c:122` |
| AUD-05 | medium | defect | A stale `<id>.ini.tmp` permanently blocks profile saves (reproduced) | `payload/sc2_profile.c:482-483`; reproducer below |
| AUD-06 | medium | risk | Shutdown has no bounded joins; `runtime_shutdown()==-1` retries forever | `payload/gc_main.c:2194`, `payload/webui.c:684` |
| AUD-07 | medium | defect (CI) | UBSan is recoverable, so the sanitizer gate can pass with UB | `tests/Makefile:92`; clang experiment below |
| AUD-08 | medium | risk | Inherited foreign `v*` tags would run the upstream release workflow with write scope | `.github/workflows/release.yml:6-7`, local tags `v0.1.0..v1.4.0` |
| AUD-09 | medium | defect | A second launch rotates/truncates the live instance's status log before the PID lock rejects it | `payload/gc_main.c:2085` vs `:2103`, `payload/gc_main.c:97-105` |
| AUD-10 | medium | defect (privacy) | Bridge probe copies a report containing account id and pid to a 0644 file | `payload/bridge_probe.c:73-76`, `payload/bridge/wireless_ds4.c:5035` |
| AUD-11 | medium | risk (security) | All-0xFF kernel capabilities plus elevated authid are held for the whole runtime | `payload/gc_main.c:1917-1923`, restore only at `:2059` |
| AUD-12 | low | defect | Long-press fires again when a held input spans a profile generation change | `payload/sc2_binding.c:25-30,131` |
| AUD-13 | low | defect | A latched toggle recomputes its output from the live shift layer instead of the latched mask | `payload/sc2_binding.c:164` |
| AUD-14 | low | risk | Status log bypasses the no-follow/owned-file policy used everywhere else | `payload/gc_main.c:100-102` |
| AUD-15 | low | doc-drift | `Host` without a port is accepted although ARCHITECTURE documents the bound port as required | `payload/webui.c:186`, `docs/ARCHITECTURE.md:112` |
| AUD-16 | low | risk | Haptic method chosen by a WebUI test persists process-wide; methods 1/4 have no fallback and availability is advertised without an OUT endpoint | `payload/webui.c:341`, `payload/sc2_haptics.c:179-190`, `payload/gc_main.c:1352` |
| AUD-17 | low | risk | Release publication uses mutable third-party action tags with `contents: write` | `.github/workflows/release.yml:107`, `.github/workflows/dev-release.yml:174` |
| AUD-18 | info | dead-code | Legacy Xbox/Nintendo/2015 adapters and the eager-VDA/assignment-lease path are unreachable; docs still describe them | `payload/gc_main.c:594,1358,1828`, `docs/ARCHITECTURE.md:68,94` |

## Resolution — v1.5.0-experimental (2026-11-27)

| ID | Resolution |
| --- | --- |
| AUD-01 | FIXED: NID-first resolution for initialize/terminate/install, retry-safe probe, success-with-warning, atomic `.partial` first install and crash recovery; host tests cover NID-only and fallback paths. |
| AUD-02 | PARTIALLY FIXED (v1.5.1): classic PS4-style rumble is now read from `scePadVirtualDeviceGetRemoteSetting` and forwarded without hooks; PS5-native haptics still require `ENABLE_GAME_HOOKS=1`, and the docs/WebUI state this precisely. |
| AUD-03 | FIXED: a skipped pre-Add creation aborts the reservation without the lifetime latch; host regression covers retry in the same session. |
| AUD-04 | FIXED: any retired endpoint restarts the owned session; the portal exposes dropped pulses. |
| AUD-05 | FIXED: stale private temp recovery plus directory fsync, with symlink/foreign-file refusal regressions. |
| AUD-06 | BOUNDED: 30 s cleanup retry then forced credential/PID rollback; WebUI clients/tile and hand-off workers use completion waits with 2–5 s limits. |
| AUD-07 | FIXED: `-fno-sanitize-recover=all` and `UBSAN_OPTIONS=halt_on_error=1`; the fixture that previously exited 0 now aborts. |
| AUD-08 | FIXED: foreign inherited tags deleted locally; `release.yml` requires the tagged commit to be an ancestor of `main`. |
| AUD-09 | FIXED: log rotation happens only after the PID lock is owned. |
| AUD-10 | FIXED: whitelisted private bridge-probe status file, no pid/account id. |
| AUD-11 | ACCEPTED RISK (documented): lifetime kernel credentials are required by the SDK kernel/MBus/hook APIs; disclosed in SECURITY.md. Not changed without hardware evidence. |
| AUD-12/13 | FIXED: held long-press output is suppressed at a profile boundary; latched toggles keep their mask; regressions added. |
| AUD-14 | FIXED: private no-follow status log and 0700 directory. |
| AUD-15 | FIXED IN DOCS: the portless literal loopback `Host` spelling is documented instead of requiring a port. |
| AUD-16 | FIXED: haptic method resets per session; availability/dead-queue accounting exposed. |
| AUD-17 | FIXED: Actions pinned to full SHAs. |
| AUD-18 | DOCUMENTED: legacy adapter and eager/lease paths are labelled unreachable scaffolding in ARCHITECTURE. |

Verification: `make -C tests clean test` and `make -C tests sanitize` exit 0 on the v1.5 tree (macOS, Apple clang 21). PS5 cross-build and all hardware rows remain NOT_RUN.

## Issue #10 verification — "No vibration/haptics in GTA VI" (2026-11-27)

Source: [issue #10](https://github.com/MNeroba/SteamController-PS5/issues/10) body plus its
`gc_status.log` attachment (2,642 lines, no further comments). Reporter environment: PS5 slim,
FW 13.60, published `v1.0.0-experimental` ELF, Puck and USB-C both affected, GTA VI (`PPSA38500`).

| Evidence (log line) | Reading |
| --- | --- |
| `runtime policy: opt-ins bridge=0 hooks=0 dualsense=0 webui=1` (3) | Game hooks were **disabled** for the whole run. |
| 0 occurrences of `game hook`, `first vibration`, `hooks attach` | The hook worker never started; no game feedback was ever captured. |
| `running title 'PPSA38500' (Grand Theft Auto VI)` (1059, 1098, 1856, 2133, 2453) | The payload detected the game; profiles were default/`PPSA38500`. |
| 5× `USB thread: /dev/ugen1.2 VID=0x28de PID=0x1304`, 5× clean exits | Puck enumeration/teardown stable across sessions. |
| 5× `VDA ret=0x803b0006` followed by `one virtual event` and 1,580 `VDI #` progress lines (every 600 inserts) | Event correlation and virtual-pad insertion ran for roughly 0.9 M frames. Input path was active. |
| `VDA local bind deferred: ... native-connected=1; using console picker` (47, 280, 403, …) | A native DualSense was present, so local MBus binding correctly stayed off; user choice on console. |
| `haptics: method 0, sent via 1 (OUT ep open), report 0x81 len 8, result 0` / `haptics: working (interrupt OUT)` (2052-2053) | The controller's haptic hardware and interrupt OUT path work from the payload. |
| 61× `test requested /api/haptic`, 10× `method 2 ... result -5` (2318-2330) | All haptic traffic was **manual WebUI testing**; method 2 (`SET_REPORT`) is unsupported (EIO), method 1/4 work. |
| `tile: automatic first-run registration failed: Required tile installer API unavailable; ... cleanup failed` (10) | Reproduces AUD-01 exactly; fixed in v1.5 by NID resolution and retry-safe installer. |

**Conclusion (VERIFIED against the attached log, EXTERNAL_HARDWARE for the release):** controller
haptics are functional, but the PS5-native GTA VI haptics cannot reach the default payload path:
`ENABLE_GAME_HOOKS` was 0 and PS5 haptics are converted to DualSense haptic audio rather than exposed
as virtual-pad motor values. The issue is a documentation/expectation defect plus a missing feedback
path for PS5 haptics, not a broken haptic transport. A later comparison with the GPL-3.0 `octet`
project found that the virtual pad does expose classic DS4-style remote settings (motors at bytes 3/4);
v1.5.1 now polls them for PS4-style rumble, while confirming that PS5 games leave those values at zero.
v1.5 documents the requirement, surfaces it in the WebUI, logs the disabled state at startup, and fixes
the independent tile/installer defects visible in the same log.

The reporter also could not download the diagnostic report in the PS5 browser. v1.5 now renders the
bounded report inline in the Diagnostics section (with the file download retained as a best-effort
fallback), so a future report can be copied without filesystem access.

Still NOT_RUN: native DualSense coexistence during gameplay, user assignment correctness, motion/touch
quality, and whether any future non-hook feedback mechanism is feasible on FW 13.60.

## Detail

### AUD-01 — tile installer resolves initialize/terminate by name only (high)

`settings_tile_ps5.c` states the rule itself at line 43 ("System SPRX exports are NID-indexed; name lookup
is not guaranteed") and uses `kernel_dynlib_resolve` for `sceAppInstUtilAppInstallTitleDir` (`Wudg3Xe3heE`),
but requires `sceAppInstUtilInitialize`/`Terminate` through C-name `dlsym` (lines 75-76). On FW 13.60 the
export inventory lists the same library with NID bindings
(`libsceappinstutil-783ade3c34.jsonl`: Initialize `540lotO7oHE`, Terminate `kLLazhNh6d4`,
TitleDir `Wudg3Xe3heE`), and the published console log reports exactly
`Required tile installer API unavailable; no files changed; cleanup failed, restart payload`.
`EXTERNAL_HARDWARE` + `VERIFIED_CODE`.

Recommended fix: resolve all three by NID via `kernel_dynlib_handle`/`kernel_dynlib_resolve` with a name
fallback, and name the failing symbol in the message. The SDK sample and ftpsrv link `-lSceAppInstUtil`
instead; that is the alternative but makes the payload unloadable if the library is absent.

Related: a failed `dlclose()` after a failed probe sets `cleanup_failed`, which then rejects every
automatic and manual retry until payload restart (`settings_tile_ps5.c:56,71`), although nothing was
initialized or installed. A successful registration with a failed `Terminate` returns `-1`, so
`webui.c:264-265` answers HTTP 500 with a body that begins "Registration accepted".

### AUD-02 — game vibration path (high, user-visible)

The only producers of `sc2_haptic_*` are: the WebUI test route, the on-screen menu, trigger effects
(`sc2_trigfx`), and `game_hooks.c`. `game_hooks` starts only when
`/data/ghostpad/runtime-policy.ini` contains `ENABLE_GAME_HOOKS=1` (`gc_main.c:2155`); the default policy
enables only the WebUI (`runtime_policy.c:29`), and the hook worker's own vibration default is off
(`game_hooks.c:48`). A virtual DualSense created with `scePadVirtualDeviceAddDevice` has no reverse
channel to its creator, so with default settings a game's `scePadSetVibration`/haptics cannot reach the
Steam Controller at all.

This is the most plausible explanation for "[Bug] No vibration/haptics in Grand Theft Auto VI" (issue #10)
on a default install. It is not a regression from 1.0.0 Experimental: it is the architecture plus missing
documentation. README/feature table mentions "haptic translation" and "game-hook routing" but never states
that in-game feedback requires an opt-in policy file and a game-process hook.

Recommendation: document the limitation prominently in README/LOADING/issue reply, and either (a) surface
the game-hook opt-in in the console UI with the required restart, or (b) provide a supported feedback path
that does not patch game processes. Do not claim in-game rumble without one of these.

### AUD-03 — pre-Add refusal latches creation off (medium)

`create_vda_for_slot()` returns `-1` before `scePadVirtualDeviceAddDevice` when the bounded log reader is
unavailable, the observation window/clock fails, or a rest/wake transition races the call between the
attach gate and this check (`gc_main.c:686-708`). Steady-state pause does not reach here because
`sc2_vda_attach()` checks it first; the practical trigger is `/dev/klog` plus the loopback fallback both
being unavailable.
`slot_lifecycle_attach()` passes that `-1` to `slot_registry_finish_virtual_create()`; `finish_create()`
treats any `h<=0` as "Add may have had side effects" and sets `virtual_blocked` plus registry-wide
`creation_blocked` (`slot_registry.c:339-345`). Nothing clears either flag. Consequences: all later
attachment attempts return `BUSY`; after a logout epoch the owned working pad is deleted and cannot be
recreated, while `inject_pad()` suppresses input through `recovery.pending`.

Recommendation: distinguish "not attempted" from "ambiguous receipt" (new create result and a registry
abort that clears `virtual_creating` without blocking). Keep the latch only for the `0x803b0006`
correlation path.

### AUD-04 — OUT endpoint retirement is sticky but only the IN bit is watched (medium)

`usb_send_out()` refuses any transfer whose endpoint bit is in
`active|completed|retired` (`usb_helpers.c:43-45`); a failed START/COMPLETE/STOP retires the bit
(`usb_resources.c:122`). The reader loop only reinitializes on `retired_transfers & 1` (IN endpoint 0,
`gc_main.c:1121,1392`). The haptics OUT endpoint is index 1. Therefore one failed haptic OUT transfer
silently disables all later haptics for the rest of the session while input keeps flowing; haptics still
reports "available" and the portal answers "queued". `VERIFIED_CODE`.

The same class of failure produces the tester's `haptics: method 2 ... result -5`: method 2 is the
`SET_REPORT` control transfer (`sc2_haptics.c:188-190`), while method 1 (interrupt OUT) succeeded on the
same console, so the report layout itself is fine on the wire.

### AUD-05 — stale temp file blocks saves (medium, reproduced)

`store_save_locked()` uses a fixed `<id>.ini.tmp` with `O_EXCL` and only removes it on paths where its own
`openat` succeeded (`sc2_profile.c:482,503,512`). A crash between `openat` and `renameat` leaves the temp
behind; every later save of that profile returns `-1`, including `sc2_store_recover_default()` when
`default.ini.tmp` is the stale file. `tests/test_sc2_profile_store.c:216-218` deliberately proves that a
pre-existing path at the temp name makes the save fail.

Reproduction executed on this host (no repository changes):

```sh
clang -std=c11 -O1 -Ipayload -DSC2_PROFILE_TEST -DSC2_DIR='"/tmp/scps5audit/profiles"' \
  -o /tmp/scps5audit/tmp_stale /tmp/scps5audit/tmp_stale.c /tmp/scps5audit/stubs.c \
  payload/sc2_profile.c payload/runtime_control.c -pthread -lm
/tmp/scps5audit/tmp_stale
# save with stale .tmp: -1
# save after manual cleanup: 0
```

Recommendation: under the existing store lock, treat `EEXIST` on the temp as stale (verify regular,
owned, non-symlink, then unlink and retry once), and `fsync` the directory after `renameat`.

### AUD-06 — unbounded shutdown (medium)

`main()` retries `runtime_shutdown()` forever with `sleep(1)` (`gc_main.c:2194`). `runtime_shutdown()`
joins the WebUI (`webui.c:684` joins the tile worker) and the DualSense hand-off worker
(`ds_handoff.c` joins a `waitpid`-driven helper) with no timeout. A blocked `sceAppInstUtil*` call or a
stuck `ptrace`/`waitpid` prevents neutralization, VDA deletion, credential restore and PID-file removal
indefinitely; only SIGKILL ends the process, leaving the PID file behind.

Recommendation: log the owner before each blocking phase, bound each join, then restore credentials and
remove the PID file even on a partial cleanup, reporting a distinct exit status.

### AUD-07 — sanitizer gate cannot fail on UB (medium, verified)

`tests/Makefile:92` passes `-fsanitize=address,undefined` without `-fno-sanitize-recover`. Verified on
this host with Apple clang 21: a signed-overflow program built that way prints
`runtime error: signed integer overflow` and exits 0; with `-fno-sanitize-recover=all` it aborts (134).
ASan still aborts on memory errors, but the "ASan/UBSan PASS" statement in the docs overstates what the
gate proves.

Recommendation: add `-fno-sanitize-recover=all` and `UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1`
to the sanitize target (and to the DEV workflow invocation) so UB fails the job.

### AUD-08 — foreign tags and release workflow (medium)

`release.yml` triggers on `push: tags: ['v*']`. The checkout still contains seven inherited local tags
(`v0.1.0`, `v0.1.1`, `v1.0.4-hotfix1`, `v1.1.0`–`v1.4.0`) that point at upstream Puckbridge history and
are absent from `origin`. A `git push --tags` would push those commits and run the workflow file from
each tagged commit with this repository's token; the upstream file is not the hardened one in this tree.

Recommendation: delete the foreign local tags (already offered to the owner), and add a guard job that
requires the tagged commit to be an ancestor of `main` before any publication step.

### AUD-09 — duplicate launch destroys the live log (medium)

`ghostpad_status_log_reset()` runs before `runtime_instance_acquire()` (`gc_main.c:2085` vs `:2103`).
The second launch renames `gc_status.log` to `.prev`, truncates and reopens the log, then refuses
startup. The running instance keeps writing to the new file, and the WebUI log/diagnostics views now show
the aborted second run instead of history. Recommendation: create the directory first, acquire the PID
lock, and only then rotate/open the log.

### AUD-10 — bridge report privacy (medium, opt-in feature)

`bridge_probe.c:73-76` copies `/data/poords4/game-pad-bridge-last.txt` to
`/data/ghostpad/bridge-probe-<title>.txt` with `fopen("w")` and no filtering. The source report starts
with `firmware=... pid=... user_id=0x...` (`wireless_ds4.c:5035`). `ARCHITECTURE.md:178` claims optional
bridge reports pass the same filter and expose booleans rather than account IDs. Bridge probing is off by
default, but the claim is false when enabled. Recommendation: write through the private-file helpers
(0600, no-follow) and whitelist fields.

### AUD-11 — kernel capabilities held for the whole runtime (medium, security)

`elevate_credentials()` sets the maximum capability mask and a kernel authid and restores them only at
shutdown (`gc_main.c:1917-1923,2059`). The unauthenticated loopback WebUI, the USB parsers and the tile
installer then run in a process that can perform kernel reads/writes. No exploit was found in the HTTP
layer (its parsing is well bounded), but the blast radius of any future memory-safety bug is the kernel.
Recommendation: keep elevation only around the specific kernel calls, or drop capabilities after startup
and re-elevate per call with the existing `credential_guard_run` helper.

### AUD-12/13 — activator edge cases (low)

- `seed_down_state()` sets `t_down = now` for held inputs, and the long branch fires on held state alone
  (`sc2_binding.c:25-30,131`). Every profile generation change (save, override, game change) therefore
  re-arms a held long-press and fires it again after `long_ms`.
- For a plain toggle, the output uses the live `base`, not `state->input[i].toggle_mask`
  (`sc2_binding.c:164`), so pressing Shift while a toggle is latched changes its emulated button.

### AUD-14/15/16 — smaller issues

- Status log: `mkdir(LOG_DIR,0755)` + `rename` + `open` without `O_NOFOLLOW`/owner checks
  (`gc_main.c:100-102`), unlike `private_file.h`.
- `loopback_host()` accepts `Host` without a port (`webui.c:186`) although `ARCHITECTURE.md:112` says
  the bound port is required. The port comparison itself is correct for explicit ports.
- A WebUI `via=1..4` test changes the process-wide haptic transport (`webui.c:341`); methods 1 and 4 do
  not fall back to `SET_REPORT`, and availability is advertised even when no OUT endpoint opened
  (`sc2_haptics.c:179-190`, `gc_main.c:1352`). A failed test run can therefore black-hole later
  game-hook haptics.

### AUD-17/18 — supply chain and dead code

- Publication steps use `softprops/action-gh-release@v3` and other major-version action tags with
  `contents: write`. Pin to full commit SHAs.
- The manager probe accepts only SC2 VID/PID, and the reader takes the SC2 branch; the legacy Xbox,
  Nintendo and 2015 Steam adapters are unreachable, as is the eager-VDA/assignment-lease path
  (`gc_main.c:594,1358,1828`). `ARCHITECTURE.md:68,94` still presents those as active. Either delete them
  or label them explicitly as unreachable scaffolding.

## Verified non-issues (checked, not defects)

- `steam_feedback_encode_rumble()` leaves the right gain byte zero on purpose: Linux `hid-steam.c`
  calls `steam_haptic_rumble(steam, 0, left, right, 2, 0)`, i.e. left gain 2, right gain 0. The host test
  enshrines the same bytes. No change needed without hardware evidence to the contrary.
- `webui_http.c` framing: one absolute deadline for headers+body, 4 KiB header/32 KiB body caps,
  duplicate Host/Origin/Content-Length/Content-Type and `Transfer-Encoding` rejected, no GET body,
  CRLF-only headers, origin must equal `http://`+Host, loopback-only bind plus 127/8 peer filter, strict
  route allowlists and icon id validation. No exploitable defect found in this layer.
- USB descriptor parsing, endpoint-map consistency, transfer ownership and cancellation retirement are
  covered by host fakes and fail closed; the remaining risk is the real ioctl behavior, which is
  hardware-unverified.
- Slot registry generation tokens, delete-failure retention, alias/handle collision rejection and the
  native-disconnected pre/post-Add checks are implemented and host-tested.
- Profile parser bounds, numeric validation and atomic replace (apart from the stale temp) are sound.

## Hardware gates that remain

Nothing in this audit changes any NOT_RUN hardware row. The FW 13.60 console log used above remains
`EXTERNAL_HARDWARE` for the published ELF; it confirms the tile failure and that Puck detection, VDA
event correlation and VDI injection ran, but it does not establish game input, motion/touch quality,
native DualSense coexistence or cleanup behavior.
