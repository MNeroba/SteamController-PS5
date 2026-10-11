#define _POSIX_C_SOURCE 200809L
#define _DARWIN_C_SOURCE

#include "webui_auth.h"
#include "webui_http.h"

#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include "webui_html.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int failures;
static int checks;

#define CHECK(cond) do { \
    checks++; \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        failures++; \
    } \
} while (0)

static const char token[] = "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";

static int parse(const char *wire, webui_http_request_t *request) {
    return webui_http_parse_headers(wire, strlen(wire), request);
}

typedef struct {
    const char *wire;
    size_t length;
    size_t offset;
    uint64_t now_ms;
    unsigned max_wait_ms;
    unsigned first_wait_ms;
    unsigned last_wait_ms;
} slow_request_t;

static int slow_request_clock(void *context, uint64_t *now_ms) {
    slow_request_t *source = context;
    *now_ms = source->now_ms;
    return 0;
}

static ssize_t slow_request_receive(void *context, char *buffer,
                                   size_t capacity, unsigned wait_ms) {
    slow_request_t *source = context;
    if (wait_ms > source->max_wait_ms) source->max_wait_ms = wait_ms;
    if (!source->first_wait_ms) source->first_wait_ms = wait_ms;
    source->last_wait_ms = wait_ms;
    if (!capacity || source->offset >= source->length) return 0;
    buffer[0] = source->wire[source->offset++];
    source->now_ms += 100;
    return 1;
}

static ssize_t delayed_complete_request_receive(void *context, char *buffer,
                                                size_t capacity, unsigned wait_ms) {
    slow_request_t *source = context;
    size_t remaining = source->length - source->offset;
    (void)wait_ms;
    if (remaining > capacity) return -1;
    memcpy(buffer, source->wire + source->offset, remaining);
    source->offset += remaining;
    source->now_ms += 300;
    return (ssize_t)remaining;
}

static ssize_t slow_request_body_receive(void *context, char *buffer,
                                         size_t capacity, unsigned wait_ms) {
    slow_request_t *source = context;
    const char *header_end = strstr(source->wire, "\r\n\r\n");
    size_t header_length = (size_t)(header_end - source->wire) + 4;
    size_t remaining = source->length - source->offset;
    size_t count = source->offset < header_length ? header_length - source->offset : 1;
    (void)wait_ms;
    if (count > remaining) count = remaining;
    if (count > capacity) return -1;
    memcpy(buffer, source->wire + source->offset, count);
    source->offset += count;
    source->now_ms += count == 1 ? 100 : 10;
    return (ssize_t)count;
}

typedef struct {
    uint64_t now_ms;
    size_t sent;
    unsigned max_wait_ms;
} slow_response_t;

typedef struct {
    uint64_t now_ms;
    char data[64];
    size_t length;
} response_sink_t;

static int slow_response_clock(void *context, uint64_t *now_ms) {
    slow_response_t *response = context;
    *now_ms = response->now_ms;
    return 0;
}

static ssize_t slow_response_send(void *context, const char *buffer,
                                  size_t length, unsigned wait_ms) {
    slow_response_t *response = context;
    (void)buffer;
    if (wait_ms > response->max_wait_ms) response->max_wait_ms = wait_ms;
    if (!length) return 0;
    response->sent++;
    response->now_ms += 100;
    return 1;
}

static void test_response_uses_one_absolute_deadline(void) {
    static const char header[] = "HTTP/1.1 200 OK\r\n";
    static const char body[] = "0123456789abcdef";
    const webui_http_chunk_t chunks[] = {
        {header, sizeof(header) - 1},
        {body, sizeof(body) - 1}
    };
    slow_response_t response = {0, 0, 0};
    size_t total = (sizeof(header) - 1) + (sizeof(body) - 1);
    int result = webui_http_write_chunks(&response, slow_response_send,
                                         slow_response_clock, 250,
                                         chunks, sizeof(chunks) / sizeof(chunks[0]));
    CHECK(result == WEBUI_HTTP_WRITE_TIMEOUT);
    CHECK(response.sent < total);
    CHECK(response.now_ms >= 250);
    CHECK(response.max_wait_ms <= 250);
}

