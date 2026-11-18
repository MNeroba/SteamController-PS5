/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once

#include <stdint.h>

#define CREDENTIAL_GUARD_CAPS_SIZE 16u

typedef struct credential_guard_ops {
    int (*read_authid)(void *context, uint64_t *out_authid);
    int (*read_caps)(void *context, uint8_t out_caps[CREDENTIAL_GUARD_CAPS_SIZE]);
    int (*write_authid)(void *context, uint64_t authid);
    int (*write_caps)(void *context, const uint8_t caps[CREDENTIAL_GUARD_CAPS_SIZE]);
} credential_guard_ops_t;

typedef struct credential_guard_snapshot {
    uint64_t authid;
    uint8_t caps[CREDENTIAL_GUARD_CAPS_SIZE];
    unsigned char restore_authid;
    unsigned char restore_caps;
    unsigned char active;
} credential_guard_snapshot_t;

#define CREDENTIAL_GUARD_SNAPSHOT_INITIALIZER {0}

typedef enum credential_guard_result {
    CREDENTIAL_GUARD_OK = 0,
    CREDENTIAL_GUARD_INVALID_ARGUMENT = -1,
    CREDENTIAL_GUARD_READ_FAILED = -2,
    CREDENTIAL_GUARD_ELEVATION_FAILED = -3,
    CREDENTIAL_GUARD_RESTORE_FAILED = -4,
    CREDENTIAL_GUARD_ALREADY_ACTIVE = -5,
    CREDENTIAL_GUARD_LOCK_FAILED = -6
} credential_guard_result_t;

typedef int (*credential_guard_operation_fn)(void *context);

/* Begin a temporary or persistent credential elevation. Passing NULL for
 * elevated_caps leaves capabilities unchanged. Initialize the snapshot to
 * CREDENTIAL_GUARD_SNAPSHOT_INITIALIZER; it must remain alive until restore
 * succeeds. */
credential_guard_result_t credential_guard_begin(
    const credential_guard_ops_t *ops,
    void *context,
    uint64_t elevated_authid,
    const uint8_t *elevated_caps,
    credential_guard_snapshot_t *snapshot);

/* Restore capabilities first, then authid. Failed fields remain armed so the
 * same snapshot can be retried safely. */
credential_guard_result_t credential_guard_restore(
    const credential_guard_ops_t *ops,
    void *context,
    credential_guard_snapshot_t *snapshot);

/* Run one operation inside a temporary elevation and always attempt restore.
 * The operation's return value is written separately from guard status. */
credential_guard_result_t credential_guard_run(
    const credential_guard_ops_t *ops,
    void *credential_context,
    uint64_t elevated_authid,
    const uint8_t *elevated_caps,
    credential_guard_operation_fn operation,
    void *operation_context,
    int *operation_result);
