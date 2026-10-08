#include "webui_json.h"

#include <stdio.h>
#include <string.h>

static int failures;
static int checks;

#define CHECK(cond) do { \
    checks++; \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        failures++; \
    } \
} while (0)

static void test_escaping_and_append(void) {
    char buffer[64];
    webui_json_writer_t writer;
    webui_json_writer_init(&writer, buffer, sizeof(buffer));
    webui_json_append(&writer, "{\"v\":");
    webui_json_append_string(&writer, "quote\" slash\\ line\n tab\t");
    webui_json_append(&writer, "}");
    CHECK(!writer.failed);
    CHECK(strcmp(buffer, "{\"v\":\"quote\\\" slash\\\\ line\\u000a tab\\u0009\"}") == 0);
    CHECK(writer.length == strlen(buffer));
}

static void test_overflow_is_reported_and_bounded(void) {
    char buffer[5];
    webui_json_writer_t writer;
    webui_json_writer_init(&writer, buffer, sizeof(buffer));
    webui_json_append(&writer, "abcd");
    CHECK(!writer.failed && writer.length == 4 && buffer[4] == '\0');
    webui_json_append(&writer, "e");
    CHECK(writer.failed);
    CHECK(writer.length == 4 && strcmp(buffer, "abcd") == 0);
    webui_json_append(&writer, "f");
    CHECK(writer.length == 4 && strcmp(buffer, "abcd") == 0);

    char small[3];
    webui_json_writer_init(&writer, small, sizeof(small));
    webui_json_append_string(&writer, "x"); /* quotes plus value need 3 bytes plus NUL */
    CHECK(writer.failed && writer.length <= sizeof(small) - 1 &&
          small[writer.length] == '\0');
    webui_json_writer_init(&writer, NULL, 0);
    CHECK(writer.failed);
}

int main(void) {
    test_escaping_and_append();
    test_overflow_is_reported_and_bounded();
    if (failures) {
        fprintf(stderr, "%d of %d web JSON checks failed\n", failures, checks);
        return 1;
    }
    printf("web JSON checks passed (%d checks)\n", checks);
    return 0;
}
