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

## M1 assignment lease publication — 2026-10-08 14:35 UTC

- Published the validated assignment lease, test, documentation, and evidence files to PR #1 as commit `7ded26db0465992d9ce4c3f03163470727ec5ae6`, parent `057955b8716987f4171071f14b97a3eefe0537a7`. The 12 uploaded Git blob SHAs matched local `git hash-object` results before tree/commit creation; the branch update used expected-head validation and succeeded.
- PR #1 remains open, draft, and mergeable. The PR description now reports 477 ASan/UBSan checks, targeted TSan results, and current ELF SHA-256 `21c7ac20345a685f065743243f44488ad0a206554a511344a34adc2d80263542`.
- PR-triggered Host tests run `37793500940` and PS5 cross-build run `37793500959` are queued; combined commit status has no completed checks. This is pending, not a CI pass.
- The assignment-dialog lease issue P1-ASGN-16 is fixed in the published branch. The full slot/session manager and physical target behavior remain open.
- Next exact action: publish the updated memory/provenance snapshot on top of `7ded26db0465992d9ce4c3f03163470727ec5ae6`; then continue auditing slot ownership and teardown in `payload/gc_main.c`.

## M1 project-memory publication — 2026-10-08 14:35 UTC

- Published the follow-up project-memory/provenance snapshot to PR #1 as commit `0a3ef29edcce29ea78efd6f1e80c67a68b38c426`, parent `7ded26db0465992d9ce4c3f03163470727ec5ae6`. Six uploaded blob SHAs matched local `git hash-object` values; branch update succeeded with expected-head validation.
- PR head is now `0a3ef29edcce29ea78efd6f1e80c67a68b38c426`, open, draft, and mergeable. The source commit's Host run `37793500940` and PS5 cross-build `37793500959` were queued at 14:35 UTC; no new PR workflow run was returned for the documentation-only sync.
- Next exact action: audit reservation, USB-thread exit, and VDA teardown ownership in `payload/gc_main.c`; add host fault-injection around the existing lifecycle path before building a full slot abstraction.

## M1 authoritative slot registry — 2026-10-08 14:47 UTC

- Local base/PR state: checkout `fix/p0-safety` still has local `HEAD` at pinned Puckbridge `78460113ef7c48502c765a1f83ba34d53c977134`; current remote PR #1 head is `4e297d1c0e19df0fc5f451d285c8dc157985ef4b`, open, draft and mergeable. This implementation and its memory/docs updates are validated locally and are pending GitHub publication.
- Design/change: added reusable `payload/slot_registry.c/.h` as the sole authoritative owner of live slot key, VID/PID, reservation/active state, generation, confirmation and VDA handle. Manager claim atomically performs duplicate-key/capacity checks; generation token flows into `usb_hid_thread`; snapshot/inject/SC attach-detach/reader cleanup use that exact generation. Virtual handles cannot be owned by two live slots; teardown takes ownership before delete and release. Removed the competing `g_slots[]`/`g_slot_lock` metadata table. USB path is only an ephemeral session key; no stable physical identity or cross-transport rebind claim.
- Lifecycle correction: SC detach now releases only its own assignment lease token; it no longer releases whichever current lease happens to mention the same numeric slot. Startup/thread/activation rollback and reader exit share registry take/delete/release ownership.
- Tests: `make -C tests clean test` PASS, 550 checks; `make -C tests sanitize` PASS, 550 ASan/UBSan checks. New slot suite passes 73 checks including 8,000 concurrent reserve/activate/release cycles. Dedicated TSan command for `test_slot_registry.c` PASS, 73 checks.
- Build: SDK v0.43 + Homebrew LLVM 18.1.8 warning-free cross-build PASS; `llvm-readobj` confirms ELF64 x86-64 and expected six runtime libraries. Current ELF SHA-256 `544e5f0e404eecc050001fed93a74fbae8186f01bfc6a6cd62231d0138345e95`. Compile/link only; no PS5 runtime or hardware.
- CI before this change: head `4e297d1c0e19df0fc5f451d285c8dc157985ef4b` has queued Host run `37793911115` and PS5 cross-build run `37793911079`; combined status was empty at 14:44 UTC. Do not infer a result. Fresh workflow runs should be checked after publication.
- Safety: no Bluetooth/HCI command, stock-radio access, firmware write, payload deployment, or physical-device test occurred. PS5, DualSense, Steam Controller and official Puck are unavailable.
- Changed paths include `payload/slot_registry.c/.h`, `payload/gc_main.c`, `payload/Makefile`, `tests/test_slot_registry.c`, `tests/Makefile`, `README.md`, architecture/test-plan/provenance docs, and `project-memory/` records.
- Next exact action: run final whitespace/security scan, publish these validated changes as a fast-forward to `fix/p0-safety` using expected head `4e297d1c0e19df0fc5f451d285c8dc157985ef4b`, update PR #1 body with 550 checks/new ELF SHA, and inspect new CI once. Then add manager/USB fault injection at the portable seam; keep physical device identity/rebind and PS5 behavior open until evidence exists.

## M1 stale-generation regression and final validation — 2026-10-08 14:49 UTC

- Extended the slot test so a stale token from the prior generation attempts both `set_virtual` and `take_virtual` after slot reuse; both reject the stale owner while the newer VDA handle remains attached. Registry target is now 78 checks.
- Final `make -C tests clean test` and `make -C tests clean sanitize` PASS, 555 total checks. Slot registry ThreadSanitizer PASS, 78 checks including 8,000 concurrent cycles.
- Rebuilt with SDK v0.43 and LLVM 18.1.8; warning-free ELF64 x86-64, all six expected SDK libraries present; final ELF SHA-256 `61ad00697a426f11bb76afe11d8bb237b225acff08247034df8d4da6b463e209`.
- Final `git diff --check` passes; scans found no trailing whitespace in new C files, no common credential/private-key patterns in publishable files, and no remaining `g_slots[]`/`g_slot_lock` references in `gc_main.c`.
- Prior PR head `4e297d1c0e19df0fc5f451d285c8dc157985ef4b` is open, draft, mergeable; queued pre-change workflow runs are `37793911115` and `37793911079`. No inference of CI success.
- Next exact action: publish the validated implementation and evidence using the `4e297d1c0e19df0fc5f451d285c8dc157985ef4b` expected-head guard; update PR description to 555 checks and current ELF SHA, then inspect new run status once.

## M1 VDA delete failure recovery and final validation — 2026-10-08 14:59 UTC

- Failure-path review found that treating a failed `scePadVirtualDeviceDeleteDevice` as completed could permit unsafe reuse of its live handle. The registry now restores ownership on backend failure, keeps the slot in `RELEASING`, blocks reuse, and lets the controller manager retry cleanup on its next scan. The callback runs outside the registry mutex; failure and retry are covered through a portable injected callback. Exact code evidence: `payload/slot_registry.c:223-263,285-299`, `payload/gc_main.c:522-598,1066-1075`, and `tests/test_slot_registry.c:174-222`.
- Final `make -C tests clean test` and `make -C tests clean sanitize` PASS: 600 checks total, including 123 slot-registry checks. Targeted ThreadSanitizer passes all 123 registry checks and 8,000 concurrent claim/release cycles. SDK v0.43/LLVM 18.1.8 cross-build passes with no warnings; ELF SHA-256 `30413450154d33b8905425a6524b9cb6c78fefcb03db807ae8e391391b3fedfd`. The final bounded whitespace/known-secret-pattern/old-table scans and `git diff --check` are clean.
- The manager retry loop, persistent delete failures, thread-start failure rollback, and PS5 VDA behavior do not yet have injected target tests. P1-SLOT-18 remains open until manager fault injection and target evidence exist. No PS5, DualSense, Steam Controller, or official Puck hardware is available. No Bluetooth/HCI commands or firmware writes occurred.
- PR #1 remains open/draft/mergeable at `4e297d1c0e19df0fc5f451d285c8dc157985ef4b`; prior Host/PS5 workflow runs `37793911115` and `37793911079` were pending at 14:44 UTC. This slot-registry increment and current evidence files are prepared for publication; after publication, fetch the new head's runs and synchronize the exact commit/CI state in memory.
- Next exact action: publish the validated source, tests, docs and memory as a fast-forward on branch `fix/p0-safety` with expected head `4e297d1c0e19df0fc5f451d285c8dc157985ef4b`; then inspect the new PR workflows once. Continue with portable manager/USB lifecycle fault injection and preserve P1-SLOT-18 as open until integration evidence exists.

## M1 slot-registry publication and CI snapshot — 2026-10-08 15:03 UTC

- Published the validated slot-registry increment on `fix/p0-safety`: commit `8808f969cff62d0e129a1bbd3c391fed36e863c6`, parent `4e297d1c0e19df0fc5f451d285c8dc157985ef4b`. The connector created 16 file blobs; their returned Git blob SHAs matched the locally computed content SHAs. The branch update succeeded with the expected-head guard; no force update, merge or release occurred.
- PR #1 is open, draft, and mergeable at the new head. Updated its description with the current 600-check host suite, targeted TSan results, ELF SHA-256 `30413450154d33b8905425a6524b9cb6c78fefcb03db807ae8e391391b3fedfd`, and explicit hardware/runtime limitations.
- The current-head Host run `37797463592` and PS5 cross-build run `37797463596` were still queued at 15:03 UTC. Combined commit status was empty; no CI pass is claimed. Older queued runs are retained in `TEST_LOG.md`.
- This entry and post-publication CI snapshot are local and need a memory-only sync commit after the source commit. Next implementation task: design a portable fault-injection seam for reservation, worker startup/exit, and VDA deletion around the manager lifecycle, then run host sanitizers and the SDK cross-build before publishing.

## M1 portable manager lifecycle transaction — 2026-10-08 15:09 UTC

- Added `payload/slot_lifecycle.c/.h` as a narrow, platform-independent coordinator over the existing `slot_registry`: claim is still done atomically by the manager; startup optionally creates/registers a VDA, activates the generation, and starts the worker; failures release only handles owned by that generation; all rollback/release/retry calls share one helper. Wired `gc_main.c` startup, worker-launch callback and release wrapper to that module. Lazy SC2 startup still skips VDA creation until input wakes it.
- Added `tests/test_slot_lifecycle.c` with injectable create/worker/delete callbacks. It covers create failure, worker-start failure after activation, delete failure leaving the slot in `RELEASING`, successful retry, duplicate/ambiguous handle protection, lazy startup and stale-token rejection. A four-thread test completes 4,000 start/release cycles. The callbacks do not replace or claim to execute PS5 APIs.
- Verification: `make -C tests clean test` PASS, 685 checks; `make -C tests clean sanitize` PASS, 685 ASan/UBSan checks. Targeted lifecycle TSan PASS, 85 checks including 4,000 concurrent cycles. SDK v0.43/LLVM 18.1.8 cross-build is warning-free; ELF SHA-256 `c1aa736cbee5ce8d0f130033252f306c4bfe043d89479da296044a3656ac1b03`; expected six runtime libraries were inspected. Final `git diff --check` and bounded whitespace/secret/obsolete-slot-table scans pass.
- New source evidence is recorded as F037 and issue P0-SLOT-05/P1-SLOT-18 now distinguish portable startup/rollback tests from unverified PS5 scan-loop, USB ioctl/read, blocked-reader and hardware behavior. Stable identity/rebind and physical PS5/Puck tests are still unavailable.
- Published PR head remains `8808f969cff62d0e129a1bbd3c391fed36e863c6`, open/draft/mergeable. Host run `37797463592` and PS5 build run `37797463596` were queued at 15:03 UTC with empty combined status. The PR body still reflects the previous 600-check source commit until this lifecycle increment is published.
- Next exact action: publish all changed lifecycle source/tests/docs/memory as a fast-forward commit on `fix/p0-safety` with expected head `8808f969cff62d0e129a1bbd3c391fed36e863c6`; update PR body to 685 checks/current ELF SHA; inspect the new PR workflows once. After that, add injected target-adapter USB I/O and actual thread-creation failure coverage where a safe portable seam is possible.

## M1 lifecycle publication — 2026-10-08 15:11 UTC

- Published the lifecycle transaction, failure tests, updated docs and synchronized memory as commit `13c64c5439d82e81c31ec78dd030dbcf8d30bfe7`, parent `8808f969cff62d0e129a1bbd3c391fed36e863c6`. The 16 GitHub blob IDs matched local Git blob hashes. The `fix/p0-safety` branch update succeeded with expected-head validation; no force-push, merge or release occurred.
- PR #1 remains open, draft and mergeable. Its body reports 685 host checks, targeted TSan results, ELF SHA-256 `c1aa736cbee5ce8d0f130033252f306c4bfe043d89479da296044a3656ac1b03`, and target/hardware limitations.
- At 15:11 UTC GitHub returned no current-head workflow runs and an empty combined status for this commit. This is `NO RUNS RETURNED`, not a CI pass; the query is preserved as CI-27. Older run IDs belong to the previous commit.
- Next exact action: audit USB resource lifecycle in `payload/gc_main.c`, then extract only the smallest safe interface needed to inject USB open/init/read and worker-start failures into portable tests; keep real PS5 manager-scan behavior marked unverified. Continue game-hook rollback review after that.

## M1 USB resource/session lifecycle — 2026-10-08 15:22 UTC

- Added portable `usb_resources.c/.h` and `usb_session.c/.h`. The USB reader's target callbacks now route descriptor open, FS init, endpoint open, input transfer start/completion, and cleanup through one state owner. Failure injection covers open, init, endpoint-open, start and complete errors. Cleanup stops transfers that may have started, closes owned endpoint indexes in reverse order, uninitializes only after successful FS init, and relinquishes a descriptor after one close attempt.
- The first draft uninitialized after a failed `USB_FS_INIT`; safety review changed that behavior. A failed init now skips `USB_FS_UNINIT` and closes the descriptor, preserving the old failure-path behavior rather than adding an unverified ioctl after a failed initialization.
- Fixed `sc2_find_active_ep()` to route early `ENXIO` through its existing endpoint STOP/CLOSE loop. Added best-effort `USB_FS_STOP` after errors and timeouts in `usb_send_out()` and SC2 haptic OUT completion.
- First targeted test invocation failed at a wrong expected event count (`event_count == 6`, actual 5); corrected expectation. Recorded as M1-38. Initial SDK build invocation could not resolve LLVM because Homebrew's `llvm-config-18` was not in PATH; setting `LLVM_CONFIG=/opt/homebrew/opt/llvm@18/bin/llvm-config` fixed the wrapper. Recorded as M1-39.
- Verification: `make -C tests clean test` PASS 821 checks; `make -C tests clean sanitize` PASS 821 ASan/UBSan checks. SDK v0.43 + LLVM 18.1.8 cross-build PASS; ELF64 x86-64 PIE, six expected runtime libraries, SHA-256 `1e2667360d07aaf8eeb4f71246e119e779a26a99f0d79fcfddc61c3cb8641811`. `git diff --check` and bounded common-secret patterns pass. This does not execute PS5 ioctls, manager scheduling, real `pthread_create`, blocked kernel reader interruption, Puck/Steam hardware, or console behavior.
- Source evidence recorded as F038 and issue P0-USB-LIFE-19; ADR-008 documents the owner/adapter contract. PR #1 still has remote head `13c64c5439d82e81c31ec78dd030dbcf8d30bfe7`; this increment and its updated memory are local, uncommitted and unpublished at this log entry.
- Next exact action: publish source, tests, docs, and memory to `fix/p0-safety` using expected head `13c64c5439d82e81c31ec78dd030dbcf8d30bfe7`; update PR description to 821 checks and current ELF hash, then inspect current-head workflow status. Follow with game-hook process-exit/payload-stop rollback audit; PS5 and Steam Controller/Puck hardware are still required for physical gates.

## M1 final USB errno/build verification — 2026-10-08 15:25 UTC

- Final review tightened `usb_send_out()` to preserve the USB completion errno across its best-effort STOP ioctl; M1-44 recompiles this exact source state successfully.
- Final SDK v0.43/LLVM 18.1.8 build is warning-free; the final ELF is ELF64 x86-64 PIE with the six expected SDK runtime libraries and SHA-256 `273bf561ececaa0093c40827ffac9cabb087252fdd028afca32ec5d623dbe545`. `git diff --check` passes; the bounded private-key/token scan returned no matches. Test evidence remains 821 regular and 821 ASan/UBSan checks (M1-40/41).
- This source/memory increment is still local; PR #1 expected remote head for publication remains `13c64c5439d82e81c31ec78dd030dbcf8d30bfe7`. Next exact action: publish the validated changes with an expected-head fast-forward, update the PR description and inspect the new CI status once.

## M1 USB lifecycle source publication — 2026-10-08 15:28 UTC

- Published the USB reader/resource increment as commit `28acb8a8b0d4c979ecfc23a81e36e53c3e236e59`, parent `13c64c5439d82e81c31ec78dd030dbcf8d30bfe7`. All 21 uploaded file blob SHAs matched locally computed Git blob IDs. The branch update used the expected-head guard with `force=false`; no merge or release occurred.
- PR #1 remains open, draft and mergeable. Its body now reports 821 host and ASan/UBSan checks, the final ELF SHA-256 `273bf561ececaa0093c40827ffac9cabb087252fdd028afca32ec5d623dbe545`, and exact physical/runtime limitations.
- Current-head Host run `37801053888` and PS5 cross-build run `37801053913` were queued at 15:28 UTC. Combined status was empty; no CI pass is claimed. The earlier empty/no-run result for parent `13c64c5` is retained in CI-27.
- The next exact action is to publish this post-publication memory/provenance/CI synchronization as a guarded fast-forward from `28acb8a8b0d4c979ecfc23a81e36e53c3e236e59`. After that, audit game-hook cleanup for process exit and payload stop; physical PS5/Steam Controller/Puck tests remain blocked on hardware.

## M1 game-hook rollback and stale-mute hardening — 2026-10-08 15:40 UTC

