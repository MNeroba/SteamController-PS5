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
#include "webui_json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <netinet/in.h>

#ifdef __PROSPERO__
void ghostpad_status_log(const char *fmt, ...);   /* gc_main.c: klog + /data/ghostpad/gc_status.log */
#define LOG(...) ghostpad_status_log("[GC] " __VA_ARGS__)
#else
#define LOG(...) fprintf(stderr, __VA_ARGS__)
#endif

#define REQ_MAX WEBUI_HTTP_BODY_MAX
#define WEBUI_TOKEN_PATH "/data/ghostpad/webui.token"
#define WEBUI_RESPONSE_TIMEOUT_MS 5000u

static char g_webui_token[WEBUI_TOKEN_HEX_LENGTH + 1];

static void send_response_chunks(int fd, const webui_http_chunk_t *chunks,
                                 size_t chunk_count);

static void reply(int fd, int code, const char *type, const char *body, size_t n) {
    char h[256];
    const char *reason = code == 200 ? "OK" : code == 401 ? "Unauthorized" :
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
static int get_id(const char *path, char *out) {
    out[0] = 0;
    const char *q = strstr(path, "id=");
    if (!q) return 0;
    q += 3;
    size_t n = strcspn(q, "& ");
    if (n > SC2_ID_MAX) return -1;
    memcpy(out, q, n); out[n] = 0;
    if (n == 0) return 0;
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
    return send(fd, buffer, length, 0);
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
    (void)webui_http_write_chunks(&fd, webui_socket_send, webui_monotonic_ms,
                                  WEBUI_RESPONSE_TIMEOUT_MS, chunks, chunk_count);
}

static void handle(int fd) {
    static char req[REQ_MAX + 1];
    webui_http_request_t request;
    char *body = NULL;
    char *path;
    char id[SC2_ID_MAX + 1];
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
    path = request.target;
    is_post = strcmp(request.method, "POST") == 0;
    if (!webui_auth_path_allows(path, request.authorization, g_webui_token)) {
        REPLY_TXT(fd, 401, "portal token required");
        return;
    }

    if (!strcmp(path, "/") || !strncmp(path, "/index", 6)) {
        reply(fd, 200, "text/html; charset=utf-8", (const char *)webui_html, webui_html_len);
        return;
    }
    if (!strncmp(path, "/api/status", 11)) {
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
    if (!strncmp(path, "/api/icon", 9)) {
        char iid[SC2_ID_MAX + 1], ip[256];
        if (get_id(path, iid) != 1 || gc_game_icon_path(iid, ip, sizeof(ip)) != 0) {
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
    if (is_post && !strncmp(path, "/api/haptic", 11)) {
        int haptics_available = sc2_status_haptics_available();
        LOG("haptics: test requested %s (available=%d)\n", path, haptics_available);
        if (!haptics_available) { REPLY_TXT(fd, 400, "Steam Controller isn't connected to the payload"); return; }
        { const char *m = strstr(path, "via="); if (m && m[4] >= '0' && m[4] <= '4') sc2_status_set_haptic_method(m[4] - '0'); }
        if      (strstr(path, "kind=left"))   sc2_haptic_pulse(SC2_PAD_LEFT,  0x1F4, 0x1F4, 200);
        else if (strstr(path, "kind=right"))  sc2_haptic_pulse(SC2_PAD_RIGHT, 0x1F4, 0x1F4, 200);
        else if (strstr(path, "kind=rumble")) sc2_haptic_rumble_for(0x9000, 0x6000, 700);
        else                                  sc2_haptic_tick();
        REPLY_TXT(fd, 200, "queued");
        return;
    }
    if (!strncmp(path, "/api/profiles", 13)) {
        static char out[8192]; sb_t s; webui_json_writer_init(&s, out, sizeof(out));
        sb_put(&s, "[");
        sc2_store_list(list_cb, &s);
        sb_put(&s, "]");
        if (s.failed) REPLY_TXT(fd, 500, "profiles response overflow");
        else reply(fd, 200, "application/json", out, s.length);
        return;
    }
    if (!strncmp(path, "/api/profile", 12)) {
        if (get_id(path, id) != 1) { REPLY_TXT(fd, 400, "bad id"); return; }
        sc2_profile_t p;
        if (is_post) {
            if (sc2_store_load(id, &p) != 0) sc2_profile_default(&p);
            sc2_profile_parse(body, &p);
            if (sc2_store_save(id, &p) != 0) { REPLY_TXT(fd, 400, "save failed"); return; }
            sc2_select_reload();
            REPLY_TXT(fd, 200, "saved");
        } else {
            if (sc2_store_load(id, &p) != 0) { REPLY_TXT(fd, 404, "no such profile"); return; }
            static char out[8192]; int n = sc2_profile_format(&p, out, sizeof(out));
            if (n < 0 || (size_t)n >= sizeof(out) - 1) REPLY_TXT(fd, 500, "profile response overflow");
            else reply(fd, 200, "text/plain", out, (size_t)n);
        }
        return;
    }
    if (is_post && !strncmp(path, "/api/delete", 11)) {
        if (get_id(path, id) != 1 || sc2_store_delete(id) != 0) { REPLY_TXT(fd, 400, "cannot delete"); return; }
        sc2_select_reload();
        REPLY_TXT(fd, 200, "deleted");
        return;
    }
    if (is_post && !strncmp(path, "/api/override", 13)) {
        if (get_id(path, id) < 0) { REPLY_TXT(fd, 400, "bad id"); return; }
        sc2_select_override(id);
        REPLY_TXT(fd, 200, "ok");
        return;
    }
    if (!strncmp(path, "/api/games", 10)) {
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
    if (!strncmp(path, "/api/gamehooks", 14)) {
        if (is_post) {
            const char *i = strstr(path, "input="), *v = strstr(path, "vibration=");
            const char *au = strstr(path, "audio=");
            if (au) game_hooks_set_audio(au[6] == '1');
            const char *m = strstr(path, "mode=");
            game_hooks_set(i ? i[6] == '1' : -1, v ? v[10] == '1' : -1, m ? m[5] - '0' : -1);
        }
        char out[512]; int n = game_hooks_json(out, sizeof(out));
        if (n < 0 || (size_t)n >= sizeof(out)) REPLY_TXT(fd, 500, "game hook response overflow");
        else reply(fd, 200, "application/json", out, (size_t)n);
        return;
    }
    if (is_post && !strncmp(path, "/api/pause", 10)) {
        const char *o = strstr(path, "on=");
        if (!o) { REPLY_TXT(fd, 400, "missing on="); return; }
        sc2_set_paused(o[3] == '1');
        REPLY_TXT(fd, 200, sc2_pause_is_active() ? "paused" : "running");
        return;
    }
    if (!strncmp(path, "/api/dualsense", 14)) {
        if (is_post) { const char *o = strstr(path, "on="); if (o) ds_handoff_set_enabled(o[3] == '1'); }
        char out[160]; int n = ds_handoff_json(out, sizeof(out));
        if (n < 0 || (size_t)n >= sizeof(out)) REPLY_TXT(fd, 500, "DualSense response overflow");
        else reply(fd, 200, "application/json", out, (size_t)n);
        return;
    }
    if (!strncmp(path, "/api/bridge", 11)) {
        if (is_post) bridge_probe_request();
        char out[768]; int n = bridge_probe_json(out, sizeof(out));
        if (n < 0 || (size_t)n >= sizeof(out)) REPLY_TXT(fd, 500, "bridge response overflow");
        else reply(fd, 200, "application/json", out, (size_t)n);
        return;
    }
    if (!strncmp(path, "/api/log", 8)) {
        static char out[48 * 1024];
        size_t n = 0;
        FILE *f = fopen(strstr(path, "prev=1") ? "/data/ghostpad/gc_status.log.prev" : "/data/ghostpad/gc_status.log", "r");
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
    for (;;) {
        int c = accept(s, NULL, NULL);
        if (c < 0) { usleep(100000); continue; }
#ifdef SO_NOSIGPIPE
        int no_sigpipe = 1;
        setsockopt(c, SOL_SOCKET, SO_NOSIGPIPE, &no_sigpipe, sizeof(no_sigpipe));
#endif
        struct timeval tv = { 2, 0 };
        setsockopt(c, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        setsockopt(c, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
        handle(c);
        close(c);
    }
    return NULL;
}

void webui_start(void) {
    pthread_t t;
    if (initialize_webui_auth() != 0) {
        LOG("webui: disabled; cannot create/read a private token at %s\n", WEBUI_TOKEN_PATH);
        return;
    }
    LOG("webui: private bearer token ready; retrieve %s through local console file access\n",
        WEBUI_TOKEN_PATH);
    int r = pthread_create(&t, NULL, server_thread, NULL);
    if (r == 0) pthread_detach(t);
    else LOG("webui: failed to start server thread (%d)\n", r);
}
