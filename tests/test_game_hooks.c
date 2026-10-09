#define _POSIX_C_SOURCE 200809L
#include "game_hooks.h"
#include "bridge/wireless_ds4.h"
#include "game_hook_safety.h"
#include "sc2_profile.h"

#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

void game_hooks_test_stop(void);

static atomic_ulong checks;
static atomic_int mock_remove_result;
static atomic_int mock_remove_calls;

void ghostpad_status_log(const char *fmt, ...) { (void)fmt; }

void sc2_select_status(char *title, char *active, char *override_id) {
    strcpy(title, "TEST_GAME");
    strcpy(active, "TEST_PROFILE");
    override_id[0] = '\0';
}

void sc2_haptic_rumble_set(uint16_t left, uint16_t right) { (void)left; (void)right; }
void sc2_trigfx_clear(void) {}
void sc2_trigfx_enable(int on) { (void)on; }
void sc2_trigfx_set(int trigger, const uint8_t cmd[56]) { (void)trigger; (void)cmd; }
int sc2_status_haptics_available(void) { return 0; }
int sc2_gyro_is_moving(void) { return 0; }

int pb_hooks_install(int want_input, int want_vibe, int want_audio,
                     pid_t *out_pid, intptr_t *out_args, char *why, size_t why_n) {
    (void)want_input; (void)want_vibe; (void)want_audio;
    *out_pid = getpid(); *out_args = 1;
    snprintf(why, why_n, "host stub");
    return 1;
}

int pb_hooks_publish(pid_t pid, intptr_t args, const PbPublishFrame *frame,
                     int enabled, int mode, int owner, int silence) {
    (void)pid; (void)args; (void)frame; (void)enabled; (void)mode; (void)owner; (void)silence;
    return 0;
}

int pb_hooks_read(pid_t pid, intptr_t args, PbStatus *status) {
    (void)pid; (void)args;
    memset(status, 0, sizeof(*status));
    return 0;
}

int pb_hooks_set_silence(pid_t pid, intptr_t args, int on) {
    (void)pid; (void)args; (void)on;
    return 0;
}

int pb_hooks_remove(pid_t pid, intptr_t args) {
    (void)pid; (void)args;
    atomic_fetch_add_explicit(&mock_remove_calls, 1, memory_order_relaxed);
    return atomic_load_explicit(&mock_remove_result, memory_order_relaxed);
}

static void pause_ms(long ms) {
    struct timespec req = { ms / 1000, (ms % 1000) * 1000000L };
    while (nanosleep(&req, &req) != 0) {}
}

static int write_malformed_settings(void) {
    FILE *file = fopen("/tmp/steamcontroller-game-hooks-test.ini", "w");
    int write_failed, close_result;
    if (!file) return -1;
    write_failed = fputs("GAME_INPUT=2\nGAME_VIBRATION=-1\nGAME_PRIORITY=1junk\n"
                         "GAME_PRIORITY=999999999999999999999999999999999999\n"
                         "GAME_AUDIO_HAPTICS=0\n", file) == EOF;
    close_result = fclose(file);
    return write_failed || close_result != 0 ? -1 : 0;
}

static void *settings_worker(void *arg) {
    intptr_t id = (intptr_t)arg;
    for (int i = 0; i < 250; i++) {
        game_hooks_set((i + (int)id) & 1, (i / 3 + (int)id) & 1, (i + (int)id) % 3);
        game_hooks_set_audio((i + (int)id) & 1);
        atomic_fetch_add_explicit(&checks, 2, memory_order_relaxed);
    }
    return NULL;
}

static void *pad_worker(void *arg) {
    (void)arg;
    for (int i = 0; i < 30000; i++) {
        ScePadData pad;
        memset(&pad, 0, sizeof(pad));
        pad.leftStick.x = (uint8_t)(i & 0xff);
        pad.leftStick.y = 128;
        pad.rightStick.x = 128;
        pad.rightStick.y = 128;
        pad.buttons = (uint32_t)(i & 0xffff);
        game_hooks_feed(&pad, i & 1);
        atomic_fetch_add_explicit(&checks, 1, memory_order_relaxed);
    }
    return NULL;
}

