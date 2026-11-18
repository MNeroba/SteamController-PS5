/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "credential_guard.h"

#include <errno.h>
#include <string.h>

static int
ops_valid_for(const credential_guard_ops_t *ops, int include_caps)
{
    return ops && ops->read_authid && ops->write_authid &&
           (!include_caps || (ops->read_caps && ops->write_caps));
}

credential_guard_result_t
credential_guard_restore(const credential_guard_ops_t *ops,
                         void *context,
                         credential_guard_snapshot_t *snapshot)
{
    if (!snapshot)
        return CREDENTIAL_GUARD_INVALID_ARGUMENT;
    if (!snapshot->active)
        return CREDENTIAL_GUARD_OK;
    if (!ops_valid_for(ops, snapshot->restore_caps))
        return CREDENTIAL_GUARD_INVALID_ARGUMENT;

    int failed = 0;
    /* Reduce capabilities before restoring the less privileged authid. */
    if (snapshot->restore_caps &&
        ops->write_caps(context, snapshot->caps) != 0)
        failed = 1;
    if (snapshot->restore_authid &&
        ops->write_authid(context, snapshot->authid) != 0)
        failed = 1;

    if (failed)
        return CREDENTIAL_GUARD_RESTORE_FAILED;

    snapshot->restore_caps = 0;
    snapshot->restore_authid = 0;
    snapshot->active = 0;
    return CREDENTIAL_GUARD_OK;
}

credential_guard_result_t
credential_guard_begin(const credential_guard_ops_t *ops,
                       void *context,
                       uint64_t elevated_authid,
                       const uint8_t *elevated_caps,
                       credential_guard_snapshot_t *snapshot)
{
    if (!snapshot || !ops_valid_for(ops, elevated_caps != NULL))
        return CREDENTIAL_GUARD_INVALID_ARGUMENT;
    if (snapshot->active)
        return CREDENTIAL_GUARD_ALREADY_ACTIVE;

    memset(snapshot, 0, sizeof(*snapshot));
    if (ops->read_authid(context, &snapshot->authid) != 0)
        return CREDENTIAL_GUARD_READ_FAILED;
    if (elevated_caps &&
        ops->read_caps(context, snapshot->caps) != 0)
        return CREDENTIAL_GUARD_READ_FAILED;

    /* A failed setter can still have made a partial change. Arm rollback
     * before calling it, and make restoration idempotent for retry. */
    snapshot->restore_authid = 1;
    snapshot->active = 1;
    if (ops->write_authid(context, elevated_authid) != 0) {
        credential_guard_result_t rollback =
            credential_guard_restore(ops, context, snapshot);
        return rollback == CREDENTIAL_GUARD_OK
            ? CREDENTIAL_GUARD_ELEVATION_FAILED : rollback;
    }

    if (elevated_caps) {
        snapshot->restore_caps = 1;
        if (ops->write_caps(context, elevated_caps) != 0) {
            credential_guard_result_t rollback =
                credential_guard_restore(ops, context, snapshot);
            return rollback == CREDENTIAL_GUARD_OK
                ? CREDENTIAL_GUARD_ELEVATION_FAILED : rollback;
        }
    }
    return CREDENTIAL_GUARD_OK;
}

credential_guard_result_t
credential_guard_run(const credential_guard_ops_t *ops,
                     void *credential_context,
                     uint64_t elevated_authid,
                     const uint8_t *elevated_caps,
                     credential_guard_operation_fn operation,
                     void *operation_context,
                     int *operation_result)
{
    if (!operation || !operation_result)
        return CREDENTIAL_GUARD_INVALID_ARGUMENT;

    credential_guard_snapshot_t snapshot = CREDENTIAL_GUARD_SNAPSHOT_INITIALIZER;
    credential_guard_result_t result = credential_guard_begin(
        ops, credential_context, elevated_authid, elevated_caps, &snapshot);
    if (result != CREDENTIAL_GUARD_OK) {
        if (result == CREDENTIAL_GUARD_RESTORE_FAILED) {
            for (unsigned attempt = 0; attempt < 2; ++attempt) {
                result = credential_guard_restore(
                    ops, credential_context, &snapshot);
                if (result == CREDENTIAL_GUARD_OK)
                    return CREDENTIAL_GUARD_ELEVATION_FAILED;
            }
        }
        return result;
    }

    int value = operation(operation_context);
    int operation_errno = errno;
    for (unsigned attempt = 0; attempt < 3; ++attempt) {
        result = credential_guard_restore(ops, credential_context, &snapshot);
        if (result == CREDENTIAL_GUARD_OK)
            break;
    }
    if (result != CREDENTIAL_GUARD_OK) {
        errno = EIO;
        return result;
    }

    errno = operation_errno;
    *operation_result = value;
    return CREDENTIAL_GUARD_OK;
}
