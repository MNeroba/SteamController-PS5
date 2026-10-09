/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef GHOSTPAD_RUNTIME_POLICY_H
#define GHOSTPAD_RUNTIME_POLICY_H

#include <stddef.h>

#define RUNTIME_POLICY_MAX_BYTES 1024
#define RUNTIME_POLICY_PATH "/data/ghostpad/runtime-policy.ini"

typedef struct runtime_policy {
    int enable_bridge_probe;
    int enable_game_hooks;
    int enable_ds_handoff;
    int enable_webui;
    int enable_legacy_usb_probe;
} runtime_policy_t;

/* All optional/system-facing features are off unless explicitly allowed. */
void runtime_policy_defaults(runtime_policy_t *policy);

/* Strict parser. Invalid or ambiguous input resets every option to safe-off. */
int runtime_policy_parse(const char *text, size_t length,
                         runtime_policy_t *policy);

/* Loads a bounded regular file without following symlinks. Any error leaves
 * all optional services disabled and returns -1. */
int runtime_policy_load_file(const char *path, runtime_policy_t *policy);

#endif
