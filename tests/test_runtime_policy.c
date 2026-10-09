/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE
#endif
#include "runtime_policy.h"

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int all_disabled(const runtime_policy_t *p)
{
    return !p->enable_bridge_probe && !p->enable_game_hooks &&
           !p->enable_ds_handoff && !p->enable_webui;
}

static void test_defaults_are_safe(void)
{
    runtime_policy_t p;
    memset(&p, 0xff, sizeof(p));
    runtime_policy_defaults(&p);
    assert(all_disabled(&p));
}

static void test_explicit_opt_ins(void)
{
    static const char config[] =
        "# optional features are opt-in\n"
        "ENABLE_BRIDGE_PROBE=1\n"
        "ENABLE_GAME_HOOKS=1\n"
        "ENABLE_DUALSENSE_HANDOFF=1\n"
        "ENABLE_WEBUI=1\n";
    runtime_policy_t p;
    assert(runtime_policy_parse(config, sizeof(config) - 1, &p) == 0);
    assert(p.enable_bridge_probe && p.enable_game_hooks &&
           p.enable_ds_handoff && p.enable_webui);
}

static void test_partial_opt_in_and_whitespace(void)
{
    static const char config[] =
        " # comment\r\n"
        " ENABLE_WEBUI = 1 \r\n"
        "ENABLE_GAME_HOOKS=0\n";
    runtime_policy_t p;
    assert(runtime_policy_parse(config, sizeof(config) - 1, &p) == 0);
    assert(p.enable_webui && !p.enable_game_hooks);
    assert(!p.enable_bridge_probe && !p.enable_ds_handoff);
}

static void test_invalid_policy_fails_closed(void)
{
    static const char *invalid[] = {
        "ENABLE_WEBUI=2\n",
        "ENABLE_WEBUI=1\nENABLE_WEBUI=0\n",
        "ENABLE_UNKNOWN=1\n",
        "ENABLE_GAME_HOOKS=yes\n",
        "ENABLE_WEBUI\n"
    };
    for (size_t i = 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
        runtime_policy_t p;
        memset(&p, 0xff, sizeof(p));
        assert(runtime_policy_parse(invalid[i], strlen(invalid[i]), &p) != 0);
        assert(all_disabled(&p));
    }
}

static void test_empty_and_oversize(void)
{
    runtime_policy_t p;
    assert(runtime_policy_parse("", 0, &p) == 0);
    assert(all_disabled(&p));
    char oversized[RUNTIME_POLICY_MAX_BYTES + 1];
    memset(oversized, ' ', sizeof(oversized));
    assert(runtime_policy_parse(oversized, sizeof(oversized), &p) != 0);
    assert(all_disabled(&p));
    static const char embedded_nul[] = {
        'E','N','A','B','L','E','_','W','E','B','U','I','=','1','\0','\n'
    };
    assert(runtime_policy_parse(embedded_nul, sizeof(embedded_nul), &p) != 0);
    assert(all_disabled(&p));
}

static void write_file(const char *path, const char *data, size_t length)
{
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    assert(fd >= 0);
    size_t written = 0;
    while (written < length) {
        ssize_t count = write(fd, data + written, length - written);
        assert(count > 0);
        written += (size_t)count;
    }
    assert(close(fd) == 0);
}

static void test_file_loader_fails_closed(void)
{
    char directory[] = "/tmp/runtime-policy-test-XXXXXX";
    assert(mkdtemp(directory) != NULL);
    char config[160], link_path[160];
    assert(snprintf(config, sizeof(config), "%s/policy.ini", directory) > 0);
    assert(snprintf(link_path, sizeof(link_path), "%s/policy-link.ini", directory) > 0);

    runtime_policy_t p;
    memset(&p, 0xff, sizeof(p));
    assert(runtime_policy_load_file(config, &p) != 0);
    assert(all_disabled(&p));

    static const char valid[] = "ENABLE_WEBUI=1\n";
    write_file(config, valid, sizeof(valid) - 1);
    assert(runtime_policy_load_file(config, &p) == 0);
    assert(p.enable_webui && !p.enable_game_hooks && !p.enable_ds_handoff);

    assert(symlink(config, link_path) == 0);
    memset(&p, 0xff, sizeof(p));
    assert(runtime_policy_load_file(link_path, &p) != 0);
    assert(all_disabled(&p));

    char oversized[RUNTIME_POLICY_MAX_BYTES + 1];
    memset(oversized, ' ', sizeof(oversized));
    write_file(config, oversized, sizeof(oversized));
    memset(&p, 0xff, sizeof(p));
    assert(runtime_policy_load_file(config, &p) != 0);
    assert(all_disabled(&p));

    memset(&p, 0xff, sizeof(p));
    assert(runtime_policy_load_file(directory, &p) != 0);
    assert(all_disabled(&p));
    assert(unlink(link_path) == 0);
    assert(unlink(config) == 0);
    assert(rmdir(directory) == 0);
}

int main(void)
{
    test_defaults_are_safe();
    test_explicit_opt_ins();
    test_partial_opt_in_and_whitespace();
    test_invalid_policy_fails_closed();
    test_empty_and_oversize();
    test_file_loader_fails_closed();
    puts("runtime safe-mode policy checks passed");
    return 0;
}
