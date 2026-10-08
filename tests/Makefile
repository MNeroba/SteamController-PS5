CC := clang
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Werror -pedantic
BUILD := build
TESTS := $(BUILD)/test_webui_http $(BUILD)/test_webui_json $(BUILD)/test_steam_usb $(BUILD)/test_steam_protocol $(BUILD)/test_steam_parser_fuzz $(BUILD)/test_sc2_usb_scan $(BUILD)/test_game_list_concurrency $(BUILD)/test_diag_once $(BUILD)/test_steam_legacy_session $(BUILD)/test_steam_mapping $(BUILD)/test_steam_motion $(BUILD)/test_steam_feedback $(BUILD)/test_sc2_profile $(BUILD)/test_sc2_binding $(BUILD)/test_sc2_control $(BUILD)/test_slot_registry $(BUILD)/test_slot_lifecycle $(BUILD)/test_usb_resources $(BUILD)/test_usb_session $(BUILD)/test_game_hook_safety $(BUILD)/test_ds_handoff $(BUILD)/test_sc2_status $(BUILD)/test_game_hooks $(BUILD)/test_sc2_gyro_state $(BUILD)/test_sc2_flick $(BUILD)/test_sc2_trigfx_state $(BUILD)/test_sc2_haptics_queue

.PHONY: all test sanitize clean

all: test

test: $(TESTS)
	./$(BUILD)/test_webui_http
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
	./$(BUILD)/test_sc2_binding
	./$(BUILD)/test_sc2_control
	./$(BUILD)/test_slot_registry
	./$(BUILD)/test_slot_lifecycle
	./$(BUILD)/test_usb_resources
	./$(BUILD)/test_usb_session
	./$(BUILD)/test_game_hook_safety
	./$(BUILD)/test_ds_handoff
	./$(BUILD)/test_sc2_status
	./$(BUILD)/test_game_hooks
	./$(BUILD)/test_sc2_gyro_state
	./$(BUILD)/test_sc2_flick
	./$(BUILD)/test_sc2_trigfx_state
	./$(BUILD)/test_sc2_haptics_queue

sanitize:
	$(MAKE) clean
	$(MAKE) CFLAGS="-std=c11 -O1 -g -Wall -Wextra -Werror -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer" test

$(BUILD):
	mkdir -p $@

$(BUILD)/test_webui_http: test_webui_http.c ../payload/webui_http.c ../payload/webui_auth.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $^ -o $@

$(BUILD)/test_webui_json: test_webui_json.c ../payload/webui_json.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $^ -o $@

$(BUILD)/test_steam_usb: test_steam_usb.c ../payload/steam_usb.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $^ -o $@

$(BUILD)/test_steam_protocol: test_steam_protocol.c ../payload/steam_protocol.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $^ -o $@

$(BUILD)/test_steam_parser_fuzz: test_steam_parser_fuzz.c ../payload/steam_protocol.c ../payload/steam_usb.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $^ -o $@

$(BUILD)/test_sc2_usb_scan: test_sc2_usb_scan.c ../payload/controller_sc2.c ../payload/steam_protocol.c | $(BUILD)
	$(CC) $(CFLAGS) -ffunction-sections -fdata-sections -Ihost_stubs -I../payload $^ -pthread -Wl,-dead_strip -o $@

$(BUILD)/test_game_list_concurrency: test_game_list_concurrency.c ../payload/game_list.c | $(BUILD)
	$(CC) $(CFLAGS) -DGAME_LIST_TEST_HOOKS -DGC_APP_META_DIR_USER='"build/game-list-test/user"' -DGC_APP_META_DIR_SYSTEM='"build/game-list-test/system"' -I../payload $^ -pthread -o $@

$(BUILD)/test_diag_once: test_diag_once.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $^ -pthread -o $@

$(BUILD)/test_steam_legacy_session: test_steam_legacy_session.c ../payload/controller_steam.c | $(BUILD)
	$(CC) $(CFLAGS) -D_DEFAULT_SOURCE -Ihost_stubs -I../payload $^ -pthread -o $@

$(BUILD)/test_steam_mapping: test_steam_mapping.c ../payload/steam_mapping.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $^ -o $@

$(BUILD)/test_steam_motion: test_steam_motion.c ../payload/steam_motion.c ../payload/steam_protocol.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $^ -lm -o $@

$(BUILD)/test_steam_feedback: test_steam_feedback.c ../payload/steam_feedback.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $^ -o $@

$(BUILD)/test_sc2_profile: test_sc2_profile.c ../payload/sc2_profile.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $^ -pthread -o $@

$(BUILD)/test_sc2_binding: test_sc2_binding.c ../payload/sc2_binding.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $^ -pthread -o $@

$(BUILD)/test_sc2_control: test_sc2_control.c ../payload/sc2_control.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $^ -pthread -o $@

$(BUILD)/test_slot_registry: test_slot_registry.c ../payload/slot_registry.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $^ -pthread -o $@

$(BUILD)/test_slot_lifecycle: test_slot_lifecycle.c ../payload/slot_lifecycle.c ../payload/slot_registry.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $^ -pthread -o $@

$(BUILD)/test_usb_resources: test_usb_resources.c ../payload/usb_resources.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $^ -o $@

$(BUILD)/test_usb_session: test_usb_session.c ../payload/usb_session.c ../payload/usb_resources.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $^ -o $@

$(BUILD)/test_game_hook_safety: test_game_hook_safety.c ../payload/game_hook_safety.h | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $< -o $@

$(BUILD)/test_ds_handoff: test_ds_handoff.c ../payload/ds_handoff.c | $(BUILD)
	$(CC) $(CFLAGS) -DDS_HANDOFF_TEST -I../payload $^ -pthread -o $@

$(BUILD)/test_sc2_status: test_sc2_status.c ../payload/sc2_status.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $^ -pthread -o $@

$(BUILD)/test_game_hooks: test_game_hooks.c ../payload/game_hooks.c | $(BUILD)
	$(CC) $(CFLAGS) -D_DEFAULT_SOURCE -DGAME_HOOKS_TEST -DGAME_HOOKS_SETTINGS_PATH=\"/tmp/steamcontroller-game-hooks-test.ini\" -Ihost_stubs -I../payload $^ -pthread -o $@

$(BUILD)/test_sc2_gyro_state: test_sc2_gyro_state.c ../payload/sc2_gyro.c ../payload/steam_motion.c | $(BUILD)
	$(CC) $(CFLAGS) -DSC2_GYRO_TEST -Ihost_stubs -I../payload $^ -pthread -lm -o $@

$(BUILD)/test_sc2_flick: test_sc2_flick.c ../payload/sc2_flick.c | $(BUILD)
	$(CC) $(CFLAGS) -I../payload $^ -pthread -lm -o $@

$(BUILD)/test_sc2_trigfx_state: test_sc2_trigfx_state.c ../payload/sc2_trigfx.c | $(BUILD)
	$(CC) $(CFLAGS) -Ihost_stubs -I../payload $^ -pthread -o $@

$(BUILD)/test_sc2_haptics_queue: test_sc2_haptics_queue.c ../payload/sc2_haptics.c ../payload/steam_feedback.c | $(BUILD)
	$(CC) $(CFLAGS) -DSC2_HAPTICS_TEST -Ihost_stubs -I../payload $^ -pthread -o $@

clean:
	rm -rf $(BUILD)
