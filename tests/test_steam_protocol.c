#include "steam_protocol.h"

#include <stdio.h>
#include <string.h>

static int failures;
static int checks;

#define CHECK(cond) do { \
    checks++; \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        failures++; \
    } \
} while (0)

#define PRESSED(input, id) (((input).controls & ((uint32_t)1u << (id))) != 0)

static void put_s16(uint8_t *p, int16_t value) {
    uint16_t raw = (uint16_t)value;
    p[0] = (uint8_t)raw;
    p[1] = (uint8_t)(raw >> 8);
}

static void test_full_input_report(void) {
    uint8_t wire[54] = {0};
    steam_report_t report;
    wire[0] = 0x42;
    wire[1] = 0x7f;
    wire[2] = 0x99; /* A, Y, QAM, R4 */
    wire[3] = 0x17; /* R5, RB, DDOWN, DLEFT */
    wire[4] = 0xff; /* all right-side/button flags */
    wire[5] = 0x3e; /* left pad, Lpad click, LT, both grips */
    put_s16(wire + 6, 16384);
    put_s16(wire + 8, 32767);
    put_s16(wire + 10, -32768);
    put_s16(wire + 12, 32767);
    put_s16(wire + 14, -1234);
    put_s16(wire + 16, 1234);
    put_s16(wire + 18, -1);
    put_s16(wire + 20, 2);
    put_s16(wire + 24, -3);
    put_s16(wire + 26, 4);
    wire[30] = 0x78; wire[31] = 0x56; wire[32] = 0x34; wire[33] = 0x12;
    put_s16(wire + 34, 16384); put_s16(wire + 36, -16384); put_s16(wire + 38, 2);
    put_s16(wire + 40, -100); put_s16(wire + 42, 200); put_s16(wire + 44, -300);

    CHECK(steam_report_decode(wire, sizeof(wire), &report) == 1);
    CHECK(report.kind == STEAM_REPORT_INPUT);
    CHECK(report.data.input.report_id == 0x42 && report.data.input.sequence == 0x7f);
    CHECK(report.data.input.wire_length == sizeof(wire));
    CHECK(PRESSED(report.data.input, STEAM_CTL_A));
    CHECK(PRESSED(report.data.input, STEAM_CTL_Y));
    CHECK(PRESSED(report.data.input, STEAM_CTL_R4));
    CHECK(PRESSED(report.data.input, STEAM_CTL_RB));
    CHECK(PRESSED(report.data.input, STEAM_CTL_DDOWN));
    CHECK(PRESSED(report.data.input, STEAM_CTL_STEAM));
    CHECK(PRESSED(report.data.input, STEAM_CTL_LB));
    CHECK(PRESSED(report.data.input, STEAM_CTL_RT));
    CHECK(PRESSED(report.data.input, STEAM_CTL_LPAD_CLICK));
    CHECK(PRESSED(report.data.input, STEAM_CTL_LT));
    CHECK(report.data.input.left_pad_touched && report.data.input.right_pad_touched);
    CHECK(report.data.input.right_stick_touched && report.data.input.left_grip_touched);
    CHECK(report.data.input.right_grip_touched);
    CHECK(report.data.input.left_trigger_raw == 16384);
    CHECK(report.data.input.right_trigger_raw == 32767);
    CHECK(report.data.input.left_x == -32768 && report.data.input.left_y == 32767);
    CHECK(report.data.input.right_x == -1234 && report.data.input.right_y == 1234);
    CHECK(report.data.input.left_pad_x == -1 && report.data.input.left_pad_y == 2);
    CHECK(report.data.input.right_pad_x == -3 && report.data.input.right_pad_y == 4);
    CHECK(report.data.input.has_imu == 1);
    CHECK(report.data.input.imu_timestamp == 0x12345678u);
    CHECK(report.data.input.accel[0] == 16384 && report.data.input.accel[1] == -16384);
    CHECK(report.data.input.gyro[0] == -100 && report.data.input.gyro[1] == 200 && report.data.input.gyro[2] == -300);
}

static void test_alternate_and_minimal_reports(void) {
    uint8_t wire[54] = {0};
    steam_report_t report;
    wire[0] = 0x45;
    CHECK(steam_report_decode(wire, 46, &report) == 1);
    CHECK(report.data.input.report_id == 0x45 && report.data.input.has_imu == 1);
    CHECK(steam_report_decode(wire, 30, &report) == 1);
    CHECK(report.data.input.has_imu == 0);
    wire[0] = 0x42;
    CHECK(steam_report_decode(wire, 45, &report) == 1);
    CHECK(report.data.input.has_imu == 0);
}

static void test_truncated_known_reports(void) {
    uint8_t wire[54] = {0};
    steam_report_t report;
    for (size_t length = 1; length < 30; length++) {
        wire[0] = 0x42;
        CHECK(steam_report_decode(wire, length, &report) == -1);
        CHECK(report.kind == STEAM_REPORT_INPUT);
        CHECK(report.data.input.controls == 0);
        wire[0] = 0x45;
        CHECK(steam_report_decode(wire, length, &report) == -1);
        CHECK(report.kind == STEAM_REPORT_INPUT);
    }
    wire[0] = 0x43;
    CHECK(steam_report_decode(wire, 1, &report) == -1);
    CHECK(steam_report_decode(wire, 2, &report) == -1);
    wire[0] = 0x79;
    CHECK(steam_report_decode(wire, 1, &report) == -1);
}

static void test_auxiliary_reports(void) {
    uint8_t wire[3] = {0x43, 2, 125};
    steam_report_t report;
    CHECK(steam_report_decode(wire, sizeof(wire), &report) == 1);
    CHECK(report.kind == STEAM_REPORT_BATTERY);
    CHECK(report.data.battery.state == 2 && report.data.battery.level == 100);
    wire[0] = 0x79; wire[1] = 1;
    CHECK(steam_report_decode(wire, 2, &report) == 1);
    CHECK(report.kind == STEAM_REPORT_WIRELESS && report.data.wireless.event == 1);
    wire[0] = 0xff;
    CHECK(steam_report_decode(wire, 3, &report) == 0);
    CHECK(steam_report_decode(NULL, 3, &report) == -1);
    CHECK(steam_report_decode(wire, 0, &report) == -1);
    CHECK(steam_report_decode(wire, 3, NULL) == -1);
}

int main(void) {
    test_full_input_report();
    test_alternate_and_minimal_reports();
    test_truncated_known_reports();
    test_auxiliary_reports();
    if (failures) {
        fprintf(stderr, "%d of %d Steam protocol checks failed\n", failures, checks);
        return 1;
    }
    printf("Steam protocol checks passed (%d checks)\n", checks);
    return 0;
}
