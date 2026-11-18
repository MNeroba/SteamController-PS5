#pragma once
#include <stdint.h>
#include "sc2_profile.h"

/* Flick stick: the right stick points where you want to face. Pushing it to
 * the edge turns the camera by the stick's angle (up = no turn, right = 90°
 * right, down = 180°), and rotating it around the edge keeps turning by the
 * same angle. The game only takes stick input, so a turn is sent as full
 * left/right stick for angle ÷ the game's full-stick turn speed.
 *
 * Runtime state belongs to one physical controller session. The caller owns
 * it and must not share it between input threads. */
typedef struct {
    int edge;
    float last_ang;
    float pending;       /* degrees, + = right */
    int64_t last_ms;
    float frame_s;
} sc2_flick_state_t;

/* Reset before the first report and on session disconnect/reset. */
void sc2_flick_reset(sc2_flick_state_t *state);

/* Returns the right stick's horizontal deflection (-127..127) for this
 * report; vertical aim is left to the gyro. A null state/profile is neutral. */
int sc2_flick_apply(sc2_flick_state_t *state, const sc2_profile_t *P,
                    int16_t rx, int16_t ry, int64_t now_ms);
