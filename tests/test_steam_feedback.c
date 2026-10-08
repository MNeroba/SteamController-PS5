#include "steam_feedback.h"

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

static void test_pulse_reports(void) {
    uint8_t report[STEAM_HAPTIC_PULSE_SIZE];
    memset(report, 0xaa, sizeof(report));
    CHECK(steam_feedback_encode_pulse(report, sizeof(report), STEAM_HAPTIC_LEFT,
                                      0x1234, 0xabcd, 0x0102) == 8);
    CHECK(report[0] == 0x81 && report[1] == 1); /* wire order swaps pads */
    CHECK(report[2] == 0x34 && report[3] == 0x12);
    CHECK(report[4] == 0xcd && report[5] == 0xab);
    CHECK(report[6] == 0x02 && report[7] == 0x01);
    CHECK(steam_feedback_encode_pulse(report, sizeof(report), STEAM_HAPTIC_RIGHT,
                                      1, 2, 3) == 8);
    CHECK(report[1] == 0);
    CHECK(steam_feedback_encode_pulse(report, sizeof(report), STEAM_HAPTIC_BOTH,
                                      1, 2, 3) == 8);
    CHECK(report[1] == 2);
    CHECK(steam_feedback_encode_pulse(report, sizeof(report) - 1, 0, 1, 2, 3) == -1);
    CHECK(steam_feedback_encode_pulse(report, sizeof(report), -1, 1, 2, 3) == -1);
    CHECK(steam_feedback_encode_pulse(report, sizeof(report), 3, 1, 2, 3) == -1);
    CHECK(steam_feedback_encode_pulse(NULL, sizeof(report), 0, 1, 2, 3) == -1);
}

static void test_rumble_reports(void) {
    uint8_t report[STEAM_HAPTIC_RUMBLE_SIZE];
    memset(report, 0xaa, sizeof(report));
    CHECK(steam_feedback_encode_rumble(report, sizeof(report), 0x1234, 0xabcd) == 10);
    CHECK(report[0] == 0x80 && report[1] == 0);
    CHECK(report[2] == 0 && report[3] == 0);
    CHECK(report[4] == 0x34 && report[5] == 0x12 && report[6] == 2);
    CHECK(report[7] == 0xcd && report[8] == 0xab && report[9] == 0);
    CHECK(steam_feedback_encode_rumble(report, sizeof(report) - 1, 1, 2) == -1);
    CHECK(steam_feedback_encode_rumble(NULL, sizeof(report), 1, 2) == -1);
}

int main(void) {
    test_pulse_reports();
    test_rumble_reports();
    if (failures) {
        fprintf(stderr, "%d of %d Steam feedback checks failed\n", failures, checks);
        return 1;
    }
    printf("Steam feedback checks passed (%d checks)\n", checks);
    return 0;
}
