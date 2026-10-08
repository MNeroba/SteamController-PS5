#ifndef WEBUI_HTTP_H
#define WEBUI_HTTP_H

#include <stddef.h>

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

/* Returns 1 when a complete valid header block is available, 0 when more
 * bytes are needed, and -1 for malformed or unsupported input. */
int webui_http_parse_headers(const char *data, size_t length,
                            webui_http_request_t *request);

#endif
