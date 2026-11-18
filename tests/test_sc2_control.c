#include "sc2_control.h"

#include <pthread.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static int checks;
static int invalid_state;
static int assignment_active;
static int assignment_failures;
static int assignment_claims;

#define CHECK(condition) do { \
    checks++; \
    if (!(condition)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        exit(1); \
    } \
} while (0)

static void *toggle_worker(void *arg) {
    intptr_t worker = (intptr_t)arg;
    for (int i = 0; i < 10000; i++) {
        int old = sc2_pause_exchange((i + (int)worker) & 1);
        if (old != 0 && old != 1) return &invalid_state;
    }
    return NULL;
}

static void *assignment_worker(void *arg) {
    int slot = (int)(intptr_t)arg;
    for (int i = 0; i < 4000; i++) {
        sc2_assignment_token_t token;
        while ((token = sc2_assignment_claim(slot)) == 0) sched_yield();

        int active = __atomic_add_fetch(&assignment_active, 1, __ATOMIC_RELAXED);
        if (active != 1)
            __atomic_store_n(&assignment_failures, 1, __ATOMIC_RELAXED);
        __atomic_add_fetch(&assignment_claims, 1, __ATOMIC_RELAXED);
        if ((i & 7) == 0) sched_yield();
        __atomic_sub_fetch(&assignment_active, 1, __ATOMIC_RELAXED);
        if (!sc2_assignment_release(token)) {
            __atomic_store_n(&assignment_failures, 1, __ATOMIC_RELAXED);
            return &invalid_state;
        }
    }
    return NULL;
}

int main(void) {
    pthread_t threads[4];
    void *result;

    CHECK(sc2_pause_exchange(0) == 0);
    CHECK(sc2_pause_is_active() == 0);
    CHECK(sc2_pause_exchange(-7) == 0);
    CHECK(sc2_pause_is_active() == 1);
    CHECK(sc2_pause_exchange(99) == 1);

    for (intptr_t i = 0; i < 4; i++)
        CHECK(pthread_create(&threads[i], NULL, toggle_worker, (void *)i) == 0);
    for (int i = 0; i < 4; i++) {
        result = NULL;
        CHECK(pthread_join(threads[i], &result) == 0);
        CHECK(result == NULL);
    }

    (void)sc2_pause_exchange(1);
    CHECK(sc2_pause_is_active() == 1);
    CHECK(sc2_pause_exchange(0) == 1);
    CHECK(sc2_pause_is_active() == 0);

    CHECK(sc2_assignment_claim(-1) == 0);
    CHECK(sc2_assignment_claim(32) == 0);
    sc2_assignment_token_t first = sc2_assignment_claim(0);
    CHECK(first != 0);
    CHECK(sc2_assignment_slot(first) == 0);
    CHECK(sc2_assignment_owner() == first);
    CHECK(sc2_assignment_claim(1) == 0);
    CHECK(sc2_assignment_release(first) == 1);

    sc2_assignment_token_t replacement = sc2_assignment_claim(0);
    CHECK(replacement != 0);
    CHECK(replacement != first);
    CHECK(sc2_assignment_release(first) == 0);
    CHECK(sc2_assignment_owner() == replacement);
    CHECK(sc2_assignment_release(replacement) == 1);
    CHECK(sc2_assignment_release(replacement) == 0);
    CHECK(sc2_assignment_slot(0) == -1);
    CHECK(sc2_assignment_slot(UINT64_C(0x100000000)) == -1);

    pthread_t assign_threads[4];
    for (intptr_t i = 0; i < 4; i++)
        CHECK(pthread_create(&assign_threads[i], NULL, assignment_worker,
                             (void *)i) == 0);
    for (int i = 0; i < 4; i++) {
        result = NULL;
        CHECK(pthread_join(assign_threads[i], &result) == 0);
        CHECK(result == NULL);
    }
    CHECK(__atomic_load_n(&assignment_failures, __ATOMIC_RELAXED) == 0);
    CHECK(__atomic_load_n(&assignment_active, __ATOMIC_RELAXED) == 0);
    CHECK(__atomic_load_n(&assignment_claims, __ATOMIC_RELAXED) == 16000);
    CHECK(sc2_assignment_owner() == 0);
    printf("Steam control-state atomic checks passed (%d checks)\n", checks);
    return 0;
}
