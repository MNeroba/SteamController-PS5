#ifndef STEAM_MAPPING_H
#define STEAM_MAPPING_H

#include <stdint.h>

typedef struct {
    uint16_t x;
    uint16_t y;
    uint8_t finger;
} steam_touch_position_t;

enum steam_dpad_direction {
    STEAM_DPAD_UP = 1u << 0,
    STEAM_DPAD_DOWN = 1u << 1,
    STEAM_DPAD_LEFT = 1u << 2,
    STEAM_DPAD_RIGHT = 1u << 3
};

/* Pure Steam raw-control transforms. These functions do not depend on PS5
 * types, profiles, transport, or global state. */
uint8_t steam_axis_to_u8(int16_t value, int deadzone, int invert);
uint8_t steam_trigger_to_u8(int16_t value);
void steam_pad_touch_map(int16_t x, int16_t y, int right_half,
                         uint8_t finger, steam_touch_position_t *out);
uint32_t steam_pad_to_dpad(int16_t x, int16_t y);

#endif
