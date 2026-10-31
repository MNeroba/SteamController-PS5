# Issues and blockers — project register

## Current — M1-249 dev.6 preparation (2026-10-31)

- **P0-VDA-PENDING-04 — BLOCKED_HARDWARE:** M1-248 now recovers one narrow observer setup failure; registration still depends on existing event correlation and has not been proven on PS5. Latest tester log ends before AddDevice and does not identify whether /dev/klog or loopback failed. Preserve native DualSense; no new ABI/ownership claim.
- **P1-STORAGE-DIAG-04 — HOST FIXED, TARGET UNVERIFIED:** missing-token detection uses the existing no-follow open's ENOENT instead of fstatat(AT_SYMLINK_NOFOLLOW). Strict token creation/validation remains. Tester filesystem cause is a hypothesis until dev.6 runs on hardware.
- **DEV-6 RELEASE CANDIDATE:** version/changelog/metadata/loading guide are being aligned. Exact-source Host/SDK workflows pending after candidate push. The previous source commits passed their own checks; that does not transfer to a versioned release SHA.
- **PUBLICATION:** no dev.6 workflow dispatch/tag/release yet; separate owner approval required. Published dev.5 artifacts unchanged.

## M1-248 tester log (historical intake; source fix now on main)

- P0-VDA-PENDING-04 remains BLOCKED_HARDWARE for actual registration, ownership and native DualSense coexistence. External dev.5 logs show `EILSEQ` before AddDevice. The single-loopback fallback is narrowly gated and host-tested; it does not prove which reader failed on PS5.
- P1-STORAGE-DIAG-04: external dev.5 report gives `file-inspect/EINVAL`; removing the second path lookup preserves strict checks. The exact target filesystem behavior is unverified.
- Published dev.5 is unchanged. Main has the source fixes; there is still no replacement ELF.
