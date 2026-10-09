/* SPDX-License-Identifier: GPL-3.0-or-later */
#define _POSIX_C_SOURCE 200809L
#include "runtime_control.h"

#include <errno.h>
#include <stdatomic.h>
#include <time.h>

static atomic_int g_runtime_stopping;

void runtime_control_reset(void) {
    atomic_store_explicit(&g_runtime_stopping, 0, memory_order_release);
}

void runtime_request_stop(void) {
    atomic_store_explicit(&g_runtime_stopping, 1, memory_order_release);
}

int runtime_is_stopping(void) {
    return atomic_load_explicit(&g_runtime_stopping, memory_order_acquire) != 0;
}

void runtime_sleep_ms(uint32_t milliseconds) {
    while (milliseconds != 0 && !runtime_is_stopping()) {
        uint32_t step = milliseconds > 20u ? 20u : milliseconds;
        struct timespec request = {
            (time_t)(step / 1000u),
            (long)(step % 1000u) * 1000000L
        };
        int result;
        do {
            result = nanosleep(&request, &request);
        } while (result != 0 && errno == EINTR && !runtime_is_stopping());
        if (result != 0 && errno != EINTR) return;
        milliseconds -= step;
    }
}