- Audited `payload/game_hooks.c` and `payload/bridge/wireless_ds4.c`. Found that removal ignored failure to disable input merging and unconditionally restored each saved import slot, which could overwrite a later module's hook. Process-exit handling also cleared the PID without clearing the stale args address. Legacy layout migration could continue installing after an old hook slot failed to restore; audio muting had no producer-staleness expiry.
- Added `game_hook_safety.h` with portable restore-owner and saturating heartbeat-lease policies. The test first failed under the previous unconditional policy (M1-45), then passed after correction. New layout v6 stores expected gateway addresses and restores only slots still pointing to that gateway; already-original slots are idempotent and conflicting owners are left untouched. Unreadable/unrestored owned slots retain target ownership and retry. Legacy v5 migration disables input/audio, verifies known gateway slots and aborts before installing replacements if cleanup is uncertain. This is still a remote read-then-write check, not an atomic compare-and-swap.
- Audio silence refreshes a producer heartbeat in `pb_hooks_set_silence_locked()` and expires after 120 stale consumer callbacks. Existing input frames expire after 60 pad reads. Payload restart intentionally reuses identical game-owned hook mappings; the stubs remain in the game's address space until process exit. No explicit signal-driven payload shutdown or unmapping was added because none is established in this payload lifecycle. This does not verify on-console mute timing or game exit behavior.
- `game_hooks.c` now clears stored args on confirmed `ESRCH`, distinguishes other process-check errors, retains live-target ownership after rollback failures and retries instead of attempting a replacement hook. No PS5 `mdbg` operations were run.
- Verification: M1-46 full host suite and ASan/UBSan suite PASS, 837 checks each. M1-47 SDK v0.43 + LLVM 18.1.8 cross-build PASS, warning-free ELF64 x86-64 PIE with six SDK runtime libraries, SHA-256 `ab7239b7dd7bed77581191585dfaa92326305ab5cbbf617ebfeffdd8be583141`. M1-48 `git diff --check` clean and bounded private-key/token scan no matches. CI on previous remote head `096a4daf7d4fbcb997aa1f3314a44a46a0c693d4`: Host `37801642039` and PS5 cross-build `37801642059` queued, combined status empty.
- Recorded F039, ADR-009, P0-HOOK-06 update, T-P17 and M1-45–48. This source and current memory are local and unpublished. Next exact action: confirm remote `fix/p0-safety` still points to `096a4daf7d4fbcb997aa1f3314a44a46a0c693d4`, then publish the complete reviewed game-hook changes/docs/memory with an expected-head fast-forward, update PR #1 with 837 checks and the new ELF hash, and inspect the new PR runs. After publication, resume remaining handoff/shared-telemetry lifecycle review. Physical console and controller are still needed for real process-exit, shutdown, audio, input and game-hook verification; BLE remains passive and blocked.

## M1 game-hook source publication and current-head CI — 2026-10-08 15:45 UTC

- Published the reviewed hook-owner rollback, v5-to-v6 migration safeguards, audio heartbeat lease, tests, docs, and issue/memory records as `2e24ac196f52cb850ba3df6de328a5c0c198e452`, parent `096a4daf7d4fbcb997aa1f3314a44a46a0c693d4`. All 15 uploaded file blob SHAs matched local Git blob IDs; `fix/p0-safety` advanced with the expected-head guard (`force=false`).
- PR #1 remains open, draft, and mergeable. The description now records 837 host/sanitizer checks, SDK ELF SHA-256 `ab7239b7dd7bed77581191585dfaa92326305ab5cbbf617ebfeffdd8be583141`, and target/hardware limits.
- Current-head workflow query returned Host run `37803237618` and PS5 cross-build run `37803237655`, both queued at 15:45 UTC. Combined commit status is empty. No GitHub CI result is claimed.
- Local checkout remains the pinned Puckbridge source `HEAD` plus an unstaged snapshot; Git CLI author is unset. Publication was performed through the connected GitHub app; source and test state was already recorded by M1-46–48 and F039/ADR-009.
- Next exact action: inspect `payload/ds_handoff.c`, the portal/manager telemetry ownership path, and remaining shared lifecycle for reproducible stale-owner or race defects; add narrowly scoped host regression tests before code changes. Do not run active Bluetooth/HCI or console remote-memory operations. PS5, Steam Controller, DualSense, and official Puck tests remain hardware-blocked.

## M1 shared-state race audit and regression suite — 2026-10-08 16:08 UTC

- Repo: `/Users/mneroba/GitHub/SteamController-PS5`; local `HEAD` remains pinned Puckbridge `78460113ef7c48502c765a1f83ba34d53c977134` with the derivative project as uncommitted workspace changes. Remote draft PR #1 still points to `2e24ac196f52cb850ba3df6de328a5c0c198e452` at the start of this publication step.
- Audit: the hand-off and runtime status race fixes from the previous continuation are now accompanied by a complete hook-worker shared-state audit and gyro reset/status audit. Game-hook settings use coherent locked snapshots and serialized latest-state persistence; owner/active and JSON-visible state use the mutex; short/null JSON buffers are rejected. Gyro IMU/calibration/motion/reset/status state is locked; retry state has a separate mutex; moving state uses locked accessors; gyro toggle edge history resets with the controller state.
- Regression: added host tests that compile the production `game_hooks.c` worker with mocked game/process bridge calls, and production `sc2_gyro.c` with host-only USB declarations. Test-only worker shutdown allows sanitizer cleanup. No PS5 remote-memory or USB ioctl ran. The initial hand-off TSan reproducer found three races and is retained in M1-50; the fixed hand-off/status tests have 60,000 + 100,000 checks. Latest hook/gyro tests pass 61,000 + 75,000 checks under TSan. Full ASan/UBSan run passes 837 counted assertions plus 296,000 concurrent operations (M1-54). A prior commentary value of 291,000 was an arithmetic error; 296,000 is the correct sum.
- Target build: SDK v0.43 and LLVM 18.1.8 cross-build warning-free, ELF64 x86-64 PIE, six expected shared libraries, SHA-256 `fdaac4f4a6d332a522fd3ff0d6ce28f4acd37e5db5592317ad553ebaf6dce9a5` (M1-55). This proves compile/link only.
- Final local review: `git diff --check` passes; repository whitespace scan and bounded private-key/GitHub/AWS/bearer-token scan found no matches (M1-56). No third-party code was imported in this increment. The updated project memory now records F040–F042, ADR-010, P1-HOOK-21/P1-GYRO-22 and test IDs M1-49–56.
- Blockers: current draft PR body/remote tree do not yet include the latest local race fixes; no PS5, Steam Controller, DualSense or Puck hardware is available. No Bluetooth/HCI or physical-console operation was attempted. Host mocks do not verify game-memory ownership, USB feature-report behavior, controller calibration, PS5 process/thread lifecycle or system UI behavior.
- Next exact action: compare local Git blob SHAs against GitHub's recursive tree for PR head `2e24ac196f52cb850ba3df6de328a5c0c198e452`; publish the changed files on `fix/p0-safety` using that exact expected-head guard; update draft PR #1 validation and inspect the new Host/PS5 workflow runs once. Then continue only with another reproducible source issue or a hardware-gated task.

## M1 per-session Steam input-state isolation — 2026-10-08 16:17 UTC

- Repository: `/Users/mneroba/GitHub/SteamController-PS5`; local `HEAD` remains the pinned Puckbridge source `78460113ef7c48502c765a1f83ba34d53c977134` plus a derivative working-tree snapshot. GitHub confirms PR #1 is open, draft, and mergeable at `e01055e13601e9451b241b221f0cc19a1a8618e5`; Host `37806756014` and PS5 cross-build `37806755932` remain queued at 16:17 UTC with empty combined status.
- Source audit found `sc2_flick.c` stored edge, pending rotation, prior angle and report interval in function globals; `controller_sc2.c` also used global QAM-hold and menu-action-edge history. Multiple USB reader threads can own separate Steam Controller sessions, so one session could emit another session's pending right-stick flick. The new expected-red test failed at `tests/test_sc2_flick.c:25` before the fix (M1-58). The first fixture also had a harness-only link failure because it unnecessarily linked `sc2_profile.c`; that negative result is retained at M1-57 and the test was reduced to the type/field it actually needs.
- Introduced caller-owned `sc2_flick_state_t` and `sc2_input_state_t`; `usb_hid_thread` creates one context and passes it through the packet adapter. Binding state, flick timing/accumulator, QAM hold/latch, and menu-action edge history are now scoped to that reader. Context reset is explicit at reader start/exit and malformed-input/disconnect paths. No cross-session global lock was introduced. Process-level profile selection, menu, gyro, haptics and status semantics remain shared and do not yet prove correct simultaneous multi-controller operation.
- Verification: `make -C tests clean sanitize` PASS (exit 0), 850 assertions and 316,000 concurrent operations total (M1-59); targeted `test_sc2_flick` ThreadSanitizer PASS with 20,000 concurrent session report calls and no race warning (M1-60); SDK v0.43/LLVM 18.1.8 cross-build PASS, warning-free ELF64 x86-64 PIE with six expected imports, SHA-256 `870e44356f36dbf797ce6636cb324b1d346874f0e87807ef9ef83d546128caa3` (M1-61). These do not run PS5 USB I/O or physical hardware. `git diff --check` passes so far; final source/credential scan and publication blob comparison are next (M1-62).
- Memory records are updated with F043, P1-SC2-SESSION-23, ADR-011, T-P22, corrected 850/316k snapshot and current CI state. No third-party implementation code was imported.
- Next exact action: compare current local file blobs against the e01055 PR tree/current contents, publish the validated per-session delta as a guarded fast-forward to `fix/p0-safety`, update the PR body with 850 assertions/316,000 operations and the new ELF SHA, then inspect the resulting current-head Host and PS5 cross-build Actions runs once. Keep PR draft. Do not use BLE/HCI or remote process-memory operations. Physical PS5, Steam Controller, DualSense and Puck gates remain blocked by unavailable hardware.


## M1 per-session Steam input state publication — 2026-10-08 16:23 UTC

- Published the per-reader Steam input/flick context, host regression, architecture, provenance, test-plan, and project-memory updates as `1682e7806ceff3dc5d0b43b8f1927ddf499a0818`, parent `e01055e13601e9451b241b221f0cc19a1a8618e5`. The parent tree was verified as `515e44245b1c1573e2c2e9da0b006ae3353cf74d`; all 16 uploaded blobs matched local Git blob IDs, and the branch advanced with expected-head validation and `force=false`.
- GitHub confirms PR #1 is open, draft, and mergeable at `1682e7806ceff3dc5d0b43b8f1927ddf499a0818`. Its body reports 850 ASan/UBSan assertions, 316,000 concurrent operations, SDK v0.43 ELF SHA-256 `870e44356f36dbf797ce6636cb324b1d346874f0e87807ef9ef83d546128caa3`, and hardware/runtime limits.
- At 16:23 UTC `github_fetch_commit_workflow_runs` returned no PR-triggered runs for this new head; `github_get_commit_combined_status` returned no statuses. Do not claim GitHub CI passed.
- Current memory records F043, P1-SC2-SESSION-23, ADR-011, T-P22 and M1-57–62; this publication-status sync is being prepared as a docs-only follow-up. Local Git `HEAD` remains the pinned Puckbridge commit with a derivative workspace snapshot; no local source commit or upstream push was made.
- Next: publish the docs-only post-publication synchronization with an expected-head guard, then audit `sc2_trigfx.c` runtime fields for a reproducible per-session race/contamination. Add an expected-red test before any code fix. Continue to keep PR #1 draft and BLE/HCI plus physical console operations gated.

## M1 adaptive-trigger state isolation — 2026-10-08 16:29 UTC

- Repository: `/Users/mneroba/GitHub/SteamController-PS5`; local Git `HEAD` remains pinned Puckbridge `78460113ef7c48502c765a1f83ba34d53c977134` with the derivative snapshot in the working tree. GitHub reports draft PR #1 open/mergeable at `555df96b0dcf2c82c0f1f2cd62f1bdfd7f203e45`; source publication for this increment is pending.
- Audit: `payload/sc2_trigfx.c` was called per USB reader but stored pull zones and vibration cooldowns in file globals. This could create false threshold clicks or cause one session's 100 ms vibration throttle to suppress another session's pulse. Effect command/enable configuration is still game-hook/process-owned and protected by its existing mutex.
- Regression: first host link failed because the host target did not include `tests/host_stubs` for the PS5 USB header (M1-63). After correcting only the include path, the expected-red interleaving failed as intended with one false pulse (M1-64). Added `sc2_trigfx_state_t` to per-reader `sc2_input_state_t`; reset it at session reset and inactive effects, copy the shared config under mutex, then process without holding the lock across haptic calls. Tests verify zone isolation, independent buzz timers and two concurrent streams.
- Verification: full `make -C tests clean sanitize` PASS, 861 assertions plus 336,000 concurrent operations (M1-65); targeted trigger test ThreadSanitizer PASS, 11 checks/20,000 concurrent reports (M1-66); SDK v0.43 + LLVM 18.1.8 cross-build PASS, ELF64 x86-64 PIE with six expected imports, ELF SHA-256 `85fa67052a36945656bbf5b0c6c82ac31a8110804cc7579ad886bc00f6e1d135` (M1-67). These do not exercise PS5 USB, game effect-hook routing, physical controller haptics, or shared output queue behavior.
- Memory additions: F044, P1-TRIGFX-24, ADR-012, T-P23, M1-63–68. No third-party code was borrowed. Current PR body still says 850/316,000 and old ELF hash until publication and metadata sync. Keep the PR draft.
- Next exact action: run M1-68 diff/whitespace/credential scans, publish the code+test+evidence delta as a guarded fast-forward from current PR head `555df96b0dcf2c82c0f1f2cd62f1bdfd7f203e45`, update PR #1 with current test/hash/hardware evidence, and inspect current-head Actions/status once. Then resume the next source-backed shared-state issue; keep native BLE passive and physical-console/PS5 operations blocked without hardware.

## M1 adaptive-trigger session isolation publication — 2026-10-08 16:33 UTC

- Published the validated adaptive-trigger state isolation, per-reader input-context wiring, host regression, architecture/test plan and detailed memory as `ec497365b8b173c792cef08f973cb540751b1332`, parent `555df96b0dcf2c82c0f1f2cd62f1bdfd7f203e45`, tree `6cc37b1ca4ea3adc1b9042310a56ce55af4f9846`. All 14 uploaded blobs matched local Git blob IDs. `fix/p0-safety` advanced using expected-head validation with `force=false`.
- PR #1 is confirmed open, draft, and mergeable at the new head; the body now reports 861 ASan/UBSan assertions, 336,000 concurrent operations, and SDK ELF SHA-256 `85fa67052a36945656bbf5b0c6c82ac31a8110804cc7579ad886bc00f6e1d135`, with host/target/hardware boundaries.
- At 16:32 UTC `github_fetch_commit_workflow_runs` returned no PR-triggered runs for this head and combined status returned no statuses (CI-42/43). This is not a CI pass.
- Local evidence: expected-red M1-64 observed the cross-session false click; the fixed suite, targeted TSan, SDK build, diff/whitespace/credential scans passed (M1-65–68). No third-party code was borrowed and no physical console, Steam Controller, Puck, or DualSense was used.
- Next exact action: inspect `payload/sc2_haptics.c` and its callers for source-backed queue concurrency/lifecycle defects. Add a failing regression before code changes; keep any shared haptic output queue limitations explicit. Hardware H-08 and PS5 runtime remain blocked; BLE/HCI remains passive.

## M1 haptic service route isolation — 2026-10-08 16:40 UTC

- Audit found `sc2_haptic_service()` is invoked by USB reader sessions but writes `g_n_extra` globally; `send_report()` later reads it while broadcasting through `eps[2+]`. Per-call logging statics were also shared without synchronization. The pulse/rumble queue itself is mutex-protected but remains process-wide.
- Expected-red: the new host seam compiles production `sc2_haptics.c` while replacing only the low-level OUT call. A barrier test with reader A at `n_extra=0` and B at `n_extra=2` made A send two unintended extra OUT reports (M1-69). ThreadSanitizer independently reported a race on `g_n_extra` in `sc2_haptic_service+0x50` (M1-70). The initial output was large because the stress test logged every broadcast; the test now switches to method 0 during stress, keeping test output compact without changing production behavior.
- Fix: pass the caller's count through each `send_report()` call and clamp it to `[0, SC2_HAP_MAX_OUT]`; protect report/service log-throttling state with a separate mutex. Queue lock is still released before sends/logging. Test covers zero vs two extra endpoints, positive and negative bounds, and 4,000 concurrent service reports.
- Verification: targeted normal and TSan tests pass 12 checks/4,000 concurrent calls with USB sender stubbed (M1-71–72); full ASan/UBSan suite passes 873 assertions and 340,000 concurrent operations across seven stress targets (M1-73); SDK v0.43/LLVM 18.1.8 cross-build passes, ELF64 x86-64 PIE with six imports, SHA-256 `4a98b147ec9026321a2f7de75ab9ed253ff0e790803a5b580c00d7d13bc2295f` (M1-74). Actual USB ioctl, physical haptic output and simultaneous-reader queue ownership were not tested.
- Memory additions: F045, P1-HAPTIC-25, ADR-013, T-P24, M1-69–75. No third-party code used. The haptic delta is validated locally but pending publication; remote PR #1 currently remains at `e532fb1d156fb830d4ef636be6ebe63a088bcf98`, draft/open/mergeable, and its body is stale at 861/336k and the prior ELF checksum. Current-head Actions for e532 had no PR runs/statuses; no CI pass is claimed. `git diff --check`, changed-file whitespace scan and bounded credential scan pass (M1-75).
- Next exact action: reconcile the current PR body/CI queue state in memory, then continue the source-backed haptic queue ownership audit. Keep the process-wide pulse queue limitation explicit; H-08/console remain hardware-blocked and BLE/HCI stays passive.

## M1 haptic service route isolation publication sync — 2026-10-08 16:49 UTC

- GitHub confirms PR #1 is open, draft, and mergeable at source commit `a902be57084fd90e70672c3fbf84db8a7a297f1c`, parent `e532fb1d156fb830d4ef636be6ebe63a088bcf98`, tree `57afe5ece0df9ed7b6fb0f5d654c7b23ae4bf6a0`. The source/test/docs increment and its detailed M1-69–75 evidence are recorded in PUB-18, F045, P1-HAPTIC-25 and ADR-013.
- The PR description matches the local evidence: 873 ASan/UBSan assertions, 340,000 concurrent operations/snapshots, targeted TSan for trigger/haptics, and SDK v0.43 ELF SHA-256 `4a98b147ec9026321a2f7de75ab9ed253ff0e790803a5b580c00d7d13bc2295f`. It states that process-wide pulse queue ownership and all hardware/target runtime behavior remain unverified.
- At 16:49 UTC current-head Host run `37810987840` and PS5 cross-build run `37810987743` are QUEUED; the combined status response is empty. No GitHub CI pass is claimed. Do not treat missing status checks as success; query the current-head runs later for a conclusion.
- No new code changed in this documentation synchronization. Local Git `HEAD` remains pinned Puckbridge `78460113ef7c48502c765a1f83ba34d53c977134` with a derivative working tree; no local commit was made. The PR remains draft.
- Next exact action: inspect the process-wide haptic pulse/rumble producer-consumer flow in `payload/sc2_haptics.c`, `payload/controller_sc2.c`, `payload/game_hooks.c`, `payload/sc2_menu.c` and `payload/webui.c`. Establish whether source ownership is available across every producer before proposing per-session routing; if a deterministic wrong-device delivery can be reproduced, add expected-red host coverage first. In parallel, let CI run and query `a902be57084fd90e70672c3fbf84db8a7a297f1c` again after a meaningful interval. Hardware and BLE gates remain closed.

