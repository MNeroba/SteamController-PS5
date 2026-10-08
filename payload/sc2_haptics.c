/* sc2_haptics.c — Steam Controller (2026) haptic output
 * Report formats from Linux hid-steam.c (Ibex):
 *   0x81 pulse : side u8, on_us u16, off_us u16, repeat u16      (8 bytes)
 *   0x80 rumble: type u8, intensity u16, L{speed u16, gain u8}, R{...} (10 bytes)
 * Rumble has to be refreshed (hid-steam resends every 50 ms). */

#include "sc2_haptics.h"
#include "sc2_status.h"
#include "usb_helpers.h"
#include "steam_feedback.h"
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/ioctl.h>

#ifdef __PROSPERO__
void ghostpad_status_log(const char *fmt, ...);
#define LOG(...) ghostpad_status_log("[GC] " __VA_ARGS__)
#else
#define LOG(...) fprintf(stderr, __VA_ARGS__)
#endif

#define QN 8
static pthread_mutex_t q_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t log_lock = PTHREAD_MUTEX_INITIALIZER;
static struct { uint8_t b[12]; uint8_t n; } q[QN];
static int q_head = 0, q_tail = 0;
static slot_registry_token_t q_owner;
static int q_owner_valid;
static int last_method = -1;
static unsigned n_err_logs;
static int logged_fail, logged_ok;

static uint16_t r_left = 0, r_right = 0;
static int64_t  r_until = 0, r_next = 0;
static int      r_active = 0;

static int token_equal(slot_registry_token_t a, slot_registry_token_t b) {
    return a.index == b.index && a.generation == b.generation;
}

static int is_owner_locked(slot_registry_token_t owner) {
    return q_owner_valid && token_equal(q_owner, owner);
}

static void clear_session_locked(void) {
    q_head = q_tail = 0;
    memset(q, 0, sizeof(q));
    r_left = r_right = 0;
    r_until = r_next = 0;
    r_active = 0;
}

int sc2_haptic_session_claim(slot_registry_token_t owner) {
    if (owner.index >= SLOT_REGISTRY_MAX_SLOTS || owner.generation == 0)
        return 0;
    pthread_mutex_lock(&q_lock);
    int claimed = 0;
    if (q_owner_valid) {
        claimed = token_equal(q_owner, owner);
    } else {
        clear_session_locked();
        q_owner = owner;
        q_owner_valid = 1;
        claimed = 1;
    }
    pthread_mutex_unlock(&q_lock);
    return claimed;
}

int sc2_haptic_session_is_owner(slot_registry_token_t owner) {
    pthread_mutex_lock(&q_lock);
    int result = is_owner_locked(owner);
    pthread_mutex_unlock(&q_lock);
    return result;
}

void sc2_haptic_session_abandon(slot_registry_token_t owner) {
    pthread_mutex_lock(&q_lock);
    if (is_owner_locked(owner)) {
        q_owner_valid = 0;
        memset(&q_owner, 0, sizeof(q_owner));
        clear_session_locked();
    }
    pthread_mutex_unlock(&q_lock);
}

