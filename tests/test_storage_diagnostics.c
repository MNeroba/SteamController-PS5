#define _POSIX_C_SOURCE 200809L
#define _DARWIN_C_SOURCE
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
static int chmod_calls, fault, stat_calls, checks, random_opens;
enum { NONE, SYNC_FAIL, WRITE_FAIL, RANDOM_OPEN_FAIL, READ_FAIL, FILE_CLOSE_FAIL,
       DIR_CLOSE_FAIL, STAT_FAIL, FOREIGN_DIR, FOREIGN_FILE, VOLATILE_RANDOM_FAIL };
static int no_chmod(int fd, mode_t mode) {
    (void)fd; (void)mode; ++chmod_calls; errno=EOPNOTSUPP; return -1;
}
static int test_open(const char *path,int flags,...) {
    mode_t mode=0;
    if (flags&O_CREAT) { va_list ap;va_start(ap,flags);mode=(mode_t)va_arg(ap,int);va_end(ap); }
    if (fault==VOLATILE_RANDOM_FAIL && !strcmp(path,"/dev/urandom") && random_opens++) {errno=ENXIO;return -1;}
    if (fault==RANDOM_OPEN_FAIL && !strcmp(path,"/dev/urandom")) { errno=ENXIO;return -1; }
    return open(path,flags,mode);
}
static int test_fstat(int fd,struct stat *st) {
    ++stat_calls;
    if (fault==STAT_FAIL) { errno=EIO;return -1; }
    int r=fstat(fd,st);
    if (!r && ((fault==FOREIGN_DIR && S_ISDIR(st->st_mode)) ||
               (fault==FOREIGN_FILE && S_ISREG(st->st_mode)))) st->st_uid=geteuid()+1;
    return r;
}
static ssize_t test_read(int fd,void *data,size_t n) {
    if (fault==READ_FAIL) { errno=EIO;return -1; }
    return read(fd,data,n);
}
static ssize_t test_write(int fd,const void *data,size_t n) {
    if (fault==WRITE_FAIL || fault==VOLATILE_RANDOM_FAIL) { errno=ENOSPC;return -1; }
    return write(fd,data,n);
}
static int test_sync(int fd) {
    if (fault==SYNC_FAIL) { errno=EIO;return -1; }
    return fsync(fd);
}
static int test_close(int fd) {
    struct stat st;int fail=fstat(fd,&st)==0 &&
        ((fault==FILE_CLOSE_FAIL && S_ISREG(st.st_mode)) || (fault==DIR_CLOSE_FAIL && S_ISDIR(st.st_mode)));
    int r=close(fd);if(fail){errno=EIO;return -1;}return r;
}
#define fchmod no_chmod
#define open test_open
#define fstat test_fstat
#define read test_read
#define write test_write
#define fsync test_sync
#define close test_close
#include "../payload/webui_auth.c"
#undef fchmod
#undef open
#undef fstat
#undef read
#undef write
#undef fsync
#undef close
#define CHECK(c) do { ++checks; assert(c); } while(0)
static char directory[]="/tmp/sc-private-diag-XXXXXX", path[256];
static char out[WEBUI_TOKEN_HEX_LENGTH+1];
static storage_status_t status;
static void rejected(int injected,storage_stage_t stage,int error) {
    fault=injected;memset(out,'x',sizeof(out));int created=123;
    CHECK(webui_auth_load_or_create_report(directory,out,&created,&status)==-1);
    CHECK(status.stage==stage); CHECK(status.error==error);CHECK(errno==error);CHECK(created==0);
    for(size_t i=0;i<sizeof(out);i++)CHECK(out[i]==0);
    fault=NONE;
}
int main(void) {
    CHECK(mkdtemp(directory));snprintf(path,sizeof(path),"%s/webui.token",directory);
    int fd=private_directory_open(directory);CHECK(fd>=0);CHECK(chmod_calls==0);CHECK(close(fd)==0);
    int created=0;CHECK(webui_auth_load_or_create_report(directory,out,&created,&status)==0);
    CHECK(created==1 && status.stage==STORAGE_OK && status.error==0);CHECK(chmod_calls==0);
    char saved[sizeof(out)];memcpy(saved,out,sizeof(out));CHECK(webui_token_is_valid(out));
    CHECK(webui_auth_load_or_create_report(directory,out,&created,&status)==0);
    CHECK(created==0 && !strcmp(saved,out));
    CHECK(chmod(path,0644)==0);rejected(NONE,STORAGE_TOKEN_MODE,EACCES);CHECK(chmod(path,0600)==0);
    rejected(FOREIGN_FILE,STORAGE_TOKEN_OWNER,EACCES);
    rejected(FOREIGN_DIR,STORAGE_DIRECTORY_OWNER,EACCES);
    rejected(STAT_FAIL,STORAGE_DIRECTORY_STAT,EIO);
    rejected(READ_FAIL,STORAGE_TOKEN_READ,EIO);
    rejected(FILE_CLOSE_FAIL,STORAGE_TOKEN_CLOSE,EIO);
    rejected(DIR_CLOSE_FAIL,STORAGE_DIRECTORY_CLOSE,EIO);
    CHECK(chmod(directory,0755)==0);rejected(NONE,STORAGE_DIRECTORY_CHMOD,EOPNOTSUPP);CHECK(chmod(directory,0700)==0);
    CHECK(unlink(path)==0);
    for(int mode=SYNC_FAIL;mode<=RANDOM_OPEN_FAIL;mode++) {
        rejected(mode,mode==SYNC_FAIL?STORAGE_TOKEN_SYNC:mode==WRITE_FAIL?STORAGE_TOKEN_WRITE:STORAGE_RANDOM_OPEN,
                 mode==WRITE_FAIL?ENOSPC:mode==RANDOM_OPEN_FAIL?ENXIO:EIO);
        CHECK(access(path,F_OK)==-1 && errno==ENOENT);
    }
    rejected(READ_FAIL,STORAGE_RANDOM_READ,EIO);
    rejected(FILE_CLOSE_FAIL,STORAGE_TOKEN_CLOSE,EIO);CHECK(access(path,F_OK)==-1);
    mode_t old=umask(0777);rejected(NONE,STORAGE_TOKEN_CHMOD,EOPNOTSUPP);umask(old);
    CHECK(access(path,F_OK)==-1);CHECK(symlink("missing-target",path)==0);
    CHECK(webui_auth_load_or_create_report(directory,out,&created,&status)==-1);
    CHECK(status.stage==STORAGE_TOKEN_OPEN);CHECK(unlink(path)==0);
    CHECK(mkfifo(path,0600)==0);rejected(NONE,STORAGE_TOKEN_TYPE,EINVAL);CHECK(unlink(path)==0);
    fd=open(path,O_CREAT|O_WRONLY,0600);CHECK(fd>=0);CHECK(write(fd,"bad\n",4)==4);CHECK(close(fd)==0);
    rejected(NONE,STORAGE_TOKEN_SIZE,EINVAL);CHECK(unlink(path)==0);
    fd=open(path,O_CREAT|O_WRONLY,0600);CHECK(fd>=0);char bad[65];memset(bad,'z',64);bad[64]='\n';
    CHECK(write(fd,bad,sizeof(bad))==(ssize_t)sizeof(bad));CHECK(close(fd)==0);
    rejected(NONE,STORAGE_TOKEN_FORMAT,EINVAL);CHECK(unlink(path)==0);CHECK(rmdir(directory)==0);
    /* Operational write failure may use a fresh RAM token; existing corrupt,
     * foreign or insecure credentials must still fail closed. */
    CHECK(mkdir(directory,0700)==0);
    fault=WRITE_FAIL;
    CHECK(webui_auth_begin(directory,out,&status)==WEBUI_AUTH_SESSION);
    CHECK(webui_token_is_valid(out));CHECK(status.stage==STORAGE_TOKEN_WRITE && status.error==ENOSPC);
    CHECK(access(path,F_OK)==-1);char session_token[65];memcpy(session_token,out,sizeof(out));
    CHECK(webui_auth_begin(directory,out,&status)==WEBUI_AUTH_SESSION);
    CHECK(strcmp(session_token,out)!=0);
    fault=VOLATILE_RANDOM_FAIL;random_opens=0;
    CHECK(webui_auth_begin(directory,out,&status)==WEBUI_AUTH_FAILED);
    CHECK(status.stage==STORAGE_RANDOM_OPEN && status.error==ENXIO);
    for(size_t i=0;i<sizeof(out);i++)CHECK(out[i]==0);
    fault=RANDOM_OPEN_FAIL;memset(out,'x',sizeof(out));
    CHECK(webui_auth_begin(directory,out,&status)==WEBUI_AUTH_FAILED);
    CHECK(status.stage==STORAGE_RANDOM_OPEN);
    for(size_t i=0;i<sizeof(out);i++)CHECK(out[i]==0);
    fault=NONE;
    CHECK(webui_auth_begin(directory,out,&status)==WEBUI_AUTH_PERSISTENT);
    CHECK(webui_token_is_valid(out));
    CHECK(chmod(path,0644)==0);
    CHECK(webui_auth_begin(directory,out,&status)==WEBUI_AUTH_FAILED);
    CHECK(status.stage==STORAGE_TOKEN_MODE);CHECK(out[0]==0);CHECK(chmod(path,0600)==0);
    fault=FOREIGN_FILE;CHECK(webui_auth_begin(directory,out,&status)==WEBUI_AUTH_FAILED);
    CHECK(status.stage==STORAGE_TOKEN_OWNER);fault=NONE;
    CHECK(unlink(path)==0);CHECK(symlink("missing",path)==0);
    CHECK(webui_auth_begin(directory,out,&status)==WEBUI_AUTH_FAILED);
    CHECK(unlink(path)==0);CHECK(rmdir(directory)==0);
    CHECK(!strcmp(storage_stage_name((storage_stage_t)-1),"unknown"));
    printf("storage diagnostics: %d checks passed\n",checks);return 0;
}
