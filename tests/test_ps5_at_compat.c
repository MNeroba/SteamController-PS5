/* SPDX-License-Identifier: GPL-3.0-or-later */
#define _POSIX_C_SOURCE 200809L
#define _DARWIN_C_SOURCE
#include "ps5_at_compat.h"
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>

static int scripted = 1, replace_before_open, calls;
static long observed_number, observed[5];
static ps5_at_raw_result_t answer;
ps5_at_raw_result_t ps5_at_test_call(long number, const long a[5]) {
    ++calls; observed_number = number; memcpy(observed, a, sizeof(observed));
    if (scripted) return answer;
    int saved = errno, result;
    switch (number) {
    case 499:
        if (replace_before_open) {
            assert(renameat((int)a[0], "replacement", (int)a[0], (const char *)a[1]) == 0);
            replace_before_open = 0;
        }
        result = openat((int)a[0], (const char *)a[1], (int)a[2], (mode_t)a[3]); break;
    case 493: result = fstatat((int)a[0], (const char *)a[1], (struct stat *)a[2], (int)a[3]); break;
    case 503: result = unlinkat((int)a[0], (const char *)a[1], (int)a[2]); break;
    case 501: result = renameat((int)a[0], (const char *)a[1], (int)a[2], (const char *)a[3]); break;
    case 495: result = linkat((int)a[0], (const char *)a[1], (int)a[2], (const char *)a[3], (int)a[4]); break;
    case 496: result = mkdirat((int)a[0], (const char *)a[1], (mode_t)a[2]); break;
    default: assert(!"unexpected syscall"); result = -1;
    }
    ps5_at_raw_result_t r = {result < 0 ? errno : result, result < 0};
    errno = saved; /* Raw kernel calls do not set libc errno. */
    return r;
}
static void arguments(void) {
    const char *from = "from", *to = "to";
    struct stat st;
    answer = (ps5_at_raw_result_t){0, 0}; calls = 0;
    assert(ps5_openat(11, from, O_CREAT | O_EXCL | O_NOFOLLOW, 0600) == 0);
    assert(observed_number == 499 && observed[0] == 11 && observed[1] == (long)from);
    assert(observed[2] == (O_CREAT | O_EXCL | O_NOFOLLOW) && observed[3] == 0600 && observed[4] == 0);
    assert(ps5_openat(11, from, O_RDONLY) == 0 && observed[3] == 0);
    assert(ps5_fstatat(12, from, &st, AT_SYMLINK_NOFOLLOW) == 0);
    assert(observed_number == 493 && observed[0] == 12 && observed[1] == (long)from);
    assert(observed[2] == (long)&st && observed[3] == AT_SYMLINK_NOFOLLOW);
    assert(ps5_unlinkat(13, from, AT_REMOVEDIR) == 0);
    assert(observed_number == 503 && observed[0] == 13 && observed[2] == AT_REMOVEDIR);
    assert(ps5_renameat(14, from, 15, to) == 0);
    assert(observed_number == 501 && observed[0] == 14 && observed[1] == (long)from);
    assert(observed[2] == 15 && observed[3] == (long)to && observed[4] == 0);
    assert(ps5_linkat(16, from, 17, to, AT_SYMLINK_FOLLOW) == 0);
    assert(observed_number == 495 && observed[0] == 16 && observed[1] == (long)from);
    assert(observed[2] == 17 && observed[3] == (long)to && observed[4] == AT_SYMLINK_FOLLOW);
    assert(ps5_mkdirat(18, to, 0700) == 0);
    assert(observed_number == 496 && observed[0] == 18 && observed[1] == (long)to && observed[2] == 0700);
    assert(calls == 7); /* No extra path lookups or fd inspection. */
    const int descriptors[] = {0, 2, 17, EMFILE, 84, 256, INT_MAX};
    for (unsigned i = 0; i < sizeof(descriptors) / sizeof(*descriptors); ++i) {
        answer = (ps5_at_raw_result_t){descriptors[i], 0}; errno = EBUSY;
        assert(ps5_openat(11, from, O_RDONLY) == descriptors[i] && errno == EBUSY);
    }
    const int errors[] = {ENOENT, EACCES, EEXIST, EMFILE, EBADF, EINTR, EIO, ENOSPC};
    for (unsigned i = 0; i < sizeof(errors) / sizeof(*errors); ++i) {
        answer = (ps5_at_raw_result_t){errors[i], 1};
        assert(ps5_openat(11, from, O_RDONLY) == -1 && errno == errors[i]);
        assert(ps5_fstatat(11, from, &st, 0) == -1 && errno == errors[i]);
        assert(ps5_unlinkat(11, from, 0) == -1 && errno == errors[i]);
        assert(ps5_renameat(11, from, 12, to) == -1 && errno == errors[i]);
        assert(ps5_linkat(11, from, 12, to, 0) == -1 && errno == errors[i]);
        assert(ps5_mkdirat(11, to, 0700) == -1 && errno == errors[i]);
    }
    answer = (ps5_at_raw_result_t){0, 1};
    assert(ps5_openat(11, from, O_RDONLY) == -1 && errno == EIO);
}
static void real_files(void) {
    scripted = 0;
    char path[] = "/tmp/sc-at-compat-XXXXXX"; assert(mkdtemp(path));
    int dir = open(path, O_RDONLY | O_DIRECTORY); assert(dir >= 0);
    int original = ps5_openat(dir, "target", O_RDWR | O_CREAT | O_EXCL | O_NOFOLLOW, 0600);
    assert(original >= 0 && write(original, "old", 3) == 3);
    assert(ps5_openat(dir, "target", O_CREAT | O_EXCL | O_WRONLY, 0600) == -1 && errno == EEXIST);
    assert(ps5_openat(dir, "missing", O_RDONLY | O_NOFOLLOW) == -1 && errno == ENOENT);
    assert(symlinkat("target", dir, "symlink") == 0);
    assert(ps5_openat(dir, "symlink", O_RDONLY | O_NOFOLLOW) == -1 && errno == ELOOP);
    /* R1: real descriptor exhaustion while fd24 already names this inode. */
    int alias = fcntl(original, F_DUPFD, EMFILE); assert(alias == EMFILE);
    struct rlimit old, limited; assert(getrlimit(RLIMIT_NOFILE, &old) == 0);
    limited = old; limited.rlim_cur = 32; assert(setrlimit(RLIMIT_NOFILE, &limited) == 0);
    int fillers[64], count = 0, fd;
    while ((fd = open("/dev/null", O_RDONLY)) >= 0) { assert(count < 64); fillers[count++] = fd; }
    assert(errno == EMFILE);
    int result = ps5_openat(dir, "target", O_RDONLY | O_NOFOLLOW), error = errno;
    assert(setrlimit(RLIMIT_NOFILE, &old) == 0);
    assert(result == -1 && error == EMFILE && fcntl(alias, F_GETFD) >= 0);
    while (count) assert(close(fillers[--count]) == 0);
    assert(close(alias) == 0);
    /* R2: a valid new inode remains owned by the caller through replacement. */
    fd = ps5_openat(dir, "replacement", O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW, 0600);
    assert(fd >= 0 && write(fd, "new", 3) == 3 && close(fd) == 0);
    replace_before_open = 1; fd = ps5_openat(dir, "target", O_RDONLY | O_NOFOLLOW);
    assert(fd >= 0); char text[4] = {0}; assert(read(fd, text, 3) == 3 && !strcmp(text, "new"));
    struct stat before, after; assert(fstat(original, &before) == 0 && fstat(fd, &after) == 0);
    assert(before.st_ino != after.st_ino && close(fd) == 0 && close(original) == 0);
    assert(ps5_mkdirat(dir, "sub", 0700) == 0);
    assert(ps5_mkdirat(dir, "sub", 0700) == -1 && errno == EEXIST);
    assert(ps5_linkat(dir, "target", dir, "copy", 0) == 0);
    assert(ps5_renameat(dir, "copy", dir, "renamed") == 0);
    assert(ps5_fstatat(dir, "renamed", &after, AT_SYMLINK_NOFOLLOW) == 0);
    assert(ps5_unlinkat(dir, "renamed", 0) == 0);
    assert(ps5_unlinkat(dir, "renamed", 0) == -1 && errno == ENOENT);
    assert(ps5_unlinkat(dir, "sub", AT_REMOVEDIR) == 0);
    assert(ps5_unlinkat(dir, "symlink", 0) == 0 && ps5_unlinkat(dir, "target", 0) == 0);
    assert(close(dir) == 0 && rmdir(path) == 0);
}
int main(void) {
    arguments(); real_files();
    puts("PS5 filesystem result/argument/fd-collision/replacement regressions passed");
    return 0;
}
