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
#include "webui_http.h"
#include "webui_route.h"
#include "webui_json.h"
#include "runtime_control.h"
#include "runtime_features.h"
#include "settings_tile.h"
#include "diagnostics.h"
#include "private_file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>

#ifdef __PROSPERO__
void ghostpad_status_log(const char *fmt, ...);   /* gc_main.c: klog + /data/ghostpad/gc_status.log */
#define LOG(...) ghostpad_status_log("[GC] " __VA_ARGS__)
#else
#define LOG(...) fprintf(stderr, __VA_ARGS__)
#endif

#define REQ_MAX (WEBUI_HTTP_HEADER_MAX + WEBUI_HTTP_BODY_MAX)
#define WEBUI_RESPONSE_TIMEOUT_MS 5000u

static pthread_t g_webui_thread;
static int g_webui_thread_started;
static pthread_t g_tile_thread;
static int g_tile_thread_started;
static pthread_mutex_t g_tile_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_tile_cond = PTHREAD_COND_INITIALIZER;
static int g_tile_done;
static webui_notify_fn g_tile_notify;
static int g_listener = -1;
static atomic_int g_webui_stopping;
#define WEBUI_CLIENT_MAX 4
typedef struct { pthread_t thread; int fd; int started; int done; } webui_client_t;
static webui_client_t g_clients[WEBUI_CLIENT_MAX];
static pthread_mutex_t g_clients_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_clients_cond = PTHREAD_COND_INITIALIZER;
/* Actual bound port, set once after a successful bind. Zero before that and
 * in unit fixtures that never bind. */
static unsigned g_listener_port;
/* Preserve the existing serialized API dispatch contract. Network framing
 * and admission are independent, so a slow body cannot hold
 * this lock. Menu/profile store concurrency is handled by the shared store. */
static pthread_mutex_t g_dispatch_lock = PTHREAD_MUTEX_INITIALIZER;
static int webui_stopping(void) {
    return atomic_load(&g_webui_stopping) || runtime_is_stopping();
}

static void send_response_chunks(int fd, const webui_http_chunk_t *chunks,
                                 size_t chunk_count);

static void reply(int fd, int code, const char *type, const char *body, size_t n) {
    char h[512];
    const char *reason = code == 200 ? "OK" : code == 401 ? "Unauthorized" :
                         code == 403 ? "Forbidden" :
                         code == 404 ? "Not Found" : code == 405 ? "Method Not Allowed" :
                         code == 413 ? "Payload Too Large" :
                         code == 408 ? "Request Timeout" :
                         code == 500 ? "Internal Server Error" : "Bad Request";
    int hl = snprintf(h, sizeof(h),
        "HTTP/1.1 %d %s\r\nContent-Type: %s\r\nContent-Length: %zu\r\n"
        "Cache-Control: no-store\r\nX-Content-Type-Options: nosniff\r\n"
        "X-Frame-Options: DENY\r\nContent-Security-Policy: frame-ancestors 'none'\r\n"
        "%sConnection: close\r\n\r\n",
        code, reason, type, n,
        code == 401 ? "WWW-Authenticate: Bearer realm=\"SteamController-PS5\"\r\n" : "");
    if (hl < 0 || (size_t)hl >= sizeof(h)) return;
    webui_http_chunk_t chunks[] = {{h, (size_t)hl}, {body, n}};
    send_response_chunks(fd, chunks, sizeof(chunks) / sizeof(chunks[0]));
}
#define REPLY_TXT(fd, code, s) reply(fd, code, "text/plain", s, strlen(s))

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
    if (webui_stopping()) { errno = ECANCELED; return -1; }
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
    if (webui_stopping()) { errno = ECANCELED; return -1; }
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

/* Only literal loopback origins are accepted: a DNS-rebinding hostname must
 * not be able to use this unauthenticated, console-local management API. */
