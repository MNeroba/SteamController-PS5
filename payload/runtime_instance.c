/* SPDX-License-Identifier: GPL-3.0-or-later */
#if !defined(__PROSPERO__) && !defined(_DEFAULT_SOURCE)
#define _DEFAULT_SOURCE 1
#endif

#include "runtime_instance.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static int read_pid_file(int fd, pid_t *out_pid) {
    char contents[32];
    if (!out_pid || lseek(fd, 0, SEEK_SET) < 0) return -1;
    ssize_t length = read(fd, contents, sizeof(contents) - 1);
    if (length < 0) return -1;
    if (length == 0) {
        *out_pid = 0;
        return 0;
    }
    if ((size_t)length >= sizeof(contents) - 1) {
        errno = EINVAL;
        return -1;
    }
    if (contents[length - 1] == '\n') --length;
    if (length <= 0) {
        errno = EINVAL;
        return -1;
    }
    contents[length] = '\0';
    for (ssize_t i = 0; i < length; ++i) {
        if (contents[i] < '0' || contents[i] > '9') {
            errno = EINVAL;
            return -1;
        }
    }
    errno = 0;
    char *end = NULL;
    long parsed = strtol(contents, &end, 10);
    if (errno != 0 || !end || *end != '\0' || parsed <= 0 ||
        parsed > INT_MAX) {
        errno = EINVAL;
        return -1;
    }
    *out_pid = (pid_t)parsed;
    return 0;
}

static int close_after_error(int fd, int error) {
    if (fd >= 0) {
        (void)flock(fd, LOCK_UN);
        (void)close(fd);
    }
    errno = error;
    return -1;
}

static int write_pid_file(int fd, pid_t pid) {
    char contents[32];
    int length = snprintf(contents, sizeof(contents), "%ld", (long)pid);
    if (length <= 0 || (size_t)length >= sizeof(contents)) {
        errno = EOVERFLOW;
        return -1;
    }
    if (ftruncate(fd, 0) != 0 || lseek(fd, 0, SEEK_SET) < 0)
        return -1;
    ssize_t written = 0;
    while (written < length) {
        ssize_t result = write(fd, contents + written,
                               (size_t)(length - written));
        if (result < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (result == 0) {
            errno = EIO;
            return -1;
        }
        written += result;
    }
    return 0;
}

int runtime_instance_acquire(const char *path, int *out_fd) {
    if (!path || !out_fd) {
        errno = EINVAL;
        return -1;
    }
    *out_fd = -1;
    int fd = open(path, O_RDWR | O_CREAT | O_NOFOLLOW, 0600);
    if (fd < 0) return -1;

    struct stat file_status;
    if (fstat(fd, &file_status) != 0)
        return close_after_error(fd, errno);
    if (!S_ISREG(file_status.st_mode))
        return close_after_error(fd, EINVAL);
    if (flock(fd, LOCK_EX | LOCK_NB) != 0)
        return close_after_error(fd, errno);
    if (fchmod(fd, 0600) != 0)
        return close_after_error(fd, errno);

    pid_t existing_pid = 0;
    if (read_pid_file(fd, &existing_pid) != 0)
        return close_after_error(fd, errno);
    if (existing_pid > 0 && existing_pid != getpid()) {
        errno = 0;
        if (kill(existing_pid, 0) == 0 || errno != ESRCH)
            return close_after_error(fd, EEXIST);
    }
    if (write_pid_file(fd, getpid()) != 0)
        return close_after_error(fd, errno);

    *out_fd = fd;
    return 0;
}

int runtime_instance_release(const char *path, int fd) {
    int result = 0;
    int saved_error = 0;
    if (!path || fd < 0) {
        errno = EINVAL;
        return -1;
    }

    pid_t recorded_pid = 0;
    struct stat locked_status, path_status;
    if (read_pid_file(fd, &recorded_pid) != 0) {
        result = -1;
        saved_error = errno;
    } else if (recorded_pid == getpid()) {
        if (fstat(fd, &locked_status) != 0) {
            result = -1;
            saved_error = errno;
        } else if (lstat(path, &path_status) != 0) {
            if (errno != ENOENT) {
                result = -1;
                saved_error = errno;
            }
        } else if (locked_status.st_dev == path_status.st_dev &&
                   locked_status.st_ino == path_status.st_ino &&
                   unlink(path) != 0 && errno != ENOENT) {
            result = -1;
            saved_error = errno;
        }
    }
    if (flock(fd, LOCK_UN) != 0 && result == 0) {
        result = -1;
        saved_error = errno;
    }
    if (close(fd) != 0 && result == 0) {
        result = -1;
        saved_error = errno;
    }
    if (result != 0) errno = saved_error;
    return result;
}