## M1 parser property coverage and multi-session haptic audit — 2026-10-08 17:00 UTC

- Added `tests/test_steam_parser_fuzz.c` and wired it into the standard host/sanitizer target. It runs 100,000 deterministic Steam input/battery/wireless buffers and 100,000 USB configuration descriptor inputs, mixing fixed report IDs, random lengths, mutated near-valid configuration descriptors and arbitrary bytes. Invariants check documented return ranges, report classification, input wire length/IMU boundary and Puck interface filtering. This is bounded property sampling, not exhaustive fuzzing.
- `make -C tests clean sanitize` passes under ASan/UBSan with 873 assertions, 340,000 concurrent operations/snapshots, and the new 200,000 parser inputs (M1-76). The targeted parser property executable also passes independently under strict warnings. No production C changed, so the prior SDK ELF build remains the latest payload build; this test-only increment does not establish PS5 runtime behavior.
- Haptic source audit confirms `controller_manager_thread()` starts multiple SC2 Puck/wired lazy sessions without acquiring the assignment-dialog lease (`gc_main.c`); `sc2_haptics.c` has one process-wide queue/rumble state and each reader calls the same drain function; producers in controller/menu/game-hook/portal paths carry no source session. Record this as source evidence F046 / P1-HAPTIC-26, not a hardware-observed misroute. The host service test covers a different race (extra OUT count), not two physical-device ownership.
- ADR-014 defers adding an implicit active-session pointer. A sound fix needs either a stated single-SC2 product gate or an explicit generation-tagged session/slot owner propagated to every producer and consumer, plus expected-red regression coverage. Until then do not claim multi-session haptic routing. No unsupported ownership policy was guessed.
- At 16:57 UTC the current PR tip `d0b9b983ac281d575957ec30e07c7a0acbd8c493` was open/draft/mergeable; its Host run `37812773026` and PS5 cross-build run `37812772968` were queued with no combined status. The local test/docs/memory increment is not yet published; branch update must use expected-head guard `d0b9b983ac281d575957ec30e07c7a0acbd8c493` and `force=false`.
- Final local checks: M1-77 `git diff --check`, trailing-whitespace scan, and bounded private-key/GitHub/AWS/bearer-token scan all pass; no borrowed code was introduced.
- Publication completed as `7bcfcdae104d33cd8dd0363d472c79bbc25ec64c`; publication details and exact blob/tree checks are in PUB-19. The PR body now includes 200,000 parser inputs and the open multi-session haptics limit.

## M1 parser property coverage publication — 2026-10-08 17:02 UTC

- Published the fixed-seed Steam report/USB descriptor property test, standard Makefile target, updated T-P25, F046/P1-HAPTIC-26/ADR-014 and memory records as `7bcfcdae104d33cd8dd0363d472c79bbc25ec64c`, parent `d0b9b983ac281d575957ec30e07c7a0acbd8c493`, tree `51c10780c3d365337c7e1e338e421a6c7634d6d8`. All 9 uploaded blob SHAs matched local Git object IDs. `fix/p0-safety` advanced with an expected-head guard and `force=false`.
- PR #1 is confirmed open, draft and mergeable. Its current body reports 873 ASan/UBSan assertions, 340,000 concurrent operations/snapshots, 200,000 parser inputs, and the existing SDK v0.43 ELF SHA-256 `4a98b147ec9026321a2f7de75ab9ed253ff0e790803a5b580c00d7d13bc2295f`. It records the open P1-HAPTIC-26 multi-reader ownership issue and physical/target gates.
- At 17:02 UTC the Actions run lookup for this tip returned no PR-triggered runs and combined status returned no statuses (CI-46/47). No GitHub CI pass is claimed.
- The increment changes tests/docs/memory only. It does not change production C or the cross-built payload. Local `HEAD` remains pinned to Puckbridge `78460113ef7c48502c765a1f83ba34d53c977134` with the derivative workspace tree. PR remains draft.
- Next exact action: continue source-backed Steam USB/Puck and session lifecycle audit without active Bluetooth or physical-console operations; after a meaningful interval, check whether GitHub scheduled workflows for `7bcfcdae104d33cd8dd0363d472c79bbc25ec64c` and record any conclusion. Keep P1-HAPTIC-26 open until an active-session policy and owner-token contract are explicit.


## M1 concurrent SC2 endpoint scanner isolation — 2026-10-08 17:10 UTC

- Audited `sc2_find_active_ep()` against the pinned Puckbridge source `78460113ef7c48502c765a1f83ba34d53c977134`. It inherited `static uint8_t bufs[SC2_MAX_EPS][64]`; the manager can run independent Puck and wired SC2 discovery workers, so completion data was shared across callers.
- Added `tests/test_sc2_usb_scan.c`, a deterministic USB ioctl fake that overlaps two actual production `sc2_find_active_ep()` calls and makes the Puck completion write a valid `0x42` report while the wired scan writes invalid bytes. Expected-red M1-80 showed both calls returning no active endpoint because the shared report was overwritten. Host-stub compile gaps (M1-78) and a non-terminating fake completion loop (M1-79, interrupted with exit 130) were corrected in the harness and retained in the test log.
- Fix: changed only the scanner scratch buffers to per-invocation automatic storage (`SC2_MAX_EPS * 64` bytes). Added test-host USB declarations and Makefile target. Added F047, P1-SC2-SCAN-27, ADR-015 and T-P26; documented the remaining actual-ioctl and target-stack gates. No third-party code borrowed.
- Validation: M1-81 normal suite PASS; M1-82 full ASan/UBSan PASS (873 counted assertions, 340,000 concurrent operations/snapshots, 200,000 parser inputs); M1-83 scanner TSan PASS; M1-84 SDK v0.43/LLVM 18.1.8 warning-free cross-build PASS with ELF SHA-256 `215f2f1b73c4b08be42392421f859469d6e03a17714e8bc63505e81344f19492`, six inspected SDK libraries; M1-85 whitespace and bounded credential scans PASS. These are host/compile-link evidence only.
- At 17:10 UTC PR #1 remains open/draft/mergeable at old head `e77e440f11e5e91d3bf5d26a9dfdbe4ff1df5d4b`. Host Actions run `37813669678` and PS5 cross-build run `37813669565` remain queued, with empty combined status. No CI pass is claimed.
- Hardware tasks remain blocked: no PS5, Steam Controller, official Puck, or DualSense available; no Bluetooth/HCI commands were run. P1-HAPTIC-26 remains open; this scan fix does not establish haptic ownership or multi-controller correctness.
- Next exact action: publish `payload/controller_sc2.c`, `tests/test_sc2_usb_scan.c`, `tests/Makefile`, its host USB stubs, architecture/test-plan updates and memory to draft PR #1 with an expected-head guard on `e77e440f11e5e91d3bf5d26a9dfdbe4ff1df5d4b`; update PR validation with the new ELF SHA; inspect current-head workflow results once. Then continue source audit of Steam USB/Puck and target lifecycle without opening Bluetooth or touching physical hardware.


## M1 concurrent endpoint scanner publication — 2026-10-08 17:12 UTC

- Published the per-invocation scanner buffers, deterministic host USB ioctl regression, host declarations/Makefile target, architecture/test-plan updates, and corresponding memory as source/test/docs commit `b6f5bae3f6cb2c45be0e798c10b6cf2345ce2edf`, parent `e77e440f11e5e91d3bf5d26a9dfdbe4ff1df5d4b`, tree `7af2ecd5cf206414a414871c6a60454598028da4`.
- All 13 GitHub blob SHAs matched local `git hash-object` values. `fix/p0-safety` advanced using expected head `e77e440f11e5e91d3bf5d26a9dfdbe4ff1df5d4b` and `force=false`.
- PR #1 is confirmed open, draft and mergeable. Its body includes the expected-red scanner reproduction, normal/ASan/UBSan/TSan results, SDK v0.43 ELF SHA-256 `215f2f1b73c4b08be42392421f859469d6e03a17714e8bc63505e81344f19492`, and target/hardware limits.
- At 17:12 UTC GitHub reported Host run `37814698394` and PS5 cross-build run `37814698350` queued on this source/test/docs commit; combined status was empty. No GitHub CI pass is claimed. This publication state is recorded in PUB-20/CI-50/CI-51.
- No console or controller was available; no BLE/HCI command was run. P1-HAPTIC-26 remains open and unrelated to the scanner fix.
- Next: finish the current PR status sync, then continue source-backed USB/Puck/session lifecycle audit and resolve the haptic producer/consumer ownership contract before any multi-controller claim.


## M1 game-title metadata buffer race — 2026-10-08 17:17 UTC

- Source audit: `sc2_profile.c`'s background watcher calls `gc_game_name()` when the active title changes, while the portal thread calls `gc_game_list()`. Both reach `json_title()` in `game_list.c`, whose inherited 64 KiB static buffer was shared. This can return the wrong game title/name and is a real C data race when the calls overlap.
- Added `tests/test_game_list_concurrency.c` using production `game_list.c`, private fixture paths and a barrier after JSON file reads. M1-86 expected-red failed deterministically: both `CUSA00001` and `CUSA00002` received `Wired Title`.
- Fixed by extracting pure `json_title_buffer()` and giving each file read its own bounded 64 KiB heap buffer, freed after parsing. Added test-only directory/hook controls and the target to normal and sanitizer suites. No protocol parser was duplicated and no borrowed code was used.
- Verification: M1-87 normal suite and M1-88 ASan/UBSan suite PASS; new target outputs `concurrent game-title lookups keep independent JSON buffers`. M1-89 targeted TSan PASS using `make -C tests clean`, then `make -C tests CFLAGS='-std=c11 -O1 -g -Wall -Wextra -Werror -pedantic -fsanitize=thread -fno-omit-frame-pointer' build/test_game_list_concurrency`, then `./build/test_game_list_concurrency` from `tests/`; no race warning. M1-90 SDK v0.43/LLVM 18.1.8 cross-build PASS, ELF SHA-256 `e35e08672fbb61d1758d05b4ff9faee1700323c14bb2d206191f924a64086c97`, six SDK libraries inspected. These checks do not test PS5 appmeta filesystem permissions/content or portal/watcher runtime.
- M1-91 final `git diff --check`, trailing-whitespace scan and bounded credential/private-key pattern scan pass.
- Evidence tracked as F048/P2-GAME-META-28/ADR-016/T-P27. The latest implementation increment awaits publication on PR head `1179594c4555699d2346d78fb4cf389f61977f4e`; publish with expected-head guard, including all code/test/doc/memory changes. Refresh the PR body to the new ELF SHA and report any queued Actions accurately.
- Continue source audit for remaining mutable function statics called from concurrent contexts. P1-HAPTIC-26 remains open; PS5 hardware and native BLE remain blocked and untouched.


## M1 one-time VDA diagnostic race — 2026-10-08 17:22 UTC

- Source audit found `gc_main.c:inject_pad()` is called by independent USB session workers, but its `static int vdi_err_logged` was guarded by an unsynchronized check-then-set. Two simultaneous `scePadVirtualDeviceInsertData()` failures could both decide to emit the one-time diagnostic.
- M1-92 wrote and ran a barrier regression before the fix; it deterministically produced two claims (exit 1). The first corrected-target build had a missing `-I../payload` include path (M1-93); fixed the test Makefile and confirmed the target builds.
- Added `gc_diag_once_t` in `payload/diag_once.h`; `inject_pad()` claims under its mutex, releases it, then calls `gp_log()`. The 16-thread test performs 80,000 claims and verifies one winner. It does not invoke the PS5 VDA API.
- M1-94 normal suite PASS; M1-95 full ASan/UBSan PASS; M1-96 TSan PASS with exact command `make -C tests clean`, `make -C tests CFLAGS='-std=c11 -O1 -g -Wall -Wextra -Werror -pedantic -fsanitize=thread -fno-omit-frame-pointer' build/test_diag_once`, `./build/test_diag_once` from `tests/`. M1-97 SDK v0.43/LLVM 18.1.8 cross-build PASS, ELF SHA-256 `9611bfeedc525e57302bc9b6ea1d9fecaf7d2c7e1a99be302a37655d39b9b785`, six expected SDK libraries inspected. M1-98 local whitespace and bounded credential scans PASS. No target runtime claim.
- Evidence F049/P2-GC-DIAG-29/ADR-017/T-P28. Current PR #1 head `dfd8cc9baaf4b49849dc27451d0fffca956c88ed` remains open/draft/mergeable; its Host run `37815721373` and PS5 cross-build run `37815721394` were queued at 17:22 UTC with empty combined status (CI-54/55).
- Next exact action: publish `payload/diag_once.h`, `payload/gc_main.c`, `tests/test_diag_once.c`, `tests/Makefile`, architecture/test plan and memory to draft PR #1 using expected-head guard `dfd8cc9baaf4b49849dc27451d0fffca956c88ed`; update PR body to the new ELF SHA and status. Then continue auditing remaining worker-shared state. Hardware and native BLE gates remain closed.


## M1 legacy wired Steam stick state isolation — 2026-10-08 17:29 UTC

- Audited the still-supported legacy wired PID `0x1102`. At pinned Puckbridge base `78460113ef7c48502c765a1f83ba34d53c977134`, source blob `7131ad17e158b7892ea0deecf6f6de016d8d375c` used function-static `g_lx/g_ly` because bytes 16–19 alternate between left-stick and left-pad samples. Concurrent USB reader threads could let a fresh device inherit another reader's stick history.
- Initial regression build missed `-Ihost_stubs` for SDK USB headers (M1-99); fixed the test target. Expected-red M1-100 then failed (exit 1): fresh left-pad-only session returned `x=206` instead of neutral 128.
- Fix: added caller-owned `steam_legacy_input_state_t` to `steam_handle_packet()` and embedded it in `usb_session_state_t`; `usb_hid_thread()` clears it as part of its existing session initialization. Updated the wired PID call site and host Makefile/test.
- M1-101 fixed sequential/concurrent decoder checks pass (two workers × 10,000 reports); targeted ThreadSanitizer passes without warnings. M1-102 full normal suite PASS; M1-103 full ASan/UBSan suite PASS. Both report 873 numbered checks, the existing stress coverage and 200,000 parser samples, plus 20,000 concurrent legacy decoder calls.
- M1-104 PS5 SDK v0.43/LLVM 18.1.8 cross-build PASS without warnings; ELF64 x86-64 PIE, six expected SDK dependencies, SHA-256 `8d8776d9c0eb9006151ebacc4a041b1d1d236aaf3881484e72731a0fcce3af0d`. This is compile/link evidence only. A first `llvm-readelf` path under the SDK failed; retry via `/opt/homebrew/opt/llvm@18/bin/llvm-readelf` succeeded.
- M1-105 `git diff --check`, checked-file trailing-whitespace scan, and bounded credential-pattern scan PASS. No borrowed source. No physical PS5/controller/Puck/DualSense and no active BLE/HCI.
- Records: F050, P2-LEGACY-STICK-30, ADR-018 and T-P29. PR #1 source commit `9b22caa2d633af4bc66b0d5e3c93772ca5e80ce3` contains the fix; the project-memory publication sync is the remaining remote update. At 17:34 UTC the PR is open/draft/mergeable; no workflow runs/statuses were returned for this commit (CI-58/59), so no CI pass is claimed. P1-HAPTIC-26 remains open.
- Next exact action: publish the project-memory sync against expected PR head `9b22caa2d633af4bc66b0d5e3c93772ca5e80ce3`, refresh PR validation with ELF SHA `8d8776d9c0eb9006151ebacc4a041b1d1d236aaf3881484e72731a0fcce3af0d`, inspect actual current-head checks, and continue auditing shared haptic queue ownership. Hardware and native BLE gates remain closed.


## 2026-10-08 17:39 UTC — README and contribution workflow

- Reworked the README as a concise project landing page with development status, milestone evidence, architecture, exact host/SDK build commands, conditional ELF loading instructions, repository map, safety limits, and attribution. Source claims are separated from host/build/hardware evidence; no physical-console compatibility is claimed.
- Added contributor expectations and a GitHub pull-request template covering focused scope, expected-red regression tests, exact commands/results, hardware evidence labels, project-specific Bluetooth/system safety gates, and third-party provenance. Used official GitHub documentation on contributor guidelines and PR templates as references.
- Validation: `git diff --check` and local Markdown-relative-link checks pass. No tests/build run for a documentation-only increment; prior code verification remains M1-102 through M1-105.
- No code, assets, or third-party source imported. The install/load recipe documents existing `payload/Makefile` behavior only; it is explicitly marked unverified on a physical PS5.
- Published the documentation increment as `5656b9fc2b54d8f0313b25924a072315e08a799b` (parent `9b22caa2d633af4bc66b0d5e3c93772ca5e80ce3`, tree `2f83bc089fb62993a222fb26565ca33d87d1d258`); all seven uploaded blob SHAs matched local Git IDs. The guarded fast-forward used expected head `9b22caa2d633af4bc66b0d5e3c93772ca5e80ce3` and `force=false`. PR #1 remains open/draft/mergeable; its body now includes the README/contribution docs, latest tests, ELF SHA, and unresolved hardware gates.
- At 17:41 UTC GitHub queued Host workflow `37818501734` and PS5 cross-build `37818501754` for head `5656b9fc2b54d8f0313b25924a072315e08a799b`; combined status was empty. No CI pass is claimed. No software test/build was rerun for docs-only changes.
- Next: publish this publication-record sync against expected PR head `5656b9fc2b54d8f0313b25924a072315e08a799b`. Then inspect current Actions and continue source audit plus expected-red host coverage for P1-HAPTIC-26. Hardware/BLE gates remain unchanged.


