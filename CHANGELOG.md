# Changelog

DEV entries describe software and build verification. They do not establish physical PS5 compatibility. Historical releases and tags remain unchanged.

## v0.1.0-dev.5

Published on 2026-10-15 (UTC) from `382e4ecef4937356b72f9e1786917ae7147c0415`. Pending registration uses event correlation; the actual PS5 ABI and native-controller coexistence remain unverified.

### Bug Fixes

- Handled pending virtual-controller registration without repeating device creation or restarting USB input; cancelled registration retains cleanup ownership (host-tested; PS5 verification pending).
- Prevented reconnect from creating another virtual controller while earlier cleanup is unresolved (host-tested).
- Avoided unnecessary permission changes when the settings directory and new credential file already have the required private permissions (host-tested).
- Removed account identifiers from raw application logs and disabled the optional handoff log observer in Safe Mode (host-tested).
- Preserved the original storage error through cleanup and cleared partial credentials after failed initialization (host-tested).

### New Features / Improvements

- Added bounded registration diagnostics and precise storage-stage/errno reporting without exposing credentials or account IDs.
- Added protected session-only WebUI access when persistent credential storage fails; pairing and authentication remain required.

## v0.1.0-dev.4

Published on 2026-10-14 from `a5c20346392eac0eb9fa04cc201cd214e2957363`; physical PS5 testing was not performed.

### Bug Fixes

- Stopped repeated virtual-controller creation after an ambiguous result, including concurrent registration attempts (host-tested).
- Rechecked an opened USB device before claiming its interfaces after hotplug (host-tested; PS5 ioctl validation pending).
- Fixed an overflow while handling an invalid USB feedback status (UBSan-tested).

### New Features / Improvements

None.

## v0.1.0-dev.3

Published on 2026-10-12 from `9aab6dbae166b28416a9a00e6346f85bf12e524e`; physical PS5 testing was not performed.

### Bug Fixes

- Prevented cancelled USB requests from reusing buffers while late completions remain possible; preserved input received during haptic output (host-tested).
- Added serialized virtual-device creation and recovery that retains ownership when deletion fails (host-tested).

### New Features / Improvements

- Added virtual DualSense recovery after confirmed logout and stable login, keeping the Steam USB connection intact (host-tested; PS5 validation pending).
- Added authenticated diagnostic downloads with bounded, filtered application logs.
- Added clearer connection, initialization and recovery messages, plus Payload Manager metadata.
- Embedded the supplied icon and optional PS5 Settings tile installer in the existing single ELF; installation requires explicit confirmation.
- Made a two-section changelog mandatory for future release notes and packaging.

## v0.1.0-dev.2

Published from `6ed81c2ba477285e932bd4593aa029554cadf757`; physical PS5 testing was not performed.

### Bug Fixes

- Kept USB transfer buffers alive through cleanup, rejected invalid completed frames and added stale-input neutralization (host-tested).
- Made profile writes safer during concurrent edits and added explicit recovery for a corrupt Default profile.
- Hardened optional game-hook installation and rollback so failed cleanup retains ownership (host-tested).

### New Features / Improvements

- Enabled the authenticated WebUI on first launch, with a short-lived console pairing code and browser enrollment; manual FTP setup is no longer required.
- Expanded HTTP admission, storage, browser and release-evidence checks.

## v0.1.0-dev.1

First published experimental version, source `ffff5e08905d394b67a7da070fcf4657a250f070`; physical PS5 testing was not performed.

### Bug Fixes

None.

### New Features / Improvements

- Introduced the experimental single-ELF Steam Controller 2026 USB-C/Puck implementation, with virtual DualSense, profiles, motion, trackpads and haptic translation.
- Included an authenticated optional WebUI with image-based remapping and profile import/export.
- Added reproducible Host, sanitizer, Chromium and SDK v0.43 build gates, checksums, source references and safety documentation.
