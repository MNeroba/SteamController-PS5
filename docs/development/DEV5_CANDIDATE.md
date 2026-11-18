# dev.5 — published release and preparation history

**Published experimental dev.5:** [dev/v0.1.0-dev.5](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v0.1.0-dev.5), source `382e4ecef4937356b72f9e1786917ae7147c0415`. [DEV workflow38810046681](https://github.com/MNeroba/SteamController-PS5/actions/runs/38810046681) completed successfully; release414294352 is a prerelease, not a draft, published **2026-10-15 21:30:53 UTC** (16 October in Riga). Seven assets contain exactly one ELF, **2,084,968 bytes**, SHA-256 `01a73f3f676121061f58b52a833c642d8cd94b048fbbc524182b8756eb642bd0`. Host48 C/Python, ASan/UBSan, Chromium220, deterministic assets and clean SDKv0.43/LLVM18 gates passed on that selected source. Physical Add/VDI/MBus, native DualSense coexistence, firmware compatibility and actual storage errno remain unverified. Event correlation is an owner-approved temporary experimental mechanism, not proof of exclusive ownership. Previous releases/tags/assets are unchanged.

**Preparation history:** this stable document path retains candidate checkpoints below; their old publication holds and local binary hashes are historical, not the status or checksum of the released ELF.

Owner explicitly authorized the analogous event-correlation approach for this stage and then dev.5 publication for the log author's test. This supersedes M1-245's withheld-publication instruction below. Publication acknowledgement is recorded above.

Implemented: one asynchronous pending Add/registry reservation; bounded fresh virtual-event identification, cancellation/release/failed-delete retention; Steam/PS-triggered local MBus with connected/unknown native protection and picker fallback; queued short PS action; typed secret-free diagnostics; operational-storage fallback to a new session-only CSPRNG credential. No Shell patches, native disconnect/unbind, storage migration, radio changes or second ELF. Actual Add/VDI/MBus contract and physical coexistence remain NOT_RUN; correlation across other payloads is an explicitly accepted experimental limitation. See [contract and tester checklist](VDA_PENDING.md).

Final local and exact-source CI/release receipts are appended to project-memory/TEST_LOG.md and SESSION_LOG.md. Earlier counts/checksums below describe the M1-245 binary only, not this updated candidate.

---

## Historical M1-245 checkpoint

# dev.5 candidate — preparation only

2026-10-15 · M1-245 · SDK v0.43 · owner explicitly withheld publication

## Implemented and host-verified

- Protected storage reports fixed operation stages and preserved errno through close/unlink cleanup. No credentials, account IDs, file contents or arbitrary paths enter these reports.
- An already owned directory with exact0700 and a newly created credential with exact0600 do not require redundant fchmod. Ownership/type/no-follow/stat and permission checks remain; insecure or malformed existing credentials are never overwritten. Random and textual scratch/output buffers are erased on failure.
- Raw application logs redact known user/account identity fields before file and kernel-log sinks. Safe Mode no longer starts the optional system-log observer; disabled hand-off ignores events. Optional bridge text reports are filtered and their client table records user-match booleans instead of account numbers. Previously shared logs/Gists are not modified; arbitrary opt-in binary memory captures are not certified sanitized.
- VDA result classification distinguishes803b0006 assignment pending, missing receipt and other errors. The native call retains its existing zeroed input layout. Diagnostics record only a changed-word bitmask, not output values. Neither output words nor klog/native handles become ownership receipts. One Add/reservation/lifetime latch and all existing generation/cleanup rules remain.

## VDA remains blocked

**P0-VDA-PENDING-04 is not resolved. This candidate does not restore PS5 input on the supplied pending-assignment trace.** It must not be represented as a fix for the tester's system UI/DualSense failure.

Reviewed unchanged pinned LibrePad28a3bfefe19cbcdbacb94a9ba911cb01ebc6e0fc virtual_controller.c:519–590, storage.c and app/main.c:52–64; OmniPadb7682efce18ee0a8d7c80bbc90afbce72151ef96 ps5_vpad.c and firmware guide; Puckbridge78460113ef7c48502c765a1f83ba34d53c977134. LibrePad preserves storage error stage/errno; its storage migration is not adopted. Analogous VDA paths depend on klog identity, MBus assignment or Shell process interventions. A bounded event window is not proof that a device belongs to this payload when other system activity exists. Public export stubs and unreversed Pad declarations do not establish the needed output ABI.

AGENTS.md requires unambiguous ownership and prohibits unknown ABI calls with side effects and unapproved system writes. No safe, source-proven replacement was identified. Keep this blocker rather than clear it after a successful host build. Next work requires a justified Add output/receipt/assignment contract or separately approved controlled hardware evidence; retain native DualSense and forbid blind handle scans, repeated Add and foreign-device deletion. No such console operation has been performed.

## Verification

- Normal host46 C suites/all Python contracts: PASS.
- ASan/UBSan complete repeat: PASS.
- Chromium220 checks: PASS.
- Storage fault suite1226 assertions; VDA result/latch23; raw-log integration4; HTTP/auth151; diagnostics bounds/redaction/concurrency: PASS.
- Clean SDKv0.43/LLVM18 cross-build45 C and optional installer dependency guard: PASS.
- Deterministic WebUI/tile embedding and Clang static analysis of auth/diagnostics: PASS.
- Actual DEV packager with the real critical manifest: expected rejection before asset creation. No release policy bypass.
- Physical PS5/storage/USB/VDA/user assignment/native DualSense/Settings tile: NOT_RUN. No known FW compatibility claimed.

Exact commands, logs, candidate source identity and local ELF checksum are recorded in project-memory/TEST_LOG.md. Source changes may be pushed for review under the existing direct-main policy. No dev.5 release/tag/workflow dispatch is authorized. Historical dev.1–dev.4 assets/tags remain unchanged; the candidate metadata URL is a future location, not an available download.


## Exact-source candidate receipt

Source [`05587f209d8171d0e9492ce87bb6224ece3c8b63`](https://github.com/MNeroba/SteamController-PS5/commit/05587f209d8171d0e9492ce87bb6224ece3c8b63), complete294-file treecea4b15351d7d4435bdc86d508f09eb2486b4cf3. [Host38805088725](https://github.com/MNeroba/SteamController-PS5/actions/runs/38805088725) and [SDK38805088852](https://github.com/MNeroba/SteamController-PS5/actions/runs/38805088852) completed/success. Fresh Git-SHA-verified export repeats normal/sanitizer/SDK/dependency successfully. Local single ELF2047056 bytes; SHA256 `72e8bc941809ece3f7a693e4a3ea2a39c5a82d47951b4122ce4e09e910dd9841`; selected source/version and exact tile icon/metadata verified embedded. Local binary and checksum are in ignored output/dev5-candidate with BLOCKED status. This is not a released or hardware-validated binary; no dev.5 GitHub Release/tag exists. The actual packager remains blocked by P0-VDA-PENDING-04.
