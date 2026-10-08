# AGENTS.md — repository-level instructions for Codex

**Authoritative specification:** `CODEX_TASK.md` (read in full). Do not replace it with a short plan. Read `docs/SOURCES_AND_EVIDENCE.md`, `docs/ARCHITECTURE.md`, `docs/BLE_FEASIBILITY.md`, `docs/TEST_PLAN.md` and all `project-memory/` before any substantial work. Existing valid upstream legal notices and build requirements take precedence over suggestions here.

## Mission

SteamController-PS5 is **specialized for Steam Controller 2026**. Deliver one safe native payload with USB-C and official Puck first and BLE if hardware gates allow. Preserve Steam-specific motion/touch/back buttons/haptics through a reusable core, with one virtual DualSense and slot/session architecture. Separate native firmware/system hooks from portable code.

## Non-negotiable design and quality rules

1. **Reuse before rewrite.** Audit PuckbridgePS5 and OmniPad first. Keep working audited code; improve existing modules or create independently testable components with narrowly defined stable contracts. Never duplicate complete protocol parsers, slot registries, web servers, virtual-pad implementations or BLE stacks.
2. **Reusable where shared.** Device-specific decoding belongs only to `steam_protocol`/device adapters. Transport interfaces must not know Steam buttons, mapping must not know PS5 ABI, and UI must not directly mutate driver state. Preserve all extended capabilities in the input model, even when PS5 output cannot expose them.
3. **Evidence labels.** `VERIFIED_CODE`, `VERIFIED_HOST_TEST`, `VERIFIED_PS5_BUILD`, `VERIFIED_HARDWARE`, `UPSTREAM_CLAIM`, `EXTERNAL_HARDWARE`, `HYPOTHESIS`, `BLOCKED`. Use exact pin SHA/file/line and test records for claims.
4. **Tests before claims.** For a reproducible bug, write a failing regression first, fix it, rerun. Run targeted host tests, ASan/UBSan where feasible, PS5 cross-build where available, then hardware tests only when a console is actually accessible. Do not claim tests you did not execute.
5. **Zero silent regressions.** Error checking, buffer bounds, race-free lifecycle, safe cleanup, stale input neutralization, protected management interface, no anonymous debug injection. No unexplained kernel offsets or firmware writes.
6. **BLE is a gated research track.** Do not open or reset the stock radio, claim interfaces, change pairing state or run raw HCI commands before read-only diagnostics and an explicitly approved active lab experiment. Maintain native DualSense functionality.
7. **DualSense default.** Edge emulation and ShellUI Steam Controller icon/name are separate research experiments. Do not jeopardize normal virtual DualSense production behavior.
8. **Legality/provenance.** Retain license and upstream copyright notices. No NDA Sony SDK, proprietary firmware, keys or unlicensed code/assets. Review BTstack's noncommercial terms before any code reuse. Don't assume independent repository name grants permission to relicense GPL code.
9. **Security.** Explicit protection for LAN controls; browser request validation and safe enrollment. Debug network input off by default. No secrets, MAC/bonding keys, serials or user identifying details in committed logs.
10. **Make the task independently continuable.** Keep project memory current, with exact next commands and unresolved blockers. Append, do not erase negative results.

## Session procedure

At start:
- Read `project-memory/PROJECT_MEMORY.md`, `DECISIONS.md`, `ISSUES.md`, recent `TEST_LOG.md`, recent `SESSION_LOG.md`.
- Check working tree, branch, upstream/fork SHAs, CI status, open PRs; reconcile stale memory.
- Choose the next safely executable item from `CODEX_TASK.md`. Verify all prerequisites with source evidence.

During work:
- Work in small coherent commits, with changed-file paths and testing appropriate to change risk.
- Keep code compilable; no half-implemented security code or inert toggles advertised as complete.
- Record meaningful new findings and actual tests immediately. If tests couldn't run, record `NOT_RUN` with reason and likely required environment.
- Before using borrowed code, add row to `docs/PROVENANCE.md`, citing file/revision/license and derived changes.

At end:
- Update all relevant `project-memory/` documents and last next action (specific path/function/command).
- Run `git diff --check`, tests, sanitizer/build gates as applicable, inspect diff, secrets and attribution.
- Report actual commits/PRs only if created. For unverified physical-console work open draft PRs or explicitly mark blocked.

## Allowed autonomous actions / hard stops

Allowed: read public repositories, compare code, choose architecture using ADR, implement/verify host code, prepare builds and tests, create local feature branches, document, push/fork/PR when authorized by connected GitHub permissions. Do not claim publication before host acknowledges it. **Hard stop/seek approval:** intrusive use of built-in Bluetooth/HCI, firmware/system resource writes, irreversible changes on a physical console, unknown ABI calls with side effects, release of unverified binaries, or any action requiring rights not granted. Continue independent safe work while waiting on a blocked hardware gate; do not fabricate a substitute.