## 2026-10-08 17:53 UTC — Constrain SC2 haptic queue ownership

- Audited P1-HAPTIC-26: multiple lazy SC2 USB/Puck readers could be started, while menu/game-hook/trigger/portal producers target one process-wide pulse queue and any service could drain it. The product support policy is now explicitly constrained to one active SC2 USB/Puck reader until every producer carries a verified session route.
- Added generation-token exclusive ownership in `sc2_haptics.c`. Manager claims before worker startup; a second SC2 token is rejected. The service requires the owning token, stale teardown tokens cannot clear a newer generation, release clears queue/rumble state and best-effort sends the zero-rumble report. This keeps the existing shared producers and avoids duplicating a queue or protocol stack.
- M1-106 expected-red confirmed the anonymous queue let a non-owner send the first session's pulse. Added ownership/rejection/stale-release/queue-cleanup tests; strengthened the fake USB sender to inspect zero motor values on rumble stop. T-P30 now covers the singleton policy and teardown signal.
- Full normal and ASan/UBSan host suites pass 889 checks; haptic-specific test passes 28 checks and 4,000 concurrent service calls. Targeted TSan passes without findings. SDK v0.43 + LLVM 18.1.8 cross-build is warning-free; ELF SHA-256 `7149999a34912ef6b4b5d055b7338161c9fd9f1387d891e7f907b460a0cb170b`, six SDK imports. `git diff --check`, README/contribution local links, and bounded credential scan pass.
- Updated README, architecture and test plan to state the single-session limit. Added F051, ADR-019, T-P30 and changed P1-HAPTIC-26 from open design to in-progress under the enforced constraint. No borrowed source or new provenance row. No hardware, PS5 runtime, live USB, physical haptic or BLE/HCI test.
- Publication next: guarded commit of `payload/sc2_haptics.c/.h`, `payload/gc_main.c`, `tests/test_sc2_haptics_queue.c`, README, architecture/test plan, and project-memory changes against last confirmed PR #1 head `38e05de61c3e78c41b52fd19515318cd1bdcf05b`; refresh PR body and inspect Actions.
- Published as PR #1 commit `c93ad1a282094bf43103b8f2f533d0d034f2aaad` (parent `38e05de61c3e78c41b52fd19515318cd1bdcf05b`, tree `15d56eba5521a39eb42cb2501ad9faa54c2d1bf6`). All 13 uploaded blob IDs matched local Git IDs; `fix/p0-safety` fast-forward used expected head `38e05de61c3e78c41b52fd19515318cd1bdcf05b` and `force=false`. The PR body now says haptics are constrained to one SC2 reader. PR #1 was open/draft/mergeable on the new head. At 17:56 UTC Host workflow `37820479723` was queued; the connector returned no PS5 cross-build run and empty combined status. No CI pass is claimed.
- Next: recheck the current host workflow result and continue the source audit of shared Steam session state. Keep the draft open because PS5 runtime, live USB/Puck and physical haptics are unverified; native BLE remains gated and no radio command was run.


## 2026-10-08 18:03 UTC — Prevent stale profile-selection commits

- Continued the audit of process-wide profile, menu, gyro and status state. `sc2_menu.c` state is consumed by the sole SC2 reader and its cross-thread open/close flags use atomics; profile APIs are concurrently called by the watcher, authenticated portal and input/menu path. The remaining issue was logical ordering, not an unlocked memory access.
- Expected-red T-P31 blocked an older production `sc2_select_reload()` after disk load, let `sc2_select_override("new")` finish, then released the old request. The stale profile became active again, reproducing the defect in `payload/sc2_profile.c`.
- Fixed with a generation captured alongside title/override state. Override, reload, title and title-name mutations advance it. File lookup stays outside `g_sel_lock`; before commit, the old operation revalidates the generation under that lock and updates active profile plus selected ID together. Stale work is discarded. Added a deterministic test-only barrier and a temporary `SC2_DIR` override without writing to `/data`.
- Full normal and ASan/UBSan suites pass 899 checks. TSan passes profile-selection generation (10) and profile parser (33). SDK v0.43/LLVM 18.1.8 cross-build passes; ELF64 x86-64 PIE SHA-256 `b363bc6ded7e3b887355005a21fc33e76d2077dc6890e7d2b5acf340c492dc77`, six SDK imports. `git diff --check`, README/contribution links and bounded credential scan pass.
- Updated README test count, architecture contract, verification matrix, and F052/P1-PROFILE-31/ADR-020/T-P31/project snapshot. This work is not yet published; expected branch head is `20a2d4acfd23573279a4022beaf5a145815ddbd2`. No code or assets were borrowed.
- The haptic source commit `c93ad1a` and memory sync `20a2d4a` are published. GitHub returned both c93 workflows as queued at 18:00 UTC: Host `37820479723` and PS5 cross-build `37820479786`, combined status empty. PR #1 remains open/draft/mergeable; no GitHub CI pass is claimed.
- No console, controller, Puck, actual portal/watcher integration, or BLE/HCI test. Next: publish this profile-selection fix/docs/memory to PR #1 using the expected-head guard; update the PR body, inspect new Actions, then continue the source audit.
- Published the profile-selection change as `9afe1f7a306daa029026602ce897404359d16942` (parent `20a2d4acfd23573279a4022beaf5a145815ddbd2`, tree `6f981f85f08dfff0036eac8a6938b8d03d634ef7`). All 13 uploaded blob SHAs matched local Git objects; branch fast-forward used expected head `20a2d4acfd23573279a4022beaf5a145815ddbd2` and `force=false`. PR body refreshed; PR #1 open/draft/mergeable. At 18:06 UTC Host `37821600441` and PS5 cross-build `37821600377` were queued, combined status empty; no CI pass claimed.
- Next audit exact target: `sc2_store_save()` and its shared `.tmp` path. Establish concurrent write/reload behavior from production source and test before changing it. Hardware and BLE gates remain closed.


## 2026-10-08 18:14 UTC — Serialize profile storage mutations

- Resumed from the current project-memory snapshot, checked the draft PR head and current queued workflows, and audited all production `sc2_store_save/load/delete/list` call sites. Menu, profile watcher and portal can overlap; save used a shared truncating `.tmp` path while delete could race the final rename.
- Added `tests/test_sc2_profile_store.c` against production `sc2_profile.c`. The expected-red test observed two writers concurrently entering the unguarded mutation region. Added one process-wide mutex across save and delete, checked `fclose`, cleaned temporary files on failure, and retained atomic rename so readers can stay outside the mutation lock.
- Normal suite and ASan/UBSan pass 921 checks. Targeted TSan passes all 22 storage checks. SDK v0.43 + LLVM 18.1.8 cross-build passes with ELF SHA-256 `821ba5da9cff951612bfae6c06580c2ed02680b2f5c94f5fc2a98c180d703379`. One initial test compile failed due to the shared test macro and one build invocation resolved the SDK path from the wrong directory; both were corrected and preserved in `TEST_LOG.md`.
- Added F053, P1-PROFILE-32, ADR-021, T-P32 and updated README/architecture/test-plan evidence to 921. Independently edited stale snapshots remain last-writer-wins; no browser, PS5 filesystem, runtime or hardware claim is made.
- Final `git diff --check`, changed-document relative Markdown-link validation, and bounded credential/private-key pattern scan pass. No borrowed code/assets were added, so provenance is unchanged.
- Next: publish the source/test/docs/memory increment against PR #1 head `203632e26f85636ab22d20a9ec7e2f5a1db59638` with expected-head protection, refresh the PR description, and recheck Actions.
- Published the source/test/docs increment as `085357422014aa658dcba47298d90a3b1a96b978` (parent `203632e26f85636ab22d20a9ec7e2f5a1db59638`, tree `16d152b8d7e602ab72e1491171462b010b1dcaeb`). All 13 blobs matched local Git object IDs; the branch update used expected-head protection and `force=false`. PR #1 remains open/draft/mergeable; its body was refreshed. The current-head workflow lookup returned no run records/statuses.
- Next: inspect adjacent profile mutation/error paths; profile tests still do not cover browser/PS5 filesystem integration or merge stale edits.


## 2026-10-08 18:22 UTC — Report on-console remap persistence failures

- At session start, reconciled the current branch against PR #1: source commit `085357422014aa658dcba47298d90a3b1a96b978` was followed by memory sync `f9b175857eeafa5f17146b44afdaef5df57dcd09` (tree `7f9a0a2e23bb28cecda3f893787855561cfb3792`), with three matching memory blob IDs and guarded `force=false` branch update. Host `37823299009` and PS5 build `37823298954` were queued at 18:18 UTC; combined status was empty.
- Continued the adjacent profile-path audit after publishing storage mutation serialization. Found `sc2_menu.c::do_remap()` ignored a failed save and still reloaded and told the user “Saved.”
- Added a production-module menu/store regression. The expected-red case put a directory at the temporary filename so `fopen()` fails; before the fix, the menu still sent success. The first harness link also caught a missing test-only profile-selection hook stub; documented and corrected it.
- The menu now shows an explicit storage error and stops before reload/success haptic. The targeted test passes 9 checks; full normal and ASan/UBSan suites pass 930. SDK v0.43/LLVM 18.1.8 cross-build passes, warning-free, ELF SHA-256 `ce13d1d28bc2d4f2c7d799b623a1f7c7b9c05f551142a743fc6f6f5cdcd8cab0`, compile/link only.
- Updated the architecture, test plan, README count, F054/P2-PROFILE-33/ADR-022, and active snapshot. No borrowed code/assets; no provenance edit. No physical console/controller/Puck or BLE/HCI test.
- Final review: `git diff --check`, 19-document local Markdown-link validation, and bounded private-key/GitHub-token scan pass. No third-party code/assets were introduced.
- Pending: publish the narrow menu increment with a guarded fast-forward against PR head `f9b175857eeafa5f17146b44afdaef5df57dcd09`, refresh PR body and inspect current Actions; keep draft open because runtime/hardware gates remain.
- Published the menu fix in PR #1 commit `a3b373c281d3a3abc4d7333b7accaf555855647a` (parent `f9b175857eeafa5f17146b44afdaef5df57dcd09`, tree `fd13358a707fcf9119015912b81aca1f8b9aa996`); all 12 uploaded blob IDs matched local Git objects and the branch update used expected-head protection, `force=false`. The PR remains open/draft/mergeable and its body is refreshed to 930 checks and ELF SHA `ce13d1d28bc2d4f2c7d799b623a1f7c7b9c05f551142a743fc6f6f5cdcd8cab0`.
- At 18:27 UTC, Host runs `37824436400`/`37824430821` and PS5 cross-build runs `37824436462`/`37824430827` were queued for the current head. Combined status is pending with no completed statuses; no CI pass is claimed.
- Reconciled active issue/finding/decision records: profile-selection publication (`9afe1f7`), profile-store publication (`0853574`), haptic-owner publication (`c93ad1a`), and hand-off/status/game-hook/gyro publication (`e01055e`) now show their published commits. Historical session snapshots remain unchanged.
- Next: recheck the workflows for source commit `a3b373c281d3a3abc4d7333b7accaf555855647a`, then continue the next safe source-evidenced audit. Hardware and BLE gates remain unchanged.
- Published the six-file memory follow-up as PR commit `34c2d41dd815f2b99d93a812ae6a5674720c3784` (parent `a3b373c281d3a3abc4d7333b7accaf555855647a`, tree `706aa7b63d1d5e852de0b1e7083d9efdefe80cae`); all six memory blob IDs matched local Git object IDs, and the branch update used the a3 expected-head guard with `force=false`. It reconciles older active issue/finding/decision records with already published commits.
- At the post-sync check, PR #1 was open/draft/mergeable at `34c2d41`. Host runs `37824753479`/`37824747089` and PS5 cross-build runs `37824753426`/`37824747072` were queued; combined status was pending and no CI pass is claimed.

## M1 portal request deadline and publication — 2026-10-08 18:48 UTC

- Audited the pinned Puckbridge request loop and confirmed the serial portal server used only a 2-second idle receive timeout. Extracted deterministic production-code coverage for a slow-drip header, a completed read after the deadline, and a slowly delivered body. Both slow-drip and late-completion regressions were observed failing before their respective fixes.
- Implemented a five-second absolute monotonic request deadline with a bounded two-second socket wait, clock checks before/after receive, and explicit 408 handling. Targeted HTTP/auth coverage is 65 checks (8 request-deadline assertions); full normal and ASan/UBSan suites pass 938 checks. SDK v0.43 / LLVM 18.1.8 compile/link passes; output ELF SHA-256 `95d58142bbbd71c1019f742c0a0dcf0c3a15b209903623302ef1c95ed5990d1f`.
- Published deadline framing/tests/docs/memory as `98adfdf56f3288c549abb746b95d53b57cf11081`, then verified the parent server still called its prior local reader. Immediately published `4734ffbe869c54222f5b3e20b5c78c598253faa3` to wire the deadline reader into the sequential server. Both branch updates were guarded by the expected head and used `force=false`; all 12 source/doc/test/memory blob IDs matched local Git hashes.
- PR #1 remains open, draft, mergeable. At 18:48 UTC Host run `37827118365` and PS5 cross-build run `37827118307` were queued for final source head `4734ffbe869c54222f5b3e20b5c78c598253faa3`; combined status had no completed checks. No GitHub CI pass is claimed.
- Hardware/firmware boundary remains unchanged: no physical PS5, Steam Controller, DualSense or Puck; no BLE/HCI/radio/firmware action.
- Next: check the current PR head and queued Actions runs. Then select the next safe source-evidenced P0 audit; keep hardware runtime and native BLE gated.

## M1 persisted game-hook settings and publication — 2026-10-08 18:59 UTC

- Resumed from the 18:48 snapshot. PR #1 was open/draft/mergeable at memory head `e2059116442e0d195b8aacb9b4baf4eb13737e3e`; Host run `37827399126` and PS5 cross-build run `37827399071` were queued, with no completed combined status. Audited pinned `payload/game_hooks.c` settings parsing and reproduced malformed local config values enabling hooks through permissive `atoi` parsing.
- Added a regression first; the expected-red production-module test returned JSON with `mode:1,input:1,vibration:1` for malformed values. Replaced `atoi` with checked complete-value `strtol` parsing, exact 0/1 boolean validation, default retention for malformed fields, and the previous fallback for syntactically valid out-of-range modes.
- Targeted game-hook test passes 61,004 checks/operations under normal and TSan. Full normal and ASan/UBSan suites pass 942 numbered assertions. SDK v0.43 / LLVM 18.1.8 cross-build passes with ELF64 x86-64 PIE SHA-256 `0f9e78bc4be21a60b2dfde0ab76da6b74e04fb1ae10edd3aa4df6c741a45aab2`; compile/link evidence only.
- Updated the README test/progress evidence, architecture description, test plan and build hash; added F056/P2-HOOK-34/ADR-024/T-P35 and the exact M1 records. `git diff --check`, whitespace/newline checks and local Markdown links pass; bounded secret-pattern scan has no matches. No borrowed code/assets; no provenance change.
- Published six source/test/docs paths as `9235825db8425e4bb335396fcc38e83daf3c3aa7` (parent `e2059116442e0d195b8aacb9b4baf4eb13737e3e`, tree `f1b6920bec1b0a06640a69eb3f6039ce448354ec`). All six remote blob SHAs matched local Git object IDs, branch update used the expected-head guard and `force=false`. PR #1 remains open/draft/mergeable. At 18:59 a fresh lookup for this source commit returned no Actions run records and no completed statuses; no GitHub CI pass is claimed.
- No physical PS5, Steam Controller, DualSense or Puck; no BLE/HCI/radio/firmware action. Next: publish the memory refresh, update the PR summary with the new 942-check evidence/hash, recheck the new PR-head Actions, then continue a safe source audit.

## M1 portal response deadline — 2026-10-08 19:05 UTC

- The prior project-memory sync was published and PR #1's description was updated to the 942-check/game-hook settings evidence and ELF hash. Rechecking its then-current head `793282dac5005df5a09bd86d93b4401c76da8211` still returned no workflow-run records and no completed combined statuses; no CI result was inferred.
- Continued the safe P0-C network audit against pinned Puckbridge `78460113ef7c48502c765a1f83ba34d53c977134`. Its `payload/webui.c:41-47` `send_all()` loop repeated `send()` after any positive partial write; the accepted socket's `SO_SNDTIMEO` was only two seconds per call. The portal is sequential, so an active slow reader could keep the only server thread busy even while fetching a public static page or a multi-megabyte icon.
- Test-first: a new deterministic write-framing regression made one byte of progress and advanced its fake monotonic clock by 100 ms per call across response header/body chunks. Against the unbounded extracted loop, two assertions failed among 69: the response was accepted and fully sent beyond the 250 ms deadline.
- Added `webui_http_write_chunks()` with one absolute five-second deadline over the complete response, pre/post-send monotonic checks, remaining-budget propagation, and a two-second per-socket-call cap. Wired the common reply helper and the icon path to it. Seven new HTTP assertions cover expiry across chunks and successful partial writes.
- Targeted HTTP/auth coverage passes 72 checks; full normal and ASan/UBSan suites pass 949 assertions. SDK v0.43 / LLVM 18.1.8 cross-build passes, ELF SHA-256 `b1731cdbbc877328d42c434d6e49eda60a8e7759f55fe55569a7fd0c51dbb6a3`; compile/link only. Updated README, architecture, test/build guidance and F057/P0-HTTP-09/ADR-025/T-P36. No borrowed source/assets.
- Physical PS5, Steam Controller, DualSense and Puck remain unavailable; PS5 socket semantics and portal runtime remain unverified. No Bluetooth/HCI/radio or firmware action. Next: final-check and publish source/docs to PR #1 against its current expected head, refresh the PR summary, check Actions, and continue with a safe source audit.
- Published the source/docs increment as `2c0b065cefc1287edcb20b62e48680b357a06af9` (parent `793282dac5005df5a09bd86d93b4401c76da8211`, tree `65c47fa6c8cecba6e3abc66bbb03c5e24a845aef`). All eight uploaded blob IDs matched local Git object IDs; the expected-head guard succeeded with `force=false`. PR #1 remains open/draft/mergeable. At 19:06, no Actions runs or completed status were returned for this source head. Next: publish the memory refresh, update PR validation details and recheck Actions.

## M1 exact portal route and query parsing — 2026-10-08 19:17 UTC