static void *observe_worker(void *arg) {
    (void)arg;
    char json[512];
    for (int i = 0; i < 30000; i++) {
        int n = game_hooks_json(json, sizeof(json));
        if (n <= 0 || !strstr(json, "\"audio\":") ||
            !strstr(json, "\"owner\":") || !strstr(json, "\"title\":"))
            return (void *)(intptr_t)1;
        (void)game_hooks_input_active();
        atomic_fetch_add_explicit(&checks, 1, memory_order_relaxed);
    }
    return NULL;
}

int main(void) {
    pthread_t settings[2], pad, observer;
    void *result = NULL;
    char too_small[3];

    if (write_malformed_settings() != 0) return 1;
    if (game_hooks_json(NULL, sizeof(too_small)) != -1 ||
        game_hooks_json(too_small, sizeof(too_small)) != -1)
        return 1;
    game_hooks_start();
    char initial_json[512];
    if (game_hooks_json(initial_json, sizeof(initial_json)) <= 0 ||
        !strstr(initial_json, "\"audio\":0") ||
        !strstr(initial_json, "\"mode\":2") ||
        !strstr(initial_json, "\"input\":0") ||
        !strstr(initial_json, "\"vibration\":0")) {
        fprintf(stderr, "FAIL malformed persisted game-hook settings were accepted: %s\n",
                initial_json);
        game_hooks_test_stop();
        return 1;
    }
    atomic_fetch_add_explicit(&checks, 4, memory_order_relaxed);
    if (pthread_create(&settings[0], NULL, settings_worker, (void *)(intptr_t)0) != 0 ||
        pthread_create(&settings[1], NULL, settings_worker, (void *)(intptr_t)1) != 0 ||
        pthread_create(&pad, NULL, pad_worker, NULL) != 0 ||
        pthread_create(&observer, NULL, observe_worker, NULL) != 0)
        return 1;
    for (int i = 0; i < 2; i++)
        if (pthread_join(settings[i], NULL) != 0) return 1;
    if (pthread_join(pad, NULL) != 0 || pthread_join(observer, &result) != 0 || result != NULL)
        return 1;

    /* Let the hook worker pass its load delay and execute the install/read path. */
    pause_ms(1750);
    game_hooks_set(1, 0, 2);
    char json[512];
    int hooked = 0;
    for (int i = 0; i < 150; i++) {
        if (game_hooks_json(json, sizeof(json)) > 0 &&
            strstr(json, "TEST_GAME") && strstr(json, "\"state\":2")) {
            hooked = 1;
            break;
        }
        pause_ms(20);
    }
    if (!hooked) {
        fprintf(stderr, "FAIL test worker did not install mock game hook: %s\n", json);
        game_hooks_test_stop();
        return 1;
    }

    /* A permanent target write failure must not make payload shutdown hang. */
    atomic_store_explicit(&mock_remove_calls, 0, memory_order_relaxed);
    atomic_store_explicit(&mock_remove_result, PB_HOOKS_REMOVE_RETRY,
                          memory_order_relaxed);
    int stop_result = game_hooks_stop();
    int remove_calls = atomic_load_explicit(&mock_remove_calls, memory_order_relaxed);
    if (stop_result != GAME_HOOKS_STOP_ROLLBACK_PENDING ||
        remove_calls != GAME_HOOK_SHUTDOWN_ROLLBACK_ATTEMPTS) {
        fprintf(stderr, "FAIL bounded hook rollback: stop=%d remove_calls=%d\n",
                stop_result, remove_calls);
        return 1;
    }
    printf("test_game_hooks: concurrent access and bounded shutdown rollback passed (%lu checks)\n",
           atomic_load_explicit(&checks, memory_order_relaxed));
    return 0;
}
