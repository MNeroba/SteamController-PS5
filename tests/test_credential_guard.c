/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "credential_guard.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

typedef struct fake_credentials {
    uint64_t authid;
    uint8_t caps[CREDENTIAL_GUARD_CAPS_SIZE];
    int fail_read_authid;
    int fail_read_caps;
    int fail_write_authid_at;
    int fail_write_caps_at;
    int authid_writes;
    int caps_writes;
    int operation_calls;
    int fail_operation;
    char calls[32];
    size_t call_count;
} fake_credentials_t;

static void record(fake_credentials_t *f, char call)
{
    assert(f->call_count < sizeof(f->calls));
    f->calls[f->call_count++] = call;
}

static int read_authid(void *context, uint64_t *out)
{
    fake_credentials_t *f = context;
    record(f, 'A');
    if (f->fail_read_authid) return -1;
    *out = f->authid;
    return 0;
}

static int read_caps(void *context, uint8_t out[CREDENTIAL_GUARD_CAPS_SIZE])
{
    fake_credentials_t *f = context;
    record(f, 'C');
    if (f->fail_read_caps) return -1;
    memcpy(out, f->caps, sizeof(f->caps));
    return 0;
}

static int write_authid(void *context, uint64_t authid)
{
    fake_credentials_t *f = context;
    record(f, 'a');
    ++f->authid_writes;
    if (f->fail_write_authid_at == f->authid_writes) return -1;
    f->authid = authid;
    return 0;
}

static int write_caps(void *context, const uint8_t caps[CREDENTIAL_GUARD_CAPS_SIZE])
{
    fake_credentials_t *f = context;
    record(f, 'c');
    ++f->caps_writes;
    if (f->fail_write_caps_at == f->caps_writes) return -1;
    memcpy(f->caps, caps, sizeof(f->caps));
    return 0;
}

static int run_operation(void *context)
{
    fake_credentials_t *f = context;
    ++f->operation_calls;
    assert(f->authid == UINT64_C(0x4800000000010003));
    assert(f->caps[0] == 0xff && f->caps[15] == 0xff);
    errno = EBUSY;
    return f->fail_operation ? -17 : 42;
}

static const credential_guard_ops_t ops = {
    read_authid, read_caps, write_authid, write_caps
};

static fake_credentials_t make_fake(void)
{
    fake_credentials_t f = {0};
    f.authid = UINT64_C(0x3800000000010003);
    for (size_t i = 0; i < sizeof(f.caps); ++i)
        f.caps[i] = (uint8_t)(i + 1);
    return f;
}

static void assert_original(const fake_credentials_t *f)
{
    assert(f->authid == UINT64_C(0x3800000000010003));
    for (size_t i = 0; i < sizeof(f->caps); ++i)
        assert(f->caps[i] == (uint8_t)(i + 1));
}

static void test_success_and_errno(void)
{
    fake_credentials_t f = make_fake();
    uint8_t elevated[CREDENTIAL_GUARD_CAPS_SIZE];
    memset(elevated, 0xff, sizeof(elevated));
    int operation_result = 0;
    errno = 0;
    assert(credential_guard_run(&ops, &f, UINT64_C(0x4800000000010003),
                                elevated, run_operation, &f,
                                &operation_result) == CREDENTIAL_GUARD_OK);
    assert(operation_result == 42);
    assert(errno == EBUSY);
    assert(f.operation_calls == 1);
    assert_original(&f);
    assert(f.calls[2] == 'a' && f.calls[3] == 'c');
    assert(f.calls[4] == 'c' && f.calls[5] == 'a');
}

static void test_read_failures_do_not_mutate(void)
{
    fake_credentials_t f = make_fake();
    f.fail_read_authid = 1;
    credential_guard_snapshot_t snapshot = {0};
    uint8_t caps[CREDENTIAL_GUARD_CAPS_SIZE] = {0};
    assert(credential_guard_begin(&ops, &f, 99, caps, &snapshot) ==
           CREDENTIAL_GUARD_READ_FAILED);
    assert(f.authid_writes == 0 && f.caps_writes == 0);
    assert(!snapshot.active);

    f = make_fake();
    f.fail_read_caps = 1;
    assert(credential_guard_begin(&ops, &f, 99, caps, &snapshot) ==
           CREDENTIAL_GUARD_READ_FAILED);
    assert(f.authid_writes == 0 && f.caps_writes == 0);
    assert(!snapshot.active);
}

