# External runtime failure analysis — 2026-10-15

**Historical external dev.4 failure analysis.** For the current published **dev.6**, use the [README](../../README.md), [loading guide](LOADING.md), [current status](../../project-memory/CURRENT_STATUS.md) and [VDA/storage contract](VDA_PENDING.md). Earlier holds, positive-only VDA policies, counts and source hashes below describe their dated checkpoints. Dev.6 includes host-tested token initialization and bounded VDA observer fixes; no physical PS5 result is established. Historical results and releases are unchanged.

M1-244 · source comparison: main `aa9d4fa881b13c101edd1965baac7cbeb8b79655` / published dev.4 `a5c20346392eac0eb9fa04cc201cd214e2957363` · SDK v0.43

## Evidence and scope

`EXTERNAL_HARDWARE`: [public Gist metadata](https://api.github.com/gists/cd25c38c5b735e3ce1808e7f2993e38e), revision `3c449292c5fdbea978d9a5df93fc795e7dbe6d38`, updated2026-10-15T10:03:54Z. The supplied comment identifies Payload Manager v0.5.1. Earlier report says FAT/FW5.50; exact installed firmware is not recorded by either log and has not been independently measured. The ELF checksum/source SHA is not supplied; the new banner says dev.4 and its latch message matches that source. Do not treat this as a cryptographically verified release binary or a passed hardware gate.

- `gc_status.log`: complete7572 bytes, SHA256 `2fb2ab8f6bf6da68629fcf3c023f40ff994ad2ae0094e269f058ddac32949e66`.
- `gc_status.log.prev`: complete1990922 bytes/36188 lines, SHA256 `4dc426dc9539848c9a1fd86bae5fae7fcd4c14ee8f29b2ac50c7dab2db432e63`. Gist API truncates this large file; all counts below use its complete raw file, not the truncated API excerpt. Banner is Ghost-Control v5/4 slots; exact historical release/source remains unidentified.
- Original logs are kept outside Git. They contain local account/device identifiers; only sanitized findings/counts belong in project documentation. No token or pairing-code values were found by the targeted inspection; this is not a comprehensive secret guarantee.

## What the new trace establishes

| Stage | Observed result | Limit |
| --- | --- | --- |
| Safe policy | bridge=0, hooks=0, dualsense=0, webui=1 | Passive hand-off-labelled klog lines do not prove disconnection or a system patch |
| User/Pad services | Initialize, foreground/list, PadInit and process privilege return0; initial-user lookup fails | No proof of correct account assignment; do not bind another account automatically |
| Steam/Puck | Valve28de:1304 detected; interfaces2–5/IN83–86 and OUT02–05; controller active on83 | OUT opens do not prove working feedback |
| Input decoder | Battery, button activity and repeated gyro calibration appear | Evidence of received/decoded input, not successful game/system injection or validated sensor orientation |
| VDA creation | Exactly one Add result803b0006; creation inhibited; no ready handle | Backend cannot complete virtual-pad ownership on this observed path |
| System events | A VDA-style DEVICE_ADDED event is logged after Add | Temporal correlation does not establish exclusive ownership or a deletable handle |
| WebUI | Private-token initialization fails before listener/pairing setup | Log contains neither failing storage operation nor errno; cause unresolved |
| Default profile | Load fails and safe defaults are selected | Message does not distinguish malformed data, access failure or unsupported filesystem semantics |

The direct no-input path is visible in `payload/gc_main.c:460–478`: only a positive Add return becomes an owned handle. `inject_pad()` requires virtual_ready. The observed negative result therefore leaves input decoded but not delivered to a virtual pad. This is an incomplete PS5 backend contract, not evidence that Puck input parsing is absent.

Pinned LibrePad source `28a3bfefe19cbcdbacb94a9ba911cb01ebc6e0fc`, `src/ps5/virtual_controller.c:519–590`, treats803b0006 as pending assignment and examines output parameters/klog before MBus binding. That is a source interpretation, not a verified ABI on this console. Its fallback is not imported: an asynchronous event alone cannot prove this payload owns a device while other system/payload activity is possible. Our current positive-only contract remains incomplete for the supplied runtime trace; the safety latch is containment, not restored controller functionality.

## Previous-run retry storm

The complete prior log contains11975 Add attempts:14 return803b0006, then11961 return803b0003. It logs14 VDA identifiers and no owned-handle-ready/VDI progress. The new trace contains one Add attempt followed by continued input/heartbeat, consistent with dev.4 inhibiting further creation. These are external logged observations, not a controlled same-environment A/B test.

Resource exhaustion is a strong hypothesis for the reported device-limit/profile-picker problem, but exact return semantics, event ownership and causal connection to lost native DualSense input are unproven. Neither log contains a Kernel Panic trace. Continued heartbeats do not establish system UI responsiveness or safe shutdown.

## Storage and logging follow-up

`webui_start()` at `payload/webui.c:595–601` collapses all token setup failures into one notification before network listener setup. `webui_auth.c` checks directory ownership/0700, token regular-file ownership/0600, exact format, randomness, openat/fchmod/fsync and I/O. The logs cannot select a failing operation. Preserve these checks; do not chmod777, delete/migrate existing storage, generate weak tokens or enable anonymous APIs as a workaround.

`ds_handoff_klog_line()` copies a bounded sample of system events even when active hand-off is disabled; these samples can contain account IDs. Treat hand-off-labelled messages as passive logging unless an actual operation is recorded. Future logging must gate/redact these samples. Existing diagnostic-report redaction does not sanitize raw gc_status.log files already shared.

## Required next work and release status

1. Resolve P0-VDA-PENDING-04: establish the actual Add output/handle/assignment contract from pinned public source or an explicitly approved controlled console capture. Extend the existing lifecycle for a proven pending receipt; no duplicate Add, guessed native handle or unowned cleanup. Until ownership is proven, keep the latch and do not ask the tester to repeat the failing payload.
2. Resolve P1-STORAGE-DIAG-04: add bounded, secret-free storage stage/error diagnostics and fault regressions before changing filesystem behavior. Cover profile rejection as well as token initialization; identify the actual console error before proposing recovery.
3. Resolve P2-KLOG-PRIVACY-04: suppress unnecessary hand-off observers in Safe Mode and redact system account identifiers at raw-log ingress, with host coverage.
4. Obtain the already-used ELF SHA256 and confirm installed FW/loader. Optional read-only owner/mode/size metadata may help; never request webui.token contents, the whole settings directory or another uncontrolled run.

Future DEV publication is blocked in dev-release.json until the unresolved VDA ownership defect is addressed. Published dev.4/older artifacts and historical results are unchanged. No new build, release, radio operation, app installation or console action was performed. The source can be reviewed publicly with this failure clearly documented; physical compatibility and stable acceptance remain blocked.
