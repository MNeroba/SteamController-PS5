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
