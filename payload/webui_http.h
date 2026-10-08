#ifndef WEBUI_HTTP_H
#define WEBUI_HTTP_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#define WEBUI_HTTP_METHOD_MAX 8
#define WEBUI_HTTP_TARGET_MAX 256
#define WEBUI_HTTP_AUTH_MAX 96
#define WEBUI_HTTP_HEADER_MAX 4096
#define WEBUI_HTTP_BODY_MAX (16 * 1024)

typedef struct {
    char method[WEBUI_HTTP_METHOD_MAX];
    char target[WEBUI_HTTP_TARGET_MAX];
    char authorization[WEBUI_HTTP_AUTH_MAX];
    size_t header_length;
    size_t content_length;
    int has_authorization;
    int has_content_length;
} webui_http_request_t;

typedef int (*webui_http_clock_fn)(void *context, uint64_t *now_ms);
typedef ssize_t (*webui_http_recv_fn)(void *context, char *buffer,
                                      size_t capacity, unsigned wait_ms);
typedef ssize_t (*webui_http_send_fn)(void *context, const char *buffer,
                                      size_t length, unsigned wait_ms);

typedef struct {
    const char *data;
    size_t length;
} webui_http_chunk_t;

enum {
    WEBUI_HTTP_READ_OK = 0,
    WEBUI_HTTP_READ_MALFORMED = -1,
    WEBUI_HTTP_READ_TOO_LARGE = -2,
    WEBUI_HTTP_READ_TIMEOUT = -3,
    WEBUI_HTTP_READ_IO_ERROR = -4,
    WEBUI_HTTP_WRITE_OK = 0,
    WEBUI_HTTP_WRITE_TIMEOUT = -5,
    WEBUI_HTTP_WRITE_IO_ERROR = -6
};

/* Returns 1 when a complete valid header block is available, 0 when more
 * bytes are needed, and -1 for malformed or unsupported input. */
int webui_http_parse_headers(const char *data, size_t length,
                            webui_http_request_t *request);

/* Read and parse one bounded request. timeout_ms is an absolute deadline from
 * the initial monotonic-clock sample, not an inactivity timeout. The clock
 * callback must report monotonic milliseconds; receive must honor wait_ms.
 * Callbacks let host tests exercise production framing without the PS5 socket
 * ABI. */
int webui_http_read_request(void *context, webui_http_recv_fn receive,
                            webui_http_clock_fn clock, unsigned timeout_ms,
                            char *buffer, size_t capacity,
                            webui_http_request_t *request, char **body);

/* Write all response chunks within one absolute monotonic deadline. */
int webui_http_write_chunks(void *context, webui_http_send_fn send,
                            webui_http_clock_fn clock, unsigned timeout_ms,
                            const webui_http_chunk_t *chunks, size_t chunk_count);

#endif
