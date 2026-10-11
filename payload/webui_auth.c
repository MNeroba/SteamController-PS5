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
        if (n <= 0) return -1;
        done += (size_t)n;
    }
    return 0;
}

static int write_exact(int fd, const unsigned char *data, size_t length) {
    size_t done = 0;
    while (done < length) {
        ssize_t n = write(fd, data + done, length - done);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) return -1;
        done += (size_t)n;
    }
    return 0;
}

static void erase_secret(void *data, size_t length) {
    volatile unsigned char *p = data;
    while (length--) *p++ = 0;
}

static int secure_random(void *data, size_t length) {
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return -1;
    int result = read_exact(fd, data, length);
    if (close(fd) != 0) result = -1;
    return result;
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

static int read_token_file(int dir_fd, char token[WEBUI_TOKEN_HEX_LENGTH + 1]) {
    unsigned char file_data[WEBUI_TOKEN_HEX_LENGTH + 2];
    struct stat st;
    size_t i;
    int fd = openat(dir_fd, TOKEN_FILE_NAME, O_RDONLY | O_NOFOLLOW | O_NONBLOCK);
    if (fd < 0) return -1;
    if (fstat(fd, &st) != 0 || !S_ISREG(st.st_mode) || st.st_uid != geteuid() ||
        (st.st_mode & 077) != 0 || st.st_size != WEBUI_TOKEN_HEX_LENGTH + 1 ||
        read_exact(fd, file_data, WEBUI_TOKEN_HEX_LENGTH + 1) != 0) {
        close(fd);
        return -1;
    }
    close(fd);
    if (file_data[WEBUI_TOKEN_HEX_LENGTH] != '\n') return -1;
    file_data[WEBUI_TOKEN_HEX_LENGTH] = '\0';
    if (!webui_token_is_valid((const char *)file_data)) return -1;
    for (i = 0; i < WEBUI_TOKEN_HEX_LENGTH; i++) token[i] = (char)file_data[i];
    token[WEBUI_TOKEN_HEX_LENGTH] = '\0';
    return 0;
}

static int create_token_file(int dir_fd, char token[WEBUI_TOKEN_HEX_LENGTH + 1]) {
    static const char hex[] = "0123456789abcdef";
    unsigned char random[WEBUI_TOKEN_HEX_LENGTH / 2];
    unsigned char text[WEBUI_TOKEN_HEX_LENGTH + 1];
    size_t i;
    int token_fd;
    if (secure_random(random, sizeof(random)) != 0) return -1;
    for (i = 0; i < sizeof(random); i++) {
        text[i * 2] = (unsigned char)hex[random[i] >> 4];
        text[i * 2 + 1] = (unsigned char)hex[random[i] & 15];
    }
    text[WEBUI_TOKEN_HEX_LENGTH] = '\n';
    token_fd = openat(dir_fd, TOKEN_FILE_NAME,
                      O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW, 0600);
    if (token_fd < 0) {
        if (errno == EEXIST) return read_token_file(dir_fd, token);
        return -1;
    }
    if (fchmod(token_fd, 0600) != 0 || write_exact(token_fd, text, sizeof(text)) != 0 ||
        fsync(token_fd) != 0) {
        close(token_fd);
        unlinkat(dir_fd, TOKEN_FILE_NAME, 0);
        return -1;
    }
    if (close(token_fd) != 0) {
        unlinkat(dir_fd, TOKEN_FILE_NAME, 0);
        return -1;
    }
    for (i = 0; i < WEBUI_TOKEN_HEX_LENGTH; i++) token[i] = (char)text[i];
    token[WEBUI_TOKEN_HEX_LENGTH] = '\0';
    memset(random, 0, sizeof(random));
    memset(text, 0, sizeof(text));
    return 1;
}

int webui_auth_load_or_create(const char *directory,
                              char token[WEBUI_TOKEN_HEX_LENGTH + 1],
                              int *created) {
    struct stat st;
    int dir_fd, result;
    if (created) *created = 0;
    if (!directory || !directory[0] || !token) return -1;
    dir_fd = private_directory_open(directory);
    if (dir_fd < 0) return -1;
    if (read_token_file(dir_fd, token) == 0) {
        close(dir_fd);
        return 0;
    }
    if (fstatat(dir_fd, TOKEN_FILE_NAME, &st, AT_SYMLINK_NOFOLLOW) == 0 ||
        errno != ENOENT) {
        close(dir_fd);
        return -1;
    }
    result = create_token_file(dir_fd, token);
    close(dir_fd);
    if (result < 0) return -1;
    if (created && result > 0) *created = 1;
    return 0;
}
