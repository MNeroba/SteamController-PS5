#include "steam_motion.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

static int failures;
static int checks;

#define CHECK(cond) do { \
    checks++; \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        failures++; \
    } \
} while (0)

#define NEAR(a, b, eps) (fabsf((float)(a) - (float)(b)) <= (eps))

static void test_units_and_axis_mapping(void) {
    steam_motion_state_t state;
    steam_motion_output_t out;
    steam_input_report_t report;
    const float bias[3] = {0};
    memset(&report, 0, sizeof(report));
    report.has_imu = 1;
    report.accel[0] = 16384;
    report.accel[1] = -8192;
    report.accel[2] = 4096;
    report.gyro[0] = 16;
    report.gyro[1] = 32;
    report.gyro[2] = 48;
    steam_motion_reset(&state);
    CHECK(steam_motion_update(&state, &report, bias, 1000, &out) == 1);
    CHECK(NEAR(out.acceleration_g[0], 1.0f, 0.0001f));
    CHECK(NEAR(out.acceleration_g[1], 0.25f, 0.0001f));
    CHECK(NEAR(out.acceleration_g[2], 0.5f, 0.0001f));
    CHECK(NEAR(out.angular_velocity_rad_s[0], 0.017453292f, 0.00001f));
    CHECK(NEAR(out.angular_velocity_rad_s[1], 0.052359876f, 0.00001f));
    CHECK(NEAR(out.angular_velocity_rad_s[2], -0.034906584f, 0.00001f));
    CHECK(out.quaternion[0] == 1.0f && out.quaternion[1] == 0.0f);
}

static void test_bias_and_rotation_limits(void) {
    steam_motion_state_t state;
    steam_motion_output_t out;
    steam_input_report_t report;
    const float bias[3] = {16.0f, 32.0f, 48.0f};
    memset(&report, 0, sizeof(report));
    report.has_imu = 1;
    report.accel[0] = 32767; /* above 1 g: suppress gravity correction */
    report.gyro[0] = 16;
    report.gyro[1] = 32;
    report.gyro[2] = 48;
    steam_motion_reset(&state);
    CHECK(steam_motion_update(&state, &report, bias, 100, &out) == 1);
    CHECK(out.angular_velocity_rad_s[0] == 0.0f);
    CHECK(out.angular_velocity_rad_s[1] == 0.0f);
    CHECK(out.angular_velocity_rad_s[2] == 0.0f);

    const float zero_bias[3] = {0};
    steam_motion_reset(&state);
    CHECK(steam_motion_update(&state, &report, zero_bias, 100, &out) == 1);
    CHECK(steam_motion_update(&state, &report, zero_bias, 1100, &out) == 1);
    CHECK(out.quaternion[1] > 0.0003f && out.quaternion[1] < 0.0006f);
    float norm = sqrtf(out.quaternion[0] * out.quaternion[0] +
                       out.quaternion[1] * out.quaternion[1] +
                       out.quaternion[2] * out.quaternion[2] +
                       out.quaternion[3] * out.quaternion[3]);
    CHECK(NEAR(norm, 1.0f, 0.00001f));
    float before = out.quaternion[1];
    CHECK(steam_motion_update(&state, &report, zero_bias, 900, &out) == 1);
    CHECK(NEAR(out.quaternion[1], before, 0.000001f)); /* negative dt is clamped */
}

static void test_missing_imu_and_reset(void) {
    steam_motion_state_t state;
    steam_motion_output_t out;
    steam_input_report_t report;
    const float bias[3] = {0};
    memset(&report, 0, sizeof(report));
    steam_motion_reset(&state);
    memset(&out, 0x7f, sizeof(out));
    CHECK(steam_motion_update(&state, &report, bias, 1, &out) == 0);
    CHECK(out.acceleration_g[0] == 0.0f && out.quaternion[0] == 0.0f);
    report.has_imu = 1;
    CHECK(steam_motion_update(&state, &report, bias, 2, &out) == 0);
    steam_motion_reset(&state);
    CHECK(state.quaternion[0] == 1.0f && state.quaternion[1] == 0.0f);
    CHECK(state.last_ms == 0);
    CHECK(steam_motion_update(NULL, &report, bias, 3, &out) == 0);
    CHECK(steam_motion_update(&state, &report, bias, 3, NULL) == 0);
}

int main(void) {
    test_units_and_axis_mapping();
    test_bias_and_rotation_limits();
    test_missing_imu_and_reset();
    if (failures) {
        fprintf(stderr, "%d of %d Steam motion checks failed\n", failures, checks);
        return 1;
    }
    printf("Steam motion checks passed (%d checks)\n", checks);
    return 0;
}
