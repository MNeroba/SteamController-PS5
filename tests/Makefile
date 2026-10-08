CC := clang
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Werror -pedantic
BUILD := build
TESTS := $(BUILD)/test_webui_http $(BUILD)/test_webui_json $(BUILD)/test_steam_usb $(BUILD)/test_steam_protocol $(BUILD)/test_steam_mapping $(BUILD)/test_steam_motion $(BUILD)/test_steam_feedback $(BUILD)/test_sc2_profile $(BUILD)/test_sc2_binding $(BUILD)/test_sc2_control

.PHONY: all test sanitize clean

all: test

test: $(TESTS)
	./$(BUILD)/test_webui_http
	./$(BUILD)/test_webui_json
	./$(BUILD)/test_steam_usb
	./$(BUILD)/test_steam_protocol
	./$(BUILD)/test_steam_mapping
	./$(BUILD)/test_steam_motion
	./$(BUILD)/test_steam_feedback
	./$(BUILD)/test_sc2_profile
	./$(BUILD)/test_sc2_binding
	./$(BUILD)/test_sc2_control

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

clean:
	rm -rf $(BUILD)
