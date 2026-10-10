/* webui.c — tiny HTTP server for the Steam Controller remap portal */

#include "webui.h"
#include "sc2_profile.h"
#include "game_list.h"
#include "sc2_haptics.h"
#include "sc2_status.h"
#include "sc2_menu.h"
#include "controller_sc2.h"
#include "sc2_gyro.h"
#include "ds_handoff.h"
#include "bridge_probe.h"
#include "game_hooks.h"
#include "webui_html.h"
#include "webui_auth.h"
#include "webui_http.h"
#include "webui_route.h"
#include "webui_peer_policy.h"
#include "webui_json.h"
#include "runtime_control.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <time.h>
#include <netinet/in.h>

#ifdef __PROSPERO__
void ghostpad_status_log(const char *fmt, ...);   /* gc_main.c: klog + /data/ghostpad/gc_status.log */
#define LOG(...) ghostpad_status_log("[GC] " __VA_ARGS__)
#else
#define LOG(...) fprintf(stderr, __VA_ARGS__)
#endif

#define REQ_MAX (WEBUI_HTTP_HEADER_MAX + WEBUI_HTTP_BODY_MAX)
#define WEBUI_TOKEN_PATH "/data/ghostpad/webui.token"
#define WEBUI_RESPONSE_TIMEOUT_MS 5000u

static char g_webui_token[WEBUI_TOKEN_HEX_LENGTH + 1];
static pthread_t g_webui_thread;
static int g_webui_thread_started;

static void send_response_chunks(int fd, const webui_http_chunk_t *chunks,
                                 size_t chunk_count);

static void reply(int fd, int code, const char *type, const char *body, size_t n) {
    char h[256];
    const char *reason = code == 200 ? "OK" : code == 401 ? "Unauthorized" :
                         code == 403 ? "Forbidden" :
                         code == 404 ? "Not Found" : code == 405 ? "Method Not Allowed" :
                         code == 413 ? "Payload Too Large" :
                         code == 408 ? "Request Timeout" :
                         code == 500 ? "Internal Server Error" : "Bad Request";
    int hl = snprintf(h, sizeof(h),
        "HTTP/1.1 %d %s\r\nContent-Type: %s\r\nContent-Length: %zu\r\n"
        "Cache-Control: no-store\r\n%sConnection: close\r\n\r\n",
        code, reason, type, n,
        code == 401 ? "WWW-Authenticate: Bearer realm=\"SteamController-PS5\"\r\n" : "");
    if (hl < 0 || (size_t)hl >= sizeof(h)) return;
    webui_http_chunk_t chunks[] = {{h, (size_t)hl}, {body, n}};
    send_response_chunks(fd, chunks, sizeof(chunks) / sizeof(chunks[0]));
}
#define REPLY_TXT(fd, code, s) reply(fd, code, "text/plain", s, strlen(s))

static int initialize_webui_auth(void) {
    int created = 0;
    if (webui_auth_load_or_create("/data/ghostpad", g_webui_token, &created) != 0)
        return -1;
    if (created) LOG("webui: generated a private portal token at %s\n", WEBUI_TOKEN_PATH);
    return 0;
}

/* query param ?id=... (validated) */
static int get_id(const webui_route_request_t *request, char *out) {
    int result;
    out[0] = 0;
    result = webui_route_param(request, "id", out, SC2_ID_MAX + 1);
    if (result != 1) return result;
    if (out[0] == 0) return 0;
    return sc2_store_valid_id(out) ? 1 : -1;
}

typedef webui_json_writer_t sb_t;
#define sb_put webui_json_append
#define json_str webui_json_append_string

static void sb_printf(sb_t *s, const char *format, ...) {
    char formatted[256];
    va_list ap;
    va_start(ap, format);
    int n = vsnprintf(formatted, sizeof(formatted), format, ap);
    va_end(ap);
    if (n < 0 || (size_t)n >= sizeof(formatted)) {
        s->failed = 1;
        return;
    }
    sb_put(s, formatted);
}

static void list_cb(const char *id, const char *name, void *u) {
    sb_t *s = u;
    if (s->length > 1) sb_put(s, ",");
    sb_put(s, "{\"id\":"); json_str(s, id);
    sb_put(s, ",\"name\":"); json_str(s, name); sb_put(s, "}");
}