- Re-read current project memory, specification P0-C, portal architecture/test plan, working tree and PR #1. PR was open, draft and mergeable at `b72c4acdef41bd57ed269b7eaf3c2ea67652d2b2`; current Actions had last been queried at 19:06 with no run records or completed statuses.
- Audited pinned Puckbridge `78460113ef7c48502c765a1f83ba34d53c977134`. `payload/webui.c` routed with path-prefix `strncmp` and extracted `id`, `on`, `kind`, `via`, hook settings and `prev` using `strstr` on the complete URI. That permits route aliases and substring-name ambiguity; current derivative still checks bearer auth on `/api/` before dispatch.
- Added production `webui_route.c/.h`, wired all portal requests through exact route resolution and per-route query allowlists, rejected duplicate/unknown/malformed/non-ASCII query input, added bounded value copies, and enforced explicit request methods. Action parameters are range-checked before domain mutations. Existing UI endpoints/fields are preserved.
- Test-first regression initially failed 4/17 checks for an `/api/pause` suffix alias, `notid=`/`id=` substring collision, duplicate key, and unsupported query key. After strict parsing, targeted route coverage passes 54 checks. Full `make clean test` and `make clean sanitize` both pass 1003 assertions. PS5 SDK v0.43/LLVM 18.1.8 cross-build passes with ELF SHA-256 `dc14fcc28673d50223c50dccff9f29533132166a7c8adbe817c7907206e17263`.
- Updated README/architecture/build/test guidance and added F058, P0-HTTP-10, ADR-026, T-P37 and M1-163–168. `git diff --check`, a 10-file whitespace/newline scan, 23 relative Markdown links, and bounded secret-pattern checks pass. No external source/assets borrowed.
- No physical PS5/controllers/Puck; no PS5 socket/browser or hardware runtime. No BLE/HCI/radio/firmware action. Next: publish the implementation/docs to draft PR #1 with exact local-to-remote blob verification and an expected-head guard; then publish memory, refresh the PR body, inspect Actions and continue another safe P0 audit.
- Published source/docs commit `0b0e85e3533210ced84cb60c3c4623978b3680ed` (parent `b72c4acdef41bd57ed269b7eaf3c2ea67652d2b2`, tree `aef2287b88e630ce167478cb2e1ae3e021bd3f1c`) across ten paths after all local/remote blob SHAs matched. The `fix/p0-safety` update succeeded against the expected head with `force=false`. PR #1 stayed open and draft; a follow-up info refresh showed it mergeable. Its body now includes the exact-route change, 1003 checks and current ELF hash. GitHub returned no Actions runs or combined statuses for source commit `0b0e85e` at 19:19 UTC; no CI pass is claimed. This memory refresh is published alongside the implementation. Next: recheck Actions for the latest PR head and continue safe portal/network audits. Hardware and BLE gates remain unchanged.
- Rebuilt the published route-header tree after finalizing its contract comment; the SDK build succeeded again and produced ELF SHA-256 `2704d75b96c0c5db5756935ebb1106c7a376965242242ebc4f5965c8648d486c`. Updated the build record and published `docs/BUILDING.md` as `9781845811d30b3e7ad04342339537abf86b89d9` (parent `0b0e85e3533210ced84cb60c3c4623978b3680ed`, tree `b684f2f723083c73838085a08cc339815a3583c5`); blob matched and guarded ref update succeeded. PR body now reflects the final hash. GitHub returned no Actions runs or combined statuses for `9781845` at 19:23 UTC; no CI pass is claimed. Next: publish the memory refresh, recheck the new PR head and continue safe P0 audits.


## M1 portal Origin and profile-body policy — 2026-10-08 19:29 UTC

- Re-read the current top project snapshot and inspected the exact portal request parser, `/api/profile` handler, and browser `api()` helper. At Puckbridge pin `78460113ef7c48502c765a1f83ba34d53c977134`, missing Content-Length was treated as length zero; profile POST then parsed an empty body, saved the existing/default profile, and returned success. The derivative already gates `/api/` with a private bearer token, so this was not an anonymous mutation path.
- Added 27 request-policy regression assertions before the helper implementation. The first build could not link the newly specified functions; after implementing them, 99 targeted HTTP/auth checks pass. The production socket handler remains target-only and was not directly host-tested.
- Implemented unique bounded Host/Origin/Content-Type parsing, optional same-host Origin validation (bearer token remains primary), and strict nonblank UTF-8 text profile body requirements. Kept Origin optional for non-browser clients. Updated README, architecture, build record, test plan, issues, findings and ADR-027.
- `make -C tests test` PASS (1030 checks); `make -C tests clean sanitize` PASS (same 1030 checks); SDK v0.43/LLVM 18.1.8 cross-build PASS, current ELF SHA-256 `1084457bba0e35a479b6bed5e0578027567decc199d0d3ae6724c301e74cd6a1`. These are host and compile/link results only.
- No physical console/controllers/Puck were available; no Bluetooth/HCI/radio/firmware action was run. Next: rerun targeted checks after documentation edits, run final whitespace/links/secret review, publish the source/docs commit using the freshly observed PR head as an expected-head guard, then publish the memory sync and check Actions. Continue safe P0 audits after publication.

- Published source/docs commit `7f1bb0ac71115c170190264802fab0a70321a8a7` (parent `1ac94ecdb903cae2d38c5b8ad8b8830eb6e94803`, tree `761768f9646c7be252e0e0e156b64d454e8b0a88`); all eight blob SHAs matched local IDs and the expected-head update succeeded with `force=false`. PR #1 is open/draft/mergeable and its body now reports 1030 local checks and ELF `1084457…`. At 19:35 no workflow runs or completed statuses were returned for this commit. Next: publish memory/provenance, recheck Actions, then continue the next safe portal-network audit.

- Memory/provenance sync `c31bb540876302d4c80b1bd48e8117e4815276a3` (parent `7f1bb0ac71115c170190264802fab0a70321a8a7`, tree `7a33007114b37d41714b1107ef0cbb673e91cbd4`) published six memory files plus provenance. All seven blob SHAs matched; expected-head branch update succeeded with `force=false`. PR #1 remains open/draft/mergeable at c31bb54; no Actions runs or combined statuses were returned at 19:37 UTC. Next: recheck current Actions state and continue auditing the portal bind/auth boundary; no hardware/BLE interaction.

## M1 portal IPv4 peer scope — 2026-10-08 19:43 UTC

- Reconciled the local workspace, project memory and PR #1. GitHub reports PR #1 open/draft/mergeable at `52695ac9d5b273ff46b80c48f33375d2853c34f3`. The local Git branch still points to the pinned upstream base `78460113ef7c48502c765a1f83ba34d53c977134`; the previously published project snapshot is represented by the uncommitted local tree and remote API commits, so publication must use the remote PR head/tree and an expected-head guard.
- Continued the P0-C audit against pinned Puckbridge `78460113ef7c48502c765a1f83ba34d53c977134`. Its `payload/webui.c:288-301` binds `INADDR_ANY` and calls `accept(s, NULL, NULL)`. The derivative already requires bearer authorization on `/api/`, but did not scope peers before handling requests.
- Test-first: 18 address-policy checks were added. Before implementation, the peer-policy test target failed to link for the missing production function. Implemented the pure IPv4 predicate and wired `accept()` to obtain the peer and close non-loopback/non-RFC1918/non-link-local addresses before request parsing, with no address/PII in logs. The listener still binds all IPv4 interfaces.
- Updated README, architecture, test plan, build record, P0 issue list and project memory. The policy narrows peers but does not add TLS, replace bearer auth, bind one interface, establish NAT behavior or provide console network evidence. Browser/PS5 runtime and hardware remain unverified.
- Normal and ASan/UBSan suites each pass 1048 checks and deterministic parser samples. SDK v0.43 / LLVM 18.1.8 cross-build passes with ELF SHA-256 `03c1cbdaa64b8d9d222329c51f5fe24222cda464250a4c6f52644741fa3a1948`; compile/link only. No hardware, BLE/HCI/radio or firmware operation was run.
- Source/docs publication: ten sequential GitHub Contents API commits updated the three new files, two Makefiles, `payload/webui.c`, and README/architecture/build/test docs; commit SHAs are recorded in `TEST_LOG.md` M1-183. Every remote blob SHA matched the local `git hash-object` output. PR #1 is open/draft/mergeable at `88898a121d0e0df41101c9e3ee4386c8569d651c`.
- At 19:47 UTC, GitHub returned no Actions run records or completed combined statuses for source head `88898a1`; no CI pass is claimed. Final whitespace/newline check, nine-Markdown-file local link check and bounded credential-pattern scan pass. P0-NET-11 remains target-unverified until Prospero socket/network behavior can be tested.
- Next: publish the finding/decision/test/session memory sync, update PR #1 validation details, check Actions on the memory-updated head, then continue a safe audit of `payload/webui_auth.c::webui_auth_load_or_create` (randomness, permissions and public index exposure). Do not touch BLE/HCI/radio or perform firmware/system writes.

- Memory publication completed in six sequential GitHub Contents API commits: F060 `da8f19635ec2184c96601e29cb69093871d94c69`; issues `e695c8a5e063f8ac3eb738ba7cf7ef4b17c2abcc`; ADR-028 `65b54d867bf29133b77c578d065da3ce212b67f3`; tests `86981e2f53dd61e2d4a5b01ad55efa31d1d78b19`; session log `e9319a2041603da135e4e991004508c1f068a84d`; snapshot `c848ad8882c07c0a401a1115ed4c16270990f33c`. All six blob SHAs matched local Git object IDs. PR #1 remained open/draft/mergeable at `c848ad8`; description updated for the 1048 checks and peer-scope change. Actions and combined status lookups for `c848ad8` were empty; no CI pass is claimed.

## M1 portal token-storage audit — 2026-10-08 19:54 UTC

- Reviewed the current PR source at `88898a121d0e0df41101c9e3ee4386c8569d651c`: `payload/webui_auth.c:42-70,73-173`, `payload/webui.c:177-208,456-466`, and UI token handling in `payload/web/index.html:580-605`. The fixed-length bearer token uses 32 bytes from `/dev/urandom`, an owner-validated `0700` directory, `openat(...O_CREAT|O_EXCL|O_NOFOLLOW,0600)` for new token files, strict existing-file validation, and fail-closed startup. `webui_auth_path_allows()` exempts the static index but requires auth for `/api/`; exact routing and methods prevent public GET routes from performing mutations. The browser stores the token in `sessionStorage` and the code does not log its value.
- Existing token-lifecycle tests cover create/reload, mode rejection, malformed contents and symlink rejection; they were included in the just-passed 1048 normal and ASan/UBSan checks. This read-only source audit found no new defect, so no implementation or test was added.
- Recorded F061 and expanded P0-SEC-01/architecture notes. Target firmware support for `/dev/urandom`, `openat`/`O_NOFOLLOW`, filesystem permissions and live browser behavior remains unverified; HTTP bearer credentials remain visible to trusted-LAN observers. No hardware, BLE/HCI/radio or firmware/system operation was performed.
- M1-186 architecture publication: `docs/ARCHITECTURE.md` was published in commit `77b03b3dac71883ba7979e20275381cd1b620047`; remote blob SHA `e7c6a9e20c4d97413240aea82893a851452140d7` matched local `git hash-object`. This docs-only commit records the audited token-storage and route-auth boundary plus the runtime/plaintext limitations; it adds no implementation or test claim. PR #1 remains open/draft/mergeable at this source/docs head.
- Next: publish F061 and the updated project-memory files, refresh the PR summary, inspect Actions for the resulting head, then continue the safe HTTP body-framing audit. Physical PS5/network behavior remains the hardware blocker.


## M1-188 GitHub tree integrity correction — 2026-10-08 20:03 UTC

- User identified the actual first destructive commit as `9235825db8425e4bb335396fcc38e83daf3c3aa7`, message `fix(game-hooks): validate persisted settings`, directly after `e2059116442e0d195b8aacb9b4baf4eb13737e3e`. GitHub compare independently confirms one commit between them, 189 changed paths (183 removed), and 43,543 removed lines; it removes core sources including `gc_main.c`, `controller_sc2.c`, `sc2_haptics.c`, `slot_registry.c`, and `steam_protocol.c`, plus license/instructions/tests/workflows. The suspected malformed Git tree mechanism remains unproven.
- Confirmed current PR head `97097c5e715c2fdd13399895dc462c4d30fda596` tree `e185d5ff44de63ff7c39ea276437329c65947456` has 27 entries/23 blobs, all matching their local counterparts, while the local manifest contains 195 files (172 absent remotely). Full preserved source is in `e2059116`, tree `d887aa466949bbb17929ce17a9b92f86469b3481` with 189 blobs. Compared with that tree, the local snapshot adds six paths (`webui_peer_policy` source/header/test and route source/header/test) and updates 19 existing blobs. `main` was confirmed at `61780b03bcef6859935cac1ef288cf3eef12d3ba` with one README file.
- This means earlier host tests and SDK build are local complete-tree evidence only, not clean-clone evidence for current PR head. PR #1 remains draft and unmerged.
- Owner explicitly instructed direct-to-main commits until the first successful release. Added ADR-029 and P0-REPO-12. Plan is to build `main` as a new fast-forward commit using the preserved `e2059116` tree and all 25 current local blobs/deltas (19 modified files and six additions), verify the full tree, validate a clean source checkout, then close the obsolete draft PR. No history rewrite/delete; do not release an unverified binary.
- M1-187 HTTP body-framing source audit completed; `make clean build/test_webui_http` then `./build/test_webui_http` in local `tests/` passes 99 checks. It does not validate the broken remote tree. No PS5 hardware, BLE/HCI/radio, or firmware/system operation was used.


## M1-189 complete tree restored directly to main — 2026-10-08 20:09 UTC

- Published complete source snapshot to `main` in fast-forward commit `0f122c64eff0b196ba023c75af9a61ba1cc9514a` (parent `61780b03bcef6859935cac1ef288cf3eef12d3ba`, tree `9834f600c9b77be4d2f7285c1b3f21fe70658e99`). Used preserved `e2059116` tree (`d887aa466949bbb17929ce17a9b92f86469b3481`) and overlaid 25 current blobs (19 modifications, six additions). No branch history was rewritten or deleted.
- GitHub recursive tree contains 195 blobs and exactly matches every local file path, mode, and SHA; no omissions, extras, or mismatches. Clean export's `git write-tree` equals the published tree SHA.
- From that exact-tree clean export, normal host suite passes 1048 checks; ASan/UBSan suite passes the same 1048; SDK v0.43/LLVM 18.1.8 build passes (ELF SHA-256 `26dce830d274755fc0bd6a83cafc8e3e6f660e756544d699a2b346ef445afbf0`). All are host/compile-link evidence only.
- True `git clone` failed because the environment could not resolve `github.com`; the clean source export has the exact Git tree hash verified through GitHub's API. GitHub push Actions are queued as run `37836992181` (PS5 cross-build) and `37836992223` (Host tests); no CI pass yet.
- Closed PR #1 unmerged and removed its task attachment; owner requested direct `main` commits until first successful release. The incomplete legacy `fix/p0-safety` branch is retained for history and must not be treated as complete.
- No PS5/controller/Puck, BLE/HCI/radio or firmware operation; no unverified binary release. Next: inspect the two push workflows, then continue safe USB/Puck lifecycle source audit in `controller_sc2.c` and `gc_main.c`.


## M1-190 validation record published to main — 2026-10-08 20:11 UTC

- Published the complete-tree validation record, current P0-REPO-12 resolution, and build guidance directly to `main` in commit `c87ff0e2bdbee41549b197a70ec5da6f70ffbd84` (parent `0f122c64eff0b196ba023c75af9a61ba1cc9514a`, tree `e03ee1fab971b0f1dd3430923925f485fb81e385`). Every changed blob SHA matched locally, and GitHub confirms 195 files still match the local tree exactly.
- `git diff --check` passed; no source code changed, so the M1-189 clean-export test/build results still apply to the exact payload tree.
- Current push workflow runs are queued: `37837501810` PS5 cross-build and `37837501974` Host tests. No CI pass is claimed. PR #1 is closed unmerged.


## M1-191 SC2 endpoint-scan cleanup hardening — 2026-10-08 20:19 UTC

- Continued the owner-directed USB/Puck lifecycle audit in `payload/controller_sc2.c` and `payload/gc_main.c`. The active-endpoint scan performed raw OPEN/START/COMPLETE/STOP/CLOSE ioctls, while the owning `usb_resources_t` remained empty and STOP/CLOSE failures were ignored. A failed CLOSE could therefore be followed by normal FS_UNINIT/reinitialization without the session cleanup path knowing an endpoint might still be open. This is a source-backed failure path; no hardware failure was observed.
- Added the fail-once CLOSE regression before implementation. The initial target build failed on the old scanner function signature; after adding the owning `usb_resources_t` contract, the test injects CLOSE `EIO` and verifies the scanner returns the error, retains the endpoint bit, and session cleanup retries.
- Added `usb_resources_note_endpoint_close()`; successful CLOSE clears the endpoint/transfer bits, while failure leaves them owned. `sc2_find_active_ep()` records scanner endpoint/transfer transitions and returns its first cleanup error. `gc_main.c` diverts retained scan resources through `usb_session_cleanup` instead of normal FS_UNINIT/reinit. The scanner still uses direct ioctls; operation-table injection is future work.
- Updated `docs/ARCHITECTURE.md`, `docs/TEST_PLAN.md`, `P0-USB-LIFE-19`, `P1-SC2-SCAN-27`, F064, ADR-030, and T-P40. No third-party code was copied.
- Normal host suite, full ASan/UBSan suite, targeted scanner TSan, SDK v0.43 compile/link, and `git diff --check` pass; details and exact commands are in `TEST_LOG.md` M1-191. Latest ELF SHA-256: `683f8581d4582827b7ca4c144d17f99919fb5c8f166c0185f2125f5579b45bb6`.
- Verification limits: no PS5/Puck/controller runtime, device removal, USB FS error behavior, BLE/HCI/radio, or firmware operation was tested. Push runs for pre-change main commit `c87ff0e2bdbee41549b197a70ec5da6f70ffbd84` were queued when last checked; recheck after the code+memory commit.


## M1-192 source and evidence publication — 2026-10-08 20:22 UTC

