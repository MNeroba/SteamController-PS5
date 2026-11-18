/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once
/* PS5 SDK v0.43 *at() stubs omit POSIX error translation. Keep the
 * contributor's call-site adapter, but establish success at the syscall
 * boundary, never by guessing whether a positive number is an fd or errno. */
#if defined(__PROSPERO__) || defined(PS5_AT_COMPAT_TEST) || defined(PS5_AT_COMPAT_TEST_TRAP)
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

int ps5_openat(int directory, const char *name, int flags, ...);
int ps5_fstatat(int directory, const char *name, struct stat *st, int flags);
int ps5_unlinkat(int directory, const char *name, int flags);
int ps5_renameat(int from_directory, const char *from, int to_directory, const char *to);
int ps5_linkat(int from_directory, const char *from, int to_directory, const char *to, int flags);
int ps5_mkdirat(int directory, const char *name, mode_t mode);

#ifdef PS5_AT_COMPAT_TEST
/* Host seam models the kernel result AND its error bit independently. */
typedef struct { long value; int failed; } ps5_at_raw_result_t;
ps5_at_raw_result_t ps5_at_test_call(long number, const long arguments[5]);
#endif

#if defined(__PROSPERO__) || defined(PS5_AT_COMPAT_REMAP_TEST)
#define openat ps5_openat
#define fstatat ps5_fstatat
#define unlinkat ps5_unlinkat
#define renameat ps5_renameat
#define linkat ps5_linkat
#define mkdirat ps5_mkdirat
#endif
#endif
