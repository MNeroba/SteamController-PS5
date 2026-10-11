/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once
/* Shared private, nonblocking regular-file storage. Feature-test macros must
 * be set before system headers by callers. No symlink following, predictable
 * temporary truncation or unchecked persistence success. */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static inline int private_directory_open(const char *path) {
    int fd = open(path, O_RDONLY | O_DIRECTORY | O_NOFOLLOW);
    struct stat st;
    if (fd < 0 && errno == ENOENT) {
        if (mkdir(path, 0700) != 0 && errno != EEXIST)
            return -1;
        fd = open(path, O_RDONLY | O_DIRECTORY | O_NOFOLLOW);
    }
    if (fd < 0)
        return -1;
    if (fstat(fd, &st) || !S_ISDIR(st.st_mode) || st.st_uid != geteuid() ||
        fchmod(fd, 0700) || fstat(fd, &st) || (st.st_mode & 077)) {
        close(fd);
        return -1;
    }
    return fd;
}
static inline int private_regular_open(int directory, const char *name) {
    int fd = openat(directory, name, O_RDONLY | O_NOFOLLOW | O_NONBLOCK);
    struct stat st;
    if (fd < 0)
        return -1;
    if (fstat(fd, &st) || !S_ISREG(st.st_mode) || st.st_uid != geteuid()) {
        close(fd);
        errno = EINVAL;
        return -1;
    }
    return fd;
}
static inline FILE *private_text_read(const char *path) {
    int fd = open(path, O_RDONLY | O_NOFOLLOW | O_NONBLOCK);
    struct stat st;
    if (fd < 0)
        return NULL;
    if (fstat(fd, &st) || !S_ISREG(st.st_mode) || st.st_uid != geteuid()) {
        close(fd);
        return NULL;
    }
    FILE *f = fdopen(fd, "r");
    if (!f)
        close(fd);
    return f;
}
static inline int private_text_replace(const char *path, const char *text, size_t n) {
    char directory[512], name[128], temporary[136];
    const char *slash = strrchr(path, '/');
    size_t length = slash ? (size_t)(slash - path) : 0;
    if (length >= sizeof(directory) || strlen(slash ? slash + 1 : path) >= sizeof(name))
        return -1;
    if (slash) {
        memcpy(directory, path, length);
        directory[length] = 0;
        if (!length)
            strcpy(directory, "/");
    } else
        strcpy(directory, ".");
    strcpy(name, slash ? slash + 1 : path);
    if (!name[0])
        return -1;
    snprintf(temporary, sizeof(temporary), "%s.tmp", name);
    int dir = private_directory_open(directory);
    if (dir < 0)
        return -1;
    int fd = openat(dir, temporary, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW, 0600);
    if (fd < 0) {
        close(dir);
        return -1;
    }
    int result = 0;
    size_t used = 0;
    while (used < n) {
        ssize_t written = write(fd, text + used, n - used);
        if (written < 0 && errno == EINTR)
            continue;
        if (written <= 0) {
            result = -1;
            break;
        }
        used += (size_t)written;
    }
    if (fsync(fd) != 0)
        result = -1;
    if (close(fd) != 0)
        result = -1;
    if (!result && renameat(dir, temporary, dir, name) != 0)
        result = -1;
    if (result)
        unlinkat(dir, temporary, 0);
    close(dir);
    return result;
}
