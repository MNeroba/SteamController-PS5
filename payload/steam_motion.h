#ifndef STEAM_MOTION_H
#define STEAM_MOTION_H

#include <stdint.h>
#include "steam_protocol.h"

typedef struct {
    float quaternion[4]; /* w, x, y, z; body-to-world orientation */
    int64_t last_ms;
} steam_motion_state_t;

typedef struct {
    float acceleration_g[3];
    float angular_velocity_rad_s[3];
    float quaternion[4];
} steam_motion_output_t;

void steam_motion_reset(steam_motion_state_t *state);

/* Converts the decoded Steam IMU into the DualSense-oriented axes used by
 * Puckbridge. Bias is expressed in raw gyro counts. Returns 1 on a valid IMU
 * sample and 0 when no usable sample is present. */
int steam_motion_update(steam_motion_state_t *state,
                        const steam_input_report_t *report,
                        const float gyro_bias_raw[3], int64_t now_ms,
                        steam_motion_output_t *out);

#endif
