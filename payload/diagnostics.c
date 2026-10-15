/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "diagnostics.h"
#include <ctype.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#ifdef __PROSPERO__
#include "build_info.h"
#else
#define STEAMCONTROLLER_SOURCE_SHA "unknown (host fixture)"
#endif
#define LOG_LINES 48
#define LOG_LINE_MAX 192

typedef struct {
    uint32_t generation;
    char path[32], stage[48], first_failure_stage[48];
    int first_failure;
    uint16_t vid, pid;
    int iface, in_ep, out_ep, error, watchdog;
    uint32_t last_length, valid_inputs, invalid_inputs;
    uint8_t last_id;
    uint64_t last_ms, starts, completions, late, failures, timeouts, protocol;
    slot_registry_entry_t slot;
    user_session_snapshot_t users;
    slot_recovery_t recovery;
} diagnostic_usb_t;
typedef struct {
    char startup[64];
    int startup_error, webui;
    storage_status_t storage;
    uint32_t firmware;
    runtime_policy_t policy;
    user_session_snapshot_t users;
    unsigned usb_sessions, vda_creates, vda_deletes, vda_create_failures, vda_delete_failures;
    int vda_create_result, vda_delete_result;
    vda_result_t vda_observation;
    vda_pending_summary_t pending;
    diagnostic_usb_t usb[SLOT_REGISTRY_MAX_SLOTS];
    char logs[LOG_LINES][LOG_LINE_MAX];
    unsigned log_head, log_count;
} diagnostic_state_t;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static diagnostic_state_t state = {.startup = "not initialized", .webui = -1};
static void copy(char *out, size_t n, const char *in) {
    snprintf(out, n, "%s", in ? in : "unavailable");
}
static int contains(const char *s, const char *word) {
    size_t n = strlen(word);
    for (size_t i = 0; s[i]; i++) {
        size_t j = 0;
        while (j < n && s[i + j] && tolower((unsigned char)s[i + j]) == word[j])
            j++;
        if (j == n)
            return 1;
    }
    return 0;
}
void diagnostics_sanitize_raw_log(char *out, size_t capacity, const char *line) {
    static const char *const private_fields[]={
        "userid", "user_id", "user id", "user-id", "fguser", "inject_uid",
        "foreground_user", "initial_user", "login_user", "user=", "user:", "user 0x",
        "uid=", "uid:", "accountid", "account_id",
        "authorization", "bearer ", "pairing code:", "serial=", "serial:",
        "bondingkey", "webui.token=", "webui.token:", NULL
    };
    if (!capacity || !out) return;
    if (!line) { out[0]=0; return; }
    /* Drop the whole identity-bearing event, including decimal/hex copies.
     * snprintf cannot safely copy overlapping buffers. */
    for (unsigned i=0;private_fields[i];i++) if (contains(line,private_fields[i])) {
        copy(out,capacity,"[GC] [redacted private event]\n"); return;
    }
    size_t n=0;while(n+1<capacity && line[n])n++;
    memmove(out,line,n);out[n]=0;
}
void diagnostics_storage(const storage_status_t *status) {
    pthread_mutex_lock(&lock);
    state.storage=status ? *status : (storage_status_t){STORAGE_OK,0};
    pthread_mutex_unlock(&lock);
}
void diagnostics_vda_pending(vda_pending_summary_t observation) {
    pthread_mutex_lock(&lock);state.pending=observation;pthread_mutex_unlock(&lock);
}
void diagnostics_vda_observation(vda_result_t result) {
    pthread_mutex_lock(&lock);
    state.vda_observation=result;
    pthread_mutex_unlock(&lock);
}
/* Omit sensitive/context-untrusted lines rather than trying to retain a secret
 * value. Escape all controls; also scrub long numeric/hex and address literals. */
