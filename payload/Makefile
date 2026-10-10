PS5_HOST ?= 192.168.69.44
PORT     ?= 9021
TARGET   := ghost-control-ps5.elf

ifndef PS5_PAYLOAD_SDK
$(error PS5_PAYLOAD_SDK is not set)
endif

include $(PS5_PAYLOAD_SDK)/toolchain/prospero.mk

CFLAGS  += -D__PROSPERO__ -Wall -Wextra -g -O2 -fPIC -fno-stack-protector
LDFLAGS += -lScePad -lSceUserService -lSceSystemService -lpthread -ldl

SRC := gc_main.c runtime_control.c runtime_instance.c credential_guard.c credential_kernel.c runtime_policy.c shellui_pad.c usb_helpers.c usb_resources.c usb_session.c controller_nintendo.c controller_xbox.c controller_steam.c controller_sc2.c steam_usb.c steam_protocol.c steam_mapping.c steam_motion.c steam_feedback.c sc2_control.c sc2_status.c sc2_profile.c slot_registry.c slot_lifecycle.c game_list.c webui.c webui_http.c webui_route.c webui_peer_policy.c webui_auth.c webui_json.c sc2_haptics.c sc2_menu.c sc2_binding.c sc2_gyro.c sc2_flick.c sc2_trigfx.c ds_handoff.c bridge_probe.c bridge/wireless_ds4.c game_hooks.c

.PHONY: all clean deploy

all: $(TARGET)

WEB_ASSETS := $(wildcard web/icons/*.png web/controllers/*.webp) web/controllers/layouts.json
webui_html.h: web/index.html gen_html.py $(WEB_ASSETS)
	python3 gen_html.py

$(TARGET): $(SRC) webui_html.h
	$(CC) $(CFLAGS) -o $@ $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)

deploy: $(TARGET)
	nc -w 8 $(PS5_HOST) $(PORT) < $(TARGET)
