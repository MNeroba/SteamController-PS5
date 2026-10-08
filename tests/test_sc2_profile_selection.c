#include "sc2_profile.h"
#include "game_list.h"
#include "gc_types.h"

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static pthread_mutex_t gate_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t gate_cond = PTHREAD_COND_INITIALIZER;
static int block_old_commit;
static int old_commit_waiting;
static int release_old_commit;
static int override_done;
static int checks;

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

void sc2_profile_test_before_commit(const char *id, uint32_t generation) {
    (void)generation;
    pthread_mutex_lock(&gate_lock);
    if (block_old_commit && strcmp(id, "old") == 0) {
        block_old_commit = 0;
        old_commit_waiting = 1;
        pthread_cond_broadcast(&gate_cond);
        while (!release_old_commit)
            pthread_cond_wait(&gate_cond, &gate_lock);
    }
    pthread_mutex_unlock(&gate_lock);
}

static void *reload_selection(void *opaque) {
    (void)opaque;
    sc2_select_reload();
    return NULL;
}

static void *select_new_override(void *opaque) {
    (void)opaque;
    sc2_select_override("new");
    pthread_mutex_lock(&gate_lock);
    override_done = 1;
    pthread_cond_broadcast(&gate_cond);
    pthread_mutex_unlock(&gate_lock);
    return NULL;
}

int main(void) {
    sc2_profile_t old_profile, new_profile, active;
    char title[SC2_ID_MAX + 1], selected[SC2_ID_MAX + 1];
    char override_id[SC2_ID_MAX + 1];
    pthread_t stale, update;

    sc2_profile_default(&old_profile);
    sc2_profile_default(&new_profile);
    old_profile.map[IN_A] = SCE_PAD_BUTTON_CROSS;
    new_profile.map[IN_A] = SCE_PAD_BUTTON_CIRCLE;
    snprintf(old_profile.name, sizeof(old_profile.name), "Old profile");
    snprintf(new_profile.name, sizeof(new_profile.name), "New profile");
    CHECK(sc2_store_save("old", &old_profile) == 0);
    CHECK(sc2_store_save("new", &new_profile) == 0);

    sc2_select_override("old");
    sc2_active_get(&active);
    CHECK(active.map[IN_A] == SCE_PAD_BUTTON_CROSS);

    pthread_mutex_lock(&gate_lock);
    block_old_commit = 1;
    pthread_mutex_unlock(&gate_lock);
    CHECK(pthread_create(&stale, NULL, reload_selection, NULL) == 0);
    pthread_mutex_lock(&gate_lock);
    while (!old_commit_waiting)
        pthread_cond_wait(&gate_cond, &gate_lock);
    pthread_mutex_unlock(&gate_lock);

    CHECK(pthread_create(&update, NULL, select_new_override, NULL) == 0);
    pthread_mutex_lock(&gate_lock);
    while (!override_done)
        pthread_cond_wait(&gate_cond, &gate_lock);
    release_old_commit = 1;
    pthread_cond_broadcast(&gate_cond);
    pthread_mutex_unlock(&gate_lock);
    CHECK(pthread_join(stale, NULL) == 0);
    CHECK(pthread_join(update, NULL) == 0);

    sc2_active_get(&active);
    sc2_select_status(title, selected, override_id);
    CHECK(active.map[IN_A] == SCE_PAD_BUTTON_CIRCLE);
    CHECK(strcmp(selected, "new") == 0);
    CHECK(strcmp(override_id, "new") == 0);

    printf("Steam profile selection concurrency: %d checks passed\n", checks);
    return 0;
}
