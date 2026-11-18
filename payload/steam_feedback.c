#include "steam_feedback.h"

#include <string.h>

static void put16le(uint8_t *p, uint16_t value) {
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8);
}

int steam_feedback_encode_pulse(uint8_t *out, size_t capacity, int side,
                                uint16_t on_us, uint16_t off_us,
                                uint16_t repeat) {
    if (!out || capacity < STEAM_HAPTIC_PULSE_SIZE ||
        side < STEAM_HAPTIC_LEFT || side > STEAM_HAPTIC_BOTH) return -1;
    memset(out, 0, STEAM_HAPTIC_PULSE_SIZE);
    out[0] = 0x81;
    out[1] = (uint8_t)(side < STEAM_HAPTIC_BOTH ? side ^ 1 : STEAM_HAPTIC_BOTH);
    put16le(out + 2, on_us);
    put16le(out + 4, off_us);
    put16le(out + 6, repeat);
    return STEAM_HAPTIC_PULSE_SIZE;
}

int steam_feedback_encode_rumble(uint8_t *out, size_t capacity,
                                 uint16_t left, uint16_t right) {
    if (!out || capacity < STEAM_HAPTIC_RUMBLE_SIZE) return -1;
    memset(out, 0, STEAM_HAPTIC_RUMBLE_SIZE);
    out[0] = 0x80;
    put16le(out + 4, left);
    out[6] = 2;
    put16le(out + 7, right);
    return STEAM_HAPTIC_RUMBLE_SIZE;
}
