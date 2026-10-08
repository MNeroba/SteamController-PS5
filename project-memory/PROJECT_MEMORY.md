# Project memory — SteamController-PS5

**Snapshot date:** 2026-10-08 · **State:** M0 COMPLETE; M1 host hardening, reusable pure modules, and a generation-tagged authoritative live-slot registry with deferred cleanup implemented · 600 HOST ASAN/UBSAN CHECKS PASS · targeted ThreadSanitizer passes including slot-registry contention · PS5 SDK v0.43 cross-build PASS · PS5 runtime/hardware incomplete · publication of this slot-registry increment is pending on draft PR #1.

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

ADR-001 accepts PuckbridgePS5 at `78460113ef7c48502c765a1f83ba34d53c977134` as the base. OmniPad at `b7682efce18ee0a8d7c80bbc90afbce72151ef96` is a source-level reference only. The user-specified private target was empty, so GitHub `main` was initialized with README commit `61780b03bcef6859935cac1ef288cf3eef12d3ba`; the complete 145-file project snapshot was published on `fix/p0-safety` as commit `4596c7fc4a9876e56cb8194961f15de0fd1e7066`, opening [draft PR #1](https://github.com/MNeroba/SteamController-PS5/pull/1). Follow-up memory, lifecycle, profile-parser, provenance, parser-concurrency, and memory-sync commits are `7721f18cde11498c92fe9f6c91aa2a3f7dfbd269`, `04cc833e99c4e1e53eaaa93ae01658daa2cc69c0`, `cf225f48f0f38e86101f4f6d2993edbe47347c58`, `afeb258a6ddb5c607b78dc9a481edccf1b3ff254`, `ade3a334d4af408102c7b149e01dd3cc087fbc1a`, and `2088cd639087b135940c7fc318223c6d3adeeca1`. Commit `65cd4f918f56fb2c547217e008f6b81c17853c04` publishes per-slot control-state hardening, the clean SDK cross-build, digest-pinned CI, and updated memory. This is a derivative repository, not a GitHub fork; source pins and notices are retained, while the remote commit does not preserve Puckbridge Git ancestry. The local checkout still has `HEAD` at the pinned upstream commit with uncommitted working-tree changes; Git CLI author identity is unset. Puckbridge and OmniPad remotes remain push-disabled. See `docs/BASE_SELECTION_ADR.md` and `docs/PROVENANCE.md`.
ADR-001 accepts PuckbridgePS5 at `78460113ef7c48502c765a1f83ba34d53c977134` as the base. OmniPad at `b7682efce18ee0a8d7c80bbc90afbce72151ef96` is a source-level reference only. The user-specified private target was empty, so GitHub `main` was initialized with README commit `61780b03bcef6859935cac1ef288cf3eef12d3ba`; the complete 145-file project snapshot was published on `fix/p0-safety` as commit `4596c7fc4a9876e56cb8194961f15de0fd1e7066`, opening [draft PR #1](https://github.com/MNeroba/SteamController-PS5/pull/1). Follow-up source and memory commits are listed in the append-only `SESSION_LOG.md` and publication table in `TEST_LOG.md`. This is a derivative repository, not a GitHub fork; source pins and notices are retained, while the remote commit does not preserve Puckbridge Git ancestry. The local checkout still has `HEAD` at the pinned upstream commit with uncommitted working-tree changes; Git CLI author identity is unset. Puckbridge and OmniPad remotes remain push-disabled. Current remote PR head is `4e297d1c0e19df0fc5f451d285c8dc157985ef4b`; the validated slot-registry increment is not yet published. See `docs/BASE_SELECTION_ADR.md` and `docs/PROVENANCE.md`.

## Gate status

| Gate | Status | Reason |
|---|---|---|
| M0: source and license selection | COMPLETE WITH LIMITS | Exact source commits compared; Puckbridge selected. Upstream hardware statements are not independently confirmed; inline drawing has repo-level GPL notice and author credit, with no separate asset license. |
| Host tests | PARTIAL_PASS | ASan/UBSan suite passes 600 checks: HTTP/auth/token storage (57), bounded JSON (9), USB config parsing (19), Steam report decoding (190), mapping (24), motion (25), feedback (20), profile parsing (33), binding transitions (49), pause/assignment control (51), and slot registry (123). Targeted ThreadSanitizer passes parser, binding, control and slot-registry contention. Slot tests cover duplicate session keys, capacity, lifecycle, stale generations, stale-token noninterference, VDA handle uniqueness, failed-delete restoration/deferred release, and 8,000 concurrent claims. Manager/USB fault injection, stable device identity/rebind, menu/input integration and physical hardware remain. |
| OmniPad baseline host tests | FAIL | Three present tests pass; target exits 2 at missing `tests/test_normalization.c`. |
| PS5 SDK ELF build | PASS_CROSS_BUILD_ONLY | SDK v0.43 (`d9c9519116944a7f1c22d262012c53b80b3520b7`, archive SHA-256 `a9cc9929f21b2b2c5d5b309f3bab4997067c45281c0622cf4838b1aecba66fcb`) and Homebrew LLVM 18.1.8 built the integrated slot registry and deferred cleanup ELF cleanly on Darwin 27.0.0 arm64. Current ELF SHA-256 `30413450154d33b8905425a6524b9cb6c78fefcb03db807ae8e391391b3fedfd`; ELF64 x86-64 and six expected SDK runtime libraries confirmed. See M1-35. No PS5 runtime or firmware behavior was exercised. SDK is local in ignored `.build-cache/`; Homebrew installed LLVM at `/opt/homebrew/opt/llvm@18`. |
| Actual PS5 FW 13.60 USB/Puck validation | BLOCKED_HARDWARE | No physical console, Steam Controller, or official Puck available in this session. |
| Native BLE B0–B6 | RESEARCH_ONLY / BLOCKED_HARDWARE | No radio commands were run; native GAP/GATT remains unproven. |
| Git history | PARTIAL | Local checkout retains pinned Puckbridge ancestry; target project uses an independent root and source snapshot commits because it was empty. GitHub app was used; Git CLI author identity remains unset. |
| GitHub PR publication | OPEN_DRAFT | [PR #1](https://github.com/MNeroba/SteamController-PS5/pull/1), branch `fix/p0-safety`; current remote head `4e297d1c0e19df0fc5f451d285c8dc157985ef4b` is open, draft, and mergeable. Runs `37793911115` (Host tests) and `37793911079` (PS5 cross-build) were queued with no combined statuses at 14:44 UTC; do not call them passed. Current slot-registry increment is locally validated and pending fast-forward publication. |

## Exact next tasks

1. Publish the validated slot-registry implementation and evidence to `fix/p0-safety` using expected-head validation from `4e297d1c0e19df0fc5f451d285c8dc157985ef4b`; then fetch the new head's CI once.
2. Add manager/USB fault injection around the registry integration in `payload/gc_main.c`; validate rollback and blocked-reader behavior. Research reliable session identity before claiming USB↔Puck/BLE rebind continuity.
3. Complete game-hook rollback for process exit/payload stop and audit remaining handoff/shared telemetry lifecycle. P1-SLOT-11's invalid-handle guard is fixed with portable and cross-build evidence; target thread-start failure injection remains unrun.
4. Obtain a physical PS5, DualSense, Steam Controller, and official Puck for USB/Puck/reconnect/motion/touch/haptics/security hardware gates.
5. Continue BLE only under passive/evidence gates in `docs/BLE_FEASIBILITY.md`; do not use native radio commands without an approved, safe hardware gate.

Every future Codex session: update this snapshot **only after** recording detailed evidence in FINDINGS/ISSUES/TEST_LOG/DECISIONS/SESSION_LOG.
