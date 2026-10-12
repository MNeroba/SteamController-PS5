# Project memory — current continuation entry

Read [CURRENT_STATUS](CURRENT_STATUS.md) first, then the append-only evidence below. Historical pre-consolidation summary is preserved verbatim at [PROJECT_MEMORY-before-dev](../docs/archive/PROJECT_MEMORY-before-dev.md); it contains earlier/intermediate statements, not current publication status.

## Identity and foundation

SteamController-PS5, private repository MNeroba/SteamController-PS5, main. Owner uses direct non-force main updates while developing solo; external contributors follow CONTRIBUTING/PR rules.

M0 is complete: PuckbridgePS5 `78460113ef7c48502c765a1f83ba34d53c977134` selected from source comparison; OmniPad `b7682efce18ee0a8d7c80bbc90afbce72151ef96` reference-only, no imported code. PoorDS4/Ghostcontrol/SDK/asset notices retained in PROVENANCE. Payload code is GPL-3.0-or-later; photos have separate owner publication permission.

## Evidence and safety

Host hardening and reusable protocol/USB/slot/profile/motion/touch/feedback/UI components are implemented and covered. Main sourceffff5e0 has green normal/ASan-UBSan/183Chromium and SDK Actions (M1-225). SDK v0.43 archive SHA-256 a9cc9929f21b2b2c5d5b309f3bab4997067c45281c0622cf4838b1aecba66fcb; LLVM18. Physical PS5, controller/Puck, Prospero ioctls/ABI and FW12.00 behavior have never been tested here.

Safe defaults disable bridge probe, game hooks, DualSense handoff and WebUI. Restricted USB policy, owned devices, credential rollback and coordinated cleanup have host tests. Stock DualSense must remain usable; no built-in Bluetooth/HCI or active firmware/system experiment without separate physical evidence/approval.

The owner explicitly authorized an experimental DEV prerelease on2026-10-10. Follow DEV_RELEASE.md and separate manual dev/ workflow with exact source/full software gates and warnings. Stable v* checker remains hardware-gated. DEV authorization does not authorize deployment or claim compatibility.

## Publication and workspace

Desktop checkout: old fix/p0-safety at upstream78460113 with inherited dirty/untracked overlay. GitHub main/full tree is authoritative. All234 local baseline files matched main79f92cb before cleanup. Never reset/stage/push the whole desktop overlay. Publish reviewed allowlisted blobs on complete parent tree, verify all retained hashes and intentional moves/deletion, update expected parent force=false, read back source/CI.

GitHub connector works for Git objects and read-only Actions, not manual workflow dispatch. Edge permission was rejected; authenticated in-app GitHub access subsequently resolved the blocker. Description update and manual DEV dispatch acknowledged; no credentials copied.

## Next safe action

DEV2 publication and actual downloaded checksum are verified (M1-229). Release dev/v0.1.0-dev.2 points to6ed81c2ba477285e932bd4593aa029554cadf757; workflow38141611911 PASS, six assets. Published ELF SHA256bab7c92d8b49b56356a8d616104fc99ef69c49084adb49a76b574e92cb07dfbf. The previous runner queue blocker is resolved. Read CURRENT_STATUS.md, TEST_LOG.md M1-229 and docs/TEST_PLAN.md before continuation. Remaining runtime acceptance requires measured FW13.60 (owner-reported upgrade2026-10-11) and explicitly authorized controlled physical tests; all hardware rows remain NOT_RUN. No firmware update, radio operation or stable release is authorized by DEV publication.

## M1-226 — Audit remediation and first-launch task intake — 2026-10-11

