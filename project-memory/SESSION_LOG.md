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
- PR: this code fix and updated memory are ready to add to draft PR #1; do not promote the PR from draft.
