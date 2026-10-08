#include "sc2_trigfx.h"
#include "sc2_haptics.h"
#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static _Atomic int pulse_count;
static _Atomic int start_workers;
static int checks;

typedef struct {
    sc2_trigfx_state_t state;
    uint8_t pull;
    int failed;
} worker_t;

#define CHECK(expr) do { \
    checks++; \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s (pulses=%d)\n", __FILE__, \
                __LINE__, #expr, atomic_load(&pulse_count)); \
        exit(1); \
    } \
} while (0)

void sc2_haptic_pulse(int pad, uint16_t on_us, uint16_t off_us, uint16_t count) {
    (void)pad;
    (void)on_us;
    (void)off_us;
    (void)count;
    atomic_fetch_add(&pulse_count, 1);
}

static void *run_session(void *opaque) {
    worker_t *worker = opaque;
    sc2_trigfx_reset(&worker->state);
    while (!atomic_load_explicit(&start_workers, memory_order_acquire))
        sched_yield();
    for (int i = 0; i < 10000; ++i) {
        sc2_trigfx_apply(&worker->state, worker->pull, 0,
                         1000 + (int64_t)i * 4, 100);
    }
    if (worker->pull == 0 && atomic_load(&pulse_count) != 2)
        worker->failed = 1;
    return NULL;
}

int main(void) {
    uint8_t command[56] = {0};
    uint32_t mode = TFX_FEEDBACK;
    memcpy(command, &mode, sizeof(mode));
    command[8] = 4; /* threshold position */
    command[9] = 4; /* haptic strength */

    sc2_trigfx_clear();
    sc2_trigfx_set(0, command);
    sc2_trigfx_enable(1);

    /* Session A is already past the configured threshold. A separate
     * session at rest must not overwrite A's trigger-zone history. */
    sc2_trigfx_state_t first_session, second_session;
    sc2_trigfx_reset(&first_session);
    sc2_trigfx_reset(&second_session);
    sc2_trigfx_apply(&first_session, 128, 0, 1, 100);
    sc2_trigfx_apply(&second_session, 0, 0, 2, 100);
    sc2_trigfx_apply(&first_session, 128, 0, 3, 100);

    CHECK(atomic_load(&pulse_count) == 0);
    sc2_trigfx_apply(&second_session, 128, 0, 4, 100);
    CHECK(atomic_load(&pulse_count) == 1);
    sc2_trigfx_apply(&first_session, 0, 0, 5, 100);
    sc2_trigfx_apply(&first_session, 128, 0, 6, 100);
    CHECK(atomic_load(&pulse_count) == 2);

    worker_t workers[2] = {{.pull = 128}, {.pull = 0}};
    pthread_t threads[2];
    CHECK(pthread_create(&threads[0], NULL, run_session, &workers[0]) == 0);
    CHECK(pthread_create(&threads[1], NULL, run_session, &workers[1]) == 0);
    atomic_store_explicit(&start_workers, 1, memory_order_release);
    CHECK(pthread_join(threads[0], NULL) == 0);
    CHECK(pthread_join(threads[1], NULL) == 0);
    CHECK(workers[0].failed == 0);
    CHECK(workers[1].failed == 0);
    CHECK(atomic_load(&pulse_count) == 2);

    memset(command, 0, sizeof(command));
    mode = TFX_VIBRATION;
    memcpy(command, &mode, sizeof(mode));
    command[8] = 1;  /* start buzzing at trigger position 1 */
    command[9] = 4;  /* amplitude */
    command[10] = 50; /* frequency in Hz */
    sc2_trigfx_clear();
    sc2_trigfx_set(0, command);
    sc2_trigfx_apply(&first_session, 128, 0, 100000, 100);
    sc2_trigfx_apply(&second_session, 128, 0, 100000, 100);
    CHECK(atomic_load(&pulse_count) == 4);

    printf("sc2 trigger effects: %d checks passed, 20,000 concurrent reports\n", checks);
    return 0;
}