Main ffff5e08905d394b67a7da070fcf4657a250f070 unchanged; no open PRs. DEV workflow 38075764498 succeeded and GitHub confirms dev/v0.1.0-dev.1 prerelease with six assets. Published ELF SHA-256 c908a67cfbe3fb0596b94ef472bc578062ee0a0bb024e683d3242e5f78354aaa. New audit: eight P1/eight P2 confirmed static findings registered in docs/development/STATIC_AUDIT.md, critical code defects added to DEV gate. New owner task FIRST_LAUNCH_TASK.md authorizes automatic authenticated WebUI and dev.2 after all gates. Physical firmware/notification/USB/PS5 remains NOT_RUN. Implementation and new gates pending; no dev.2 published.

M1-227: automatic authenticated WebUI, local notification pairing and 16 static audit findings remediated. Normal, ASan/UBSan and SDK gates PASS on full 255-file source export; Chromium202 PASS on the matching portal, clean-export browser repeat PASS. DEV policy blocks newly discovered critical defects; no hardware or release claim before acknowledgement.

M1-229: exact source push gates and all repeated DEV gates PASS; prerelease dev/v0.1.0-dev.2 and six assets acknowledged, actual downloaded ELF/checksum verified. See CURRENT_STATUS.md for digest and remaining hardware gates.

M1-231: owner reports upgrading the console from12.00 to13.60 on2026-10-11; current target is13.60, unmeasured/unverified. Earlier12.00 records and dev.2 metadata remain historical. SDKv0.43 and safety policy unchanged. New DEV packaging currently pins12.00 and must be adapted with its regression before another release; see CURRENT_STATUS.md. No on-console action was taken.

## M1-232 — dev.3 implementation continuation

Current intended target13.60 (owner report, firmware_verified=false); SDKv0.43. Exactly one ELF, with optional authenticated Settings & help → PS5 tile installation and embedded owner-supplied PNG/metadata. ADR-054/F105/M1-232 document ownership, retry and failure contracts; H-13 is physical-only. Local normal/sanitizer40 C suites, Chromium214 and SDK build PASS; publish only after fresh complete-source gates and remote acknowledgement. Current next commands are in SESSION_LOG M1-232. Existing dev.2 tag/assets/history remain immutable.

M1-233 continuation: select the lazy-loading follow-up source, superseding initial0a3dacb for release. Single ELF no longer imports AppInstUtil at startup; readelf gate protects this. Fresh40 C/normal/sanitizer/Chromium214/SDK checks pass, DEV unit contracts now7. Current feature list/docs include FW13.60 and H-13 NOT_RUN. Remote runner allocation is the current external publication gate; no DEV3 tag/assets exist yet.

M1-234: final owned-directory umask077 readability regression fixed, all normal/sanitizer40 C and7 DEV unit gates pass; SDK/dependency gate passes. The final permission-fix commit supersedes b214da9 for dev.3 selection. Exactly one ELF; no hardware run or release acknowledgement. Use current status for full selected source/next dispatch command once published.

## M1-234 final publication checkpoint — 2026-10-11 20:05 UTC

Implementation source ACK: f480e58b953c75a87d7f1b929789ad023fc05fb1; tree 91184e1a2c60c599356a8752a42433473fc47896; all267 blobs/43 productionC retained, no deletions/unscoped edits. Exact fresh-source export verified. Final local one-ELF digest14447b40d07a1ce9713a808bb2f42d0307a9d1dd8cd318e628b93a111da3755e; workspace payload/ghost-control-ps5.elf and its checksum now match. Local6-file package /tmp/sc-dev3-reviewed-assets has exactly one ELF, correct source SHA, embedded PNG/metadata and matching checksum. Normal/sanitizer40 C +7 DEV contracts, Chromium214 and SDK/dependency gates PASS; on-console FW13.60/H-13 NOT_RUN.

External release blocker: exact-source Host38170245378/job114561164646 and SDK38170245381/job114561164692 still queued, runner_id0, steps0 at20:05:58 UTC. Earlier candidate jobs had the same unassigned state for over16min; obsolete0a3dacb cancellations requested. No compiler failure, global incident or billing cause is proven. dev.3 tag/release/assets NOT_PUBLISHED; do not represent local packaging as a GitHub download. Leave current candidate checks queued; do not bypass the gate.

