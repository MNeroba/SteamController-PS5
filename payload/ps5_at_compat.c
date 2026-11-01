/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "ps5_at_compat.h"
#if defined(__PROSPERO__) || defined(PS5_AT_COMPAT_TEST) || defined(PS5_AT_COMPAT_TEST_TRAP)
#include <errno.h>
#include <limits.h>
#include <stdarg.h>

#ifndef PS5_AT_COMPAT_TEST
typedef struct { long value; int failed; } ps5_at_raw_result_t;
#endif

/* Numbers and amd64 calling convention are pinned to community SDK v0.43;
 * see docs/PROVENANCE.md. FreeBSD signals failure using CF with positive
 * errno in RAX. The SDK stubs discard CF; inode inspection cannot recover it.
 * Only the six existing filesystem calls below can reach this boundary. */
static ps5_at_raw_result_t call_at(long number, long a1, long a2, long a3, long a4, long a5) {
#ifdef PS5_AT_COMPAT_TEST
    const long arguments[5] = {a1, a2, a3, a4, a5};
    return ps5_at_test_call(number, arguments);
#else
#if !defined(__x86_64__)
#error PS5 filesystem syscall adapter requires the amd64 target ABI
#endif
    _Static_assert(sizeof(long) == 8, "PS5 syscall arguments require LP64");
    register long fourth __asm__("r10") = a4;
    register long fifth __asm__("r8") = a5;
    int failed;
    __asm__ volatile(
#ifdef PS5_AT_COMPAT_TEST_TRAP
        "call ps5_at_test_trap"
#else
        "syscall"
#endif
        : "+a"(number), "=@ccc"(failed), "+D"(a1), "+S"(a2), "+d"(a3),
          "+r"(fourth), "+r"(fifth)
        :
        : "rcx", "r11", "memory");
    return (ps5_at_raw_result_t){number, failed};
#endif
}

static int posix_result(ps5_at_raw_result_t result) {
    if (result.failed) {
        errno = result.value > 0 && result.value <= INT_MAX ? (int)result.value : EIO;
        return -1;
    }
    return (int)result.value;
}

int ps5_openat(int directory, const char *name, int flags, ...) {
    mode_t mode = 0;
    if (flags & O_CREAT) {
        va_list ap;
        va_start(ap, flags);
        mode = (mode_t)va_arg(ap, int);
        va_end(ap);
    }
    return posix_result(call_at(499, directory, (long)name, flags, mode, 0));
}
int ps5_fstatat(int directory, const char *name, struct stat *st, int flags) {
    return posix_result(call_at(493, directory, (long)name, (long)st, flags, 0));
}
int ps5_unlinkat(int directory, const char *name, int flags) {
    return posix_result(call_at(503, directory, (long)name, flags, 0, 0));
}
int ps5_renameat(int from_directory, const char *from, int to_directory, const char *to) {
    return posix_result(call_at(501, from_directory, (long)from, to_directory, (long)to, 0));
}
int ps5_linkat(int from_directory, const char *from, int to_directory, const char *to, int flags) {
    return posix_result(call_at(495, from_directory, (long)from, to_directory, (long)to, flags));
}
int ps5_mkdirat(int directory, const char *name, mode_t mode) {
    return posix_result(call_at(496, directory, (long)name, mode, 0, 0));
}
#endif
