# PR #8 review — 2026-11-01 (Europe/Riga)

Review only; no merge, external review/comment, source edit, release or hardware operation.
PR: https://github.com/MNeroba/SteamController-PS5/pull/8
Base: a446e25e7995348949036bc6b62d3cf87e9bc97f
Head: c3a7ddb4f721b512e294dea23ed67e4d5cd1ef1a (rechecked unchanged at end)
Clean review checkout: /private/tmp/steamcontroller-pr8-review. The inherited desktop overlay was not used as the PR source.

## Findings — changes required

### PR8-R1 — P1: errno can alias an existing descriptor to the same inode

payload/ps5_at_compat.h:61–66. The wrapper cannot establish descriptor ownership by comparing st_dev/st_ino. With an existing descriptor 24 pointing to the requested file and descriptor exhaustion, SDK-style openat failure returns positive EMFILE (24). fstat(24) succeeds and matches the requested inode, so ps5_openat returns an already-owned descriptor as a newly acquired one. A caller then reads/writes/closes another operation's descriptor. This defeats the ownership premise even without a path race.

VERIFIED_HOST_TEST: reproduced with real files, dup2 to fd 24, actual RLIMIT_NOFILE exhaustion, and only a boundary adapter converting host -1/errno into the SDK positive-error convention. Result: wrapper returned 24; pre-existing same-file fd=24; newly acquired=-1. ASan/UBSan produced no unrelated sanitizer failure. Expected-red process rc=1.

### PR8-R2 — P1: real descriptors leak on post-open validation failure

payload/ps5_at_compat.h:63–73. A profile load can race the existing atomic rename in sc2_store_save (loads do not hold g_store_lock). If replacement occurs between pre-stat and open, the returned descriptor validly refers to the new inode. The wrapper rejects it, invents errno from its fd number, and returns without releasing it. Repeated races exhaust descriptors. Similarly a transient post-open fstat failure hides a successful O_EXCL create from its caller; the caller cannot close or remove the newly created empty token/temp file.

VERIFIED_HOST_TEST: real renameat after the pre-stat and before the real open, with SDK-style error adaptation. Result: returned -1 errno=4; acquired fd=4 still open=1. The harness closed it only after recording the leak. Separately injected fstat/EINTR after a successful create: acquired=1, released=0. Expected-red rc=1.

Recommended correction: normalize filesystem errors at a syscall boundary that retains the actual error indication; do not infer acquisition or failure from the numeric return plus inode. SDK v0.43 libc/syscalls.c stubs issue syscall/ret without POSIX errno translation (openat:1901; fstatat:1841; linkat:1861; mkdirat:1871; renameat:1921; unlinkat:1941). Preserve existing nofollow/exclusive-create/owner/permission checks and test the actual target compatibility layer on the host through injectable syscall results.

## Useful fixes and evidence limits

The 1 MiB WebUI worker stack is a bounded change (four clients). Existing real-listener/auth/shutdown tests pass with it, normally and under ASan/UBSan. The author's PS5 stack measurements and HTTP/VDI/menu results on one FW 13.20 fat console/Puck are EXTERNAL_HARDWARE reports, not our independently performed hardware tests. Native DualSense coexistence and games were explicitly not systematically tested.

The klog bound increase retains nonblocking reads and a finite limit. An independent scripted-read regression fails against base at 128 KiB and 4095 KiB, passes against the PR at 0/128/4095 KiB, and confirms fail-closed EOVERFLOW with no stale events at 4096 KiB. There is no new device-deletion, native-DualSense mutation, ShellUI patch, BLE operation or ownership-ABI proof in this diff.

Correction to earlier M1-248 interpretation: SDK v0.43 include/freebsd/sys/errno.h:163 defines EOVERFLOW=84 and :165 EILSEQ=86. The previous diagnosis of tester errno84 as Linux EILSEQ was wrong. Historical entries are retained; this record supersedes that interpretation. The base drain itself synthesizes EOVERFLOW at its iteration limit. Raising the bound addresses that cause, rather than validating the earlier EILSEQ fallback.

SDK evidence pin: d9c9519116944a7f1c22d262012c53b80b3520b7, https://github.com/ps5-payload-dev/sdk/tree/d9c9519116944a7f1c22d262012c53b80b3520b7 . No SDK code copied into production.

## Commands actually executed

Environment: Darwin arm64; Apple clang 21.0.0 (clang-2100.4.1.1). All checks below use exact PR head above.