static int response_sink_clock(void *context, uint64_t *now_ms) {
    response_sink_t *sink = context;
    *now_ms = sink->now_ms;
    return 0;
}

static ssize_t response_sink_send(void *context, const char *buffer,
                                  size_t length, unsigned wait_ms) {
    response_sink_t *sink = context;
    size_t count = length < 3 ? length : 3;
    (void)wait_ms;
    if (count > sizeof(sink->data) - sink->length) return -1;
    memcpy(sink->data + sink->length, buffer, count);
    sink->length += count;
    sink->now_ms += 5;
    return (ssize_t)count;
}

static void test_response_writes_all_chunks(void) {
    static const char header[] = "H: 1\r\n\r\n";
    static const char body[] = "body";
    const webui_http_chunk_t chunks[] = {
        {header, sizeof(header) - 1}, {body, sizeof(body) - 1}
    };
    const char expected[] = "H: 1\r\n\r\nbody";
    response_sink_t sink = {0, {0}, 0};
    int result = webui_http_write_chunks(&sink, response_sink_send,
                                         response_sink_clock, 250,
                                         chunks, sizeof(chunks) / sizeof(chunks[0]));
    CHECK(result == WEBUI_HTTP_WRITE_OK);
    CHECK(sink.length == sizeof(expected) - 1);
    CHECK(memcmp(sink.data, expected, sizeof(expected) - 1) == 0);
}

typedef struct { uint64_t now; size_t sent; unsigned calls; int mode; char *data; size_t capacity; } delivery_t;
static int delivery_clock(void *ctx, uint64_t *now) { *now = ((delivery_t *)ctx)->now; return 0; }
static ssize_t delivery_send(void *ctx, const char *data, size_t length, unsigned wait) {
    delivery_t *s = ctx; s->calls++; s->now += 1;
    if (s->mode == 2 && s->sent > 1024) { errno = EPIPE; return -1; }
    if (s->mode == 3) { s->now += wait; errno = EAGAIN; return -1; }
    if (s->mode == 4) return 0;
    if (s->calls % 5 == 1) { errno = EINTR; return -1; }
    if (s->calls % 5 == 2) { errno = EAGAIN; return -1; }
    size_t count = length > 997 ? 997 : length;
    if (count > s->capacity - s->sent) return -1;
    memcpy(s->data + s->sent, data, count); s->sent += count;
    return (ssize_t)count;
}
static void test_embedded_delivery(void) {
    char header[160];
    int bytes = snprintf(header, sizeof(header), "HTTP/1.1 200 OK\r\nContent-Length: %u\r\nConnection: close\r\n\r\n", webui_html_len);
    webui_http_chunk_t chunks[] = {{header, (size_t)bytes}, {(const char *)webui_html, webui_html_len}};
    size_t total = (size_t)bytes + webui_html_len;
    char *received = malloc(total); CHECK(received != NULL); if (!received) return;
    delivery_t s = {0, 0, 0, 1, received, total};
    CHECK(webui_http_write_chunks(&s, delivery_send, delivery_clock, 3000, chunks, 2) == WEBUI_HTTP_WRITE_OK);
    CHECK(s.sent == total);
    CHECK(memcmp(received, header, (size_t)bytes) == 0);
    CHECK(memcmp(received + bytes, webui_html, webui_html_len) == 0);
    for (int mode = 2; mode <= 4; mode++) {
        s = (delivery_t){0, 0, 0, mode, received, total};
        int rc = webui_http_write_chunks(&s, delivery_send, delivery_clock, 3000, chunks, 2);
        CHECK(rc == (mode == 3 ? WEBUI_HTTP_WRITE_TIMEOUT : WEBUI_HTTP_WRITE_IO_ERROR));
        CHECK(s.sent < total);
    }
    free(received);
}

static void test_request_uses_absolute_deadline(void) {
    static const char wire[] = "GET / HTTP/1.1\r\nHost: test\r\n\r\n";
    slow_request_t source = {wire, sizeof(wire) - 1, 0, 0, 0, 0, 0};
    webui_http_request_t request;
    char buffer[128], *body = NULL;
    int result = webui_http_read_request(&source, slow_request_receive,
                                         slow_request_clock, 250,
                                         buffer, sizeof(buffer), &request, &body);
    CHECK(result == WEBUI_HTTP_READ_TIMEOUT);
    CHECK(source.offset < source.length);
    CHECK(source.max_wait_ms <= 250);
    CHECK(source.last_wait_ms < source.first_wait_ms);
}

