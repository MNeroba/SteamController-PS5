# Project memory — SteamController-PS5

**Snapshot date:** 2026-10-08 · **State:** M0 COMPLETE; M1 host hardening and reusable pure modules implemented, target integration incomplete · 374 HOST SANITIZER CHECKS PASS · draft PR #1 open; host CI queued; PS5 build/runtime remain blocked.

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

ADR-001 accepts PuckbridgePS5 at `78460113ef7c48502c765a1f83ba34d53c977134` as the base. OmniPad at `b7682efce18ee0a8d7c80bbc90afbce72151ef96` is a source-level reference only. The user-specified private target was empty, so GitHub `main` was initialized with README commit `61780b03bcef6859935cac1ef288cf3eef12d3ba`; the complete 145-file project snapshot was published on `fix/p0-safety` as commit `4596c7fc4a9876e56cb8194961f15de0fd1e7066`, opening [draft PR #1](https://github.com/MNeroba/SteamController-PS5/pull/1). Follow-up memory, lifecycle, profile-parser, provenance, and parser-concurrency commits are `7721f18cde11498c92fe9f6c91aa2a3f7dfbd269`, `04cc833e99c4e1e53eaaa93ae01658daa2cc69c0`, `cf225f48f0f38e86101f4f6d2993edbe47347c58`, `afeb258a6ddb5c607b78dc9a481edccf1b3ff254`, and `ade3a334d4af408102c7b149e01dd3cc087fbc1a`; the latest code commit removes shared `strtok` state and adds concurrent parser checks. This is a derivative repository, not a GitHub fork; source pins and notices are retained, while the remote commit does not preserve Puckbridge Git ancestry. The local checkout still has `HEAD` at the pinned upstream commit with uncommitted working-tree changes; Git CLI author identity is unset. Puckbridge and OmniPad remotes remain push-disabled. See `docs/BASE_SELECTION_ADR.md` and `docs/PROVENANCE.md`.

## Gate status

| Gate | Status | Reason |
|---|---|---|
| M0: source and license selection | COMPLETE WITH LIMITS | Exact source commits compared; Puckbridge selected. Upstream hardware statements are not independently confirmed; inline drawing has repo-level GPL notice and author credit, with no separate asset license. |
| Host tests | PARTIAL_PASS | ASan/UBSan suite passes 374 checks: HTTP/auth/token storage (57), bounded JSON (9), USB config parsing (19), Steam report decoding (190), mapping transforms (24), motion math (25), feedback encoding (20), and strict/concurrent profile parsing (30). Session/slot lifecycle, profile transitions, most PS5 adapter code, and full USB lifecycle coverage remain. |
| OmniPad baseline host tests | FAIL | Three present tests pass; target exits 2 at missing `tests/test_normalization.c`. |
| PS5 SDK ELF build | BLOCKED_ENVIRONMENT | `PS5_PAYLOAD_SDK` unset. |
| Actual PS5 FW 13.60 USB/Puck validation | BLOCKED_HARDWARE | No physical console, Steam Controller, or official Puck available in this session. |
| Native BLE B0–B6 | RESEARCH_ONLY / BLOCKED_HARDWARE | No radio commands were run; native GAP/GATT remains unproven. |
| Git history | PARTIAL | Local checkout retains pinned Puckbridge ancestry; target project uses an independent root and source snapshot commits because it was empty. GitHub app was used; Git CLI author identity remains unset. |
| GitHub PR publication | OPEN_DRAFT | [PR #1](https://github.com/MNeroba/SteamController-PS5/pull/1), branch `fix/p0-safety`, latest code commit/current branch head `ade3a334d4af408102c7b149e01dd3cc087fbc1a`; mergeable, draft. Host tests run `37784104792` is queued. |

## Exact next tasks

1. Monitor Host tests workflow run `37784104792` for PR #1; fix any failure and keep the PR in draft until available verification is complete. The lazy-slot `-1` deletion guard, strict profile numeric parser, and per-call combo tokenizer are published.
2. Install/configure a compatible PS5 payload SDK; current environment has no `prospero.mk` under `/Users/mneroba/GitHub`, and `make -C payload` stops because `PS5_PAYLOAD_SDK` is unset.
3. Add host tests for session lifecycle, profile transitions, and slot generations; test the PS5 adapter's neutral-publication branch. The pure mapping, motion, and feedback helpers now have host coverage, but hardware behavior remains unverified.
4. Complete game-hook rollback for process exit/payload stop and continue auditing the inherited slot/handoff lifecycle. Allocation and USB-thread creation rollback now avoid deleting invalid `-1` handles; target verification remains open as P1-SLOT-11.
5. Obtain a physical PS5, DualSense, Steam Controller, and official Puck for USB/Puck/reconnect/motion/touch/haptics/security hardware gates.
6. Continue BLE only under passive/evidence gates in `docs/BLE_FEASIBILITY.md`; no stock DualSense risk.

Every future Codex session: update this snapshot **only after** recording detailed evidence in FINDINGS/ISSUES/TEST_LOG/DECISIONS/SESSION_LOG.
