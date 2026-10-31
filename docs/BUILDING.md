# Build and verify from a fresh checkout

Community **PS5 Payload SDK v0.43**, source `d9c9519116944a7f1c22d262012c53b80b3520b7`. SDK ZIP SHA-256:

```text
a9cc9929f21b2b2c5d5b309f3bab4997067c45281c0622cf4838b1aecba66fcb
```

A PS5 cross-build verifies compilation/linking only. FW **13.60 is the intended, unverified test target**; historical FW12.00 records are retained. Historical output hashes and intermediate results are preserved in [build history](archive/BUILDING-history.md) and [TEST_LOG](../project-memory/TEST_LOG.md).

## Prerequisites

- Python 3, `make`, Clang, pthreads; no Sony SDK.
- LLVM **18** and unzip/curl for the community SDK.
- Node **24** and pinned Playwright CLI **0.1.22** only for browser checks.
- The repository is public; cloning and downloading releases do not require a GitHub account.

```sh
git clone https://github.com/MNeroba/SteamController-PS5.git
cd SteamController-PS5
# Optional: rebuild the exact published version rather than current main.
git checkout dev/v0.1.0-dev.6
```

Ubuntu 24.04:

```sh
sudo apt-get update
sudo apt-get install -y make python3 clang-18 lld-18 llvm-18 unzip curl
export LLVM_CONFIG=llvm-config-18
```

macOS with Homebrew:

```sh
brew install llvm@18 python
export LLVM_CONFIG="$(brew --prefix llvm@18)/bin/llvm-config"
```

## Install the checksum-pinned SDK

```sh
mkdir -p .build-cache/ps5-payload-sdk-v0.43
curl -fL -o .build-cache/ps5-payload-sdk-v0.43/ps5-payload-sdk.zip \
  https://github.com/ps5-payload-dev/sdk/releases/download/v0.43/ps5-payload-sdk.zip
# Works on macOS; on Linux you may substitute sha256sum --check.
printf '%s  %s\n' \
  a9cc9929f21b2b2c5d5b309f3bab4997067c45281c0622cf4838b1aecba66fcb \
  .build-cache/ps5-payload-sdk-v0.43/ps5-payload-sdk.zip | shasum -a 256 -c
unzip -q -o .build-cache/ps5-payload-sdk-v0.43/ps5-payload-sdk.zip \
  -d .build-cache/ps5-payload-sdk-v0.43
export PS5_PAYLOAD_SDK="$PWD/.build-cache/ps5-payload-sdk-v0.43/ps5-payload-sdk"
```

SDK/archive/output remain ignored. Preserve applicable SDK notices and corresponding-source obligations when distributing an ELF. See [provenance](PROVENANCE.md).

## Build the real ELF

```sh
make -C payload clean
make -C payload
file payload/ghost-control-ps5.elf
shasum -a 256 payload/ghost-control-ps5.elf
```

`webui_html.h` is **generated, untracked**. Both payload and host Makefiles regenerate it from `web/index.html`, all PNG/WebP assets and `layouts.json`. Do not edit the header or copy it from a previous build. Payload clean removes ELF and generated header, never source assets. Changes to local headers also rebuild the payload.

`build_info.h` and `settings_tile_assets.h` are generated and untracked too. Build identity is derived from the checked-out source; modified/unknown trees must not be described as the published release. Release automation supplies the selected full `SOURCE_SHA` and `DEV_VERSION`; do not override them to mislabel local changes.

The internal ELF/data paths retain upstream names for compatibility. Current published DEV assets use `SteamController-PS5-v0.1.0-dev.6.elf`. The internal build output remains `ghost-control-ps5.elf`. Separate toolchains/paths may produce different debug metadata; no claim of byte-identical ELF across platforms is made. Asset embedding is deterministic.

## Host and sanitizer checks

```sh
make -C tests clean test
make -C tests sanitize
```

These run production portable modules, storage fault injection, protocol/descriptor randomized inputs, binding/motion/feedback, lifecycle/security contracts, asset/hotspot and stable/DEV publication checks. ASan/UBSan do not emulate PS5 hardware.

## Chromium WebUI regression

After host cleaning/building, install the pinned CLI and Chromium:

```sh
npm install --global @playwright/cli@0.1.22
playwright-cli install-browser chromium --with-deps
bash scripts/run_webui_browser.sh
```

The script starts a loopback-only RAM fixture, runs the production C profile parser/formatter behind mocked console routes and requires at least **220 assertions**. Logs/screenshots go to ignored `output/playwright/`; the existing browser scenario also refreshes real UI documentation screenshots. It stops its own fixture/browser. Do not clean `tests/build` while the fixture is running. Edge fallback/manual commands and scope limits: [WEBUI_REMAP](WEBUI_REMAP.md).

## Fresh-source and asset checks

```sh
python3 tests/test_webui_controllers.py
python3 payload/gen_html.py
shasum -a 256 payload/webui_html.h > /tmp/steamcontroller-web.sha256
python3 payload/gen_html.py
shasum -a 256 -c /tmp/steamcontroller-web.sha256
```

A fresh checkout does not contain ELF, SDK, caches, generated header or tokens. GitHub Host and PS5 workflows build it without locally retained files. DEV additionally repeats the full browser/build gates on an explicitly selected successful mainline SHA. [Publication procedure](development/DEV_RELEASE.md).

## Loading and remaining physical tests

Read [LOADING](development/LOADING.md) before sending anything to a development console. No persistent payload boot service is provided. The optional Settings tile persists launcher metadata only and does not start the ELF. Loader compatibility, measured firmware, USB/Puck, native DualSense, motion/touch/haptics and cleanup require physical testing under [TEST_PLAN](TEST_PLAN.md). The stable hardware gate remains closed until matching actual evidence exists.
