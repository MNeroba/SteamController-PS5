# Codex session handoff log

## Initial documentation snapshot — 2026-10-08

- Work performed: consolidated user goal, previous discussions, current public repository sources and BLE research into final Codex specification and support docs.
- Repository/fork: NOT_CREATED for new project by this package.
- Commits/PRs: NONE created here.
- Implementation/tests: NONE; consult `TEST_LOG.md` for explicit NOT_RUN records.
- Next exact action: read all docs; clone/pin Puckbridge and OmniPad; fill ADR-001 and PROVENANCE; attempt pristine build and record real results; audit P0 security before new feature work.
- Constraints: Native PS5 BLE 13.60 not demonstrated; never claim Sony Partner SDK has generic BLE API; no physical console access has been used in preparing these documents.

For each subsequent session append: UTC timestamp; repo/branch/HEAD SHA; files changed; test IDs and actual results; current blocker; what source/ABI changed; next exact command/step. Do not remove prior entries.

## M0 source audit and foundation import — 2026-10-08

- Work: read the project specification and project docs/memory; cloned and inspected PuckbridgePS5 and OmniPad-PS5 source snapshots; selected Puckbridge; copied its source files into the workspace while preserving the starter README at `docs/PROJECT_STARTER_README.md`.
- Source pins: Puckbridge `78460113ef7c48502c765a1f83ba34d53c977134`; OmniPad `b7682efce18ee0a8d7c80bbc90afbce72151ef96`.
- Findings: Puckbridge has the Steam Controller path but unauthenticated all-interface web controls; `controller_sc2.c` couples parser/mapping/PS5 output; OmniPad suite is incomplete. See F010-F016 and P0 issues.
- Validation: OmniPad baseline `make test` ran in the isolated audit clone: three available test executables passed; Makefile fails on missing `tests/test_normalization.c` (exit 2). Puckbridge build fails before compile because `PS5_PAYLOAD_SDK` is unset. No Puckbridge host tests were found.
- Blockers: no physical PS5/controller/Puck; no SDK; workspace `.git` is read-only and source `.git` copy was denied; GitHub authentication is invalid.
- Next: implement web authentication and strict request parsing with host regression tests; then separate Steam report decoding from PS5 state mapping.

## M1 P0 hardening and reusable parser work — 2026-10-08

- Work: added private bearer-token provisioning and API authorization to the portal; replaced loose HTTP reads with strict bounded header/body framing; added pure Steam report and USB configuration parsers and wired both into the selected controller path; changed gyro/motion code to consume typed IMU state.
- Files: `payload/webui.c`, `webui_auth.*`, `webui_http.*`, `steam_protocol.*`, `steam_usb.*`, `controller_sc2.*`, `sc2_gyro.*`, `payload/Makefile`, `payload/web/index.html`, generated `webui_html.h`, host tests, CI workflow, manuals and project memory.
- Validation: `make -C tests clean sanitize` passes 179 ASan/UBSan assertions across portal HTTP/auth/token lifecycle, Steam report decoding, and USB endpoint descriptor parsing. Earlier Puckbridge build remains blocked before compile because `PS5_PAYLOAD_SDK` is unset.
- Security boundary: bearer token is stored at `/data/ghostpad/webui.token`; the directory is tightened to 0700 and file must be owner-only. The portal remains plain HTTP, so same-network active interception is still possible; README restricts use to a trusted LAN.
- Source behavior: Steam input now decodes into typed controls/axes/touch/IMU before the existing profile and PS5 mapping layers; USB descriptor walkers now share one bounds-checked parser. These changes are host tested at the pure-module level, not at PS5 integration/runtime level.
- Current blockers: no writable Git metadata yet, invalid GitHub token/repository visibility, no SDK, and no physical PS5/Steam Controller/Puck. Native BLE remains research-only; no radio commands were run.
- Next: audit slot/handoff and game-hook rollback paths, bootstrap Git history, extend host tests for mapping/session/feedback, and request no new physical hardware work unless available.

## M1 P0 audit and repo bootstrap continuation — 2026-10-08 12:47 UTC

