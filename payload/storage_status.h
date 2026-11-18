/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once
#include <errno.h>

/* Fixed stage labels and errno only. Never store paths, UIDs or file contents. */
typedef enum {
    STORAGE_OK, STORAGE_ARGUMENT, STORAGE_DIRECTORY_OPEN, STORAGE_DIRECTORY_CREATE,
    STORAGE_DIRECTORY_STAT, STORAGE_DIRECTORY_TYPE, STORAGE_DIRECTORY_OWNER,
    STORAGE_DIRECTORY_CHMOD, STORAGE_DIRECTORY_MODE, STORAGE_DIRECTORY_CLOSE,
    STORAGE_TOKEN_OPEN, STORAGE_TOKEN_STAT, STORAGE_TOKEN_TYPE, STORAGE_TOKEN_OWNER,
    STORAGE_TOKEN_MODE, STORAGE_TOKEN_SIZE, STORAGE_TOKEN_READ, STORAGE_TOKEN_FORMAT,
    STORAGE_TOKEN_INSPECT, STORAGE_TOKEN_CREATE, STORAGE_TOKEN_CHMOD, STORAGE_TOKEN_WRITE,
    STORAGE_TOKEN_SYNC, STORAGE_TOKEN_CLOSE, STORAGE_RANDOM_OPEN, STORAGE_RANDOM_READ,
    STORAGE_RANDOM_CLOSE
} storage_stage_t;
typedef struct { storage_stage_t stage; int error; } storage_status_t;
static inline void storage_status_clear(storage_status_t *s) {
    if (s) { s->stage=STORAGE_OK; s->error=0; }
}
static inline int storage_failure(storage_status_t *s, storage_stage_t stage, int error) {
    if (!error) error=EIO;
    if (s) { s->stage=stage; s->error=error; }
    errno=error; return -1;
}
static inline const char *storage_stage_name(storage_stage_t stage) {
    static const char *const names[]={
        "ok", "argument", "directory-open", "directory-create", "directory-stat",
        "directory-type", "directory-owner", "directory-chmod", "directory-mode",
        "directory-close", "file-open", "file-stat", "file-type", "file-owner",
        "file-mode", "file-size", "file-read", "file-format", "file-inspect",
        "file-create", "file-chmod", "file-write", "file-sync", "file-close",
        "random-open", "random-read", "random-close"
    };
    return (unsigned)stage<sizeof(names)/sizeof(names[0]) ? names[stage] : "unknown";
}
