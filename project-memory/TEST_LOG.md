# Test execution log — append only

**Historical kickoff snapshot:** no Codex implementation/runtime validation had been performed when this project package was created. Later executed results are recorded below; research lookups and document checks do not count as host, SDK or PS5 tests.

| ID | Date (UTC) | Commit | Target | Command / environment | Result | Reason/evidence |
|---|---|---|---|---|---|---|
| INIT-01 | 2026-10-08 | N/A | Host build/tests | NOT_RUN | NOT_RUN | No implementation fork checked out |
| INIT-02 | 2026-10-08 | N/A | PS5 ELF crossbuild | NOT_RUN | NOT_RUN | No selected base/SDK build executed |
| INIT-03 | 2026-10-08 | N/A | PS5 FW 13.60 | NOT_RUN | BLOCKED_HARDWARE | No real-console session executed |
| INIT-04 | 2026-10-08 | N/A | BLE B1-B6 | NOT_RUN | BLOCKED_HARDWARE | No FW 13.60 native LE experiment executed |

### M0 test records

| ID | Date | Commit | Target | Command / environment | Result | Evidence |
|---|---|---|---|---|---|---|
| M0-01 | 2026-10-08 | `78460113ef7c48502c765a1f83ba34d53c977134` | Puckbridge pristine PS5 payload build | `make` in `payload/`; macOS 27.0 arm64, Apple clang 21.0.0, `PS5_PAYLOAD_SDK` unset | FAIL / BLOCKED_ENVIRONMENT | Makefile stops at line 6: `PS5_PAYLOAD_SDK is not set` (exit 2); compilation did not start. |
| M0-02 | 2026-10-08 | `b7682efce18ee0a8d7c80bbc90afbce72151ef96` | OmniPad pristine host test target | `make test` in isolated checkout; macOS 27.0 arm64, Apple clang 21.0.0 | FAIL | Parser, virtual-pad ABI and flow simulation tests pass; target then fails because `tests/test_normalization.c` is missing (`clang: error: no such file or directory`, exit 2). |
| M0-03 | 2026-10-08 | `78460113ef7c48502c765a1f83ba34d53c977134` | Puckbridge host-test inventory | `find . -type f \( -iname '*test*' -o -path '*/tests/*' \) -print` in pinned checkout | PASS (inventory) | No test files found; this is a source inventory, not behavioral validation. |
| M0-04 | 2026-10-08 | N/A | Native PS5 BLE B1-B6 | No physical console or controller available; no HCI/radio command executed | BLOCKED_HARDWARE | See `docs/BLE_DECISION.md`; stock DualSense was not put at risk. |
| M0-05 | 2026-10-08 | Working tree based on `78460113ef7c48502c765a1f83ba34d53c977134` | Portal HTTP framing, bearer auth, token file provisioning | `make -C tests clean sanitize`; macOS 27.0 arm64, Apple clang 21.0.0, `-fsanitize=address,undefined -fno-omit-frame-pointer` | PASS | 53 assertions cover valid requests, incomplete/duplicate/invalid headers and lengths, unsupported transfer encoding, authorization matching, 256-bit token creation/reload, file mode, malformed-file rejection and symlink rejection. Exit 0. |
| M1-01 | 2026-10-08 | Working tree based on `78460113ef7c48502c765a1f83ba34d53c977134` | Portal security, Steam protocol decoder, Steam USB descriptor parser | `make -C tests clean sanitize`; macOS 27.0 arm64, Apple clang 21.0.0, ASan+UBSan | PASS | 179 assertions: web UI/auth/token 57, USB descriptor parser 19, Steam wire parser 103. Exit 0. Does not compile PS5 integration files. |
| M1-02 | 2026-10-08 | Working tree on `fix/p0-safety`, HEAD `78460113ef7c48502c765a1f83ba34d53c977134` | Portal security, USB parser, Steam reports including malformed-input neutral classification | `make -C tests clean sanitize`; Darwin 27.0.0 arm64, Apple clang 21.0.0 (`clang-2100.3.34.2`), ASan+UBSan | PASS | 266 assertions: web UI/auth/token 57, USB descriptor parser 19, Steam wire parser 190. Exit 0. PS5 integration files are not compiled. |
| M1-03 | 2026-10-08 | Working tree on `fix/p0-safety`, HEAD `78460113ef7c48502c765a1f83ba34d53c977134` | Selected payload cross-build attempt | `make -C payload`; Darwin 27.0.0 arm64; `PS5_PAYLOAD_SDK` unset and no `prospero.mk` found under `/Users/mneroba/GitHub` | BLOCKED_ENVIRONMENT (exit 2) | Makefile stops at line 6 before compilation: `PS5_PAYLOAD_SDK is not set`. No ELF or SDK verification. |
| M1-04 | 2026-10-08 | Working tree on `fix/p0-safety`, HEAD `78460113ef7c48502c765a1f83ba34d53c977134` | Portal HTTP/auth/token, bounded JSON, Steam protocol and USB descriptors, mapping, motion math, feedback serialization | `make -C tests clean sanitize`; Darwin 27.0.0 arm64, Apple clang 21.0.0 (`clang-2100.3.34.2`), ASan+UBSan | PASS (exit 0) | 344 assertions: HTTP/auth/token 57, JSON 9, USB config 19, Steam protocol 190, mapping 24, motion 25, feedback 20. Pure host modules only; no PS5 integration files are compiled. |

### GitHub publication and CI

| ID | Date (UTC) | Commit | Target | Command / environment | Result | Evidence |
|---|---|---|---|---|---|---|
| PUB-01 | 2026-10-08 13:05 UTC | GitHub snapshot `4596c7fc4a9876e56cb8194961f15de0fd1e7066`; base README commit `61780b03bcef6859935cac1ef288cf3eef12d3ba` | Private repository initialization and draft PR | GitHub app connector; `MNeroba/SteamController-PS5`, `main` → `fix/p0-safety` | CREATED (DRAFT) | [PR #1](https://github.com/MNeroba/SteamController-PS5/pull/1), 145 changed files; PR metadata reported mergeable. `gh` CLI token remained invalid; connector permission was admin. |
| CI-01 | 2026-10-08 13:06 UTC | `4596c7fc4a9876e56cb8194961f15de0fd1e7066` | GitHub Actions Host tests | Workflow run `37781721767` | QUEUED | No CI conclusion available yet; inspect the run before changing the PR status. |

### New entry template

```
## T-YYYYMMDD-NN
Date/time (UTC):
Issue/gate ID:
Repo/branch and full commit SHA:
Test type: host | sanitizer | fuzz | PS5-cross-build | PS5-hardware | BLE-diagnostic
Exact command or manual procedure:
OS/compiler/SDK/PS5 model/FW/transport/controller firmware:
Fixture URL and SHA256 (if any):
Expected result:
Observed result including exit code:
PASS | FAIL | NOT_RUN | BLOCKED:
Evidence file/hash/link:
Regression or next action:
```

Always retain failed results and add a separate rerun record. If artifact is not physically present, never link to a guessed ELF/path.
