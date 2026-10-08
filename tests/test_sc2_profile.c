#include "sc2_profile.h"
#include "game_list.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int checks;

#define CHECK(condition) do { \
    checks++; \
    if (!(condition)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        exit(1); \
    } \
} while (0)

/* The parser test links the complete profile source file; these target-only
 * game lookup functions are unused by this host test. */
int gc_game_name(const char *id, char *out, size_t outsz) {
    (void)id;
    if (outsz) out[0] = '\0';
    return 0;
}

int sceLncUtilGetAppIdOfRunningBigApp(void) { return 0; }
int sceLncUtilGetAppTitleId(uint32_t app_id, char *title_id) {
    (void)app_id;
    (void)title_id;
    return -1;
}

int main(void) {
    sc2_profile_t p;
    char digits[181];
    char bad[1024];

    sc2_profile_default(&p);
    CHECK(p.long_ms == 400);
    CHECK(p.deadzone == 9);
    CHECK(p.gyro_sens_x == 20);

    sc2_profile_parse(
        "LONG_MS=9999\nDOUBLE_MS=-1\nTURBO_MS=1\nDEADZONE=45\n"
        "GYRO_SENS_X=0\nGYRO_CURVE=999\nTRIGGER_FX=-4\nFLICK_SPEED=1\n"
        "A.turbo=0\nA.toggle=1\nINVERT_LY=2\nMOTION=0\n", &p);
    CHECK(p.long_ms == 2000);
    CHECK(p.double_ms == 100);
    CHECK(p.turbo_ms == 20);
    CHECK(p.deadzone == 40);
    CHECK(p.gyro_sens_x == 1);
    CHECK(p.gyro_curve == 30);
    CHECK(p.trig_fx == 0);
    CHECK(p.flick_speed == 90);
    CHECK(p.turbo[IN_A] == 0);
    CHECK(p.toggle[IN_A] == 1);
    CHECK(p.invert_ly == 1);
    CHECK(p.motion == 0);

    memset(digits, '9', sizeof(digits) - 1);
    digits[sizeof(digits) - 1] = '\0';
    p.turbo[IN_A] = 1;
    p.toggle[IN_A] = 0;
    snprintf(bad, sizeof(bad),
        "LONG_MS=%s\nDOUBLE_MS=200ms\nTURBO_MS=\nDEADZONE=%s\n"
        "A.turbo=not-a-number\nA.toggle=%s\nINVERT_LY=1x\n"
        "GYRO_SENS_X=2147483648\nFLICK_SPEED=-%s\n",
        digits, digits, digits, digits);
    sc2_profile_parse(bad, &p);
    CHECK(p.long_ms == 2000);
    CHECK(p.double_ms == 100);
    CHECK(p.turbo_ms == 20);
    CHECK(p.deadzone == 40);
    CHECK(p.turbo[IN_A] == 1);
    CHECK(p.toggle[IN_A] == 0);
    CHECK(p.invert_ly == 1);
    CHECK(p.gyro_sens_x == 1);
    CHECK(p.flick_speed == 90);

    sc2_profile_parse("LONG_MS=2147483648\nTURBO_MS=-2147483648\n", &p);
    CHECK(p.long_ms == 2000);  /* outside int range is ignored */
    CHECK(p.turbo_ms == 20);   /* in-range signed values are clamped */

    printf("Steam profile parser checks passed (%d checks)\n", checks);
    return 0;
}
