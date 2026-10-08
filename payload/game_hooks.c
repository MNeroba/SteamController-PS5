/* game_hooks.c — installs the in-game hooks when a game runs and feeds them.
 * Settings persist in /data/ghostpad/settings.ini. */
#include "game_hooks.h"
#include "bridge/wireless_ds4.h"
#include "sc2_profile.h"
#include "sc2_haptics.h"
#include "sc2_status.h"
#include "sc2_gyro.h"
#include "sc2_trigfx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#ifdef GAME_HOOKS_TEST
#include <stdatomic.h>
static atomic_int g_test_stop;
static pthread_t g_test_thread;
static int g_test_thread_started;
#endif

void ghostpad_status_log(const char *fmt, ...);
#define LOG(...) ghostpad_status_log("[GC] " __VA_ARGS__)
#ifndef GAME_HOOKS_SETTINGS_PATH
#define GAME_HOOKS_SETTINGS_PATH "/data/ghostpad/settings.ini"
#endif

static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_settings_io_lock = PTHREAD_MUTEX_INITIALIZER;
typedef struct {
    int input_on, vibe_on, mode, audio_on;
} GameHookSettings;
static GameHookSettings g_settings = { 0, 0, 2, 1 }; /* hooks are opt-in; mode: both, last used, SC2 only */
static int g_owner = 0;
static ScePadData g_pad; static int g_pad_conn = 0; static uint32_t g_pad_seq = 0;
static int g_active = 0;
static struct {
    char title[SC2_ID_MAX + 1];
    int  state;              /* 0 off, 1 waiting, 2 hooked, 3 failed */
    char why[96];
    pid_t pid; intptr_t args;
    PbStatus st;
} g;

