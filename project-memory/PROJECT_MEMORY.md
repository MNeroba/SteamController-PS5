# Project memory — SteamController-PS5

**Snapshot date:** 2026-10-08 · **State:** M0 COMPLETE; M1 host hardening, reusable pure modules, authoritative slot registry/lifecycle, injected USB session/resource lifecycle, and game-hook safety policies implemented locally · 837 HOST ASAN/UBSAN CHECKS PASS · targeted ThreadSanitizer passes parser, state, registry, and lifecycle concurrency · latest PS5 SDK v0.43 cross-build PASS · game-hook increment pending publication · PS5 runtime/hardware incomplete.

## Intent locked by product owner

Build a standalone, Steam Controller 2026-specialized native PS5 payload. One runtime, one virtual DualSense/session/slot architecture, USB-C and official Puck first, BLE when actually proven; preserve native extended motion, touch, extra buttons/haptics with profiles and reliable PS5 system/game usage. Every shared change must be high-quality reusable architecture, not a controller-specific monolithic patch. FW 13.60 first hardware baseline; future firmware support requires evidence.

## Current established source research

- PuckbridgePS5 has source paths for SC 2026 via official Puck and wired USB, profiles, touch, gyro, motion, haptics, and game hooks. FW 13.60 support remains an upstream claim until reproduced. Its upstream portal was passwordless; this working tree now requires a private token and the payload cross-compiles, but it has not been run on PS5 hardware. The upstream limitation of one SC active through Puck remains.
- OmniPad-PS5 has USB hotplug and multi-pad virtual slots, but no active direct BT as of v1.0.4; relevant security, buffer and lifecycle issues identified at source level. Status: `VERIFIED_CODE` for observed patterns, hardware claims still upstream.
- SteamlessController, SDL, SteamlessLink and OpenPuck provide Steam Controller USB/Puck/BLE protocol references; **Puck radio != BLE GATT**.
- SharpProspero describes 17 libSceBluetoothHid entry points, but not a demonstrated arbitrary BLE Central/GATT API on FW 13.60.
- PS5 Linux internal Bluetooth evidence and merged btusb/xHCI fixes do not imply safe ProsperoOS shared-radio access; Bluetooth reported unstable in Linux PR.
- BTstack-dev reported a BLE keyboard refused by retail PS5; stock BLE-HOGP pairing cannot be assumed supported.
- No actual FW 13.60 native BLE advertising, GATT subscription or Steam Controller direct BLE reports verified.

## Foundation decision

