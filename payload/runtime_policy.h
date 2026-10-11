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
} runtime_policy_t;

/* Authenticated WebUI is on; intrusive/system-facing features are off. */
void runtime_policy_defaults(runtime_policy_t *policy);

/* Strict parser. Invalid or ambiguous input resets every option to safe-off. */
int runtime_policy_parse(const char *text, size_t length,
                         runtime_policy_t *policy);

#define RUNTIME_POLICY_MISSING 1
/* 0: valid file, 1: missing file with first-launch defaults, -1: invalid,
 * unsafe or inaccessible storage with every optional service disabled. */
int runtime_policy_load_file(const char *path, runtime_policy_t *policy);

#endif
