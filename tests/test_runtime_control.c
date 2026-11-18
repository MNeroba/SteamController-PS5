/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "runtime_control.h"

#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>

#define WORKERS 12

static atomic_int started;
static atomic_int stopped;

static void *waiter(void *opaque) {
    (void)opaque;
    atomic_fetch_add_explicit(&started, 1, memory_order_relaxed);
    while (!runtime_is_stopping()) runtime_sleep_ms(1);
    atomic_fetch_add_explicit(&stopped, 1, memory_order_relaxed);
    return NULL;
}

int main(void) {
    pthread_t workers[WORKERS];
    runtime_control_reset();
    if (runtime_is_stopping()) return 1;

    for (int i = 0; i < WORKERS; i++)
        if (pthread_create(&workers[i], NULL, waiter, NULL) != 0) return 1;
    while (atomic_load_explicit(&started, memory_order_relaxed) != WORKERS)
        runtime_sleep_ms(1);

    runtime_request_stop();
    runtime_request_stop();
    for (int i = 0; i < WORKERS; i++)
        if (pthread_join(workers[i], NULL) != 0) return 1;
    if (atomic_load_explicit(&stopped, memory_order_relaxed) != WORKERS ||
        !runtime_is_stopping()) {
        fprintf(stderr, "runtime stop signal was not visible to every worker\n");
        return 1;
    }

    runtime_control_reset();
    if (runtime_is_stopping()) return 1;
    puts("runtime stop signal checks passed (12 workers)");
    return 0;
}
