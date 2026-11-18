#include "sc2_control.h"

static int g_pause_active;
static uint64_t g_assignment_owner;
static uint32_t g_assignment_generation;

int sc2_pause_is_active(void) {
    return __atomic_load_n(&g_pause_active, __ATOMIC_ACQUIRE);
}

int sc2_pause_exchange(int active) {
    active = active ? 1 : 0;
    return __atomic_exchange_n(&g_pause_active, active, __ATOMIC_ACQ_REL);
}

sc2_assignment_token_t sc2_assignment_claim(int slot) {
    if (slot < 0 || slot >= 32) return 0;

    uint32_t generation = __atomic_add_fetch(&g_assignment_generation, 1,
                                              __ATOMIC_RELAXED);
    if (generation == 0)
        generation = __atomic_add_fetch(&g_assignment_generation, 1,
                                        __ATOMIC_RELAXED);

    sc2_assignment_token_t token =
        ((uint64_t)generation << 32) | (uint32_t)(slot + 1);
    uint64_t expected = 0;
    if (!__atomic_compare_exchange_n(&g_assignment_owner, &expected, token, 0,
                                     __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
        return 0;
    return token;
}

sc2_assignment_token_t sc2_assignment_owner(void) {
    return __atomic_load_n(&g_assignment_owner, __ATOMIC_ACQUIRE);
}

int sc2_assignment_slot(sc2_assignment_token_t token) {
    uint32_t encoded_slot = (uint32_t)token;
    if (token == 0 || encoded_slot == 0 || encoded_slot > 32) return -1;
    return (int)encoded_slot - 1;
}

int sc2_assignment_release(sc2_assignment_token_t token) {
    if (token == 0) return 0;
    return __atomic_compare_exchange_n(&g_assignment_owner, &token, 0, 0,
                                       __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
}
