/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "credential_kernel.h"
#include "runtime_control.h"

#include <pthread.h>
#include <string.h>

#if defined(__PROSPERO__)
#include <ps5/kernel.h>
#elif defined(__ORBIS__)
#include <ps4/kernel.h>
#else
#error "credential_kernel requires a supported Sony kernel SDK"
#endif

static int
kernel_read_authid(void *context, uint64_t *out_authid)
{
    pid_t pid = *(pid_t *)context;
    if (pid <= 0 || !out_authid)
        return -1;
    uint64_t authid = kernel_get_ucred_authid(pid);
    /* This API returns the value directly and exposes no separate status.
     * Existing callers used zero as an unavailable/error sentinel; retain
     * that fail-closed assumption until the Prospero ABI is verified. */
    if (authid == 0)
        return -1;
    *out_authid = authid;
    return 0;
}

static int
kernel_read_caps(void *context, uint8_t out_caps[CREDENTIAL_GUARD_CAPS_SIZE])
{
    pid_t pid = *(pid_t *)context;
    return (pid > 0 && out_caps)
        ? kernel_get_ucred_caps(pid, out_caps) : -1;
}

static int
kernel_write_authid(void *context, uint64_t authid)
{
    pid_t pid = *(pid_t *)context;
    return pid > 0 ? kernel_set_ucred_authid(pid, authid) : -1;
}

static int
kernel_write_caps(void *context, const uint8_t caps[CREDENTIAL_GUARD_CAPS_SIZE])
{
    pid_t pid = *(pid_t *)context;
    return (pid > 0 && caps) ? kernel_set_ucred_caps(pid, caps) : -1;
}

static const credential_guard_ops_t g_kernel_ops = {
    .read_authid = kernel_read_authid,
    .read_caps = kernel_read_caps,
    .write_authid = kernel_write_authid,
    .write_caps = kernel_write_caps
};
static pthread_mutex_t g_credential_scope_lock = PTHREAD_MUTEX_INITIALIZER;

credential_guard_result_t
credential_kernel_begin(pid_t pid,
                        uint64_t elevated_authid,
                        const uint8_t *elevated_caps,
                        credential_kernel_scope_t *scope)
{
    if (!scope || pid <= 0)
        return CREDENTIAL_GUARD_INVALID_ARGUMENT;
    if (scope->snapshot.active)
        return CREDENTIAL_GUARD_ALREADY_ACTIVE;
    memset(scope, 0, sizeof(*scope));
    scope->pid = pid;
    credential_guard_result_t result = credential_guard_begin(
        &g_kernel_ops, &scope->pid, elevated_authid, elevated_caps,
        &scope->snapshot);
    if (result != CREDENTIAL_GUARD_OK &&
        result != CREDENTIAL_GUARD_RESTORE_FAILED)
        scope->pid = 0;
    return result;
}

credential_guard_result_t
credential_kernel_restore(credential_kernel_scope_t *scope)
{
    if (!scope)
        return CREDENTIAL_GUARD_INVALID_ARGUMENT;
    if (!scope->snapshot.active) {
        scope->pid = 0;
        return CREDENTIAL_GUARD_OK;
    }
    if (scope->pid <= 0)
        return CREDENTIAL_GUARD_INVALID_ARGUMENT;
    credential_guard_result_t result = credential_guard_restore(
        &g_kernel_ops, &scope->pid, &scope->snapshot);
    if (result == CREDENTIAL_GUARD_OK)
        scope->pid = 0;
    return result;
}

credential_guard_result_t
credential_kernel_run(pid_t pid,
                      uint64_t elevated_authid,
                      const uint8_t *elevated_caps,
                      credential_guard_operation_fn operation,
                      void *operation_context,
                      int *operation_result)
{
    if (pid <= 0)
        return CREDENTIAL_GUARD_INVALID_ARGUMENT;
    if (pthread_mutex_lock(&g_credential_scope_lock) != 0)
        return CREDENTIAL_GUARD_LOCK_FAILED;
    credential_guard_result_t result = credential_guard_run(
        &g_kernel_ops, &pid, elevated_authid, elevated_caps, operation,
        operation_context, operation_result);
    if (pthread_mutex_unlock(&g_credential_scope_lock) != 0) {
        runtime_request_stop();
        return CREDENTIAL_GUARD_LOCK_FAILED;
    }
    if (result == CREDENTIAL_GUARD_RESTORE_FAILED)
        runtime_request_stop();
    return result;
}

const char *
credential_guard_result_name(credential_guard_result_t result)
{
    switch (result) {
    case CREDENTIAL_GUARD_OK: return "ok";
    case CREDENTIAL_GUARD_INVALID_ARGUMENT: return "invalid argument";
    case CREDENTIAL_GUARD_READ_FAILED: return "credential read failed";
    case CREDENTIAL_GUARD_ELEVATION_FAILED: return "credential elevation failed";
    case CREDENTIAL_GUARD_RESTORE_FAILED: return "credential restoration failed";
    case CREDENTIAL_GUARD_ALREADY_ACTIVE: return "credential scope already active";
    case CREDENTIAL_GUARD_LOCK_FAILED: return "credential scope lock failed";
    }
    return "unknown credential error";
}
