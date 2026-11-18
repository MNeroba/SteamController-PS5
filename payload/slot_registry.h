/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once

#include <pthread.h>
#include <stdint.h>

#define SLOT_REGISTRY_MAX_SLOTS 4
#define SLOT_REGISTRY_ID_MAX 32

typedef enum {
    SLOT_REGISTRY_FREE = 0,
    SLOT_REGISTRY_RESERVED = 1,
    SLOT_REGISTRY_ACTIVE = 2,
    SLOT_REGISTRY_RELEASING = 3
} slot_registry_state_t;

typedef enum {
    SLOT_REGISTRY_OK = 0,
    SLOT_REGISTRY_ERROR = -1,
    SLOT_REGISTRY_DUPLICATE = -2,
    SLOT_REGISTRY_FULL = -3,
    SLOT_REGISTRY_STALE = -4,
    SLOT_REGISTRY_BUSY = -5,
    SLOT_REGISTRY_BAD_STATE = -6,
    SLOT_REGISTRY_HANDLE_IN_USE = -7,
    SLOT_REGISTRY_DELETE_FAILED = -8
} slot_registry_result_t;

typedef struct {
    uint32_t index;
    uint32_t generation;
} slot_registry_token_t;

typedef struct {
    slot_registry_state_t state;
    slot_registry_token_t token;
    char identity[SLOT_REGISTRY_ID_MAX];
    uint16_t vendor_id;
    uint16_t product_id;
    int32_t virtual_handle;
    uint64_t virtual_device_id; /* private MBus identity; never expose account IDs */
    int virtual_retiring; /* cancellation cleanup unresolved: never inject */
    int virtual_correlated; /* owner-authorized event correlation, NOT a direct ABI receipt */
    int virtual_ready;
    int virtual_deleting;
    int virtual_creating;
    int virtual_blocked; /* ambiguous receipt: never repeat AddDevice */
    int confirmed;
} slot_registry_entry_t;

typedef int (*slot_registry_delete_fn)(int32_t handle, void *context);

/* The registry is the single owner of physical-session to virtual-slot
 * metadata. Identity is a caller-supplied key unique for the current session;
 * this module does not claim that a USB path is a permanent device identity.
 * Initialize once before starting workers; destroy only after they all stop. */
typedef struct {
    pthread_mutex_t mutex;
    uint32_t next_generation[SLOT_REGISTRY_MAX_SLOTS];
    slot_registry_entry_t entries[SLOT_REGISTRY_MAX_SLOTS];
    int initialized;
    int creation_blocked; /* ambiguous Add receipt inhibits all future creation */
} slot_registry_t;

int slot_registry_init(slot_registry_t *registry);
void slot_registry_destroy(slot_registry_t *registry);

int slot_registry_claim(slot_registry_t *registry, const char *identity,
                        uint16_t vendor_id, uint16_t product_id,
                        slot_registry_token_t *out_token);
int slot_registry_find_identity(slot_registry_t *registry,
                                const char *identity, uint32_t *out_index);
int slot_registry_activate(slot_registry_t *registry,
                           slot_registry_token_t token);
int slot_registry_snapshot(slot_registry_t *registry,
                           slot_registry_token_t token,
                           slot_registry_entry_t *out_entry);
int slot_registry_snapshot_index(slot_registry_t *registry, uint32_t index,
                                 slot_registry_entry_t *out_entry);
/* Prevent further activation/virtual attachment while cleanup is pending. */
int slot_registry_request_release(slot_registry_t *registry,
                                  slot_registry_token_t token);
int slot_registry_set_virtual(slot_registry_t *registry,
                              slot_registry_token_t token,
                              int32_t virtual_handle);
int slot_registry_take_virtual(slot_registry_t *registry,
                               slot_registry_token_t token,
                               int32_t *out_handle);
/* Calls the backend outside the registry lock. Failed deletion restores
 * ownership and returns SLOT_REGISTRY_DELETE_FAILED. */
int slot_registry_delete_virtual(slot_registry_t *registry,
                                 slot_registry_token_t token,
                                 slot_registry_delete_fn delete_fn,
                                 void *context);
/* Button activity only: this is not proof of PS5 account binding.
 * Returns 1 on first confirmation, 0 if already confirmed, or a negative
 * SLOT_REGISTRY_* result for an invalid/stale token. */
int slot_registry_mark_confirmed(slot_registry_t *registry,
                                 slot_registry_token_t token);
/* Release only succeeds after request_release and after VDA ownership is gone. */
int slot_registry_release(slot_registry_t *registry,
                          slot_registry_token_t token);
uint32_t slot_registry_count_claimed(slot_registry_t *registry);

/* Reserve creation before any side effect. Finish retains a positive owned
 * handle even if release was requested during AddDevice, for later cleanup.
 * A nonpositive or aliased receipt inhibits further creation for this registry
 * lifetime: it does not prove AddDevice had no system-side effect. */
int slot_registry_begin_virtual_create(slot_registry_t *registry, slot_registry_token_t token);
int slot_registry_finish_virtual_create(slot_registry_t *registry, slot_registry_token_t token, int32_t handle);

/* Owner-authorized experimental LibrePad-style correlation. Only a unique,
 * complete, typed virtual event from the exact reserved creation window may
 * reach this API. ID low32 is the upstream-derived VDI handle hypothesis. */
int slot_registry_finish_correlated_create(slot_registry_t *registry,
                                           slot_registry_token_t token,uint64_t device_id,int retiring);
