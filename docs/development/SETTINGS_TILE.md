# Settings tile — one ELF, experimental

The controller payload embeds a 512×512 version of the owner-supplied image and all launcher metadata. No companion ELF or external runtime asset is required.

1. Load the current [dev.6 ELF](https://github.com/MNeroba/SteamController-PS5/releases/tag/dev/v0.1.0-dev.6) through an independently confirmed compatible development loader.
2. Pair with the existing WebUI on a trusted isolated LAN.
3. Open **Settings & help → PS5 tile → Create tile on PS5** and confirm the app registration write.
4. Check **Media → Steam Controller Settings** on the console. A successful HTTP response means registration was accepted, not that the tile has been seen/tested.
5. The tile opens `http://127.0.0.1:8090/`. Keep the payload running. The PS5 browser is a separate authentication session. A notification code enrolls only one browser. If the LAN browser already redeemed it, keep that browser for diagnostics. In persistent mode, the owner may use the existing private recovery token through the pairing dialog; never share it. In **Session only** mode there is no filesystem recovery token and no second-browser enrollment path after redemption. Do not repeatedly reload the payload to obtain another code, especially while VDA registration or cleanup is unresolved. There is no authentication bypass.

The tile is persistent app metadata, not a boot service. After reboot the controller ELF still needs to be loaded normally. No PKG/FPKG is created. PS5 FW13.60 is owner-reported, not measured or verified compatible; registration, visibility, controller navigation and browser loopback remain **NOT_RUN**. A stock console cannot run arbitrary ELF files from this project.

## Safety and errors

The feature is off until an authenticated, same-origin POST with the fixed confirmation body. It writes only `/user/app/SCPS50001/{.steamcontroller-owner,.install.lock,sce_sys/param.json,sce_sys/icon0.png}` and requests registration for that exact title through SDK v0.43's `sceAppInstUtilAppInstallTitleDir`. It never runs global AppInstallAll, edits app databases or writes `/system_ex`, Shell memory, credentials or radio state.

Existing unowned directories, symlinks, FIFOs, hard links, unsafe permissions, altered metadata/icons and simultaneous writers are rejected. Existing bytes are never overwritten. A failed new-file write removes only that newly created file; incomplete owned directories/assets are retained for safe retry. If the platform registration fails, assets are retained because it may already have partly registered the title. The installer library/APIs are resolved only after confirmation, with no mandatory AppInstUtil startup dependency; missing capability fails only this optional action. Initialization failure makes no file changes; termination failure blocks further requests until payload restart. Permissions/platform errors are returned explicitly; no automatic privilege escalation is attempted.

Remove the tile using its normal console **Options → Delete** action. Native uninstall ABI calls and recursive filesystem removal are intentionally absent from the implementation. If the console leaves owned assets, identical files may be reused; conflicting leftovers cause a safe refusal and require a separately reviewed manual repair. Do not delete other titles to resolve a collision.

## Evidence

API/schema references and image hashes: [PROVENANCE.md](../PROVENANCE.md). Software tests cover the filesystem transaction, mocked native control flow, real HTTP auth/origin/method/body gates and browser confirmation/retry presentation. They do not exercise the console installer ABI. [H-13](../TEST_PLAN.md#h-13--optional-settings-tile-on-measured-fw-1360) requires physical-console validation. Keep stock DualSense available and record exact errors rather than assuming upstream firmware claims apply.