#define WEBUI_REQUEST_TIMEOUT_MS 5000u
#define WEBUI_IDLE_TIMEOUT_MS 2000u

static ssize_t webui_socket_receive(void *context, char *buffer, size_t capacity,
                                    unsigned wait_ms) {
    int fd = *(int *)context;
    struct timeval timeout;
    if (wait_ms == 0) { errno = EAGAIN; return -1; }
    if (wait_ms > WEBUI_IDLE_TIMEOUT_MS) wait_ms = WEBUI_IDLE_TIMEOUT_MS;
    timeout.tv_sec = (time_t)(wait_ms / 1000u);
    timeout.tv_usec = (suseconds_t)(wait_ms % 1000u) * 1000;
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) != 0)
        return -1;
    return recv(fd, buffer, capacity, 0);
}

static ssize_t webui_socket_send(void *context, const char *buffer, size_t length,
                                 unsigned wait_ms) {
    int fd = *(int *)context;
    struct timeval timeout;
    if (wait_ms == 0) { errno = EAGAIN; return -1; }
    if (wait_ms > WEBUI_IDLE_TIMEOUT_MS) wait_ms = WEBUI_IDLE_TIMEOUT_MS;
    timeout.tv_sec = (time_t)(wait_ms / 1000u);
    timeout.tv_usec = (suseconds_t)(wait_ms % 1000u) * 1000;
    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) != 0)
        return -1;
#ifdef MSG_NOSIGNAL
    return send(fd, buffer, length, MSG_NOSIGNAL);
#else
    return send(fd, buffer, length, 0);
#endif
}

static int webui_monotonic_ms(void *context, uint64_t *now_ms) {
    struct timespec now;
    (void)context;
    if (!now_ms || clock_gettime(CLOCK_MONOTONIC, &now) != 0) return -1;
    *now_ms = (uint64_t)now.tv_sec * 1000u + (uint64_t)now.tv_nsec / 1000000u;
    return 0;
}

static void send_response_chunks(int fd, const webui_http_chunk_t *chunks,
                                 size_t chunk_count) {
    int result = webui_http_write_chunks(&fd, webui_socket_send, webui_monotonic_ms,
                                         WEBUI_RESPONSE_TIMEOUT_MS, chunks, chunk_count);
    if (result != WEBUI_HTTP_WRITE_OK)
        LOG("webui: incomplete response (%d); closing client\n", result);
}

