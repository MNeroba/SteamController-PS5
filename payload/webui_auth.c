#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE
#endif

#include "webui_auth.h"
#include "private_file.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#define TOKEN_FILE_NAME "webui.token"

static int is_hex(unsigned char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}

static unsigned char lower_ascii(unsigned char c) {
    return c >= 'A' && c <= 'Z' ? (unsigned char)(c + ('a' - 'A')) : c;
}

static int read_exact(int fd, unsigned char *out, size_t length) {
    size_t done = 0;
    while (done < length) {
        ssize_t n = read(fd, out + done, length - done);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) { if (!n) errno=EIO; return -1; }
        done += (size_t)n;
    }
    return 0;
}

static int write_exact(int fd, const unsigned char *data, size_t length) {
    size_t done = 0;
    while (done < length) {
        ssize_t n = write(fd, data + done, length - done);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) { if (!n) errno=EIO; return -1; }
        done += (size_t)n;
    }
    return 0;
}

static void erase_secret(void *data, size_t length) {
    volatile unsigned char *p = data;
    while (length--) *p++ = 0;
}

static int secure_random_report(void *data, size_t length, storage_status_t *status) {
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return storage_failure(status,STORAGE_RANDOM_OPEN,errno);
    int result=read_exact(fd,data,length), error=errno;
    if (close(fd)!=0 && result==0)
        return storage_failure(status,STORAGE_RANDOM_CLOSE,errno);
    if (result!=0) return storage_failure(status,STORAGE_RANDOM_READ,error);
    return 0;
}
static int secure_random(void *data, size_t length) {
    return secure_random_report(data,length,NULL);
}

int webui_pairing_init(webui_pairing_t *p, uint64_t now_ms,
                       char code[WEBUI_PAIR_CODE_LENGTH + 1]) {
    uint32_t random;
    if (!p || !code) return -1;
    memset(p, 0, sizeof(*p));
    code[0] = 0;
    /* Rejection sampling avoids bias from the 32-bit to decimal conversion. */
    do {
        if (secure_random(&random, sizeof(random)) != 0) return -1;
    } while (random >= 4200000000u);
    if (pthread_mutex_init(&p->lock, NULL) != 0) {
        erase_secret(&random, sizeof(random)); return -1;
    }
    snprintf(p->code, sizeof(p->code), "%08u", (unsigned)(random % 100000000u));
    erase_secret(&random, sizeof(random));
    p->started_ms = now_ms;
    p->active = 1;
    memcpy(code, p->code, sizeof(p->code));
    return 0;
}

int webui_pairing_redeem(webui_pairing_t *p, const char *code, uint64_t now_ms) {
    if (!p || pthread_mutex_lock(&p->lock) != 0) return 0;
    int ok = 0;
    if (p->active && (now_ms < p->started_ms ||
                     now_ms - p->started_ms >= WEBUI_PAIR_TTL_MS)) {
        p->active = 0; erase_secret(p->code, sizeof(p->code));
    }
    if (p->active) {
        unsigned char difference = 0;
        ++p->attempts;
        if (code && strlen(code) == WEBUI_PAIR_CODE_LENGTH) {
            for (size_t i = 0; i < WEBUI_PAIR_CODE_LENGTH; ++i)
                difference |= (unsigned char)code[i] ^ (unsigned char)p->code[i];
            ok = difference == 0;
        }
        if (ok || p->attempts >= WEBUI_PAIR_ATTEMPTS) {
            p->active = 0; erase_secret(p->code, sizeof(p->code));
        }
    }
    pthread_mutex_unlock(&p->lock);
    return ok;
}

void webui_pairing_destroy(webui_pairing_t *p) {
    if (!p) return;
    erase_secret(p->code, sizeof(p->code));
    p->active = 0;
    pthread_mutex_destroy(&p->lock);
}

int webui_token_is_valid(const char *token) {
    size_t i;
    if (!token || strlen(token) != WEBUI_TOKEN_HEX_LENGTH) return 0;
    for (i = 0; i < WEBUI_TOKEN_HEX_LENGTH; i++)
        if (!is_hex((unsigned char)token[i])) return 0;
    return 1;
}

