/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef RUNTIME_INSTANCE_H
#define RUNTIME_INSTANCE_H

/* Acquire and retain an exclusive lock on the runtime PID file. A recorded
 * live PID is never signaled; acquisition fails with EEXIST. On success,
 * release removes the path only when it still names this process's file. */
int runtime_instance_acquire(const char *path, int *out_fd);
int runtime_instance_release(const char *path, int fd);

#endif
