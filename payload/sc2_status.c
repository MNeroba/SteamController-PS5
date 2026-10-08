/* Cross-thread Steam Controller status snapshot for manager and portal. */
#include "sc2_status.h"

#include <pthread.h>
#include <string.h>

static pthread_mutex_t g_status_lock = PTHREAD_MUTEX_INITIALIZER;
static sc2_status_snapshot_t g_status = { .battery_level = -1, .haptic_out_ep = -1 };

void sc2_status_set_input(uint32_t inputs, int connected) {
    pthread_mutex_lock(&g_status_lock);
    g_status.live_inputs = inputs;
    g_status.live_connected = connected ? 1 : 0;
    pthread_mutex_unlock(&g_status_lock);
}

unsigned sc2_status_update_battery(int level, int state) {
    unsigned result = 0;
    pthread_mutex_lock(&g_status_lock);
    if (g_status.battery_level < 0 || g_status.battery_state != state)
        result |= SC2_STATUS_BATTERY_CHANGED;
    g_status.battery_level = level;
    g_status.battery_state = state;
    if (level < 0 || state != SC2_STATUS_BATT_DISCHARGING || level > 20) {
        g_status.battery_warning_level = 0;
    } else if (level <= 5 && g_status.battery_warning_level < 2) {
        g_status.battery_warning_level = 2;
        result |= SC2_STATUS_BATTERY_WARN_5;
    } else if (level <= 15 && g_status.battery_warning_level < 1) {
        g_status.battery_warning_level = 1;
        result |= SC2_STATUS_BATTERY_WARN_15;
    }
    pthread_mutex_unlock(&g_status_lock);
    return result;
}

void sc2_status_set_haptics(int available, int out_ep) {
    pthread_mutex_lock(&g_status_lock);
    g_status.haptic_available = available ? 1 : 0;
    g_status.haptic_out_ep = out_ep;
    pthread_mutex_unlock(&g_status_lock);
}

void sc2_status_set_haptic_method(int method) {
    if (method < 0 || method > 4) return;
    pthread_mutex_lock(&g_status_lock);
    g_status.haptic_method = method;
    pthread_mutex_unlock(&g_status_lock);
}

int sc2_status_haptic_method(void) {
    pthread_mutex_lock(&g_status_lock);
    int method = g_status.haptic_method;
    pthread_mutex_unlock(&g_status_lock);
    return method;
}

int sc2_status_haptics_available(void) {
    pthread_mutex_lock(&g_status_lock);
    int available = g_status.haptic_available;
    pthread_mutex_unlock(&g_status_lock);
    return available;
}

void sc2_status_haptic_queued(void) {
    pthread_mutex_lock(&g_status_lock);
    g_status.haptic_queued++;
    pthread_mutex_unlock(&g_status_lock);
}

void sc2_status_haptic_result(int via, int error) {
    pthread_mutex_lock(&g_status_lock);
    g_status.haptic_last_via = via;
    g_status.haptic_last_err = error;
    if (error == 0) g_status.haptic_sent++;
    else g_status.haptic_failed++;
    pthread_mutex_unlock(&g_status_lock);
}

void sc2_status_snapshot(sc2_status_snapshot_t *out) {
    if (!out) return;
    pthread_mutex_lock(&g_status_lock);
    memcpy(out, &g_status, sizeof(*out));
    pthread_mutex_unlock(&g_status_lock);
}
