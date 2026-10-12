# Repository audit — 2026-10-10

Baseline: `79f92cb5b3fe63656b8454dad9399256b9cf3e62`, tree `302a4d5c86d2bc72a2dd20392067ca8de6fecfe4`; 234 blobs / 250 recursive entries, not truncated. Every local file matched its published Git blob SHA before changes. This report was written before removing or relocating files.

## Scope and decisions

All C/header includes, Makefile source lists and test targets, Python/JS fixtures, workflow gates, documentation links, asset notices and memory records were inspected. All 41 C translation units in `payload/Makefile` remain compiled; legacy controller adapters and firmware-sensitive bridge/ShellUI modules remain because they have build/runtime references. Their presence does not enable them in Safe Mode. No source module or test is removed.

Only `payload/webui_html.h` leaves version control: it is deterministic output of `gen_html.py`. Both Makefiles must generate it from HTML, layouts and 45 image assets; cleaning removes only derived outputs. Root legacy documents are archived, preserving bytes and historical claims as UPSTREAM_CLAIM. The original full CODEX_TASK and build history are preserved under docs/archive before current guidance is condensed.

Memory logs remain at stable paths: hardware gate parsing explicitly reads TEST_LOG.md and historical IDs/SHAs are provenance. A short CURRENT_STATUS and navigation index provide the entry point; no negative result or original record is erased. Git history stays intact; no force push, bundle or archival ref is necessary because no rewrite is proposed.

No ELF, object, SDK, pycache, token or browser output is tracked in the baseline. New ignore rules cover generated header, checksums and Python caches. Supplied controller photos have owner publication permission, not a blanket GPL/CC0 license; separate NOTICE retained. SDK is downloaded from the community project, not vendored.

## File-by-file disposition

KEEP means preserved at its path (some documentation/build files will be updated). ARCHIVE means byte-preserving relocation. GENERATE means reproducible derived output. No REMOVE or MERGE entry was justified for source/tests.

