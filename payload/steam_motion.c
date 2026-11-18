#include "steam_motion.h"

#include <math.h>
#include <string.h>

#define ACCEL_PER_G 16384.0f
#define GYRO_PER_DPS 16.0f
#define DEG2RAD 0.017453292f
#define TILT_GAIN 0.5f

void steam_motion_reset(steam_motion_state_t *state) {
    if (!state) return;
    memset(state, 0, sizeof(*state));
    state->quaternion[0] = 1.0f;
}

int steam_motion_update(steam_motion_state_t *state,
                        const steam_input_report_t *report,
                        const float gyro_bias_raw[3], int64_t now_ms,
                        steam_motion_output_t *out) {
    float bias[3] = {0.0f, 0.0f, 0.0f};
    float ax, ay, az, wx, wy, wz, dt;
    float w, x, y, z, cx, cy, cz, an, h;
    float nw, nx, ny, nz, qn;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!state || !report || !report->has_imu) return 0;
    if (gyro_bias_raw)
        for (int i = 0; i < 3; i++) bias[i] = gyro_bias_raw[i];

    if (!report->accel[0] && !report->accel[1] && !report->accel[2]) return 0;

    /* Match the selected upstream controller axes: X, Z, -Y. */
    ax = report->accel[0] / ACCEL_PER_G;
    ay = report->accel[2] / ACCEL_PER_G;
    az = -report->accel[1] / ACCEL_PER_G;
    wx = (report->gyro[0] - bias[0]) / GYRO_PER_DPS * DEG2RAD;
    wy = (report->gyro[2] - bias[2]) / GYRO_PER_DPS * DEG2RAD;
    wz = -(report->gyro[1] - bias[1]) / GYRO_PER_DPS * DEG2RAD;

    dt = state->last_ms ? (float)(now_ms - state->last_ms) / 1000.0f : 0.0f;
    state->last_ms = now_ms;
    if (dt < 0.0f) dt = 0.0f;
    if (dt > 0.05f) dt = 0.05f;

    w = state->quaternion[0]; x = state->quaternion[1];
    y = state->quaternion[2]; z = state->quaternion[3];
    an = sqrtf(ax * ax + ay * ay + az * az);
    cx = wx; cy = wy; cz = wz;
    if (an > 0.8f && an < 1.2f) {
        float vx = 2.0f * (x * y + w * z);
        float vy = 1.0f - 2.0f * (x * x + z * z);
        float vz = 2.0f * (y * z - w * x);
        float ux = ax / an, uy = ay / an, uz = az / an;
        cx += TILT_GAIN * (uy * vz - uz * vy);
        cy += TILT_GAIN * (uz * vx - ux * vz);
        cz += TILT_GAIN * (ux * vy - uy * vx);
    }

    h = 0.5f * dt;
    nw = w + h * (-x * cx - y * cy - z * cz);
    nx = x + h * ( w * cx + y * cz - z * cy);
    ny = y + h * ( w * cy - x * cz + z * cx);
    nz = z + h * ( w * cz + x * cy - y * cx);
    qn = sqrtf(nw * nw + nx * nx + ny * ny + nz * nz);
    if (qn > 0.0f) {
        state->quaternion[0] = nw / qn;
        state->quaternion[1] = nx / qn;
        state->quaternion[2] = ny / qn;
        state->quaternion[3] = nz / qn;
    }

    out->acceleration_g[0] = ax;
    out->acceleration_g[1] = ay;
    out->acceleration_g[2] = az;
    out->angular_velocity_rad_s[0] = wx;
    out->angular_velocity_rad_s[1] = wy;
    out->angular_velocity_rad_s[2] = wz;
    memcpy(out->quaternion, state->quaternion, sizeof(out->quaternion));
    return 1;
}
