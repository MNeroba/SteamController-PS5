# Pending virtual-controller registration — dev.7

2026-11-01 · M1-250 · dev.7 experimental; maintainer physical PS5 acceptance NOT_RUN

## dev.7 correction (2026-11-01)

PR8 author TheBrokenG0d reports WebUI and sustained Puck/system-menu input on one fat PS5/FW13.20 at source c3a7ddb. This is EXTERNAL_HARDWARE, not a hardware test of the revised dev.7 adapter; native DualSense coexistence and games were not systematically tested. SDKv0.43 defines **EOVERFLOW=84 and EILSEQ=86**. The older diagnosis of tester errno84 as EILSEQ was incorrect; historical records below are preserved. The prior drain itself synthesized EOVERFLOW after64 reads. The corrected drain remains nonblocking and refuses Add when its larger bound cannot reach EAGAIN.

The SDK's filesystem stubs omit POSIX error translation. The revised target adapter captures the syscall error flag separately from the returned value, preserving the existing credential/profile/tile security checks and actual fd ownership. Its host and SDK evidence does not establish console behavior. [Detailed review, regressions and limits](../../project-memory/PR8_REVIEW.md).

## Contract and evidence

The actual PS5 `AddDevice` output/return ABI is **not established** by a host build. The inherited call still supplies exactly32 bytes: size32, creation word1, six zero words, DualSense type3. Positive return ownership remains inherited and unverified on PS5. Output words are observed only as a changed-word bitmask; there are no sentinel writes or output-word handle guesses.

