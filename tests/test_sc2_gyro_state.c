#include "sc2_gyro.h"

#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static atomic_ulong checks;

static void make_input(sc2_profile_t *profile, steam_input_report_t *report) {
    memset(profile, 0, sizeof(*profile));
    profile->gyro_mode = GYRO_ALWAYS;
    profile->gyro_sens_x = profile->gyro_sens_y = 40;
    profile->gyro_steady = 2;
    profile->gyro_curve = 10;
    profile->motion = 1;
    memset(report, 0, sizeof(*report));
    report->has_imu = 1;
    report->accel[2] = 16384;
    report->gyro[0] = 240;
    report->gyro[2] = -120;
    report->left_grip_touched = 1;
}

static void *input_worker(void *arg) {
    intptr_t offset = (intptr_t)arg;
    sc2_profile_t profile;
    steam_input_report_t report;
    make_input(&profile, &report);
    for (int i = 0; i < 20000; i++) {
        int dx, dy;
        int64_t now = 1000 + offset + (int64_t)i * 4;
        sc2_gyro_buttons((i % 2) ? PB_ACT_GYRO_ON : 0);
        sc2_gyro_apply(&profile, &report, now, &dx, &dy);
        ScePadData pad;
        memset(&pad, 0, sizeof(pad));
        sc2_gyro_motion(&profile, &report, now, &pad);
        sc2_gyro_set_moving(dx || dy);
        atomic_fetch_add_explicit(&checks, 1, memory_order_relaxed);
    }
    return NULL;
}

static void *reset_worker(void *arg) {
    (void)arg;
    for (int i = 0; i < 5000; i++) {
        sc2_gyro_reset();
        sc2_gyro_buttons((i & 1) ? PB_ACT_GYRO_TOGGLE : 0);
        atomic_fetch_add_explicit(&checks, 1, memory_order_relaxed);
    }
    return NULL;
}

static void *status_worker(void *arg) {
    (void)arg;
    for (int i = 0; i < 30000; i++) {
        sc2_gyro_status_t status;
        sc2_gyro_status(&status);
        if (status.wanted < 0 || status.wanted > 1 || status.imu < 0 || status.imu > 1 ||
            status.active < 0 || status.active > 1 || status.calibrated < 0 || status.calibrated > 1)
            return (void *)(intptr_t)1;
        (void)sc2_gyro_is_moving();
        atomic_fetch_add_explicit(&checks, 1, memory_order_relaxed);
    }
    return NULL;
}

int main(void) {
    pthread_t input[2], reset, status;
    void *result = NULL;
    if (pthread_create(&input[0], NULL, input_worker, (void *)(intptr_t)0) != 0 ||
        pthread_create(&input[1], NULL, input_worker, (void *)(intptr_t)1) != 0 ||
        pthread_create(&reset, NULL, reset_worker, NULL) != 0 ||
        pthread_create(&status, NULL, status_worker, NULL) != 0)
        return 1;
    for (int i = 0; i < 2; i++)
        if (pthread_join(input[i], NULL) != 0) return 1;
    if (pthread_join(reset, NULL) != 0 || pthread_join(status, &result) != 0 || result != NULL)
        return 1;
    printf("test_sc2_gyro_state: concurrent IMU/reset/status access passed (%lu checks)\n",
           atomic_load_explicit(&checks, memory_order_relaxed));
    return 0;
}
