/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once
/* Local patch: on the tested console the *at() family from libkernel returns
 * a POSITIVE errno value on failure (with errno left at 0) instead of -1/errno.
 * Measured on hardware with a probe payload:
 *   openat(missing)            -> 2   (ENOENT, looks like fd 2 = stderr socket)
 *   openat(O_EXCL, existing)   -> 17  (EEXIST, looks like fd 17)
 *   fstatat/unlinkat/renameat(missing) -> 2, errno 0
 * Successful calls behave normally. These wrappers restore POSIX semantics and
 * are force-included into every payload translation unit for PS5 builds only.
 */
#ifdef __PROSPERO__
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

static inline int ps5_at_result(int r) {
    if (r > 0) { errno = r; return -1; }
    return r;
}
static inline int ps5_fstatat(int dir, const char *name, struct stat *st, int flags) {
    return ps5_at_result(fstatat(dir, name, st, flags));
}
static inline int ps5_unlinkat(int dir, const char *name, int flags) {
    return ps5_at_result(unlinkat(dir, name, flags));
}
static inline int ps5_renameat(int from_dir, const char *from, int to_dir, const char *to) {
    return ps5_at_result(renameat(from_dir, from, to_dir, to));
}
static inline int ps5_linkat(int from_dir, const char *from, int to_dir, const char *to, int flags) {
    return ps5_at_result(linkat(from_dir, from, to_dir, to, flags));
}
static inline int ps5_mkdirat(int dir, const char *name, mode_t mode) {
    return ps5_at_result(mkdirat(dir, name, mode));
}

/* openat() cannot be fixed by sign alone because a positive errno is
 * indistinguishable from a real descriptor. Check the name first and verify
 * the returned descriptor refers to that same file (dev/ino) afterwards. A
 * mismatch means the "descriptor" was really an errno: report it and never
 * use or close it. */
static inline int ps5_openat(int dir, const char *name, int flags, ...) {
    mode_t mode = 0;
    if (flags & O_CREAT) {
        va_list ap; va_start(ap, flags); mode = (mode_t)va_arg(ap, int); va_end(ap);
    }
    struct stat before, after;
    int exists = fstatat(dir, name, &before, AT_SYMLINK_NOFOLLOW);
    if (exists < 0) return -1;
    if (exists > 0 && exists != ENOENT) { errno = exists; return -1; }
    if (exists == 0) {
        if ((flags & O_CREAT) && (flags & O_EXCL)) { errno = EEXIST; return -1; }
        if ((flags & O_NOFOLLOW) && S_ISLNK(before.st_mode)) { errno = ELOOP; return -1; }
    } else if (!(flags & O_CREAT)) {
        errno = ENOENT; return -1;
    }
    errno = 0;
    int fd = openat(dir, name, flags, mode);
    if (fd < 0) return -1;
    int fstat_ok = fstat(fd, &after) == 0;
    if (exists == 0) {
        if (fstat_ok && after.st_dev == before.st_dev && after.st_ino == before.st_ino)
            return fd;
    } else if (fstat_ok && fstatat(dir, name, &before, AT_SYMLINK_NOFOLLOW) == 0 &&
               after.st_dev == before.st_dev && after.st_ino == before.st_ino) {
        return fd;
    }
    /* Not the requested file: treat the return value as an errno. */
    errno = fd > 0 && fd < 256 ? fd : EIO;
    return -1;
}

#define openat ps5_openat
#define fstatat ps5_fstatat
#define unlinkat ps5_unlinkat
#define renameat ps5_renameat
#define linkat ps5_linkat
#define mkdirat ps5_mkdirat
#endif
