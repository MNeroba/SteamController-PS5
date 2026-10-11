#include "ds_handoff.h"
#include "sc2_profile.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

void (*sc2_notify_fn)(const char *msg) = NULL;
static unsigned long long checks;

void pb_kernel_lock(void) {}
void pb_kernel_unlock(void) {}
int shellui_pad_disconnect_device(uint64_t physical_device_id) {
    (void)physical_device_id;
    return 0;
}

static void *toggle_worker(void *arg) {
    intptr_t offset = (intptr_t)arg;
    for (int i = 0; i < 200; i++)
        ds_handoff_set_enabled((i + (int)offset) & 1);
    return NULL;
}

static void *observe_worker(void *arg) {
    (void)arg;
    char json[128];
    for (int i = 0; i < 20000; i++) {
        ds_handoff_sc2_session(i & 1);
        ds_handoff_sc2_used();
        (void)ds_handoff_take_release();
        if (ds_handoff_json(json, sizeof(json)) <= 0 ||
            !strstr(json, "\"enabled\":") ||
            !strstr(json, "\"state\":"))
            return (void *)(intptr_t)1;
        __atomic_add_fetch(&checks, 1, __ATOMIC_RELAXED);
    }
    return NULL;
}

int main(void) {
    assert(ds_handoff_set_enabled(1) != 0);
    ds_handoff_start();
    assert(ds_handoff_set_enabled(1) == 0);
    assert(mkdir("build/handoff-settings/dualsense.ini.tmp",0700) == 0);
    assert(ds_handoff_set_enabled(0) != 0);
    char settings_json[128]; ds_handoff_json(settings_json,sizeof(settings_json));
    assert(strstr(settings_json,"\"enabled\":1"));
    assert(rmdir("build/handoff-settings/dualsense.ini.tmp") == 0);
    pthread_t writer, readers[3];
    void *result = NULL;

    if (pthread_create(&writer, NULL, toggle_worker, (void *)(intptr_t)0) != 0)
        return 1;
    for (intptr_t i = 0; i < 3; i++)
        if (pthread_create(&readers[i], NULL, observe_worker, (void *)i) != 0)
            return 1;
    if (pthread_join(writer, NULL) != 0)
        return 1;
    for (int i = 0; i < 3; i++) {
        if (pthread_join(readers[i], &result) != 0 || result != NULL)
            return 1;
    }

    printf("test_ds_handoff: concurrent setting/session/JSON access passed (%llu snapshots)\n",
           __atomic_load_n(&checks, __ATOMIC_RELAXED));
    return 0;
}
