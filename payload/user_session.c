/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "user_session.h"
#include <string.h>

void user_session_observe(user_session_t *s, int result, const int32_t users[4],
                          int foreground_result, int32_t foreground, uint64_t now) {
    if (!s)
        return;
    int32_t sorted[4] = {-1, -1, -1, -1};
    int count = 0, valid = result == 0 && users;
    if (valid)
        for (int i = 0; i < 4; i++) {
            if (users[i] == -1)
                continue;
            if (users[i] <= 0) {
                valid = 0;
                break;
            }
            for (int j = 0; j < count; j++)
                if (sorted[j] == users[i])
                    valid = 0;
            if (!valid)
                break;
            int j = count++;
            while (j > 0 && sorted[j - 1] > users[i]) {
                sorted[j] = sorted[j - 1];
                j--;
            }
            sorted[j] = users[i];
        }
    pthread_mutex_lock(&s->mutex);
    /* The manager polls at 2s; guard replay/rapid sampling as well. */
    if (s->have_time && (now < s->last_ms || now - s->last_ms < 500))
        valid = 0;
    s->have_time = 1;
    s->last_ms = now;
    s->value.read_ok = valid;
    s->value.foreground_valid = 0;
    if (!valid) {
        s->value.read_errors++;
        s->empty_checks = s->login_checks = 0;
        s->value.can_register = 0;
        pthread_mutex_unlock(&s->mutex);
        return;
    }
    s->value.logged_users = count;
    for (int i = 0; i < count; i++)
        if (foreground_result == 0 && sorted[i] == foreground)
            s->value.foreground_valid = 1;
    if (count) {
        s->had_users = 1;
        s->empty_checks = 0;
        if (!memcmp(sorted, s->candidate, sizeof(sorted))) {
            if (s->login_checks < USER_SESSION_LOGIN_CHECKS)
                s->login_checks++;
        } else {
            memcpy(s->candidate, sorted, sizeof(sorted));
            s->login_checks = 1;
        }
        if (s->value.logged_out && s->login_checks >= USER_SESSION_LOGIN_CHECKS) {
            s->value.logged_out = 0;
            s->value.login_count++;
        }
    } else {
        s->login_checks = 0;
        memset(s->candidate, 0, sizeof(s->candidate));
        if (s->had_users && !s->value.logged_out &&
            ++s->empty_checks >= USER_SESSION_EMPTY_CHECKS) {
            s->value.logged_out = 1;
            s->value.logout_count++;
            if (++s->value.logout_epoch == 0)
                ++s->value.logout_epoch;
            s->had_users = 0;
        }
    }
    /* Startup at the picker is legitimate; after a logout wait for stable login. */
    s->value.can_register = !s->value.logged_out;
    pthread_mutex_unlock(&s->mutex);
}
void user_session_snapshot(user_session_t *s, user_session_snapshot_t *out) {
    if (!s || !out)
        return;
    pthread_mutex_lock(&s->mutex);
    *out = s->value;
    pthread_mutex_unlock(&s->mutex);
}
