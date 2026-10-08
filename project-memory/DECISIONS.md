# Architecture decisions

| ADR | Status | Subject | Current decision |
|---|---|---|---|
| ADR-001 | ACCEPTED 2026-10-08 | Choose Puckbridge vs OmniPad vs derivative | Puckbridge pinned at `78460113ef7c48502c765a1f83ba34d53c977134` is the executable foundation; OmniPad pinned at `b7682efce18ee0a8d7c80bbc90afbce72151ef96` is reference only. See `docs/BASE_SELECTION_ADR.md`. |
| ADR-002 | ACCEPTED (design constraint) | Normal virtual DualSense as production backend | Keeps PS5 compatibility; Edge research separate |
| ADR-003 | ACCEPTED (design constraint) | One input/protocol/session/slot architecture | Steam focus with reusable shared subsystems; no duplicate payloads |
| ADR-004 | ACCEPTED (safety constraint) | BLE hardware path | Passive first; native BLE only after B0–B6 evidence and operator-approved active tests |
| ADR-005 | ACCEPTED (release constraint) | USB/Puck release independent from BLE | No requirement to solve native BLE before shipping tested wired/Puck build |
| ADR-006 | ACCEPTED (licensing constraint) | Restricted and incompatible code | No confidential Sony SDK or unapproved BTstack reuse in public GPL build |

For any new significant design decision: date, status `PROPOSED/ACCEPTED/SUPERSEDED`, options, source evidence and SHA, decision/negative consequences, test gate and reconsideration trigger. Do not silently rewrite an accepted ADR; add a superseding entry.

## New decisions

| ADR | Date | Status | Decision |
|---|---|---|---|
| ADR-001 | 2026-10-08 | ACCEPTED | Use pinned Puckbridge as foundation because it has the actual Steam Controller 2026 USB/Puck parser and PS5 mapping route. OmniPad's slot and USB lifecycle remain reference material only; its web, TCP and Bluetooth code are not imported. Reconsider only on the source/rights/behavior triggers in `docs/BASE_SELECTION_ADR.md`. |