static void handle(int fd) {
    static char req[REQ_MAX + 1];
    webui_http_request_t request;
    char *body = NULL;
    char *path;
    char id[SC2_ID_MAX + 1];
    webui_route_request_t route;
    int is_post;
    int read_result;

    read_result = webui_http_read_request(&fd, webui_socket_receive,
                                         webui_monotonic_ms,
                                         WEBUI_REQUEST_TIMEOUT_MS,
                                         req, sizeof(req), &request, &body);
    if (read_result != WEBUI_HTTP_READ_OK) {
        if (read_result == WEBUI_HTTP_READ_TIMEOUT)
            REPLY_TXT(fd, 408, "request timed out");
        else if (read_result == WEBUI_HTTP_READ_TOO_LARGE)
            REPLY_TXT(fd, 413, "request too large");
        else if (read_result == WEBUI_HTTP_READ_MALFORMED)
            REPLY_TXT(fd, 400, "malformed request");
        else
            REPLY_TXT(fd, 400, "incomplete request");
        return;
    }
    if (!webui_http_origin_matches_host(&request)) {
        REPLY_TXT(fd, 403, "request origin does not match host");
        return;
    }
    path = request.target;
    is_post = strcmp(request.method, "POST") == 0;
    if (!webui_auth_path_allows(path, request.authorization, g_webui_token)) {
        REPLY_TXT(fd, 401, "portal token required");
        return;
    }
    if (webui_route_parse(path, &route) != 0) {
        REPLY_TXT(fd, 400, "malformed route");
        return;
    }
    if (route.id == WEBUI_ROUTE_NOT_FOUND) {
        REPLY_TXT(fd, 404, "not found");
        return;
    }
    if (!webui_route_method_allowed(route.id, is_post)) {
        REPLY_TXT(fd, 405, "method not allowed");
        return;
    }
    if (!is_post && strchr(path, '?') &&
        (route.id == WEBUI_ROUTE_GAMEHOOKS || route.id == WEBUI_ROUTE_DUALSENSE)) {
        REPLY_TXT(fd, 400, "mutation parameters require POST");
        return;
    }

    if (route.id == WEBUI_ROUTE_INDEX) {
        reply(fd, 200, "text/html; charset=utf-8", (const char *)webui_html, webui_html_len);
        return;
    }
    if (route.id == WEBUI_ROUTE_STATUS) {
        sc2_status_snapshot_t runtime;
        sc2_status_snapshot(&runtime);
        char t[SC2_ID_MAX+1], a[SC2_ID_MAX+1], ov[SC2_ID_MAX+1];
        char tn[GC_GAME_NAME_MAX], pn[48];
        sc2_select_status(t, a, ov);
        sc2_select_title_name(tn, sizeof(tn));
        sc2_select_active_name(pn, sizeof(pn));
        static char out[1024]; sb_t sb; webui_json_writer_init(&sb, out, sizeof(out));
        sb_put(&sb, "{\"title\":"); json_str(&sb, t);
        sb_put(&sb, ",\"title_name\":"); json_str(&sb, tn);
        sb_put(&sb, ",\"active\":"); json_str(&sb, a);
        sb_put(&sb, ",\"active_name\":"); json_str(&sb, pn);
        sb_put(&sb, ",\"override\":"); json_str(&sb, ov);
        sb_printf(&sb, ",\"inputs\":%u,\"connected\":%d,\"menu\":%d,\"haptics\":%d,\"paused\":%d",
                  (unsigned)runtime.live_inputs, runtime.live_connected, sc2_menu_is_open(), runtime.haptic_available, sc2_pause_is_active());
        sb_printf(&sb, ",\"battery\":%d,\"charging\":%d",
                  runtime.battery_level, runtime.battery_state == SC2_BATT_CHARGING ? 1 : runtime.battery_state == SC2_BATT_FULL ? 2 : 0);
        sb_printf(&sb, ",\"hap\":{\"q\":%u,\"sent\":%u,\"fail\":%u,\"err\":%d,\"via\":%d,\"out\":%d}",
                  runtime.haptic_queued, runtime.haptic_sent, runtime.haptic_failed, runtime.haptic_last_err, runtime.haptic_last_via, runtime.haptic_out_ep);
        sc2_gyro_status_t gs; sc2_gyro_status(&gs);
        sb_printf(&sb, ",\"gyro\":{\"wanted\":%d,\"imu\":%d,\"active\":%d,\"cal\":%d,\"grips\":%d,\"button\":%d,"
                  "\"yaw\":%.1f,\"pitch\":%.1f,\"roll\":%.1f}}",
                  gs.wanted, gs.imu, gs.active, gs.calibrated, gs.grips, gs.button,
                  (double)gs.yaw, (double)gs.pitch, (double)gs.roll);
        if (sb.failed) REPLY_TXT(fd, 500, "status response overflow");
        else reply(fd, 200, "application/json", out, sb.length);
        return;
    }
    if (route.id == WEBUI_ROUTE_ICON) {
        char iid[SC2_ID_MAX + 1], ip[256];
        if (get_id(&route, iid) != 1 || gc_game_icon_path(iid, ip, sizeof(ip)) != 0) {
            REPLY_TXT(fd, 404, "no icon"); return;
        }
        static char img[2 * 1024 * 1024];
        FILE *f = fopen(ip, "rb");
        if (!f) { REPLY_TXT(fd, 404, "no icon"); return; }
        size_t n = fread(img, 1, sizeof(img), f);
        fclose(f);
        char h[256];
        int hl = snprintf(h, sizeof(h),
            "HTTP/1.1 200 OK\r\nContent-Type: image/png\r\nContent-Length: %zu\r\n"
            "Cache-Control: max-age=86400\r\nConnection: close\r\n\r\n", n);
        if (hl < 0 || (size_t)hl >= sizeof(h)) return;
        webui_http_chunk_t chunks[] = {{h, (size_t)hl}, {img, n}};
        send_response_chunks(fd, chunks, sizeof(chunks) / sizeof(chunks[0]));
        return;
    }
    if (route.id == WEBUI_ROUTE_HAPTIC) {
        int haptics_available = sc2_status_haptics_available();
        char via[8], kind[16];
        int has_via = webui_route_param(&route, "via", via, sizeof(via));
        int has_kind = webui_route_param(&route, "kind", kind, sizeof(kind));
        LOG("haptics: test requested %s (available=%d)\n", path, haptics_available);
        if (has_via < 0 || has_kind != 1 ||
            (has_via == 1 && (strlen(via) != 1 || via[0] < '0' || via[0] > '4')) ||
            (strcmp(kind, "left") && strcmp(kind, "right") && strcmp(kind, "rumble"))) {
            REPLY_TXT(fd, 400, "invalid haptic parameters"); return;
        }
        if (!haptics_available) { REPLY_TXT(fd, 400, "Steam Controller isn't connected to the payload"); return; }
        if (has_via == 1) sc2_status_set_haptic_method(via[0] - '0');
        if      (has_kind == 1 && !strcmp(kind, "left"))   sc2_haptic_pulse(SC2_PAD_LEFT,  0x1F4, 0x1F4, 200);
        else if (has_kind == 1 && !strcmp(kind, "right"))  sc2_haptic_pulse(SC2_PAD_RIGHT, 0x1F4, 0x1F4, 200);
        else if (has_kind == 1 && !strcmp(kind, "rumble")) sc2_haptic_rumble_for(0x9000, 0x6000, 700);
        REPLY_TXT(fd, 200, "queued");
        return;
    }
    if (route.id == WEBUI_ROUTE_PROFILES) {
        static char out[8192]; sb_t s; webui_json_writer_init(&s, out, sizeof(out));
        sb_put(&s, "[");
        sc2_store_list(list_cb, &s);
        sb_put(&s, "]");
        if (s.failed) REPLY_TXT(fd, 500, "profiles response overflow");
        else reply(fd, 200, "application/json", out, s.length);
        return;
    }
    if (route.id == WEBUI_ROUTE_PROFILE) {
        if (get_id(&route, id) != 1) { REPLY_TXT(fd, 400, "bad id"); return; }
        sc2_profile_t p;
        if (is_post) {
            if (!webui_http_profile_body_valid(&request, body)) {
                REPLY_TXT(fd, 400, "profile requires a non-empty text/plain UTF-8 body");
                return;
            }
            int loaded = sc2_store_load(id, &p);
            if (loaded == SC2_STORE_NOT_FOUND) sc2_profile_default(&p);
            else if (loaded != 0) { REPLY_TXT(fd, 400, "existing profile unreadable; repair or delete it explicitly"); return; }
            if (sc2_profile_parse_checked(body, &p, 0) != 0) { REPLY_TXT(fd, 400, "invalid profile"); return; }
            if (sc2_store_save(id, &p) != 0) { REPLY_TXT(fd, 400, "save failed"); return; }
            sc2_select_reload();
            REPLY_TXT(fd, 200, "saved");
        } else {
            if (sc2_store_load(id, &p) != 0) { REPLY_TXT(fd, 404, "no such profile"); return; }
            static char out[SC2_PROFILE_CAPACITY]; int n = sc2_profile_format(&p, out, sizeof(out));
            if (n < 0 || (size_t)n >= sizeof(out)) REPLY_TXT(fd, 500, "profile response overflow");
            else reply(fd, 200, "text/plain", out, (size_t)n);
        }
        return;
    }
    if (route.id == WEBUI_ROUTE_DELETE) {
        if (get_id(&route, id) != 1 || sc2_store_delete(id) != 0) { REPLY_TXT(fd, 400, "cannot delete"); return; }
        sc2_select_reload();
        REPLY_TXT(fd, 200, "deleted");
        return;
    }
    if (route.id == WEBUI_ROUTE_OVERRIDE) {
        if (get_id(&route, id) < 0) { REPLY_TXT(fd, 400, "bad id"); return; }
        sc2_select_override(id);
        REPLY_TXT(fd, 200, "ok");
        return;
    }
    if (route.id == WEBUI_ROUTE_GAMES) {
        static gc_game_t games[256];
        int n = gc_game_list(games, 256);
        static char out[64 * 1024]; sb_t s; webui_json_writer_init(&s, out, sizeof(out));
        sb_put(&s, "[");
        for (int i = 0; i < n; i++) {
            if (i) sb_put(&s, ",");
            sb_put(&s, "{\"id\":"); json_str(&s, games[i].id);
            sb_put(&s, ",\"name\":"); json_str(&s, games[i].name);
            sb_put(&s, games[i].has_icon ? ",\"icon\":true" : ",\"icon\":false");
            sb_put(&s, "}");
        }
        sb_put(&s, "]");
        if (s.failed) REPLY_TXT(fd, 500, "games response overflow");
        else reply(fd, 200, "application/json", out, s.length);
        return;
    }
    if (route.id == WEBUI_ROUTE_GAMEHOOKS) {
        if (is_post) {
            char input[8], vibration[8], audio[8], mode[8];
            int hi = webui_route_param(&route, "input", input, sizeof(input));
            int hv = webui_route_param(&route, "vibration", vibration, sizeof(vibration));
            int ha = webui_route_param(&route, "audio", audio, sizeof(audio));
            int hm = webui_route_param(&route, "mode", mode, sizeof(mode));
            int i = -1, v = -1, a = -1, m = -1;
            if (hi < 0 || hv < 0 || ha < 0 || hm < 0 ||
                (hi == 1 && strcmp(input, "0") && strcmp(input, "1")) ||
                (hv == 1 && strcmp(vibration, "0") && strcmp(vibration, "1")) ||
                (ha == 1 && strcmp(audio, "0") && strcmp(audio, "1")) ||
                (hm == 1 && (strlen(mode) != 1 || mode[0] < '0' || mode[0] > '2'))) {
                REPLY_TXT(fd, 400, "invalid game-hook parameters"); return;
            }
            if (hi == 1) i = input[0] - '0';
            if (hv == 1) v = vibration[0] - '0';
            if (ha == 1) a = audio[0] - '0';
            if (hm == 1) m = mode[0] - '0';
            if (ha == 1) game_hooks_set_audio(a);
            game_hooks_set(i, v, m);
        }
        char out[512]; int n = game_hooks_json(out, sizeof(out));
        if (n < 0 || (size_t)n >= sizeof(out)) REPLY_TXT(fd, 500, "game hook response overflow");
        else reply(fd, 200, "application/json", out, (size_t)n);
        return;
    }
    if (route.id == WEBUI_ROUTE_PAUSE) {
        char on[8];
        if (webui_route_param(&route, "on", on, sizeof(on)) != 1 ||
            (strcmp(on, "0") && strcmp(on, "1"))) {
            REPLY_TXT(fd, 400, "invalid on parameter"); return;
        }
        sc2_set_paused(on[0] == '1');
        REPLY_TXT(fd, 200, sc2_pause_is_active() ? "paused" : "running");
        return;
    }
    if (route.id == WEBUI_ROUTE_DUALSENSE) {
        if (is_post) {
            char on[8];
            if (webui_route_param(&route, "on", on, sizeof(on)) != 1 ||
                (strcmp(on, "0") && strcmp(on, "1"))) {
                REPLY_TXT(fd, 400, "invalid on parameter"); return;
            }
            if (ds_handoff_set_enabled(on[0] == '1') != 0) {
                REPLY_TXT(fd, 403, "DualSense hand-off is disabled by runtime policy");
                return;
            }
        }
        char out[160]; int n = ds_handoff_json(out, sizeof(out));
        if (n < 0 || (size_t)n >= sizeof(out)) REPLY_TXT(fd, 500, "DualSense response overflow");
        else reply(fd, 200, "application/json", out, (size_t)n);
        return;
    }
    if (route.id == WEBUI_ROUTE_BRIDGE) {
        if (is_post) bridge_probe_request();
        char out[768]; int n = bridge_probe_json(out, sizeof(out));
        if (n < 0 || (size_t)n >= sizeof(out)) REPLY_TXT(fd, 500, "bridge response overflow");
        else reply(fd, 200, "application/json", out, (size_t)n);
        return;
    }
    if (route.id == WEBUI_ROUTE_LOG) {
        static char out[48 * 1024];
        size_t n = 0;
        char previous[8];
        int has_previous = webui_route_param(&route, "prev", previous, sizeof(previous));
        if (has_previous < 0 ||
            (has_previous == 1 && strcmp(previous, "0") && strcmp(previous, "1"))) {
            REPLY_TXT(fd, 400, "invalid prev parameter"); return;
        }
        FILE *f = fopen(has_previous == 1 && !strcmp(previous, "1") ?
                        "/data/ghostpad/gc_status.log.prev" :
                        "/data/ghostpad/gc_status.log", "r");
        if (f) {
            fseek(f, 0, SEEK_END);
            long sz = ftell(f);
            if (sz > (long)sizeof(out) - 1) fseek(f, sz - (long)(sizeof(out) - 1), SEEK_SET);
            else fseek(f, 0, SEEK_SET);
            n = fread(out, 1, sizeof(out) - 1, f);
            fclose(f);
        }
        if (!n) { REPLY_TXT(fd, 200, "Log is empty."); return; }
        reply(fd, 200, "text/plain; charset=utf-8", out, n);
        return;
    }
    REPLY_TXT(fd, 404, "not found");
}

