#include "webui_http.h"

#include <errno.h>
#include <stdint.h>
#include <string.h>

static size_t find_header_end(const char *data, size_t length) {
    size_t i;
    for (i = 0; i + 3 < length; i++) {
        if (data[i] == '\r' && data[i + 1] == '\n' &&
            data[i + 2] == '\r' && data[i + 3] == '\n')
            return i + 4;
    }
    return 0;
}

static int ascii_equal_nocase(const char *a, const char *b, size_t length) {
    size_t i;
    for (i = 0; i < length; i++) {
        unsigned char ac = (unsigned char)a[i];
        unsigned char bc = (unsigned char)b[i];
        if (ac >= 'A' && ac <= 'Z') ac = (unsigned char)(ac + ('a' - 'A'));
        if (bc >= 'A' && bc <= 'Z') bc = (unsigned char)(bc + ('a' - 'A'));
        if (ac != bc) return 0;
    }
    return 1;
}

static int is_token_char(unsigned char c) {
    if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') ||
        (c >= 'a' && c <= 'z')) return 1;
    switch (c) {
        case '!': case '#': case '$': case '%': case '&': case '\'':
        case '*': case '+': case '-': case '.': case '^': case '_':
        case '`': case '|': case '~': return 1;
        default: return 0;
    }
}

static int parse_request_line(const char *line, size_t length,
                              webui_http_request_t *request) {
    size_t method_end = 0, target_start, target_end, version_start, i;
    while (method_end < length && line[method_end] != ' ') method_end++;
    if (method_end == 0 || method_end >= length ||
        method_end >= sizeof(request->method)) return -1;
    for (i = 0; i < method_end; i++) {
        unsigned char c = (unsigned char)line[i];
        if (c < 'A' || c > 'Z') return -1;
    }
    if (!((method_end == 3 && memcmp(line, "GET", 3) == 0) ||
          (method_end == 4 && memcmp(line, "POST", 4) == 0))) return -1;

    target_start = method_end + 1;
    while (target_start < length && line[target_start] == ' ') target_start++;
    target_end = target_start;
    while (target_end < length && line[target_end] != ' ') target_end++;
    if (target_end == target_start || target_end - target_start >= sizeof(request->target) ||
        line[target_start] != '/') return -1;
    for (i = target_start; i < target_end; i++) {
        unsigned char c = (unsigned char)line[i];
        if (c <= 0x20 || c == 0x7f || c == '#') return -1;
    }
    if (target_end >= length) return -1;
    version_start = target_end;
    while (version_start < length && line[version_start] == ' ') version_start++;
    if (version_start + 8 != length ||
        (memcmp(line + version_start, "HTTP/1.1", 8) != 0 &&
         memcmp(line + version_start, "HTTP/1.0", 8) != 0)) return -1;

    memcpy(request->method, line, method_end);
    request->method[method_end] = '\0';
    memcpy(request->target, line + target_start, target_end - target_start);
    request->target[target_end - target_start] = '\0';
    return 0;
}

static int parse_content_length(const char *value, size_t length, size_t *out) {
    size_t result = 0, i;
    if (length == 0) return -1;
    for (i = 0; i < length; i++) {
        unsigned char c = (unsigned char)value[i];
        size_t digit;
        if (c < '0' || c > '9') return -1;
        digit = (size_t)(c - '0');
        if (result > (SIZE_MAX - digit) / 10) return -1;
        result = result * 10 + digit;
        if (result > WEBUI_HTTP_BODY_MAX) return -1;
    }
    *out = result;
    return 0;
}

int webui_http_parse_headers(const char *data, size_t length,
                            webui_http_request_t *request) {
    size_t header_end, line_start, line_end, colon, value_start, value_end, i;
    if (!data || !request) return -1;
    memset(request, 0, sizeof(*request));
    header_end = find_header_end(data, length);
    if (header_end == 0) return length >= WEBUI_HTTP_HEADER_MAX ? -1 : 0;
    if (header_end > WEBUI_HTTP_HEADER_MAX) return -1;
    for (i = 0; i < header_end; i++) if (data[i] == '\0') return -1;

    line_end = 0;
    while (line_end + 1 < header_end &&
           !(data[line_end] == '\r' && data[line_end + 1] == '\n')) line_end++;
    if (line_end + 1 >= header_end ||
        parse_request_line(data, line_end, request) != 0) return -1;

    line_start = line_end + 2;
    while (line_start + 2 < header_end) {
        line_end = line_start;
        while (line_end + 1 < header_end &&
               !(data[line_end] == '\r' && data[line_end + 1] == '\n')) line_end++;
        if (line_end + 1 >= header_end) return -1;
        if (line_end == line_start) break;
        if (data[line_start] == ' ' || data[line_start] == '\t') return -1;

        colon = line_start;
        while (colon < line_end && data[colon] != ':') {
            if (!is_token_char((unsigned char)data[colon])) return -1;
            colon++;
        }
        if (colon == line_start || colon == line_end) return -1;
        value_start = colon + 1;
        while (value_start < line_end &&
               (data[value_start] == ' ' || data[value_start] == '\t')) value_start++;
        value_end = line_end;
        while (value_end > value_start &&
               (data[value_end - 1] == ' ' || data[value_end - 1] == '\t')) value_end--;
        for (i = value_start; i < value_end; i++) {
            unsigned char c = (unsigned char)data[i];
            if ((c < 0x20 && c != '\t') || c == 0x7f) return -1;
        }

        if (colon - line_start == 14 &&
            ascii_equal_nocase(data + line_start, "content-length", 14)) {
            if (request->has_content_length ||
                parse_content_length(data + value_start, value_end - value_start,
                                     &request->content_length) != 0) return -1;
            request->has_content_length = 1;
        } else if (colon - line_start == 13 &&
                   ascii_equal_nocase(data + line_start, "authorization", 13)) {
            size_t n = value_end - value_start;
            if (request->has_authorization ||
                n >= sizeof(request->authorization)) return -1;
            memcpy(request->authorization, data + value_start, n);
            request->authorization[n] = '\0';
            request->has_authorization = 1;
        } else if (colon - line_start == 17 &&
                   ascii_equal_nocase(data + line_start, "transfer-encoding", 17)) {
            return -1;
        }
        line_start = line_end + 2;
    }

    if (!request->has_content_length) request->content_length = 0;
    if (strcmp(request->method, "GET") == 0 && request->content_length != 0) return -1;
    request->header_length = header_end;
    return 1;
}

