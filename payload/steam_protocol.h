#ifndef STEAM_PROTOCOL_H
#define STEAM_PROTOCOL_H

#include <stddef.h>
#include <stdint.h>

enum steam_control {
    STEAM_CTL_A, STEAM_CTL_B, STEAM_CTL_X, STEAM_CTL_Y,
    STEAM_CTL_LB, STEAM_CTL_RB, STEAM_CTL_LT, STEAM_CTL_RT,
    STEAM_CTL_L3, STEAM_CTL_R3,
    STEAM_CTL_DUP, STEAM_CTL_DDOWN, STEAM_CTL_DLEFT, STEAM_CTL_DRIGHT,
    STEAM_CTL_VIEW, STEAM_CTL_MENU, STEAM_CTL_STEAM, STEAM_CTL_QAM,
    STEAM_CTL_L4, STEAM_CTL_L5, STEAM_CTL_R4, STEAM_CTL_R5,
    STEAM_CTL_LPAD_CLICK, STEAM_CTL_RPAD_CLICK,
    STEAM_CTL_COUNT
};

enum steam_report_kind {
    STEAM_REPORT_INPUT = 1,
    STEAM_REPORT_BATTERY,
    STEAM_REPORT_WIRELESS
};

typedef struct {
    uint8_t report_id;
    uint8_t sequence;
    uint32_t controls; /* bit positions are enum steam_control */
    int16_t left_trigger_raw, right_trigger_raw;
    int16_t left_x, left_y, right_x, right_y;
    int16_t left_pad_x, left_pad_y, right_pad_x, right_pad_y;
    uint8_t left_pad_touched, right_pad_touched;
    uint8_t right_stick_touched, left_grip_touched, right_grip_touched;
    uint8_t has_imu;
    uint32_t imu_timestamp;
    int16_t accel[3], gyro[3];
    size_t wire_length;
} steam_input_report_t;

typedef struct {
    enum steam_report_kind kind;
    union {
        steam_input_report_t input;
        struct { uint8_t state, level; } battery;
        struct { uint8_t event; } wireless;
    } data;
} steam_report_t;

/* Returns 1 for a decoded known report, 0 for an unrelated report ID, and
 * -1 for a truncated or invalid known report. No PS5 or USB types are used. */
/* On a truncated known input report, returns -1 and leaves kind set to INPUT
 * so a session adapter can neutralize the last published controls. */
int steam_report_decode(const uint8_t *data, size_t length, steam_report_t *out);

#endif
