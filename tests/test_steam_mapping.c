#include "steam_mapping.h"

#include <stdio.h>
#include <limits.h>

static int failures;
static int checks;

#define CHECK(cond) do { \
    checks++; \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        failures++; \
    } \
} while (0)

static void test_axes_and_triggers(void) {
    CHECK(steam_axis_to_u8(INT16_MIN, 100, 0) == 0);
    CHECK(steam_axis_to_u8(INT16_MAX, 100, 0) == 255);
    CHECK(steam_axis_to_u8(INT16_MIN, 100, 1) == 255);
    CHECK(steam_axis_to_u8(INT16_MAX, 100, 1) == 0);
    CHECK(steam_axis_to_u8(0, 100, 0) == 128);
    CHECK(steam_axis_to_u8(-99, 100, 0) == 128);
    CHECK(steam_axis_to_u8(-100, 100, 0) == 127);
    CHECK(steam_trigger_to_u8(INT16_MIN) == 0);
    CHECK(steam_trigger_to_u8(0) == 0);
    CHECK(steam_trigger_to_u8(127) == 0);
    CHECK(steam_trigger_to_u8(128) == 1);
    CHECK(steam_trigger_to_u8(INT16_MAX) == 255);
}

static void test_touch_coordinates(void) {
    steam_touch_position_t p;
    steam_pad_touch_map(INT16_MIN, INT16_MAX, 0, 7, &p);
    CHECK(p.x == 0 && p.y == 0 && p.finger == 7);
    steam_pad_touch_map(INT16_MAX, INT16_MIN, 0, 0, &p);
    CHECK(p.x == 959 && p.y == 1079 && p.finger == 0);
    steam_pad_touch_map(INT16_MIN, INT16_MAX, 1, 1, &p);
    CHECK(p.x == 960 && p.y == 0);
    steam_pad_touch_map(INT16_MAX, INT16_MIN, 1, 1, &p);
    CHECK(p.x == 1919 && p.y == 1079);
    steam_pad_touch_map(0, 0, 0, 0, NULL); /* null output is ignored */
}

static void test_dpad_thresholds(void) {
    CHECK(steam_pad_to_dpad(0, 0) == 0);
    CHECK(steam_pad_to_dpad(0, 13000) == 0);
    CHECK(steam_pad_to_dpad(0, 13001) == STEAM_DPAD_UP);
    CHECK(steam_pad_to_dpad(0, -13000) == 0);
    CHECK(steam_pad_to_dpad(0, -13001) == STEAM_DPAD_DOWN);
    CHECK(steam_pad_to_dpad(-13001, 0) == STEAM_DPAD_LEFT);
    CHECK(steam_pad_to_dpad(13001, 0) == STEAM_DPAD_RIGHT);
    CHECK(steam_pad_to_dpad(-32768, 32767) ==
          (STEAM_DPAD_UP | STEAM_DPAD_LEFT));
}

int main(void) {
    test_axes_and_triggers();
    test_touch_coordinates();
    test_dpad_thresholds();
    if (failures) {
        fprintf(stderr, "%d of %d Steam mapping checks failed\n", failures, checks);
        return 1;
    }
    printf("Steam mapping checks passed (%d checks)\n", checks);
    return 0;
}
