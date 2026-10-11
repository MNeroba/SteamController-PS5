#include "webui_route.h"

#include <string.h>

typedef struct {
    const char *path;
    size_t length;
    webui_route_id_t id;
} route_path_t;

typedef struct {
    const char *name;
    size_t length;
} query_name_t;

#define ROUTE_PATH(path_value, route_value) \
    {path_value, sizeof(path_value) - 1, route_value}

static const route_path_t routes[] = {
    ROUTE_PATH("/", WEBUI_ROUTE_INDEX),
    ROUTE_PATH("/index", WEBUI_ROUTE_INDEX),
    ROUTE_PATH("/pair", WEBUI_ROUTE_PAIR),
    ROUTE_PATH("/api/recover-default", WEBUI_ROUTE_RECOVER_DEFAULT),
    ROUTE_PATH("/api/status", WEBUI_ROUTE_STATUS),
    ROUTE_PATH("/api/icon", WEBUI_ROUTE_ICON),
    ROUTE_PATH("/api/haptic", WEBUI_ROUTE_HAPTIC),
    ROUTE_PATH("/api/profiles", WEBUI_ROUTE_PROFILES),
    ROUTE_PATH("/api/profile", WEBUI_ROUTE_PROFILE),
    ROUTE_PATH("/api/delete", WEBUI_ROUTE_DELETE),
    ROUTE_PATH("/api/override", WEBUI_ROUTE_OVERRIDE),
    ROUTE_PATH("/api/games", WEBUI_ROUTE_GAMES),
    ROUTE_PATH("/api/gamehooks", WEBUI_ROUTE_GAMEHOOKS),
    ROUTE_PATH("/api/pause", WEBUI_ROUTE_PAUSE),
    ROUTE_PATH("/api/dualsense", WEBUI_ROUTE_DUALSENSE),
    ROUTE_PATH("/api/bridge", WEBUI_ROUTE_BRIDGE),
    ROUTE_PATH("/api/log", WEBUI_ROUTE_LOG)
};

static const char *const no_params[] = {NULL};
static const char *const icon_params[] = {"id", NULL};
static const char *const haptic_params[] = {"kind", "via", NULL};
static const char *const id_params[] = {"id", NULL};
static const char *const gamehook_params[] = {"input", "vibration", "audio", "mode", NULL};
static const char *const on_params[] = {"on", NULL};
static const char *const log_params[] = {"prev", NULL};

static const char *const *allowed_params(webui_route_id_t route) {
    switch (route) {
        case WEBUI_ROUTE_ICON: return icon_params;
        case WEBUI_ROUTE_HAPTIC: return haptic_params;
        case WEBUI_ROUTE_PROFILE:
        case WEBUI_ROUTE_DELETE:
        case WEBUI_ROUTE_OVERRIDE: return id_params;
        case WEBUI_ROUTE_GAMEHOOKS: return gamehook_params;
        case WEBUI_ROUTE_PAUSE:
        case WEBUI_ROUTE_DUALSENSE: return on_params;
        case WEBUI_ROUTE_LOG: return log_params;
        case WEBUI_ROUTE_RECOVER_DEFAULT:
        case WEBUI_ROUTE_PAIR:
        case WEBUI_ROUTE_INDEX:
        case WEBUI_ROUTE_STATUS:
        case WEBUI_ROUTE_PROFILES:
        case WEBUI_ROUTE_GAMES:
        case WEBUI_ROUTE_BRIDGE: return no_params;
        case WEBUI_ROUTE_NOT_FOUND:
        default: return NULL;
    }
}

static int ascii_param_char(unsigned char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') || c == '_' || c == '-';
}

static int name_allowed(webui_route_id_t route, const char *name, size_t length) {
    const char *const *allowed = allowed_params(route);
    size_t i;
    if (!allowed) return 0;
    for (i = 0; allowed[i]; i++) {
        if (strlen(allowed[i]) == length && memcmp(allowed[i], name, length) == 0)
            return 1;
    }
    return 0;
}