Next exact action: query latest push runs for f480e58b953c75a87d7f1b929789ad023fc05fb1; only after both succeed, dispatch dev-release.yml on main with source_sha=f480e58b953c75a87d7f1b929789ad023fc05fb1, version=v0.1.0-dev.3 (authenticated browser form is prepared). Then wait repeated gates, verify tag points to selected source, prerelease=true, six assets/one ELF, actual downloaded SHA256 and source; append acknowledgement. Existing dev.2/tag/assets and historical FW12.00 records unchanged. No physical deployment, app registration or radio/firmware operation performed.

## M1-235 — LibrePad v1.1.2 task intake, 2026-10-12

Owner requests integration in existing unpublished dev.3 and explicitly withholds release/tag approval. This supersedes earlier dispatch next actions: DO NOT DISPATCH or publish dev.3 until owner approval. Main6dc554633b133c0dd8c865168010306e38e66384 has267 blobs; all match local bytes. Original dev.3 sourcef480e58b953c75a87d7f1b929789ad023fc05fb1 Host38170245378/SDK38170245381 both completed/success; old runner queue is resolved. No open PR; only dev.1/dev.2 published, unchanged. Reference tagv1.1.2 annotatedbb60defad681563742448faaefa29b440ea83a0e resolves7466317b4b356249e07770f3914c995e1a4d633a. RootGPL3, no code copied yet. Release now warns of rejected symlink storage migration and recommends1.1.3; do not adopt migration. Logout recovery is upstream host evidence only. Preserve single ELF/tile, USB/Puck/security/defaults and /data/ghostpad. Next: audit pinned virtual/USB/diagnostics/metadata source, add regressions, implement smallest owned-worker recovery and bounded authenticated report, changelog/metadata gates, full local validation and reviewed main push; stop before DEV publication. No hardware available; all physical rows NOT_RUN.


## M1-236 — LibrePad audit and regression implementation, 2026-10-12

VERIFIED_CODE: reference LibrePadPS5 v1.1.2 commit7466317b4b356249e07770f3914c995e1a4d633a, src/ps5/virtual_controller.c:375–435/470–650, src/input/usb_device.c:184–252, src/dashboard/http_server.c:315–365, config/payload/LibrePadPS5.elf.json. Root GPL3 inspected; no implementation copied. Storage migration and XInput/8BitDo initialization excluded. Payload Manager schema: itsPLK/ps5-payload-manager e4e72dc9d831dd09ee8fabc222ad1e7f5139050e CUSTOM_REPOSITORIES.md. Prepared dev.3 asset URL is unpublished and private-repository access is required; it is not an available download.

VERIFIED_HOST_TEST targeted: user observer, slot recovery/create reservations/12-thread race, USB IN/OUT completion order/late cancellation/failed operations, diagnostic filtering/bounds/generation/concurrent report, production authenticated WebUI route. Baseline STOP→START reproducer /tmp/sc-usb-cancel-red.c failed before fix(exit134) then passed; logs /tmp/sc-librepad-usb-red.log. /tmp/sc-librepad-usb-build.log targeted USB helpers56/session127/scan PASS. Diagnostic concurrency2000 records+reports PASS. Initial user test cadence499ms fixture, uninitialized helper fixture ppBuffer and package Python expression failed during development; corrected, not passed evidence. Initial server sandbox bind errno1 NOT_RUN; approved loopback retry /tmp/sc-librepad-server.log PASS. First clean SDKv0.43 build /tmp/sc-librepad-sdk-first.log PASS (45 production C); full final repeat pending.

