CC := clang
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Werror -pedantic
BUILD := build
ifeq ($(shell uname -s),Darwin)
SC2_SCAN_GC_SECTIONS := -Wl,-dead_strip
else
SC2_SCAN_GC_SECTIONS := -Wl,--gc-sections
endif
TESTS := $(BUILD)/test_vda_registration $(BUILD)/test_vda_pending $(BUILD)/test_storage_diagnostics $(BUILD)/test_vda_result $(BUILD)/test_diagnostics $(BUILD)/test_usb_completion $(BUILD)/test_slot_recovery $(BUILD)/test_user_session $(BUILD)/test_settings_tile_ps5 $(BUILD)/test_settings_tile $(BUILD)/test_webui_server $(BUILD)/test_webui_http $(BUILD)/test_webui_peer_policy $(BUILD)/test_webui_route $(BUILD)/test_webui_json $(BUILD)/test_steam_usb $(BUILD)/test_steam_protocol $(BUILD)/test_steam_parser_fuzz $(BUILD)/test_sc2_usb_scan $(BUILD)/test_game_list_concurrency $(BUILD)/test_diag_once $(BUILD)/test_steam_legacy_session $(BUILD)/test_steam_mapping $(BUILD)/test_steam_motion $(BUILD)/test_steam_feedback $(BUILD)/test_sc2_profile $(BUILD)/test_sc2_profile_selection $(BUILD)/test_sc2_profile_store $(BUILD)/test_sc2_menu_storage $(BUILD)/test_sc2_binding $(BUILD)/test_sc2_control $(BUILD)/test_slot_registry $(BUILD)/test_slot_lifecycle $(BUILD)/test_usb_resources $(BUILD)/test_usb_session $(BUILD)/test_usb_helpers $(BUILD)/test_game_hook_safety $(BUILD)/test_ds_handoff $(BUILD)/test_sc2_status $(BUILD)/test_game_hooks $(BUILD)/test_sc2_gyro_state $(BUILD)/test_sc2_flick $(BUILD)/test_sc2_trigfx_state $(BUILD)/test_sc2_haptics_queue $(BUILD)/test_runtime_control $(BUILD)/test_runtime_instance $(BUILD)/test_credential_guard $(BUILD)/test_runtime_policy

.PHONY: all test sanitize clean browser-fixture

all: test

