# Current status — 2026-10-11

## Published implementation

- First-launch/dev.2 source published directly on main: `6ed81c2ba477285e932bd4593aa029554cadf757`, parentffff5e08905d394b67a7da070fcf4657a250f070; verified tree50f3f5dd7b26cbddd446de9f0d09498b6c6bb3b6,255 blobs/all41 production C units/all251 previous files retained/zero deletions. Documentation acknowledgement may advance main without changing this implementation.
- All16 confirmed P1/P2 audit findings have scoped fixes. Full clean-source normal/ASan-UBSan/SDKv0.43LLVM18.1.8 gates PASS; Chromium202 PASS twice, including clean export. Evidence M1-227/228, ADR-053, STATIC_AUDIT.md. Scoped diff/credential/attribution checks PASS.
- Missing policy starts authenticated WebUI8090. Pair with the eight-digit local notification code (five minutes/five attempts/one redemption); valid session restoration and advanced token recovery supported. Explicit disable and invalid-file fail-closed behavior preserved. Hooks/probe/DualSense handoff stay off.
- One active Steam Controller2026 session. PS5 FW12.00 owner-reported/unverified. BLE/Edge/custom ShellUI identity research only. No physical console operation performed.

## Current external blocker

At2026-10-11 08:43UTC, Host38125160695 and SDK38125160699 remained queued for more than11 minutes without an assigned runner or executed step. API and GitHub job page both confirm waiting; no CI failure has been reported. Cause is unknown. Official githubstatus.com showed operational, which does not establish why these runs are queued. DEV2 has **not been dispatched or published**; mandatory push gates must first succeed.

## Exact continuation

1. Read memory/specs and fetch main plus these exact runs:
   - https://github.com/MNeroba/SteamController-PS5/actions/runs/38125160695
   - https://github.com/MNeroba/SteamController-PS5/actions/runs/38125160699
2. If either fails, read its logs, fix the actual failure and validate the new full source SHA; do not bypass gates.
3. Once both pass for `6ed81c2ba477285e932bd4593aa029554cadf757`, manually run dev-release.yml from **main**, source_sha=`6ed81c2ba477285e932bd4593aa029554cadf757`, version=`v0.1.0-dev.2`. The authenticated in-app GitHub form was prepared with these inputs; submission deliberately withheld while CI is queued.
4. Require all repeated DEV gates, then verify actual `dev/v0.1.0-dev.2` prerelease/tag/six assets/ELF SHA256 against its downloaded checksum. Append acknowledgement; do not reuse DEV1 ELF or claim publication beforehand.

DEV1 is published at dev/v0.1.0-dev.1, workflow38075764498 PASS, six assets, ELF SHA256c908a67cfbe3fb0596b94ef472bc578062ee0a0bb024e683d3242e5f78354aaa. Local dev.2 clean-source ELF SHA256d299cc90947e051c2d30dc27826f94e7f17108dd945f2f369e8a710921a81b4f is not a published Linux asset digest.

Stable H-01/H-02/H-03/H-04/H-05/H-09/H-12 remain NOT_RUN. Notification delivery, loader/runtime, USB cancellation/watchdog timing, VDA ownership/neutralization, credentials, native hooks/unmap/rollback, motion/touch/haptics, real portal and native DualSense coexistence remain hardware-dependent. No hardware result or historical negative record has been changed.
