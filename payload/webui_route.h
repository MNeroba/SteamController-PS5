#ifndef WEBUI_ROUTE_H
#define WEBUI_ROUTE_H

#include <stddef.h>

typedef enum {
    WEBUI_ROUTE_NOT_FOUND = 0,
    WEBUI_ROUTE_INDEX,
    WEBUI_ROUTE_STATUS,
    WEBUI_ROUTE_ICON,
    WEBUI_ROUTE_HAPTIC,
    WEBUI_ROUTE_PROFILES,
    WEBUI_ROUTE_PROFILE,
    WEBUI_ROUTE_DELETE,
    WEBUI_ROUTE_OVERRIDE,
    WEBUI_ROUTE_GAMES,
    WEBUI_ROUTE_GAMEHOOKS,
    WEBUI_ROUTE_PAUSE,
    WEBUI_ROUTE_DUALSENSE,
    WEBUI_ROUTE_BRIDGE,
    WEBUI_ROUTE_LOG
} webui_route_id_t;

typedef struct {
    webui_route_id_t id;
    const char *target;
} webui_route_request_t;

/* Parse exact route paths and validate the route-specific ASCII query schema.
 * Unknown paths return success with WEBUI_ROUTE_NOT_FOUND; malformed known
 * routes/queries return -1. The target storage must outlive the request. */
int webui_route_parse(const char *target, webui_route_request_t *request);

/* is_post is nonzero for POST and zero for GET. */
int webui_route_method_allowed(webui_route_id_t route, int is_post);

/* Copy an exact query key into a bounded buffer. Returns 1 when present, 0
 * when absent, and -1 for invalid arguments or a value that does not fit. */
int webui_route_param(const webui_route_request_t *request, const char *name,
                      char *value, size_t capacity);

#endif