- Repo: branch `fix/p0-safety`, `HEAD=78460113ef7c48502c765a1f83ba34d53c977134`; no implementation commit created. Puckbridge's pinned `.git` metadata was copied into the writable project after the initial sandbox denial. `origin` is the user-supplied `https://github.com/MNeroba/SteamController-PS5.git`; Puckbridge and OmniPad reference remotes are fetch-only.
- Changes: VDA creation is serialized around the shared klog identity queue; active USB endpoints now require a complete decoded Steam input frame; malformed known input is classified and mapped to neutral pad state; game hooks default off and installed hooks are removed on title changes; profile watcher, hooks, bridge probe, web server, klog and manager thread start failures are logged.
- Tests: `make -C tests clean sanitize` PASS, 266 ASan/UBSan checks (57 HTTP/auth, 19 USB descriptors, 190 Steam protocol). `git diff --check` PASS. Generated `payload/webui_html.h` regenerated and byte-decoded to confirm the token prompt and Authorization header are present.
- Build: `make -C payload` BLOCKED (exit 2) before compilation because `PS5_PAYLOAD_SDK` is unset; search under `/Users/mneroba/GitHub` found no `prospero.mk`.
- Issues: game title/settings rollback improved; process-exit/payload-stop cleanup and generation IDs remain. HID usage-page validation lacks a verified target API/descriptor capture. No actual USB, touch, motion, haptics, PS5, DualSense, Puck or BLE test occurred.
- Publication blocker: `gh auth status` says the MNeroba token is invalid; supplied repo URL returns 404 unauthenticated. Git author name/email are unset, so changes remain uncommitted. No PR was created.
- Next: restore GitHub credentials/identity; finish portable mapping/motion/haptics tests; build with a real SDK; then execute requested hardware plan. Do not run native BLE experiments without evidence gate access.

## M1 portable mapping, motion, feedback and JSON continuation — 2026-10-08 12:55 UTC

- Repo: `/Users/mneroba/GitHub/SteamController-PS5`, branch `fix/p0-safety`, `HEAD=78460113ef7c48502c765a1f83ba34d53c977134`; implementation changes remain uncommitted.
- Changes: added pure Steam axis/trigger/trackpad/d-pad transforms, motion conversion/quaternion integration, bounded haptic report serialization and bounded JSON output; connected mapping/motion/feedback helpers to inherited PS5 adapters and web handlers. Updated test, architecture and README coverage statements.
- Tests: latest `make -C tests clean sanitize` PASS, 344 ASan/UBSan checks on Darwin 27.0.0 arm64 with Apple clang 21.0.0: HTTP/auth 57, JSON 9, USB descriptors 19, protocol 190, mapping 24, motion 25, feedback 20. PS5 adapter files are not compiled by this suite.
- Build/hardware/publication: PS5 SDK remains absent; no physical PS5, DualSense, Steam Controller or Puck is available; native BLE commands were not run. GitHub authentication was previously rejected as invalid and Git author identity is unset; no commit/PR exists.
- Remaining code gates: session/slot generations, profile transition tests, PS5 adapter neutral-state test, game hook process-exit/payload-stop rollback, verified HID usage-page acquisition, PS5 build and USB/Puck/motion/touch/feedback hardware matrix.
- Next exact actions: rerun sanitizer and `git diff --check`; inspect all generated and untracked changes; verify remotes/auth/current repository access; retry `make -C payload` only after locating a real configured SDK; then commit/publish only if valid GitHub credentials and author identity are available. Preserve native BLE as gated research.

## M1 GitHub draft publication — 2026-10-08 13:06 UTC

