#include "steam_mapping.h"

uint8_t steam_axis_to_u8(int16_t value, int deadzone, int invert) {
    if (value > -deadzone && value < deadzone) return 128u;
    int scaled = ((int)value + 32768) >> 8;
    return (uint8_t)(invert ? 255 - scaled : scaled);
}

uint8_t steam_trigger_to_u8(int16_t value) {
    if (value <= 0) return 0;
    int scaled = value >> 7;
    return (uint8_t)(scaled > 255 ? 255 : scaled);
}

void steam_pad_touch_map(int16_t x, int16_t y, int right_half,
                         uint8_t finger, steam_touch_position_t *out) {
    if (!out) return;
    int tx = ((int)x + 32768) * 959 / 65535;
    int ty = (32767 - (int)y) * 1079 / 65535;
    if (ty < 0) ty = 0;
    out->x = (uint16_t)(tx + (right_half ? 960 : 0));
    out->y = (uint16_t)ty;
    out->finger = finger;
}

uint32_t steam_pad_to_dpad(int16_t x, int16_t y) {
    const int radius = 13000;
    uint32_t buttons = 0;
    if (y >  radius) buttons |= STEAM_DPAD_UP;
    if (y < -radius) buttons |= STEAM_DPAD_DOWN;
    if (x < -radius) buttons |= STEAM_DPAD_LEFT;
    if (x >  radius) buttons |= STEAM_DPAD_RIGHT;
    return buttons;
}