static void test_request_rejects_data_after_deadline(void) {
    static const char wire[] = "GET / HTTP/1.1\r\nHost: test\r\n\r\n";
    slow_request_t source = {wire, sizeof(wire) - 1, 0, 0, 0, 0, 0};
    webui_http_request_t request;
    char buffer[128], *body = NULL;
    int result = webui_http_read_request(&source, delayed_complete_request_receive,
                                         slow_request_clock, 250,
                                         buffer, sizeof(buffer), &request, &body);
    CHECK(result == WEBUI_HTTP_READ_TIMEOUT);
    CHECK(source.now_ms > 250);
}

static void test_body_uses_same_absolute_deadline(void) {
    static const char wire[] = "POST /api/profile HTTP/1.1\r\n"
                               "Content-Length: 4\r\n\r\ntest";
    slow_request_t source = {wire, sizeof(wire) - 1, 0, 0, 0, 0, 0};
    webui_http_request_t request;
    char buffer[128], *body = NULL;
    int result = webui_http_read_request(&source, slow_request_body_receive,
                                         slow_request_clock, 250,
                                         buffer, sizeof(buffer), &request, &body);
    CHECK(result == WEBUI_HTTP_READ_TIMEOUT);
    CHECK(source.offset < source.length);
}

static void test_valid_get(void) {
    const char wire[] = "GET /api/status HTTP/1.1\r\n"
                        "Host: console.local\r\n"
                        "Authorization: Bearer "
                        "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\r\n"
                        "\r\n";
    webui_http_request_t request;
    CHECK(parse(wire, &request) == 1);
    CHECK(strcmp(request.method, "GET") == 0);
    CHECK(strcmp(request.target, "/api/status") == 0);
    CHECK(request.header_length == sizeof(wire) - 1);
    CHECK(request.content_length == 0);
    CHECK(request.has_authorization == 1);
    CHECK(webui_auth_matches(request.authorization, token));
}

static void test_valid_post_and_spacing(void) {
    const char wire[] = "POST    /api/profile?id=default  HTTP/1.1\r\n"
                        "Content-Length: 7\r\n\r\nname=ok";
    webui_http_request_t request;
    CHECK(parse(wire, &request) == 1);
    CHECK(strcmp(request.method, "POST") == 0);
    CHECK(strcmp(request.target, "/api/profile?id=default") == 0);
    CHECK(request.content_length == 7);
    CHECK(request.header_length + request.content_length == sizeof(wire) - 1);
}

static void test_profile_body_contract(void) {
    const char valid[] = "POST /api/profile?id=default HTTP/1.1\r\n"
                         "Content-Length: 7\r\n"
                         "Content-Type: text/plain;charset=UTF-8\r\n\r\n";
    const char missing_metadata[] = "POST /api/profile?id=default HTTP/1.1\r\n\r\n";
    const char wrong_type[] = "POST /api/profile?id=default HTTP/1.1\r\n"
                              "Content-Length: 7\r\n"
                              "Content-Type: application/json\r\n\r\n";
    const char no_charset[] = "POST /api/profile?id=default HTTP/1.1\r\n"
                              "Content-Length: 7\r\n"
                              "Content-Type: text/plain\r\n\r\n";
    const char wrong_charset[] = "POST /api/profile?id=default HTTP/1.1\r\n"
                                 "Content-Length: 7\r\n"
                                 "Content-Type: text/plain;charset=iso-8859-1\r\n\r\n";
    const char duplicate_type[] = "POST /api/profile?id=default HTTP/1.1\r\n"
                                  "Content-Length: 7\r\n"
                                  "Content-Type: text/plain\r\n"
                                  "Content-Type: application/json\r\n\r\n";
    webui_http_request_t request;

    CHECK(parse(valid, &request) == 1);
    CHECK(webui_http_profile_body_valid(&request, "name=ok"));
    CHECK(parse(valid, &request) == 1);
    CHECK(!webui_http_profile_body_valid(&request, "       "));
    CHECK(parse(missing_metadata, &request) == 1);
    CHECK(!webui_http_profile_body_valid(&request, ""));
    CHECK(parse(wrong_type, &request) == 1);
    CHECK(!webui_http_profile_body_valid(&request, "name=ok"));
    CHECK(parse(no_charset, &request) == 1);
    CHECK(!webui_http_profile_body_valid(&request, "name=ok"));
    CHECK(parse(wrong_charset, &request) == 1);
    CHECK(!webui_http_profile_body_valid(&request, "name=ok"));
    CHECK(parse(duplicate_type, &request) == -1);
}

