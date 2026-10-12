/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "user_session.h"
#include <assert.h>
#include <stdio.h>
static uint64_t now;
static user_session_snapshot_t v;
static void read_list(user_session_t *s, int rc, const int32_t *users) {
    now += 500;
    user_session_observe(s, rc, users, 0, 0x10000001, now);
    user_session_snapshot(s, &v);
}
int main(void) {
    user_session_t s = USER_SESSION_INITIALIZER;
    int32_t empty[4] = {-1, -1, -1, -1}, a[4] = {0x10000001, -1, -1, -1},
            b[4] = {0x10000002, -1, -1, -1};
    for (int i = 0; i < 6; i++)
        read_list(&s, 0, empty);
    assert(v.logout_epoch == 0 && v.can_register); /* startup at picker never recovery */
    read_list(&s, 0, a);
    assert(v.foreground_valid);
    read_list(&s, 0, empty);
    read_list(&s, -1, empty);
    read_list(&s, 0, empty);
    read_list(&s, 0, a);
    assert(v.logout_epoch == 0); /* errors/transient empties break consensus */
    for (int i = 0; i < 3; i++)
        read_list(&s, 0, empty);
    assert(v.logged_out && v.logout_epoch == 1 && !v.can_register);
    for (int i = 0; i < 10; i++)
        read_list(&s, 0, empty);
    assert(v.logout_epoch == 1); /* no repeated recreate at login screen */
    read_list(&s, 0, a);
    assert(!v.can_register);
    read_list(&s, 0, b);
    assert(!v.can_register && !v.foreground_valid);
    read_list(&s, 0, a);
    assert(!v.can_register);
    read_list(&s, 0, a);
    assert(v.can_register && v.login_count == 1);
    read_list(&s, 0, empty);
    read_list(&s, 0, empty);
    int32_t malformed[4] = {0, -1, -1, -1};
    read_list(&s, 0, malformed);
    assert(v.logout_epoch == 1 && !v.read_ok);
    int32_t duplicate[4] = {0x10000001, 0x10000001, -1, -1};
    read_list(&s, 0, duplicate);
    assert(!v.read_ok);
    read_list(&s, 0, a);
    read_list(&s, 0, empty);
    user_session_observe(&s, 0, empty, 0, 1, ++now);
    user_session_snapshot(&s, &v);
    assert(!v.read_ok);
    read_list(&s, 0, empty);
    read_list(&s, 0, empty);
    assert(v.logout_epoch == 1);
    read_list(&s, 0, empty);
    assert(v.logout_epoch == 2);
    pthread_mutex_destroy(&s.mutex);
    puts("user-session login/logout consensus checks passed");
    return 0;
}
