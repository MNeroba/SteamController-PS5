/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once

#include <stdint.h>

#define GAME_HOOK_SHUTDOWN_ROLLBACK_ATTEMPTS 5u

/* Bound the number of remote rollback attempts when the target refuses writes.
 * An individual target call can still have target-dependent latency. Gateway
 * memory stays mapped; input/audio leases provide fallback after producer exit. */
static inline int
game_hook_shutdown_retry(unsigned failed_attempts)
{
    return failed_attempts < GAME_HOOK_SHUTDOWN_ROLLBACK_ATTEMPTS;
}

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

/* Small install transaction used by the native bridge, independent of PS5
 * ABI. Arm rollback before each possibly partial write; restore only slots
 * which still point at this transaction's gateway. Metadata is persisted in
 * the target before invoking this function. -2 means retain it for retry. */
typedef struct {
    uint64_t slot, original, gateway;
    int protection;
} game_hook_change_t;
typedef int (*game_hook_read_slot_fn)(void *, uint64_t, uint64_t *);
typedef int (*game_hook_write_slot_fn)(void *, uint64_t, uint64_t, int);
#define GAME_HOOK_INSTALL_OK 1
#define GAME_HOOK_INSTALL_FAILED (-1)
#define GAME_HOOK_INSTALL_PENDING (-2)
#define GAME_HOOK_INSTALL_UNUSED (-3)
static inline int game_hook_apply_slots(const game_hook_change_t *changes,
    unsigned count, game_hook_read_slot_fn read_slot,
    game_hook_write_slot_fn write_slot, void *context) {
    if (!changes || !count || !read_slot || !write_slot) return GAME_HOOK_INSTALL_FAILED;
    unsigned touched = 0;
    for (unsigned i = 0; i < count; ++i) {
        uint64_t current;
        const game_hook_change_t *c = &changes[i];
        if (!c->slot || game_hook_slot_restore_action(c->gateway,c->gateway,c->original) != GAME_HOOK_SLOT_RESTORE ||
            read_slot(context,c->slot,&current) || current != c->original) break;
        touched = i + 1;
        if (write_slot(context,c->slot,c->gateway,c->protection) != 0) break;
        if (i + 1 == count) return GAME_HOOK_INSTALL_OK;
    }
    if (!touched) return GAME_HOOK_INSTALL_UNUSED;
    int pending = 0;
    while (touched) {
        const game_hook_change_t *c = &changes[--touched];
        uint64_t current;
        if (read_slot(context,c->slot,&current) != 0) { pending = 1; continue; }
        game_hook_slot_action_t action = game_hook_slot_restore_action(current,c->gateway,c->original);
        if (action == GAME_HOOK_SLOT_RESTORE &&
            write_slot(context,c->slot,c->original,c->protection) != 0) pending = 1;
        else if (action == GAME_HOOK_SLOT_INVALID) pending = 1;
    }
    return pending ? GAME_HOOK_INSTALL_PENDING : GAME_HOOK_INSTALL_FAILED;
}
