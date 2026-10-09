/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once

#include "credential_guard.h"

#include <sys/types.h>

typedef struct credential_kernel_scope {
    pid_t pid;
    credential_guard_snapshot_t snapshot;
} credential_kernel_scope_t;

#define CREDENTIAL_KERNEL_SCOPE_INITIALIZER {0}

/* Kernel-backed adapters used by payload code. Host tests exercise the
 * platform-independent credential_guard core with injected operations. */
credential_guard_result_t credential_kernel_begin(
    pid_t pid,
    uint64_t elevated_authid,
    const uint8_t *elevated_caps,
    credential_kernel_scope_t *scope);
credential_guard_result_t credential_kernel_restore(
    credential_kernel_scope_t *scope);
credential_guard_result_t credential_kernel_run(
    pid_t pid,
    uint64_t elevated_authid,
    const uint8_t *elevated_caps,
    credential_guard_operation_fn operation,
    void *operation_context,
    int *operation_result);
const char *credential_guard_result_name(credential_guard_result_t result);
