# Static audit remediation register

**Historical original static findings and remediation.** For the current published **dev.6**, use the [README](../../README.md), [loading guide](LOADING.md), [current status](../../project-memory/CURRENT_STATUS.md) and [VDA/storage contract](VDA_PENDING.md). Earlier holds, positive-only VDA policies, counts and source hashes below describe their dated checkpoints. Dev.6 includes host-tested token initialization and bounded VDA observer fixes; no physical PS5 result is established. Historical results and releases are unchanged.

Audit source: `ffff5e08905d394b67a7da070fcf4657a250f070`, 251 blobs, all 41 production C units present. Findings are CONFIRMED_STATIC; target consequences remain unverified. No P0 was confirmed. Owner authorized remediation on 2026-10-11. Preserve original historical results.

| ID | Status | Finding |
| --- | --- | --- |
| AUD-P1-001 | FIXED_CODE/HOST/BUILD — owned frame storage and busy-endpoint regression | USB stack buffers escape after failed STOP/CLOSE; busy send overwrites live transfer metadata |
| AUD-P1-002 | FIXED_CODE/HOST/BUILD — watchdog, validated refresh and owned cleanup contract | No deadline neutralizes input after sustained invalid/missing USB reports |
| AUD-P1-003 | FIXED_CODE/HOST/BUILD — completion status/frame-count scanner regressions | IN COMPLETE status and frame count unchecked before decoding |
| AUD-P1-004 | FIXED_CODE/HOST/BUILD — reader claim after activity; portable lease ownership | Idle Puck reserves the only SC2 feedback lease and prevents wired activation |
| AUD-P1-005 | FIXED_CODE/HOST/BUILD — transaction fault injection and actual worker pending rollback; native adapter target-unverified | Partial game-hook install loses ownership after failed rollback and leaks remote mappings |
| AUD-P1-006 | FIXED_CODE/BUILD — retained-page refusal and requested import checks; native path target-unverified | Retained hook layout reused without verifying requested capabilities/build |
| AUD-P1-007 | FIXED_HOST — boundary regression/ASan; target unverified | Truncated game metadata Unicode escape can advance beyond allocated buffer |
| AUD-P1-008 | FIXED_HOST — source/SDK evidence binding, full stable software/checksum gates | Stable hardware evidence not bound to released production source; missing full software gates |
| AUD-P2-001 | FIXED_HOST/BUILD — checked atomic settings; failure retains RAM and previous file | Settings persistence errors silently return success and truncate old settings |
| AUD-P2-002 | FIXED_BROWSER — bearer icon fetch, bounded blob cache | Native game icon image requests omit bearer authentication |
| AUD-P2-003 | FIXED_HOST — literal bridge title Chromium regression | Bridge game name interpolated into HTML without escaping |
| AUD-P2-004 | FIXED_HOST/BROWSER — confirmed authenticated recovery, preserved corrupt bytes | Corrupt default profile lacks explicit authenticated UI recovery |
| AUD-P2-005 | FIXED_HOST — nonblocking regular-file policy/token/profile/settings reads | FIFO/special-file storage can block before regular-file validation |
| AUD-P2-006 | FIXED_CODE/BUILD — EOF exits capture; source guard | klog EOF spins in capture loop |
| AUD-P2-007 | FIXED_HOST — serialized load/patch/save and concurrent disjoint-update test | Menu and WebUI read-modify-write profile operations can lose updates |
| AUD-P2-008 | FIXED_HOST — pre-body auth, four bounded workers, live slow-client test | Unauthenticated slow request body monopolizes sequential portal until deadline |

First-launch attachment is preserved in FIRST_LAUNCH_TASK.md. Safe-off hooks do not imply a repair of optional hook installation. All physical PS5 tests remain NOT_RUN.

Closure evidence: M1-227 in project-memory/TEST_LOG.md; ADR-053. No unresolved critical code blocker from this register remains after the recorded software checks. Native hook adapters, USB ioctl timing, neutralization, notification pairing and every PS5 runtime result remain NOT_RUN. A host source guard is not a target execution test. Reopen any finding if hardware or later review contradicts the current evidence.
