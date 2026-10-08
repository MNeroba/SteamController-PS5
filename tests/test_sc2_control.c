#include "sc2_control.h"

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static int checks;
static int invalid_state;

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
    printf("Steam control-state atomic checks passed (%d checks)\n", checks);
    return 0;
}