static void *server_thread(void *arg) {
    (void)arg;
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) { LOG("webui: socket fail %d\n", errno); return NULL; }
    int one = 1; setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    struct sockaddr_in a; memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET; a.sin_port = htons(WEBUI_PORT); a.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s, (struct sockaddr *)&a, sizeof(a)) != 0 || listen(s, 8) != 0) {
        LOG("webui: bind/listen :%d fail errno=%d\n", WEBUI_PORT, errno);
        close(s); return NULL;
    }
    LOG("webui: listening on :%d\n", WEBUI_PORT);
    while (!runtime_is_stopping()) {
        if (s >= FD_SETSIZE) {
            LOG("webui: listener descriptor exceeds select capacity\n");
            break;
        }
        fd_set readable;
        FD_ZERO(&readable);
        FD_SET(s, &readable);
        struct timeval wait = { 0, 200000 };
        int ready = select(s + 1, &readable, NULL, NULL, &wait);
        if (ready == 0) continue;
        if (ready < 0) {
            if (errno == EINTR) continue;
            LOG("webui: select failed errno=%d\n", errno);
            break;
        }
        if (runtime_is_stopping()) break;
        struct sockaddr_in peer;
        socklen_t peer_length = sizeof(peer);
        int c = accept(s, (struct sockaddr *)&peer, &peer_length);
        if (c < 0) {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) continue;
            LOG("webui: accept failed errno=%d\n", errno);
            continue;
        }
        if (peer_length < sizeof(peer) || peer.sin_family != AF_INET ||
            !webui_peer_ipv4_is_lan(peer.sin_addr.s_addr)) {
            LOG("webui: rejected peer outside local IPv4 ranges\n");
            close(c);
            continue;
        }
