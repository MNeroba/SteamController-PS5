/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE
#endif
#include "runtime_policy.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

enum {
    POLICY_BRIDGE = 1u << 0,
    POLICY_HOOKS = 1u << 1,
    POLICY_HANDOFF = 1u << 2,
    POLICY_WEBUI = 1u << 3
};

void
runtime_policy_defaults(runtime_policy_t *policy)
{
    if (policy)
        memset(policy, 0, sizeof(*policy));
}

static char *
trim_space(char *text)
{
    while (*text == ' ' || *text == '\t')
        ++text;
    size_t length = strlen(text);
    while (length && (text[length - 1] == ' ' || text[length - 1] == '\t' ||
                      text[length - 1] == '\r'))
        text[--length] = '\0';
    return text;
}

static int
policy_key(runtime_policy_t *policy, const char *key, int value)
{
    if (strcmp(key, "ENABLE_BRIDGE_PROBE") == 0)
        policy->enable_bridge_probe = value;
    else if (strcmp(key, "ENABLE_GAME_HOOKS") == 0)
        policy->enable_game_hooks = value;
    else if (strcmp(key, "ENABLE_DUALSENSE_HANDOFF") == 0)
        policy->enable_ds_handoff = value;
    else if (strcmp(key, "ENABLE_WEBUI") == 0)
        policy->enable_webui = value;
    else
        return -1;
    return 0;
}

int
runtime_policy_parse(const char *text, size_t length,
                     runtime_policy_t *policy)
{
    if (!policy)
        return -1;
    runtime_policy_defaults(policy);
    if ((!text && length) || length > RUNTIME_POLICY_MAX_BYTES ||
        (text && memchr(text, '\0', length) != NULL))
        return -1;

    char buffer[RUNTIME_POLICY_MAX_BYTES + 1];
    if (length)
        memcpy(buffer, text, length);
    buffer[length] = '\0';

    unsigned seen = 0;
    size_t offset = 0;
    while (offset < length) {
        size_t end = offset;
        while (end < length && buffer[end] != '\n')
            ++end;
        buffer[end] = '\0';
        char *line = trim_space(buffer + offset);
        offset = end + 1;
        if (!*line || *line == '#')
            continue;

        char *equals = strchr(line, '=');
        if (!equals)
            goto invalid;
        *equals = '\0';
        char *key = trim_space(line);
        char *value_text = trim_space(equals + 1);
        if (!*key || (strcmp(value_text, "0") != 0 &&
                      strcmp(value_text, "1") != 0))
            goto invalid;

        unsigned bit;
        if (strcmp(key, "ENABLE_BRIDGE_PROBE") == 0)
            bit = POLICY_BRIDGE;
        else if (strcmp(key, "ENABLE_GAME_HOOKS") == 0)
            bit = POLICY_HOOKS;
        else if (strcmp(key, "ENABLE_DUALSENSE_HANDOFF") == 0)
            bit = POLICY_HANDOFF;
        else if (strcmp(key, "ENABLE_WEBUI") == 0)
            bit = POLICY_WEBUI;
        else
            goto invalid;
        if (seen & bit)
            goto invalid;
        seen |= bit;
        if (policy_key(policy, key, value_text[0] == '1') != 0)
            goto invalid;
    }
    return 0;

invalid:
    runtime_policy_defaults(policy);
    return -1;
}

int
runtime_policy_load_file(const char *path, runtime_policy_t *policy)
{
    if (!policy)
        return -1;
    runtime_policy_defaults(policy);
    if (!path || !*path)
        return -1;

    int fd = open(path, O_RDONLY | O_NOFOLLOW);
    if (fd < 0)
        return -1;

    struct stat st;
    if (fstat(fd, &st) != 0 || !S_ISREG(st.st_mode) ||
        st.st_size < 0 || st.st_size > RUNTIME_POLICY_MAX_BYTES) {
        close(fd);
        return -1;
    }

    char text[RUNTIME_POLICY_MAX_BYTES + 1];
    size_t used = 0;
    int failed = 0;
    while (used < sizeof(text)) {
        ssize_t count = read(fd, text + used, sizeof(text) - used);
        if (count < 0 && errno == EINTR)
            continue;
        if (count < 0) {
            failed = 1;
            break;
        }
        if (count == 0)
            break;
        used += (size_t)count;
    }
    close(fd);
    if (failed || used > RUNTIME_POLICY_MAX_BYTES ||
        runtime_policy_parse(text, used, policy) != 0) {
        runtime_policy_defaults(policy);
        return -1;
    }
    return 0;
}
