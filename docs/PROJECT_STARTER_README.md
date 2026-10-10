# Project continuation notes

This document reflects the current SteamController-PS5 repository. It replaces the initial setup checklist; M0 source selection, the reusable host-testable modules, P0 code hardening, and the mainline publication path have already been completed for the software evidence currently available.

## Current state

- The selected foundation is PuckbridgePS5 at `78460113ef7c48502c765a1f83ba34d53c977134`. OmniPad-PS5 at `b7682efce18ee0a8d7c80bbc90afbce72151ef96` was compared as a read-only reference; no OmniPad code was imported. See [ADR-001](BASE_SELECTION_ADR.md) and [provenance](PROVENANCE.md).
- The project source is published on the private repository's `main` branch. The project owner has chosen direct, non-force mainline updates while developing solo; do not reopen the old draft PR workflow for owner changes.
- The portable strict-C host suite and ASan/UBSan suite pass. The PS5 community SDK v0.43 cross-build and GitHub Actions build pass. These are not console runtime tests.
- The USB-C/Puck, virtual-pad, motion/touch, haptics, portal, and hook paths remain hardware-unverified. There is no verified Release. Use the [feature-status page](../STEAM_CONTROLLER_2026.md), [hardware matrix](../project-memory/HARDWARE_MATRIX.md), and [current project memory](../project-memory/PROJECT_MEMORY.md) for exact scope.
- Native PS5 BLE remains a research-only track. No active radio operation has been run. Edge identity and ShellUI artwork are optional research, not supported features.

## Safe continuation workflow

1. Read `AGENTS.md`, `CODEX_TASK.md`, all `docs/`, and `project-memory/` before substantial changes.
2. Check the current `main` SHA, GitHub Actions, repository artifacts, and working-tree state. The desktop checkout can lag behind GitHub `main`; compare exact tree/blob SHAs before editing and never reset an unexplained user worktree.
3. Select only a host-testable improvement that has source evidence and a regression path. Preserve licensing notices and append findings/test/session records.
4. Run the portable tests, ASan/UBSan, and SDK cross-build when a code change warrants them. Distinguish a compile from actual console behavior.
5. Update `main` only with the project owner's established direct, fast-forward publication policy and verify the resulting tree and checks. Do not create a Release until the fail-closed hardware matrix passes.
6. Keep all H-01 through H-12 results `NOT_RUN / BLOCKED_HARDWARE` unless actual hardware steps are performed. Do not emulate a console result with host stubs.

## Primary references

- [README](../README.md) — project status, build and development-console caveats.
- [CODEX_TASK.md](../CODEX_TASK.md) — full requirements and milestone acceptance criteria.
- [Architecture](ARCHITECTURE.md), [source evidence](SOURCES_AND_EVIDENCE.md), [BLE feasibility](BLE_FEASIBILITY.md), and [test plan](TEST_PLAN.md).
- [`project-memory/`](../project-memory/PROJECT_MEMORY.md) — current state, findings, issues, decisions, test/session records, and hardware matrix.
- [CONTRIBUTING.md](../CONTRIBUTING.md) — requirements for external contributions.
