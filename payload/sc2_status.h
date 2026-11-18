#pragma once

#include <stdint.h>

/* Coherent cross-thread diagnostics for the active Steam Controller path.
 * USB/input workers publish snapshots; the manager and portal read them.
 * No PS5 ABI or controller decoding belongs in this module. */
typedef struct {
    uint32_t live_inputs;
    int live_connected;
    int battery_level;
    int battery_state;
    int battery_warning_level;
    int haptic_available;
    int haptic_method;
    int haptic_out_ep;
    unsigned haptic_queued;
    unsigned haptic_sent;
    unsigned haptic_failed;
    int haptic_last_err;
    int haptic_last_via;
} sc2_status_snapshot_t;

enum {
    SC2_STATUS_BATTERY_CHANGED = 1u << 0,
    SC2_STATUS_BATTERY_WARN_15 = 1u << 1,
    SC2_STATUS_BATTERY_WARN_5 = 1u << 2
};

enum {
    SC2_STATUS_BATT_DISCHARGING = 1,
    SC2_STATUS_BATT_CHARGING,
    SC2_STATUS_BATT_FULL
};

void sc2_status_set_input(uint32_t inputs, int connected);
unsigned sc2_status_update_battery(int level, int state);
void sc2_status_set_haptics(int available, int out_ep);
void sc2_status_set_haptic_method(int method);
int sc2_status_haptic_method(void);
int sc2_status_haptics_available(void);
void sc2_status_haptic_queued(void);
void sc2_status_haptic_result(int via, int error);
void sc2_status_snapshot(sc2_status_snapshot_t *out);
