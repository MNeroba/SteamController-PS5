#include "webui_route.h"

#include <stdio.h>
#include <string.h>

static int checks;
static int failures;

#define CHECK(condition) do { \
    checks++; \
    if (!(condition)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        failures++; \
    } \
} while (0)

static void test_route_names_are_exact(void) {
    webui_route_request_t request;
    static const struct {
        const char *target;
        webui_route_id_t expected;
    } canonical[] = {
        {"/", WEBUI_ROUTE_INDEX},
        {"/index", WEBUI_ROUTE_INDEX},
        {"/api/status", WEBUI_ROUTE_STATUS},
        {"/api/icon?id=CUSA00001", WEBUI_ROUTE_ICON},
        {"/api/haptic?kind=left&via=4", WEBUI_ROUTE_HAPTIC},
        {"/api/profiles", WEBUI_ROUTE_PROFILES},
        {"/api/profile?id=default", WEBUI_ROUTE_PROFILE},
        {"/api/delete?id=CUSA00001", WEBUI_ROUTE_DELETE},
        {"/api/override?id=", WEBUI_ROUTE_OVERRIDE},
        {"/api/games", WEBUI_ROUTE_GAMES},
        {"/api/gamehooks?input=0&vibration=1&audio=1&mode=2", WEBUI_ROUTE_GAMEHOOKS},
        {"/api/pause?on=1", WEBUI_ROUTE_PAUSE},
        {"/api/dualsense?on=0", WEBUI_ROUTE_DUALSENSE},
        {"/api/bridge", WEBUI_ROUTE_BRIDGE},
        {"/api/log?prev=1", WEBUI_ROUTE_LOG}
    };
    size_t i;
    for (i = 0; i < sizeof(canonical) / sizeof(canonical[0]); i++) {
        CHECK(webui_route_parse(canonical[i].target, &request) == 0);
        CHECK(request.id == canonical[i].expected);
    }
    CHECK(webui_route_parse("/api/pause?on=1", &request) == 0);
    CHECK(request.id == WEBUI_ROUTE_PAUSE);
    CHECK(webui_route_parse("/api/pause-extra?on=1", &request) == 0);
    CHECK(request.id == WEBUI_ROUTE_NOT_FOUND);
    CHECK(webui_route_parse("/apix", &request) == 0);
    CHECK(request.id == WEBUI_ROUTE_NOT_FOUND);
    CHECK(webui_route_parse("/api/profile-extra?id=default", &request) == 0);
    CHECK(request.id == WEBUI_ROUTE_NOT_FOUND);
}

static void test_query_keys_are_exact_and_unique(void) {
    webui_route_request_t request;
    char value[32];
    CHECK(webui_route_parse("/api/profile?notid=default", &request) == -1);
    CHECK(webui_route_parse("/api/profile?id=default", &request) == 0);
    CHECK(webui_route_param(&request, "notid", value, sizeof(value)) == 0);
    CHECK(webui_route_parse("/api/pause?on=0&on=1", &request) == -1);
    CHECK(webui_route_parse("/api/status?on=1", &request) == -1);
    CHECK(webui_route_parse("/api/pause?on", &request) == -1);
    CHECK(webui_route_parse("/api/pause?on=1&", &request) == -1);
    CHECK(webui_route_parse("/api/pause?on==1", &request) == -1);
    CHECK(webui_route_parse("/api/pause?on=%31", &request) == -1);
}

static void test_query_values_are_bounded_and_methods_explicit(void) {
    webui_route_request_t request;
    char value[4];
    CHECK(webui_route_parse("/api/pause?on=1", &request) == 0);
    CHECK(webui_route_param(&request, "on", value, sizeof(value)) == 1);
    CHECK(strcmp(value, "1") == 0);
    CHECK(webui_route_method_allowed(request.id, 1));
    CHECK(!webui_route_method_allowed(request.id, 0));
    CHECK(webui_route_parse("/api/profile?id=ABCDE", &request) == 0);
    CHECK(webui_route_param(&request, "id", value, sizeof(value)) == -1);
}

int main(void) {
    webui_route_request_t tile;
    CHECK(webui_route_parse("/api/settings-tile", &tile)==0);
    CHECK(tile.id==WEBUI_ROUTE_SETTINGS_TILE);
    CHECK(webui_route_method_allowed(tile.id,1));
    CHECK(!webui_route_method_allowed(tile.id,0));
    CHECK(webui_route_parse("/api/settings-tile?title=OTHER", &tile)==-1);
    test_route_names_are_exact();
    test_query_keys_are_exact_and_unique();
    test_query_values_are_bounded_and_methods_explicit();
    if (failures) {
        fprintf(stderr, "%d/%d webui route checks failed\n", failures, checks);
        return 1;
    }
    printf("%d webui route checks passed\n", checks);
    return 0;
}
