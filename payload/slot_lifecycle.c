/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "slot_lifecycle.h"

static int rollback_start(slot_registry_t *registry,
                          slot_registry_token_t token,
                          const slot_lifecycle_ops_t *ops,
                          void *context,
                          int *cleanup_result) {
    int result = slot_lifecycle_release(registry, token,
                                        ops->delete_virtual, context);
    if (cleanup_result) *cleanup_result = result;
    return result;
}

int slot_lifecycle_release(slot_registry_t *registry,
                           slot_registry_token_t token,
                           slot_registry_delete_fn delete_virtual,
                           void *context) {
    int result;
    if (!registry || !delete_virtual) return SLOT_REGISTRY_ERROR;

    result = slot_registry_request_release(registry, token);
    if (result != SLOT_REGISTRY_OK) return result;
    result = slot_registry_delete_virtual(registry, token, delete_virtual,
                                          context);
    if (result != SLOT_REGISTRY_OK) return result;
    return slot_registry_release(registry, token);
}

int slot_lifecycle_start(slot_registry_t *registry,
                         slot_registry_token_t token,
                         int attach_virtual,
                         const slot_lifecycle_ops_t *ops,
                         void *context,
                         int *cleanup_result) {
    slot_registry_entry_t entry;
    int32_t handle = -1;
    int bind_ok = 0;
    int result;

    if (cleanup_result) *cleanup_result = SLOT_REGISTRY_OK;
    if (!registry || !ops || !ops->start_worker || !ops->delete_virtual ||
        (attach_virtual && !ops->create_virtual))
        return SLOT_LIFECYCLE_INVALID;
    if (slot_registry_snapshot(registry, token, &entry) != SLOT_REGISTRY_OK ||
        entry.state != SLOT_REGISTRY_RESERVED)
        return SLOT_LIFECYCLE_INVALID;

    if (attach_virtual) {
        if (ops->create_virtual(token.index, &handle, &bind_ok, context) != 0 ||
            handle < 0) {
            rollback_start(registry, token, ops, context, cleanup_result);
            return SLOT_LIFECYCLE_VDA_CREATE_FAILED;
        }
        result = slot_registry_set_virtual(registry, token, handle);
        if (result != SLOT_REGISTRY_OK) {
            /* The created ID is not registry-owned. It may alias another
             * slot, so rollback must never delete that unowned value. */
            rollback_start(registry, token, ops, context, cleanup_result);
            return SLOT_LIFECYCLE_VDA_ATTACH_FAILED;
        }
    }

    if (slot_registry_activate(registry, token) != SLOT_REGISTRY_OK) {
        rollback_start(registry, token, ops, context, cleanup_result);
        return SLOT_LIFECYCLE_ACTIVATE_FAILED;
    }
    if (ops->start_worker(token, handle, bind_ok, context) != 0) {
        rollback_start(registry, token, ops, context, cleanup_result);
        return SLOT_LIFECYCLE_WORKER_START_FAILED;
    }
    return SLOT_LIFECYCLE_OK;
}