static int remaining_timeout_ms(webui_http_clock_fn clock, void *context,
                                uint64_t request_started_ms, unsigned timeout_ms,
                                unsigned *remaining_ms) {
    uint64_t now;
    if (!clock || clock(context, &now) != 0 || now < request_started_ms)
        return WEBUI_HTTP_READ_IO_ERROR;
    if (now - request_started_ms >= timeout_ms) return WEBUI_HTTP_READ_TIMEOUT;
    *remaining_ms = timeout_ms - (unsigned)(now - request_started_ms);
    return WEBUI_HTTP_READ_OK;
}

int webui_http_read_request(void *context, webui_http_recv_fn receive,
                            webui_http_clock_fn clock, unsigned timeout_ms,
                            char *buffer, size_t capacity,
                            webui_http_request_t *request, char **body) {
    size_t got = 0, total;
    uint64_t request_started_ms;
    int parsed = 0, result;

    if (!receive || !clock || !buffer || capacity < 2 || !request || !body ||
        timeout_ms == 0 || clock(context, &request_started_ms) != 0)
        return WEBUI_HTTP_READ_IO_ERROR;

    while (!parsed) {
        ssize_t n;
        unsigned remaining;
        if (got >= capacity - 1) return WEBUI_HTTP_READ_TOO_LARGE;
        result = remaining_timeout_ms(clock, context, request_started_ms,
                                      timeout_ms, &remaining);
        if (result != WEBUI_HTTP_READ_OK) return result;
        n = receive(context, buffer + got, capacity - 1 - got, remaining);
        if (n < 0) {
            if (errno == EINTR) continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return WEBUI_HTTP_READ_TIMEOUT;
            return WEBUI_HTTP_READ_IO_ERROR;
        }
        if (n == 0) return WEBUI_HTTP_READ_IO_ERROR;
        if ((size_t)n > capacity - 1 - got) return WEBUI_HTTP_READ_IO_ERROR;
        result = remaining_timeout_ms(clock, context, request_started_ms,
                                      timeout_ms, &remaining);
        if (result != WEBUI_HTTP_READ_OK) return result;
        got += (size_t)n;
        buffer[got] = '\0';
        parsed = webui_http_parse_headers(buffer, got, request);
        if (parsed < 0) return WEBUI_HTTP_READ_MALFORMED;
    }

    if (request->header_length > capacity - 1 ||
        request->content_length > capacity - 1 - request->header_length)
        return WEBUI_HTTP_READ_TOO_LARGE;
    total = request->header_length + request->content_length;
    if (got > total) return WEBUI_HTTP_READ_MALFORMED;
    while (got < total) {
        ssize_t n;
        unsigned remaining;
        result = remaining_timeout_ms(clock, context, request_started_ms,
                                      timeout_ms, &remaining);
        if (result != WEBUI_HTTP_READ_OK) return result;
        n = receive(context, buffer + got, total - got, remaining);
        if (n < 0) {
            if (errno == EINTR) continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return WEBUI_HTTP_READ_TIMEOUT;
            return WEBUI_HTTP_READ_IO_ERROR;
        }
        if (n == 0) return WEBUI_HTTP_READ_IO_ERROR;
        if ((size_t)n > total - got) return WEBUI_HTTP_READ_IO_ERROR;
        result = remaining_timeout_ms(clock, context, request_started_ms,
                                      timeout_ms, &remaining);
        if (result != WEBUI_HTTP_READ_OK) return result;
        got += (size_t)n;
        buffer[got] = '\0';
    }

    if (memchr(buffer + request->header_length, '\0', request->content_length))
        return WEBUI_HTTP_READ_MALFORMED;
    *body = buffer + request->header_length;
    (*body)[request->content_length] = '\0';
    return WEBUI_HTTP_READ_OK;
}