static void filtered(char *out, size_t capacity, const char *in) {
    static const char *const sensitive[] = {
        "authorization", "bearer", "token",      "credential", "password", "secret",
        "pair",          "serial", "bond",       "mac=",       "mac:",     "deviceid",
        "userid",        "fguser", "inject_uid", "klog:",      NULL};
    if (!capacity)
        return;
    if (!in) {
        copy(out, capacity, "unavailable");
        return;
    }
    char bounded[4097];
    size_t input_length=0;
    while(input_length<sizeof(bounded)-1 && in[input_length])input_length++;
    memcpy(bounded,in,input_length);bounded[input_length]=0;in=bounded;
    for (int i = 0; sensitive[i]; i++)
        if (contains(in, sensitive[i])) {
            copy(out, capacity, "[redacted sensitive application line]");
            return;
        }
    size_t used = 0, i = 0;
    while (in[i] && i < 4096 && used + 1 < capacity) {
        size_t run = 0, digits = 0;
        while (run < 4096 - i && isxdigit((unsigned char)in[i + run]))
            run++;
        while (digits < 4096 - i && isdigit((unsigned char)in[i + digits]))
            digits++;
        if (run >= 12 || digits >= 8) {
            const char *r = "[redacted]";
            size_t n = strlen(r);
            if (used + n >= capacity)
                break;
            memcpy(out + used, r, n);
            used += n;
            i += run >= 12 ? run : digits;
            continue;
        }
        /* Colon-separated hardware addresses are never needed in a USB report. */
        if (run == 2 && in[i + run] == ':' && isxdigit((unsigned char)in[i + 3])) {
            const char *r = "[redacted address]";
            size_t n = strlen(r);
            if (used + n >= capacity)
                break;
            memcpy(out + used, r, n);
            used += n;
            while (in[i] && (isxdigit((unsigned char)in[i]) || in[i] == ':'))
                i++;
            continue;
        }
        unsigned char c = (unsigned char)in[i++];
        if (c < 32 || c == 127) {
            if (used + 4 >= capacity)
                break;
            snprintf(out + used, 5, "\\x%02x", c);
            used += 4;
        } else
            out[used++] = (char)c;
    }
    out[used] = 0;
}
void diagnostics_runtime(const char *stage, int error, uint32_t fw,
                         const runtime_policy_t *policy) {
    pthread_mutex_lock(&lock);
    copy(state.startup, sizeof(state.startup), stage);
    state.startup_error = error;
    state.firmware = fw;
    if (policy)
        state.policy = *policy;
    pthread_mutex_unlock(&lock);
}
void diagnostics_startup(const char *stage, int error) {
    pthread_mutex_lock(&lock);
    copy(state.startup, sizeof(state.startup), stage);
    state.startup_error = error;
    pthread_mutex_unlock(&lock);
}
void diagnostics_webui(int status) {
    pthread_mutex_lock(&lock);
    state.webui = status;
    pthread_mutex_unlock(&lock);
}
void diagnostics_vda_result(int creation, int result) {
    pthread_mutex_lock(&lock);
    if (creation) {
        state.vda_creates++;
        state.vda_create_result = result;
        if (result <= 0 && (uint32_t)result!=UINT32_C(0x803b0006))
            state.vda_create_failures++;
    } else {
        state.vda_deletes++;
        state.vda_delete_result = result;
        if (result)
            state.vda_delete_failures++;
    }
    pthread_mutex_unlock(&lock);
}
void diagnostics_users(const user_session_snapshot_t *users) {
    if (!users)
        return;
    pthread_mutex_lock(&lock);
    state.users = *users;
    pthread_mutex_unlock(&lock);
}
void diagnostics_log(const char *line, uint64_t ms) {
    char clean[LOG_LINE_MAX - 32];
    filtered(clean, sizeof(clean), line);
    pthread_mutex_lock(&lock);
    snprintf(state.logs[state.log_head], LOG_LINE_MAX, "[+%llu ms] %s", (unsigned long long)ms,
             clean);
    state.log_head = (state.log_head + 1) % LOG_LINES;
    if (state.log_count < LOG_LINES)
        state.log_count++;
    pthread_mutex_unlock(&lock);
}
static diagnostic_usb_t *find(slot_registry_token_t t) {
    if (t.index >= SLOT_REGISTRY_MAX_SLOTS || !t.generation ||
        state.usb[t.index].generation != t.generation)
        return NULL;
    return &state.usb[t.index];
}
void diagnostics_usb_begin(slot_registry_token_t t, const char *path, uint16_t vid, uint16_t pid) {
    if (t.index >= SLOT_REGISTRY_MAX_SLOTS || !t.generation)
        return;
    pthread_mutex_lock(&lock);
    diagnostic_usb_t *u = &state.usb[t.index];
    memset(u, 0, sizeof(*u));
    u->generation = t.generation;
    u->vid = vid;
    u->pid = pid;
    u->iface = u->in_ep = u->out_ep = -1;
    copy(u->path, sizeof(u->path), path);
    copy(u->stage, sizeof(u->stage), "detected");
    state.usb_sessions++;
    pthread_mutex_unlock(&lock);
}
void diagnostics_usb_stage(slot_registry_token_t t, const char *stage, int error) {
    pthread_mutex_lock(&lock);
    diagnostic_usb_t *u = find(t);
    if (u) {
        if (stage)
            copy(u->stage, sizeof(u->stage), stage);
        u->error = error;
        if (error && !u->first_failure) {
            u->first_failure = error;
            copy(u->first_failure_stage, sizeof(u->first_failure_stage), u->stage);
        }
    }
    pthread_mutex_unlock(&lock);
}
void diagnostics_usb_endpoints(slot_registry_token_t t, int iface, int in, int out) {
    pthread_mutex_lock(&lock);
    diagnostic_usb_t *u = find(t);
    if (u) {
        u->iface = iface;
        u->in_ep = in;
        u->out_ep = out;
    }
    pthread_mutex_unlock(&lock);
}
void diagnostics_usb_input(slot_registry_token_t t, uint32_t len, uint8_t id, uint64_t now,
                           int valid) {
    pthread_mutex_lock(&lock);
    diagnostic_usb_t *u = find(t);
    if (u) {
        if (valid) {
            u->last_length = len;
            u->last_id = id;
            u->last_ms = now;
            u->valid_inputs++;
        } else
            u->invalid_inputs++;
    }
    pthread_mutex_unlock(&lock);
}
void diagnostics_usb_snapshot(slot_registry_token_t t, const usb_resources_t *r, int watchdog) {
    if (!r)
        return;
    pthread_mutex_lock(&lock);
    diagnostic_usb_t *u = find(t);
    if (u) {
        u->starts = r->starts;
        u->completions = r->completions;
        u->late = r->late_completions;
        u->failures = r->failures;
        u->timeouts = r->timeouts;
        u->protocol = r->protocol_errors;
        u->watchdog = watchdog;
    }
    pthread_mutex_unlock(&lock);
}
void diagnostics_virtual(slot_registry_token_t t, const slot_registry_entry_t *e,
                         const user_session_snapshot_t *users, const slot_recovery_t *recovery) {
    pthread_mutex_lock(&lock);
    diagnostic_usb_t *u = find(t);
    if (u) {
        if (e)
            u->slot = *e;
        if (users)
            u->users = *users;
        if (recovery)
            u->recovery = *recovery;
    }
    pthread_mutex_unlock(&lock);
}