The external dev.4 trace returns `0x803b0006`, then logs a typed virtual-device addition while USB input continues. This is EXTERNAL_HARDWARE evidence, not our controlled execution. LibrePadPS5 [28a3bfefe19cbcdbacb94a9ba911cb01ebc6e0fc, virtual_controller.c:519–590](https://github.com/arwebSE/LibrePadPS5/blob/28a3bfefe19cbcdbacb94a9ba911cb01ebc6e0fc/src/ps5/virtual_controller.c#L519) uses a unique klog event and the low32 bits of its device ID as VDI/delete handle. Its local MBus prototype is at :43/:338. These are VERIFIED_CODE / UPSTREAM_CLAIM, not independent proof of this firmware's ABI.

The owner explicitly requested the analogous approach for this stage, deferring a stronger ownership mechanism, then authorized dev.5 publication for the log author's test. This supersedes the earlier positive-only policy for this experimental path. **Temporal correlation cannot prove exclusive ownership across unrelated payloads. Keep other virtual-controller payloads stopped.** A future physical button challenge is a proposal, not an implemented ownership proof.

## Implemented flow

1. Reserve creation in the existing generation-tagged slot registry before any Add. Open a dedicated nonblocking `/dev/klog` or bounded loopback3232 stream. Drain at most4096 nonblocking 1 KiB reads (4 MiB bound) to EAGAIN; an incomplete/unavailable backlog refuses Add. When the optional handoff observer is active, only an independent loopback stream is permitted.
2. Refresh the monotonic observation timestamp after the drain, preserving the partial-line guard, then perform Add once. Clock/arming failure refuses Add. A positive direct return retains the existing path. `0x803b0006` leaves the reservation pending; the USB owner continues input, motion and feedback without a3-second blocking wait or reader restart.
3. Observe a full3-second minimum window, bounded to4KiB per poll and1024 bytes per line. Require exact LoginMgr DEVICE_ADDED fields: type1, subtype22, battery capability0, a valid uint64 ID and positive signed low32. Duplicate observations of the same ID count as one candidate; different IDs, malformed/truncated data, clock/read/close failure or unread backlog at completion refuse correlation. Known physical DualSense events (battery1) are excluded; contradictory native/virtual IDs refuse correlation.
4. Fresh user-list validation and existing native-handle comparisons can reject a candidate, never supply one. Store an accepted event receipt in the existing registry with `virtual_correlated`, distinct from a direct Add receipt. The assumed ID-to-handle relationship remains HYPOTHESIS pending console validation.
5. Cancel/pause/confirmed logout/release/disconnect finishes identification for cleanup. A cancelled receipt is marked retiring before it can receive input. Failed deletion retains ownership and blocks a replacement; manager retries cleanup. Unresolved identification keeps the lifetime no-repeat latch and never deletes a guessed device. An unidentified orphan cannot be safely cleaned automatically.
6. A real Steam/PS action during pending registration is retained as a bounded150ms PS pulse once ready. Account selection normally uses the PS5 picker. Local `sceMbusBindDeviceWithUserId` is attempted only on a Steam/PS edge, for exactly one current foreground user and successfully read disconnected native state. Selection/native state is rechecked after symbol loading. Connected/unknown native state, multiple accounts or missing symbols use the picker. There are no native disconnect/unbind calls, ShellUI/ShellCore patches, handle scans or new privilege changes.

The native-state observation and MBus call are not atomic: physical coexistence/selection must still be checked on PS5. No zero-Kernel-Panic or firmware-compatibility guarantee is made.

## WebUI and storage

Existing checked private storage remains preferred. Operational failures such as ENOSPC, EROFS or unsupported permission operations may use a new256-bit CSPRNG token held only in RAM. The existing notification code, five-minute/five-attempt/one-redemption pairing, bearer authentication, Origin/Host and LAN gates remain. Token changes on restart; filesystem recovery-token access is unavailable in this mode. Notification and diagnostic status2 identify session-only mode. Profile persistence may still fail and the UI must not report success.

Malformed, foreign, permissive, symlink or unreadable existing credentials remain fatal; entropy failure never enables access. No unsafe file is overwritten or storage migrated. Reports preserve the failed storage stage/errno and bounded VDA counts/status, without account/device IDs or credentials. The tester's actual storage errno remains unknown until a new console report is received; injected ENOSPC28 is a host test, not a diagnosis of that console.

## Controlled tester checklist

Use the exact release ELF/checksum and retain native DualSense. Start from a known responsive console with other virtual-controller payloads stopped; do not repeatedly reload a pending/failing build. Record measured firmware/model, loader, source/ELF identity and USB-C/Puck transport without account IDs or serials.

Check once: detected Steam input → one Add/pending → correlated virtual ready → Steam/PS → chosen profile → released buttons/sticks in the UI. Verify native DualSense before/during/after. Stop on unexpected native input loss or an unresponsive picker; follow the loader's established recovery procedure rather than continuing experiments. Then verify pause/resume, orderly stop, and one reconnect on each available transport. No radio, hook or tile experiment is needed for these tests.

Pair WebUI using its console notification; if session-only, download Settings & help → Diagnostics → Download Diagnostic Report before stopping. Review before sharing. If WebUI is unavailable, share only sanitized relevant application-log lines with the stage/errno and VDA results; never publish tokens, pairing codes or account IDs. A successful compile or host test does not replace this checklist.

## 2026-10-31 — dev.5 tester logs and dev.6 follow-up (published)

The tester's updated dev.5 logs show protected WebUI storage failing at `file-inspect` with `EINVAL` (22). The code reached this stage after a no-follow token open reported absence, then used `fstatat(..., AT_SYMLINK_NOFOLLOW)` to inspect the same path. An unsupported flag on the console filesystem is a plausible cause, but the actual filesystem behavior is not proven by the log. The dev.6 implementation treats only `ENOENT` from the existing `openat(..., O_NOFOLLOW)` as absence and creates with `O_EXCL|O_NOFOLLOW`; symlink, malformed, foreign, inaccessible and racing files remain fail-closed. A host regression injects `EINVAL` for any `fstatat` call and verifies safe token creation without using that call.

The same logs show Steam Controller USB/Puck input active, while the fresh VDA log reader returned `EILSEQ` before `scePadVirtualDeviceAddDevice()` was called. The log does not identify which reader source returned the error, so it does not establish an Add/VDA result. The published dev.6 build labels the selected source and retries exactly once through the existing loopback reader only when `/dev/klog` itself returns `EILSEQ`. It drains the new stream to `EAGAIN` before Add; any other error, failed retry or incomplete drain still prevents Add. A host-injected `EILSEQ` regression verifies this bounded recovery policy. No ownership or firmware claim is added.

The token-initialization and bounded log-reader fixes were included in the published dev.6 ELF from source `4c8b89bc789bf1dfd064264ff432d3919b5a1899`. Exact-source Host, SDK and secret scan passed before publication; the full repeated DEV workflow [41246199948](https://github.com/MNeroba/SteamController-PS5/actions/runs/41246199948) also passed. The earlier dev.5 tester logs do not test these fixes. Documentation descendant `4bd783c1b8dc1082942b85a7fcee72a0e0c45632` also passed Host [41245148448](https://github.com/MNeroba/SteamController-PS5/actions/runs/41245148448) and SDK [41245148430](https://github.com/MNeroba/SteamController-PS5/actions/runs/41245148430).

Dev.6 carries these host-verified source fixes and contains no additional VDA ownership or native DualSense behavior change. The old tester logs do not show whether `/dev/klog` or loopback was selected, and the target filesystem cause for `EINVAL` remains a hypothesis. Published dev.6 remains host/build verified only. The observer retry does not resolve the VDA AddDevice ownership ABI or establish native DualSense coexistence; physical PS5 validation is NOT_RUN. The SDK/browser bind limitation from this desktop environment is retained in project memory; physical PS5 verification is NOT_RUN.