static void test_origin_matches_request_host(void) {
    const char same_origin[] = "POST /api/profile?id=default HTTP/1.1\r\n"
                               "Host: console.local:8090\r\n"
                               "Origin: http://console.local:8090\r\n\r\n";
    const char different_origin[] = "POST /api/profile?id=default HTTP/1.1\r\n"
                                    "Host: console.local:8090\r\n"
                                    "Origin: http://attacker.example\r\n\r\n";
    const char origin_without_host[] = "POST /api/profile?id=default HTTP/1.1\r\n"
                                       "Origin: http://console.local:8090\r\n\r\n";
    const char case_insensitive_origin[] = "POST /api/profile?id=default HTTP/1.1\r\n"
                                           "Host: CONSOLE.local:8090\r\n"
                                           "Origin: HTTP://console.LOCAL:8090\r\n\r\n";
    const char wrong_scheme[] = "POST /api/profile?id=default HTTP/1.1\r\n"
                                "Host: console.local:8090\r\n"
                                "Origin: https://console.local:8090\r\n\r\n";
    const char no_origin[] = "POST /api/profile?id=default HTTP/1.1\r\n"
                             "Host: console.local:8090\r\n\r\n";
    webui_http_request_t request;

    CHECK(parse(same_origin, &request) == 1);
    CHECK(webui_http_origin_matches_host(&request));
    CHECK(parse(different_origin, &request) == 1);
    CHECK(!webui_http_origin_matches_host(&request));
    CHECK(parse(origin_without_host, &request) == 1);
    CHECK(!webui_http_origin_matches_host(&request));
    CHECK(parse(case_insensitive_origin, &request) == 1);
    CHECK(webui_http_origin_matches_host(&request));
    CHECK(parse(wrong_scheme, &request) == 1);
    CHECK(!webui_http_origin_matches_host(&request));
    CHECK(parse(no_origin, &request) == 1);
    CHECK(webui_http_origin_matches_host(&request));
}

static void test_incomplete_headers(void) {
    const char wire[] = "GET /api/status HTTP/1.1\r\nHost: console.local\r\n";
    webui_http_request_t request;
    CHECK(webui_http_parse_headers(wire, sizeof(wire) - 1, &request) == 0);
}

static void test_reject_duplicate_or_ambiguous_headers(void) {
    const char duplicate_length[] = "POST /api/profile HTTP/1.1\r\n"
                                    "Content-Length: 1\r\nContent-Length: 1\r\n\r\n";
    const char duplicate_auth[] = "GET /api/status HTTP/1.1\r\n"
                                  "Authorization: Bearer abc\r\n"
                                  "Authorization: Bearer def\r\n\r\n";
    const char duplicate_host[] = "GET /api/status HTTP/1.1\r\n"
                                  "Host: console.local\r\n"
                                  "Host: console.local\r\n\r\n";
    const char duplicate_origin[] = "GET /api/status HTTP/1.1\r\n"
                                    "Host: console.local\r\n"
                                    "Origin: http://console.local\r\n"
                                    "Origin: http://attacker.example\r\n\r\n";
    const char transfer_encoding[] = "POST /api/profile HTTP/1.1\r\n"
                                     "Transfer-Encoding: chunked\r\n\r\n";
    const char folded[] = "GET /api/status HTTP/1.1\r\n Host: console.local\r\n\r\n";
    webui_http_request_t request;
    CHECK(parse(duplicate_length, &request) == -1);
    CHECK(parse(duplicate_auth, &request) == -1);
    CHECK(parse(duplicate_host, &request) == -1);
    CHECK(parse(duplicate_origin, &request) == -1);
    CHECK(parse(transfer_encoding, &request) == -1);
    CHECK(parse(folded, &request) == -1);
}

