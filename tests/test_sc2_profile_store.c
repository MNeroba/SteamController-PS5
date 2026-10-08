#include "sc2_profile.h"
#include "game_list.h"
#include "gc_types.h"

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static pthread_mutex_t gate_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t gate_cond = PTHREAD_COND_INITIALIZER;
static unsigned int ready_threads;
static int start_writers;
static unsigned int hook_entries;
static unsigned int hook_round_entries;
static unsigned int hook_active;
static unsigned int hook_max_active;
static unsigned int checks;

#define CHECK(condition) do { \
    checks++; \
    if (!(condition)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        exit(1); \
    } \
} while (0)

int gc_game_name(const char *id, char *out, size_t outsz) {
    (void)id;
    if (outsz) out[0] = '\0';
    return 0;
}

int sceLncUtilGetAppIdOfRunningBigApp(void) { return 0; }
int sceLncUtilGetAppTitleId(uint32_t app_id, char *title_id) {
    (void)app_id;
    (void)title_id;
    return -1;
}

void sc2_profile_test_store_critical_enter(void) {
    struct timespec deadline;
    clock_gettime(CLOCK_REALTIME, &deadline);
    deadline.tv_sec += 2;

    pthread_mutex_lock(&gate_lock);
    hook_active++;
    if (hook_active > hook_max_active) hook_max_active = hook_active;
    hook_entries++;
    hook_round_entries++;
    pthread_cond_broadcast(&gate_cond);
    while (hook_round_entries < 2) {
        int rc = pthread_cond_timedwait(&gate_cond, &gate_lock, &deadline);
        if (rc != 0) break;
    }
    hook_active--;
    pthread_mutex_unlock(&gate_lock);
}

void sc2_profile_test_before_commit(const char *id, uint32_t generation) {
    (void)id;
    (void)generation;
}

typedef struct {
    sc2_profile_t profile;
    int result;
} writer_t;

typedef struct {
    int result;
} deleter_t;

static void wait_for_start(void) {
    pthread_mutex_lock(&gate_lock);
    ready_threads++;
    pthread_cond_broadcast(&gate_cond);
    while (!start_writers)
        pthread_cond_wait(&gate_cond, &gate_lock);
    pthread_mutex_unlock(&gate_lock);
}

static void *save_profile(void *opaque) {
    writer_t *writer = (writer_t *)opaque;
    wait_for_start();
    writer->result = sc2_store_save("concurrent", &writer->profile);
    return NULL;
}

static void *delete_profile(void *opaque) {
    deleter_t *deleter = (deleter_t *)opaque;
    wait_for_start();
    deleter->result = sc2_store_delete("concurrent");
    return NULL;
}

static void release_writers(void) {
    pthread_mutex_lock(&gate_lock);
    while (ready_threads < 2)
        pthread_cond_wait(&gate_cond, &gate_lock);
    start_writers = 1;
    pthread_cond_broadcast(&gate_cond);
    pthread_mutex_unlock(&gate_lock);
}

int main(void) {
    writer_t writers[2];
    pthread_t threads[2];
    sc2_profile_t stored;

    sc2_profile_default(&writers[0].profile);
    sc2_profile_default(&writers[1].profile);
    writers[0].profile.map[IN_A] = SCE_PAD_BUTTON_CROSS;
    writers[1].profile.map[IN_A] = SCE_PAD_BUTTON_CIRCLE;
    snprintf(writers[0].profile.name, sizeof(writers[0].profile.name), "Writer one");
    snprintf(writers[1].profile.name, sizeof(writers[1].profile.name), "Writer two");
    writers[0].result = writers[1].result = -99;

    CHECK(pthread_create(&threads[0], NULL, save_profile, &writers[0]) == 0);
    CHECK(pthread_create(&threads[1], NULL, save_profile, &writers[1]) == 0);
    release_writers();

    CHECK(pthread_join(threads[0], NULL) == 0);
    CHECK(pthread_join(threads[1], NULL) == 0);
    CHECK(hook_entries == 2);
    CHECK(hook_round_entries == 2);
    CHECK(writers[0].result == 0);
    CHECK(writers[1].result == 0);
    CHECK(sc2_store_load("concurrent", &stored) == 0);
    CHECK(stored.map[IN_A] == SCE_PAD_BUTTON_CROSS ||
          stored.map[IN_A] == SCE_PAD_BUTTON_CIRCLE);
    CHECK(strcmp(stored.name, "Writer one") == 0 ||
          strcmp(stored.name, "Writer two") == 0);
    CHECK((stored.map[IN_A] == SCE_PAD_BUTTON_CROSS &&
           strcmp(stored.name, "Writer one") == 0) ||
          (stored.map[IN_A] == SCE_PAD_BUTTON_CIRCLE &&
           strcmp(stored.name, "Writer two") == 0));

    deleter_t deleter = { .result = -99 };
    pthread_mutex_lock(&gate_lock);
    ready_threads = 0;
    start_writers = 0;
    hook_round_entries = 0;
    pthread_mutex_unlock(&gate_lock);
    CHECK(pthread_create(&threads[0], NULL, save_profile, &writers[0]) == 0);
    CHECK(pthread_create(&threads[1], NULL, delete_profile, &deleter) == 0);
    release_writers();
    CHECK(pthread_join(threads[0], NULL) == 0);
    CHECK(pthread_join(threads[1], NULL) == 0);
    CHECK(writers[0].result == 0);
    CHECK(deleter.result == 0);
    CHECK(hook_entries == 4);
    CHECK(hook_round_entries == 2);
    CHECK(hook_max_active == 1);
    int load_result = sc2_store_load("concurrent", &stored);
    CHECK(load_result == 0 || load_result == -1);

    printf("Steam profile storage concurrency: %u checks passed\n", checks);
    return 0;
}
