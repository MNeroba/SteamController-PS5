#include "webui_json.h"

#include <string.h>

void webui_json_writer_init(webui_json_writer_t *writer, char *buffer,
                            size_t capacity) {
    if (!writer) return;
    writer->buffer = buffer;
    writer->length = 0;
    writer->capacity = capacity;
    writer->failed = !buffer || capacity == 0;
    if (!writer->failed) buffer[0] = '\0';
}

void webui_json_append(webui_json_writer_t *writer, const char *text) {
    size_t length;
    if (!writer || writer->failed) return;
    if (!text || !writer->buffer || writer->length >= writer->capacity) {
        writer->failed = 1;
        return;
    }
    length = strlen(text);
    if (length > writer->capacity - writer->length - 1) {
        writer->failed = 1;
        return;
    }
    memcpy(writer->buffer + writer->length, text, length);
    writer->length += length;
    writer->buffer[writer->length] = '\0';
}

void webui_json_append_string(webui_json_writer_t *writer, const char *text) {
    static const char hex[] = "0123456789abcdef";
    if (!writer || writer->failed) return;
    if (!text) {
        writer->failed = 1;
        return;
    }
    webui_json_append(writer, "\"");
    for (const unsigned char *p = (const unsigned char *)text; *p && !writer->failed; p++) {
        char escaped[7] = {0};
        char literal[2] = {(char)*p, '\0'};
        if (*p == '"' || *p == '\\') {
            escaped[0] = '\\'; escaped[1] = (char)*p;
            webui_json_append(writer, escaped);
        } else if (*p < 0x20) {
            escaped[0] = '\\'; escaped[1] = 'u'; escaped[2] = '0'; escaped[3] = '0';
            escaped[4] = hex[*p >> 4]; escaped[5] = hex[*p & 0x0f];
            webui_json_append(writer, escaped);
        } else {
            webui_json_append(writer, literal);
        }
    }
    webui_json_append(writer, "\"");
}