- git diff --check a446e25e7995348949036bc6b62d3cf87e9bc97f HEAD: PASS, rc=0.
- make -C tests clean test: compiled, then stopped at listener test because sandbox bind returned EPERM (errno1). NOT a PR test failure; negative attempt retained.
- make -C tests test (loopback permitted): PASS rc=0; all 48 C test targets and 11 Python test files. Log /private/tmp/pr8-review-tests/host-unrestricted.log.
- make -C tests sanitize (loopback permitted): PASS rc=0; full ASan/UBSan suite. Log /private/tmp/pr8-review-tests/sanitize.log.
- clang -std=c11 -O1 -g -Wall -Wextra -fsanitize=address,undefined -fno-omit-frame-pointer /private/tmp/pr8-review-tests/compat_real_files.c -o /private/tmp/pr8-review-tests/compat_real_files: PASS compilation; executable intentionally FAIL rc=1 demonstrating R1/R2 above.
- Additional mocked target-layer /private/tmp/pr8-review-tests/compat_repro.c: normal success PASS; errno/fd alias, atomic replacement, post-create fstat/EINTR FAIL as expected. Also exposes O_CREAT failing if the underlying library is later changed to ordinary -1/ENOENT semantics; that last case is not treated as a current SDK v0.43 blocker.
- Independent /private/tmp/pr8-review-tests/klog_bound.c built with -DVDA_PENDING_TEST and ASan/UBSan against PR vda_pending.c: PASS four boundary scenarios; same test against git-show base vda_pending.c: expected FAIL at 128/4095 KiB.
- PS5 cross-build: NOT_RUN locally, PS5_PAYLOAD_SDK unset; contributor's reported v0.43/LLVM18 build is external evidence only.
- Browser script: NOT_RUN during this review. Real HTTP listener/auth tests above ran; no frontend changes in PR.
- Physical PS5: NOT_RUN by reviewer.
- GitHub PR workflows 41262660977 (SDK), 41262660994 (secret scan), 41262661014 (Host): action_required, awaiting GitHub first-contributor approval, not successful CI. No approval/rerun was submitted during this review.

Existing normal host tests do not compile __PROSPERO__ and thus do not execute the new compatibility header. Their success does not cover R1/R2. Add dedicated target-layer regression coverage before merging that commit.

## Next action

Rework or split the *at() compatibility commit, using unambiguous syscall error handling; retain the stack/klog fixes. Add regressions for fd/errno collision, rename races, fstat failures, exclusive creation and both cleanup/ownership outcomes. Rerun Host/ASan/UBSan, browser and clean SDK v0.43 checks on the revised exact head, then obtain sanitized hardware results separately. No merge/release authorized by this review request.

## Real-file reproducer (review artifact, not production code)

The executable deliberately exits 1 when both bugs are observed; the adapter changes only the host error return convention to match the SDK stubs. All files and resource-limit changes are confined to this test process and its temporary directory.

