#include "steam_protocol.h"

#include <string.h>

#define RID_INPUT     0x42
#define RID_INPUT2    0x45
#define RID_BATTERY   0x43
#define RID_WIRELESS  0x79
#define INPUT_MIN_LEN 30
#define IMU_MIN_LEN   46

static int16_t read_s16le(const uint8_t *p) {
    uint16_t value = (uint16_t)p[0] | ((uint16_t)p[1] << 8);
    return (int16_t)(value < 0x8000u ? (int32_t)value : (int32_t)value - 0x10000);
}

static uint32_t read_u32le(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static void set_control(steam_input_report_t *report, enum steam_control control,
                        int pressed) {
    if (pressed) report->controls |= (uint32_t)1u << control;
}

static int decode_input(const uint8_t *b, size_t length, steam_input_report_t *r) {
    if (length < INPUT_MIN_LEN) return -1;
    memset(r, 0, sizeof(*r));
    r->report_id = b[0];
    r->sequence = b[1];
    r->wire_length = length;

    set_control(r, STEAM_CTL_A,           b[2] & 0x01);
    set_control(r, STEAM_CTL_B,           b[2] & 0x02);
    set_control(r, STEAM_CTL_X,           b[2] & 0x04);
    set_control(r, STEAM_CTL_Y,           b[2] & 0x08);
    set_control(r, STEAM_CTL_QAM,         b[2] & 0x10);
    set_control(r, STEAM_CTL_R3,          b[2] & 0x20);
    set_control(r, STEAM_CTL_MENU,        b[2] & 0x40);
    set_control(r, STEAM_CTL_R4,          b[2] & 0x80);
    set_control(r, STEAM_CTL_R5,          b[3] & 0x01);
    set_control(r, STEAM_CTL_RB,          b[3] & 0x02);
    set_control(r, STEAM_CTL_DDOWN,       b[3] & 0x04);
    set_control(r, STEAM_CTL_DRIGHT,      b[3] & 0x08);
    set_control(r, STEAM_CTL_DLEFT,       b[3] & 0x10);
    set_control(r, STEAM_CTL_DUP,         b[3] & 0x20);
    set_control(r, STEAM_CTL_VIEW,        b[3] & 0x40);
    set_control(r, STEAM_CTL_L3,          b[3] & 0x80);
    set_control(r, STEAM_CTL_STEAM,       b[4] & 0x01);
    set_control(r, STEAM_CTL_L4,          b[4] & 0x02);
    set_control(r, STEAM_CTL_L5,          b[4] & 0x04);
    set_control(r, STEAM_CTL_LB,          b[4] & 0x08);
    set_control(r, STEAM_CTL_RPAD_CLICK,  b[4] & 0x40);
    set_control(r, STEAM_CTL_RT,          b[4] & 0x80);
    set_control(r, STEAM_CTL_LPAD_CLICK,  b[5] & 0x04);
    set_control(r, STEAM_CTL_LT,          b[5] & 0x08);

    r->right_stick_touched = (b[4] & 0x10) != 0;
    r->right_pad_touched = (b[4] & 0x20) != 0;
    r->right_grip_touched = (b[5] & 0x10) != 0;
    r->left_grip_touched = (b[5] & 0x20) != 0;
    r->left_pad_touched = (b[5] & 0x02) != 0;

    r->left_trigger_raw = read_s16le(b + 6);
    r->right_trigger_raw = read_s16le(b + 8);
    r->left_x = read_s16le(b + 10);
    r->left_y = read_s16le(b + 12);
    r->right_x = read_s16le(b + 14);
    r->right_y = read_s16le(b + 16);
    r->left_pad_x = read_s16le(b + 18);
    r->left_pad_y = read_s16le(b + 20);
    r->right_pad_x = read_s16le(b + 24);
    r->right_pad_y = read_s16le(b + 26);

    if (length >= IMU_MIN_LEN) {
        r->has_imu = 1;
        r->imu_timestamp = read_u32le(b + 30);
        for (int i = 0; i < 3; i++) r->accel[i] = read_s16le(b + 34 + i * 2);
        for (int i = 0; i < 3; i++) r->gyro[i] = read_s16le(b + 40 + i * 2);
    }
    return 0;
}

int steam_report_decode(const uint8_t *data, size_t length, steam_report_t *out) {
    if (!data || !out || length == 0) return -1;
    memset(out, 0, sizeof(*out));
    switch (data[0]) {
        case RID_INPUT:
        case RID_INPUT2:
            out->kind = STEAM_REPORT_INPUT;
            if (decode_input(data, length, &out->data.input) != 0) return -1;
            return 1;
        case RID_BATTERY:
            if (length < 3) return -1;
            out->kind = STEAM_REPORT_BATTERY;
            out->data.battery.state = data[1];
            out->data.battery.level = data[2] > 100 ? 100 : data[2];
            return 1;
        case RID_WIRELESS:
            if (length < 2) return -1;
            out->kind = STEAM_REPORT_WIRELESS;
            out->data.wireless.event = data[1];
            return 1;
        default:
            return 0;
    }
}
