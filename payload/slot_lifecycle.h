/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once

#include "slot_registry.h"
#include "user_session.h"

#include <stdint.h>

typedef enum {
    SLOT_LIFECYCLE_OK = 0,
    SLOT_LIFECYCLE_INVALID = -1,
    SLOT_LIFECYCLE_VDA_CREATE_FAILED = -2,
    SLOT_LIFECYCLE_VDA_ATTACH_FAILED = -3,
    SLOT_LIFECYCLE_ACTIVATE_FAILED = -4,
    SLOT_LIFECYCLE_WORKER_START_FAILED = -5
} slot_lifecycle_result_t;

typedef struct {
    int (*create_virtual)(uint32_t slot, int32_t *out_handle,
                          int *out_bind_ok, void *context);
    int (*start_worker)(slot_registry_token_t token, int32_t virtual_handle,
                        int bind_ok, void *context);
    slot_registry_delete_fn delete_virtual;
} slot_lifecycle_ops_t;

/* Run the portable manager startup transaction for a slot already claimed in
 * RESERVED state. On failure this requests release and deletes only a VDA
 * handle registered to this generation. cleanup_result reports whether that
 * rollback completed or remains deferred. A create callback must not return a
 * failure after creating an unreported virtual device. */
int slot_lifecycle_start(slot_registry_t *registry,
                         slot_registry_token_t token,
                         int attach_virtual,
                         const slot_lifecycle_ops_t *ops,
                         void *context,
                         int *cleanup_result);

/* Shared teardown transaction used by manager rollback, reader exit, and
 * deferred cleanup retries. Failed backend deletion retains the slot/handle. */
int slot_lifecycle_release(slot_registry_t *registry,
                           slot_registry_token_t token,
                           slot_registry_delete_fn delete_virtual,
                           void *context);

int slot_lifecycle_attach(slot_registry_t *registry, slot_registry_token_t token,
                          const slot_lifecycle_ops_t *ops, void *context, int *bind_ok);
typedef struct { uint32_t epoch, attempts, recovered, deletion_failures; int pending; } slot_recovery_t;
/* Called only by the existing USB worker at report boundaries. No USB I/O,
 * mapping reset or thread restart. eligibility excludes pause/release/sleep. */
int slot_lifecycle_recover(slot_registry_t *registry, slot_registry_token_t token,
                           slot_recovery_t *recovery, const user_session_snapshot_t *users,
                           int eligible, const slot_lifecycle_ops_t *ops, void *context);