Ownership decisions: three spaced consistent empty reads after previously logged-in users; two same sorted lists for relogin; API/malformed/rapid errors break consensus. Existing USB worker performs proven-handle deletion/re-registration without Steam parser reset, USB restart or automatic account reassignment. Paused/released/disconnected sessions excluded; failed deletion keeps ownership; in-flight creation reserves generation; ambiguous alias blocks future Add for the entire registry and never deletes another slot handle. Startup login-screen remains console-controlled. Foreground/initial fallback UID removed unless positive list membership is observed; button/klog events do not prove account binding.

USB pending IN survives haptic completion multiplexing; unread completions cannot be overwritten. Cancelled indices remain retired until successful FS teardown, never rearmed within that FS. Invalid/absent input expires500ms and neutralizes the owned pad. OUT optional failure need not stop IN; unavailable output remains conservatively retired. Actual Prospero cancellation, VDA receipt meaning, login list transitions/native coexistence and tile/browser NOT_RUN. No physical PS5 operation performed.

Next: finish report/build identity/metadata/changelog gates, repeat full host/sanitizer/Chromium/SDK, publish reviewed main source using complete remote base tree; DO NOT create dev.3 tag/release or dispatch workflow without explicit owner approval.


## M1-237 — Final dev.3 software gates, 2026-10-12

VERIFIED_HOST_TEST: make -C tests clean test /tmp/sc-librepad-final44-normal.log PASS; make -C tests sanitize /tmp/sc-librepad-final44-sanitize.log PASS;44 C suites (four new portable suites) plus existing source/security/release contracts,9 artwork/generator,7 DEV package/policy and5 changelog/metadata tests. No sanitizer errors. Chromium220 PASS output/playwright/librepad-dev3-reviewed/browser.json and /tmp/sc-librepad-browser-reviewed.log: all prior214 assertions plus diagnostic filename/auth/review/errors/retry/size. Earlier final-browser attempt failed profile save because a concurrent make clean deleted its running fixture; sequential rebuild/retry PASS. Future clean/test and browser must run sequentially. Earlier asset dependency-string assertion failed after adding version.h; updated contract verifies the required new version dependency and passes.

VERIFIED_PS5_BUILD: explicit make -C payload clean then default make -C payload with PS5_PAYLOAD_SDK=.build-cache/ps5-payload-sdk-v0.43/ps5-payload-sdk and LLVM_CONFIG=/opt/homebrew/opt/llvm@18/bin/llvm-config, /tmp/sc-librepad-final45-sdk.log PASS,45 production C, no warnings/errors. Optional installer check with --readelf /opt/homebrew/opt/llvm@18/bin/llvm-readelf PASS. Deterministic HTML695105 bytes/41 icons/4 unchanged artwork; header SHA256b88dea7dd597b1dd788cfb909bdaf23c88f6b1717a189d8b42188bd8956b8f0e; tile header1d32115f80e9a306bcef0bf57883f32ac6bd9804cf4851cb28324bb736f73c3d. git diff --check PASS; scoped diff /tmp/sc-librepad-final-review.diff reviewed; no source deletions, generated headers/binaries/credentials excluded. Stable checker correctly rejects NOT_RUN hardware rows (/tmp/sc-librepad-stable-gate.log), no policy weakening.

Late review extended creation reservations to both initial/lazy lifecycle paths and retired unresolved CLOSE transfers as well as STOP; targeted slot85/USB fault checks and all final suites PASS. Reports include global VDA attempts/failures/raw result codes, preserve first failure stage, bound filtering input, avoid noisy notifications and derive download filename from embedded build version. FreeBSD12b769ed36a115da3b5013c92cdf294a3a5de98d read-only cancellation/terminal-close engineering reference supports the conservative design, not Prospero hardware proof. SDKstub exports GetLoginUserIdList verified.

Source commit/complete tree acknowledgement and exact-source local7-file one-ELF checksum package pending next record. No dev.3 tag/release/workflow dispatch or physical PS5 operation performed. Owner review/explicit publication approval is still required; H13/H14/H15 and stable physical gates NOT_RUN.
