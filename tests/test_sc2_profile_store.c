#include "sc2_profile.h"
#include "game_list.h"
#include "gc_types.h"

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>

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

static int fail_replace;
int sc2_profile_test_fail_replace(void) { return fail_replace; }

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

static int update_button(sc2_profile_t *p, void *context) {
    int input = (int)(intptr_t)context;
    p->map[input] = SCE_PAD_BUTTON_PS;
    return 0;
}
static void *patch_button(void *context) {
    return (void *)(intptr_t)sc2_store_update("patch", update_button, context);
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

    /* Complete reads, atomic failure and the largest supported combination. */
    hook_round_entries = 2;
    sc2_profile_t original, candidate, loaded;
    sc2_profile_default(&original);
    CHECK(sc2_store_save("integrity", &original) == 0);
    char buf[SC2_PROFILE_CAPACITY + 8];
    int bytes = sc2_profile_format(&original, buf, sizeof(buf));
    CHECK(bytes > 0);
    const char *path = SC2_DIR "/bad.ini";
    FILE *file = fopen(path, "w"); CHECK(file != NULL);
    CHECK(fwrite(buf, 1, (size_t)bytes - 1, file) == (size_t)bytes - 1);
    CHECK(fclose(file) == 0);
    loaded = original; loaded.deadzone = 27;
    CHECK(sc2_store_load("bad", &loaded) != 0 && loaded.deadzone == 27);
    file = fopen(path, "w"); CHECK(file != NULL);
    CHECK(fwrite("A=UNKNOWN\n", 1, 10, file) == 10); CHECK(fclose(file) == 0);
    CHECK(sc2_store_load("bad", &loaded) != 0);
    file = fopen(path, "w"); CHECK(file != NULL);
    memset(buf, 'x', sizeof(buf)); CHECK(fwrite(buf, 1, sizeof(buf), file) == sizeof(buf)); CHECK(fclose(file) == 0);
    CHECK(sc2_store_load("bad", &loaded) != 0);
    file = fopen(path, "w"); CHECK(file != NULL);
    bytes = sc2_profile_format(&original, buf, sizeof(buf)); buf[bytes / 2] = 0;
    CHECK(fwrite(buf, 1, (size_t)bytes, file) == (size_t)bytes); CHECK(fclose(file) == 0);
    CHECK(sc2_store_load("bad", &loaded) != 0);
    /* Even syntactically valid line-boundary truncation lacks required fields. */
    bytes = sc2_profile_format(&original, buf, sizeof(buf));
    file = fopen(path, "w"); CHECK(file != NULL);
    CHECK(fwrite(buf, 1, (size_t)bytes - strlen("MOTION=1\n"), file) == (size_t)bytes - strlen("MOTION=1\n")); CHECK(fclose(file) == 0);
    CHECK(sc2_store_load("bad", &loaded) == SC2_STORE_INVALID);
    bytes = sc2_profile_format(&original, buf, sizeof(buf));
    char *motion = strstr(buf, "MOTION=1"); CHECK(motion != NULL); motion[7] = 'x';
    file = fopen(path, "w"); CHECK(file != NULL);
    CHECK(fwrite(buf, 1, (size_t)bytes, file) == (size_t)bytes); CHECK(fclose(file) == 0);
    CHECK(sc2_store_load("bad", &loaded) == SC2_STORE_INVALID);
    candidate = original; candidate.map[IN_A] = SCE_PAD_BUTTON_CIRCLE;
    for (fail_replace = 1; fail_replace <= 3; fail_replace++) {
        CHECK(sc2_store_save("integrity", &candidate) != 0);
        CHECK(sc2_store_load("integrity", &loaded) == 0);
        CHECK(memcmp(&original, &loaded, sizeof(original)) == 0);
        CHECK(access(SC2_DIR "/integrity.ini.tmp", F_OK) != 0);
    }
    fail_replace = 0;
    CHECK(mkdir(SC2_DIR "/integrity.ini.tmp", 0700) == 0);
    CHECK(sc2_store_save("integrity", &candidate) != 0);
    CHECK(sc2_store_load("integrity", &loaded) == 0 && loaded.map[IN_A] == original.map[IN_A]);
    CHECK(rmdir(SC2_DIR "/integrity.ini.tmp") == 0);
    candidate = original;
    for (int i = 0; i < SC2_IN_COUNT; i++) {
        if (i == IN_QAM) continue;
        candidate.map[i] = candidate.lng[i] = candidate.dbl[i] = candidate.shf[i] = 0x110000u | 0xffffu | PB_ACT_MASK;
        candidate.toggle[i] = 1;
    }
    candidate.full[0] = candidate.full[1] = candidate.map[IN_A];
    memset(candidate.name, 'N', sizeof(candidate.name) - 1);
    bytes = sc2_profile_format(&candidate, buf, sizeof(buf)); CHECK(bytes > 8192);
    CHECK(sc2_store_save("maximum", &candidate) == 0);
    CHECK(sc2_store_load("maximum", &loaded) == 0);
    CHECK(memcmp(&candidate, &loaded, sizeof(candidate)) == 0);
    candidate.name[0] = '\n';
    CHECK(sc2_store_save("integrity", &candidate) != 0);
    CHECK(sc2_store_load("integrity", &loaded) == 0 && memcmp(&original, &loaded, sizeof(original)) == 0);
    /* Both real callers now hold the store lock through load/patch/save. */
    CHECK(sc2_store_save("patch", &original) == 0);
    CHECK(pthread_create(&threads[0], NULL, patch_button, (void *)(intptr_t)IN_A) == 0);
    CHECK(pthread_create(&threads[1], NULL, patch_button, (void *)(intptr_t)IN_B) == 0);
    void *patch_result;
    CHECK(pthread_join(threads[0], &patch_result) == 0 && patch_result == NULL);
    CHECK(pthread_join(threads[1], &patch_result) == 0 && patch_result == NULL);
    CHECK(sc2_store_load("patch", &loaded) == 0);
    CHECK(loaded.map[IN_A] == SCE_PAD_BUTTON_PS && loaded.map[IN_B] == SCE_PAD_BUTTON_PS);
    unlink(SC2_DIR "/pipe.ini");
    CHECK(mkfifo(SC2_DIR "/pipe.ini", 0600) == 0);
    alarm(3);
    CHECK(sc2_store_load("pipe", &loaded) == SC2_STORE_IO_ERROR);
    alarm(0);
    CHECK(unlink(SC2_DIR "/pipe.ini") == 0);
    unlink(SC2_DIR "/link.ini");
    CHECK(symlink("integrity.ini", SC2_DIR "/link.ini") == 0);
    CHECK(sc2_store_load("link", &loaded) == SC2_STORE_IO_ERROR);
    CHECK(unlink(SC2_DIR "/link.ini") == 0);
    unlink(SC2_DIR "/default.ini");
    for (unsigned i=0;i<16;++i) { char backup[128];
        snprintf(backup,sizeof(backup),SC2_DIR "/default.ini.recovery.%u",i); unlink(backup);
    }
    file=fopen(SC2_DIR "/default.ini","w"); CHECK(file != NULL);
    CHECK(fputs("broken historical bytes\n",file) >= 0 && fclose(file) == 0);
    CHECK(sc2_store_update("default",update_button,(void*)(intptr_t)IN_A) == SC2_STORE_INVALID);
    CHECK(sc2_store_recover_default() == 0);
    CHECK(sc2_store_load("default", &loaded) == 0 && !memcmp(&original,&loaded,sizeof(original)));
    file=fopen(SC2_DIR "/default.ini.recovery.0","r"); CHECK(file != NULL);
    CHECK(fgets(buf,sizeof(buf),file) && !strcmp(buf,"broken historical bytes\n"));
    CHECK(fclose(file) == 0);
    CHECK(sc2_store_recover_default() != 0); /* Cannot silently reset a valid profile. */
    unlink(path);
    printf("Steam profile storage concurrency: %u checks passed\n", checks);
    return 0;
}