typedef struct {
    char *out;
    size_t cap, used;
    int failed;
} writer_t;
static void add(writer_t *w, const char *fmt, ...) {
    if (w->failed)
        return;
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(w->out + w->used, w->cap - w->used, fmt, ap);
    va_end(ap);
    if (n < 0 || (size_t)n >= w->cap - w->used) {
        w->failed = 1;
        return;
    }
    w->used += (size_t)n;
}
static void field(writer_t *w, const char *label, const char *value) {
    char clean[512];
    filtered(clean, sizeof(clean), value);
    add(w, "%s: %s\n", label, clean);
}
int diagnostics_report(char *out, size_t cap, const diagnostics_steam_t *steam) {
    if (!out || !steam || cap < 1)
        return -1;
    if (cap > DIAGNOSTICS_REPORT_MAX)
        cap = DIAGNOSTICS_REPORT_MAX;
    diagnostic_state_t snap;
    pthread_mutex_lock(&lock);
    snap = state;
    pthread_mutex_unlock(&lock);
    writer_t w = {.out = out, .cap = cap};
    add(&w, "SteamController-PS5 diagnostic report\nVersion: %s\nSource SHA: %s\nPayload SDK: %s\n",
        STEAMCONTROLLER_VERSION, STEAMCONTROLLER_SOURCE_SHA, STEAMCONTROLLER_SDK_VERSION);
    if (snap.firmware)
        add(&w, "Firmware (runtime): %x.%02x (0x%08x)\n", (snap.firmware >> 24) & 255,
            (snap.firmware >> 16) & 255, snap.firmware);
    else
        add(&w, "Firmware (runtime): unavailable; intended firmware is not a measurement\n");
    add(&w, "Experimental development build; no physical PS5 verification.\nReview before sharing "
            "publicly: profile names and USB paths may identify your setup.\nNo pairing "
            "credentials, serials or unrelated system files are included.\n\n--- Startup ---\n");
    field(&w, "Stage", snap.startup);
    add(&w,
        "Startup result: %d (0x%08x)\nWebUI status: %d (2=running with session-only authentication, 1=running, 0=stopped/disabled, "
        "-1=unavailable)\n",
        snap.startup_error, (unsigned)snap.startup_error, snap.webui);
    add(&w,"WebUI storage stage: %s; errno=%d\n",storage_stage_name(snap.storage.stage),snap.storage.error);
    add(&w,
        "Policy: safe=%d webui=%d hooks=%d bridge=%d handoff=%d\nUSB session starts: %u (includes "
        "rediscovery; not a physical reconnect measurement)\n",
        !(snap.policy.enable_game_hooks || snap.policy.enable_bridge_probe ||
          snap.policy.enable_ds_handoff),
        snap.policy.enable_webui, snap.policy.enable_game_hooks, snap.policy.enable_bridge_probe,
        snap.policy.enable_ds_handoff, snap.usb_sessions);
    add(&w,
        "User session: valid=%d signed-in=%d foreground-member=%d errors=%u confirmed-logout=%d "
        "epoch=%u\n",
        snap.users.read_ok, snap.users.logged_users, snap.users.foreground_valid,
        snap.users.read_errors, snap.users.logged_out, snap.users.logout_epoch);
    add(&w,
        "VDA operations: create-attempts=%u failures=%u last=0x%08x; delete-attempts=%u "
        "failures=%u last=0x%08x\nPositive Add return ownership is inherited and requires physical "
        "validation.\n",
        snap.vda_creates, snap.vda_create_failures, (unsigned)snap.vda_create_result,
        snap.vda_deletes, snap.vda_delete_failures, (unsigned)snap.vda_delete_result);
    add(&w,"VDA pending observation: %s; events=%u distinct=%u malformed=%u truncated=%u elapsed_ms=%llu errno=%d\n",
        vda_pending_name(snap.pending.status),snap.pending.virtual_events,snap.pending.distinct_virtuals,
        snap.pending.malformed,snap.pending.truncated,(unsigned long long)snap.pending.elapsed_ms,snap.pending.error);
    if(snap.vda_creates) add(&w,"VDA outcome: %s; changed output-word mask=0x%02x "
        "(not handle evidence)\n",vda_result_name(snap.vda_observation.kind),snap.vda_observation.changed_words);
    add(&w, "\n--- USB and virtual controller ---\n");
    int devices = 0;
    for (unsigned i = 0; i < SLOT_REGISTRY_MAX_SLOTS; i++) {
        diagnostic_usb_t *u = &snap.usb[i];
        if (!u->generation)
            continue;
        devices++;
        add(&w, "Slot %u generation %u: VID/PID=%04x:%04x transport=%s\n", i, u->generation, u->vid,
            u->pid,
            u->vid == 0x28de && u->pid == 0x1304   ? "Valve Puck"
            : u->vid == 0x28de && u->pid == 0x1302 ? "USB-C"
                                                   : "unknown");
        field(&w, "USB device path", u->path);
        field(&w, "USB initialization stage", u->stage);
        add(&w,
            "Stage result: %d (0x%08x)\nValidated interface/IN/OUT: %d/%d/%d "
            "(-1=unavailable)\nLast valid input: id=0x%02x length=%u at +%llu ms; valid=%u "
            "invalid=%u\n",
            u->error, (unsigned)u->error, u->iface, u->in_ep, u->out_ep, u->last_id, u->last_length,
            (unsigned long long)u->last_ms, u->valid_inputs, u->invalid_inputs);
        if (u->first_failure) {
            field(&w, "First failure stage", u->first_failure_stage);
            add(&w, "First failure: %d (0x%08x)\n", u->first_failure, (unsigned)u->first_failure);
        }
        add(&w,
            "Transfers: starts=%llu completed=%llu late-cancelled=%llu failures=%llu timeouts=%llu "
            "protocol=%llu\nWatchdog expired: %d\n",
            (unsigned long long)u->starts, (unsigned long long)u->completions,
            (unsigned long long)u->late, (unsigned long long)u->failures,
            (unsigned long long)u->timeouts, (unsigned long long)u->protocol, u->watchdog);
        add(&w,
            "Owned virtual pad: ready=%d creating=%d deleting=%d ambiguous=%d retiring=%d event-correlated=%d "
            "slot-state=%d\nBinding: console profile picker; account binding NOT confirmed by "
            "button activity\nUser observation: valid=%d signed-in=%d foreground-member=%d "
            "errors=%u logout-epoch=%u logged-out=%d\nRecovery: pending=%d delete-attempts=%u "
            "delete-failures=%u registrations=%u\n",
            u->slot.virtual_ready, u->slot.virtual_creating, u->slot.virtual_deleting,
            u->slot.virtual_blocked, u->slot.virtual_retiring, u->slot.virtual_correlated, u->slot.state, u->users.read_ok, u->users.logged_users,
            u->users.foreground_valid, u->users.read_errors, u->users.logout_epoch,
            u->users.logged_out, u->recovery.pending, u->recovery.attempts,
            u->recovery.deletion_failures, u->recovery.recovered);
    }
    if (!devices)
        add(&w, "No Steam Controller detected; USB initialization has not started.\n");
    add(&w, "\n--- Steam features ---\n");
    field(&w, "Profile", steam->profile);
    add(&w,
        "Connected=%d paused=%d active-input-mask=0x%08x mapped-inputs=%d shift=%d\nGyro: "
        "wanted=%d IMU=%d active=%d calibrated=%d\nTrackpad modes: left=%d right=%d\nHaptics: "
        "available=%d OUT=%d queued=%u sent=%u failed=%u last-error=%d via=%d\n",
        steam->steam.live_connected, steam->paused, steam->steam.live_inputs, steam->mappings,
        steam->shift, steam->gyro_wanted, steam->gyro_imu, steam->gyro_active,
        steam->gyro_calibrated, steam->left_trackpad_mode, steam->right_trackpad_mode,
        steam->steam.haptic_available, steam->steam.haptic_out_ep, steam->steam.haptic_queued,
        steam->steam.haptic_sent, steam->steam.haptic_failed, steam->steam.haptic_last_err,
        steam->steam.haptic_last_via);
    add(&w,
        "\n--- Latest bounded application log ---\nLatest %u/%d memory records; older records and "
        "sensitive lines omitted. Timestamps are monotonic.\n",
        snap.log_count, LOG_LINES);
    if (!snap.log_count)
        add(&w, "Application log unavailable or not created yet. No filesystem was read.\n");
    for (unsigned i = 0; i < snap.log_count; i++) {
        unsigned index = (snap.log_head + LOG_LINES - snap.log_count + i) % LOG_LINES;
        add(&w, "%s\n", snap.logs[index]);
    }
    if (w.failed) {
        out[0] = 0;
        return -1;
    }
    return (int)w.used;
}
