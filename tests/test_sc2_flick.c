#include "sc2_flick.h"
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int checks;
static int start_workers;

typedef struct {
    sc2_profile_t profile;
    sc2_flick_state_t state;
    int flicking;
    int failed;
    int first_result;
} worker_t;

#define CHECK(expr) do { \
    checks++; \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        exit(1); \
    } \
} while (0)

static void *run_session(void *opaque) {
    worker_t *worker = opaque;
    sc2_flick_reset(&worker->state);
    while (!__atomic_load_n(&start_workers, __ATOMIC_ACQUIRE)) sched_yield();
    for (int i = 0; i < 10000; ++i) {
        int output = sc2_flick_apply(&worker->state, &worker->profile,
            worker->flicking ? 32767 : 0, 0, 1000 + (int64_t)i * 4);
        if (i == 0) worker->first_result = output;
        if (!worker->flicking && output != 0) worker->failed = 1;
    }
    return NULL;
}

int main(void) {
    sc2_profile_t profile;
    sc2_flick_state_t first_session, second_session;
    memset(&profile, 0, sizeof(profile));
    profile.flick_speed = 360;

    sc2_flick_reset(&first_session);
    sc2_flick_reset(&second_session);
    CHECK(sc2_flick_apply(&first_session, &profile, 32767, 0, 1000) == 127);
    /* A second Steam session at rest must not inherit the first session's
     * queued turn, edge state, or timing history. */
    CHECK(sc2_flick_apply(&second_session, &profile, 0, 0, 1000) == 0);
    CHECK(sc2_flick_apply(&first_session, &profile, 32767, 0, 1004) == 127);
    CHECK(sc2_flick_apply(&second_session, &profile, 0, 0, 1004) == 0);
    sc2_flick_reset(&first_session);
    CHECK(sc2_flick_apply(&first_session, &profile, 0, 32767, 1008) == 0);
    CHECK(sc2_flick_apply(NULL, &profile, 32767, 0, 1012) == 0);
    CHECK(sc2_flick_apply(&second_session, NULL, 32767, 0, 1012) == 0);

    worker_t workers[2];
    memset(workers, 0, sizeof(workers));
    for (int i = 0; i < 2; ++i) {
        workers[i].profile.flick_speed = 360;
        workers[i].flicking = i == 0;
    }
    pthread_t threads[2];
    CHECK(pthread_create(&threads[0], NULL, run_session, &workers[0]) == 0);
    CHECK(pthread_create(&threads[1], NULL, run_session, &workers[1]) == 0);
    __atomic_store_n(&start_workers, 1, __ATOMIC_RELEASE);
    CHECK(pthread_join(threads[0], NULL) == 0);
    CHECK(pthread_join(threads[1], NULL) == 0);
    CHECK(workers[0].first_result == 127);
    CHECK(workers[1].failed == 0);

    printf("sc2 flick: %d checks passed, 20,000 concurrent session reports\n", checks);
    return 0;
}
