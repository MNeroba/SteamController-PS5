# Persistent project memory

| File | Purpose |
| --- | --- |
| [CURRENT_STATUS](CURRENT_STATUS.md) | Short current source/CI/readiness/blockers and next action |
| [PROJECT_MEMORY](PROJECT_MEMORY.md) | Continuation policy, foundation and environment |
| [DECISIONS](DECISIONS.md) | Append-only architecture/safety/publication decisions |
| [ISSUES](ISSUES.md) | Code defects, fixed host scope and remaining physical risks |
| [FINDINGS](FINDINGS.md) | Evidence and negative findings |
| [TEST_LOG](TEST_LOG.md) | Exact commands, environments, hashes and results; stable gate reads this path |
| [SESSION_LOG](SESSION_LOG.md) | Work sequence and publication acknowledgements |
| [HARDWARE_MATRIX](HARDWARE_MATRIX.md) | Owner-reported target vs measured physical evidence |

Read current summaries first; use historical logs to trace a particular ADR, defect or test. M1-220/221 preserves binding/profile history; M1-229 is published dev.2, M1-232/234 is the earlier single-ELF tile candidate, M1-235–237 records the LibrePad integration, M1-238 is published dev.3, M1-239/240 records public preparation, M1-243 is published dev.4, and M1-246/247 is published dev.5; M1-249 records published dev.6 and Public visibility/security settings. Future release publication requires explicit owner approval. Logs and old negative attempts are retained; no hardware result has been changed.


M1-250: PR8 review/correction and published dev.7 are recorded in [PR8_REVIEW](PR8_REVIEW.md), current status and the append-only test/session logs. SDK errno84 is EOVERFLOW; the earlier EILSEQ diagnosis is superseded without changing historical records.
