/* SPDX-License-Identifier: GPL-3.0-or-later */
#define _POSIX_C_SOURCE 200809L
#define _DARWIN_C_SOURCE
#include <stdio.h>
#if defined(__x86_64__)
#include <assert.h>
#include <errno.h>
#define PS5_AT_COMPAT_TEST_TRAP
/* Execute the production register/CF constraints with a trap that NEVER
 * enters the kernel. Explicit names also work on x86_64 Darwin. */
volatile long test_value __asm__("ps5_test_value");
volatile int test_failed __asm__("ps5_test_failed");
volatile long test_observed[6] __asm__("ps5_test_observed");
__attribute__((naked, no_sanitize("address", "undefined")))
void test_trap(void) __asm__("ps5_at_test_trap");
void test_trap(void) {
    __asm__ volatile(
        "movq %rax, ps5_test_observed(%rip)\n\t"
        "movq %rdi, ps5_test_observed+8(%rip)\n\t"
        "movq %rsi, ps5_test_observed+16(%rip)\n\t"
        "movq %rdx, ps5_test_observed+24(%rip)\n\t"
        "movq %r10, ps5_test_observed+32(%rip)\n\t"
        "movq %r8, ps5_test_observed+40(%rip)\n\t"
        "movq ps5_test_value(%rip), %rax\n\t"
        "cmpl $0, ps5_test_failed(%rip)\n\t"
        "je 1f\n\tstc\n\tret\n1:\n\tclc\n\tret");
}
#include "../payload/ps5_at_compat.c"
int main(void) {
    const char *from = "from", *to = "to";
    test_value = 24; test_failed = 0; errno = EBUSY;
    assert(ps5_openat(11, from, O_CREAT | O_EXCL, 0600) == 24 && errno == EBUSY);
    assert(test_observed[0] == 499 && test_observed[1] == 11 && test_observed[2] == (long)from);
    assert(test_observed[3] == (O_CREAT | O_EXCL) && test_observed[4] == 0600 && test_observed[5] == 0);
    test_failed = 1;
    assert(ps5_openat(11, from, O_RDONLY) == -1 && errno == 24);
    test_failed = 0; test_value = 0;
    assert(ps5_linkat(12, from, 13, to, AT_SYMLINK_FOLLOW) == 0);
    assert(test_observed[0] == 495 && test_observed[1] == 12 && test_observed[2] == (long)from);
    assert(test_observed[3] == 13 && test_observed[4] == (long)to && test_observed[5] == AT_SYMLINK_FOLLOW);
    test_failed = 1; test_value = ENOENT;
    assert(ps5_unlinkat(12, from, 0) == -1 && errno == ENOENT);
    puts("PS5 amd64 syscall register and carry-flag regressions passed (no kernel syscall)");
    return 0;
}
#else
int main(void) {
    puts("SKIP: amd64 register/CF execution requires x86_64; Ubuntu CI runs this test");
    return 0;
}
#endif