```c
#define _POSIX_C_SOURCE 200809L
#define _DARWIN_C_SOURCE
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <unistd.h>
static int replace_after_stat, replaced, last_fd=-1;
/* Simulate only the SDK's raw positive-errno convention. All filesystem
 * operations, descriptor allocation and fstat inode checks are real. */
static int target_fstatat(int dir, const char *path, struct stat *st, int flags) {
    int r=fstatat(dir,path,st,flags),e=errno;
    if (!r && replace_after_stat && !replaced) {
        assert(renameat(dir,"replacement",dir,"target")==0); replaced=1;
    }
    return r<0?e:r;
}
static int target_openat(int dir,const char *path,int flags,...) {
    int permissions=0;
    if(flags&O_CREAT){va_list ap;va_start(ap,flags);permissions=va_arg(ap,int);va_end(ap);}
    int r=openat(dir,path,flags,permissions);
    if(r<0){int e=errno;errno=0;return e;}
    last_fd=r;return r;
}
#define __PROSPERO__
#define fstatat target_fstatat
#define openat target_openat
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmacro-redefined"
#include "/private/tmp/steamcontroller-pr8-review/payload/ps5_at_compat.h"
#pragma clang diagnostic pop
#undef openat
#undef fstatat
#undef unlinkat
#undef renameat
#undef linkat
#undef mkdirat
static void put(int dir,const char *path) {
    int fd=openat(dir,path,O_WRONLY|O_CREAT|O_EXCL,0600);assert(fd>=0);
    assert(write(fd,"x",1)==1);assert(close(fd)==0);
}
int main(void) {
    char path[]="/private/tmp/pr8-at-real-XXXXXX";assert(mkdtemp(path));
    int dir=open(path,O_RDONLY|O_DIRECTORY);assert(dir>=0);
    put(dir,"target");put(dir,"replacement");
    int original=openat(dir,"target",O_RDONLY);assert(original>=0);
    assert(dup2(original,EMFILE)==EMFILE);
    struct rlimit old,limited;assert(getrlimit(RLIMIT_NOFILE,&old)==0);
    limited=old;limited.rlim_cur=32;assert(setrlimit(RLIMIT_NOFILE,&limited)==0);
    int fillers[64],count=0,fd;
    while((fd=open("/dev/null",O_RDONLY))>=0){assert(count<64);fillers[count++]=fd;}
    assert(errno==EMFILE);last_fd=-1;
    int alias=ps5_openat(dir,"target",O_RDONLY|O_NOFOLLOW);
    assert(setrlimit(RLIMIT_NOFILE,&old)==0);
    printf("real EMFILE collision: wrapper returned %d; pre-existing same-file fd=%d; newly acquired=%d\n",alias,EMFILE,last_fd);
    while(count)assert(close(fillers[--count])==0);
    assert(close(EMFILE)==0);assert(close(original)==0);
    replace_after_stat=1;last_fd=-1;errno=0;
    int result=ps5_openat(dir,"target",O_RDONLY|O_NOFOLLOW),error=errno;
    int leaked=last_fd>=0&&fcntl(last_fd,F_GETFD)>=0;
    printf("real atomic replacement: wrapper returned %d errno=%d; acquired fd=%d still open=%d\n",result,error,last_fd,leaked);
    if(last_fd>=0)assert(close(last_fd)==0);
    assert(unlinkat(dir,"target",0)==0);assert(close(dir)==0);assert(rmdir(path)==0);
    return alias==EMFILE&&result==-1&&leaked ? 1 : 0;
}
```


## Maintainer correction — PR8 / dev.7 preparation (2026-11-01)

The owner authorized fixing both P1 findings, retaining the contributor's useful changes, merging PR8 after successful validation and publishing dev.7 (a new version, preserving dev.6). The source changes are in the existing contributor branch ancestry; original author commits are retained.

The ambiguous inode/fd heuristic is replaced by a typed adapter for the same six SDK v0.43 filesystem syscalls. It captures the amd64 carry/error flag at the syscall instruction and translates failure to -1/errno. Successful descriptor values are never guessed, pre-inspected or discarded, so the existing caller owns cleanup and nofollow/exclusive-create validation. No generic syscall endpoint, privilege change, kernel/Shell patch, radio operation, native-controller mutation or storage migration is added. This is a reference to an established public ABI, not a firmware compatibility claim.

VERIFIED_HOST_TEST: real EMFILE/fd24 collision now returns -1/EMFILE and leaves the existing descriptor open; atomic replacement returns the valid new file descriptor; the real token creation path closes/removes a newly created empty file after injected fstat/EINTR and can retry. New tests cover all six syscall numbers/arguments, success values that equal errno numbers, missing/hostile files, exclusive creation and errors. A separate amd64 trap test executes the production register constraints and CF capture without a kernel call (also executed locally as x86_64 under Rosetta).

VERIFIED_PS5_BUILD / VERIFIED_CODE: checksum-pinned SDK v0.43 with Homebrew LLVM18.1.8 compiled/linked the complete ELF. llvm-objdump shows syscall immediately followed by the carry-based branch in each wrapper. Settings tile dependency check passes with no mandatory AppInstUtil dependency. This local modified-tree build intentionally embeds unknown/uncommitted source and is not a release artifact.

Local checks on the corrected working tree atop c3a7ddb: make -C tests clean test PASS (51 C targets; amd64 test prints explicit SKIP under native ARM, separately executed successfully as x86_64); make -C tests sanitize PASS; bash scripts/run_webui_browser.sh PASS220; git diff --check PASS. sdk/ps5_at tests do not establish real-console compatibility. The first SDK extraction lost executable bits because Python zip extraction was used; re-extraction with unzip corrected the environment and the build passed. An initial ELF dependency check could not find llvm-readelf-18; the explicit Homebrew llvm-readelf path passed. No earlier negative evidence removed.

The contributor reports Puck/system-menu input and WebUI on a single FW13.20 fat PS5 at c3a7ddb. Those are EXTERNAL_HARDWARE reports for the contributor build. The revised adapter, FW13.60, native DualSense coexistence, games, motion/haptics fidelity, console cleanup and the optional Settings tile still need physical testing. The previous review findings are resolved in source/host tests; exact-head remote CI and release receipts follow separately.
