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

    printf("Steam binding transition checks passed (%d checks)\n", checks);
    return 0;
}
