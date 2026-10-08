#include "sc2_haptics.h"
#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>

static pthread_mutex_t gate_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t gate_cond = PTHREAD_COND_INITIALIZER;
static int block_first_a = 1;
static int a_waiting;
static int release_a;
static _Atomic int start_stress;
static _Atomic int fake_method = 4;
static _Atomic int a_extra_reports;
static _Atomic int b_extra_reports;
static _Atomic int over_limit_extra_reports;
static _Atomic int negative_extra_reports;
static int checks;

typedef struct { int fd; } worker_t;

#define CHECK(expr) do { \
    checks++; \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s (A extra=%d, B extra=%d)\n", \
                __FILE__, __LINE__, #expr, atomic_load(&a_extra_reports), \
                atomic_load(&b_extra_reports)); \
        exit(1); \
    } \
} while (0)

int sc2_haptics_test_send_out(int fd, int idx) {
    if (idx >= 2 && fd == 1) atomic_fetch_add(&a_extra_reports, 1);
    if (idx >= 2 && fd == 2) atomic_fetch_add(&b_extra_reports, 1);
    if (idx >= 2 && fd == 5) atomic_fetch_add(&over_limit_extra_reports, 1);
    if (idx >= 2 && fd == 6) atomic_fetch_add(&negative_extra_reports, 1);
    if (fd == 1 && idx == 1) {
        pthread_mutex_lock(&gate_lock);
        if (block_first_a) {
            block_first_a = 0;
            a_waiting = 1;
            pthread_cond_broadcast(&gate_cond);
            while (!release_a) pthread_cond_wait(&gate_cond, &gate_lock);
        }
        pthread_mutex_unlock(&gate_lock);
    }
    return 0;
}

int sc2_status_haptic_method(void) { return atomic_load(&fake_method); }
void sc2_status_haptic_queued(void) {}
void sc2_status_haptic_result(int via, int error) { (void)via; (void)error; }

static void *run_service(void *opaque) {
    worker_t *worker = opaque;
    sc2_haptic_service(worker->fd, NULL, 1, 0, worker->fd == 1 ? 0 : 2);
    return NULL;
}

static void *run_stress(void *opaque) {
    worker_t *worker = opaque;
    while (!atomic_load_explicit(&start_stress, memory_order_acquire))
        sched_yield();
    for (int i = 0; i < 2000; ++i) {
        sc2_haptic_pulse(SC2_PAD_LEFT, 400, 100, 1);
        sc2_haptic_service(worker->fd, NULL, 1, 0,
                           worker->fd == 3 ? 1 : 2);
    }
    return NULL;
}

int main(void) {
    sc2_haptic_pulse(SC2_PAD_LEFT, 400, 100, 1);
    sc2_haptic_pulse(SC2_PAD_RIGHT, 400, 100, 1);

    worker_t first = {.fd = 1}, second = {.fd = 2};
    pthread_t a, b;
    CHECK(pthread_create(&a, NULL, run_service, &first) == 0);
    pthread_mutex_lock(&gate_lock);
    while (!a_waiting) pthread_cond_wait(&gate_cond, &gate_lock);
    pthread_mutex_unlock(&gate_lock);

    CHECK(pthread_create(&b, NULL, run_service, &second) == 0);
    CHECK(pthread_join(b, NULL) == 0);
    pthread_mutex_lock(&gate_lock);
    release_a = 1;
    pthread_cond_broadcast(&gate_cond);
    pthread_mutex_unlock(&gate_lock);
    CHECK(pthread_join(a, NULL) == 0);

    atomic_store(&fake_method, 0);
    worker_t stress[2] = {{.fd = 3}, {.fd = 4}};
    CHECK(pthread_create(&a, NULL, run_stress, &stress[0]) == 0);
    CHECK(pthread_create(&b, NULL, run_stress, &stress[1]) == 0);
    atomic_store_explicit(&start_stress, 1, memory_order_release);
    CHECK(pthread_join(a, NULL) == 0);
    CHECK(pthread_join(b, NULL) == 0);

    /* Session A has no extra OUTs. Session B has exactly two. A must not
     * inherit B's route count while its queued report is being sent. */
    CHECK(atomic_load(&a_extra_reports) == 0);
    CHECK(atomic_load(&b_extra_reports) == 2);

    atomic_store(&fake_method, 4);
    sc2_haptic_pulse(SC2_PAD_LEFT, 400, 100, 1);
    sc2_haptic_service(5, NULL, 1, 0, 100);
    CHECK(atomic_load(&over_limit_extra_reports) == SC2_HAP_MAX_OUT);
    sc2_haptic_pulse(SC2_PAD_LEFT, 400, 100, 1);
    sc2_haptic_service(6, NULL, 1, 0, -10);
    CHECK(atomic_load(&negative_extra_reports) == 0);

    printf("sc2 haptic queue: %d checks passed, 4,000 concurrent service reports\n", checks);
    return 0;
}
