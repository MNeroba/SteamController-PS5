#pragma once
#include <stdint.h>
#include "sc2_profile.h"

typedef struct {
    uint8_t  down, toggled, long_fired, dbl_active;
    int64_t  t_down, t_up;
    uint32_t pulse_mask;
    int64_t  pulse_until;
} sc2_bind_input_state_t;

/* Binding activator state belongs to one physical controller session.
 * The caller owns its lifetime and must not share it between input threads. */
typedef struct {
    sc2_bind_input_state_t input[SC2_IN_COUNT];
    sc2_profile_t profile_snapshot;
    uint32_t profile_generation;
    uint32_t reset_epoch;
    int profile_valid;
    uint8_t neutral_report;
} sc2_bind_state_t;

/* Evaluate Steam Input-style bindings for one report.
 * in: physical inputs (1 << IN_*); full_l/full_r: trigger full-pull clicks.
 * state is private to one physical controller session. Returns the output
 * mask (DualSense buttons plus PB_ACT_* bits). neutral_report is set when
 * state is initialized or invalidated so the adapter can publish one fully
 * neutral pad report before applying the new profile. Held inputs are seeded
 * during that frame so they do not become synthetic press/toggle edges. */
uint32_t sc2_bind_eval(sc2_bind_state_t *state, const sc2_profile_t *P,
                       uint32_t profile_generation, uint32_t in,
                       int full_l, int full_r, int64_t now_ms);

/* Clear latched state (toggles, pending taps) — on profile change or menu. */
void sc2_bind_reset(sc2_bind_state_t *state);
/* Request an owner-thread reset of every session context. Safe from portal
 * or notification threads: the generation is atomic and each owner applies it. */
void sc2_bind_invalidate_sessions(void);