static int64_t now_ms(void) {
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

static GameHookSettings settings_snapshot(void) {
    GameHookSettings settings;
    pthread_mutex_lock(&g_lock);
    settings = g_settings;
    pthread_mutex_unlock(&g_lock);
    return settings;
}

static void active_set(int active) {
    pthread_mutex_lock(&g_lock);
    g_active = active ? 1 : 0;
    pthread_mutex_unlock(&g_lock);
}

static void load_settings(void) {
    FILE *f = fopen(GAME_HOOKS_SETTINGS_PATH, "r"); if (!f) return;
    GameHookSettings settings = { 0, 0, 2, 1 };
    char l[128];
    while (fgets(l, sizeof(l), f)) {
        if (!strncmp(l, "GAME_INPUT=", 11))     settings.input_on = atoi(l + 11) != 0;
        if (!strncmp(l, "GAME_VIBRATION=", 15)) settings.vibe_on  = atoi(l + 15) != 0;
        if (!strncmp(l, "GAME_AUDIO_HAPTICS=", 19)) settings.audio_on = atoi(l + 19) != 0;
        if (!strncmp(l, "GAME_PRIORITY=", 14))  { int m = atoi(l + 14); settings.mode = (m >= 0 && m <= 2) ? m : 1; }
    }
    fclose(f);
    pthread_mutex_lock(&g_lock);
    g_settings = settings;
    pthread_mutex_unlock(&g_lock);
}

static void save_settings(void) {
    pthread_mutex_lock(&g_settings_io_lock);
    GameHookSettings settings = settings_snapshot();
    FILE *f = fopen(GAME_HOOKS_SETTINGS_PATH, "w");
    if (!f) { pthread_mutex_unlock(&g_settings_io_lock); return; }
    fprintf(f, "GAME_INPUT=%d\nGAME_VIBRATION=%d\nGAME_PRIORITY=%d\nGAME_AUDIO_HAPTICS=%d\n",
            settings.input_on ? 1 : 0, settings.vibe_on ? 1 : 0, settings.mode, settings.audio_on ? 1 : 0);
    fclose(f);
    pthread_mutex_unlock(&g_settings_io_lock);
}
static void set_state(int st, const char *why) {
    pthread_mutex_lock(&g_lock);
    g.state = st; snprintf(g.why, sizeof(g.why), "%s", why ? why : "");
    pthread_mutex_unlock(&g_lock);
}

void game_hooks_feed(const ScePadData *pad, int connected) {
    pthread_mutex_lock(&g_lock);
    if (pad) g_pad = *pad;
    g_pad_conn = connected;
    g_pad_seq++;
    pthread_mutex_unlock(&g_lock);
}

int game_hooks_input_active(void) {
    pthread_mutex_lock(&g_lock);
    int active = g_active;
    pthread_mutex_unlock(&g_lock);
    return active;
}

static int sc2_in_use(const ScePadData *p) {
    if (p->buttons) return 1;
    int v[4] = { p->leftStick.x, p->leftStick.y, p->rightStick.x, p->rightStick.y };
    for (int i = 0; i < 4; i++) if (v[i] < 128 - 40 || v[i] > 128 + 40) return 1;
    return p->analogButtons.l2 > 40 || p->analogButtons.r2 > 40 || p->touchData.fingers;
}

static void to_frame(const ScePadData *p, PbPublishFrame *f) {
    memset(f, 0, sizeof(*f));
    f->buttons = p->buttons;
    f->lx = p->leftStick.x;  f->ly = p->leftStick.y;
    f->rx = p->rightStick.x; f->ry = p->rightStick.y;
    f->l2 = p->analogButtons.l2; f->r2 = p->analogButtons.r2;
    f->move_l = (abs((int)f->lx - 128) > 12 || abs((int)f->ly - 128) > 12);
    f->move_r = (abs((int)f->rx - 128) > 12 || abs((int)f->ry - 128) > 12) || sc2_gyro_is_moving();
    f->fingers = p->touchData.fingers;
    for (int i = 0; i < 2 && i < f->fingers; i++) {
        f->tx[i] = p->touchData.touch[i].x; f->ty[i] = p->touchData.touch[i].y; f->tid[i] = p->touchData.touch[i].finger;
    }
}

static int unhook(const char *why) {
    if (g.state == 2 && g.pid > 0) {
        if (kill(g.pid, 0) != 0) {
            if (errno != ESRCH) {
                LOG("game hooks: cannot verify process %d (%s, errno=%d)\n",
                    g.pid, why, errno);
                active_set(0);
                return -1;
            }
        } else {
            int r = pb_hooks_remove(g.pid, g.args);
            LOG("game hooks: removal from %s returned %d (%s)\n",
                g.title, r, why);
            if (r != 0 && r != PB_HOOKS_REMOVE_CONFLICT) {
                active_set(0);
                return -1;
            }
            if (r == PB_HOOKS_REMOVE_CONFLICT)
                LOG("game hooks: left a later owner’s import slot unchanged\n");
        }
    }
    active_set(0);
    g.pid = -1; g.args = 0;
    return 0;
}

static void *hooks_thread(void *arg) {
    (void)arg;
    char cur[SC2_ID_MAX + 1] = "";
    int64_t title_since = 0, next_try = 0, next_stat = 0;
    int tries = 0, rumbling = 0;
    uint32_t last_vseq = 0, last_pad_seq = 0, last_native_act = 0, last_aseq = 0, last_tseq = 0;
    int64_t last_hap_ms = 0;
    int last_in = -1, last_vib = -1, rollback_pending = 0;

    for (;;) {
#ifdef GAME_HOOKS_TEST
        if (atomic_load_explicit(&g_test_stop, memory_order_relaxed)) break;
#endif
        char t[SC2_ID_MAX + 1], a[SC2_ID_MAX + 1], o[SC2_ID_MAX + 1];
        sc2_select_status(t, a, o);
        int64_t now = now_ms();
        GameHookSettings settings = settings_snapshot();
        int want_in = settings.input_on, want_vib = settings.vibe_on;

        if (strcmp(t, cur) || want_in != last_in || want_vib != last_vib) {   /* game or settings changed */
            if (rumbling) { sc2_haptic_rumble_set(0, 0); rumbling = 0; }
            if (g.state == 2 && unhook(!strcmp(t, cur) ? "settings changed" : "game changed") != 0)
                rollback_pending = 1;
            else { active_set(0); g.pid = -1; g.args = 0; }
            snprintf(cur, sizeof(cur), "%s", t);
            last_in = want_in; last_vib = want_vib;
            title_since = strcmp(t, g.title) ? now : title_since;
            tries = 0; next_try = 0; last_vseq = 0; last_tseq = 0;
            sc2_trigfx_clear(); sc2_trigfx_enable(0);
            pthread_mutex_lock(&g_lock);
            snprintf(g.title, sizeof(g.title), "%s", t);
            memset(&g.st, 0, sizeof(g.st));
            pthread_mutex_unlock(&g_lock);
            if (rollback_pending)
                set_state(2, "hook removal pending; retaining target ownership");
            else
                set_state(0, "");
        }

        if (rollback_pending) {
            if (unhook("retrying incomplete rollback") != 0) {
                set_state(2, "hook removal pending; retrying safely");
                usleep(100000);
                continue;
            }
            rollback_pending = 0;
            set_state(0, "");
        }

        if ((!want_in && !want_vib) || !cur[0]) {
            if (g.state != 0) set_state(0, !cur[0] ? "no game running" : "off");
            usleep(250000); continue;
        }

        if (g.state != 2) {
            if (g.state == 3 && tries >= 12) { usleep(250000); continue; }
            if (tries >= 40) { set_state(3, "gave up: game never finished loading its pad library"); tries = 12; continue; }
            if (now - title_since < 1500 || now < next_try) {
                if (g.state != 1) set_state(1, "waiting for the game to finish loading");
                usleep(100000); continue;
            }
            pid_t pid; intptr_t args; char why[96];
            int r = pb_hooks_install(want_in, want_vib, settings.audio_on, &pid, &args, why, sizeof(why));
            tries++;
            LOG("game hooks: install on %s (input=%d vibration=%d) → %d (%s)\n", cur, want_in, want_vib, r, why);
            if (r == 1) {
                pthread_mutex_lock(&g_lock);
                g.pid = pid; g.args = args; g.state = 2;
                snprintf(g.why, sizeof(g.why), "%s", why);
                pthread_mutex_unlock(&g_lock);
                active_set(want_in);
            } else {
                set_state(r == -4 ? 1 : 3, why);
                next_try = now + (r == -4 ? 1000 : 5000);
            }
            continue;
        }

        if (kill(g.pid, 0) != 0) {                  /* game process gone */
            if (errno != ESRCH) { usleep(100000); continue; }
            if (rumbling) { sc2_haptic_rumble_set(0, 0); rumbling = 0; }
            active_set(0); g.pid = -1; g.args = 0; title_since = now; tries = 0;
            last_tseq = 0; sc2_trigfx_clear(); sc2_trigfx_enable(0);
            set_state(1, "game restarted, hooking again");
            continue;
        }

        PbStatus st; int have_st = pb_hooks_read(g.pid, g.args, &st) == 0;

        ScePadData p; int conn; uint32_t seq;
        pthread_mutex_lock(&g_lock); p = g_pad; conn = g_pad_conn; seq = g_pad_seq; pthread_mutex_unlock(&g_lock);

        /* Which controller is active right now.
         *   input merge off (Option A, the usual path): the Steam Controller is
         *     the active pad whenever it is connected; when it sleeps/turns off
         *     its virtual pad is removed and the DualSense is player 1.
         *   input merge on: fall back to the last-used owner logic. */
        pthread_mutex_lock(&g_lock);
        int prev_owner = g_owner;
        pthread_mutex_unlock(&g_lock);
        int owner = prev_owner;
        if (!want_in) {
            owner = conn ? 1 : 0;
        } else if (settings.mode == 2) owner = 1;
        else if (settings.mode == 0) owner = 0;
        else {
            if (conn && sc2_in_use(&p)) owner = 1;
            else if (have_st && st.native_act != last_native_act) owner = 0;
            if (!conn) owner = 0;
        }
        pthread_mutex_lock(&g_lock);
        g_owner = owner;
        pthread_mutex_unlock(&g_lock);
        if (have_st) last_native_act = st.native_act;
        int sc2_active = conn && (want_in ? (owner == 1) : 1);

        /* Silence the DualSense's haptics/speaker only while the Steam
         * Controller is the active pad. Written every loop (even with input
         * merge off), so it is authoritative on its own. */
        pb_hooks_set_silence(g.pid, g.args, (settings.audio_on && sc2_active) ? 1 : 0);

        if (owner != prev_owner) {
            LOG("game hooks: %s now controls the player\n", owner ? "Steam Controller" : "DualSense");
            if (!sc2_active && rumbling) { sc2_haptic_rumble_set(0, 0); rumbling = 0; }
        }

        /* input merge path (only when enabled): publish the SC2 frame */
        if (want_in && (seq != last_pad_seq || owner != prev_owner || now >= next_stat)) {
            last_pad_seq = seq;
            PbPublishFrame f; to_frame(&p, &f);
            pb_hooks_publish(g.pid, g.args, &f, conn, settings.mode, owner, settings.audio_on);
        }

        /* audio haptics (PS5 games): actuator levels → Steam Controller rumble,
         * only while the Steam Controller is the active pad */
        if (have_st && settings.audio_on && sc2_active && st.aseq != last_aseq) {
            last_aseq = st.aseq; last_hap_ms = now;
            if (sc2_status_haptics_available()) {
                uint32_t l = st.hap_l * 3u, r = st.hap_r * 3u;   /* voice-coil levels are small */
                if (l > 255) l = 255; if (r > 255) r = 255;
                sc2_haptic_rumble_set((uint16_t)(l << 8), (uint16_t)(r << 8));
                rumbling = (l || r);
            }
        } else if (rumbling && last_hap_ms && (!sc2_active || (settings.audio_on && now - last_hap_ms > 120))) {
            sc2_haptic_rumble_set(0, 0); rumbling = 0; last_hap_ms = 0;
        }

        /* adaptive trigger effects: kept per trigger, felt while the Steam
         * Controller is the active pad */
        sc2_trigfx_enable(want_vib && sc2_active);
        if (have_st && st.tseq != last_tseq) {
            if (!last_tseq) LOG("game hooks: first trigger effect from %s\n", cur);
            last_tseq = st.tseq;
            sc2_trigfx_set(0, st.tcmd[0]);
            sc2_trigfx_set(1, st.tcmd[1]);
        }

        /* classic vibration (PS4 games): forward to SC2 while it is the active pad */
        if (have_st) {
            if (want_vib && sc2_active && st.vseq != last_vseq) {
                if (!last_vseq) LOG("game hooks: first vibration from %s (handle 0x%08x)\n", cur, (uint32_t)st.vhandle);
                last_vseq = st.vseq;
                if (sc2_status_haptics_available()) {
                    sc2_haptic_rumble_set((uint16_t)(st.vlarge << 8), (uint16_t)(st.vsmall << 8));
                    rumbling = st.vlarge || st.vsmall;
                }
            }
            pthread_mutex_lock(&g_lock); g.st = st; pthread_mutex_unlock(&g_lock);
        }
        if (now >= next_stat) next_stat = now + 500;
        usleep(8000);
    }
#ifdef GAME_HOOKS_TEST
    if (g.state == 2) (void)unhook("host test shutdown");
#endif
    return NULL;
}

void game_hooks_start(void) {
    load_settings();
    g.pid = -1;
    pthread_t th;
    int r = pthread_create(&th, NULL, hooks_thread, NULL);
    if (r == 0) {
#ifdef GAME_HOOKS_TEST
        g_test_thread = th;
        g_test_thread_started = 1;
#else
        pthread_detach(th);
#endif
    }
    else LOG("game hooks: worker thread failed (%d)\n", r);
}

#ifdef GAME_HOOKS_TEST
void game_hooks_test_stop(void) {
    atomic_store_explicit(&g_test_stop, 1, memory_order_relaxed);
    if (g_test_thread_started) {
        (void)pthread_join(g_test_thread, NULL);
        g_test_thread_started = 0;
    }
}
#endif

void game_hooks_set_audio(int on) {
    pthread_mutex_lock(&g_lock);
    g_settings.audio_on = on ? 1 : 0;
    pthread_mutex_unlock(&g_lock);
    save_settings();
    LOG("game hooks: DualSense audio haptics/speaker handling %s\n", on ? "on" : "off");
}

void game_hooks_set(int input_on, int vibe_on, int mode) {
    pthread_mutex_lock(&g_lock);
    if (input_on >= 0) g_settings.input_on = input_on ? 1 : 0;
    if (vibe_on  >= 0) g_settings.vibe_on  = vibe_on ? 1 : 0;
    if (mode >= 0 && mode <= 2) g_settings.mode = mode;
    GameHookSettings settings = g_settings;
    pthread_mutex_unlock(&g_lock);
    save_settings();
    LOG("game hooks: input %s, vibration %s\n", settings.input_on ? "on" : "off", settings.vibe_on ? "on" : "off");
}

int game_hooks_json(char *out, size_t n) {
    if (!out || n < 4) return -1;
    pthread_mutex_lock(&g_lock);
    GameHookSettings settings = g_settings;
    int w = snprintf(out, n,
        "{\"audio\":%d,\"hap_ports\":%u,\"spk_ports\":%u,\"a_calls\":%llu,\"a_muted\":%llu,\"hap_l\":%u,\"hap_r\":%u,"
        "\"mode\":%d,\"owner\":%d,\"input\":%d,\"vibration\":%d,\"title\":\"%s\",\"state\":%d,\"reads\":%llu,\"merged\":%llu,"
        "\"vcalls\":%llu,\"large\":%u,\"small\":%u,\"why\":\"",
        settings.audio_on, g.st.n_hap, g.st.n_spk, (unsigned long long)g.st.a_calls, (unsigned long long)g.st.a_muted,
        g.st.hap_l, g.st.hap_r,
        settings.mode, g_owner, settings.input_on, settings.vibe_on, g.title, g.state, (unsigned long long)g.st.in_calls,
        (unsigned long long)g.st.in_merged, (unsigned long long)g.st.vcalls, g.st.vlarge, g.st.vsmall);
    for (const char *p = g.why; *p && w > 0 && (size_t)w < n - 4; p++)
        if (*p != '"' && *p != '\\' && (unsigned char)*p >= 0x20) out[w++] = *p;
    if (w > 0 && (size_t)w < n - 3) { out[w++] = '"'; out[w++] = '}'; out[w] = 0; }
    pthread_mutex_unlock(&g_lock);
    return w;
}
