/* SPDX-License-Identifier: GPL-3.0-or-later
 * diag_once.h — synchronized one-time diagnostic claims
 */
#pragma once

#include <pthread.h>

typedef struct {
    pthread_mutex_t lock;
    int claimed;
} gc_diag_once_t;

#define GC_DIAG_ONCE_INIT { PTHREAD_MUTEX_INITIALIZER, 0 }

static inline int gc_diag_once_claim(gc_diag_once_t *state) {
    if (!state) return 0;
    pthread_mutex_lock(&state->lock);
    int claim = !state->claimed;
    if (claim) state->claimed = 1;
    pthread_mutex_unlock(&state->lock);
    return claim;
}