WEB_ASSETS := $(wildcard ../payload/web/icons/*.png ../payload/web/controllers/*.webp) ../payload/web/controllers/layouts.json
../payload/webui_html.h: ../payload/web/index.html ../payload/gen_html.py ../payload/version.h $(WEB_ASSETS)
	python3 ../payload/gen_html.py

browser-fixture: $(BUILD)/webui_profile_fixture

$(BUILD)/webui_profile_fixture: webui_profile_fixture.c ../payload/sc2_profile.c ../payload/runtime_control.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $(filter %.c,$^) -pthread -o $@

test: $(TESTS)
	./$(BUILD)/test_vda_registration
	./$(BUILD)/test_vda_pending
	./$(BUILD)/test_storage_diagnostics
	./$(BUILD)/test_vda_result
	./$(BUILD)/test_diagnostics
	./$(BUILD)/test_usb_completion
	./$(BUILD)/test_slot_recovery
	./$(BUILD)/test_user_session
	./$(BUILD)/test_settings_tile_ps5
	./$(BUILD)/test_settings_tile
	./$(BUILD)/test_webui_server
	./$(BUILD)/test_webui_http
	./$(BUILD)/test_webui_peer_policy
	./$(BUILD)/test_webui_route
	./$(BUILD)/test_webui_json
	./$(BUILD)/test_steam_usb
	./$(BUILD)/test_steam_protocol
	./$(BUILD)/test_steam_parser_fuzz
	./$(BUILD)/test_sc2_usb_scan
	./$(BUILD)/test_game_list_concurrency
	./$(BUILD)/test_diag_once
	./$(BUILD)/test_steam_legacy_session
	./$(BUILD)/test_steam_mapping
	./$(BUILD)/test_steam_motion
	./$(BUILD)/test_steam_feedback
	./$(BUILD)/test_sc2_profile
	./$(BUILD)/test_sc2_profile_selection
	./$(BUILD)/test_sc2_profile_store
	./$(BUILD)/test_sc2_menu_storage
	./$(BUILD)/test_sc2_binding
	./$(BUILD)/test_sc2_control
	./$(BUILD)/test_slot_registry
	./$(BUILD)/test_slot_lifecycle
	./$(BUILD)/test_usb_resources
	./$(BUILD)/test_usb_session
	./$(BUILD)/test_usb_helpers
	./$(BUILD)/test_game_hook_safety
	./$(BUILD)/test_ds_handoff
	./$(BUILD)/test_sc2_status
	./$(BUILD)/test_game_hooks
	./$(BUILD)/test_sc2_gyro_state
	./$(BUILD)/test_sc2_flick
	./$(BUILD)/test_sc2_trigfx_state
	./$(BUILD)/test_sc2_haptics_queue
	./$(BUILD)/test_runtime_control
	./$(BUILD)/test_runtime_instance
	./$(BUILD)/test_credential_guard
	./$(BUILD)/test_runtime_policy
	python3 test_log_privacy.py
	python3 test_vda_ownership.py
	python3 test_shutdown_contract.py
	python3 test_privilege_contract.py
	python3 test_safe_mode_contract.py
	python3 test_usb_detach_contract.py
	python3 test_release_gate.py
	python3 test_pid_file_contract.py
	python3 test_webui_controllers.py
	python3 test_dev_release.py
	python3 test_changelog.py

sanitize:
	$(MAKE) clean
	$(MAKE) CFLAGS="-std=c11 -O1 -g -Wall -Wextra -Werror -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer" test

$(BUILD):
	mkdir -p $@

$(BUILD)/test_webui_http: test_webui_http.c ../payload/webui_http.c ../payload/webui_auth.c ../payload/webui_html.h | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $(filter %.c,$^) -pthread -o $@

$(BUILD)/test_webui_peer_policy: test_webui_peer_policy.c ../payload/webui_peer_policy.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $(filter %.c,$^) -o $@

$(BUILD)/test_webui_route: test_webui_route.c ../payload/webui_route.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $(filter %.c,$^) -o $@

$(BUILD)/test_webui_json: test_webui_json.c ../payload/webui_json.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $(filter %.c,$^) -o $@

$(BUILD)/test_steam_usb: test_steam_usb.c ../payload/steam_usb.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $(filter %.c,$^) -o $@

$(BUILD)/test_steam_protocol: test_steam_protocol.c ../payload/steam_protocol.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $(filter %.c,$^) -o $@

$(BUILD)/test_steam_parser_fuzz: test_steam_parser_fuzz.c ../payload/steam_protocol.c ../payload/steam_usb.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $(filter %.c,$^) -o $@

$(BUILD)/test_sc2_usb_scan: test_sc2_usb_scan.c ../payload/controller_sc2.c ../payload/steam_protocol.c ../payload/steam_usb.c ../payload/usb_session.c ../payload/usb_resources.c ../payload/runtime_control.c | $(BUILD)
	$(CC) $(CFLAGS) -ffunction-sections -fdata-sections -Ihost_stubs -I../payload $(filter %.c,$^) -pthread $(SC2_SCAN_GC_SECTIONS) -o $@

$(BUILD)/test_game_list_concurrency: test_game_list_concurrency.c ../payload/game_list.c | $(BUILD)
	$(CC) $(CFLAGS) -DGAME_LIST_TEST_HOOKS -DGC_APP_META_DIR_USER='"build/game-list-test/user"' -DGC_APP_META_DIR_SYSTEM='"build/game-list-test/system"' -I../payload $(filter %.c,$^) -pthread -o $@

$(BUILD)/test_diag_once: test_diag_once.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $(filter %.c,$^) -pthread -o $@

$(BUILD)/test_steam_legacy_session: test_steam_legacy_session.c ../payload/controller_steam.c | $(BUILD)
	$(CC) $(CFLAGS) -D_DEFAULT_SOURCE -Ihost_stubs -I../payload $(filter %.c,$^) -pthread -o $@

$(BUILD)/test_steam_mapping: test_steam_mapping.c ../payload/steam_mapping.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $(filter %.c,$^) -o $@

$(BUILD)/test_steam_motion: test_steam_motion.c ../payload/steam_motion.c ../payload/steam_protocol.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $(filter %.c,$^) -lm -o $@

$(BUILD)/test_steam_feedback: test_steam_feedback.c ../payload/steam_feedback.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $(filter %.c,$^) -o $@

$(BUILD)/test_sc2_profile: test_sc2_profile.c ../payload/sc2_profile.c ../payload/runtime_control.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $(filter %.c,$^) -pthread -o $@

$(BUILD)/test_sc2_profile_selection: test_sc2_profile_selection.c ../payload/sc2_profile.c ../payload/runtime_control.c | $(BUILD)
	$(CC) $(CFLAGS) -DSC2_PROFILE_TEST -DSC2_DIR='"build/profile-selection-test"' -I../payload $(filter %.c,$^) -pthread -o $@

$(BUILD)/test_sc2_profile_store: test_sc2_profile_store.c ../payload/sc2_profile.c ../payload/runtime_control.c | $(BUILD)
	$(CC) $(CFLAGS) -D_POSIX_C_SOURCE=200809L -DSC2_PROFILE_TEST -DSC2_PROFILE_STORE_TEST -DSC2_DIR='"build/profile-store-test"' -I../payload $(filter %.c,$^) -pthread -o $@

$(BUILD)/test_sc2_menu_storage: test_sc2_menu_storage.c ../payload/sc2_menu.c ../payload/sc2_profile.c ../payload/runtime_control.c | $(BUILD)
	$(CC) $(CFLAGS) -D_POSIX_C_SOURCE=200809L -DSC2_PROFILE_TEST -DSC2_MENU_TEST -DSC2_DIR='"build/profile-menu-test"' -Ihost_stubs -I../payload $(filter %.c,$^) -pthread -o $@

$(BUILD)/test_sc2_binding: test_sc2_binding.c ../payload/sc2_binding.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $(filter %.c,$^) -pthread -o $@

$(BUILD)/test_sc2_control: test_sc2_control.c ../payload/sc2_control.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $(filter %.c,$^) -pthread -o $@

$(BUILD)/test_slot_registry: test_slot_registry.c ../payload/slot_registry.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $(filter %.c,$^) -pthread -o $@

$(BUILD)/test_slot_lifecycle: test_slot_lifecycle.c ../payload/slot_lifecycle.c ../payload/slot_registry.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $(filter %.c,$^) -pthread -o $@

$(BUILD)/test_usb_resources: test_usb_resources.c ../payload/usb_resources.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $(filter %.c,$^) -o $@

$(BUILD)/test_usb_session: test_usb_session.c ../payload/usb_session.c ../payload/usb_resources.c | $(BUILD)
	$(CC) $(CFLAGS) -Ihost_stubs -I../payload $(filter %.c,$^) -o $@

$(BUILD)/test_usb_helpers: test_usb_helpers.c ../payload/usb_helpers.c ../payload/usb_session.c ../payload/usb_resources.c | $(BUILD)
	$(CC) $(CFLAGS) -D_DEFAULT_SOURCE -Ihost_stubs -I../payload $(filter %.c,$^) -o $@

$(BUILD)/test_game_hook_safety: test_game_hook_safety.c ../payload/game_hook_safety.h | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $< -o $@

$(BUILD)/test_ds_handoff: test_ds_handoff.c ../payload/ds_handoff.c ../payload/runtime_control.c | $(BUILD)
	$(CC) $(CFLAGS) -DDS_HANDOFF_TEST -DDS_HANDOFF_SETTINGS_PATH=\"build/handoff-settings/dualsense.ini\" -I../payload $(filter %.c,$^) -pthread -o $@

$(BUILD)/test_sc2_status: test_sc2_status.c ../payload/sc2_status.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $(filter %.c,$^) -pthread -o $@

$(BUILD)/test_game_hooks: test_game_hooks.c ../payload/game_hooks.c ../payload/runtime_control.c | $(BUILD)
	$(CC) $(CFLAGS) -D_DEFAULT_SOURCE -DGAME_HOOKS_TEST -DGAME_HOOKS_SETTINGS_PATH=\"build/game-hooks-settings/settings.ini\" -Ihost_stubs -I../payload $(filter %.c,$^) -pthread -o $@

$(BUILD)/test_sc2_gyro_state: test_sc2_gyro_state.c ../payload/sc2_gyro.c ../payload/steam_motion.c ../payload/runtime_control.c | $(BUILD)
	$(CC) $(CFLAGS) -DSC2_GYRO_TEST -Ihost_stubs -I../payload $(filter %.c,$^) -pthread -lm -o $@

$(BUILD)/test_sc2_flick: test_sc2_flick.c ../payload/sc2_flick.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $(filter %.c,$^) -pthread -lm -o $@

$(BUILD)/test_sc2_trigfx_state: test_sc2_trigfx_state.c ../payload/sc2_trigfx.c | $(BUILD)
	$(CC) $(CFLAGS) -Ihost_stubs -I../payload $(filter %.c,$^) -pthread -o $@

$(BUILD)/test_sc2_haptics_queue: test_sc2_haptics_queue.c ../payload/sc2_haptics.c ../payload/steam_feedback.c ../payload/usb_resources.c | $(BUILD)
	$(CC) $(CFLAGS) -DSC2_HAPTICS_TEST -Ihost_stubs -I../payload $(filter %.c,$^) -pthread -o $@

$(BUILD)/test_runtime_control: test_runtime_control.c ../payload/runtime_control.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $(filter %.c,$^) -pthread -o $@

$(BUILD)/test_runtime_instance: test_runtime_instance.c ../payload/runtime_instance.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $(filter %.c,$^) -o $@

$(BUILD)/test_credential_guard: test_credential_guard.c ../payload/credential_guard.c ../payload/credential_guard.h | $(BUILD)
	$(CC) $(CFLAGS) -I../payload test_credential_guard.c ../payload/credential_guard.c -o $@

$(BUILD)/test_runtime_policy: test_runtime_policy.c ../payload/runtime_policy.c ../payload/runtime_policy.h | $(BUILD)
	$(CC) $(CFLAGS) -I../payload test_runtime_policy.c ../payload/runtime_policy.c -o $@

clean:
	rm -rf $(BUILD)

$(BUILD)/test_webui_server: test_webui_server.c ../payload/diagnostics.c ../payload/vda_pending.c ../payload/webui.c ../payload/webui_auth.c ../payload/webui_http.c ../payload/webui_route.c ../payload/webui_peer_policy.c ../payload/webui_json.c ../payload/runtime_control.c ../payload/sc2_status.c ../payload/sc2_control.c ../payload/webui_html.h | $(BUILD)
	$(CC) $(CFLAGS) -I../payload -Ihost_stubs $(filter-out ../payload/webui.c,$(filter-out ../payload/webui_auth.c,$(filter %.c,$^))) -pthread -o $@

# Shared storage contracts must invalidate incremental host binaries.
$(BUILD)/test_webui_http $(BUILD)/test_webui_server $(BUILD)/test_sc2_profile $(BUILD)/test_sc2_profile_selection $(BUILD)/test_sc2_profile_store $(BUILD)/test_sc2_menu_storage $(BUILD)/test_ds_handoff $(BUILD)/test_game_hooks: ../payload/private_file.h

$(BUILD)/test_settings_tile: test_settings_tile.c ../payload/settings_tile.c ../payload/settings_tile.h | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $(filter %.c,$^) -o $@

../payload/settings_tile_assets.h: ../payload/gen_settings_tile.py ../payload/settings-tile/icon0.png ../payload/settings-tile/param.json
	python3 ../payload/gen_settings_tile.py

$(BUILD)/test_settings_tile_ps5: test_settings_tile_ps5.c ../payload/settings_tile.c ../payload/settings_tile_ps5.c ../payload/settings_tile.h ../payload/settings_tile_assets.h | $(BUILD)
	$(CC) $(CFLAGS) -I../payload test_settings_tile_ps5.c ../payload/settings_tile.c -o $@

$(BUILD)/test_webui_server: ../payload/settings_tile.h

$(BUILD)/test_user_session: test_user_session.c ../payload/user_session.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $(filter %.c,$^) -pthread -o $@

$(BUILD)/test_slot_recovery: test_slot_recovery.c ../payload/slot_lifecycle.c ../payload/slot_registry.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $(filter %.c,$^) -pthread -o $@

$(BUILD)/test_usb_completion: test_usb_completion.c ../payload/usb_helpers.c ../payload/usb_session.c ../payload/usb_resources.c | $(BUILD)
	$(CC) $(CFLAGS) -D_DEFAULT_SOURCE -Ihost_stubs -I../payload $(filter %.c,$^) -o $@

$(BUILD)/test_diagnostics: test_diagnostics.c ../payload/diagnostics.c ../payload/vda_pending.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $(filter %.c,$^) -pthread -o $@

# Shared contracts invalidate all portable test binaries.
$(TESTS): $(wildcard ../payload/*.h)

$(BUILD)/test_storage_diagnostics: test_storage_diagnostics.c ../payload/webui_auth.c ../payload/webui_auth.h ../payload/private_file.h ../payload/storage_status.h | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $< -pthread -o $@

$(BUILD)/test_vda_result: test_vda_result.c ../payload/slot_registry.c ../payload/vda_result.h | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $(filter %.c,$^) -pthread -o $@

$(BUILD)/test_diagnostics $(BUILD)/test_webui_http $(BUILD)/test_webui_server: ../payload/storage_status.h ../payload/vda_result.h

$(BUILD)/test_vda_pending: test_vda_pending.c ../payload/vda_pending.c | $(BUILD)
	$(CC) $(CFLAGS) -DVDA_PENDING_TEST -I../payload $(filter %.c,$^) -o $@

$(BUILD)/test_vda_registration: test_vda_registration.c ../payload/vda_pending.c ../payload/slot_lifecycle.c ../payload/slot_registry.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $(filter %.c,$^) -pthread -o $@