ADR-001 accepts PuckbridgePS5 at `78460113ef7c48502c765a1f83ba34d53c977134` as the base. OmniPad at `b7682efce18ee0a8d7c80bbc90afbce72151ef96` is a source-level reference only. The user-specified private target was empty, so GitHub `main` was initialized with README commit `61780b03bcef6859935cac1ef288cf3eef12d3ba`; the complete 145-file project snapshot was published on `fix/p0-safety` as commit `4596c7fc4a9876e56cb8194961f15de0fd1e7066`, opening [draft PR #1](https://github.com/MNeroba/SteamController-PS5/pull/1). Follow-up source and memory commits are listed in the append-only `SESSION_LOG.md` and publication table in `TEST_LOG.md`. The slot registry and failed-delete retry implementation was published as commit `8808f969cff62d0e129a1bbd3c391fed36e863c6`, parent `4e297d1c0e19df0fc5f451d285c8dc157985ef4b`; the shared lifecycle coordinator and its 85 tests were published as `13c64c5439d82e81c31ec78dd030dbcf8d30bfe7`, parent `8808f969cff62d0e129a1bbd3c391fed36e863c6`; the USB session/resource lifecycle and 821-check evidence were published as `28acb8a8b0d4c979ecfc23a81e36e53c3e236e59`, parent `13c64c5439d82e81c31ec78dd030dbcf8d30bfe7`. Post-publication memory sync is commit `096a4daf7d4fbcb997aa1f3314a44a46a0c693d4`. This is a derivative repository, not a GitHub fork; source pins and notices are retained, while the remote commit does not preserve Puckbridge Git ancestry. The local checkout still has `HEAD` at the pinned upstream commit with uncommitted working-tree changes; Git CLI author identity is unset. Puckbridge and OmniPad remotes remain push-disabled. Current remote PR head is `096a4daf7d4fbcb997aa1f3314a44a46a0c693d4`; PR #1 is open, draft and mergeable. Host run `37801642039` and PS5 cross-build run `37801642059` are queued at 15:37 UTC with empty combined status; no CI result is claimed. Game-hook safety changes remain local and unpublished. See `docs/BASE_SELECTION_ADR.md` and `docs/PROVENANCE.md`.

## Gate status

| Gate | Status | Reason |
|---|---|---|
| M0: source and license selection | COMPLETE WITH LIMITS | Exact source commits compared; Puckbridge selected. Upstream hardware statements are not independently confirmed; inline drawing has repo-level GPL notice and author credit, with no separate asset license. |
| Host tests | PARTIAL_PASS | ASan/UBSan suite passes 837 checks: HTTP/auth token storage (57), JSON (9), USB config (19), Steam decode (190), mapping (24), motion (25), feedback (20), profiles (33), bindings (49), pause/assignment control (51), slot registry (123), slot lifecycle (85), USB resources (66), USB session (70), and game-hook restore/heartbeat policy (16). Targeted ThreadSanitizer passes parser, binding, control, registry contention and lifecycle startup/release stress. Fault injection covers create/worker failures, ambiguous VDA handles, delete retry, USB open/init/endpoint-open/start/complete failures and cleanup. The PS5 manager scan/ioctl adapter, actual `pthread_create`, game remote memory operations, blocked kernel reader, stable device identity/rebind, menu/input integration and physical hardware remain. |
| OmniPad baseline host tests | FAIL | Three present tests pass; target exits 2 at missing `tests/test_normalization.c`. |
| PS5 SDK ELF build | PASS_CROSS_BUILD_ONLY | SDK v0.43 (`d9c9519116944a7f1c22d262012c53b80b3520b7`, archive SHA-256 `a9cc9929f21b2b2c5d5b309f3bab4997067c45281c0622cf4838b1aecba66fcb`) and Homebrew LLVM 18.1.8 built the game-hook safety integration cleanly on Darwin 27.0.0 arm64. Current ELF SHA-256 `ab7239b7dd7bed77581191585dfaa92326305ab5cbbf617ebfeffdd8be583141`; ELF64 x86-64 and six expected SDK runtime libraries confirmed. See M1-47. No PS5 runtime or firmware behavior was exercised. SDK is local in ignored `.build-cache/`; Homebrew installed LLVM at `/opt/homebrew/opt/llvm@18`. |
| Actual PS5 FW 13.60 USB/Puck validation | BLOCKED_HARDWARE | No physical console, Steam Controller, or official Puck available in this session. |
| Native BLE B0–B6 | RESEARCH_ONLY / BLOCKED_HARDWARE | No radio commands were run; native GAP/GATT remains unproven. |
| Git history | PARTIAL | Local checkout retains pinned Puckbridge ancestry; target project uses an independent root and source snapshot commits because it was empty. GitHub app was used; Git CLI author identity remains unset. |
| GitHub PR publication | OPEN_DRAFT | [PR #1](https://github.com/MNeroba/SteamController-PS5/pull/1), branch `fix/p0-safety`; current remote head `096a4daf7d4fbcb997aa1f3314a44a46a0c693d4` is open, draft and mergeable. PR description still reports 821 host checks and ELF SHA `273bf561ececaa0093c40827ffac9cabb087252fdd028afca32ec5d623dbe545`; update it after game-hook source publication. Host run `37801642039` and PS5 cross-build run `37801642059` were queued at 15:37 UTC; combined status is empty, so no CI result is claimed. |

## Exact next tasks

1. Finalize review and publish the game-hook owner-checked rollback/audio-heartbeat changes and their evidence to `fix/p0-safety`, expected current head `096a4daf7d4fbcb997aa1f3314a44a46a0c693d4`; update PR #1 validation text and inspect new CI runs.
2. Review `payload/ds_handoff.c`, portal/manager telemetry and remaining shared lifecycles for stale ownership/races; separate portable seams from target-only code and test only reproducible changes.
3. Obtain PS5, Steam Controller, DualSense and official Puck hardware for USB/Puck/session/motion/touch/haptics/game-hook shutdown gates. Game hooks were built but not run; no remote memory syscall, explicit payload-stop behavior, or process-exit hardware result is verified. Keep native BLE evidence-gated and passive until safe hardware and approval are available.

Every future Codex session: update this snapshot **only after** recording detailed evidence in FINDINGS/ISSUES/TEST_LOG/DECISIONS/SESSION_LOG.