static void test_reject_invalid_request_lines_and_lengths(void) {
    const char bad_method[] = "DELETE /api/status HTTP/1.1\r\n\r\n";
    const char bad_length[] = "POST /api/profile HTTP/1.1\r\nContent-Length: +3\r\n\r\n";
    const char huge_length[] = "POST /api/profile HTTP/1.1\r\nContent-Length: 999999999999999999999999999999\r\n\r\n";
    const char over_limit[] = "POST /api/profile HTTP/1.1\r\nContent-Length: 32769\r\n\r\n";
    const char get_with_body[] = "GET /api/status HTTP/1.1\r\nContent-Length: 1\r\n\r\nx";
    const char similar_header_name[] = "POST /api/profile HTTP/1.1\r\n"
                                       "Content-LengthX: 0\r\n\r\n";
    webui_http_request_t request;
    CHECK(parse(bad_method, &request) == -1);
    CHECK(parse(bad_length, &request) == -1);
    CHECK(parse(huge_length, &request) == -1);
    CHECK(parse(over_limit, &request) == -1);
    CHECK(parse(get_with_body, &request) == -1);
    CHECK(parse(similar_header_name, &request) == 1);
    CHECK(request.has_content_length == 0);
}

static void test_authentication(void) {
    char auth[sizeof("Bearer ") + WEBUI_TOKEN_HEX_LENGTH];
    (void)snprintf(auth, sizeof(auth), "Bearer %s", token);
    CHECK(webui_token_is_valid(token));
    CHECK(!webui_token_is_valid("short"));
    CHECK(webui_auth_matches(auth, token));
    CHECK(!webui_auth_path_allows("/api/status", NULL, token));
    CHECK(webui_auth_path_allows("/api/status", auth, token));
    CHECK(webui_auth_path_allows("/", NULL, token));
    CHECK(!webui_auth_path_allows(NULL, NULL, token));
    CHECK(!webui_auth_matches("Bearer ", token));
    CHECK(!webui_auth_matches(NULL, token));
    CHECK(!webui_auth_matches("Basic 0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef", token));
    auth[sizeof(auth) - 2] = auth[sizeof(auth) - 2] == '0' ? '1' : '0';
    CHECK(!webui_auth_matches(auth, token));
}

static void test_token_file_lifecycle(void) {
    char directory[] = "/tmp/steamcontroller-webui-XXXXXX";
    char path[256], first[WEBUI_TOKEN_HEX_LENGTH + 1];
    char second[WEBUI_TOKEN_HEX_LENGTH + 1];
    struct stat st;
    int created = 0;
    int result;
    int fd;
    char *made_directory;

    made_directory = mkdtemp(directory);
    CHECK(made_directory != NULL);
    if (!made_directory) return;
    CHECK(snprintf(path, sizeof(path), "%s/webui.token", directory) > 0);
    result = webui_auth_load_or_create(directory, first, &created);
    CHECK(result == 0);
    if (result != 0) { (void)rmdir(directory); return; }
    CHECK(created == 1);
    CHECK(webui_token_is_valid(first));
    CHECK(stat(path, &st) == 0);
    CHECK((st.st_mode & 0777) == 0600);

    created = 1;
    CHECK(webui_auth_load_or_create(directory, second, &created) == 0);
    CHECK(created == 0);
    CHECK(strcmp(first, second) == 0);

    CHECK(chmod(path, 0644) == 0);
    CHECK(webui_auth_load_or_create(directory, second, &created) == -1);
    CHECK(chmod(path, 0600) == 0);

    fd = open(path, O_WRONLY | O_TRUNC);
    CHECK(fd >= 0);
    if (fd >= 0) {
        CHECK(write(fd, "not a token\n", 12) == 12);
        CHECK(close(fd) == 0);
    }
    CHECK(webui_auth_load_or_create(directory, second, &created) == -1);
    CHECK(unlink(path) == 0);
    CHECK(symlink("missing-token-target", path) == 0);
    CHECK(webui_auth_load_or_create(directory, second, &created) == -1);
    CHECK(unlink(path) == 0);
    CHECK(mkfifo(path, 0600) == 0);
    CHECK(webui_auth_load_or_create(directory, second, &created) == -1);
    CHECK(unlink(path) == 0);
    CHECK(webui_auth_load_or_create("/dev/null/invalid", second, &created) == -1);
    CHECK(rmdir(directory) == 0);
}

