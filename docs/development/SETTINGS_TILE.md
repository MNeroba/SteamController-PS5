# Settings tile — one ELF, experimental

The controller payload embeds a 512×512 version of the owner-supplied image and all launcher metadata. No companion ELF or external runtime asset is required.

1. Load the development ELF with an independently confirmed compatible loader. On first startup, the background worker checks a post-success registration marker plus exact embedded assets and requests registration when the tile is absent, stale or was only staged without a successful platform response.
2. Check **Media → Steam Controller Settings** on the console. A successful installer response is not proof of visibility or operation on your firmware.
3. Open the tile on the PS5; it opens `http://127.0.0.1:8090/` while the payload is running. There is no pairing code. Phones, PCs and other LAN devices cannot access this loopback listener.
4. If automatic registration failed, use an already available PS5 browser/launcher to open that URL and inspect the error before an explicit retry in Settings & help. Do not repeatedly reload while VDA registration or cleanup is unresolved.

The tile is persistent app metadata, not a boot service. After reboot, start the ELF normally. No PKG/FPKG or separate image upload is required. FW13.60, automatic registration and browser behavior remain **NOT_RUN** on physical hardware.

## Safety and errors

The background worker retries registration when the post-success marker or exact assets are missing; a title directory alone is not treated as installed. A manual retry requires a same-origin local POST with the fixed confirmation body. It writes only `/user/app/SCPS50001/{.steamcontroller-owner,.steamcontroller-registered,.install.lock,sce_sys/param.json,sce_sys/icon0.png}` and requests registration for that exact title. All three imports are resolved by their firmware NIDs before any name lookup: `sceAppInstUtilInitialize` `540lotO7oHE`, `sceAppInstUtilTerminate` `kLLazhNh6d4` and `sceAppInstUtilAppInstallTitleDir` `Wudg3Xe3heE`, each with a symbol-name fallback. The NIDs are the FW 13.60 export bindings used by working `ftpsrv`/`websrv`/BFpilot-style payloads; a failed probe names the missing symbol and remains retryable. It never runs global AppInstallAll, edits app databases or writes `/system_ex`, Shell memory, credentials or radio state.

Existing unowned directories, symlinks, FIFOs, hard links, unsafe permissions, altered metadata/icons and simultaneous writers are rejected. Existing bytes are never overwritten. A failed new-file write removes only that newly created file. The first install builds the title directory under `SCPS50001.partial` with its owner marker and renames it into place, so an interrupted install never leaves an unowned title directory; an owned or empty partial directory is discarded on the next attempt, while a foreign one fails safe. Incomplete owned directories/assets are retained for safe retry. If the platform registration fails, assets are retained because it may already have partly registered the title. The installer library/APIs are resolved only after confirmation, with no mandatory AppInstUtil startup dependency; missing capability fails only this optional action. Initialization failure makes no file changes. A failed termination/unload is reported in the message but never turns an accepted registration into an error and never blocks a later retry. Permissions/platform errors are returned explicitly; no automatic privilege escalation is attempted.

Remove the tile using its normal console **Options → Delete** action. Native uninstall ABI calls and recursive filesystem removal are intentionally absent from the implementation. If the console leaves owned assets, identical files may be reused; conflicting leftovers cause a safe refusal and require a separately reviewed manual repair. Do not delete other titles to resolve a collision.

## Evidence

API/schema references and image hashes: [PROVENANCE.md](../PROVENANCE.md). Software tests cover the filesystem transaction, mocked native control flow, real HTTP loopback/Host/origin/method/body gates and browser confirmation/retry presentation. They do not exercise the console installer ABI. [H-13](../TEST_PLAN.md#h-13--optional-settings-tile-on-measured-fw-1360) requires physical-console validation. Keep stock DualSense available and record exact errors rather than assuming upstream firmware claims apply.
