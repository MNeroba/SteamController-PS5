/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "diagnostics.h"
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
static void *writer(void *arg) {
    (void)arg;
    for (int i = 0; i < 2000; i++)
        diagnostics_log("USB operation failure errno=5", (uint64_t)i);
    return NULL;
}
int main(void) {
    diagnostics_steam_t s = {0};
    strcpy(s.profile, "Default");
    char out[DIAGNOSTICS_REPORT_MAX];
    int n = diagnostics_report(out, sizeof(out), &s);
    assert(n > 0 && (size_t)n < sizeof(out));
    assert(strstr(out, "No Steam Controller detected") &&
           strstr(out, "Firmware (runtime): unavailable") && strstr(out, "log unavailable"));
    runtime_policy_t p = {.enable_webui = 1};
    diagnostics_runtime("initialize USB", -5, 0x13600000, &p);
    diagnostics_webui(1);
    diagnostics_vda_result(1, -5);
    diagnostics_vda_result(0, -6);
    slot_registry_token_t t = {0, 20};
    diagnostics_usb_begin(t, "/dev/ugen1.2", 0x28de, 0x1304);
    diagnostics_usb_endpoints(t, 2, 0x84, 0x04);
    diagnostics_usb_stage(t, "FS_INIT failed", -5);
    usb_resources_t r = {.starts = 4, .timeouts = 2, .failures = 1, .late_completions = 3};
    diagnostics_usb_snapshot(t, &r, 1);
    slot_registry_entry_t e = {.state = SLOT_REGISTRY_ACTIVE, .virtual_ready = 1};
    user_session_snapshot_t u = {.read_ok = 1, .logged_users = 1, .logout_epoch = 2};
    slot_recovery_t recovery = {.pending = 1, .deletion_failures = 2};
    diagnostics_virtual(t, &e, &u, &recovery);
    diagnostics_usb_input(t, 64, 0x42, 1400, 1);
    diagnostics_usb_input(t, 65, 0, 1500, 0);
    diagnostics_log("ordinary error=0x803B0006\nESC:\033", 1500);
    const char *private[] = {
        "Bearer supersecret",
        "Pairing code: 12345678",
        "serial=abc123",
        "BondingKey=abc",
        "userId=0x10000000",
        "MAC:01:23:45:67:89:ab",
        "token=0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"};
    for (unsigned i = 0; i < sizeof(private) / sizeof(private[0]); i++)
        diagnostics_log(private[i], 1600 + i);
    diagnostics_log("0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef", 1700);
    diagnostics_log("01:23:45:67:89:ab", 1800);
    diagnostics_log("12345678", 1801);
    strcpy(s.profile, "My\nProfile\033");
    s.steam.haptic_last_err = -5;
    s.gyro_imu = 1;
    n = diagnostics_report(out, sizeof(out), &s);
    assert(n > 0);
    assert(strstr(out, "create-attempts=1 failures=1") &&
           strstr(out, "delete-attempts=1 failures=1"));
    assert(strstr(out, "13.60") && strstr(out, "FS_INIT failed") && strstr(out, "Valve Puck"));
    assert(strstr(out, "0x803B0006") && strstr(out, "My\\x0aProfile\\x1b"));
    assert(!strstr(out, "supersecret") && !strstr(out, "12345678") && !strstr(out, "01:23:45") &&
           !strstr(out, "0123456789abcdef") && !strstr(out, "0x10000000") && !strchr(out, '\033'));
    char huge[8192];memset(huge,'x',sizeof(huge));huge[sizeof(huge)-1]=0;
    diagnostics_log(huge,1900);assert(diagnostics_report(out,sizeof(out),&s)>0);
    char small[32];
    assert(diagnostics_report(small, sizeof(small), &s) == -1 && !small[0]);
    for (int i = 0; i < 60; i++) {
        char line[80];
        snprintf(line, sizeof(line), "bounded record %d", i);
        diagnostics_log(line, 2000 + i);
    }
    n = diagnostics_report(out, sizeof(out), &s);
    assert(n > 0 && strstr(out, "bounded record 59") && !strstr(out, "bounded record 0\n"));
    /* Old generation can never overwrite a newly observed USB session. */
    diagnostics_usb_begin((slot_registry_token_t){0, 21}, "/dev/ugen1.3", 0x28de, 0x1302);
    diagnostics_usb_stage(t, "stale-session-error", -999);
    diagnostics_usb_input(t, 99, 99, 99, 1);
    assert(diagnostics_report(out, sizeof(out), &s) > 0 && !strstr(out, "stale-session-error"));
    pthread_t thread;
    assert(pthread_create(&thread, NULL, writer, NULL) == 0);
    for (int i = 0; i < 2000; i++)
        assert(diagnostics_report(out, sizeof(out), &s) > 0);
    assert(pthread_join(thread, NULL) == 0);
    puts("bounded diagnostic generation/redaction/escaping/concurrency checks passed");
    return 0;
}