int webui_auth_matches(const char *authorization, const char *token) {
    static const char scheme[] = "Bearer ";
    unsigned char difference = 0;
    size_t i;
    if (!authorization || !webui_token_is_valid(token) ||
        strlen(authorization) != (sizeof(scheme) - 1 + WEBUI_TOKEN_HEX_LENGTH))
        return 0;
    for (i = 0; i < sizeof(scheme) - 1; i++)
        if (lower_ascii((unsigned char)authorization[i]) !=
            lower_ascii((unsigned char)scheme[i])) return 0;
    for (i = 0; i < WEBUI_TOKEN_HEX_LENGTH; i++)
        difference |= (unsigned char)authorization[sizeof(scheme) - 1 + i] ^
                      (unsigned char)token[i];
    return difference == 0;
}

int webui_auth_path_allows(const char *path, const char *authorization,
                           const char *token) {
    if (!path) return 0;
    if (strncmp(path, "/api/", 5) != 0) return 1;
    return webui_auth_matches(authorization, token);
}

static int read_token_file(int dir_fd, char token[WEBUI_TOKEN_HEX_LENGTH + 1],
                            storage_status_t *status) {
    unsigned char data[WEBUI_TOKEN_HEX_LENGTH+1]={0};
    struct stat st;
    storage_stage_t stage=STORAGE_OK; int error=0;
    int fd=openat(dir_fd,TOKEN_FILE_NAME,O_RDONLY|O_NOFOLLOW|O_NONBLOCK);
    if (fd<0) return storage_failure(status,STORAGE_TOKEN_OPEN,errno);
    if (fstat(fd,&st)!=0) { stage=STORAGE_TOKEN_STAT; error=errno; }
    else if (!S_ISREG(st.st_mode)) { stage=STORAGE_TOKEN_TYPE; error=EINVAL; }
    else if (st.st_uid!=geteuid()) { stage=STORAGE_TOKEN_OWNER; error=EACCES; }
    else if (st.st_mode & 077) { stage=STORAGE_TOKEN_MODE; error=EACCES; }
    else if (st.st_size!=WEBUI_TOKEN_HEX_LENGTH+1) { stage=STORAGE_TOKEN_SIZE; error=EINVAL; }
    else if (read_exact(fd,data,sizeof(data))!=0) { stage=STORAGE_TOKEN_READ; error=errno; }
    if (close(fd)!=0 && stage==STORAGE_OK) { stage=STORAGE_TOKEN_CLOSE; error=errno; }
    if (stage==STORAGE_OK) {
        if (data[WEBUI_TOKEN_HEX_LENGTH]!='\n') { stage=STORAGE_TOKEN_FORMAT; error=EINVAL; }
        data[WEBUI_TOKEN_HEX_LENGTH]=0;
        if (stage==STORAGE_OK && !webui_token_is_valid((const char *)data)) {
            stage=STORAGE_TOKEN_FORMAT; error=EINVAL;
        }
    }
    if (stage==STORAGE_OK) memcpy(token,data,sizeof(data));
    erase_secret(data,sizeof(data));
    return stage==STORAGE_OK ? 0 : storage_failure(status,stage,error);
}

