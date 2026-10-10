# PS5 payload cross-build

The payload uses the community [ps5-payload-dev SDK](https://github.com/ps5-payload-dev/sdk), pinned to release v0.43 / source commit `d9c9519116944a7f1c22d262012c53b80b3520b7`. The release asset is `ps5-payload-sdk.zip`, with SHA-256 `a9cc9929f21b2b2c5d5b309f3bab4997067c45281c0622cf4838b1aecba66fcb`. The release commit is unsigned in GitHub's verification metadata; the build verifies the archive against the digest published by GitHub.

On macOS, the SDK's instructions require Homebrew LLVM 18 and `LLVM_CONFIG`. The following keeps SDK files in an ignored project-local cache and checks the archive before extraction:

```sh
brew install llvm@18
export LLVM_CONFIG="$(brew --prefix llvm@18)/bin/llvm-config"

mkdir -p .build-cache/ps5-payload-sdk-v0.43
curl -fL -o .build-cache/ps5-payload-sdk-v0.43/ps5-payload-sdk.zip \
  https://github.com/ps5-payload-dev/sdk/releases/download/v0.43/ps5-payload-sdk.zip
printf '%s  %s\n' \
  a9cc9929f21b2b2c5d5b309f3bab4997067c45281c0622cf4838b1aecba66fcb \
  .build-cache/ps5-payload-sdk-v0.43/ps5-payload-sdk.zip | shasum -a 256 -c
unzip -q -o .build-cache/ps5-payload-sdk-v0.43/ps5-payload-sdk.zip \
  -d .build-cache/ps5-payload-sdk-v0.43

export PS5_PAYLOAD_SDK="$PWD/.build-cache/ps5-payload-sdk-v0.43/ps5-payload-sdk"
python3 payload/gen_html.py
make -C payload clean
make -C payload
```

The release workflow uses the same SDK version and SHA-256 check before building. Generated `.elf` output and `.build-cache/` are ignored by Git.

## Recorded build

On 2026-10-08, the payload built successfully on Darwin 27.0.0 arm64 with Homebrew LLVM 18.1.8 and SDK v0.43. The output was a 64-bit x86-64 PS5 shared-object ELF; its SHA-256 is recorded in `project-memory/TEST_LOG.md` (test M1-15). The build checked compilation and SDK linker stubs only. It did not run on a PS5, verify a specific firmware, or validate USB/Puck hardware behavior.

The SDK repository describes most of its files as GPL-3.0-or-later and identifies exceptions such as BSD-licensed FreeBSD headers. The SDK distribution is not included in this repository. Check the applicable source notices and corresponding-source obligations before distributing a built payload.

## Latest rebuild

On 2026-10-08, after the portal request/response deadlines, strict game-hook settings, and exact portal route parsing changes, the same SDK v0.43 / LLVM 18.1.8 command succeeded on Darwin 27.0.1 arm64. Output: ELF64 x86-64 PIE, SHA-256 `2704d75b96c0c5db5756935ebb1106c7a376965242242ebc4f5965c8648d486c`. This confirms compilation and linking only; it is not evidence of console loading or firmware compatibility. See M1-170 in `project-memory/TEST_LOG.md`.

After adding same-host Origin checks and strict profile request-body metadata, the same command succeeded again. Current ELF64 x86-64 PIE SHA-256: `1084457bba0e35a479b6bed5e0578027567decc199d0d3ae6724c301e74cd6a1`. Compile/link only; no browser, PS5 socket, or console behavior was exercised. See M1-176 in `project-memory/TEST_LOG.md`.

After adding the default IPv4 peer-scope check to the portal accept loop, the SDK v0.43 / LLVM 18.1.8 build succeeded again on Darwin 27.0.1 arm64. ELF64 x86-64 PIE SHA-256: `03c1cbdaa64b8d9d222329c51f5fe24222cda464250a4c6f52644741fa3a1948`. This verifies compile/link only; it does not validate Prospero socket address behavior or console networking. See M1-182 in `project-memory/TEST_LOG.md`.


## Complete `main` tree validation

On 2026-10-08, the complete project was published to `main` at commit `0f122c64eff0b196ba023c75af9a61ba1cc9514a`. GitHub reports 195 files; a local clean export produced the identical Git tree SHA `9834f600c9b77be4d2f7285c1b3f21fe70658e99`. From that export, `make clean test` and `make clean sanitize` both passed (1048 checks each), and the SDK v0.43 cross-build succeeded with ELF SHA-256 `26dce830d274755fc0bd6a83cafc8e3e6f660e756544d699a2b346ef445afbf0`. This is host-test and compile/link evidence only. A direct network `git clone` could not complete because this environment could not resolve `github.com`; the build ran from a clean source export verified against the exact GitHub tree. GitHub push workflows were queued after publication.

## M1-194 scanner operation-table rebuild

On 2026-10-08, the scanner operation-table change built with the same pinned SDK v0.43 and LLVM 18.1.8 command. The ELF is 64-bit x86-64, with SHA-256 `1af60f12c8d5f34860beb82975db609d29dc1d8e1635d1b17cc7084f134e0e78`; `llvm-readobj` lists the expected PS5 SDK runtime libraries. This verifies compilation/linking only. No real USB FS ioctl, console, Steam Controller, Puck, or firmware behavior was tested. Exact environment and command are in M1-194 of `project-memory/TEST_LOG.md`.

The validated scanner source and this build record were published to `main` in commit `6bda48be9c544898451e60573fb8fc363fd4505e` (tree `1fce5b32bd377cb791ca42df0c2d7e29c97cd6d1`). GitHub's acknowledged recursive tree contained all 196 source blobs across 209 entries; this tree check does not add PS5 runtime evidence. Actions run IDs and their actual state are tracked in `project-memory/TEST_LOG.md`.

## M1-195 descriptor fallback classification

On 2026-10-08, the SC2 descriptor-status/fallback change built with the same pinned SDK v0.43 and LLVM 18.1.8 command. The ELF is 64-bit x86-64, with SHA-256 `87fa9f441430dd1235654e308988bf28144b2fe5a241f692ea9456f1b9d6d73b`; `llvm-readobj` lists the expected PS5 SDK runtime libraries. This verifies compilation/linking only. It does not exercise `USB_GET_FULL_DESC`, `ENXIO` handling, device removal, console runtime or real Steam Controller/Puck behavior. Exact command and host test results are in M1-195 of `project-memory/TEST_LOG.md`.

The M1-195 source/docs snapshot was published to `main` as commit `72048849da463b1e18a9c91e07d2f64926f6e6af` (tree `ac4e13add70bcf641b6c83114aa1e8d743401f8b`). GitHub's recursive-tree acknowledgement contained all 196 blobs across 209 entries; this confirms source publication, not target runtime behavior.


## M1-197 USB configuration count validation

On 2026-10-08, after strict validation of `bNumInterfaces`, per-interface/alternate-setting `bNumEndpoints`, duplicate default interface descriptors, and selected-map capacity, the same pinned SDK v0.43 / LLVM 18.1.8 cross-build succeeded on Darwin 27.0.1 arm64. The output `payload/ghost-control-ps5.elf` is an ELF64 x86-64 shared object with SHA-256 `5b4ab572d235b2a5e87f4e4f4d031d37ac9b0d3e84ce2a6b2d6103e89055f551`; `llvm-readobj` reports the expected PS5 SDK libraries. This confirms compilation and linking only. No USB ioctl, Steam Controller, Puck, console, or firmware behavior was exercised; see M1-197 in `project-memory/TEST_LOG.md`. The complete source/evidence snapshot was published to main at commit bd833e3f1914610e2e9250e721796da3b3ceec2f (tree 2db08d87687c3e5381bb1f81fd8a14a1ac10e762); GitHub acknowledged all 196 blobs across 209 entries.


## M1-198 fixed standard USB descriptor lengths

On 2026-10-08, after requiring standard configuration/interface/endpoint descriptor lengths of 9/9/7 bytes, the pinned SDK v0.43 / LLVM 18.1.8 cross-build succeeded on Darwin 27.0.1 arm64. The output payload/ghost-control-ps5.elf is an ELF64 x86-64 shared object with SHA-256 362d584f60d26a0206a983c55980ebe7083f4c05fc9bd7adf9b878839a49fa1c; llvm-readobj reports the expected PS5 SDK libraries. This confirms compilation and linking only. No USB ioctl, Steam Controller, Puck, console, or firmware behavior was exercised; see M1-198 in project-memory/TEST_LOG.md. M1-198 was published to main at commit e84f792fb37c47a332f4b914b25fbd74640b7cf0 (tree dfb72ab1b4dad7f3aac9d8bdbd672f9deaa0385a); GitHub confirms all 196 blobs across 209 entries.

## Current reproducible build and Actions artifact

The current source snapshot on `main` is commit `0b00c987d4c4e8ed80d52c9e2bbe723a1cd68b79`. GitHub Actions Host tests (`37986069382`) and PS5 cross-build (`37986069353`) both completed successfully for that commit. The cross-build run uploaded the temporary artifact `ghost-control-ps5-cross-build`, containing `ghost-control-ps5.elf` and its `.sha256` file. Open the [PS5 cross-build workflow](https://github.com/MNeroba/SteamController-PS5/actions/workflows/ps5-cross-build.yml) and download the artifact from the latest successful run. Artifacts expire under repository retention settings; they are not Releases and do not prove that the ELF loads or works on a PS5.

An independent local rebuild of the same 218-blob source tree passed on 2026-10-10 with macOS 27.0.1 arm64, Apple clang 21.0.0, Homebrew LLVM 18.1.8, and SDK v0.43. The cached SDK archive matched the documented SHA-256 `a9cc9929f21b2b2c5d5b309f3bab4997067c45281c0622cf4838b1aecba66fcb`. Commands: `make -C tests clean test`, `make -C tests clean sanitize`, `make -C payload clean`, and `make -C payload`; all passed. The locally produced ELF SHA-256 is `b78a4a5967921f463adbc54afecf866f6561f2d96108a1aeba475631c79249f7`; `llvm-readobj` reports `libSceLibcInternal.sprx`, `libSceNet.sprx`, `libScePad.sprx`, `libSceSystemService.sprx`, `libSceUserService.sprx`, and `libkernel_web.sprx`. This is host-test and compile/link evidence only. No PS5 runtime or hardware test was performed.

## Final no-console verification — 2026-10-10

The payload source is unchanged from main source commit 0b00c987d4c4e8ed80d52c9e2bbe723a1cd68b79. The finalized README, build guidance, issue statuses and project memory are on main at 6b727b6502585c90878944a00d3fc29d6ef2bc36. Both workflows passed on that commit: [Host tests](https://github.com/MNeroba/SteamController-PS5/actions/runs/38047930840) and [PS5 cross-build](https://github.com/MNeroba/SteamController-PS5/actions/runs/38047930852). GitHub uploaded artifact ghost-control-ps5-cross-build (ID 11668467159; ZIP SHA-256 8701025029d3fa43c77c8a2eeafea3a7526cf71aefcac6e52db92455dfcceceb), available until 2027-01-08T11:18:17Z. Download it from the linked workflow run. It is a temporary build output, not a release or hardware proof.

Local make -C tests clean test and make -C tests clean sanitize also passed. The exact source tree has 218 blobs and its recursive GitHub tree is not truncated.

For the local SDK build, export both variables before invoking Make. On the recorded macOS setup, set LLVM_CONFIG to $(brew --prefix llvm@18)/bin/llvm-config and PS5_PAYLOAD_SDK to $PWD/.build-cache/ps5-payload-sdk-v0.43/ps5-payload-sdk, then run make -C payload clean followed by make -C payload.

This run produced ELF SHA-256 b78a4a5967921f463adbc54afecf866f6561f2d96108a1aeba475631c79249f7 and linked against the six libraries listed above. It confirms compilation/linking only. No payload deployment, console loading, USB/Puck operation, firmware compatibility or runtime behavior was tested. The required hardware rows remain blocked; the CI artifact is temporary and is not a release.

## Re-run after firmware-target update — 2026-10-10

After removing the release check's fixed 13.60 requirement and updating the target to owner-reported FW 12.00, both host commands passed again: `make -C tests clean test` and `make -C tests clean sanitize`. The clean PS5 cross-build also passed with SDK v0.43 and Homebrew LLVM 18.1.8 using the environment documented above. The ELF SHA-256 is unchanged at `b78a4a5967921f463adbc54afecf866f6561f2d96108a1aeba475631c79249f7`, with the same six expected SDK libraries. These tests validate source policy, host behavior, and compilation/linking only; no PS5 12.00 console or controller was tested, and compatibility is not claimed.


GitHub Actions on the FW 12.00-target documentation/checker commit `81501afa53e2acd329db0298ffccdc533161e749` passed both [Host tests](https://github.com/MNeroba/SteamController-PS5/actions/runs/38053866134) and [PS5 cross-build](https://github.com/MNeroba/SteamController-PS5/actions/runs/38053866148). The cross-build uploaded temporary artifact `ghost-control-ps5-cross-build` (ID `11670507757`, SHA-256 `7259c645662dcdd0e664c0d95d6ccde17fa453992fc81e032eb6d9b822b622ba`, expires 2027-01-08T12:57:22Z). It is not a release and does not establish console runtime or firmware compatibility.

## Image remapping build — 2026-10-10

At the initial image-remapping publication, the generated page was 672,184 bytes and seven embedding tests passed. Make tracks the WebP/layout/icon dependencies. The follow-up calibration and Remap UX verification are recorded below. Compile/link and local-fixture browser evidence do not establish PS5 FW 12.00 loading or operation. No Release or console deployment occurred.

### Remap UX follow-up — 2026-10-10

DualSense L1/R1 overlays now target the upper shoulder buttons, with L2/R2 on the lower triggers; triangle and circle centers were calibrated against the supplied artwork. The generated page is 674,696 bytes. Eight portable asset/geometry/embedding tests, the full normal suite, and ASan/UBSan pass. SDK v0.43 / Homebrew LLVM 18.1.8 clean cross-build passes; current ELF SHA-256: `3f19ca8a85af5583840ccc9542aa91796740b6b166d6ae553723e7b588565fbf`. The six expected SDK runtime libraries are unchanged. The local Microsoft Edge fixture passes 114 browser checks. See `docs/WEBUI_REMAP.md` and `project-memory/TEST_LOG.md` for exact commands and limits.

The image-remapping source is published at `4065ff41d51b86fdf3a60f972f72b2b24855c647`. [Host tests](https://github.com/MNeroba/SteamController-PS5/actions/runs/38061016300) and [PS5 cross-build](https://github.com/MNeroba/SteamController-PS5/actions/runs/38061016345) both pass for that exact commit. The PS5 workflow uploaded temporary artifact `ghost-control-ps5-cross-build` (ID 11673116887, ZIP SHA-256 `bf4acba960df78e5baa70c46a17dcaa2d7b0824ceeed33cb0dedb192c8d951ca`, expires 2027-01-08T14:47:32Z). This is an available compiled output, not a console-verified release.
