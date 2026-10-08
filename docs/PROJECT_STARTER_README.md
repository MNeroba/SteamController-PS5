# SteamController-PS5 — Final Codex development package

**Research consolidation: 8 October 2026.** These are *instructions and reference records*, not working payload binaries or evidence of physical-console testing.

## Start here

1. Copy the contents of this directory into the Codex working workspace/repository root.
2. Tell Codex to read `AGENTS.md`, `CODEX_TASK.md`, `docs/` and `project-memory/` completely.
3. Codex must begin at **M0: pin and compare PuckbridgePS5 vs OmniPad-PS5**, document an ADR, fix selected-base P0 risks, then deliver USB/Puck and extended Steam input.
4. BLE internal radio, virtual Edge, and system controller artwork are **experimental and not release blockers**.
5. Maintain project-memory continuously, with actual test and evidence states.

## Entrypoints

- `CODEX_TASK.md` — full executable specification and milestones.
- `AGENTS.md` — repository-level working rules and Codex session policy.
- `docs/SOURCES_AND_EVIDENCE.md` — direct links, observed upstream SHAs, proven facts versus claims.
- `docs/BLE_FEASIBILITY.md` — safe research gates on PS5 13.60 and abort conditions.
- `docs/ARCHITECTURE.md` — reusable modular boundaries and non-negotiable invariants.
- `docs/TEST_PLAN.md` — concrete host and PS5 test IDs; result labels.
- `docs/BASE_SELECTION_ADR.md` — evidence-based choice of implementation foundation.
- `docs/PROVENANCE.md` — third-party licensing and origin register.
- `project-memory/` — permanent project state, findings, issues, tests, decisions, sessions and hardware compatibility.

## Recommended prompt to Codex

> Execute `CODEX_TASK.md` end-to-end, following `AGENTS.md`. Start with M0 source audits, record pins and licensing, make `docs/BASE_SELECTION_ADR.md` evidence-backed, then implement the highest-priority P0 fixes and the USB/Puck Steam Controller release path. Use source links, host fixtures and real test results; never represent a PS5 build as a hardware test. Maintain all project-memory files after every meaningful change. Create small PRs where authorized. Treat native BLE as a gated research feature and preserve the stock DualSense. Do not duplicate existing mature modules or turn specialized Steam support into a fragile monolith.