static int create_token_file(int dir_fd,char token[WEBUI_TOKEN_HEX_LENGTH+1],
                              storage_status_t *status) {
    static const char hex[]="0123456789abcdef";
    unsigned char random[WEBUI_TOKEN_HEX_LENGTH/2]={0},text[WEBUI_TOKEN_HEX_LENGTH+1]={0};
    if (secure_random_report(random,sizeof(random),status)!=0) {
        erase_secret(random,sizeof(random)); return -1;
    }
    for (size_t i=0;i<sizeof(random);i++) {
        text[i*2]=(unsigned char)hex[random[i]>>4];
        text[i*2+1]=(unsigned char)hex[random[i]&15];
    }
    erase_secret(random,sizeof(random)); text[WEBUI_TOKEN_HEX_LENGTH]='\n';
    int fd=openat(dir_fd,TOKEN_FILE_NAME,O_WRONLY|O_CREAT|O_EXCL|O_NOFOLLOW,0600);
    if (fd<0) {
        int error=errno; erase_secret(text,sizeof(text));
        if (error==EEXIST) return read_token_file(dir_fd,token,status);
        return storage_failure(status,STORAGE_TOKEN_CREATE,error);
    }
    storage_stage_t stage=STORAGE_OK; int error=0; struct stat st;
    if (fstat(fd,&st)!=0) { stage=STORAGE_TOKEN_STAT; error=errno; }
    else if (!S_ISREG(st.st_mode) || st.st_uid!=geteuid()) { stage=STORAGE_TOKEN_OWNER; error=EACCES; }
    else if ((st.st_mode&07777)!=0600 && fchmod(fd,0600)!=0) {
        stage=STORAGE_TOKEN_CHMOD; error=errno;
    }
    else if (fstat(fd,&st)!=0) { stage=STORAGE_TOKEN_STAT; error=errno; }
    else if (!S_ISREG(st.st_mode) || st.st_uid!=geteuid() || (st.st_mode&07777)!=0600) {
        stage=STORAGE_TOKEN_MODE; error=EACCES;
    }
    else if (write_exact(fd,text,sizeof(text))!=0) { stage=STORAGE_TOKEN_WRITE; error=errno; }
    else if (fsync(fd)!=0) { stage=STORAGE_TOKEN_SYNC; error=errno; }
    if (close(fd)!=0 && stage==STORAGE_OK) { stage=STORAGE_TOKEN_CLOSE; error=errno; }
    if (stage!=STORAGE_OK) unlinkat(dir_fd,TOKEN_FILE_NAME,0);
    else { memcpy(token,text,WEBUI_TOKEN_HEX_LENGTH); token[WEBUI_TOKEN_HEX_LENGTH]=0; }
    erase_secret(text,sizeof(text));
    return stage==STORAGE_OK ? 1 : storage_failure(status,stage,error);
}

int webui_auth_load_or_create_report(const char *directory,
                                     char token[WEBUI_TOKEN_HEX_LENGTH+1],
                                     int *created,storage_status_t *status) {
    struct stat st;
    storage_status_t local; if (!status) status=&local;
    storage_status_clear(status); if (created) *created=0;
    if (token) erase_secret(token,WEBUI_TOKEN_HEX_LENGTH+1);
    if (!directory || !directory[0] || !token)
        return storage_failure(status,STORAGE_ARGUMENT,EINVAL);
    int dir_fd=private_directory_open_report(directory,status);
    if (dir_fd<0) return -1;
    int result=read_token_file(dir_fd,token,status);
    if (result!=0) {
        if (fstatat(dir_fd,TOKEN_FILE_NAME,&st,AT_SYMLINK_NOFOLLOW)==0) {
            /* Preserve the original security/format/read failure. Never replace
             * an existing invalid, foreign or inaccessible credential. */
            result=-1;
        } else if (errno!=ENOENT) result=storage_failure(status,STORAGE_TOKEN_INSPECT,errno);
        else result=create_token_file(dir_fd,token,status);
    }
    int close_error=close(dir_fd),error=errno;
    if (result<0) {
        erase_secret(token,WEBUI_TOKEN_HEX_LENGTH+1);
        return storage_failure(status,status->stage,status->error);
    }
    if (close_error!=0) {
        erase_secret(token,WEBUI_TOKEN_HEX_LENGTH+1);
        return storage_failure(status,STORAGE_DIRECTORY_CLOSE,error);
    }
    if (created && result>0) *created=1;
    storage_status_clear(status); return 0;
}
int webui_auth_load_or_create(const char *directory,
                              char token[WEBUI_TOKEN_HEX_LENGTH+1],int *created) {
    return webui_auth_load_or_create_report(directory,token,created,NULL);
}
