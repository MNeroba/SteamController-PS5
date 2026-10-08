#define _POSIX_C_SOURCE 200809L
#define _DARWIN_C_SOURCE

#include "webui_auth.h"
#include "webui_http.h"

#include <fcntl.h>
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
    const char transfer_encoding[] = "POST /api/profile HTTP/1.1\r\n"
                                     "Transfer-Encoding: chunked\r\n\r\n";
    const char folded[] = "GET /api/status HTTP/1.1\r\n Host: console.local\r\n\r\n";
    webui_http_request_t request;
    CHECK(parse(duplicate_length, &request) == -1);
    CHECK(parse(duplicate_auth, &request) == -1);
    CHECK(parse(transfer_encoding, &request) == -1);
    CHECK(parse(folded, &request) == -1);
}

static void test_reject_invalid_request_lines_and_lengths(void) {
    const char bad_method[] = "DELETE /api/status HTTP/1.1\r\n\r\n";
    const char bad_length[] = "POST /api/profile HTTP/1.1\r\nContent-Length: +3\r\n\r\n";
    const char huge_length[] = "POST /api/profile HTTP/1.1\r\nContent-Length: 999999999999999999999999999999\r\n\r\n";
    const char over_limit[] = "POST /api/profile HTTP/1.1\r\nContent-Length: 16385\r\n\r\n";
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
    CHECK(rmdir(directory) == 0);
}

int main(void) {
    test_valid_get();
    test_valid_post_and_spacing();
    test_incomplete_headers();
    test_reject_duplicate_or_ambiguous_headers();
    test_reject_invalid_request_lines_and_lengths();
    test_authentication();
    test_token_file_lifecycle();
    if (failures) {
        fprintf(stderr, "%d of %d web UI HTTP/auth checks failed\n", failures, checks);
        return 1;
    }
    printf("web UI HTTP/auth checks passed (%d checks)\n", checks);
    return 0;
}
