#ifndef STEAM_FEEDBACK_H
#define STEAM_FEEDBACK_H

#include <stddef.h>
#include <stdint.h>

enum steam_haptic_side {
    STEAM_HAPTIC_LEFT = 0,
    STEAM_HAPTIC_RIGHT = 1,
    STEAM_HAPTIC_BOTH = 2
};

#define STEAM_HAPTIC_PULSE_SIZE 8
#define STEAM_HAPTIC_RUMBLE_SIZE 10

/* Serialize verified upstream report layouts. Returns encoded length, or -1
 * for invalid arguments/capacity. No USB I/O or timing is performed here. */
int steam_feedback_encode_pulse(uint8_t *out, size_t capacity, int side,
                                uint16_t on_us, uint16_t off_us,
                                uint16_t repeat);
int steam_feedback_encode_rumble(uint8_t *out, size_t capacity,
                                 uint16_t left, uint16_t right);

#endif
