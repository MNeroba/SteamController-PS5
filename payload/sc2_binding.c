/* sc2_binding.c — Steam Input-style activators for the Steam Controller (2026)
 *
 * Per input:
 *   regular      fires while held (optionally turbo or toggle)
 *   long press   if set: a quick tap sends the regular output as a short pulse,
 *                holding past LONG_MS sends the long output instead
 *   double press if set: regular fires on the first press; a second press
 *                within DOUBLE_MS sends the double output while held
 *   shift layer  while the profile's SHIFT input is held, inputs use their
 *                shift binding (if they have one)
 * Triggers also have a full-pull output from the hardware click.
 */
#include "sc2_binding.h"
#include "gc_types.h"
#include <string.h>

#define TAP_PULSE_MS 90

static uint32_t g_reset_epoch;

static uint32_t reset_epoch(void) {
    return __atomic_load_n(&g_reset_epoch, __ATOMIC_ACQUIRE);
}

static void seed_down_state(sc2_bind_state_t *state, uint32_t in, int64_t now) {
    for (int i = 0; i < SC2_IN_COUNT; i++) {
        state->input[i].down = (uint8_t)((in >> i) & 1u);
        if (state->input[i].down) state->input[i].t_down = now;
    }
}

static int same_bindings(const sc2_profile_t *a, const sc2_profile_t *b) {
    return a->shift_in == b->shift_in &&
           a->long_ms == b->long_ms &&
           a->double_ms == b->double_ms &&
           a->turbo_ms == b->turbo_ms &&
           memcmp(a->map, b->map, sizeof(a->map)) == 0 &&
           memcmp(a->lng, b->lng, sizeof(a->lng)) == 0 &&
           memcmp(a->dbl, b->dbl, sizeof(a->dbl)) == 0 &&
           memcmp(a->shf, b->shf, sizeof(a->shf)) == 0 &&
           memcmp(a->full, b->full, sizeof(a->full)) == 0 &&
           memcmp(a->turbo, b->turbo, sizeof(a->turbo)) == 0 &&
           memcmp(a->toggle, b->toggle, sizeof(a->toggle)) == 0;
}

void sc2_bind_reset(sc2_bind_state_t *state) {
    if (state) {
        memset(state, 0, sizeof(*state));
        state->reset_epoch = reset_epoch();
    }
}

void sc2_bind_invalidate_sessions(void) {
    __atomic_add_fetch(&g_reset_epoch, 1u, __ATOMIC_RELEASE);
}

uint32_t sc2_bind_eval(sc2_bind_state_t *state, const sc2_profile_t *P,
                       uint32_t profile_generation, uint32_t in,
                       int full_l, int full_r, int64_t now) {
    uint32_t out = 0;
    if (!state || !P) return 0;
    state->neutral_report = 0;
    uint32_t current_reset_epoch = reset_epoch();
    if (state->reset_epoch != current_reset_epoch) {
        sc2_bind_reset(state);
        state->profile_snapshot = *P;
        state->profile_generation = profile_generation;
        state->profile_valid = 1;
        state->neutral_report = 1;
        seed_down_state(state, in, now);
        return 0;
    }
    if (!state->profile_valid) {
        state->profile_snapshot = *P;
        state->profile_generation = profile_generation;
        state->profile_valid = 1;
        state->neutral_report = 1;
        seed_down_state(state, in, now);
        return 0;
    } else if (state->profile_generation != profile_generation ||
               !same_bindings(&state->profile_snapshot, P)) {
        /* Force a neutral report at the profile boundary. Preserve physical
         * down state so a held button cannot become a fresh toggle/double tap. */
        sc2_bind_reset(state);
        state->profile_snapshot = *P;
        state->profile_generation = profile_generation;
        state->profile_valid = 1;
        state->neutral_report = 1;
        seed_down_state(state, in, now);
        return 0;
    }
    int shift_in = P->shift_in;
    int shifted = shift_in >= 0 && shift_in < SC2_IN_COUNT && (in & (1u << shift_in));

    for (int i = 0; i < SC2_IN_COUNT; i++) {
        int pressed = (in >> i) & 1;
        int edge_dn = pressed && !state->input[i].down;
        int edge_up = !pressed && state->input[i].down;
        state->input[i].down = (uint8_t)pressed;

        if (i == shift_in) continue;                       /* shift key is a modifier only */

        uint32_t base = (shifted && P->shf[i]) ? P->shf[i] : P->map[i];
        uint32_t lng  = shifted ? 0 : P->lng[i];
        uint32_t dbl  = shifted ? 0 : P->dbl[i];

        if (edge_dn) {
            state->input[i].dbl_active = (uint8_t)(dbl && state->input[i].t_up && now - state->input[i].t_up <= P->double_ms);
            state->input[i].t_down = now;
            state->input[i].long_fired = 0;
            if (P->toggle[i] && !lng && !state->input[i].dbl_active) state->input[i].toggled ^= 1;
        }

        if (pressed) {
            if (state->input[i].dbl_active) {
                out |= dbl;
            } else if (lng) {
                if (now - state->input[i].t_down >= P->long_ms) { out |= lng; state->input[i].long_fired = 1; }
            } else if (P->toggle[i]) {
                /* handled below */
            } else if (P->turbo[i]) {
                int64_t ph = (now - state->input[i].t_down) / (P->turbo_ms ? P->turbo_ms : 60);
                if ((ph & 1) == 0) out |= base;
            } else {
                out |= base;
            }
        }

        if (edge_up) {
            if (lng && !state->input[i].long_fired && !state->input[i].dbl_active) {   /* short tap on a long-press input */
                state->input[i].pulse_mask = base;
                state->input[i].pulse_until = now + TAP_PULSE_MS;
            }
            state->input[i].dbl_active = 0;
            state->input[i].t_up = now;
        }

        if (P->toggle[i] && state->input[i].toggled) out |= base;
        if (state->input[i].pulse_until > now) out |= state->input[i].pulse_mask;
    }

    /* Shift inputs are modifiers only, including a trigger's hardware click. */
    if (full_l && shift_in != IN_LT) out |= P->full[0];
    if (full_r && shift_in != IN_RT) out |= P->full[1];
    return out;
}