| Baseline file | Decision | Reason / destination | Git blob SHA |
| --- | --- | --- | --- |
| `.gitattributes` | KEEP | Project instructions, build, active documentation, metadata or stable hardware gate | `dfe0770424b2a19faf507a501ebfc23be8f54e7b` |
| `.github/pull_request_template.md` | KEEP | CI/release or contribution contract; stable hardware publication gate retained | `9f6b20d4e65a3a710bee5a39bf3045f28e698afa` |
| `.github/workflows/host-tests.yml` | KEEP | CI/release or contribution contract; stable hardware publication gate retained | `d517e8416292065dbd0a08d1fe0a4deb014a6cbf` |
| `.github/workflows/ps5-cross-build.yml` | KEEP | CI/release or contribution contract; stable hardware publication gate retained | `edb8e2737285a37903f552b175588feb2ee2c73a` |
| `.github/workflows/release.yml` | KEEP | CI/release or contribution contract; stable hardware publication gate retained | `a5015308c32409dcb1db8dfa32d17eb659f13ce7` |
| `.gitignore` | KEEP | Project instructions, build, active documentation, metadata or stable hardware gate | `c08223e7bd320f67fc77dcfe7145d784ef3ccee7` |
| `AGENTS.md` | KEEP | Project instructions, build, active documentation, metadata or stable hardware gate | `7544aae4b08253f8faa0f4e551998f439fdf5003` |
| `CODEX_TASK.md` | KEEP | Current continuation specification; full original preserved at docs/archive/CODEX_TASK-1.1.md | `617c3f0696febbb454070c1c215fd79eddcbc740` |
| `CONTRIBUTING.md` | KEEP | Licensing or external contribution requirements | `9f2095493df3a308e2697ae0ba6a564578324177` |
| `LICENSE` | KEEP | Licensing or external contribution requirements | `f288702d2fa16d3cdf0035b15a9fcbc552cd88e7` |
| `ProControllerResearch.md` | ARCHIVE | Historical upstream/continuation document → `docs/archive/upstream/ProControllerResearch.md`; no local hardware claim | `f7fbd688642db25906eecb84e469c5fb83777edb` |
| `README.md` | KEEP | Project instructions, build, active documentation, metadata or stable hardware gate | `ee36b4326eafe36de7ce28d3801eae040513200b` |
| `README.upstream.md` | ARCHIVE | Historical upstream/continuation document → `docs/archive/upstream/README.upstream.md`; no local hardware claim | `bbc84608c8016056b5e191c902adda7a053d8211` |
| `README.upstream.puckbridge.md` | ARCHIVE | Historical upstream/continuation document → `docs/archive/upstream/README.upstream.puckbridge.md`; no local hardware claim | `43d0865ac4aa2f3c8476587251ad9edb4e859265` |
| `STEAM_CONTROLLER.md` | ARCHIVE | Historical upstream/continuation document → `docs/archive/upstream/STEAM_CONTROLLER.md`; no local hardware claim | `7952f7710fac864716cf734821b4882f9061fe62` |
| `STEAM_CONTROLLER_2026.md` | KEEP | Project instructions, build, active documentation, metadata or stable hardware gate | `dfdd810e6428e068e20a2dc1c354402e0dfdccb9` |
| `docs/ARCHITECTURE.md` | KEEP | Project instructions, build, active documentation, metadata or stable hardware gate | `2b25f16bdea7a7936b77d4b52c4b5e0b25af7b5b` |
| `docs/BASE_SELECTION_ADR.md` | KEEP | Project instructions, build, active documentation, metadata or stable hardware gate | `48c1682d10ae159ca18b85d2a046bde574994042` |
| `docs/BLE_DECISION.md` | KEEP | Project instructions, build, active documentation, metadata or stable hardware gate | `24981115cb58e14a3e817430c276ce93ceb88bca` |
| `docs/BLE_FEASIBILITY.md` | KEEP | Project instructions, build, active documentation, metadata or stable hardware gate | `04e743cbd8e7ed97ed812bb7238398227e352407` |
| `docs/BUILDING.md` | KEEP | Current fresh-checkout guide; original chronological evidence preserved at docs/archive/BUILDING-history.md | `df7f823d7e62e50550c1860215838b0c14dde0fa` |
| `docs/PROJECT_STARTER_README.md` | ARCHIVE | Historical upstream/continuation document → `docs/archive/PROJECT_STARTER_README.md`; no local hardware claim | `83108d3227d57782fbff8b1eb0abca50d90f530f` |
| `docs/PROVENANCE.md` | KEEP | Project instructions, build, active documentation, metadata or stable hardware gate | `8caca1f036f07ac9ee99ca27bce50c9a8bb3f558` |
| `docs/SOURCES_AND_EVIDENCE.md` | KEEP | Project instructions, build, active documentation, metadata or stable hardware gate | `502a6575ab354394affc005d3a522361f5bfe819` |
| `docs/TEST_PLAN.md` | KEEP | Project instructions, build, active documentation, metadata or stable hardware gate | `ccaeb6c87203ff3b7128b62aa1cff125277062fe` |
| `docs/WEBUI_REMAP.md` | KEEP | Project instructions, build, active documentation, metadata or stable hardware gate | `ca6478a1badf552bc122db3690d893401b41f585` |
| `docs/images/remap-desktop-back.png` | KEEP | Real WebUI screenshot; README and mapping documentation | `69ff4633f6d3ef5730ce72bba3da4afe8e291de4` |
| `docs/images/remap-desktop.png` | KEEP | Real WebUI screenshot; README and mapping documentation | `31c07645d1afc69a549b26e269b3afa2a99318a6` |
| `docs/images/remap-editor.png` | KEEP | Real WebUI screenshot; README and mapping documentation | `bd751c7efb56b22fccd021633de38d040d0b9dc7` |
| `docs/images/remap-mobile.png` | KEEP | Real WebUI screenshot; README and mapping documentation | `10ec6f983ea6925014a69e1f2a5b973f0035ed98` |
| `docs/webui-remap-host-results.json` | KEEP | Project instructions, build, active documentation, metadata or stable hardware gate | `d85f99337c546cc51f17037fbe79d8df6533a669` |
| `othercontrollersGuide.md` | ARCHIVE | Historical upstream/continuation document → `docs/archive/upstream/othercontrollersGuide.md`; no local hardware claim | `5e1ad9b93e3cec95b61aecfa4b3da5eb53558d6e` |
| `payload/Makefile` | KEEP | Project instructions, build, active documentation, metadata or stable hardware gate | `5487025c902bc008d4d846e668374dcff7da2f73` |
| `payload/bridge/NOTICE.md` | KEEP | Project instructions, build, active documentation, metadata or stable hardware gate | `93c3a81695bb44f4be5d1e3181fdfcee7a5edfbe` |
| `payload/bridge/pad_types.h` | KEEP | Shared ABI/header; includes: wireless_ds4.c | `9314d84ee6488f592ea672e330a58e9bbeb5d827` |
| `payload/bridge/wireless_ds4.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `7da914ed0553380d44decf2a0295edfe312c8992` |
| `payload/bridge/wireless_ds4.h` | KEEP | Shared ABI/header; includes: ds_handoff.c, bridge_probe.c, game_hooks.c, gc_main.c, wireless_ds4.c | `cc62bf698d0f40917b0039230ece40529f77b806` |
| `payload/bridge_probe.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `e8d3b073cd3d69b0914d105ad4b5675a6c3d8b0d` |
| `payload/bridge_probe.h` | KEEP | Shared ABI/header; includes: webui.c, bridge_probe.c, gc_main.c | `420f7ed939f119b5c8d1a65271facaacc1e80c4a` |
| `payload/controller_nintendo.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `bf960d6749f3811b9340168ffb73f3b8dff0967a` |
| `payload/controller_nintendo.h` | KEEP | Shared ABI/header; includes: controller_nintendo.c, gc_main.c | `4721f71986ddc1f41f39ae5eaa118426e99a369d` |
| `payload/controller_sc2.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `704752bc8f87e0accec23e198ff1dbe627d1beef` |
| `payload/controller_sc2.h` | KEEP | Shared ABI/header; includes: webui.c, controller_sc2.c, gc_main.c | `a3901ed11eb3edd08e805c6294c2e8079dd74b25` |
| `payload/controller_steam.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `1f62e82a7387cb32c3d51ea26583daf0ca4a2c58` |
| `payload/controller_steam.h` | KEEP | Shared ABI/header; includes: controller_steam.c, gc_main.c | `3363b0f164511e746e2aaa75352039cfff571732` |
| `payload/controller_xbox.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `a24b2068bc70dd74db0b08dd0a0f4b2f157be9d4` |
| `payload/controller_xbox.h` | KEEP | Shared ABI/header; includes: controller_xbox.c, gc_main.c | `fa88819ef2371a662ec31c34db330b4229a28f65` |
| `payload/credential_guard.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `2bd3912377ec6c4e3fd3316a30ff27bdcdc8df05` |
| `payload/credential_guard.h` | KEEP | Shared ABI/header; includes: credential_guard.c | `8dc84f91aabac43e64cccb9cba163f27fb1c7e3c` |
| `payload/credential_kernel.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `8da9c8f9863651d8f3beecfe97b7ddf09a00d000` |
| `payload/credential_kernel.h` | KEEP | Shared ABI/header; includes: credential_kernel.c, shellui_pad.c, gc_main.c, wireless_ds4.c | `e25726282aed2cada867b51ef3377ec78a8b066c` |
| `payload/diag_once.h` | KEEP | Shared ABI/header; includes: gc_main.c | `bdfa98a646a0d1e72a35698bd09c52d4a5e660e2` |
| `payload/ds_handoff.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `bcf6d8e24392337f5354fbd52cf80653a7749036` |
| `payload/ds_handoff.h` | KEEP | Shared ABI/header; includes: ds_handoff.c, webui.c, gc_main.c | `534c025de523422dfd88d313d50b02c7617ff6d3` |
| `payload/game_hook_safety.h` | KEEP | Shared ABI/header; includes: game_hooks.c, wireless_ds4.c | `2387b70d140d023857a5556893a1876f74ed7ad6` |
| `payload/game_hooks.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `c6f49b2b83ec5e468c3ed5675e5b181891d29156` |
| `payload/game_hooks.h` | KEEP | Shared ABI/header; includes: webui.c, game_hooks.c, gc_main.c | `8004d8a4a91c3c1d99ca940ad254ad58ed595888` |
| `payload/game_list.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `05afaf2bfa147fd31f648dbf9078fcfee48ab1b0` |
| `payload/game_list.h` | KEEP | Shared ABI/header; includes: game_list.c, webui.c, sc2_profile.c | `c2d9cde4df9d73a2ce1ac82f0fc552633a6d34bb` |
| `payload/gc_main.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `ed8819e60535e967f2521f5da7ced49373e3a990` |
| `payload/gc_types.h` | KEEP | Shared ABI/header; includes: sc2_binding.c, sc2_profile.c, gc_main.c, sc2_menu.c | `739080e3174eae293835e97dabf86c0c3f865028` |
| `payload/gen_html.py` | KEEP | Project instructions, build, active documentation, metadata or stable hardware gate | `a561d1c2f8f0d4cbbb063e533f6bcbdda75ad0e4` |
| `payload/runtime_control.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `219761a94f1d9736e6b5f6af8e89c6f4405ac683` |
| `payload/runtime_control.h` | KEEP | Shared ABI/header; includes: ds_handoff.c, credential_kernel.c, webui.c, runtime_control.c, controller_sc2.c | `b1b9a0cece5b8cf55f0b6a7477aef41212886542` |
| `payload/runtime_instance.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `ad57cfaca8419a664d9664db9060b6579a4f10d8` |
| `payload/runtime_instance.h` | KEEP | Shared ABI/header; includes: runtime_instance.c, gc_main.c | `056b2bdbba9c6e5265b7bade219f8086ea40a402` |
| `payload/runtime_policy.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `67c31330ac3c3c80fb38bd4db273bd7580fb1795` |
| `payload/runtime_policy.h` | KEEP | Shared ABI/header; includes: runtime_policy.c, gc_main.c | `a47c827335984e5ea31b3a0d8fa48bb2721e773a` |
| `payload/sc2_binding.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `1863de90a530fb1f85444b928d035da336c9cbaa` |
| `payload/sc2_binding.h` | KEEP | Shared ABI/header; includes: sc2_binding.c, controller_sc2.c | `cd1d0024282dc4eba91f301fe9239c904f5cc44c` |
| `payload/sc2_control.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `83252cbb8cea49ea21a3935793bc983740a42436` |
| `payload/sc2_control.h` | KEEP | Shared ABI/header; includes: gc_main.c, sc2_control.c | `1568e295ec5254a1339a6329c47c43353203f5b6` |
| `payload/sc2_flick.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `6204d4592baaa33a3b81cbf64076b68b0581a083` |
| `payload/sc2_flick.h` | KEEP | Shared ABI/header; includes: controller_sc2.c, sc2_flick.c | `219171925453b34c3997e5b81ad69f8496cb18e8` |
| `payload/sc2_gyro.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `12a912252f17de7aaa0907c2b81c5f7a8a533c86` |
| `payload/sc2_gyro.h` | KEEP | Shared ABI/header; includes: webui.c, controller_sc2.c, game_hooks.c, sc2_gyro.c, gc_main.c | `2056b487fc968ec6ac3e87d2466f68b68a843700` |
| `payload/sc2_haptics.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `97e9b86fbf83bdbf564c70d9936f80e842a370dc` |
| `payload/sc2_haptics.h` | KEEP | Shared ABI/header; includes: webui.c, sc2_trigfx.c, controller_sc2.c, sc2_haptics.c, game_hooks.c | `0b4327d1572b9f90e67a214851bc59b6d2efbc0d` |
| `payload/sc2_menu.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `d728b55f5d56a533e47a1571d0b564ed554feb0b` |
| `payload/sc2_menu.h` | KEEP | Shared ABI/header; includes: webui.c, controller_sc2.c, gc_main.c, sc2_menu.c | `0644aee3be345b19851e860cf805337577fc61d6` |
| `payload/sc2_profile.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `fb785c3e190859e026733d0ccd5d319e40c768d9` |
| `payload/sc2_profile.h` | KEEP | Shared ABI/header; includes: ds_handoff.c, webui.c, controller_sc2.c, bridge_probe.c, game_hooks.c | `32624775719bf0f5081309591718d340381a771b` |
| `payload/sc2_status.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `3df0074abea5b058b560cd3ad17d479fd53be724` |
| `payload/sc2_status.h` | KEEP | Shared ABI/header; includes: webui.c, sc2_status.c, controller_sc2.c, sc2_haptics.c, game_hooks.c | `cfa95a3ca9d0c03a5d4e2b16ee1b26d00a4bf46c` |
| `payload/sc2_trigfx.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `d6160614454f435446c1e416a1c8b7e12f5b64a4` |
| `payload/sc2_trigfx.h` | KEEP | Shared ABI/header; includes: sc2_trigfx.c, controller_sc2.c, game_hooks.c | `a225e7baa3e0431aadf09ef97163321c1a5df734` |
| `payload/shellui_pad.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `cb57555df062128f7a1fd126d1f13d78cf73eb80` |
| `payload/shellui_pad.h` | KEEP | Shared ABI/header; includes: ds_handoff.c, shellui_pad.c, gc_main.c | `4ca17940f211a8ba0e06e64e32ccfa161fd6b7bf` |
| `payload/slot_lifecycle.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `61c92500d49aff88de5db37d705137b95253640d` |
| `payload/slot_lifecycle.h` | KEEP | Shared ABI/header; includes: slot_lifecycle.c, gc_main.c | `0faafaa347228dd9ce216c3616086837a364c9dc` |
| `payload/slot_registry.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `a23132340ac47d29f74c6b849963b5744311b0a0` |
| `payload/slot_registry.h` | KEEP | Shared ABI/header; includes: slot_registry.c, gc_main.c | `0dccb905c5460d1127bb563498708f1f349721db` |
| `payload/steam_feedback.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `2781bc72ce931d0a7e6f31bfb9b332e387c39884` |
| `payload/steam_feedback.h` | KEEP | Shared ABI/header; includes: steam_feedback.c, sc2_haptics.c | `5772822679e0d0d03462aa6afccd2173eb7d637d` |
| `payload/steam_mapping.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `779be274d40f6b8341853583e8a5aeb0c3bfa1e9` |
| `payload/steam_mapping.h` | KEEP | Shared ABI/header; includes: controller_sc2.c, steam_mapping.c | `b2b488268ccf54e0ac45e5aa6f6a32005730b929` |
| `payload/steam_motion.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `57b7345d590d6175242a5560d2c4dc081cb4b3ba` |
| `payload/steam_motion.h` | KEEP | Shared ABI/header; includes: steam_motion.c | `85824c5945d40bffee911c397ca7a5d36fa9d846` |
| `payload/steam_protocol.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `356d8cd7c45031cf91c60e36ac7e446a6e748774` |
| `payload/steam_protocol.h` | KEEP | Shared ABI/header; includes: controller_sc2.c, steam_protocol.c | `c466781954c8b61edd1e97f85b013f4deef84310` |
| `payload/steam_usb.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `f450969f9a7cd15bed6b1044080d2615b041c0f8` |
| `payload/steam_usb.h` | KEEP | Shared ABI/header; includes: steam_usb.c, gc_main.c | `1b76abeffd5319fdeefea691ec2d6464fbb2f97b` |
| `payload/usb_helpers.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `2b2ff6ac14d54e7c7701a93d8f83ba7ed7774aaf` |
| `payload/usb_helpers.h` | KEEP | Shared ABI/header; includes: controller_nintendo.c, controller_xbox.c, sc2_haptics.c, gc_main.c, usb_helpers.c | `5e5941719cdfc0174b24d91e0d38c9a42ba64970` |
| `payload/usb_resources.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `5a6da3a4c0e67939c0d3babc86c0a9de6fe82f88` |
| `payload/usb_resources.h` | KEEP | Shared ABI/header; includes: usb_resources.c | `7cfa1741e7a9f44ea18d9de57cd054806fe23122` |
| `payload/usb_session.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `5b4863928d2128232944ac35ff2007bc4f00ebcd` |
| `payload/usb_session.h` | KEEP | Shared ABI/header; includes: usb_session.c, gc_main.c | `b321af0ee25eca23bd41141d92f3618fde4471b6` |
| `payload/web/controllers/NOTICE.md` | KEEP | Embedded image/layout or permission notice; generator, geometry and browser checks | `2bca30fbeed8e7cb4d2b3d373b936da9153f1838` |
| `payload/web/controllers/dual-back.webp` | KEEP | Embedded image/layout or permission notice; generator, geometry and browser checks | `d9863fbe251bdaa035f2399c6a1a9fbc5a7c8deb` |
| `payload/web/controllers/dual-front.webp` | KEEP | Embedded image/layout or permission notice; generator, geometry and browser checks | `157c65a1d7c282c2546a1c1bddd7856e0b06a407` |
| `payload/web/controllers/layouts.json` | KEEP | Embedded image/layout or permission notice; generator, geometry and browser checks | `650a4426d313f097f3e63ceb3e68e972ed08888d` |
| `payload/web/controllers/steam-back.webp` | KEEP | Embedded image/layout or permission notice; generator, geometry and browser checks | `52af86ee00cdfda897b763d93e9bea53e9cc79d2` |
| `payload/web/controllers/steam-front.webp` | KEEP | Embedded image/layout or permission notice; generator, geometry and browser checks | `4adc7d0b9ebbfb6635b590d99980628813d1d498` |
| `payload/web/icons/LICENSE.txt` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `554ce582aafb5dc524832f6b8b6c1e3af8f4333d` |
| `payload/web/icons/o_CIRCLE.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `abbae24381fd075a883848675c0d15a99d723db4` |
| `payload/web/icons/o_CREATE.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `49c0bb319ce09e02ed6ded1e1c3e4393b1e16ff2` |
| `payload/web/icons/o_CROSS.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `d83a76a0b0af7086a1baacdbf805447a78efcadf` |
| `payload/web/icons/o_DOWN.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `9af9b66207a2bc34fa6c003e5365af76b74ddcf1` |
| `payload/web/icons/o_L1.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `e1dc81a239882e036c2fa0acc52eeabdf188957f` |
| `payload/web/icons/o_L2.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `6d8807a10f17eaece41ccc0997bc382f7d3ec7fd` |
| `payload/web/icons/o_L3.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `6abf81be0f7b180439886ec7dab2ba2b1153e83a` |
| `payload/web/icons/o_LEFT.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `38d8fd4c3787a0688bf8bc8fd084421ab133ec8a` |
| `payload/web/icons/o_OPTIONS.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `e7bdfe18a97c2d6c32c142ba7ad0c1d58e6452f4` |
| `payload/web/icons/o_R1.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `274fde67614248ee0f4159cdc895d7508ad58ce6` |
| `payload/web/icons/o_R2.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `874fec9c6f21e412f10d0b31e07708b8711c9b0b` |
| `payload/web/icons/o_R3.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `cac9e2fd567b5893bb69102356d9a3bc3383118a` |
| `payload/web/icons/o_RIGHT.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `88d7cee542b5352c5bbced25e9504268ffa89e8e` |
| `payload/web/icons/o_SQUARE.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `5d5977fceaeb3b819be04cae58607f6a232a95d9` |
| `payload/web/icons/o_TOUCHPAD.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `ad9e727748da8304fbba3fdb3e27ec436ccc6cbb` |
| `payload/web/icons/o_TRIANGLE.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `57c4adfc0d12af57c732be7cf1b61f585b830a15` |
| `payload/web/icons/o_UP.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `31db0fb0cb563637a3af36dde4716c384d5ce05b` |
| `payload/web/icons/s_A.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `e19868dfba4e668d6c5d147f585e3ff3e4e91ea1` |
| `payload/web/icons/s_B.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `9381c658dca5d71b031a16998d39408079aed855` |
| `payload/web/icons/s_DDOWN.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `fd1734da258ea3448856685b56d90d6540808927` |
| `payload/web/icons/s_DLEFT.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `97c649326d8676a1800ca885e1d5c34e9b651474` |
| `payload/web/icons/s_DRIGHT.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `b213ea4b9d007a8f5fcc76dab69b01ea64ec9b2a` |
| `payload/web/icons/s_DUP.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `ae258441a89b82fa19784acb2a4190cefa6ef0fe` |
| `payload/web/icons/s_L3.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `de7d93f16e16f92900773018c52ca24d1a02f66e` |
| `payload/web/icons/s_L4.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `ed264129192f43d488363ec6813e01f28bd32930` |
| `payload/web/icons/s_L5.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `3e3b6c69d37246098b699267f616e2d457351d48` |
| `payload/web/icons/s_LB.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `4f9289caf67eadd2e288eb5f75d6ff7efaf1aa62` |
| `payload/web/icons/s_LPAD_CLICK.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `bddf0dbeaebc5d87e717d9220464f045686e2d7d` |
| `payload/web/icons/s_LT.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `18280d22454fac90915b187b693677a5c94d961a` |
| `payload/web/icons/s_MENU.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `0b68a92dcf125196530a22ad02556bfbef279fb1` |
| `payload/web/icons/s_QAM.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `466104dc2bd4b96e2e4f4467467687605172029d` |
| `payload/web/icons/s_R3.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `e362ff396b6f1d6cddb381ee71d6ae40e04b8705` |
| `payload/web/icons/s_R4.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `cfd11000996a9571592c96a66c739e5b96dd805e` |
| `payload/web/icons/s_R5.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `6d6675e1292a6421b772cc0685b64987c5a67e87` |
| `payload/web/icons/s_RB.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `4cc534ea29ddd4bc99355628ad92c65f494b70c6` |
| `payload/web/icons/s_RPAD_CLICK.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `8ff92183c1a016d7aac450ef8a050448def2c65a` |
| `payload/web/icons/s_RT.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `9dfba6ab4a527f6b667b0b8a3f0d3eaa7bb27e72` |
| `payload/web/icons/s_STEAM.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `5251bede8e74c43745b93023fb8f74204ccfc722` |
| `payload/web/icons/s_VIEW.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `ecebf0784d84ef36d4e1d7c8ed5b3d5777e6abdf` |
| `payload/web/icons/s_X.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `aaf07637e09b56e4bf7060804c47415899038175` |
| `payload/web/icons/s_Y.png` | KEEP | Embedded offline prompt asset / CC0 notice; generator and asset tests | `da210dc173648789a962639804d25f4fbf649794` |
| `payload/web/index.html` | KEEP | Project instructions, build, active documentation, metadata or stable hardware gate | `79c2702aee09f517d31397bd762c5e884bc90047` |
| `payload/webui.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `6a4dd3b381a104e43b6208fce8e59716330e09f6` |
| `payload/webui.h` | KEEP | Shared ABI/header; includes: webui.c, gc_main.c | `65fefadaf0117cdca65445175b053b7a0ec2b7c3` |
| `payload/webui_auth.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `83de0b8d0bb58cbbff93e0d3be4447b1c5fed374` |
| `payload/webui_auth.h` | KEEP | Shared ABI/header; includes: webui.c, webui_auth.c | `1acdd8f5701171d587a812ffa68da2fcab99e67e` |
| `payload/webui_html.h` | GENERATE | Derived from HTML, layouts, PNG/WebP assets; Makefile generation required before untracking | `929cb33e6627fb571dacccd3bfeb7faabde40046` |
| `payload/webui_http.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `ac3b6d068e2c40dbcde09245248ab928abc9b11b` |
| `payload/webui_http.h` | KEEP | Shared ABI/header; includes: webui_http.c, webui.c | `63878b14ae93ea592d0dd5ba96f1bc72159baad6` |
| `payload/webui_json.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `257dd3fd9f832da771ef378c1a1315a560a8e0ba` |
| `payload/webui_json.h` | KEEP | Shared ABI/header; includes: webui.c, webui_json.c | `d5efa22a22afaf45ba102f5c4191fdc05b991793` |
| `payload/webui_peer_policy.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `49b15c0a262408fb22b7dcef0c792a2d50a32598` |
| `payload/webui_peer_policy.h` | KEEP | Shared ABI/header; includes: webui_peer_policy.c, webui.c | `50fc44a6d5752df5e1406850516a92f122b082d1` |
| `payload/webui_route.c` | KEEP | Compiled production module in payload/Makefile; no cosmetic relocation | `873259715b328468473a6488953b648fd44ab635` |
| `payload/webui_route.h` | KEEP | Shared ABI/header; includes: webui.c, webui_route.c | `ea4f960aa2c524208e11ea85fa8df6c9227ebced` |
| `project-memory/DECISIONS.md` | KEEP | Append-only evidence/decisions; stable release-gate and continuation paths | `0605cec3043ecb9694c8acca4a7b2025775711ee` |
| `project-memory/FINDINGS.md` | KEEP | Append-only evidence/decisions; stable release-gate and continuation paths | `f9d70e3e4bac213debc88534ba10b1c9b7c5b5ab` |
| `project-memory/HARDWARE_MATRIX.md` | KEEP | Append-only evidence/decisions; stable release-gate and continuation paths | `f5207a75053b152ec0aad78efe8836d8d4519d44` |
| `project-memory/ISSUES.md` | KEEP | Append-only evidence/decisions; stable release-gate and continuation paths | `a91422f7e0883c5a8da7d49e2ae09272a6a1f40d` |
| `project-memory/PROJECT_MEMORY.md` | KEEP | Append-only evidence/decisions; stable release-gate and continuation paths | `2856259ea60598e84549af5341f37b36c4369ddb` |
| `project-memory/SESSION_LOG.md` | KEEP | Append-only evidence/decisions; stable release-gate and continuation paths | `b382430b8b72bd762b8e7a89e1e20a3bf6557e70` |
| `project-memory/TEST_LOG.md` | KEEP | Append-only evidence/decisions; stable release-gate and continuation paths | `77c6b76b2bfac14edbe70c3942146802cbe9b504` |
| `scripts/check_release_gate.py` | KEEP | Project instructions, build, active documentation, metadata or stable hardware gate | `282a8ee0f005384da7af01a158fbb2a14cabb24a` |
| `templates/ADR_TEMPLATE.md` | KEEP | Evidence/ADR/issue templates referenced by execution specification | `335fc910449c81a953f6289b2f602f4cc583c49b` |
| `templates/FINDING_TEMPLATE.md` | KEEP | Evidence/ADR/issue templates referenced by execution specification | `00d8d787c0a27d9a79f71321c25fb6848cdb3c26` |
| `templates/ISSUE_TEMPLATE.md` | KEEP | Evidence/ADR/issue templates referenced by execution specification | `3989e99ed92d3e0f91588c96a5da62f6931c48f2` |
| `templates/TEST_ENTRY_TEMPLATE.md` | KEEP | Evidence/ADR/issue templates referenced by execution specification | `f1b90c29b9c755ecef773dfab4375b34ba97346d` |
| `tests/Makefile` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `547905061d156dd98cad974f3703da849cd91f6a` |
| `tests/host_stubs/dev/usb/usb.h` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `d3c1ba855651f97aa9f94c4e533a2f7086ae1dd3` |
| `tests/host_stubs/dev/usb/usb_ioctl.h` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `f7d12c035f9845d88245e4612cd425f3425c0247` |
| `tests/test_credential_guard.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `70b330ba3b15d634f85421ded37f49d60dfdc8da` |
| `tests/test_diag_once.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `756564b5a936478e1af82d9b7e05ae898d03213a` |
| `tests/test_ds_handoff.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `cc76c2c547f764479d241900333882d0e238bdcc` |
| `tests/test_game_hook_safety.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `f351d5a4ed030695ef76d72fafd3de95194c90f9` |
| `tests/test_game_hooks.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `2fa0ef91b1f40bb16cfb695a1078af901caa2fd0` |
| `tests/test_game_list_concurrency.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `ec78e191e35d2a708e1295ec75c321c52edac87a` |
| `tests/test_pid_file_contract.py` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `e5bea3cd0288017c8a844ccf8a665352b85a3998` |
| `tests/test_privilege_contract.py` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `5f2702c2f4adc792b78b65914cb7168a042afd1b` |
| `tests/test_release_gate.py` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `63e2920b55379483f55c2e0a5199240063153b33` |
| `tests/test_runtime_control.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `422c239ac23d42a1538710701fc240e8a7543355` |
| `tests/test_runtime_instance.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `77783668f89945628eed511ae6643cebc1b5738e` |
| `tests/test_runtime_policy.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `15dc0baf91a113851eb760256cb8edf6c031833d` |
| `tests/test_safe_mode_contract.py` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `2cdd30cf8d8229f9bd322e9dd455f474d763782f` |
| `tests/test_sc2_binding.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `f25126a0851c7ec7022f1bf0e9454144b7b71a4b` |
| `tests/test_sc2_control.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `10954f3e03261d0ed51f82dbe10b0d3e556e5018` |
| `tests/test_sc2_flick.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `cdac5c0aee150c6142ab1dece23aa1b783e5c65a` |
| `tests/test_sc2_gyro_state.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `3b53bafedc443a3829140f9b99516d12623ce16e` |
| `tests/test_sc2_haptics_queue.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `fc5034b96577ba2602dcac1673a1095992723037` |
| `tests/test_sc2_menu_storage.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `b1ca2ae278e5f83ec64bd51254f34a826901bef2` |
| `tests/test_sc2_profile.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `1f7efe01c5fbfb9f8295290f0ee7e730e645c357` |
| `tests/test_sc2_profile_selection.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `8d3f6b6e3447d280c8b7587047fa332a2a29113a` |
| `tests/test_sc2_profile_store.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `7883dc15521a002467e1e21f252b5426f6efaf51` |
| `tests/test_sc2_status.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `7f668ff3beee02c10f53d2047bc83d8dfd044218` |
| `tests/test_sc2_trigfx_state.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `086309cbf90f132e7602f5ee6acc91aff4a291e0` |
| `tests/test_sc2_usb_scan.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `a31eff03fe6c329e71c58728aec0c762d9ed737a` |
| `tests/test_shutdown_contract.py` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `affb6a62ade5cbfe2898fad4c3971d8480983991` |
| `tests/test_slot_lifecycle.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `1915e4efe020034ee0fd3e5653513f4592be6833` |
| `tests/test_slot_registry.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `69b95c8813088d72dd5ad1ed51cff8c1f7818866` |
| `tests/test_steam_feedback.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `8d003e3a3dc2bdca0fe7c2c98aa09c30ab07f487` |
| `tests/test_steam_legacy_session.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `972438a0309d38127a81b99f65a25d9d0b568278` |
| `tests/test_steam_mapping.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `778b5f144d8cdeecead6d2e5fbb9986d8227da7e` |
| `tests/test_steam_motion.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `b3251918c0f75ea5fb8faff656b9426ebe86144f` |
| `tests/test_steam_parser_fuzz.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `4a08b876a1fd22a4a556564d87679f1fd5028e5b` |
| `tests/test_steam_protocol.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `3dd04e141c4c0b3b0d5bbf3a6ccec14cb3493126` |
| `tests/test_steam_usb.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `2e14de36b3700ac33ff5bcc48857811782a0d109` |
| `tests/test_usb_detach_contract.py` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `49cdcc0207101d91234b5e3ee7ea6dafc6a83b4c` |
| `tests/test_usb_helpers.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `eef3f4202b4bf5f4bbf37dc5adc8e4134aa7c956` |
| `tests/test_usb_resources.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `3f35409c0002cd4e570a89590a781f808d9a20f1` |
| `tests/test_usb_session.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `eb40dd59fdeef35e26dfabda07d8f5cf1330f0bd` |
| `tests/test_vda_ownership.py` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `cd8163cd893ba7debcb952844f6d660346fac868` |
| `tests/test_webui_controllers.py` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `262acd3137255af5b43b35180d595794eac40311` |
| `tests/test_webui_http.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `14eb767abd03d0932770bbe39bfc9cdb5632488d` |
| `tests/test_webui_json.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `469c849fa6097f13d5911d079453bd1a54eff562` |
| `tests/test_webui_peer_policy.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `4119d681b00a4dd5c2e6d05c5546e4256da60d6f` |
| `tests/test_webui_route.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `12148e9f6a6a4e4243963a3e75dba3160c012706` |
| `tests/webui_preview.py` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `938305521c5a958b04b404bdb271d540197c6415` |
| `tests/webui_profile_fixture.c` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `15eb4d1a141e66ffb00f32e961fffa5ab6eb83b9` |
| `tests/webui_remap_browser.js` | KEEP | Host regression, contract, browser fixture or USB stub; retained test coverage | `f3c441a13fb311df71524931e0c52a3ae5ed0942` |
| `xboxSeriesSButtonBits.md` | ARCHIVE | Historical upstream/continuation document → `docs/archive/upstream/xboxSeriesSButtonBits.md`; no local hardware claim | `b5480ed828d2601e8a0003702290c594b7525bfc` |

## Implementation follow-up

The pre-consolidation PROJECT_MEMORY summary is preserved verbatim at docs/archive/PROJECT_MEMORY-before-dev.md while its active entry is condensed. Additional files are authored documentation, release scripts/workflow and unit tests. Six upstream documents and the starter note retain their original Git blob SHAs at archive destinations. CODEX_TASK-1.1.md and BUILDING-history.md retain the original bytes as well. No source module, regression or legal notice is removed.


## Public-facing documentation checkpoint — M1-239 (2026-10-12)

Pre-update main3a99866d63c13744c6dc3df90439f70d1c713b90 contains282 blobs,45 production C units. Reviewed documentation/template changes preserve every original path and all source/assets/tests/workflows; no files moved/removed, archives and release history untouched. Added SECURITY.md, two issue forms and PUBLIC_READINESS.md. Updated active README/build/feature/spec/UI/source/BLE/continuation guidance, contributor checklist and manifest review narrative; safety flags/SDK/firmware_verified/critical-blocker list unchanged. Current privacy scope, historical local-path findings and remaining full-history/metadata/security actions are recorded in PUBLIC_READINESS. Exact changed-path inventory and complete-tree verification accompany the publication record.


## Public-history checkpoint — M1-240 (2026-10-12)

Reviewed authoritative mainaf183a2/tree703c039 with286 retained blobs; both branches/all3 tags cover140 complete trees/commits and1,476 distinct blobs. All1,413 text objects scanned;63 images SHA-matched/metadata inspected. No files moved/removed, history rewrite, release/tag/asset mutation or runtime/test/workflow change. Focused edits update PUBLIC_READINESS and current/append-only project memory with scope, negative attempts and remaining owner metadata/security/visibility actions. See F110/M1-240.
