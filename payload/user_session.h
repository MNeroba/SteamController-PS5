/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once
#include <pthread.h>
#include <stdint.h>

#define USER_SESSION_MAX_USERS 4
#define USER_SESSION_EMPTY_CHECKS 3
#define USER_SESSION_LOGIN_CHECKS 2
/* A read-only observer, not a second slot/user owner. Only the USB worker
 * acts on epochs. Three spaced empty lists AFTER login confirm one logout;
 * errors/malformed/fast observations break consensus. Account IDs stay private. */
typedef struct {
    uint32_t logout_epoch, logout_count, login_count, read_errors;
    int read_ok, logged_out, can_register, logged_users;
    int foreground_valid; /* membership only; never an automatic bind target */
} user_session_snapshot_t;
typedef struct {
    pthread_mutex_t mutex;
    user_session_snapshot_t value;
    int had_users, empty_checks, login_checks, have_time;
    uint64_t last_ms;
    int32_t candidate[4];
} user_session_t;
#define USER_SESSION_INITIALIZER {.mutex = PTHREAD_MUTEX_INITIALIZER}
void user_session_observe(user_session_t *state, int result, const int32_t users[4],
                          int foreground_result, int32_t foreground_user, uint64_t now_ms);
void user_session_snapshot(user_session_t *state, user_session_snapshot_t *out);
