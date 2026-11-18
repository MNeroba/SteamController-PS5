#include "diag_once.h"

#include <stdatomic.h>
#include <stdio.h>

#define THREAD_COUNT 16
#define CLAIM_ATTEMPTS 5000

static gc_diag_once_t error_logged = GC_DIAG_ONCE_INIT;
static atomic_int log_claims;

static void *report_error(void *unused) {
    (void)unused;
    for (int i = 0; i < CLAIM_ATTEMPTS; i++) {
        if (gc_diag_once_claim(&error_logged))
        atomic_fetch_add_explicit(&log_claims, 1, memory_order_relaxed);
    }
    return NULL;
}

int main(void) {
    pthread_t threads[THREAD_COUNT];
    for (int i = 0; i < THREAD_COUNT; i++) {
        if (pthread_create(&threads[i], NULL, report_error, NULL) != 0) {
        fprintf(stderr, "failed to start diagnostic race reproducer\n");
        return 1;
        }
    }
    for (int i = 0; i < THREAD_COUNT; i++) pthread_join(threads[i], NULL);

    int claims = atomic_load_explicit(&log_claims, memory_order_relaxed);
    if (claims != 1) {
        fprintf(stderr, "diagnostic error was claimed %d times (wanted once)\n", claims);
        return 1;
    }
    puts("concurrent diagnostic reports are rate-limited once");
    return 0;
}