#ifdef SO_NOSIGPIPE
        int no_sigpipe = 1;
        if (setsockopt(c, SOL_SOCKET, SO_NOSIGPIPE, &no_sigpipe, sizeof(no_sigpipe)) != 0) {
            LOG("webui: cannot protect client socket from SIGPIPE\n");
            close(c); continue;
        }
#endif
        struct timeval tv = { 2, 0 };
        setsockopt(c, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        setsockopt(c, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
        if (!runtime_is_stopping()) handle(c);
        close(c);
    }
    close(s);
    return NULL;
}

void webui_start(void) {
    if (initialize_webui_auth() != 0) {
        LOG("webui: disabled; cannot create/read a private token at %s\n", WEBUI_TOKEN_PATH);
        return;
    }
    LOG("webui: private bearer token ready; retrieve %s through local console file access\n",
        WEBUI_TOKEN_PATH);
    int r = pthread_create(&g_webui_thread, NULL, server_thread, NULL);
    if (r == 0) g_webui_thread_started = 1;
    else LOG("webui: failed to start server thread (%d)\n", r);
}

int webui_stop(void) {
    if (!g_webui_thread_started) return 0;
    int result = pthread_join(g_webui_thread, NULL);
    if (result != 0) return -1;
    g_webui_thread_started = 0;
    return 0;
}