static int64_t now_ms(void) {
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

static void push(const uint8_t *b, uint8_t n) {
    pthread_mutex_lock(&q_lock);
    int nx = (q_tail + 1) % QN;
    if (q_owner_valid && nx != q_head) {
        memcpy(q[q_tail].b, b, n);
        q[q_tail].n = n;
        q_tail = nx;
        sc2_status_haptic_queued();
    }
    pthread_mutex_unlock(&q_lock);
}

void sc2_haptic_pulse(int pad, uint16_t on_us, uint16_t off_us, uint16_t count) {
    uint8_t b[STEAM_HAPTIC_PULSE_SIZE];
    int n = steam_feedback_encode_pulse(b, sizeof(b), pad, on_us, off_us, count);
    if (n > 0) push(b, (uint8_t)n);
}

void sc2_haptic_tick(void) { sc2_haptic_pulse(SC2_PAD_BOTH, 0x190, 0, 1); }

void sc2_haptic_rumble_set(uint16_t left, uint16_t right) {
    pthread_mutex_lock(&q_lock);
    if (q_owner_valid) {
        r_left = left; r_right = right;
        if (left || right) { r_until = INT64_MAX; r_active = 1; r_next = 0; }
        else               { r_until = 0; }             /* service sends the stop */
    }
    pthread_mutex_unlock(&q_lock);
}

void sc2_haptic_rumble_for(uint16_t left, uint16_t right, int ms) {
    pthread_mutex_lock(&q_lock);
    if (q_owner_valid) {
        r_left = left; r_right = right;
        r_until = now_ms() + ms; r_next = 0; r_active = 1;
    }
    pthread_mutex_unlock(&q_lock);
}

static int ctrl_report(int fd, int iface, int type, const uint8_t *b, uint8_t n) {
    uint8_t buf[64]; memset(buf, 0, sizeof(buf)); memcpy(buf, b, n);
    uint16_t len = (type == 3) ? 64 : n;          /* feature reports are fixed-size */
    struct usb_ctl_request req; memset(&req, 0, sizeof(req));
    req.ucr_data = buf;
    req.ucr_request.bmRequestType = 0x21;
    req.ucr_request.bRequest      = 0x09;                  /* SET_REPORT */
    USETW(req.ucr_request.wValue,  (uint16_t)((type << 8) | b[0]));
    USETW(req.ucr_request.wIndex,  iface);
    USETW(req.ucr_request.wLength, len);
    return ioctl(fd, USB_DO_REQUEST, &req) == 0 ? 0 : -errno;
}

/* Interrupt OUT on any opened FS endpoint index */
#ifdef SC2_HAPTICS_TEST
int sc2_haptics_test_send_out(int fd, int idx, const uint8_t *data, uint32_t len);
static int send_out_idx(usb_resources_t *resources,
                        const usb_session_ops_t *ops,
                        struct usb_fs_endpoint *eps, int idx,
                        const uint8_t *data, uint32_t len) {
    (void)ops;
    (void)eps;
    if (!resources) return -1;
    return sc2_haptics_test_send_out(resources->fd, idx, data, len);
}
#else
static int send_out_idx(usb_resources_t *resources,
                        const usb_session_ops_t *ops,
                        struct usb_fs_endpoint *eps, int idx,
                        const uint8_t *data, uint32_t len) {
    if (!resources || !ops || !eps || idx < 0 ||
        idx >= USB_RESOURCES_MAX_ENDPOINTS)
        return -EINVAL;
    return usb_send_out(resources, ops, &eps[idx], (uint8_t)idx, data, len,
                        "SC2 haptic", 10000, 20, NULL);
}
#endif

static int send_report(usb_resources_t *resources,
                       const usb_session_ops_t *ops,
                       struct usb_fs_endpoint *eps, int out_opened, int iface,
                       int n_extra, const uint8_t *b, uint8_t n) {
    int m = sc2_status_haptic_method(), r = -1, used = 0;
    if ((m == 0 || m == 1) && out_opened) { r = send_out_idx(resources, ops, eps, 1, b, n); used = 1; }
    if (m == 4) {
        used = 4; r = out_opened ? send_out_idx(resources, ops, eps, 1, b, n) : -1;
        for (int k = 0; k < n_extra; k++) {
            int rk = send_out_idx(resources, ops, eps, 2 + k, b, n);
            LOG("haptics: broadcast OUT index %d → %d\n", 2 + k, rk);
            if (rk == 0) r = 0;
        }
    }
    if (r != 0 && resources && resources->fd >= 0 &&
        (m == 0 || m == 2)) { r = ctrl_report(resources->fd, iface, 2, b, n); used = 2; }
    if (r != 0 && resources && resources->fd >= 0 && m == 3) {
        r = ctrl_report(resources->fd, iface, 3, b, n); used = 3;
    }
    sc2_status_haptic_result(used, r);
    pthread_mutex_lock(&log_lock);
    int should_log = m != last_method || r != 0;
    if (should_log && r != 0) {
        if (n_err_logs >= 10) should_log = 0;
        else n_err_logs++;
    }
    last_method = m;
    pthread_mutex_unlock(&log_lock);
    if (should_log)
        LOG("haptics: method %d, sent via %d (OUT ep %s), report 0x%02x len %u, result %d\n",
            m, used, out_opened ? "open" : "closed", b[0], n, r);
    return r;
}

int sc2_haptic_session_release(slot_registry_token_t owner,
                               usb_resources_t *resources,
                               const usb_session_ops_t *ops,
                               struct usb_fs_endpoint *eps, int out_opened,
                               int iface, int n_extra) {
    pthread_mutex_lock(&q_lock);
    if (!is_owner_locked(owner)) {
        pthread_mutex_unlock(&q_lock);
        return 0;
    }
    int stop_rumble = r_active;
    q_owner_valid = 0;
    memset(&q_owner, 0, sizeof(q_owner));
    clear_session_locked();
    pthread_mutex_unlock(&q_lock);

    if (!stop_rumble || !resources || resources->fd < 0) return 1;
    if (n_extra < 0) n_extra = 0;
    if (n_extra > SC2_HAP_MAX_OUT) n_extra = SC2_HAP_MAX_OUT;
    uint8_t report[STEAM_HAPTIC_RUMBLE_SIZE];
    int n = steam_feedback_encode_rumble(report, sizeof(report), 0, 0);
    if (n <= 0) return 0;
    return send_report(resources, ops, eps, out_opened, iface, n_extra, report,
                       (uint8_t)n) == 0;
}

void sc2_haptic_service(slot_registry_token_t owner,
                        usb_resources_t *resources,
                        const usb_session_ops_t *ops,
                        struct usb_fs_endpoint *eps, int out_opened,
                        int iface, int n_extra) {
    if (!resources || resources->fd < 0) return;
    if (n_extra < 0) n_extra = 0;
    if (n_extra > SC2_HAP_MAX_OUT) n_extra = SC2_HAP_MAX_OUT;
    uint8_t b[12]; uint8_t n;

    for (;;) {
        pthread_mutex_lock(&q_lock);
        if (!is_owner_locked(owner)) { pthread_mutex_unlock(&q_lock); return; }
        if (q_head == q_tail) { pthread_mutex_unlock(&q_lock); break; }
        memcpy(b, q[q_head].b, q[q_head].n); n = q[q_head].n;
        q_head = (q_head + 1) % QN;
        pthread_mutex_unlock(&q_lock);

        int r = send_report(resources, ops, eps, out_opened, iface, n_extra, b, n);
        pthread_mutex_lock(&log_lock);
        int log_failure = r && !logged_fail;
        int log_success = !r && !logged_ok;
        if (log_failure) logged_fail = 1;
        if (log_success) logged_ok = 1;
        pthread_mutex_unlock(&log_lock);
        if (log_failure) LOG("haptics: send 0x%02x failed (%d)\n", b[0], r);
        if (log_success) LOG("haptics: working (%s)\n", out_opened ? "interrupt OUT" : "SET_REPORT");
    }

    /* rumble refresh */
    pthread_mutex_lock(&q_lock);
    if (!is_owner_locked(owner)) { pthread_mutex_unlock(&q_lock); return; }
    int64_t t = now_ms();
    int send = 0; uint16_t l = 0, r = 0;
    if (r_active) {
        if (t >= r_until)     { r_active = 0; send = 1; }       /* stop */
        else if (t >= r_next) { l = r_left; r = r_right; r_next = t + 50; send = 1; }
    }
    pthread_mutex_unlock(&q_lock);
    if (send) {
        steam_feedback_encode_rumble(b, sizeof(b), l, r);
        send_report(resources, ops, eps, out_opened, iface, n_extra, b,
                    STEAM_HAPTIC_RUMBLE_SIZE);
}
}
