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

On 2026-10-08, after the portal request-deadline and strict game-hook settings parsing changes, the same SDK v0.43 / LLVM 18.1.8 command succeeded on Darwin arm64. Output: ELF64 x86-64 PIE, SHA-256 `0f9e78bc4be21a60b2dfde0ab76da6b74e04fb1ae10edd3aa4df6c741a45aab2`. This confirms compilation and linking only; it is not evidence of console loading or firmware compatibility. See M1-153 in `project-memory/TEST_LOG.md`.