- Target repo: private `MNeroba/SteamController-PS5`, confirmed through the connected GitHub app with admin permission. It had no branches and size 0. The GitHub CLI token is invalid, so publication used the authorized app connector.
- Bootstrap: initialized `main` with README commit `61780b03bcef6859935cac1ef288cf3eef12d3ba`; created `fix/p0-safety`; published the complete 146-path working-tree snapshot (145 changed files) in commit `4596c7fc4a9876e56cb8194961f15de0fd1e7066`. The target is a derivative repository, not a GitHub fork; Puckbridge's exact SHA and licensing remain recorded, but the remote commit does not carry its Git ancestry.
- PR: opened and attached draft [#1](https://github.com/MNeroba/SteamController-PS5/pull/1), title “Steam Controller foundation: P0 hardening and portable input modules.” GitHub reports it open, draft, and mergeable. Draft status reflects the absent SDK and physical test hardware.
- CI: GitHub Actions Host tests run `37781721767` was queued for the initial source snapshot; after memory synchronization, run `37781980759` was queued at the current PR head `7721f18cde11498c92fe9f6c91aa2a3f7dfbd269`. No conclusion yet. The PR branch memory update is commit `7721f18cde11498c92fe9f6c91aa2a3f7dfbd269`.
- Local sanitizer M1-04 passed 344 checks. A subsequent rerun after the lazy-slot source guard also passed 344 checks; static source assertion confirms both delete-call guards, and `git diff --check` plus publishable-text whitespace scan pass. `gc_main.c` is not compiled by host tests; `make -C payload` remains blocked before compilation by unset `PS5_PAYLOAD_SDK`.
- Safety/provenance: pre-publication scan found no private-key/common GitHub/AWS credential patterns in the 146 publishable files. No native BLE/HCI command or physical controller experiment was run.
- Next: inspect the queued workflow, then publish the lazy-slot guard with updated issue/finding/test memory and resolve the SDK/hardware gates listed in `PROJECT_MEMORY.md`; keep the PR draft until claims are supported.

## M1 lazy-slot rollback audit — 2026-10-08 13:06 UTC

- Source finding: lazy SC2 slots retain `handle=-1` until the controller wakes and a virtual device is created. The manager's allocation-failure and `pthread_create`-failure cleanup paths previously passed that sentinel to `scePadVirtualDeviceDeleteDevice`.
- Change: guarded both cleanup calls with `handle >= 0`; recorded as F028 / P1-SLOT-11. This is code review evidence, not a PS5 ABI/runtime verification.
- Validation: `make -C tests clean sanitize` PASS, 344 ASan/UBSan checks; a Python source assertion confirmed both call sites are guarded; `git diff --check` and publishable-text whitespace scan PASS. The host suite does not compile `gc_main.c`. PS5 SDK is unavailable, and no thread-creation fault injection or hardware test was possible.
- PR: code fix and updated memory are published in follow-up commit `04cc833e99c4e1e53eaaa93ae01658daa2cc69c0` on draft PR #1; do not promote the PR from draft.

## M1 profile numeric parsing audit — 2026-10-08 13:14 UTC

- Finding: `sc2_profile_parse` processed authenticated portal and saved profile numeric values with `atoi`. A long value outside `int` range has undefined behavior in C; partial strings such as `20junk` were silently accepted as 20.
- Change: replaced numeric `atoi` conversions with checked `strtol`, explicit `ERANGE`/`INT_MIN`/`INT_MAX`, full-input validation, and preservation of the prior/default setting when invalid. Valid in-range values still follow field-specific clamping. Added direct host tests with overflow, empty input, trailing junk, boundary and valid clamp cases.
- Tests: `make -C tests clean sanitize` PASS, 370 ASan/UBSan checks (profile parser 26); test builds `sc2_profile.c` on host with only target game lookup stubs. This does not test portal-to-disk integration or PS5 selection/runtime.
- Publication: profile parser and its test were published on draft PR #1 in commit `cf225f48f0f38e86101f4f6d2993edbe47347c58`; the PR body was updated to report 370 host checks. Host tests run `37782987424` was queued for that head.
- Next: inspect the queued PR workflow. Keep PS5 SDK, actual console and controllers as explicit outstanding gates.

## M1 profile parser publication — 2026-10-08 13:16 UTC

- Published strict profile integer parsing and `tests/test_sc2_profile.c` to draft PR #1 in commit `cf225f48f0f38e86101f4f6d2993edbe47347c58`; PR body now reports 370 ASan/UBSan checks and remaining target gates.
- Current PR head is mergeable and draft. GitHub Actions Host tests run `37782987424` is queued; no workflow conclusion is available yet.
- Test record M1-06: `make -C tests clean sanitize` PASS, 370 checks. The profile parser is host-compiled with stubs for target game lookup; the PS5 adapter, saved-profile integration, and hardware remain unverified.
- Next: inspect CI result. Keep the PR draft until PS5 SDK and physical USB/Puck/controller tests can be performed; BLE remains gated and untouched.

## M1 memory/provenance branch synchronization — 2026-10-08 13:17 UTC

- Updated the GitHub draft branch with current profile-parser publication, test records, and provenance in docs-only commit `afeb258a6ddb5c607b78dc9a481edccf1b3ff254` (parent `cf225f48f0f38e86101f4f6d2993edbe47347c58`). PR #1 remains open, mergeable, and draft.
- The docs commit triggered Host tests run `37783171488`; its job is queued and has no logs or conclusion. Earlier queued runs also had no result at inspection.
- The local project memory now records this branch head and latest queue state. Continue monitoring for a real CI conclusion; do not describe queued CI as passing.

## M1 profile parser concurrency audit — 2026-10-08 13:21 UTC

- Source finding: `sc2_profile_parse` can run from the profile watcher and authenticated portal on separate threads. Its `parse_combo` helper used process-global `strtok` state, which could interleave tokens across concurrent profile parses.
- Change: replaced `strtok` with a bounded tokenizer whose cursor and buffer are local to each call; added a four-thread regression stress test using four distinct profile mappings, 5,000 parses per thread.
- Test authoring note: the first draft used the wrong expected constant for the persisted `CREATE` alias; corrected the fixture to match parser/serializer output before recording the passing run.
- Validation: `make -C tests clean sanitize` PASS, 374 checks (profile parser 30) under ASan/UBSan on Darwin 27.0.0 arm64, Apple clang 21.0.0. This is not ThreadSanitizer and does not build/test the PS5 profile watcher or portal integration.
- Current publication state: the source/test fix is not yet published; PR #1 remains at `afeb258a6ddb5c607b78dc9a481edccf1b3ff254`, and Host tests run `37783171488` is queued with no conclusion.
- Next: publish the parser/test and count/memory updates to the existing draft PR; inspect the resulting Actions status without promoting the PR or claiming target verification.

## M1 profile parser concurrency publication — 2026-10-08 13:24 UTC

- Published `payload/sc2_profile.c`, its concurrent regression test, architecture/test-plan updates, and public README count to PR #1 in commit `ade3a334d4af408102c7b149e01dd3cc087fbc1a` (parent `afeb258a6ddb5c607b78dc9a481edccf1b3ff254`).
- Updated PR #1 body; GitHub reports it open, draft, and mergeable at the new head.
- Local `make -C tests clean sanitize` passes 374 checks. The PR's Host tests run `37784104792` is queued with no conclusion; earlier queued runs also remain unresolved.
- Next: monitor run `37784104792`; target SDK and hardware gates remain as listed in this memory.

## M1 project-memory synchronization — 2026-10-08 13:26 UTC

- Published findings, issues, test/session logs, project snapshot, and provenance to PR #1 in commit `2088cd639087b135940c7fc318223c6d3adeeca1` (parent `ade3a334d4af408102c7b149e01dd3cc087fbc1a`).
- Current head is `2088cd639087b135940c7fc318223c6d3adeeca1`; the PR API reports open/draft, mergeable true, `mergeable_state=unstable` while Actions is pending.
- Host-test runs `37784387092` (`pull_request`) and `37784376678` (`push`) are queued without logs or conclusions.
- Next: inspect these current-head runs once; do not report a CI pass unless a completed job confirms it.

## M1 profile-binding state and transition fix — 2026-10-08 13:35 UTC

- Source finding: `sc2_binding.c` used one process-global activator array. A reproduced regression showed a toggled button returned after switching to another profile and then back; multiple slots could also share that state. The controller did not consume an active-profile generation, so state could remain latched across profile replacement.
- Changes: binding state is now caller-owned per slot; `sc2_active_get_versioned` copies a profile and generation under one mutex; each generation change clears activators and emits one neutral report. `gc_main.c` owns a binding context per slot and resets it at slot startup/exit. Corrected the test matrix to label only actual coverage as partial host-verified or unrun.
- Regression evidence: before the fix, `make -C tests build/test_sc2_binding && ./tests/build/test_sc2_binding` failed on the stale-toggle assertion. After the fix, the binding target passes 16 checks; profile parser/active-generation target passes 33.
- Validation: `make -C tests clean sanitize` PASS, 393 ASan/UBSan checks on Darwin 27.0.0 arm64 with Apple clang 21.0.0. Full PS5 adapter files remain uncompiled because `PS5_PAYLOAD_SDK` is unset; no slot lifecycle injection or hardware test was run.
- Publication state: per-slot binding/profile-generation follow-up and refreshed docs are not yet on PR #1; current published branch head is `2088cd639087b135940c7fc318223c6d3adeeca1`.
- Next: publish these changes and memory/test-plan updates to draft PR #1, then check the new Host tests run. Keep target-build/hardware/ BLE gates explicit.

## M1 pause/resume binding invalidation — 2026-10-08 13:36 UTC

- Follow-up audit found the portal can pause/resume between input reports. A direct `memset` from the portal thread would race with the slot's binding evaluator, while resetting only on the next paused packet would miss a pause/resume cycle with no packets in between.
- Change: added an atomic reset epoch. Pause/resume advances it; each slot's input owner notices the mismatch at its next binding evaluation, clears local state, seeds current held inputs, and emits neutral output for one frame. No external thread mutates slot state.
- Test: `make -C tests clean sanitize` PASS, 397 ASan/UBSan checks; the binding test asserts asynchronous invalidation clears a latched toggle. This does not substitute for ThreadSanitizer or PS5 portal/input-thread integration.
- Next: publish the per-slot/profile-generation/async-reset changes, current 397-check test record, and corrected partial-coverage labels to PR #1.

## M1 neutral output and shared control-state audit — 2026-10-08 13:44 UTC

- Source finding: the wireless-disconnect branch reset stick centers but left prior buttons, analog triggers, and touch contacts in the reused pad structure. A profile-generation boundary returned an empty binding mask while still allowing analog/touch output from the same report, so it was not a fully neutral report.
- Changes: added one full `ScePadData` neutralizer for malformed known input, wireless disconnect, and binding-state initialization/profile/reset transitions; held inputs are seeded on transitions to prevent synthetic toggle/double edges. Binding tests now cover startup with a held toggle input and invalidator/input-owner concurrency.
- Shared state: moved pause flag operations to atomic load/exchange helpers in `sc2_control.c`; menu close from portal/manager-side code is a request applied by `sc2_menu_filter`, and all direct `sc2_paused`/`sc2_menu_open` references were removed. Other live telemetry remains shared volatile state and is still an open audit item.
- Validation: `make -C tests clean sanitize` PASS, 446 checks. ThreadSanitizer PASS for `test_sc2_profile` (33), `test_sc2_binding` (49), and `test_sc2_control` (20). `git diff --check` PASS. The host suite does not compile the PS5 adapter, menu, web server or manager.
- Build: `make -C payload` BLOCKED before compilation because `PS5_PAYLOAD_SDK` is unset; no ELF was produced.
- Publication: these local changes are not yet on PR #1; remote head is still `2088cd639087b135940c7fc318223c6d3adeeca1`. The earlier current-head Actions runs remain queued as last observed. Publish a source/docs commit, update the PR description, sync memory, then inspect the new workflow runs.

## M1 SDK v0.43 cross-build — 2026-10-08 13:53 UTC

- SDK decision: install was worthwhile because the target sources can now be compiled and linked locally. The official [`ps5-payload-sdk` v0.43 release](https://github.com/ps5-payload-dev/sdk/releases/tag/v0.43) points to commit `d9c9519116944a7f1c22d262012c53b80b3520b7`; its `ps5-payload-sdk.zip` asset matched the GitHub release API SHA-256 `a9cc9929f21b2b2c5d5b309f3bab4997067c45281c0622cf4838b1aecba66fcb`. GitHub metadata marks the source commit unsigned. SDK and archive are in ignored `.build-cache/ps5-payload-sdk-v0.43/`; no SDK contents enter source publication.
- Environment: installed Homebrew `llvm@18` version 18.1.8 (Homebrew reported 1.7 GB installed); `LLVM_CONFIG=/opt/homebrew/opt/llvm@18/bin/llvm-config`. This is the SDK README's documented macOS toolchain.
- Initial cross-build showed two unused-function warnings for helpers only called in the `__ORBIS__` VDA path. Guarded those helpers with `#ifdef __ORBIS__`; the clean rebuild then completed without warnings. An initial `make -C payload clean` without `PS5_PAYLOAD_SDK` stopped at the Makefile's environment guard; reran clean/build with both SDK variables set.
- Validation: `make -C payload clean && make -C payload` PASS; `llvm-readobj --file-headers --needed-libs` confirmed ELF64 x86-64 shared object and six expected PS5 runtime imports; `llvm-nm -u` showed expected `scePad*`/`sceUserService*` unresolved dynamic imports. ELF SHA-256: `34aaef13a4911147c6342469486aeaee6b2e10ec8ffa6d2d455f78c4ccdb80a4`. This is compile/link evidence only; no PS5, FW 13.60, game, Steam Controller or Puck runtime test occurred.
- Reproducibility changes: added `docs/BUILDING.md`, ignored `.build-cache/`, and pinned/verifies the SDK archive digest in `.github/workflows/release.yml`. `README.md`, provenance, issues, test log and project snapshot now distinguish cross-build success from hardware validation.
- Next: publish the current working-tree code and documentation to the open draft PR #1, then inspect the resulting Host tests, PS5 cross-build workflow, and release workflow; retain physical USB/Puck and game runtime gates.

## M1 publication and CI queue — 2026-10-08 14:03 UTC

- Published the 34 paths that differed from PR head `2088cd639087b135940c7fc318223c6d3adeeca1` as commit `65cd4f918f56fb2c547217e008f6b81c17853c04` (tree `98868818861c84f241f17f02b77e3811dec8a877`). Each uploaded Git blob SHA was checked against `git hash-object`; `.build-cache/` and the ELF remain ignored and were not sent.
- Updated draft PR #1 description with current implementation, 446 host checks, three targeted TSan results, SDK/hash/build evidence and hardware gates. GitHub reports the PR open, draft, and mergeable.
- New-head Actions runs: Host tests PR `37789014753`, PS5 cross-build PR `37789014700`, Host tests push `37789007214`, PS5 cross-build push `37789007082`. All four jobs were queued at the 14:01 UTC check; no conclusions were available.
- Next: inspect those run IDs again and record the actual conclusions, then continue remaining session/slot lifecycle review. Keep hardware USB/Puck and system/game testing blocked until a real PS5 and controller are available.

## M1 memory sync and queued-run recheck — 2026-10-08 14:06 UTC

- Published memory/provenance sync commit `78bcf3d840dd53b7f5c2c92e0c0b40612d3a6d6c` on top of `65cd4f918f56fb2c547217e008f6b81c17853c04`; the PR remained open, draft and mergeable.
- Rechecked GitHub Actions for head `78bcf3d`: Host PR `37789382680`, PS5 cross-build PR `37789382717`, Host push `37789373824`, and cross-build push `37789374156` all remained queued with unchanged timestamps and no logs. Treat this as CI queue delay, not a pass or a code failure; do not repeatedly poll without new evidence.
- Next: resume source review or wait for a meaningful workflow status change; physical PS5/Steam Controller/Puck gates remain blocked by hardware availability.

## M1 latest CI queue snapshot — 2026-10-08 14:08 UTC

- Memory-only CI snapshot commit `014f418ade3c7d09655ed302b09cf9a4d0e508fc` was the PR head at check time; GitHub reported PR #1 open, draft, and mergeable.
- Four latest runs were created and remained queued: Host push `37789861487`, PS5 cross-build push `37789861783`, Host PR `37789868606`, and PS5 cross-build PR `37789868667`. No job logs or conclusions were available. Older queued runs for prior heads remain historical and are not treated as current pass/fail evidence.
- Next: wait for an actual workflow status change before polling again; the same local 446-check ASan/UBSan suite and SDK v0.43 cross-build pass, while GitHub execution is still pending and hardware remains unavailable.

## M1 generation-tagged assignment lease — 2026-10-08 14:30 UTC

- Source review: `g_assign_slot` in `payload/gc_main.c` was a shared `volatile int` read and written by the manager and detached USB reader. Volatile does not synchronize those threads; after the manager's 30-second timeout, a delayed reader could set the shared value to `-1` after a newer controller had acquired the assignment dialog.
- Change: added atomic assignment lease helpers in `payload/sc2_control.c/.h`. Tokens encode a slot and generation; compare/exchange only releases the exact owner. Wired claim, first-button confirmation, timeout, detach, VDA failure, allocation/thread-start rollback, and reader exit through the token. `g_assign_slot` is removed. This is assignment-dialog serialization, not a complete slot/session manager.
- Tests: full `make -C tests clean sanitize` PASS, 477 checks. The expanded `test_sc2_control` now has 51 checks including 31 assignment assertions and 16,000 concurrent claims. The first TSan attempt failed because the test's activity counter was decremented after release, allowing a successor to be counted during the previous lease's bookkeeping; the test was corrected, and the rerun passes under ThreadSanitizer.
- Build: PS5 SDK v0.43 + LLVM 18.1.8 cross-build PASS without warnings. ELF64 x86-64 dependencies inspected; SHA-256 `21c7ac20345a685f065743243f44488ad0a206554a511344a34adc2d80263542`. No physical PS5 or USB hardware test.
- CI: before publishing this patch, PR #1 head was `057955b8716987f4171071f14b97a3eefe0537a7`; Host run `37790078829` and PS5 cross-build run `37790078672` remained queued at 14:30 UTC. Do not infer a pass.
- Changed paths: `payload/gc_main.c`, `payload/sc2_control.c`, `payload/sc2_control.h`, `tests/test_sc2_control.c`, `README.md`, `docs/ARCHITECTURE.md`, `docs/TEST_PLAN.md`, and project-memory evidence files.
- Next exact action: publish these validated file contents as a fast-forward commit on `fix/p0-safety`, then fetch the new head's workflow runs once; continue with the full slot/session ownership audit in `payload/gc_main.c` and portable fault-injection tests.
