#ifndef WEBUI_JSON_H
#define WEBUI_JSON_H

#include <stddef.h>

typedef struct {
    char *buffer;
    size_t length;
    size_t capacity;
    int failed;
} webui_json_writer_t;

void webui_json_writer_init(webui_json_writer_t *writer, char *buffer,
                            size_t capacity);
void webui_json_append(webui_json_writer_t *writer, const char *text);
void webui_json_append_string(webui_json_writer_t *writer, const char *text);

#endif
