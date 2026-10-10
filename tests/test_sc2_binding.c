#include "sc2_binding.h"
#include "gc_types.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int checks;

static void *invalidate_worker(void *unused) {
    (void)unused;
    for (int i = 0; i < 10000; i++) sc2_bind_invalidate_sessions();
    return NULL;
}

#define CHECK(condition) do { \
    checks++; \
    if (!(condition)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        exit(1); \
    } \
} while (0)

static void pressure(sc2_bind_state_t *state, const sc2_profile_t *p, uint32_t in,
                     int full_l, int full_r, int64_t now, uint8_t lt, uint8_t rt,
                     uint8_t expected_l2, uint8_t expected_r2) {
    (void)sc2_bind_eval(state, p, 1, in, full_l, full_r, now);
    uint8_t l2, r2;
    sc2_bind_trigger_pressure(state, lt, rt, &l2, &r2);
    CHECK(l2 == expected_l2); CHECK(r2 == expected_r2);
}

static void trigger_regressions(void) {
    sc2_profile_t p = {0}; sc2_bind_state_t state = {0};
    p.shift_in = -1; p.long_ms = 400; p.double_ms = 250; p.turbo_ms = 60;
    p.map[IN_LT] = SCE_PAD_BUTTON_L2; p.map[IN_RT] = SCE_PAD_BUTTON_R2;
    p.map[IN_A] = SCE_PAD_BUTTON_CROSS; p.lng[IN_A] = SCE_PAD_BUTTON_L2;
    pressure(&state, &p, 0, 0, 0, 0, 100, 70, 0, 0); /* initial neutral */
    pressure(&state, &p, 0, 0, 0, 1, 8, 0, 8, 0); /* below digital threshold */
    pressure(&state, &p, 1u << IN_LT, 0, 0, 10, 100, 0, 100, 0);
    pressure(&state, &p, (1u << IN_LT) | (1u << IN_A), 0, 0, 20, 100, 0, 100, 0);
    pressure(&state, &p, (1u << IN_LT) | (1u << IN_A), 0, 0, 419, 100, 0, 100, 0);
    pressure(&state, &p, (1u << IN_LT) | (1u << IN_A), 0, 0, 420, 100, 0, 255, 0);
    pressure(&state, &p, 1u << IN_LT, 0, 0, 430, 100, 0, 100, 0);
    p.lng[IN_A] = 0; p.dbl[IN_A] = SCE_PAD_BUTTON_L2;
    pressure(&state, &p, 0, 0, 0, 500, 0, 0, 0, 0);
    pressure(&state, &p, (1u << IN_LT) | (1u << IN_A), 0, 0, 510, 90, 0, 90, 0);
    pressure(&state, &p, 1u << IN_LT, 0, 0, 520, 90, 0, 90, 0);
    pressure(&state, &p, (1u << IN_LT) | (1u << IN_A), 0, 0, 530, 90, 0, 255, 0);
    pressure(&state, &p, 1u << IN_LT, 0, 0, 540, 90, 0, 90, 0);
    p.shift_in = IN_L4; p.shf[IN_LT] = SCE_PAD_BUTTON_R2; p.dbl[IN_A] = 0;
    pressure(&state, &p, 0, 0, 0, 600, 0, 0, 0, 0);
    pressure(&state, &p, (1u << IN_LT) | (1u << IN_L4), 0, 0, 610, 120, 30, 0, 120);
    pressure(&state, &p, 1u << IN_LT, 0, 0, 620, 120, 30, 120, 30);
    p.map[IN_LT] = SCE_PAD_BUTTON_R2 | SCE_PAD_BUTTON_CROSS;
    p.map[IN_RT] = SCE_PAD_BUTTON_L2;
    pressure(&state, &p, 0, 0, 0, 700, 0, 0, 0, 0);
    pressure(&state, &p, (1u << IN_LT) | (1u << IN_RT), 0, 0, 710, 110, 75, 75, 110);
    p.shift_in = IN_LT; p.full[0] = SCE_PAD_BUTTON_L2;
    pressure(&state, &p, 0, 0, 0, 800, 0, 0, 0, 0);
    pressure(&state, &p, 1u << IN_LT, 1, 0, 810, 255, 0, 0, 0);
    p.shift_in = -1; p.map[IN_A] = SCE_PAD_BUTTON_L2; p.turbo[IN_A] = 1;
    pressure(&state, &p, 0, 0, 0, 900, 0, 0, 0, 0);
    pressure(&state, &p, 1u << IN_A, 0, 0, 910, 0, 0, 255, 0);
    pressure(&state, &p, 1u << IN_A, 0, 0, 970, 0, 0, 0, 0);
    p.turbo[IN_A] = 0; p.toggle[IN_A] = 1;
    pressure(&state, &p, 0, 0, 0, 1000, 0, 0, 0, 0);
    pressure(&state, &p, 1u << IN_A, 0, 0, 1010, 0, 0, 255, 0);
    pressure(&state, &p, 0, 0, 0, 1020, 0, 0, 255, 0);
    sc2_bind_reset(&state);
    pressure(&state, &p, 0, 0, 0, 1030, 0, 0, 0, 0);
    p.toggle[IN_A] = 0;
    pressure(&state, &p, 0, 0, 0, 1040, 0, 0, 0, 0);
    pressure(&state, &p, 1u << IN_LT, 1, 0, 1050, 200, 0, 255, 200);
    p.lng[IN_A] = SCE_PAD_BUTTON_R2; p.toggle[IN_A] = 1;
    pressure(&state, &p, 0, 0, 0, 1100, 0, 0, 0, 0);
    pressure(&state, &p, 1u << IN_A, 0, 0, 1110, 0, 0, 0, 0);
    pressure(&state, &p, 0, 0, 0, 1120, 0, 0, 255, 0); /* quick regular tap */
    pressure(&state, &p, 0, 0, 0, 1210, 0, 0, 0, 0);
    pressure(&state, &p, 1u << IN_A, 0, 0, 1300, 0, 0, 0, 0);
    pressure(&state, &p, 1u << IN_A, 0, 0, 1700, 0, 0, 0, 255);
    pressure(&state, &p, 0, 0, 0, 1710, 0, 0, 0, 255);
    pressure(&state, &p, 1u << IN_A, 0, 0, 1800, 0, 0, 0, 255);
    pressure(&state, &p, 1u << IN_A, 0, 0, 2200, 0, 0, 0, 0);
    pressure(&state, &p, 0, 0, 0, 2210, 0, 0, 0, 0);
    p.toggle[IN_A] = 0; p.lng[IN_A] = 0; p.map[IN_A] = SCE_PAD_BUTTON_CROSS;
    p.shf[IN_A] = SCE_PAD_BUTTON_L2; p.shift_in = IN_L4;
    p.map[IN_LT] = SCE_PAD_BUTTON_L2; p.map[IN_RT] = SCE_PAD_BUTTON_L2;
    p.full[0] = 0;
    pressure(&state, &p, 0, 0, 0, 2300, 0, 0, 0, 0);
    pressure(&state, &p, (1u << IN_A) | (1u << IN_LT) | (1u << IN_RT), 0, 0, 2310, 80, 110, 110, 0);
    pressure(&state, &p, (1u << IN_A) | (1u << IN_L4) | (1u << IN_LT), 0, 0, 2320, 80, 0, 255, 80);
    pressure(&state, &p, (1u << IN_A) | (1u << IN_LT), 0, 0, 2330, 80, 0, 80, 0);
    p.shift_in = -1; p.dbl[IN_LT] = SCE_PAD_BUTTON_R2;
    pressure(&state, &p, 0, 0, 0, 2400, 0, 0, 0, 0);
    pressure(&state, &p, 1u << IN_LT, 0, 0, 2410, 90, 0, 90, 0);
    pressure(&state, &p, 0, 0, 0, 2420, 0, 0, 0, 0);
    pressure(&state, &p, 1u << IN_LT, 0, 0, 2430, 90, 0, 0, 255);
    pressure(&state, &p, 0, 0, 0, 2440, 0, 0, 0, 0);
    p.dbl[IN_LT] = 0; p.lng[IN_LT] = SCE_PAD_BUTTON_R2;
    pressure(&state, &p, 0, 0, 0, 2500, 0, 0, 0, 0);
    pressure(&state, &p, 1u << IN_LT, 0, 0, 2510, 90, 0, 0, 0);
    pressure(&state, &p, 1u << IN_LT, 0, 0, 2910, 90, 0, 0, 255);
    pressure(&state, &p, 0, 0, 0, 2920, 0, 0, 0, 0);
}

