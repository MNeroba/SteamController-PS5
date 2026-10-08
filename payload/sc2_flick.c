/* sc2_flick.c — flick stick (see sc2_flick.h) */

#include "sc2_flick.h"
#include <math.h>

#define FLICK_START   0.90f       /* push past this to flick */
#define FLICK_HOLD    0.80f       /* stay at the edge above this */
#define PENDING_MAX   720.0f      /* degrees still to turn, at most */

void sc2_flick_reset(sc2_flick_state_t *state) {
    if (!state) return;
    state->edge = 0;
    state->last_ang = 0;
    state->pending = 0;
    state->last_ms = 0;
    state->frame_s = 0.004f;
}

static float wrap180(float a) {
    while (a > 180.0f)   a -= 360.0f;
    while (a <= -180.0f) a += 360.0f;
    return a;
}

int sc2_flick_apply(sc2_flick_state_t *state, const sc2_profile_t *P,
                    int16_t rx, int16_t ry, int64_t now) {
    if (!state || !P) return 0;

    /* how long this report's output will stand: the last report interval */
    if (state->last_ms) {
        float d = (float)(now - state->last_ms) / 1000.0f;
        if (d >= 0.001f && d <= 0.02f)
            state->frame_s += (d - state->frame_s) * 0.1f;
    }
    state->last_ms = now;

    float x = rx / 32767.0f, y = ry / 32767.0f;      /* +y = up */
    float mag = sqrtf(x * x + y * y);
    float ang = atan2f(x, y) * 57.29578f;           /* 0 = up, + = right */
    if (!state->edge && mag >= FLICK_START) {
        state->edge = 1;
        state->pending += ang;                      /* face where the stick points */
    } else if (state->edge && mag >= FLICK_HOLD) {
        state->pending += wrap180(ang - state->last_ang); /* rotate around edge */
    } else if (mag < FLICK_HOLD) {
        state->edge = 0;
    }
    state->last_ang = ang;
    if (state->pending >  PENDING_MAX) state->pending =  PENDING_MAX;
    if (state->pending < -PENDING_MAX) state->pending = -PENDING_MAX;

    /* send it as full stick, one report at a time, until it's all turned.
     * Each report turns a whole step; what's left over (under half a step,
     * either way) is kept, so slow rotation adds up exactly. */
    float step = (P->flick_speed ? P->flick_speed : 360) * state->frame_s;
    if (state->pending >=  step * 0.5f) { state->pending -= step; return 127; }
    if (state->pending <= -step * 0.5f) { state->pending += step; return -127; }
    return 0;
}
