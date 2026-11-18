#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE 1
#endif
#include "runtime_instance.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static int checks;
static int failures;

#define CHECK(condition) do { \
    ++checks; \
    if (!(condition)) { \
        fprintf(stderr, "FAIL %s:%d: %s (errno=%d)\n", \
                __FILE__, __LINE__, #condition, errno); \
        ++failures; \
    } \
} while (0)

static int write_recorded_pid(const char *path, pid_t pid) {
    char text[32];
    int length = snprintf(text, sizeof(text), "%ld", (long)pid);
    if (length <= 0 || (size_t)length >= sizeof(text)) return -1;
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fd < 0) return -1;
    ssize_t written = write(fd, text, (size_t)length);
    int result = written == length ? 0 : -1;
    close(fd);
    return result;
}

static pid_t read_recorded_pid(const char *path) {
    char text[32] = {0};
    int fd = open(path, O_RDONLY);
    if (fd < 0) return -1;
    ssize_t length = read(fd, text, sizeof(text) - 1);
    close(fd);
    if (length <= 0) return -1;
    return (pid_t)strtol(text, NULL, 10);
}

static void test_single_instance_and_stale_pid(void) {
    char directory[] = "/tmp/steam-controller-instance-XXXXXX";
    CHECK(mkdtemp(directory) != NULL);
    char path[256];
    snprintf(path, sizeof(path), "%s/runtime.pid", directory);

    int first_fd = -1;
    CHECK(runtime_instance_acquire(path, &first_fd) == 0);
    CHECK(first_fd >= 0);
    CHECK(read_recorded_pid(path) == getpid());
    struct stat st;
    CHECK(stat(path, &st) == 0 && (st.st_mode & 0777) == 0600);

    int second_fd = -1;
    errno = 0;
    CHECK(runtime_instance_acquire(path, &second_fd) == -1);
    CHECK(second_fd == -1);
    CHECK(runtime_instance_release(path, first_fd) == 0);
    CHECK(access(path, F_OK) == -1 && errno == ENOENT);

    int ready_pipe[2], finish_pipe[2];
    CHECK(pipe(ready_pipe) == 0);
    CHECK(pipe(finish_pipe) == 0);
    pid_t child = fork();
    CHECK(child >= 0);
    if (child == 0) {
        close(ready_pipe[0]);
        close(finish_pipe[1]);
        char ready = 'R';
        (void)write(ready_pipe[1], &ready, 1);
        close(ready_pipe[1]);
        char finish;
        (void)read(finish_pipe[0], &finish, 1);
        close(finish_pipe[0]);
        _exit(0);
    }
    close(ready_pipe[1]);
    close(finish_pipe[0]);
    char ready = 0;
    CHECK(read(ready_pipe[0], &ready, 1) == 1 && ready == 'R');
    close(ready_pipe[0]);
    CHECK(write_recorded_pid(path, child) == 0);
    errno = 0;
    CHECK(runtime_instance_acquire(path, &second_fd) == -1 && errno == EEXIST);
    CHECK(kill(child, 0) == 0); /* A live unrelated PID was observed, not signaled. */

    char finish = 'X';
    CHECK(write(finish_pipe[1], &finish, 1) == 1);
    close(finish_pipe[1]);
    int child_status = 0;
    CHECK(waitpid(child, &child_status, 0) == child &&
          WIFEXITED(child_status) && WEXITSTATUS(child_status) == 0);

    int stale_fd = -1;
    CHECK(runtime_instance_acquire(path, &stale_fd) == 0);
    CHECK(read_recorded_pid(path) == getpid());
    CHECK(runtime_instance_release(path, stale_fd) == 0);
    CHECK(rmdir(directory) == 0);
}

static void test_symlink_pid_path_is_rejected(void) {
    char directory[] = "/tmp/steam-controller-instance-link-XXXXXX";
    CHECK(mkdtemp(directory) != NULL);
    char target[256], path[256];
    snprintf(target, sizeof(target), "%s/target", directory);
    snprintf(path, sizeof(path), "%s/runtime.pid", directory);
    int fd = open(target, O_WRONLY | O_CREAT | O_EXCL, 0600);
    CHECK(fd >= 0);
    if (fd >= 0) close(fd);
    CHECK(symlink(target, path) == 0);
    int lock_fd = -1;
    CHECK(runtime_instance_acquire(path, &lock_fd) == -1);
    CHECK(lock_fd == -1);
    CHECK(access(target, F_OK) == 0);
    CHECK(unlink(path) == 0);
    CHECK(unlink(target) == 0);
    CHECK(rmdir(directory) == 0);
}

static void test_malformed_pid_file_is_not_overwritten(void) {
    char directory[] = "/tmp/steam-controller-instance-bad-XXXXXX";
    CHECK(mkdtemp(directory) != NULL);
    char path[256];
    snprintf(path, sizeof(path), "%s/runtime.pid", directory);
    int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0600);
    CHECK(fd >= 0);
    if (fd >= 0) {
        CHECK(write(fd, "123junk", 7) == 7);
        close(fd);
    }
    int lock_fd = -1;
    errno = 0;
    CHECK(runtime_instance_acquire(path, &lock_fd) == -1 && errno == EINVAL);
    CHECK(lock_fd == -1);
    char contents[16] = {0};
    fd = open(path, O_RDONLY);
    CHECK(fd >= 0);
    if (fd >= 0) {
        CHECK(read(fd, contents, sizeof(contents) - 1) == 7);
        close(fd);
    }
    CHECK(strcmp(contents, "123junk") == 0);
    CHECK(unlink(path) == 0);
    CHECK(rmdir(directory) == 0);
}

int main(void) {
    test_single_instance_and_stale_pid();
    test_symlink_pid_path_is_rejected();
    test_malformed_pid_file_is_not_overwritten();
    if (failures) return 1;
    printf("runtime instance lock checks passed (%d checks)\n", checks);
    return 0;
}
