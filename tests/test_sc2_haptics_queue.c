#include "sc2_haptics.h"
#include <stdint.h>
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
static _Atomic int session_a_primary_reports;
static _Atomic int session_b_primary_reports;
static _Atomic int zero_rumble_stop_reports;
static int checks;

typedef struct {
    int fd;
    slot_registry_token_t owner;
    usb_resources_t resources;
} worker_t;

static void init_fake_resources(usb_resources_t *resources, int fd) {
    usb_resources_init(resources);
    resources->fd = fd;
}

#define CHECK(expr) do { \
    checks++; \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s (A extra=%d, B extra=%d)\n", \
                __FILE__, __LINE__, #expr, atomic_load(&a_extra_reports), \
                atomic_load(&b_extra_reports)); \
        exit(1); \
    } \
} while (0)

int sc2_haptics_test_send_out(int fd, int idx, const uint8_t *data, uint32_t len) {
    if (fd == 11 && idx == 1) atomic_fetch_add(&session_a_primary_reports, 1);
    if (fd == 12 && idx == 1) atomic_fetch_add(&session_b_primary_reports, 1);
    if (fd == 11 && idx == 1 && data && len == 10 && data[0] == 0x80 &&
        data[4] == 0 && data[5] == 0 && data[7] == 0 && data[8] == 0)
        atomic_fetch_add(&zero_rumble_stop_reports, 1);
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
    sc2_haptic_service(worker->owner, &worker->resources, NULL, NULL, 1, 0,
                       worker->fd == 1 ? 0 : 2);
    return NULL;
}

static void *enqueue_from_process_producer(void *opaque) {
    (void)opaque;
    /* Portal/game-hook producers target the sole active Steam session. */
    sc2_haptic_pulse(SC2_PAD_RIGHT, 500, 100, 1);
    return NULL;
}

static void *run_stress(void *opaque) {
    worker_t *worker = opaque;
    while (!atomic_load_explicit(&start_stress, memory_order_acquire))
        sched_yield();
    for (int i = 0; i < 2000; ++i) {
        sc2_haptic_pulse(SC2_PAD_LEFT, 400, 100, 1);
        sc2_haptic_service(worker->owner, &worker->resources, NULL, NULL, 1, 0,
                           worker->fd == 3 ? 1 : 2);
    }
    return NULL;
}

int main(void) {
    const slot_registry_token_t session_a = {.index = 0, .generation = 1};
    const slot_registry_token_t session_b = {.index = 1, .generation = 1};
    CHECK(sc2_haptic_session_claim(session_a));
    CHECK(!sc2_haptic_session_claim(session_b));
    CHECK(sc2_haptic_session_is_owner(session_a));
    CHECK(!sc2_haptic_session_is_owner(session_b));

    sc2_haptic_pulse(SC2_PAD_LEFT, 400, 100, 1);
    sc2_haptic_pulse(SC2_PAD_RIGHT, 400, 100, 1);

    worker_t first = {.fd = 1, .owner = session_a};
    worker_t second = {.fd = 2, .owner = session_a};
    init_fake_resources(&first.resources, first.fd);
    init_fake_resources(&second.resources, second.fd);
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
    worker_t stress[2] = {
        {.fd = 3, .owner = session_a},
        {.fd = 4, .owner = session_a}
    };
    init_fake_resources(&stress[0].resources, stress[0].fd);
    init_fake_resources(&stress[1].resources, stress[1].fd);
    CHECK(pthread_create(&a, NULL, run_stress, &stress[0]) == 0);
    CHECK(pthread_create(&b, NULL, run_stress, &stress[1]) == 0);
    atomic_store_explicit(&start_stress, 1, memory_order_release);
    CHECK(pthread_join(a, NULL) == 0);
    CHECK(pthread_join(b, NULL) == 0);

    /* Two concurrent service calls share one owner in this stress harness.
     * Their extra-OUT route counts must still remain call-local. */
    CHECK(atomic_load(&a_extra_reports) == 0);
    CHECK(atomic_load(&b_extra_reports) == 2);

    atomic_store(&fake_method, 4);
    sc2_haptic_pulse(SC2_PAD_LEFT, 400, 100, 1);
    usb_resources_t one_call;
    init_fake_resources(&one_call, 5);
    sc2_haptic_service(session_a, &one_call, NULL, NULL, 1, 0, 100);
    CHECK(atomic_load(&over_limit_extra_reports) == SC2_HAP_MAX_OUT);
    sc2_haptic_pulse(SC2_PAD_LEFT, 400, 100, 1);
    init_fake_resources(&one_call, 6);
    sc2_haptic_service(session_a, &one_call, NULL, NULL, 1, 0, -10);
    CHECK(atomic_load(&negative_extra_reports) == 0);

    /* A non-owner service call must not drain the sole owner's queue. */
    atomic_store(&fake_method, 0);
    CHECK(pthread_create(&a, NULL, enqueue_from_process_producer, NULL) == 0);
    CHECK(pthread_join(a, NULL) == 0);
    init_fake_resources(&one_call, 12);
    sc2_haptic_service(session_b, &one_call, NULL, NULL, 1, 0, 0);
    CHECK(atomic_load(&session_b_primary_reports) == 0);
    CHECK(atomic_load(&session_a_primary_reports) == 0);
    init_fake_resources(&one_call, 11);
    sc2_haptic_service(session_a, &one_call, NULL, NULL, 1, 0, 0);
    CHECK(atomic_load(&session_a_primary_reports) == 1);

    /* Teardown clears queued output and rumble before another generation can
     * claim the sole SC2 haptic session. A stale token cannot release B. */
    sc2_haptic_pulse(SC2_PAD_LEFT, 400, 100, 1);
    sc2_haptic_rumble_set(0x4000, 0x4000);
    init_fake_resources(&one_call, 11);
    CHECK(sc2_haptic_session_release(session_a, &one_call, NULL, NULL,
                                     1, 0, 0));
    CHECK(atomic_load(&zero_rumble_stop_reports) == 1);
    CHECK(!sc2_haptic_session_is_owner(session_a));
    CHECK(sc2_haptic_session_claim(session_b));
    sc2_haptic_session_abandon(session_a);
    CHECK(sc2_haptic_session_is_owner(session_b));
    init_fake_resources(&one_call, 12);
    sc2_haptic_service(session_b, &one_call, NULL, NULL, 1, 0, 0);
    CHECK(atomic_load(&session_b_primary_reports) == 0);
    init_fake_resources(&one_call, -1);
    CHECK(sc2_haptic_session_release(session_b, &one_call, NULL, NULL,
                                     0, 0, 0));

    printf("sc2 haptic queue: %d checks passed, 4,000 concurrent service reports\n", checks);
    return 0;
}
