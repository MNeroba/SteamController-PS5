/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once

#include <stdint.h>

typedef enum {
    GAME_HOOK_SLOT_INVALID = -1,
    GAME_HOOK_SLOT_ALREADY_RESTORED = 0,
    GAME_HOOK_SLOT_RESTORE = 1,
    GAME_HOOK_SLOT_CONFLICT = 2
} game_hook_slot_action_t;

/* Count consumer callbacks since the producer last advanced its heartbeat.
 * The age saturates so long-lived hooks cannot wrap back into a fresh lease. */
static __attribute__((always_inline)) inline uint32_t
game_hook_heartbeat_age(volatile uint32_t *heartbeat,
                        volatile uint32_t *seen_heartbeat,
                        volatile uint32_t *age, uint32_t stale_limit)
{
    if (!heartbeat || !seen_heartbeat || !age || stale_limit == UINT32_MAX)
        return UINT32_MAX;
    uint32_t current = __atomic_load_n(heartbeat, __ATOMIC_ACQUIRE);
    uint32_t seen = __atomic_exchange_n(seen_heartbeat, current,
                                       __ATOMIC_ACQ_REL);
    if (current != seen) {
        __atomic_store_n(age, 0u, __ATOMIC_RELEASE);
        return 0u;
    }

    uint32_t value = __atomic_load_n(age, __ATOMIC_RELAXED);
    while (value <= stale_limit) {
        uint32_t next = value + 1u;
        if (__atomic_compare_exchange_n(age, &value, next, 0,
                                        __ATOMIC_RELAXED, __ATOMIC_RELAXED))
            return next;
    }
    return value;
}

/* Decide whether a slot still belongs to this hook before restoring it. */
static inline game_hook_slot_action_t
game_hook_slot_restore_action(uint64_t observed, uint64_t gateway,
                              uint64_t original)
{
    if (gateway == 0 || original == 0 || gateway == original)
        return GAME_HOOK_SLOT_INVALID;
    if (observed == original)
        return GAME_HOOK_SLOT_ALREADY_RESTORED;
    if (observed != gateway)
        return GAME_HOOK_SLOT_CONFLICT;
    return GAME_HOOK_SLOT_RESTORE;
}

/* Shared callbacks must stop muting audio after the payload heartbeat expires. */
static inline int
game_hook_audio_lease_active(uint32_t requested, uint32_t stale_calls,
                             uint32_t stale_limit)
{
    return requested != 0 && stale_calls <= stale_limit;
}
