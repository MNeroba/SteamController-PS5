/* SPDX-License-Identifier: GPL-3.0-or-later */
#define _POSIX_C_SOURCE 200809L
#define _DARWIN_C_SOURCE
#include "ps5_at_compat.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int created_fd = -1, fail_created_stat;
ps5_at_raw_result_t ps5_at_test_call(long number, const long a[5]) {
    int saved = errno, r;
    if (number == 499) {
        r = openat((int)a[0], (const char *)a[1], (int)a[2], (mode_t)a[3]);
        if (r >= 0 && (a[2] & O_CREAT)) created_fd = r;
    } else if (number == 503) r = unlinkat((int)a[0], (const char *)a[1], (int)a[2]);
    else { assert(!"unexpected token syscall"); r = -1; }
    ps5_at_raw_result_t result = {r < 0 ? errno : r, r < 0}; errno = saved; return result;
}
static int injected_stat(int fd, struct stat *st) {
    if (fail_created_stat && fd == created_fd) { errno = EINTR; return -1; }
    return fstat(fd, st);
}
/* Exercise the existing credential cleanup through the actual target adapter. */
#define openat ps5_openat
#define unlinkat ps5_unlinkat
#define fstat injected_stat
#include "../payload/webui_auth.c"
#undef openat
#undef unlinkat
#undef fstat
int main(void) {
    char directory[] = "/tmp/sc-at-token-XXXXXX"; assert(mkdtemp(directory));
    char token[65], path[128]; snprintf(path, sizeof(path), "%s/webui.token", directory);
    storage_status_t status; int created = 123;
    fail_created_stat = 1;
    assert(webui_auth_load_or_create_report(directory, token, &created, &status) == -1);
    assert(status.stage == STORAGE_TOKEN_STAT && status.error == EINTR && !created);
    assert(fcntl(created_fd, F_GETFD) == -1 && errno == EBADF);
    assert(access(path, F_OK) == -1 && errno == ENOENT);
    for (unsigned i = 0; i < sizeof(token); ++i) assert(token[i] == 0);
    fail_created_stat = 0; created_fd = -1;
    assert(webui_auth_load_or_create_report(directory, token, &created, &status) == 0 && created);
    assert(webui_token_is_valid(token)); char saved[65]; memcpy(saved, token, sizeof(saved));
    assert(webui_auth_load_or_create_report(directory, token, &created, &status) == 0 && !created);
    assert(!strcmp(token, saved));
    assert(unlink(path) == 0 && rmdir(directory) == 0);
    puts("PS5 token adapter: post-create stat failure closes fd, removes empty token and permits retry");
    return 0;
}
