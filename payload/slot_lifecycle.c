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
        result=slot_registry_begin_virtual_create(registry,token);
        if(result!=SLOT_REGISTRY_OK) {
            rollback_start(registry,token,ops,context,cleanup_result);
            return SLOT_LIFECYCLE_VDA_ATTACH_FAILED;
        }
        int created=ops->create_virtual(token.index,&handle,&bind_ok,context);
        result=slot_registry_finish_virtual_create(registry,token,handle);
        if(created!=0 || handle<=0) {
            rollback_start(registry,token,ops,context,cleanup_result);
            return SLOT_LIFECYCLE_VDA_CREATE_FAILED;
        }
        if(result!=SLOT_REGISTRY_OK) {
            /* Never delete an ambiguous receipt; the registry inhibits Add. */
            rollback_start(registry,token,ops,context,cleanup_result);
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

int slot_lifecycle_attach(slot_registry_t *r,slot_registry_token_t t,
                          const slot_lifecycle_ops_t *ops,void *ctx,int *bind_ok) {
    if(!ops || !ops->create_virtual)return SLOT_LIFECYCLE_INVALID;
    int rc=slot_registry_begin_virtual_create(r,t);if(rc)return rc;
    int32_t h=-1;int bind=0;
    int created=ops->create_virtual(t.index,&h,&bind,ctx);
    rc=slot_registry_finish_virtual_create(r,t,h);
    if(bind_ok)*bind_ok=bind;
    return created || rc ? SLOT_LIFECYCLE_VDA_ATTACH_FAILED : SLOT_LIFECYCLE_OK;
}
int slot_lifecycle_recover(slot_registry_t *r,slot_registry_token_t t,
                           slot_recovery_t *recovery,const user_session_snapshot_t *users,
                           int eligible,const slot_lifecycle_ops_t *ops,void *ctx) {
    slot_registry_entry_t e;
    if(!recovery || !users || !ops || !ops->delete_virtual ||
       slot_registry_snapshot(r,t,&e)!=SLOT_REGISTRY_OK)return SLOT_LIFECYCLE_INVALID;
    if(users->logout_epoch!=recovery->epoch) {
        recovery->epoch=users->logout_epoch;
        if(e.virtual_ready || e.virtual_creating)recovery->pending=1;
    }
    if(!eligible || e.state!=SLOT_REGISTRY_ACTIVE) {recovery->pending=0;return 0;}
    if(!recovery->pending)return 0;
    if(e.virtual_creating || e.virtual_deleting)return SLOT_REGISTRY_BUSY;
    if(e.virtual_ready) {
        recovery->attempts++;
        int rc=slot_registry_delete_virtual(r,t,ops->delete_virtual,ctx);
        if(rc) {recovery->deletion_failures++;return rc;}
    }
    if(!users->can_register)return 1; /* confirmed logout; await stable login */
    int rc=slot_lifecycle_attach(r,t,ops,ctx,0);
    if(!rc) {recovery->pending=0;recovery->recovered++;}
    return rc;
}