- Owner's correction confirmed the exact first destructive commit: `9235825db8425e4bb335396fcc38e83daf3c3aa7` (`fix(game-hooks): validate persisted settings`), directly after complete-source commit `e2059116442e0d195b8aacb9b4baf4eb13737e3e`; 189 changed paths, 183 removals, and 43,543 deleted lines. The suspected incomplete-tree construction mechanism remains unproven.
- Published M1-191 source/test/docs/project records directly to `main` in fast-forward commit `283dda3a7b0c115dbe6587e6eb1c355f37504956` (parent `c87ff0e2bdbee41549b197a70ec5da6f70ffbd84`, tree `a87604674d7a651610300707ce2d184857acee5c`). The 14 changed blob IDs all match local Git object hashes; recursive tree remains 195 files/208 entries with unchanged paths and no unintended blob or mode changes.
- GitHub Actions runs `37838885358` (PS5 cross-build) and `37838885367` (Host tests) were queued at 20:22 UTC. The prior c87 runs were still queued. No CI result is claimed. PR #1 is closed unmerged; no history was rewritten.
- Updated P0-REPO-12 and the memory snapshot; next safe source audit is remaining direct USB transfer/cleanup code in `payload/usb_helpers.c` and `payload/sc2_haptics.c`. PS5/Puck/controller runtime and all BLE/radio/firmware operations remain untested or blocked.


## M1-193 USB COMPLETE result and OUT cleanup hardening — 2026-10-08 20:34 UTC

- Reconciled the owner's exact repository-history correction: first destructive commit `9235825db8425e4bb335396fcc38e83daf3c3aa7` directly follows full snapshot `e2059116442e0d195b8aacb9b4baf4eb13737e3e`; the incomplete-tree mechanism remains unproven. F063/M1-192 already record the exact comparison.
- Inspected pinned SDK v0.43 `target/include/dev/usb/usb_ioctl.h`: `struct usb_fs_complete` returns `ep_index` at lines 170–172 and `USB_FS_COMPLETE` is `_IOR` at line 331. Session code previously cleared the caller's requested endpoint even when the driver reported another endpoint complete.
- Added the two-active-endpoint regression before the implementation. It failed on the old behavior, then passed when the session callback began reporting the actual completed index and the state tracker cleared that index. A second fail-once COMPLETE/STOP regression verifies that active ownership remains, a duplicate START is rejected, and common cleanup retries STOP.
- Routed Xbox handshake input, Xbox and Nintendo OUT sends, and SC2 haptics through the session/resource owner. A failed STOP logs the error and retains state for cleanup retry. The SC2 endpoint scanner remains a direct but state-tracked multiplexed ioctl path.
- M1-193 normal and ASan/UBSan suites both pass 1123 counted checks; SDK v0.43 cross-build passes (ELF SHA-256 `52f43e080aa171e9290441b10fe503f4a39bc8b8aac36f17bd61655145edcb22`); `git diff --check` passes. Exact commands and limits are in `TEST_LOG.md`.
- Before source publication, GitHub `main` is `3320e8d757f78c9056b8b5c6dd6812107a809e81`; local Git HEAD is pinned upstream Puckbridge `78460113ef7c48502c765a1f83ba34d53c977134`, not a descendant of main. Publish with guarded Git Data API update after verifying the full tree, and record the GitHub-acknowledged SHA.
- No PS5/Puck/controller hardware, BLE/HCI/radio or firmware operations were used. Next safe audit after publication: route the remaining direct but state-tracked multiplexed scan operations in `payload/controller_sc2.c::sc2_find_active_ep()` through an injectable platform seam without changing its multi-endpoint COMPLETE semantics. Hardware runtime remains a separate gate.


## M1-193 publication reconciliation — 2026-10-08 20:38 UTC

- Published the M1-193 source directly to GitHub `main` as `1082dad728d6d769b9e5ada1dcdf88df0357e8b1` (parent `3320e8d757f78c9056b8b5c6dd6812107a809e81`, tree `dfb41cd6c13c520cc79cbaa24ded1ee288b24e8c`) using the prior head as an expected-head guard and `force=false`.
- Re-fetched GitHub `main` and the recursive tree after the update. The ref acknowledges the published SHA; the tree contains 196 blobs/209 entries and exactly matches the complete local manifest, including the five core payload files previously called out. All 25 uploaded/changed blobs match local Git object IDs.
- Actions push runs `37840779701` (PS5 cross-build) and `37840779559` (Host tests) are queued. Combined status is empty, so no CI pass is claimed. Local host/sanitizer and cross-build evidence remains as recorded in M1-193; PS5/controller/Puck hardware, BLE, radio, and firmware runtime were not exercised.
- The owner's correction is retained in F063 and M1-192/M1-193: `9235825` is the first destructive commit after `e2059116`, and its incomplete-tree mechanism remains unproven. No history was rewritten; direct-to-main remains the owner-requested workflow until the first successful release.
- Next safe work: audit and introduce test injection for `payload/controller_sc2.c::sc2_find_active_ep()` while retaining multiplexed endpoint completion and tracked cleanup semantics. Physical-hardware validation remains blocked on access to the PS5, SC 2026, DualSense, and official Puck.


## M1-194 scanner operation-table integration and history correction — 2026-10-08 20:54 UTC

- Reconciled GitHub state before continuing: `main` is at memory-sync commit `89a3703d385d2e4208d037f423613caac45cba18`, parent source `1082dad728d6d769b9e5ada1dcdf88df0357e8b1`; its tree contains the verified 196-blob project snapshot. PR #1 remains closed/unmerged. Host-test Actions run `37841572840` and PS5 cross-build run `37841572874` for `89a3703` are queued, with no combined status; no CI result is claimed.
- The owner reiterated the first destructive commit: `9235825db8425e4bb335396fcc38e83daf3c3aa7` (`fix(game-hooks): validate persisted settings`) directly follows complete-source commit `e2059116442e0d195b8aacb9b4baf4eb13737e3e`; GitHub compare reports 189 changed paths, 183 removals and 43,543 deleted lines. The exact incomplete-tree construction mechanism is not proven. This remains recorded in F063/P0-REPO-12; no history rewrite or deletion is planned.
- Completed the planned M1-194 USB scanner seam: `sc2_find_active_ep()` now receives the reader's `usb_session_ops_t` and context, reuses shared open/start/complete-any/stop/close helpers, and refuses to scan if a transfer is already active. COMPLETE accounts whichever endpoint the shared FS queue returns before decoding; cleanup failures preserve ownership for existing session cleanup/retry. No scanner-specific operation table or duplicated lifecycle manager was introduced. ADR-032/F066/T-P42 and architecture/test-plan records describe the contract and limits.
- Updated the README progress evidence to the latest 1148 host checks. Full normal and ASan/UBSan suites PASS, each 1148 counted checks plus 340,000 concurrent operations/snapshots and 200,000 deterministic parser inputs; targeted scanner TSan PASS. SDK v0.43/LLVM 18.1.8 cross-build PASS, ELF64 x86-64 shared object SHA-256 `1af60f12c8d5f34860beb82975db609d29dc1d8e1635d1b17cc7084f134e0e78`. Exact commands and environment are appended in `TEST_LOG.md` under M1-194. `git diff --check` passes.
- The local checkout is still branch `fix/p0-safety` at pinned Puckbridge base `78460113ef7c48502c765a1f83ba34d53c977134`; local Git objects do not contain remote main commits. The validated M1-194 delta is not yet published. The ignored `steam-controller.jpg` is excluded from the source tree. Before publishing, compare every local tracked project file to GitHub's recursive 196-blob main tree, create only the verified source/docs/memory blob updates over its current tree, and update `main` with expected SHA `89a3703...` and `force=false`; then fetch and verify the acknowledged recursive tree.
- No physical PS5, controller, Puck or DualSense is available. No Prospero ioctl/hotplug, BLE/HCI/radio or firmware operation was performed; the cross-build is not runtime evidence. No binary or release was published.
- Next exact actions: (1) final review changed paths, provenance/credential patterns, and `git diff --check`; (2) publish validated M1-194 source/docs/memory directly to `main` using a guarded fast-forward, with exact blob and full-tree checks; (3) append the GitHub-acknowledged commit/tree and new Actions run state to project memory; (4) continue safe USB source review, with real PS5/Steam Controller/Puck validation still hardware-blocked.


## M1-194 source publication and memory reconciliation — 2026-10-08 21:03 UTC

- Rechecked GitHub before the documentation sync: `main` pointed to source commit `6bda48be9c544898451e60573fb8fc363fd4505e` with tree `1fce5b32bd377cb791ca42df0c2d7e29c97cd6d1`, parent `89a3703d385d2e4208d037f423613caac45cba18`. The branch was unchanged from the successful M1-194 source publication.
- Re-fetched the recursive tree: 209 entries, 196 blobs, `truncated=false`; all local paths/modes/blob SHAs match; no extras or omissions. `AGENTS.md`, `CODEX_TASK.md`, and the core payload files are present. Earlier publication checked every uploaded blob against local `git hash-object` before guarded update; the tree was verified again after acknowledgement.
- Current-head GitHub Actions are queued: `37843842873` Host tests and `37843843107` PS5 cross-build. Combined status is empty, so no GitHub CI pass is claimed. Local M1-194 host, sanitizer, TSan and SDK compile/link results remain those in `TEST_LOG.md`.
- The first destructive project commit is exactly `9235825db8425e4bb335396fcc38e83daf3c3aa7`, directly after the complete-source `e2059116442e0d195b8aacb9b4baf4eb13737e3e`; GitHub compare reports 189 changed paths, 183 removed and 43,543 deleted lines. The incomplete-tree mechanism remains an unproven hypothesis. No history rewrite was done.
- Next safe action: audit `payload/gc_main.c` and `payload/steam_usb.c` for USB descriptor identity/usage-page and lifecycle issues, add a host seam/regression before any reproducible fix, then rerun targeted host/sanitizer and SDK build gates. No PS5, Steam Controller 2026, DualSense or official Puck runtime was available; physical validation remains blocked. BLE remains gated research; no radio/HCI or firmware writes were performed.


## M1-195 SC2 configuration-map fallback hardening — 2026-10-08 21:12 UTC

- Reconciled the base before the next source change: GitHub `main` is at memory-sync commit `d4bf2acd02d46542a9b927d004be920bedd2fac3`, tree `ccbe3bc6832f79606c1819ba0897216cb4a107c4`, parent M1-194 source `6bda48be9c544898451e60573fb8fc363fd4505e`. Its Host tests `37844355107` and PS5 cross-build `37844355101` are queued; combined status is empty. No CI pass is claimed.
- The source audit found that `sc2_list_in_eps()` collapsed ioctl errors, malformed configurations, and valid Puck maps with zero eligible endpoints into `0`. `gc_main.c` treated `0` as “descriptor unavailable,” so every such condition triggered the eight-address guessed probe; ENXIO on unplug was also treated as recoverable. The config was read once for the IN list and again for OUT pairing.
- Test-first regressions reproduced the EIO collapse (`result=0`, expected unavailable status) and ENXIO collapse (`result=-1`, expected device-gone). `sc2_ep_map()` now returns distinct unavailable/invalid/device-gone statuses and one parsed snapshot; `sc2_collect_in_eps()` derives candidates from it. The compatibility scan runs only after a non-ENXIO GET_FULL_DESC error; malformed, disconnected, and valid-but-ineligible maps fail closed. The same parsed map supplies the haptic interface/OUT mapping.
- Added T-P43, ADR-033, F067 and P1-SC2-MAP-28; updated architecture, test plan, README and test evidence. Normal and ASan/UBSan suites pass (1148 counted checks, existing stress/parser inputs); targeted scanner TSan passes; SDK v0.43/LLVM 18.1.8 compile/link passes (ELF SHA-256 `87fa9f441430dd1235654e308988bf28144b2fe5a241f692ea9456f1b9d6d73b`). `git diff --check` passes. Exact commands are in M1-195 of `TEST_LOG.md`.
- The changed source/docs/memory are local in branch `fix/p0-safety`, whose Git HEAD remains the pinned Puckbridge base `78460113ef7c48502c765a1f83ba34d53c977134`; GitHub's current `main` is the verified direct target above. Next action: compare the complete local publish manifest with the 196-blob remote tree, upload only M1-195 changed paths, create a commit parented to `d4bf2acd...`, advance `main` with expected-head guard and `force=false`, then verify the acknowledged recursive tree and Actions. No PR or history rewrite.
- Hardware limits: no real PS5 ioctl/error semantics, Steam Controller 2026, official Puck or DualSense runtime; USB HID usage-page/report-descriptor verification remains open. BLE remains gated and no radio/HCI/firmware operation was run.


## M1-195 source publication reconciliation — 2026-10-08 21:16 UTC

- Published M1-195 directly to GitHub `main` as `72048849da463b1e18a9c91e07d2f64926f6e6af` (parent `d4bf2acd02d46542a9b927d004be920bedd2fac3`, tree `ac4e13add70bcf641b6c83114aa1e8d743401f8b`). The update was a guarded fast-forward with the verified prior head and `force=false`; no PR or force update was used.
- Verified the complete local manifest against the prior remote 196-blob tree: all 196 paths/modes/blob hashes matched before the change, and exactly the 15 intended files differed after the source delta. All 15 uploaded blob IDs matched local Git hashes. The acknowledged recursive tree contains 209 entries/196 blobs, is not truncated, and includes the required core sources and repository instructions.
- Source-commit Actions `37845565583` Host tests and `37845565557` PS5 cross-build are queued. Combined status is empty; no GitHub CI pass is claimed. Local full normal/ASan/UBSan, scanner TSan, SDK cross-build, ELF SHA, and regression details remain in M1-195 of `TEST_LOG.md`.
- The confirmed repository loss remains `9235825db8425e4bb335396fcc38e83daf3c3aa7` directly after complete source `e2059116442e0d195b8aacb9b4baf4eb13737e3e`, with 189 paths, 183 removals and 43,543 deleted lines. The suspected incomplete-tree mechanism is unproven. No history rewrite was performed.
- Next safe work: inspect the pinned SDK's `USB_GET_REPORT_DESC` interface and primary public descriptor evidence to determine whether usage-page validation can be implemented without assumptions. Do not access hardware or radios; actual report descriptor capture and Puck/PS5 behavior remain hardware-gated.

## M1-196 repository-tree forensic confirmation and HID descriptor evidence review — 2026-10-08 21:19 UTC

- GitHub commit API confirms the first destructive commit is `9235825db8425e4bb335396fcc38e83daf3c3aa7` (`fix(game-hooks): validate persisted settings`), parent `e2059116442e0d195b8aacb9b4baf4eb13737e3e`, child tree `f1b6920bec1b0a06640a69eb3f6039ce448354ec`. The parent tree `d887aa466949bbb17929ce17a9b92f86469b3481` has 202 entries/189 blobs; the child has 9 entries/6 blobs, containing only README, three docs, `payload/game_hooks.c`, and `tests/test_game_hooks.c`. Commit metadata reports 189 changed paths: 183 removed and six modified, 58 added lines and 43,543 deleted lines. This confirms the resulting tree is partial. Why it was created that way remains unproven; history was retained.
- The public SteamlessController default branch currently resolves to pin `1de3f7a40ec9f73b0683898e60c7cc8a5695e0b0` (2026-10-04). Its `src/steam/SteamController.h` blob `340551ffb2be6461d89e79f08f7a384c5ab36f6d` defines Valve VID/PID and usage constants: vendor page `0xFF00`, controller `0x0001`, Puck dock `0x0002`. Its LICENSE blob `bf6efcde7907313ebc0cace102b48fede413e2cc` is MIT. This is pinned source evidence, not a live descriptor or PS5/hardware validation; no code was copied.
- The pinned community PS5 SDK v0.43 header `target/include/dev/usb/usb_ioctl.h` declares `USB_GET_REPORT_DESC` as `_IOWR` and exposes generic descriptor data/length/interface/alternate fields. It contains no runtime evidence for actual ioctl behavior or Steam descriptor contents. The USB-IF HID 1.11 specification §§5.2–5.3 describes descriptor item formats and collection-scoped usages; it provides the parsing rules, not a device fixture. See S03/S10/S11, F068, T-P44, and the still-blocked P0-USB-DESC-10.
- GitHub main remains at M1-195 source commit `72048849da463b1e18a9c91e07d2f64926f6e6af`, tree `ac4e13add70bcf641b6c83114aa1e8d743401f8b`. Its two Actions runs (`37845565583`, Host tests; `37845565557`, PS5 cross-build) were checked at 21:19 UTC and remained queued. No CI success is claimed.
- Full-manifest reconciliation against GitHub main tree `ac4e13add70bcf641b6c83114aa1e8d743401f8b`: both sides contain exactly 196 blobs, with no missing/extra paths. Nine blob differences are intentional: the eight M1-196 source/evidence-memory edits plus the previously local-only M1-195 publication record in `docs/BUILDING.md`; every other path, mode, and blob matches. `docs/BUILDING.md` is included in this direct-main documentation update.
- This turn changed documentation and project memory only. No host tests, sanitizer, SDK build, USB ioctl, PS5/controller/Puck access, BLE/HCI/radio command, firmware/system write, or binary release was performed. The existing M1-195 test/build evidence remains the latest implementation verification. `git diff --check` passed after the local review.
- Next: publish this source/evidence-memory update to `main` with an expected-head guard and `force=false`, then verify blobs/tree/Actions. Keep report-descriptor integration blocked until a safe identified Steam Controller/Puck descriptor capture and target ioctl behavior are available; continue a separate host-only source audit meanwhile.

### M1-196 evidence/docs publication reconciliation — 2026-10-08 21:25 UTC

- Published the repository-tree forensic result, pinned HID usage/licensing evidence, updated architecture/test plan, and memory directly to `main` as commit `26da8ab06513fe2f64d3b034b8a8bd951cc519ef`, parent `72048849da463b1e18a9c91e07d2f64926f6e6af`, tree `bff654ef29907a726937f6198986073a26c3f9ce`. The expected-head guard matched and the ref update used `force=false`; no PR or history rewrite was used.
- All nine changed blob IDs matched local Git hashes. Re-fetched `main` and recursive tree: commit and tree match, 209 entries/196 blobs, `truncated=false`; every local path, mode, and blob hash matches with no missing/extra items.
- Actions `37846617692` Host tests and `37846617695` PS5 cross-build are queued; combined status is empty. No CI pass is claimed. These are documentation/evidence changes; M1-195 remains the latest code validation (1148 host checks, ASan/UBSan, targeted scanner TSan, SDK v0.43 compile/link).
- Next: monitor the queued actions and continue a host-only audit of `payload/steam_usb.c::steam_usb_parse_config()`. Live HID report-descriptor access remains blocked until a safely identified Steam Controller/Puck and target ioctl evidence are available.