static void test_failed_authid_set_rolls_back(void)
{
    fake_credentials_t f = make_fake();
    f.fail_write_authid_at = 1;
    credential_guard_snapshot_t snapshot = {0};
    assert(credential_guard_begin(&ops, &f, 99, NULL, &snapshot) ==
           CREDENTIAL_GUARD_ELEVATION_FAILED);
    assert_original(&f);
    assert(f.authid_writes == 2);
    assert(!snapshot.active);
}

static void test_failed_caps_set_rolls_back_both(void)
{
    fake_credentials_t f = make_fake();
    f.fail_write_caps_at = 1;
    credential_guard_snapshot_t snapshot = {0};
    uint8_t caps[CREDENTIAL_GUARD_CAPS_SIZE];
    memset(caps, 0xff, sizeof(caps));
    assert(credential_guard_begin(&ops, &f, 99, caps, &snapshot) ==
           CREDENTIAL_GUARD_ELEVATION_FAILED);
    assert_original(&f);
    assert(f.calls[2] == 'a' && f.calls[3] == 'c');
    assert(f.calls[4] == 'c' && f.calls[5] == 'a');
    assert(!snapshot.active);
}

static void test_restore_failure_keeps_retryable_scope(void)
{
    fake_credentials_t f = make_fake();
    credential_guard_snapshot_t snapshot = {0};
    uint8_t caps[CREDENTIAL_GUARD_CAPS_SIZE];
    memset(caps, 0xff, sizeof(caps));
    assert(credential_guard_begin(&ops, &f, 99, caps, &snapshot) ==
           CREDENTIAL_GUARD_OK);
    f.fail_write_caps_at = f.caps_writes + 1;
    assert(credential_guard_restore(&ops, &f, &snapshot) ==
           CREDENTIAL_GUARD_RESTORE_FAILED);
    assert(snapshot.active);
    assert(f.authid == UINT64_C(0x3800000000010003));
    assert(f.caps[0] == 0xff);
    f.fail_write_caps_at = 0;
    assert(credential_guard_restore(&ops, &f, &snapshot) ==
           CREDENTIAL_GUARD_OK);
    assert_original(&f);
    assert(!snapshot.active);
}

static void test_operation_result_does_not_skip_restore(void)
{
    fake_credentials_t f = make_fake();
    f.fail_operation = 1;
    uint8_t caps[CREDENTIAL_GUARD_CAPS_SIZE];
    memset(caps, 0xff, sizeof(caps));
    int operation_result = 0;
    assert(credential_guard_run(&ops, &f,
                                UINT64_C(0x4800000000010003), caps,
                                run_operation, &f,
                                &operation_result) == CREDENTIAL_GUARD_OK);
    assert(operation_result == -17);
    assert_original(&f);
}

static void test_run_retries_transient_restore_failure(void)
{
    fake_credentials_t f = make_fake();
    f.fail_write_caps_at = 2; /* elevation succeeds; first restore fails */
    uint8_t caps[CREDENTIAL_GUARD_CAPS_SIZE];
    memset(caps, 0xff, sizeof(caps));
    int operation_result = 0;
    assert(credential_guard_run(&ops, &f,
                                UINT64_C(0x4800000000010003), caps,
                                run_operation, &f,
                                &operation_result) == CREDENTIAL_GUARD_OK);
    assert(operation_result == 42);
    assert(f.caps_writes == 3);
    assert_original(&f);
}

int main(void)
{
    test_success_and_errno();
    test_read_failures_do_not_mutate();
    test_failed_authid_set_rolls_back();
    test_failed_caps_set_rolls_back_both();
    test_restore_failure_keeps_retryable_scope();
    test_operation_result_does_not_skip_restore();
    test_run_retries_transient_restore_failure();
    puts("credential guard tests passed");
    return 0;
}