int main(void) {
    sc2_profile_t toggle_profile;
    sc2_profile_t other_profile;
    sc2_profile_t long_profile;
    sc2_bind_state_t state = {0};
    sc2_bind_state_t first_session = {0};
    sc2_bind_state_t second_session = {0};
    sc2_bind_state_t generation_state = {0};
    sc2_bind_state_t invalidate_state = {0};
    sc2_bind_state_t concurrent_state = {0};
    sc2_bind_state_t startup_held = {0};
    pthread_t invalidator;
    memset(&toggle_profile, 0, sizeof(toggle_profile));
    toggle_profile.shift_in = -1;
    toggle_profile.long_ms = 400;
    toggle_profile.double_ms = 250;
    toggle_profile.turbo_ms = 60;
    toggle_profile.map[IN_A] = SCE_PAD_BUTTON_CROSS;
    toggle_profile.toggle[IN_A] = 1;

    other_profile = toggle_profile;
    other_profile.map[IN_A] = SCE_PAD_BUTTON_CIRCLE;
    other_profile.toggle[IN_A] = 0;

    sc2_bind_reset(&state);
    CHECK(sc2_bind_eval(&state, &toggle_profile, 1, 0, 0, 0, 0) == 0);
    CHECK(state.neutral_report);
    CHECK(sc2_bind_eval(&state, &toggle_profile, 1, (uint32_t)1u << IN_A, 0, 0, 0) ==
          SCE_PAD_BUTTON_CROSS);
    CHECK(!state.neutral_report);
    CHECK(sc2_bind_eval(&state, &toggle_profile, 1, 0, 0, 0, 10) == SCE_PAD_BUTTON_CROSS);
    CHECK(sc2_bind_eval(&state, &other_profile, 2, 0, 0, 0, 20) == 0);
    CHECK(state.neutral_report);
    /* Returning to the old profile must not revive its stale latched toggle. */
    CHECK(sc2_bind_eval(&state, &toggle_profile, 3, 0, 0, 0, 30) == 0);
    CHECK(state.neutral_report);

    /* A held key on first contact is seeded, not mistaken for a new toggle. */
    CHECK(sc2_bind_eval(&startup_held, &toggle_profile, 1,
                        (uint32_t)1u << IN_A, 0, 0, 35) == 0);
    CHECK(startup_held.neutral_report);
    CHECK(sc2_bind_eval(&startup_held, &toggle_profile, 1,
                        (uint32_t)1u << IN_A, 0, 0, 36) == 0);
    CHECK(!startup_held.neutral_report);
    CHECK(sc2_bind_eval(&startup_held, &toggle_profile, 1, 0, 0, 0, 37) == 0);
    CHECK(sc2_bind_eval(&startup_held, &toggle_profile, 1,
                        (uint32_t)1u << IN_A, 0, 0, 38) == SCE_PAD_BUTTON_CROSS);

    /* A generation change resets state even when the profile bytes match. */
    CHECK(sc2_bind_eval(&generation_state, &toggle_profile, 10, 0, 0, 0, 39) == 0);
    CHECK(sc2_bind_eval(&generation_state, &toggle_profile, 10,
                        (uint32_t)1u << IN_A, 0, 0, 40) == SCE_PAD_BUTTON_CROSS);
    CHECK(sc2_bind_eval(&generation_state, &toggle_profile, 10, 0, 0, 0, 50) ==
          SCE_PAD_BUTTON_CROSS);
    CHECK(sc2_bind_eval(&generation_state, &toggle_profile, 11, 0, 0, 0, 60) == 0);
    CHECK(generation_state.neutral_report);

    /* Each session keeps its own toggle state. */
    CHECK(sc2_bind_eval(&first_session, &toggle_profile, 1, 0, 0, 0, 90) == 0);
    CHECK(sc2_bind_eval(&second_session, &toggle_profile, 1, 0, 0, 0, 90) == 0);
    CHECK(sc2_bind_eval(&first_session, &toggle_profile, 1,
                        (uint32_t)1u << IN_A, 0, 0, 100) == SCE_PAD_BUTTON_CROSS);
    CHECK(sc2_bind_eval(&second_session, &toggle_profile, 1, 0, 0, 0, 100) == 0);
    CHECK(sc2_bind_eval(&first_session, &toggle_profile, 1, 0, 0, 0, 110) ==
          SCE_PAD_BUTTON_CROSS);
    CHECK(sc2_bind_eval(&second_session, &toggle_profile, 1,
                        (uint32_t)1u << IN_A, 0, 0, 120) == SCE_PAD_BUTTON_CROSS);
    CHECK(sc2_bind_eval(&first_session, &toggle_profile, 1, 0, 0, 0, 130) ==
          SCE_PAD_BUTTON_CROSS);

    /* Long-press state is also tied to the session and profile. */
    long_profile = other_profile;
    long_profile.lng[IN_A] = SCE_PAD_BUTTON_SQUARE;
    long_profile.long_ms = 400;
    sc2_bind_reset(&state);
    CHECK(sc2_bind_eval(&state, &long_profile, 5, 0, 0, 0, 999) == 0);
    CHECK(state.neutral_report);
    CHECK(sc2_bind_eval(&state, &long_profile, 5,
                        (uint32_t)1u << IN_A, 0, 0, 1000) == 0);
    CHECK(sc2_bind_eval(&state, &long_profile, 5,
                        (uint32_t)1u << IN_A, 0, 0, 1399) == 0);
    CHECK(sc2_bind_eval(&state, &long_profile, 5,
                        (uint32_t)1u << IN_A, 0, 0, 1400) ==
          SCE_PAD_BUTTON_SQUARE);
    CHECK(sc2_bind_eval(&state, &long_profile, 5, 0, 0, 0, 1410) == 0);

    /* A portal pause/resume requests reset without touching another thread's
     * per-slot memory; the owning input thread applies it on its next report. */
    CHECK(sc2_bind_eval(&invalidate_state, &toggle_profile, 20, 0, 0, 0, 1499) == 0);
    CHECK(sc2_bind_eval(&invalidate_state, &toggle_profile, 20,
                        (uint32_t)1u << IN_A, 0, 0, 1500) == SCE_PAD_BUTTON_CROSS);
    CHECK(sc2_bind_eval(&invalidate_state, &toggle_profile, 20, 0, 0, 0, 1510) ==
          SCE_PAD_BUTTON_CROSS);
    sc2_bind_invalidate_sessions();
    CHECK(sc2_bind_eval(&invalidate_state, &toggle_profile, 20, 0, 0, 0, 1520) == 0);
    CHECK(invalidate_state.neutral_report);
    CHECK(sc2_bind_eval(&invalidate_state, &toggle_profile, 20, 0, 0, 0, 1530) == 0);
    CHECK(!invalidate_state.neutral_report);

    /* The portal-side invalidator can run concurrently while only the owner
     * thread reads or mutates its per-session context. */
    CHECK(sc2_bind_eval(&concurrent_state, &toggle_profile, 30, 0, 0, 0, 1600) == 0);
    CHECK(sc2_bind_eval(&concurrent_state, &toggle_profile, 30,
                        (uint32_t)1u << IN_A, 0, 0, 1601) == SCE_PAD_BUTTON_CROSS);
    CHECK(pthread_create(&invalidator, NULL, invalidate_worker, NULL) == 0);
    for (int i = 0; i < 10000; i++)
        (void)sc2_bind_eval(&concurrent_state, &toggle_profile, 30,
                            (uint32_t)1u << IN_A, 0, 0, 1700 + i);
    CHECK(pthread_join(invalidator, NULL) == 0);
    sc2_bind_invalidate_sessions();
    CHECK(sc2_bind_eval(&concurrent_state, &toggle_profile, 30,
                        (uint32_t)1u << IN_A, 0, 0, 12000) == 0);
    CHECK(concurrent_state.neutral_report);
    CHECK(sc2_bind_eval(&concurrent_state, &toggle_profile, 30,
                        (uint32_t)1u << IN_A, 0, 0, 12001) == 0);
    CHECK(sc2_bind_eval(&concurrent_state, &toggle_profile, 30, 0, 0, 0, 12002) == 0);
    CHECK(sc2_bind_eval(&concurrent_state, &toggle_profile, 30,
                        (uint32_t)1u << IN_A, 0, 0, 12003) == SCE_PAD_BUTTON_CROSS);

    /* A Shift trigger is a modifier at every stage of its travel. The other
     * trigger's full pull remains independently usable. */
    for (int shift = IN_LT; shift <= IN_RT; shift++) {
        sc2_bind_state_t trigger_state = {0};
        sc2_profile_t trigger_profile = other_profile;
        trigger_profile.shift_in = (int8_t)shift;
        trigger_profile.map[shift] = SCE_PAD_BUTTON_L2;
        trigger_profile.full[0] = SCE_PAD_BUTTON_SQUARE;
        trigger_profile.full[1] = SCE_PAD_BUTTON_CIRCLE;
        trigger_profile.shf[IN_A] = SCE_PAD_BUTTON_TRIANGLE;
        CHECK(sc2_bind_eval(&trigger_state, &trigger_profile, 40, 0, 0, 0, 13000) == 0);
        CHECK(sc2_bind_eval(&trigger_state, &trigger_profile, 40,
                            (1u << shift), shift == IN_LT, shift == IN_RT, 13001) == 0);
        CHECK(sc2_bind_eval(&trigger_state, &trigger_profile, 40,
                            (1u << shift) | (1u << IN_A), 1, 1, 13002) ==
              (SCE_PAD_BUTTON_TRIANGLE | trigger_profile.full[shift == IN_LT]));
    }

    /* Long + Turbo repeats the long destination after its threshold. */
    sc2_bind_reset(&state);
    long_profile.turbo[IN_A] = 1;
    CHECK(sc2_bind_eval(&state, &long_profile, 99, 0, 0, 0, 20000) == 0);
    CHECK(sc2_bind_eval(&state, &long_profile, 99, 1u << IN_A, 0, 0, 20001) == 0);
    CHECK(sc2_bind_eval(&state, &long_profile, 99, 1u << IN_A, 0, 0, 20401) == SCE_PAD_BUTTON_SQUARE);
    CHECK(sc2_bind_eval(&state, &long_profile, 99, 1u << IN_A, 0, 0, 20461) == 0);
    CHECK(sc2_bind_eval(&state, &long_profile, 99, 0, 0, 0, 20470) == 0);
    CHECK(sc2_bind_eval(&state, &long_profile, 99, 1u << IN_A, 0, 0, 20500) == 0);
    CHECK(sc2_bind_eval(&state, &long_profile, 99, 0, 0, 0, 20520) == SCE_PAD_BUTTON_CIRCLE);
    CHECK(sc2_bind_eval(&state, &long_profile, 99, 0, 0, 0, 20610) == 0);
    trigger_regressions();
    printf("Steam binding transition checks passed (%d checks)\n", checks);
    return 0;
}
