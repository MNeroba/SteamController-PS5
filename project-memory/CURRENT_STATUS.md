# Current status — 2026-10-11

## Verified source and scope

- Last acknowledged main: `ffff5e08905d394b67a7da070fcf4657a250f070`; DEV1 workflow38075764498 PASS, actual prerelease `dev/v0.1.0-dev.1` has six assets, ELF SHA256 `c908a67cfbe3fb0596b94ef472bc578062ee0a0bb024e683d3242e5f78354aaa`.
- First-launch/dev.2 implementation and 16 static-audit fixes completed locally. Full 255-file clean source export: normal/ASan-UBSan/SDKv0.43LLVM18 PASS; Chromium202 PASS on matching portal, clean-export repeat PASS. Evidence: M1-227, ADR-053, STATIC_AUDIT.md.
- Missing policy enables authenticated WebUI8090 with short-lived local notification pairing. Explicit disable and unsafe/malformed fail-closed policy retained. Hooks/probe/DualSense handoff stay off. USB/Puck ownership, valid input expiration, optional hook rollback, checked persistence and authenticated icons hardened.
- One active Steam Controller2026 session. No physical PS5 test. FW12.00 is owner-reported and unverified. BLE/Edge/custom ShellUI identity remain research.

## Remaining gates / next commands

Clean-export Chromium202, scoped diff/secret/provenance checks and complete-tree verification PASS. Publish reviewed paths onto current full main tree with expected SHA and force=false; never publish the entire inherited dirty checkout. Require successful Host and SDK push Actions on the exact new SHA, then dispatch dev-release.yml on main with source_sha=<exact-tested-40-char-SHA>, version=v0.1.0-dev.2. Verify actual prerelease/tag/six assets/checksum and append acknowledgement; dev.2 is not yet published.

Stable H-01/H-02/H-03/H-04/H-05/H-09/H-12 remain NOT_RUN. Physical notification, loader/runtime, USB cancellation/watchdog, VDA ownership/neutralization, credentials, hooks/unmap/rollback, motion/touch/haptics, console portal filesystem and DualSense coexistence remain hardware-dependent. No console or stock-radio experiment performed.