## M1-197 USB configuration descriptor count validation — 2026-10-08 21:31 UTC

- Repository/local state: branch `fix/p0-safety`, baseline commit `78460113ef7c48502c765a1f83ba34d53c977134` (Puckbridge pin), working-tree project overlay; GitHub `main` base is `cdb7b911462218cb0cd815f2ca6b9cc953ee6b7d`, tree `424028c8ce429cb9eaf38b0764b6386929b3a177`.
- Cause confirmation carried forward: `9235825db8425e4bb335396fcc38e83daf3c3aa7` directly follows full-source `e2059116442e0d195b8aacb9b4baf4eb13737e3e`, and its actual tree has six blobs versus 189 in the parent; API compare reports 189 paths, 183 removals, and 43,543 deletions. The incomplete child tree is verified; its construction mechanism remains unproven. The preserved history is unchanged.
- Code: regression-first implementation in `payload/steam_usb.c/.h` and `tests/test_steam_usb.c`. The parser now validates declared configuration/interface endpoint counts, unique default-altsetting interfaces, and fixed map capacity. Added 24 production parser assertions; the initial count/capacity regressions failed before the fix.
- Docs/memory: S11 in `docs/SOURCES_AND_EVIDENCE.md`, USB parser contract in `docs/ARCHITECTURE.md`, T-P45, ADR-034, F069, P0-USB-DESC-09, `docs/BUILDING.md`, and this test/session log updated. Actual report-descriptor parsing stays blocked under P0-USB-DESC-10.
- Verification: full normal suite PASS, full ASan/UBSan suite PASS; parser property target passes 100,000 Steam report and 100,000 USB configuration inputs. SDK v0.43 / LLVM 18.1.8 clean cross-build PASS, ELF SHA-256 `5b4ab572d235b2a5e87f4e4f4d031d37ac9b0d3e84ce2a6b2d6103e89055f551`. Detailed commands/results are in M1-197 in `TEST_LOG.md`.
- GitHub: current `main` is M1-196 memory-sync commit `cdb7b911462218cb0cd815f2ca6b9cc953ee6b7d`; its Host and PS5 Actions runs `37846785391` and `37846785551` are still queued. The M1-197 source/docs/memory update has not yet been published. `git diff --check` must be repeated after the final edits and before tree creation.
- Hardware/safety: no physical PS5, Steam Controller, Puck, or DualSense was available; no target ioctl, BLE/HCI/radio, firmware/system write, or release action occurred. Do not claim PS5 runtime or firmware compatibility.
- Next exact action: `git diff --check`, run credential scan, compute full local blob manifest and compare to `main` tree `424028c8ce429cb9eaf38b0764b6386929b3a177`; then create a new main tree over that base and update `main` only with expected-head `cdb7b911462218cb0cd815f2ca6b9cc953ee6b7d`, `force=false`.


### M1-197 direct-main publication reconciliation — 2026-10-08 21:36 UTC

- Published the USB configuration count fix, tests, source-backed evidence, architecture/test/build documentation, and project-memory records directly to main as commit bd833e3f1914610e2e9250e721796da3b3ceec2f, parent cdb7b911462218cb0cd815f2ca6b9cc953ee6b7d, tree 2db08d87687c3e5381bb1f81fd8a14a1ac10e762. The update used an expected-head guard and force=false; no PR or history rewrite was used.
- All 13 modified blob IDs matched local Git object hashes before tree creation. Re-fetched main and recursive tree: commit and tree match, 209 entries/196 blobs, truncated=false; every path, mode, and blob hash matches the complete local manifest with no missing or extra paths.
- Host tests run 37847899123 and PS5 cross-build run 37847899130 are queued; combined status has no completed checks. No CI pass is claimed.
- M1-197 local evidence: full normal and ASan/UBSan suites pass; parser test passes 24 checks; fixed-seed property test covers 100,000 Steam reports and 100,000 USB configurations. SDK v0.43/LLVM 18.1.8 compile/link passes with ELF SHA-256 5b4ab572d235b2a5e87f4e4f4d031d37ac9b0d3e84ce2a6b2d6103e89055f551. No physical PS5, controller, Puck, DualSense interface, USB ioctl, BLE/HCI/radio command, firmware write, or binary release was used.
- P0-USB-DESC-09 is fixed at the parser/host-test level. Keep P0-USB-DESC-10 blocked until a safely identified Steam Controller/Puck descriptor and target ioctl behavior can be obtained. Next: recheck the two Actions runs and continue a separate host-only source audit.


## M1-198 fixed standard USB descriptor lengths — 2026-10-08 21:40 UTC

- Source evidence: USB-IF USB 2.0 sections 9.6.2/9.6.3/9.6.6 define standard configuration/interface/endpoint descriptor sizes. The parser previously enforced only minimum lengths.
- Regression-first: three exact-size tests were added; all three initially failed against production code. A bad endpoint fixture total-length value was corrected before considering the expected-red result valid.
- Code/docs: require exact 9/9/7 byte descriptor lengths in payload/steam_usb.c; update the API comment, Sources and Evidence (S12), Architecture, Test Plan (T-P46), ADR-035, F070, P1-USB-DESC-11, Building, and memory logs.
- Verification: full normal test suite and full ASan/UBSan suite pass; parser is 27 checks and property tests run 100,000 report plus 100,000 USB configuration inputs. SDK v0.43 / LLVM 18.1.8 cross-build passes; ELF SHA-256 362d584f60d26a0206a983c55980ebe7083f4c05fc9bd7adf9b878839a49fa1c.
- GitHub: current main is M1-197 memory-sync commit 2742b15fd0bac06db54f5fc50730c10d35063463. Host/PS5 Actions 37848117055/37848117173 are queued; no CI result is claimed. M1-198 is not published yet.
- Hardware/safety: no PS5, controller, Puck, USB ioctl, BLE/HCI/radio, firmware/system write, or release action. Descriptor parser behavior remains host/build evidence only.
- Next exact action: git diff --check, scan all nonignored files for trailing whitespace/secrets, compare the complete local manifest to main tree 70e1f0e4d321457fe37f1dc4698617d092544efe, and prepare a non-force guarded main update against 2742b15fd0bac06db54f5fc50730c10d35063463.


### M1-198 direct-main publication reconciliation — 2026-10-08 21:43 UTC

- Published fixed USB standard descriptor lengths, parser regressions, source evidence, architecture/test/build docs, and project memory directly to main as commit e84f792fb37c47a332f4b914b25fbd74640b7cf0, parent 2742b15fd0bac06db54f5fc50730c10d35063463, tree dfb72ab1b4dad7f3aac9d8bdbd672f9deaa0385a. The ref update used expected-head 2742b15fd0bac06db54f5fc50730c10d35063463 with force=false; no PR or history rewrite was used.
- All 13 changed blobs matched local Git object hashes before tree creation. Re-fetched main/tree: 209 entries/196 blobs, truncated=false, exact path/mode/blob match to the complete local manifest with no missing/extra files.
- Actions 37848691432 (Host tests) and 37848691412 (PS5 cross-build) are queued; combined status has no completed checks. No CI pass is claimed.
- M1-198 local evidence: regression-first exact-length checks, full normal and ASan/UBSan suites, 27 USB parser checks and 100,000 report + 100,000 configuration parser inputs pass. SDK v0.43/LLVM 18.1.8 cross-build passes; ELF SHA-256 362d584f60d26a0206a983c55980ebe7083f4c05fc9bd7adf9b878839a49fa1c.
- No physical PS5, Steam Controller, Puck, DualSense interface, USB ioctl, BLE/HCI/radio, firmware/system write, or binary release was used. P0-USB-DESC-10 remains blocked pending safe identified hardware evidence.

## M1-199 initial GNU_SOURCE candidate — strict-C11 Ubuntu host build — 2026-10-09 05:17 UTC

- Re-read the repository instructions, full execution specification, required architecture/BLE/test documentation, and current project-state/evidence logs. Reconciled main as `4c947486fd41371d9a96238d39e8b7a826c1c34c`, tree `9aee8ff14d4c6e9eb1267aa6c8fa94e98422dd18`.
- GitHub Host tests failed on both initial run `37848828922` attempt 1 and the user-requested rerun attempt 2; earlier source run `37848691432` showed the same error. The failing translation unit was `payload/webui_auth.c` under `-std=c11` on Ubuntu. The existing PS5 cross-build on main passed (`37848829021`).
- Local workspace remains branch `fix/p0-safety`, HEAD `78460113ef7c48502c765a1f83ba34d53c977134`, with the existing Puckbridge-based overlay and many pre-existing modified/untracked project files. Confirmed the local `payload/webui_auth.c` content matched GitHub main before editing; did not switch branches, reset files, or use a normal Git push.
- Added guarded `_GNU_SOURCE=1` before every include in `payload/webui_auth.c`. This asks the Linux libc for the declarations/flags used by existing token-storage code under strict C11; it does not relax global compiler flags or change runtime behavior. Added T-P47, ADR-036, F071 and P1-CI-01 and updated the current project-memory snapshot.
- Verification: strict source syntax compile PASS; `make -C tests clean test` PASS; `make -C tests sanitize` PASS; SDK v0.43 / LLVM 18.1.8 full compile/link PASS, ELF SHA-256 `553ce9f8a69be70f60409292f9f4b01fdaa18815a56e5ded3708f436de6ce497`. Detailed environment/commands are in M1-199 of `TEST_LOG.md`. Post-fix GitHub Actions remain unverified.
- Safety: no PS5/controller/Puck hardware or USB ioctl; no BLE/HCI/radio, firmware/system write, PR, or binary release.
- Exact next action: run `git diff --check`, inspect all M1-199 changes and their local blob hashes; publish only `payload/webui_auth.c`, `docs/TEST_PLAN.md`, and the six directly relevant `project-memory/*.md` files over GitHub main expected SHA `4c947486fd41371d9a96238d39e8b7a826c1c34c`, guarded `force=false`; verify the acknowledged commit/tree and wait for the new Host tests and PS5 cross-build.


### M1-199 follow-up — final POSIX feature-test macro — 2026-10-09 05:20 UTC

- Replaced the initially tested `_GNU_SOURCE` candidate with the user's recommended `_POSIX_C_SOURCE=200809L`, guarded by `__linux__` and `!defined(_POSIX_C_SOURCE)` before all includes. This scopes feature visibility to the failing Linux host translation unit and preserves default macOS/PS5 SDK declarations.
- Final variant passes Darwin strict-C11 syntax compilation, full `make -C tests clean test`, full `make -C tests sanitize` (ASan/UBSan), and SDK v0.43/LLVM 18.1.8 clean/build with explicit `PS5_PAYLOAD_SDK`; final ELF SHA-256 `4c27faf3dc8b5e7589f8f9df4ec396f055377be8157ef41e9016fbb93b498aac`. `git diff --check` passes. Linux CI is still required because the local host is Darwin and skips the guarded macro branch.
- No hardware, radio, firmware, USB ioctl, PR, or binary-release action occurred.
- Exact next action: re-fetch GitHub `main` and its tree. If main still equals `4c947486fd41371d9a96238d39e8b7a826c1c34c`, publish only the reviewed eight files (`payload/webui_auth.c`, `docs/TEST_PLAN.md`, and six `project-memory/*.md`) using a base-tree-preserving Git Data commit and expected-head ref update with `force=false`; then verify the commit/tree and check both Actions workflows.


### M1-199 direct-main publication confirmation — 2026-10-09 05:24 UTC

- Published the final Linux-only `_POSIX_C_SOURCE=200809L` source fix and project-memory update directly to main as `aefa83283ba0360d0ee1444ce0d78d3c7e0504da` (parent `4c947486fd41371d9a96238d39e8b7a826c1c34c`, tree `f9c8a7d93716a900815483d024443f968f1c90f7`). Ref update was expected-head guarded and `force=false`; GitHub acknowledged it.
- Verified the new recursive tree: 209 entries/196 blobs, `truncated=false`; all eight updated blob IDs match local Git hashes, required core payload files remain present. Host run `37888347135` and PS5 cross-build `37888347062` were queued on the new commit at 05:24 UTC.
- Exact next action: inspect both Actions workflows after their runners start and record their result; Linux Host completion decides whether this CI failure is truly resolved. No PR, hardware, USB ioctl, BLE/HCI/radio, firmware/system write, or binary release occurred.

### M1-200 — P0-1 virtual-device ownership — 2026-10-09 06:22 UTC

- Re-read P0 instructions and reconfirmed workspace branch `fix/p0-safety`, local HEAD `78460113ef7c48502c765a1f83ba34d53c977134`, pre-publication GitHub `main` `aefa83283ba0360d0ee1444ce0d78d3c7e0504da`; Actions `37888347135` and `37888347062` are still queued.
- Found and removed the anonymous `0..63` VDA startup-delete sweep. Removed `scePadGetHandle()` fallback, output-token promotion, and async klog-derived IDs as handle/ownership candidates from `create_vda_for_slot()`. Direct ShellUI stubs now gate deletion on the positive handle returned from their own AddDevice call and on that exact handle remaining selected; looked-up/open/injected handles are not deleted.
- Added `tests/test_vda_ownership.py`, wired it into the full test recipe, and added T-P48/F072/P0-VDA-01/ADR-037. Three source guards failed on pre-fix code and pass now.
- Normal host suite, ASan/UBSan suite and clean SDK v0.43 cross-build pass; ELF SHA is in M1-200 TEST_LOG. This is compile-only evidence. No target or physical-device operation was run.
- Remaining uncertainty: the installed SDK has the `libScePad.so` binary but no `AddDevice` API contract header. The code only treats a positive immediate return as attributable; semantics of the API, success-without-returned-handle cleanup, real delete behavior and device reconnection remain target-unverified.
- Exact next action: publish the reviewed M1-200 paths via GitHub Git Data against a fresh `main` tree and expected-head ref update. Then inspect thread lifecycle ownership and implement P0-2 before starting P0-3.

### M1-200 direct-main publication — 2026-10-09 06:26 UTC

- Re-fetched main immediately before publication; head was `aefa83283ba0360d0ee1444ce0d78d3c7e0504da`, tree `f9c8a7d93716a900815483d024443f968f1c90f7`. Built the new tree on that base, created commit `ab69c02d502238a4956e37d1f611f8382c486f43`, and advanced `main` with expected SHA and `force=false`; GitHub returned success.
- Re-fetched ref, commit and recursive tree after update: parent `aefa832...`, tree `448d610603c7dfb095841899f67db9158fa55dd8`, 210 entries, non-truncated. All 11 changed blob SHAs match local content and required payload files remain. No PR or history rewrite.
- Actions for the source commit: PS5 cross-build `37893512335` queued; Host tests `37893512422` queued. Previous fix CI `37888347135`/`37888347062` also still need outcome checks. No workflow is called passed while queued.
- Next work, in required order: check Actions, then implement centralized stop/USB cancellation/neutral input/registry rollback; only after review and tests begin the privilege audit.

### M1-200 memory synchronization — 2026-10-09 06:27 UTC

- Re-fetched main at source commit `ab69c02d502238a4956e37d1f611f8382c486f43`, tree `448d610603c7dfb095841899f67db9158fa55dd8`; published five updated project-memory files in `feaf320018625295d48909b3531c8e567f2ff8a6`, tree `761fa26ff18f01f7d93537a47a3c09a5099e3eaf`, guarded expected-head and `force=false`.
- Re-fetched main/tree after update: current main `feaf320...`, 210 entries, `truncated=false`, 5 expected memory blob SHAs matched. Host run `37893626574` and SDK run `37893626605` are queued on the sync commit.
- Checked Actions for M1-199, source M1-200, and memory-sync M1-200: all six observed workflows are queued. Do not claim CI completion until runners finish.

### M1-201 — P0-2 coordinated shutdown and CI follow-up — 2026-10-09 06:50 UTC

- Reconciled the reported `webui_auth.c` Ubuntu strict-C11 failure against GitHub `main`: the Linux-scoped `_POSIX_C_SOURCE=200809L` definition already precedes every include in commit `aefa83283ba0360d0ee1444ce0d78d3c7e0504da`. Kept this scoped fix unchanged to preserve Darwin and PS5 SDK feature declarations.
- Continued P0-2 in the prescribed order. The central shutdown path gates and joins the DualSense hand-off worker, signals all stop-aware loops, joins the listener/probe/manager/game-hook/profile/klog workers, lets each USB reader clean its own operations, neutralizes owned virtual input before deletion, retries registry rollback, and retains shared state if any join or cleanup fails. SC2 active-endpoint scan now accepts cancellation and performs STOP/CLOSE through the reader's existing resource owner.
- Added 12-worker stop signaling coverage, four static contract guards, and an injected cancel-after-START regression that requires STOP/CLOSE, zero active transfer/endpoint state, and `-ECANCELED`. Added T-P49/F073/P0-SHUTDOWN-01/ADR-038. No detached native service threads or cross-thread USB cancellation were introduced.
- Verification: `make -C tests clean test` PASS; `make -C tests sanitize` PASS; SDK v0.43/LLVM 18.1.8 clean build PASS with explicit `LLVM_CONFIG`; ELF SHA-256 `5b3a7df02d4e845f56eb351134e562cc65ec5b296b80187b49a4daed157f85fc`; `git diff --check` PASS. The first SDK attempt without `LLVM_CONFIG` failed because its wrapper resolved the compiler to nonexistent `/llvm-clang`; the explicit SDK toolchain variable fixed that invocation. Detailed test evidence is in `TEST_LOG.md`.
- Actions on the previous source/fix commits remain queued as of 06:50 UTC: `37888347135`, `37888347062`, `37893512422`, `37893512335`, `37893626574`, `37893626605`. No CI pass is claimed. Current P0-2 source/test/doc/memory change set is ready for direct-main publication under the owner's standing instruction, after rechecking main and its exact base tree.
- No PS5, DualSense, Steam Controller, official Puck, USB ioctl, Bluetooth/HCI/radio, firmware/system write, PR, or binary release was used. Physical signal, USB and VDA cleanup gates remain open.


### M1-201 direct-main publication confirmation — 2026-10-09 06:55 UTC