static int reject_unauthorized(void *context, const webui_http_request_t *r) {
    (void)context;
    return webui_auth_matches(r->authorization, token)?WEBUI_HTTP_READ_OK:WEBUI_HTTP_READ_UNAUTHORIZED;
}
static void test_admission_precedes_body(void) {
    static const char wire[] = "POST /api/profile HTTP/1.1\r\nContent-Length: 100\r\n\r\n";
    slow_request_t source = {wire, sizeof(wire)-1, 0, 0, 0, 0, 0};
    webui_http_request_t request;char buffer[256],*body;
    CHECK(webui_http_read_request_checked(&source, slow_request_body_receive,
        slow_request_clock, 5000, buffer, sizeof(buffer), &request, &body,
        reject_unauthorized, NULL)==WEBUI_HTTP_READ_UNAUTHORIZED);
    CHECK(source.offset==sizeof(wire)-1);
    CHECK(source.now_ms==10);
}

typedef struct { webui_pairing_t *pair; const char *code; int result; } redemption_t;
static void *redeem_thread(void *arg) {
    redemption_t *r = arg;
    r->result = webui_pairing_redeem(r->pair, r->code, 1001);
    return NULL;
}
static void test_pairing(void) {
    webui_pairing_t pair;
    char code[WEBUI_PAIR_CODE_LENGTH + 1];
    CHECK(webui_pairing_init(&pair, 1000, code) == 0);
    CHECK(strlen(code) == 8);
    CHECK(webui_pairing_redeem(&pair, "invalid", 1000) == 0);
    CHECK(webui_pairing_redeem(&pair, code, 1001) == 1);
    CHECK(webui_pairing_redeem(&pair, code, 1002) == 0);
    webui_pairing_destroy(&pair);
    CHECK(webui_pairing_init(&pair, 1000, code) == 0);
    CHECK(webui_pairing_redeem(&pair, code, 1000 + WEBUI_PAIR_TTL_MS) == 0);
    webui_pairing_destroy(&pair);
    CHECK(webui_pairing_init(&pair, 1000, code) == 0);
    for (unsigned i = 0; i < WEBUI_PAIR_ATTEMPTS; ++i)
        CHECK(webui_pairing_redeem(&pair, "bad", 1000) == 0);
    CHECK(webui_pairing_redeem(&pair, code, 1001) == 0);
    webui_pairing_destroy(&pair);
    CHECK(webui_pairing_init(&pair, 1000, code) == 0);
    CHECK(webui_pairing_redeem(&pair, code, 999) == 0);
    webui_pairing_destroy(&pair);
    CHECK(webui_pairing_init(&pair, 1000, code) == 0);
    pthread_t threads[8]; redemption_t r[8];
    for (int i = 0; i < 8; ++i) {
        r[i] = (redemption_t){&pair, code, 0};
        CHECK(pthread_create(&threads[i], NULL, redeem_thread, &r[i]) == 0);
    }
    int successes = 0;
    for (int i = 0; i < 8; ++i) { CHECK(pthread_join(threads[i], NULL) == 0); successes += r[i].result; }
    CHECK(successes == 1);
    webui_pairing_destroy(&pair);
}

int main(void) {
    test_request_uses_absolute_deadline();
    test_request_rejects_data_after_deadline();
    test_body_uses_same_absolute_deadline();
    test_response_uses_one_absolute_deadline();
    test_response_writes_all_chunks();
    test_embedded_delivery();
    test_valid_get();
    test_valid_post_and_spacing();
    test_profile_body_contract();
    test_origin_matches_request_host();
    test_incomplete_headers();
    test_reject_duplicate_or_ambiguous_headers();
    test_reject_invalid_request_lines_and_lengths();
    test_authentication();
    test_token_file_lifecycle();
    test_pairing();
    test_admission_precedes_body();
    if (failures) {
        fprintf(stderr, "%d of %d web UI HTTP/auth checks failed\n", failures, checks);
        return 1;
    }
    printf("web UI HTTP/auth checks passed (%d checks)\n", checks);
    return 0;
}
