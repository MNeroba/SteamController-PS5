#include "sc2_status.h"

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define ITERATIONS 25000

static int failed;
static unsigned long long checks;

static void fail(void) {
    __atomic_store_n(&failed, 1, __ATOMIC_RELAXED);
}

static void *writer(void *arg) {
    intptr_t id = (intptr_t)arg;
    for (int i = 0; i < ITERATIONS; i++) {
        uint32_t value = (uint32_t)(i * 4 + (int)id);
        int level = i % 101;
        int available = i & 1;
        sc2_status_set_input(value, (int)(value & 1u));
        (void)sc2_status_update_battery(level, level % 3 + 1);
        sc2_status_set_haptics(available, available ? 3 : -1);
        sc2_status_set_haptic_method(i % 5);
        sc2_status_haptic_queued();
        sc2_status_haptic_result(i % 5, (i % 7) ? 0 : -1);
    }
    return NULL;
}

static void *reader(void *arg) {
    (void)arg;
    for (int i = 0; i < ITERATIONS; i++) {
        sc2_status_snapshot_t status;
        sc2_status_snapshot(&status);
        if (status.live_connected != (int)(status.live_inputs & 1u) ||
            status.battery_level < -1 || status.battery_level > 100 ||
            (status.battery_level >= 0 &&
             (status.battery_state < 1 || status.battery_state > 3 ||
              status.battery_state != status.battery_level % 3 + 1)) ||
            status.haptic_method < 0 || status.haptic_method > 4 ||
            status.haptic_available != (status.haptic_out_ep == 3))
            fail();
        __atomic_add_fetch(&checks, 1, __ATOMIC_RELAXED);
    }
    return NULL;
}

int main(void) {
    pthread_t writers[4], readers[4];
    sc2_status_snapshot_t initial;
    sc2_status_snapshot(&initial);
    if (initial.battery_level != -1 || initial.haptic_out_ep != -1) fail();
    if (sc2_status_update_battery(15, SC2_STATUS_BATT_DISCHARGING) !=
        (SC2_STATUS_BATTERY_CHANGED | SC2_STATUS_BATTERY_WARN_15)) fail();
    if (sc2_status_update_battery(10, SC2_STATUS_BATT_DISCHARGING) != 0) fail();
    if (sc2_status_update_battery(5, SC2_STATUS_BATT_DISCHARGING) !=
        SC2_STATUS_BATTERY_WARN_5) fail();
    (void)sc2_status_update_battery(25, SC2_STATUS_BATT_DISCHARGING);
    if (sc2_status_update_battery(15, SC2_STATUS_BATT_DISCHARGING) !=
        SC2_STATUS_BATTERY_WARN_15) fail();
    sc2_status_set_haptic_method(-1);
    sc2_status_set_haptic_method(5);
    if (sc2_status_haptic_method() != 0) fail();
    for (intptr_t i = 0; i < 4; i++) {
        if (pthread_create(&writers[i], NULL, writer, (void *)i) != 0 ||
            pthread_create(&readers[i], NULL, reader, (void *)i) != 0)
            return 1;
    }
    for (int i = 0; i < 4; i++)
        if (pthread_join(writers[i], NULL) != 0 ||
            pthread_join(readers[i], NULL) != 0)
            return 1;
    if (__atomic_load_n(&failed, __ATOMIC_RELAXED)) return 1;
    printf("test_sc2_status: 8 concurrent workers passed (%llu coherent snapshots)\n",
           __atomic_load_n(&checks, __ATOMIC_RELAXED));
    return 0;
}