static int loopback_host(const char *host) {
    size_t n=0;
    if (!host) return 0;
    if (!strncmp(host,"localhost",9)) n=9;
    else if (!strncmp(host,"127.0.0.1",9)) n=9;
    else return 0;
    if (!host[n]) return 1;
    if (host[n++] != ':' || !host[n]) return 0;
    unsigned port=0;
    for (;host[n];n++) {
        if (host[n]<'0' || host[n]>'9') return 0;
        port=port*10u+(unsigned)(host[n]-'0');
        if (port>65535) return 0;
    }
    /* A literal loopback host on any other port is not this listener: reject
     * it so a different local service cannot borrow our Host value. */
    return g_listener_port && port==g_listener_port;
}
static int admit_request(void *context, const webui_http_request_t *request) {
    (void)context;
    if (!request->has_host || !loopback_host(request->host) ||
        !webui_http_origin_matches_host(request) ||
        (!strcmp(request->method,"POST") && !request->has_origin))
        return WEBUI_HTTP_READ_FORBIDDEN;
    webui_route_request_t route;
    if (webui_route_parse(request->target, &route) != 0) return WEBUI_HTTP_READ_MALFORMED;
    return WEBUI_HTTP_READ_OK;
}

static int patch_profile(sc2_profile_t *p, void *text) {
    return sc2_profile_parse_checked(text,p,0);
}
static void dispatch(int fd, const webui_http_request_t *request_ptr, char *body,
                     webui_route_request_t route) {
    webui_http_request_t request = *request_ptr;
    char *path = request.target;
    char id[SC2_ID_MAX + 1];
    int is_post = strcmp(request.method, "POST") == 0;
    if (route.id == WEBUI_ROUTE_NOT_FOUND) {
        REPLY_TXT(fd, 404, "not found"); return;
    }
    if (!webui_route_method_allowed(route.id, is_post)) {
        REPLY_TXT(fd, 405, "method not allowed"); return;
    }
    if (!is_post && strchr(path, '?') &&
        (route.id == WEBUI_ROUTE_GAMEHOOKS || route.id == WEBUI_ROUTE_DUALSENSE ||
         route.id == WEBUI_ROUTE_BRIDGE)) {
        REPLY_TXT(fd, 400, "mutation parameters require POST"); return;
    }
    if (route.id == WEBUI_ROUTE_INDEX) {
        reply(fd, 200, "text/html; charset=utf-8", (const char *)webui_html, webui_html_len);
        return;
    }
    if (route.id == WEBUI_ROUTE_DIAGNOSTICS) {
        diagnostics_steam_t snapshot={0};
        sc2_status_snapshot(&snapshot.steam);snapshot.paused=sc2_pause_is_active();
        sc2_profile_t profile;sc2_active_get(&profile);
        sc2_select_active_name(snapshot.profile,sizeof(snapshot.profile));
        snapshot.left_trackpad_mode=profile.lpad;snapshot.right_trackpad_mode=profile.rpad;
        snapshot.shift=profile.shift_in;
        for(int i=0;i<SC2_IN_COUNT;i++)if(profile.map[i])snapshot.mappings++;
        sc2_gyro_status_t gyro;sc2_gyro_status(&gyro);
        snapshot.gyro_wanted=gyro.wanted;snapshot.gyro_imu=gyro.imu;
        snapshot.gyro_active=gyro.active;snapshot.gyro_calibrated=gyro.calibrated;
        char *report=malloc(DIAGNOSTICS_REPORT_MAX);
        if(!report){REPLY_TXT(fd,500,"Cannot allocate diagnostic report");return;}
        int length=diagnostics_report(report,DIAGNOSTICS_REPORT_MAX,&snapshot);
        if(length<0){free(report);REPLY_TXT(fd,500,"Diagnostic report limit exceeded");return;}
        char header[384];int n=snprintf(header,sizeof(header),
            "HTTP/1.1 200 OK\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: %d\r\n"
            "Content-Disposition: attachment; filename=\"%s\"\r\nCache-Control: no-store\r\n"
            "X-Content-Type-Options: nosniff\r\nConnection: close\r\n\r\n",length,DIAGNOSTICS_FILENAME);
        if(n>0&&(size_t)n<sizeof(header)) {
            webui_http_chunk_t chunks[]={{header,(size_t)n},{report,(size_t)length}};
            send_response_chunks(fd,chunks,2);
        }
        free(report);return;
    }
    if (route.id == WEBUI_ROUTE_SETTINGS_TILE) {
        if (webui_stopping() || request.content_length != sizeof(SETTINGS_TILE_CONFIRM)-1 ||
            !webui_http_profile_body_valid(&request, body) ||
            memcmp(body, SETTINGS_TILE_CONFIRM, sizeof(SETTINGS_TILE_CONFIRM)-1)) {
            REPLY_TXT(fd, 400, "Explicit tile confirmation required; no installation performed"); return;
        }
        char message[256];
        int installed = settings_tile_create(message, sizeof(message));
        REPLY_TXT(fd, installed ? 500 : 200, message);
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
        char out[2048]; sb_t sb; webui_json_writer_init(&sb, out, sizeof(out));
        sb_put(&sb, "{\"title\":"); json_str(&sb, t);
        sb_put(&sb, ",\"title_name\":"); json_str(&sb, tn);
        sb_put(&sb, ",\"active\":"); json_str(&sb, a);
        sb_put(&sb, ",\"active_name\":"); json_str(&sb, pn);
        sb_put(&sb, ",\"override\":"); json_str(&sb, ov);
        sb_printf(&sb, ",\"inputs\":%u,\"connected\":%d,\"menu\":%d,\"haptics\":%d,\"paused\":%d",
                  (unsigned)runtime.live_inputs, runtime.live_connected, sc2_menu_is_open(), runtime.haptic_available, sc2_pause_is_active());
        sb_printf(&sb, ",\"battery\":%d,\"charging\":%d",
                  runtime.battery_level, runtime.battery_state == SC2_BATT_CHARGING ? 1 : runtime.battery_state == SC2_BATT_FULL ? 2 : 0);
        sb_printf(&sb, ",\"hap\":{\"q\":%u,\"sent\":%u,\"fail\":%u,\"drop\":%u,\"err\":%d,\"via\":%d,\"out\":%d}",
                  runtime.haptic_queued, runtime.haptic_sent, runtime.haptic_failed, runtime.haptic_dropped,
                  runtime.haptic_last_err, runtime.haptic_last_via, runtime.haptic_out_ep);
        sc2_gyro_status_t gs; sc2_gyro_status(&gs);
        sb_printf(&sb, ",\"gyro\":{\"wanted\":%d,\"imu\":%d,\"active\":%d,\"cal\":%d,\"grips\":%d,\"button\":%d,"
                  "\"yaw\":%.1f,\"pitch\":%.1f,\"roll\":%.1f}}",
                  gs.wanted, gs.imu, gs.active, gs.calibrated, gs.grips, gs.button,
                  isfinite(gs.yaw) ? (double)gs.yaw : 0.0,
                  isfinite(gs.pitch) ? (double)gs.pitch : 0.0,
                  isfinite(gs.roll) ? (double)gs.roll : 0.0);
        if (sb.failed) REPLY_TXT(fd, 500, "status response overflow");
        else reply(fd, 200, "application/json", out, sb.length);
        return;
    }
    if (route.id == WEBUI_ROUTE_ICON) {
        char iid[SC2_ID_MAX + 1], ip[256];
        if (get_id(&route, iid) != 1 || gc_game_icon_path(iid, ip, sizeof(ip)) != 0) {
            REPLY_TXT(fd, 404, "no icon"); return;
        }
        const size_t img_capacity = 2 * 1024 * 1024;
        char *img = malloc(img_capacity);
        if (!img) { REPLY_TXT(fd, 500, "icon allocation failed"); return; }
        int ifd = open(ip, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
        struct stat ist;
        if (ifd < 0 || fstat(ifd, &ist) != 0 || !S_ISREG(ist.st_mode)) {
            if (ifd >= 0) close(ifd);
            free(img); REPLY_TXT(fd, 404, "no icon"); return;
        }
        FILE *f = fdopen(ifd, "rb");
        if (!f) { close(ifd); free(img); REPLY_TXT(fd, 500, "icon unreadable"); return; }
        /* Refuse an oversized or unreadable icon instead of serving a
         * truncated PNG with a matching Content-Length. */
        long icon_size = -1;
        if (fseek(f, 0, SEEK_END) == 0) icon_size = ftell(f);
        if (icon_size <= 0 || (size_t)icon_size > img_capacity ||
            fseek(f, 0, SEEK_SET) != 0) {
            fclose(f); free(img); REPLY_TXT(fd, 500, "icon too large or unreadable"); return;
        }
        size_t n = fread(img, 1, (size_t)icon_size, f);
        int short_read = n != (size_t)icon_size || ferror(f);
        fclose(f);
        if (short_read) { free(img); REPLY_TXT(fd, 500, "icon too large or unreadable"); return; }
        char h[256];
        int hl = snprintf(h, sizeof(h),
            "HTTP/1.1 200 OK\r\nContent-Type: image/png\r\nContent-Length: %zu\r\n"
            "Cache-Control: max-age=86400\r\nX-Content-Type-Options: nosniff\r\nConnection: close\r\n\r\n", n);
        if (hl < 0 || (size_t)hl >= sizeof(h)) { free(img); return; }
        webui_http_chunk_t chunks[] = {{h, (size_t)hl}, {img, n}};
        send_response_chunks(fd, chunks, sizeof(chunks) / sizeof(chunks[0]));
        free(img);
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
        char out[8192]; sb_t s; webui_json_writer_init(&s, out, sizeof(out));
        sb_put(&s, "[");
        sc2_store_list(list_cb, &s);
        sb_put(&s, "]");
        if (s.failed) REPLY_TXT(fd, 500, "profiles response overflow");
        else reply(fd, 200, "application/json", out, s.length);
        return;
    }
    if (route.id == WEBUI_ROUTE_RECOVER_DEFAULT) {
        if (sc2_store_recover_default()!=0) { REPLY_TXT(fd,400,"Default recovery failed or profile is already valid");return; }
        sc2_select_reload();REPLY_TXT(fd,200,"Default backed up and reset");return;
    }
    if (route.id == WEBUI_ROUTE_PROFILE) {
        if (get_id(&route, id) != 1) { REPLY_TXT(fd, 400, "bad id"); return; }
        sc2_profile_t p;
        if (is_post) {
            if (!webui_http_profile_body_valid(&request, body)) {
                REPLY_TXT(fd, 400, "profile requires a non-empty text/plain UTF-8 body");
                return;
            }
            if (sc2_store_update(id,patch_profile,body) != 0) {
                REPLY_TXT(fd,400,"profile update failed; check storage or recover corrupt Default explicitly");return;
            }
            sc2_select_reload();
            REPLY_TXT(fd, 200, "saved");
        } else {
            int loaded = sc2_store_load(id, &p);
            if (loaded != 0) { REPLY_TXT(fd, loaded == SC2_STORE_NOT_FOUND ? 404 : 500, "profile unavailable or corrupt"); return; }
            char out[SC2_PROFILE_CAPACITY]; int n = sc2_profile_format(&p, out, sizeof(out));
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
        gc_game_t games[256];
        int n = gc_game_list(games, 256);
        char out[64 * 1024]; sb_t s; webui_json_writer_init(&s, out, sizeof(out));
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
            char input[8], vibration[8], audio[8], mode[8], hooks[8];
            int hi = webui_route_param(&route, "input", input, sizeof(input));
            int hv = webui_route_param(&route, "vibration", vibration, sizeof(vibration));
            int ha = webui_route_param(&route, "audio", audio, sizeof(audio));
            int hm = webui_route_param(&route, "mode", mode, sizeof(mode));
            int hh = webui_route_param(&route, "hooks", hooks, sizeof(hooks));
            int i = -1, v = -1, a = -1, m = -1;
            if (hi < 0 || hv < 0 || ha < 0 || hm < 0 || hh < 0 ||
                (hi == 1 && strcmp(input, "0") && strcmp(input, "1")) ||
                (hv == 1 && strcmp(vibration, "0") && strcmp(vibration, "1")) ||
                (ha == 1 && strcmp(audio, "0") && strcmp(audio, "1")) ||
                (hm == 1 && (strlen(mode) != 1 || mode[0] < '0' || mode[0] > '2')) ||
                (hh == 1 && strcmp(hooks, "0") && strcmp(hooks, "1"))) {
                REPLY_TXT(fd, 400, "invalid game-hook parameters"); return;
            }
            if (hh == 1) {
                char toggle_message[192];
                int toggle = game_hooks_set_runtime_enabled(hooks[0] == '1',
                                                            toggle_message,
                                                            sizeof(toggle_message));
                REPLY_TXT(fd, toggle ? 500 : 200, toggle_message);
                return;
            }
            if (hi == 1) i = input[0] - '0';
            if (hv == 1) v = vibration[0] - '0';
            if (ha == 1) a = audio[0] - '0';
            if (hm == 1) m = mode[0] - '0';
            if (game_hooks_configure(i, v, m, ha == 1 ? a : -1)) {
                REPLY_TXT(fd, 500, "settings persistence failed"); return;
            }
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
            char on[8], handoff[8];
            int has_on = webui_route_param(&route, "on", on, sizeof(on));
            int has_handoff = webui_route_param(&route, "handoff", handoff, sizeof(handoff));
            if (has_on < 0 || has_handoff < 0 ||
                (has_on == 1 && has_handoff == 1)) {
                REPLY_TXT(fd, 400, "invalid DualSense parameters"); return;
            }
            if (has_handoff == 1) {
                if (strcmp(handoff, "0") && strcmp(handoff, "1")) {
                    REPLY_TXT(fd, 400, "invalid handoff parameter"); return;
                }
                char message[192];
                int result = runtime_feature_set_handoff(handoff[0] == '1',
                                                         message, sizeof(message));
                REPLY_TXT(fd, result ? 500 : 200, message);
                return;
            }
            if (has_on != 1 || (strcmp(on, "0") && strcmp(on, "1"))) {
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
        if (is_post) {
            char enable[8];
            int has_enable = webui_route_param(&route, "enable", enable, sizeof(enable));
            if (has_enable < 0 ||
                (has_enable == 1 && strcmp(enable, "0") && strcmp(enable, "1"))) {
                REPLY_TXT(fd, 400, "invalid enable parameter"); return;
            }
            if (has_enable == 1) {
                char message[192];
                int result = runtime_feature_set_bridge(enable[0] == '1',
                                                        message, sizeof(message));
                REPLY_TXT(fd, result ? 500 : 200, message);
                return;
            }
            bridge_probe_request();
        }
        char out[1024]; int n = bridge_probe_json(out, sizeof(out));
        if (n < 0 || (size_t)n >= sizeof(out)) REPLY_TXT(fd, 500, "bridge response overflow");
        else reply(fd, 200, "application/json", out, (size_t)n);
        return;
    }
    if (route.id == WEBUI_ROUTE_LOG) {
        char out[48 * 1024];
        size_t n = 0;
        char previous[8];
        int has_previous = webui_route_param(&route, "prev", previous, sizeof(previous));
        if (has_previous < 0 ||
            (has_previous == 1 && strcmp(previous, "0") && strcmp(previous, "1"))) {
            REPLY_TXT(fd, 400, "invalid prev parameter"); return;
        }
        const char *log_name = has_previous == 1 && !strcmp(previous, "1") ?
                               "gc_status.log.prev" : "gc_status.log";
        FILE *f = NULL;
        int log_dir = private_directory_open("/data/ghostpad");
        if (log_dir >= 0) {
            int log_fd = private_regular_open(log_dir, log_name);
            close(log_dir);
            if (log_fd >= 0) f = fdopen(log_fd, "r");
            if (log_fd >= 0 && !f) close(log_fd);
        }
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

static void handle(int fd) {
    char req[REQ_MAX + 1];
    webui_http_request_t request;
    char *body = NULL;
    int result = webui_http_read_request_checked(&fd, webui_socket_receive,
        webui_monotonic_ms, WEBUI_REQUEST_TIMEOUT_MS, req, sizeof(req),
        &request, &body, admit_request, NULL);
    if (result != WEBUI_HTTP_READ_OK) {
        if (result == WEBUI_HTTP_READ_UNAUTHORIZED) REPLY_TXT(fd, 401, "local access required");
        else if (result == WEBUI_HTTP_READ_FORBIDDEN) REPLY_TXT(fd, 403, "request origin does not match host");
        else if (result == WEBUI_HTTP_READ_TIMEOUT) REPLY_TXT(fd, 408, "request timed out");
        else if (result == WEBUI_HTTP_READ_TOO_LARGE) REPLY_TXT(fd, 413, "request too large");
        else REPLY_TXT(fd, 400, "malformed or incomplete request");
        return;
    }
    webui_route_request_t route;
    if (webui_route_parse(request.target, &route) != 0) {
        REPLY_TXT(fd, 400, "malformed route"); return;
    }
    if (route.id == WEBUI_ROUTE_INDEX) { dispatch(fd, &request, body, route); return; }
    pthread_mutex_lock(&g_dispatch_lock);
    if (!webui_stopping()) dispatch(fd, &request, body, route);
    pthread_mutex_unlock(&g_dispatch_lock);
}

static void *client_thread(void *arg) {
    webui_client_t *client = arg;
    int fd = client->fd;
    if (!webui_stopping()) handle(fd);
    pthread_mutex_lock(&g_clients_lock);
    close(fd);
    client->fd = -1;
    client->done = 1;
    pthread_cond_broadcast(&g_clients_cond);
    pthread_mutex_unlock(&g_clients_lock);
    return NULL;
}

/* Wait for the worker's own completion signal before joining. A client that
 * blocks inside the installer/hook service cannot hold shutdown forever; the
 * process exit is the final boundary for a worker that is still running. */
static int join_client_bounded(webui_client_t *client, unsigned wait_ms) {
    struct timespec deadline;
    if (clock_gettime(CLOCK_REALTIME, &deadline) != 0) return -1;
    deadline.tv_sec += (time_t)(wait_ms / 1000u);
    deadline.tv_nsec += (long)(wait_ms % 1000u) * 1000000L;
    if (deadline.tv_nsec >= 1000000000L) {
        deadline.tv_sec++;
        deadline.tv_nsec -= 1000000000L;
    }
    pthread_mutex_lock(&g_clients_lock);
    while (client->started && !client->done) {
        if (pthread_cond_timedwait(&g_clients_cond, &g_clients_lock,
                                   &deadline) == ETIMEDOUT)
            break;
    }
    int done = client->done;
    pthread_mutex_unlock(&g_clients_lock);
    if (!done) {
        LOG("webui: worker still busy at shutdown; process exit will reclaim it\n");
        return 0;
    }
    if (pthread_join(client->thread, NULL) != 0) return -1;
    client->started = 0;
    return 0;
}

static void *server_thread(void *arg) {
    (void)arg;
    int s = g_listener;
    while (!webui_stopping()) {
        /* Join completed workers before reusing any slot; no detached worker
         * can outlive service shutdown. */
        for (int i = 0; i < WEBUI_CLIENT_MAX; ++i) {
            pthread_mutex_lock(&g_clients_lock);
            int done = g_clients[i].started && g_clients[i].done;
            pthread_mutex_unlock(&g_clients_lock);
            if (done && pthread_join(g_clients[i].thread, NULL) == 0)
                g_clients[i].started = 0;
        }
        fd_set readable;
        FD_ZERO(&readable); FD_SET(s, &readable);
        struct timeval wait = {0, 200000};
        int ready = select(s + 1, &readable, NULL, NULL, &wait);
        if (ready == 0) continue;
        if (ready < 0) {
            if (errno == EINTR) continue;
            LOG("webui: select failed errno=%d\n", errno); break;
        }
        if (webui_stopping()) break;
        struct sockaddr_in peer;
        socklen_t peer_length = sizeof(peer);
        int c = accept(s, (struct sockaddr *)&peer, &peer_length);
        if (c < 0) {
            if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK)
                LOG("webui: accept failed errno=%d\n", errno);
            continue;
        }
        if (peer_length < sizeof(peer) || peer.sin_family != AF_INET ||
            (ntohl(peer.sin_addr.s_addr) >> 24) != 127u) { close(c); continue; }
#ifdef SO_NOSIGPIPE
        int one = 1;
        if (setsockopt(c, SOL_SOCKET, SO_NOSIGPIPE, &one, sizeof(one)) != 0) {
            close(c); continue;
        }
#endif
        int slot = 0;
        while (slot < WEBUI_CLIENT_MAX && g_clients[slot].started) ++slot;
        if (slot == WEBUI_CLIENT_MAX) { close(c); continue; }
        webui_client_t *client = &g_clients[slot];
        client->fd = c; client->done = 0;
        /* Local patch: dispatch() alone needs ~105 KiB of stack (plus ~38 KiB
         * in handle); the PS5 default thread stack is too small and the first
         * HTTP request crashed the payload. Use an explicit 1 MiB stack. */
        pthread_attr_t attr;
        int r = pthread_attr_init(&attr);
        if (r == 0) {
            r = pthread_attr_setstacksize(&attr, 1024u * 1024u);
            if (r == 0) r = pthread_create(&client->thread, &attr, client_thread, client);
            pthread_attr_destroy(&attr);
        }
        if (r != 0) { close(c); client->fd = -1; LOG("webui: client thread failed (%d)\n", r); }
        else client->started = 1;
    }
    atomic_store(&g_webui_stopping, 1);
    pthread_mutex_lock(&g_clients_lock);
    for (int i = 0; i < WEBUI_CLIENT_MAX; ++i)
        if (g_clients[i].started && g_clients[i].fd >= 0) shutdown(g_clients[i].fd, SHUT_RDWR);
    pthread_mutex_unlock(&g_clients_lock);
    for (int i = 0; i < WEBUI_CLIENT_MAX; ++i)
        if (g_clients[i].started) (void)join_client_bounded(&g_clients[i], 2000);
    close(s);
    return NULL;
}

/* Automatic first-run registration is isolated from controller startup. A
 * title directory alone is not proof of registration: skip only when the
 * post-success marker and exact embedded assets match. Unknown state or a
 * stale/unregistered directory is retried; foreign paths fail closed. */
static void *automatic_tile_impl(void *unused) {
    (void)unused;
    char message[256];
    pthread_mutex_lock(&g_dispatch_lock);
    if (webui_stopping()) { pthread_mutex_unlock(&g_dispatch_lock); return NULL; }
    int registered=settings_tile_is_registered();
    if (registered==1) { pthread_mutex_unlock(&g_dispatch_lock); return NULL; }
    if (registered<0) {
        LOG("tile: cannot determine registration state; no automatic installation\n");
        if (g_tile_notify) g_tile_notify("SteamController-PS5: cannot verify Media tile state; no installation attempted");
        pthread_mutex_unlock(&g_dispatch_lock); return NULL;
    }
    int result=settings_tile_create(message,sizeof(message));
    LOG("tile: automatic first-run registration %s: %s\n",result?"failed":"accepted",message);
    if (g_tile_notify) g_tile_notify(message);
    pthread_mutex_unlock(&g_dispatch_lock);
    return NULL;
}

static void *automatic_tile_thread(void *unused) {
    void *result = automatic_tile_impl(unused);
    pthread_mutex_lock(&g_tile_lock);
    g_tile_done = 1;
    pthread_cond_broadcast(&g_tile_cond);
    pthread_mutex_unlock(&g_tile_lock);
    return result;
}

int webui_start(webui_notify_fn notify) {
    if(!g_webui_thread_started)diagnostics_webui(-1);
    if (g_webui_thread_started) return -1;
    if (!notify) {
        LOG("webui: disabled; local notification callback unavailable\n");
        return -1;
    }
    int s = socket(AF_INET, SOCK_STREAM, 0);
    int one = 1;
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET; address.sin_port = htons(WEBUI_PORT);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (s < 0 || s >= FD_SETSIZE ||
        setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) != 0 ||
        bind(s, (struct sockaddr *)&address, sizeof(address)) != 0 || listen(s, 8) != 0) {
        LOG("webui: listener initialization failed errno=%d\n", errno);
        if (s >= 0) close(s);
        notify("SteamController-PS5 — WebUI unavailable: socket/bind/listen failed"); return -1;
    }
    memset(g_clients, 0, sizeof(g_clients));
    g_listener = s;
    {
        struct sockaddr_in bound;
        socklen_t bound_length = sizeof(bound);
        if (getsockname(s, (struct sockaddr *)&bound, &bound_length) == 0)
            g_listener_port = (unsigned)ntohs(bound.sin_port);
    }
    atomic_store(&g_webui_stopping, 0);
    int result = pthread_create(&g_webui_thread, NULL, server_thread, NULL);
    if (result != 0) {
        close(s); g_listener = -1;
        LOG("webui: server thread failed (%d)\n", result);
        notify("SteamController-PS5 — WebUI unavailable: server thread failed"); return -1;
    }
    g_webui_thread_started = 1;
    diagnostics_webui(1);
    notify("SteamController-PS5 — WebUI ready on PS5 (localhost:8090)");
    g_tile_notify=notify;
    pthread_mutex_lock(&g_tile_lock);g_tile_done=0;pthread_mutex_unlock(&g_tile_lock);
    if (pthread_create(&g_tile_thread,NULL,automatic_tile_thread,NULL)==0)
        g_tile_thread_started=1;
    else {
        LOG("tile: automatic installer thread could not start\n");
        notify("SteamController-PS5: automatic Media tile installer could not start");
    }
    LOG("webui: listener ready on loopback port %d; no network access\n", WEBUI_PORT);
    return 0;
}

int webui_stop(void) {
    if (!g_webui_thread_started) return 0;
    atomic_store(&g_webui_stopping, 1);
    if (pthread_join(g_webui_thread, NULL) != 0) return -1;
    if (g_tile_thread_started) {
        struct timespec deadline;
        int have_deadline = clock_gettime(CLOCK_REALTIME, &deadline) == 0;
        if (have_deadline) deadline.tv_sec += 5;
        pthread_mutex_lock(&g_tile_lock);
        while (!g_tile_done && have_deadline)
            if (pthread_cond_timedwait(&g_tile_cond, &g_tile_lock, &deadline) == ETIMEDOUT)
                break;
        int done = g_tile_done;
        pthread_mutex_unlock(&g_tile_lock);
        if (!done) {
            /* Do not join a blocking installer call here: shutdown proceeds and
             * process exit reclaims the worker. */
            LOG("tile: worker still busy at shutdown; process exit will reclaim it\n");
        } else if (pthread_join(g_tile_thread, NULL) != 0) {
            return -1;
        }
        g_tile_thread_started=0;
    }
    g_tile_notify=NULL;
    g_webui_thread_started = 0; g_listener = -1;
    diagnostics_webui(0);
    return 0;
}