static int validate_query(webui_route_id_t route, const char *query) {
    query_name_t seen[4];
    size_t count = 0;
    const char *pair = query;
    if (!query) return 0;
    if (!*query) return -1;
    for (;;) {
        const char *end = strchr(pair, '&');
        const char *equal = strchr(pair, '=');
        size_t pair_length = end ? (size_t)(end - pair) : strlen(pair);
        size_t name_length;
        size_t value_length;
        size_t i;
        if (!equal || (end && equal >= end)) return -1;
        name_length = (size_t)(equal - pair);
        if (name_length == 0 || name_length >= pair_length) return -1;
        value_length = pair_length - name_length - 1;
        if (!name_allowed(route, pair, name_length) || count >= 4) return -1;
        for (i = 0; i < name_length; i++)
            if (!ascii_param_char((unsigned char)pair[i])) return -1;
        for (i = 0; i < value_length; i++)
            if (!ascii_param_char((unsigned char)equal[1 + i])) return -1;
        for (i = 0; i < count; i++)
            if (seen[i].length == name_length &&
                memcmp(seen[i].name, pair, name_length) == 0)
                return -1;
        seen[count].name = pair;
        seen[count].length = name_length;
        count++;
        if (!end) return 0;
        pair = end + 1;
        if (!*pair) return -1;
    }
}

int webui_route_parse(const char *target, webui_route_request_t *request) {
    const char *query;
    size_t path_length;
    size_t i;
    if (!target || !request || target[0] != '/') return -1;
    request->id = WEBUI_ROUTE_NOT_FOUND;
    request->target = target;
    query = strchr(target, '?');
    path_length = query ? (size_t)(query - target) : strlen(target);
    for (i = 0; i < sizeof(routes) / sizeof(routes[0]); i++) {
        if (path_length == routes[i].length &&
            memcmp(target, routes[i].path, path_length) == 0) {
            request->id = routes[i].id;
            break;
        }
    }
    if (request->id == WEBUI_ROUTE_NOT_FOUND) return 0;
    return validate_query(request->id, query ? query + 1 : NULL);
}

int webui_route_method_allowed(webui_route_id_t route, int is_post) {
    switch (route) {
        case WEBUI_ROUTE_INDEX:
        case WEBUI_ROUTE_STATUS:
        case WEBUI_ROUTE_ICON:
        case WEBUI_ROUTE_PROFILES:
        case WEBUI_ROUTE_GAMES:
        case WEBUI_ROUTE_LOG:
            return !is_post;
        case WEBUI_ROUTE_RECOVER_DEFAULT:
        case WEBUI_ROUTE_PAIR:
        case WEBUI_ROUTE_HAPTIC:
        case WEBUI_ROUTE_DELETE:
        case WEBUI_ROUTE_OVERRIDE:
        case WEBUI_ROUTE_PAUSE:
            return is_post;
        case WEBUI_ROUTE_PROFILE:
        case WEBUI_ROUTE_GAMEHOOKS:
        case WEBUI_ROUTE_DUALSENSE:
        case WEBUI_ROUTE_BRIDGE:
            return 1;
        case WEBUI_ROUTE_NOT_FOUND:
        default:
            return 0;
    }
}

int webui_route_param(const webui_route_request_t *request, const char *name,
                      char *value, size_t capacity) {
    const char *query;
    size_t name_length;
    if (!request || !request->target || !name || !value || capacity == 0)
        return -1;
    value[0] = '\0';
    name_length = strlen(name);
    if (name_length == 0) return -1;
    query = strchr(request->target, '?');
    if (!query) return 0;
    query++;
    while (*query) {
        const char *end = strchr(query, '&');
        const char *equal = strchr(query, '=');
        size_t pair_length = end ? (size_t)(end - query) : strlen(query);
        size_t current_name_length;
        size_t value_length;
        if (!equal || (end && equal >= end)) return -1;
        current_name_length = (size_t)(equal - query);
        if (current_name_length >= pair_length) return -1;
        value_length = pair_length - current_name_length - 1;
        if (current_name_length == name_length &&
            memcmp(query, name, name_length) == 0) {
            if (value_length >= capacity) return -1;
            memcpy(value, equal + 1, value_length);
            value[value_length] = '\0';
            return 1;
        }
        if (!end) break;
        query = end + 1;
    }
    return 0;
}