- Re-fetched `main` immediately before updating it: `feaf320018625295d48909b3531c8e567f2ff8a6`, tree `761fa26ff18f01f7d93537a47a3c09a5099e3eaf`. Published the reviewed P0-2 source/test/docs/evidence tree as `115341056cb18eafa500eb1d3bd1020934db7106` (parent `feaf320...`, tree `90ba6303485b3d28986bc33c346897f2a3d18201`); GitHub acknowledged the expected-head guarded `force=false` update.
- Git tree was built on the full main base. The 26 intended blob changes were checked against local Git blob hashes. Post-publication ref/commit/tree verification reports 214 entries/201 blobs, `truncated=false`, no deleted base paths, and all required core files present.
- New Actions runs `37896053365` (Host) and `37896053373` (PS5 cross-build) both target the published commit and are queued. M1-199/M1-200 Actions were still queued at 06:50 UTC.
- Local gates remain: full normal host suite PASS; ASan/UBSan suite PASS; SDK v0.43/LLVM 18.1.8 cross-build PASS (ELF SHA-256 `5b3a7df02d4e845f56eb351134e562cc65ec5b296b80187b49a4daed157f85fc`); `git diff --check` PASS. No hardware, USB ioctl, Bluetooth, firmware/system write, PR, or release action occurred.
- Continue with actual Actions outcomes, then P0-3 privilege audit. Physical-console shutdown and rollback tests remain gated on PS5/controller/Puck access.

### M1-202 — P0-3 credential guard — 2026-10-09 07:21 UTC

- Read current project task/memory and reconfirmed main at 18ef42e0371973eabbae80d1392be8ef79eabba9, tree bb9079f23bea6624686a32d8a156d4b1a3de6300. The workspace is a pre-existing broad overlay on fix/p0-safety; no branch reset/switch or blanket staging.
- Implemented the shared injectable credential guard and kernel adapter; audited gc_main.c, shellui_pad.c, and wireless_ds4.c. Checked error results in runtime startup and persistent-scope restore after worker teardown. Added host tests and T-P50.
- Full normal tests, ASan/UBSan, SDK v0.43 cross-build, static contract and git diff --check pass. ELF SHA-256: 2c5e38031289c5e44f3e805ea59c5a9679a101c90f963a68568bd581091d440d. No hardware or intrusive system operation.
- Recorded next open items from the user audit: safe-mode startup gate (P0-SAFE-01), USB detach identity gate (P1-USB-DETACH-01), PID reuse (P1-PID-REUSE-01), and unbounded hook rollback (P1-HOOK-ROLLBACK-01). Source evidence in F075-F078.
- Exact next actions: publish only the 12 P0-3 source/test/docs paths via GitHub tree/blob/commit APIs atop freshly re-read main with expected-SHA force=false; re-fetch and verify every changed blob and full tree. Then record returned commit/Actions and memory sync; continue with P0 safe mode.

### M1-202 direct-main publication confirmation — 2026-10-09 07:24 UTC

- P0-3 source commit 3d0a6e69738f551f54a1c175b180a0716fc417ab is verified on main, parent 18ef42e0371973eabbae80d1392be8ef79eabba9, tree 16ceb9d230ed404a4746d6011a26f3b2ae3e57c2. The recursive tree preserves all base blobs and all 12 expected source blobs match local Git hashes.
- GitHub Host/PS5 Actions 37898795378/37898795346 target P0-3 and were queued. Previous P0-2-era workflows also remain queued; none are called green.

### M1-203 — P0-4 default safe runtime policy — 2026-10-09 07:27 UTC

- The policy reader is a separate portable module, uses a strict 1024-byte config bound and refuses symlinks/non-regular files. Missing/invalid input resets all optional features off. Exact Steam VID/PID discovery remains active; unknown Nintendo/Xbox endpoint guessing and associated detach path are off. ShellUI patch entrypoints have no startup callsite, bridge/game hooks and dualsense hand-off are policy-gated, and WebUI is off without opt-in.
- Added parser/file tests and static startup-contract tests; tightened DS hand-off API so a WebUI request cannot bypass the policy gate.
- Full normal host suite, full ASan/UBSan suite, SDK v0.43/LLVM 18.1.8 cross-build, static contracts and diff check pass; ELF SHA-256 a70050eb64f42971c0480b46230957769f9ee6a565c8c074be80418e159cc2ba.
- No PS5/controller/Puck runtime, USB ioctl, Bluetooth, firmware write, PR or release. Exact Steam interface detach remains the next mandatory P0-5 review.
- Next exact action: publish only the reviewed safe-mode files atop fresh main 3d0a6e69738f551f54a1c175b180a0716fc417ab with blob-hash/tree verification and expected-SHA force=false; then record commit/Actions and proceed to P0-5.

### M1-203 direct-main publication confirmation — 2026-10-09 07:29 UTC

- P0-4 safe-mode source/docs/test commit a366f2d6ca3923a3645f52500d9000cd89a9c9dd is on main with parent 3d0a6e69738f551f54a1c175b180a0716fc417ab. GitHub compare confirms exactly the 13 intended paths. Blob hashes match local source, complete tree is 224 entries/211 blobs, and no previous base blob was deleted.
- Actions 37900071213 (Host) and 37900071175 (PS5 build) were queued; P0-3 runs 37898795378/37898795346 remained queued. Local normal and sanitizer suites and SDK v0.43 cross-build passed; no remote CI result is claimed.
- P0-SAFE-01 has a host implementation on main. P0-5 still has to inspect and restrict all known-Steam interface detaches before any physical console recommendation.

### M1-204 — P0 USB ownership and release-safety completion — 2026-10-09

- Completed source audit of every USB interface detach. Removed unknown-device probing/detach paths and endpoint guessing; routed the single detach ioctl through exact Steam 2026 VID/PID plus validated descriptor-map policy. Added expected-red map coverage and source contracts.
- Replaced unsafe saved-PID `SIGTERM` startup with an exclusive file-lock/owner identity module. Added regression coverage for a live unrelated child, stale PID takeover, malformed files, symlink refusal, and owned-file cleanup.
- Added fail-closed release hardware evidence checks, then strengthened them so each required matrix row must link to its own exact TEST_LOG heading and repeat matching hardware/firmware/transport/commit data plus `VERIFIED_HARDWARE`.
- Local normal suite, ASan/UBSan suite, 7 release-gate synthetic tests, 41 PID-module checks, 45 USB policy/parser checks, Ruby workflow syntax parse, `git diff --check`, and SDK v0.43 / LLVM 18.1.8 payload cross-build pass. ELF SHA-256: `05d46cc3180317a197f2ef53f5c26665175c0724515177ff87a1cf54a18995b2`.
- Published 22 paths to GitHub `main` as `afc35de59206df8a479db031d320e0d31740dff2` with expected-head protection. Re-fetched tree: 232 entries / 218 blobs, no base paths lost, all changed blob SHAs match, core payload paths intact.
- GitHub PS5 cross-build run `37980961982` passed. Host run `37980962018` failed on the test-only Darwin linker flag; follow-up portability fix was published in `694c13a44af8f24c2172d1fef112287970d3dfd9`. New Host/PS5 Actions `37981319013`/`37981319108` were in progress at the latest check.
- No hardware gates were run; hardware matrix and release gate remain blocked. P1 hook rollback remains open because the source call graph does not prove active ShellUI patch ownership and the game-hook worker retries rollback indefinitely. No speculative target patch/unpatch change was made.

### M1-205 — Host CI portability correction — 2026-10-09

- Inspected the new Host Actions log and reproduced the mechanism: Ubuntu GNU `ld` rejected `-Wl,-dead_strip`. Selected linker garbage-collection flags by host OS, preserving Darwin behavior and using GNU `--gc-sections` elsewhere.
- Targeted SC2 scanner test, full normal host suite, and full ASan/UBSan suite pass locally. Published only `tests/Makefile` and `docs/TEST_PLAN.md` to main as `694c13a44af8f24c2172d1fef112287970d3dfd9` with expected-head protection; parent `afc35de...`; no base files or unmodified blobs were removed or changed.
- Exact next action: check Actions runs `37981319013` and `37981319108`; if Host fails again, inspect the failing step before editing. Then publish a memory snapshot update with final run conclusions. Hardware/ABI gates remain unchanged.

### M1-206 — Remove remaining strict-C11 `usleep()` from SC2 test builds — 2026-10-09

- Inspected Host failure `37981319013`: the linker correction worked; the next compiler error was an undeclared `usleep()` in `sc2_gyro.c`. GitHub PS5 cross-build `37981319108` passed on the parent commit.
- Reused `runtime_sleep_ms()` for SC2 scan and gyro retry waits. Added POSIX.1-2008 visibility to `sc2_gyro.c` and linked the shared runtime module into the scanner and gyro host test targets.
- Targeted scanner/gyro tests, full normal suite, full ASan/UBSan suite, and local SDK v0.43/LLVM 18.1.8 cross-build pass. New ELF SHA-256: `c5d1a56605654ae9b5e8330b892dcec475f4d78c62de9adf0a07a3c3b8eea13a`.
- Source change is local and not yet on main at this log entry. Exact next action: re-run diff/YAML/secrets checks, publish only `controller_sc2.c`, `sc2_gyro.c`, `tests/Makefile`, `docs/TEST_PLAN.md` with a fresh expected-head check, then inspect both triggered Actions runs. No target hardware tests were attempted.

### M1-206 publication confirmation — 2026-10-09

- Published the POSIX wait-helper and test-link changes directly to main as `451b10f03200f7bf3b95806324286c33709bb498`, parent `694c13a44af8f24c2172d1fef112287970d3dfd9`. Exactly four intended blobs changed; full tree has 232 entries / 218 blobs, no base path or non-overlay blob was lost or altered, and required payload modules remain present.
- Local targeted scanner/gyro tests, full normal suite, full ASan/UBSan suite, and SDK v0.43/LLVM 18.1.8 cross-build pass. Updated ELF SHA-256 `c5d1a56605654ae9b5e8330b892dcec475f4d78c62de9adf0a07a3c3b8eea13a`.
- Actions `37982325945` (Host) and `37982325957` (PS5 cross-build) were in progress at the time of publication. Exact next action: poll both runs, inspect logs for any failure, and update persistent memory with final outcomes. P0 hardware gates and P1 hook rollback remain unchanged.

### M1-207 — Add the missing standard declaration for `mkdtemp()` — 2026-10-09

- Inspected Host failure `37982325945`; POSIX strict-C11 compilation found `test_runtime_policy.c` used `mkdtemp()` without including `<stdlib.h>`. The paired SDK build `37982325957` passed.
- Added the standard header and a T-P57 entry. Targeted runtime-policy build, full normal test suite and full ASan/UBSan suite pass locally.
- Published only `tests/test_runtime_policy.c` and `docs/TEST_PLAN.md` to main as `d727f28af5f7e94452da2a017270db368266ac59`; exact parent `451b10f...`, complete 232-entry tree preserved, zero unrelated blob changes.
- GitHub Actions `37982788726` (Host) and `37982788729` (PS5 cross-build) were in progress at 19:48:57 UTC. Next: inspect both jobs, update persistent memory with the outcome, and correct further failures if present. Hardware evidence remains blocked and hook rollback remains open.

### M1-207 final CI confirmation — 2026-10-09

- GitHub Actions on `d727f28af5f7e94452da2a017270db368266ac59`: Host tests `37982788726` PASS; PS5 cross-build `37982788729` PASS. The full strict-C11 sanitizer host suite, including the SC2 scanner/gyro and runtime policy tests, now compiles and completes on Ubuntu.
- P1-CI-01 (POSIX declarations in `webui_auth.c`) and P1-CI-02 (POSIX waits, GNU/Darwin linker flag, and `mkdtemp` header) are resolved at the host/build level; this does not verify the target runtime.
- Source commits remain directly on `main`; no PR or release was created. Current main tree is complete at 232 entries / 218 blobs. Hardware release gates H-01/H-02/H-03/H-04/H-05/H-09/H-12 remain blocked. Exact next action: wait for physical PS5 FW 13.60, Steam Controller 2026, Puck and spare DualSense access to execute those gates; continue P1 hook rollback source/ABI analysis without speculative target patching.

### M1-208 — Close the unbounded hook-shutdown wait — 2026-10-09

- Reproduced the P1 shutdown hang with the actual host-compiled `game_hooks.c`: force a successful mock install, make every `pb_hooks_remove()` return `PB_HOOKS_REMOVE_RETRY`, and observe the old stop path exceed six seconds.
- Added best-effort neutral input/audio shutdown, five bounded rollback attempts, a distinct `GAME_HOOKS_STOP_ROLLBACK_PENDING` result, and main-thread logging/nonzero exit when target removal still fails. The target-resident gateway is deliberately not unmapped; stale-read/audio leases remain the fallback. A blocked remote syscall itself can still delay an individual attempt, and the target lease behavior is not console-verified.
- Audited ShellUI call sites. The legacy `shellui_pad_patch_vda`, `shellui_pad_patch_vda_self`, and `shellui_pad_unpatch` APIs have no production callers outside their defining source/header. `shellui_pad_disconnect_device` is called by opt-in `ds_handoff`; safe-mode policy disables that service unless explicitly enabled. Did not run ptrace, system-memory writes, or console operations.
- Final `make -C tests clean test`, `make -C tests sanitize`, all five shutdown contracts, all six safe-mode contracts, SDK v0.43/LLVM 18.1.8 cross-build, and `git diff --check` pass. Local ELF SHA-256: `c1fc0b3cad3d5a82119c2d4c01d97686729a3b77e9c8c17b7c2ac85a06071aed`. Published directly to main as `41104df71fd72c711663570db13766161dceb35e` (parent `affb04de5e79cefd006363fc33300500c6d6ba3f`); GitHub compare confirms a one-commit fast-forward with only the 15 planned files. Host/PS5 Actions status is not visible through the available push-run connector and `gh` has no API connectivity. Physical H-12 remains blocked pending console/controller access.


### M1-209 — Project readiness finalization without PS5 hardware — 2026-10-10

- Resumed from the solo-owner direct-main policy and verified GitHub main was 0b00c987d4c4e8ed80d52c9e2bbe723a1cd68b79. Open PRs: none; Releases: none. Latest Host and PS5 cross-build workflows for main both pass (37986069382, 37986069353).
- Reconciled the 218-blob remote source snapshot with the local candidate, retaining the local dirty checkout without reset. Confirmed prior project notes understated current Actions status and that the root continuation guide needed current evidence language.
- Re-ran the full normal suite and ASan/UBSan suite: PASS. Rebuilt payload with SDK v0.43 and Homebrew LLVM 18.1.8 after explicitly exporting LLVM_CONFIG and PS5_PAYLOAD_SDK: PASS; ELF SHA-256 b78a4a5967921f463adbc54afecf866f6561f2d96108a1aeba475631c79249f7.
- Updated the current-state summary and P0/P2 issue statuses; appended test, session, finding and architecture decision entries. Prepared public README feature/build/install guidance, feature evidence matrix, build/CI artifact note, and continuation instructions. Direct-main publication and full-tree post-write verification remain the next steps in this session.
- Hardware-dependent gates remain blocked as requested: no PS5/controller/Puck test, no deploy, no BLE/HCI or firmware/system write, and no Release. The automated release gate correctly refuses publication until H-01/H-02/H-03/H-04/H-05/H-09/H-12 evidence exists.


### M1-210 — Publish and verify no-console finalization — 2026-10-10

- Published 11 reviewed README/status/build/test-plan/project-memory files directly to main in d2107fe1b0719e5878d468b0f0120580ce4f8a1e. The commit is a fast-forward child of 0b00c987d4c4e8ed80d52c9e2bbe723a1cd68b79. Full recursive tree verification retained all 218 blobs/232 entries, with exactly the selected 11 blobs updated and zero additions/deletions.
- Both workflows on the published commit completed successfully: Host 38047541353; PS5 cross-build 38047541326. Verified upload artifact ID 11668855395 and its SHA-256/expiry.
- Refined issue statuses to distinguish host fixes from target-unverified integration. The release gate remains blocked exactly on required real-hardware rows.
- Final state: software source is host-tested and cross-buildable; there is no verified PS5 runtime/release. No console deployment or active Bluetooth operation was performed. Next meaningful validation requires the development hardware named in PROJECT_MEMORY.md and the H-01–H-12 matrix.


### M1-211 — Reconcile latest main checks and artifact — 2026-10-10

- Direct-main documentation/memory follow-up 6b727b6502585c90878944a00d3fc29d6ef2bc36 was acknowledged as a fast-forward from d2107fe1b0719e5878d468b0f0120580ce4f8a1e. Its tree retains all 218 blobs and 232 recursive entries; seven approved paths changed, with no additions/deletions.
- Host run 38047930840 and PS5 cross-build run 38047930852 both completed successfully. The artifact ID, digest and expiry are recorded in TEST_LOG.md and docs/BUILDING.md.
- Source code remains the previously verified commit 0b00c987d4c4e8ed80d52c9e2bbe723a1cd68b79. The remaining blockers require physical development hardware and are not safely resolvable by host-only changes.

### M1-212 — Adapt project target to PS5 FW 12.00 — 2026-10-10

- Confirmed GitHub main at `c49df86ac2b8dd177aac5af28aa330aa2ce450ba`, no open PRs, and latest Host / PS5 cross-build Actions passed on that baseline. Kept the dirty local overlay intact.
- Removed the 13.60 project requirement; made 12.00 the owner-reported target. Updated the specification, release checker, regression tests, hardware matrix, README, BLE protocol/decision, test plan, source evidence notes, feature-status notice, and project memory. Historical test logs and upstream claims were left as historical evidence.
- Strengthened release validation: exact numeric firmware required; exactly one `Firmware:` line in that gate's anchored TEST_LOG section must equal the matrix value byte-for-byte. Nine checker tests pass, including no-version-pin and 12.00.1 prefix-collision cases. The live matrix correctly remains blocked.
- Re-ran `make -C tests clean test` (PASS), `make -C tests clean sanitize` (PASS), and clean SDK v0.43/LLVM 18.1.8 cross-build (PASS; ELF SHA-256 `b78a4a5967921f463adbc54afecf866f6561f2d96108a1aeba475631c79249f7`). `git diff --check`/final review and exact main tree verification are pending.
- No PS5/controller/Puck runtime test, deployment, active BLE/HCI, firmware update/write, or release was performed. No compatibility claim for FW 12.00. Next action: publish only the allowlisted target/checker/test/docs/memory files as a fast-forward to current main and inspect resulting Actions.
