# Project memory — SteamController-PS5

**Snapshot date:** 2026-10-08 · **State:** M0 COMPLETE; M1 host hardening and reusable pure modules implemented, with target integration incomplete · 344 HOST SANITIZER CHECKS PASS · PS5 build/runtime and GitHub publication gates remain blocked.

## Intent locked by product owner

Build a standalone, Steam Controller 2026-specialized native PS5 payload. One runtime, one virtual DualSense/session/slot architecture, USB-C and official Puck first, BLE when actually proven; preserve native extended motion, touch, extra buttons/haptics with profiles and reliable PS5 system/game usage. Every shared change must be high-quality reusable architecture, not a controller-specific monolithic patch. FW 13.60 first hardware baseline; future firmware support requires evidence.

## Current established source research

- PuckbridgePS5 has source paths for SC 2026 via official Puck and wired USB, profiles, touch, gyro, motion, haptics, and game hooks. FW 13.60 support remains an upstream claim until reproduced. Its upstream portal was passwordless; this working tree now requires a private token, though the server has not been built/run on PS5. The upstream limitation of one SC active through Puck remains.
- OmniPad-PS5 has USB hotplug and multi-pad virtual slots, but no active direct BT as of v1.0.4; relevant security, buffer and lifecycle issues identified at source level. Status: `VERIFIED_CODE` for observed patterns, hardware claims still upstream.
- SteamlessController, SDL, SteamlessLink and OpenPuck provide Steam Controller USB/Puck/BLE protocol references; **Puck radio != BLE GATT**.
- SharpProspero describes 17 libSceBluetoothHid entry points, but not a demonstrated arbitrary BLE Central/GATT API on FW 13.60.
- PS5 Linux internal Bluetooth evidence and merged btusb/xHCI fixes do not imply safe ProsperoOS shared-radio access; Bluetooth reported unstable in Linux PR.
- BTstack-dev reported a BLE keyboard refused by retail PS5; stock BLE-HOGP pairing cannot be assumed supported.
- No actual FW 13.60 native BLE advertising, GATT subscription or Steam Controller direct BLE reports verified.

## Foundation decision

ADR-001 accepts PuckbridgePS5 at `78460113ef7c48502c765a1f83ba34d53c977134` as the base. OmniPad at `b7682efce18ee0a8d7c80bbc90afbce72151ef96` is a source-level reference only. The pinned Puckbridge source and Git metadata are now present. `HEAD` remains the pin on branch `fix/p0-safety`; `origin` is the user-supplied project URL. Puckbridge and OmniPad remotes have push disabled. Changes remain uncommitted because Git author name/email are unset. See `docs/BASE_SELECTION_ADR.md` and `docs/PROVENANCE.md`.

## Gate status

| Gate | Status | Reason |
|---|---|---|
| M0: source and license selection | COMPLETE WITH LIMITS | Exact source commits compared; Puckbridge selected. Upstream hardware statements are not independently confirmed; inline drawing has repo-level GPL notice and author credit, with no separate asset license. |
| Host tests | PARTIAL_PASS | ASan/UBSan suite passes 344 checks: HTTP/auth/token storage (57), bounded JSON (9), USB config parsing (19), Steam report decoding (190), mapping transforms (24), motion math (25), and feedback encoding (20). Session/slot lifecycle, profile transitions, PS5 adapter, and full USB lifecycle coverage remain. |
| OmniPad baseline host tests | FAIL | Three present tests pass; target exits 2 at missing `tests/test_normalization.c`. |
| PS5 SDK ELF build | BLOCKED_ENVIRONMENT | `PS5_PAYLOAD_SDK` unset. |
| Actual PS5 FW 13.60 USB/Puck validation | BLOCKED_HARDWARE | No physical console, Steam Controller, or official Puck available in this session. |
| Native BLE B0–B6 | RESEARCH_ONLY / BLOCKED_HARDWARE | No radio commands were run; native GAP/GATT remains unproven. |
| Git history | BOOTSTRAPPED | Pinned Puckbridge ancestry restored; target origin and fetch-only source remotes configured. No new commit yet because author identity is unset. |
| GitHub PR publication | BLOCKED_ENVIRONMENT | `gh auth status` reports the MNeroba token invalid; supplied target URL returned 404 unauthenticated. No PR has been created. |

## Exact next tasks

1. Restore valid GitHub authentication and configure Git author name/email; then commit, push the feature branch and create the requested PR against the confirmed target repo.
2. Install/configure a compatible PS5 payload SDK; current environment has no `prospero.mk` under `/Users/mneroba/GitHub`, and `make -C payload` stops because `PS5_PAYLOAD_SDK` is unset.
3. Add host tests for session lifecycle, profile transitions, and slot generations; test the PS5 adapter's neutral-publication branch. The pure mapping, motion, and feedback helpers now have host coverage, but hardware behavior remains unverified.
4. Complete game-hook rollback for process exit/payload stop and continue auditing the inherited slot/handoff lifecycle.
5. Obtain a physical PS5, DualSense, Steam Controller, and official Puck for USB/Puck/reconnect/motion/touch/haptics/security hardware gates.
6. Continue BLE only under passive/evidence gates in `docs/BLE_FEASIBILITY.md`; no stock DualSense risk.

Every future Codex session: update this snapshot **only after** recording detailed evidence in FINDINGS/ISSUES/TEST_LOG/DECISIONS/SESSION_LOG.
