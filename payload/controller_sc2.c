/* controller_sc2.c — Steam Controller (2026) via Puck / USB for Ghost-Control
 * Raw report → physical inputs → active profile (sc2_profile.c) → ScePadData */

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "controller_sc2.h"
#include "sc2_status.h"
#include "sc2_profile.h"
#include "sc2_menu.h"
#include "sc2_binding.h"
#include "sc2_haptics.h"
#include "sc2_gyro.h"
#include "sc2_trigfx.h"
#include "sc2_flick.h"
#include "runtime_control.h"
#include "usb_helpers.h"
#include "steam_protocol.h"
#include "steam_mapping.h"
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>

#ifdef __PROSPERO__
void ghostpad_status_log(const char *fmt, ...);   /* gc_main.c: klog + /data/ghostpad/gc_status.log */
#define LOG(...) ghostpad_status_log("[GC] " __VA_ARGS__)
#else
#define LOG(...) fprintf(stderr, __VA_ARGS__)
#endif

#define SC2_TRIG_THRESHOLD  16          /* digital press point (0-255) */
#define SC2_PAUSE_HOLD_MS   5000        /* hold … this long to pause/resume */

#define RID_INPUT     0x42
#define RID_INPUT2    0x45

/* ── endpoint discovery ───────────────────────────────────────────────── */

static int sc2_read_usb_map(int fd, int puck_only, sc2_ep_map_t *map) {
    uint8_t d[1024] = {0};
    struct usb_gen_descriptor g;
    if (!map) return SC2_USB_MAP_INVALID;
    memset(map, 0, sizeof(*map));
    memset(&g, 0, sizeof(g));
    g.ugd_data = d; g.ugd_maxlen = sizeof(d);
    g.ugd_config_index = 0xFF;                 /* current config */
    if (ioctl(fd, USB_GET_FULL_DESC, &g) != 0) {
        int error = errno;
        LOG("sc2: GET_FULL_DESC fail errno=%d\n", error);
        return error == ENXIO ? SC2_USB_MAP_DEVICE_GONE :
                                SC2_USB_MAP_UNAVAILABLE;
    }
    unsigned total = g.ugd_actlen ? (unsigned)g.ugd_actlen :
                     (unsigned)d[2] | ((unsigned)d[3] << 8);
    if (total > sizeof(d)) {
        LOG("sc2: GET_FULL_DESC length %u exceeds buffer\n", total);
        return SC2_USB_MAP_INVALID;
    }
    int n = steam_usb_parse_config(d, total, puck_only, map);
    if (n < 0) {
        LOG("sc2: malformed USB configuration descriptor (length %u)\n", total);
        return SC2_USB_MAP_INVALID;
    }
    return n;
}

int sc2_ep_map(int fd, int puck_only, sc2_ep_map_t *m) {
    if (!m) return SC2_USB_MAP_INVALID;
    int count = sc2_read_usb_map(fd, puck_only, m);
    if (count < 0) return count;
    for (int k = 0; k < m->n; k++)
        LOG("sc2: iface %d class 0x%02x: IN 0x%02x OUT 0x%02x\n", m->it[k].iface,
            m->it[k].iclass, m->it[k].in_ep, m->it[k].out_ep);
    return count;
}

int sc2_collect_in_eps(const sc2_ep_map_t *map, uint8_t *out, int max) {
    if (!map || !out || max <= 0 || map->n < 0 ||
        map->n > STEAM_USB_MAX_INTERFACES)
        return SC2_USB_MAP_INVALID;
    int n = 0;
    for (int i = 0; i < map->n && n < max; i++) {
        if (map->it[i].iclass == 0x03 && map->it[i].in_ep)
            out[n++] = map->it[i].in_ep;
    }
    return n;
}

int sc2_iface_of_in(const sc2_ep_map_t *m, uint8_t in_ep) {
    for (int k = 0; k < m->n; k++) if (m->it[k].in_ep == in_ep) return k;
    return -1;
}

void sc2_input_state_reset(sc2_input_state_t *state) {
    if (!state) return;
    memset(state, 0, sizeof(*state));
    sc2_bind_reset(&state->binding);
    sc2_flick_reset(&state->flick);
    sc2_trigfx_reset(&state->trigfx);
}

/* ── active-slot search ───────────────────────────────────────────────── */

static void sc2_scan_remember_error(int *first_error, int result) {
    if (*first_error == 0 && result != 0) *first_error = result;
}

static int sc2_scan_errno(void) {
    return errno ? -errno : USB_RESOURCES_ERROR;
}

static int sc2_scan_operation_error(int result) {
    return result == USB_SESSION_BAD_STATE ? USB_RESOURCES_BAD_STATE :
           sc2_scan_errno();
}

int sc2_find_active_ep(int fd, usb_resources_t *resources,
                       const usb_session_ops_t *ops, void *ops_context,
                       struct usb_fs_endpoint *eps,
                       const uint8_t *addrs, int n, int timeout_ms,
                       sc2_scan_cancel_fn should_cancel,
                       void *cancel_context) {

    int opened[SC2_MAX_EPS] = {0}, started[SC2_MAX_EPS] = {0};
    int found = 0, first_error = 0;
    if (!resources || resources->fd != fd || !resources->fs_initialized ||
        !ops || !ops->open_endpoint || !ops->start_transfer ||
        !ops->complete_transfer || !ops->resources.stop_transfer ||
        !ops->resources.close_endpoint || !eps || !addrs || n < 0 ||
        timeout_ms < 0)
        return USB_RESOURCES_BAD_STATE;
    if (n > SC2_MAX_EPS) n = SC2_MAX_EPS;
    if (n == 0) return 0;
    /* COMPLETE is multiplexed for the whole FS session. Do not consume a
     * completion owned by another endpoint before the scan has opened its
     * candidate set. */
    if ((resources->active_transfers | resources->completed_transfers | resources->retired_transfers) != 0)
        return USB_RESOURCES_BAD_STATE;
    for (int i = 0; i < n; i++) {
        if (resources->opened_endpoints & (UINT32_C(1) << i))
            return USB_RESOURCES_BAD_STATE;
    }

    for (int i = 0; i < n; i++) {
        struct usb_fs_open o; memset(&o, 0, sizeof(o));
        o.ep_index = i; o.ep_no = addrs[i]; o.max_bufsize = 64; o.max_frames = 1;
        errno = 0;
        int open_result = usb_session_open_endpoint(resources, ops,
                                                     (uint8_t)i, &o,
                                                     ops_context);
        if (open_result != 0) {
            if (open_result == USB_SESSION_BAD_STATE) {
                found = USB_SESSION_BAD_STATE;
                break;
            }
            if (errno == ENXIO) {
                found = -ENXIO;
                break;
            }
            continue;
        }
        opened[i] = 1;
        memset(resources->frame_data[i], 0, USB_RESOURCES_FRAME_CAPACITY);
        resources->frame_buffer[i][0] = resources->frame_data[i];
        resources->frame_length[i][0] = USB_RESOURCES_FRAME_CAPACITY;
        eps[i].ppBuffer = resources->frame_buffer[i];
        eps[i].pLength = resources->frame_length[i]; eps[i].nFrames = 1;
        eps[i].timeout = 100;
        eps[i].flags = USB_FS_FLAG_SINGLE_SHORT_OK | USB_FS_FLAG_MULTI_SHORT_OK;
    }

    for (int t = 0; t < timeout_ms && !found &&
         !(should_cancel && should_cancel(cancel_context)); t += 5) {
        for (int i = 0; i < n && !found; i++) {
            if (should_cancel && should_cancel(cancel_context)) break;
            if (!opened[i]) continue;
            if (!started[i]) {
                resources->frame_length[i][0] = USB_RESOURCES_FRAME_CAPACITY; eps[i].aFrames = 0; eps[i].status = 0;
                struct usb_fs_start s; memset(&s, 0, sizeof(s)); s.ep_index = i;
                errno = 0;
                int start_result = usb_session_start_transfer(resources, ops,
                        (uint8_t)i, &s, ops_context);
                if (start_result == 0) started[i] = 1;
                else if (start_result == USB_SESSION_BAD_STATE) {
                    found = USB_SESSION_BAD_STATE;
                    break;
                } else if (errno == ENXIO) { found = -ENXIO; break; }
                else if (errno == EBUSY) started[i] = 1;
            }
        }
        struct usb_fs_complete c; memset(&c, 0, sizeof(c));
        uint8_t completed_endpoint = UINT8_MAX;
        int complete_result = USB_SESSION_OK;
        while (!found && complete_result == USB_SESSION_OK &&
               !(should_cancel && should_cancel(cancel_context)) &&
               resources->active_transfers != 0) {
            errno = 0;
            complete_result = usb_session_complete_any_transfer(resources,
                    ops, &c, &completed_endpoint, ops_context);
            if (complete_result != USB_SESSION_OK) break;
            (void)usb_resources_take_completion(resources, completed_endpoint);
            int i = completed_endpoint;
            if (i >= 0 && i < n && opened[i] && started[i]) {
                started[i] = 0;
                steam_report_t report;
                if (usb_completed_frame_valid(&eps[i], USB_RESOURCES_FRAME_CAPACITY) &&
                    steam_report_decode(resources->frame_data[i], resources->frame_length[i][0], &report) == 1 &&
                    report.kind == STEAM_REPORT_INPUT) {
                    LOG("sc2: controller active on ep 0x%02x\n", addrs[i]);
                    found = addrs[i];
                }
            }
            memset(&c, 0, sizeof(c));
        }
        if (!found && complete_result == USB_SESSION_BAD_STATE) {
            sc2_scan_remember_error(&first_error,
                                    sc2_scan_operation_error(complete_result));
            found = first_error;
        }
        if (!found && errno == ENXIO) found = -ENXIO;
        if (!found) runtime_sleep_ms(5);
    }

    if (!found && should_cancel && should_cancel(cancel_context))
        found = -ECANCELED;

    for (int i = 0; i < n; i++) {
        if (!opened[i]) continue;
        if (resources->active_transfers & (UINT32_C(1) << i)) {
            errno = 0;
            int stop_result = usb_session_stop_transfer(resources, ops,
                    (uint8_t)i, ops_context);
            if (stop_result != 0) {
                sc2_scan_remember_error(&first_error,
                        sc2_scan_operation_error(stop_result));
            }
        }
        errno = 0;
        int close_result = usb_session_close_endpoint(resources, ops,
                (uint8_t)i, ops_context);
        if (close_result != 0) {
            sc2_scan_remember_error(&first_error,
                    sc2_scan_operation_error(close_result));
        }
    }
    return first_error != 0 ? first_error : found;
}

/* ── pause / resume ───────────────────────────────────────────────────── */

static pthread_mutex_t g_pause_transition_lock = PTHREAD_MUTEX_INITIALIZER;

void sc2_set_paused(int on) {
    on = on ? 1 : 0;
    pthread_mutex_lock(&g_pause_transition_lock);
    int previous = sc2_pause_exchange(on);
    if (on == previous) {
        pthread_mutex_unlock(&g_pause_transition_lock);
        return;
    }
    sc2_menu_request_close();
    sc2_bind_invalidate_sessions();
    if (on) {
        sc2_haptic_rumble_set(0, 0);
        sc2_haptic_rumble_for(0x8000, 0x8000, 450);      /* one long buzz */
        LOG("sc2: Puckbridge paused\n");
        if (sc2_notify_fn) sc2_notify_fn("Puckbridge paused. Your DualSense works as normal. Hold the ... button for 5 seconds to resume.");
    } else {
        sc2_haptic_rumble_for(0x8000, 0x8000, 120);      /* short buzz */
        LOG("sc2: Puckbridge resumed\n");
        if (sc2_notify_fn) sc2_notify_fn("Puckbridge resumed.");
    }
    pthread_mutex_unlock(&g_pause_transition_lock);
}

/* ── power off ────────────────────────────────────────────────────────── */

int sc2_power_off(int fd, int iface) {
    int r = -1;
    for (int tries = 0; tries < 3; tries++) {
        uint8_t buf[64]; memset(buf, 0, sizeof(buf));
        buf[0] = 0x01;                                   /* feature report id */
        buf[1] = 0x9F; buf[2] = 0x04;                    /* ID_TURN_OFF_CONTROLLER, len 4 */
        buf[3] = 'o'; buf[4] = 'f'; buf[5] = 'f'; buf[6] = '!';
        struct usb_ctl_request req; memset(&req, 0, sizeof(req));
        req.ucr_data = buf;
        req.ucr_request.bmRequestType = 0x21;
        req.ucr_request.bRequest      = 0x09;            /* SET_REPORT */
        USETW(req.ucr_request.wValue,  (uint16_t)((3 << 8) | 0x01));
        USETW(req.ucr_request.wIndex,  iface);
        USETW(req.ucr_request.wLength, sizeof(buf));
        r = ioctl(fd, USB_DO_REQUEST, &req) == 0 ? 0 : -errno;
        if (r != -EPIPE) break;                          /* wireless sometimes stalls once */
        runtime_sleep_ms(20);
    }
    LOG("sc2: power-off request (iface %d) -> %d\n", iface, r);
    return r;
}

/* ── parsing ──────────────────────────────────────────────────────────── */

/* Pad (+Y up) → DualSense touch (0..1919 x 0..1079), left/right half */
static void pad_touch(ScePadTouch *t, int16_t x, int16_t y, int right, uint8_t id) {
    steam_touch_position_t position;
    steam_pad_touch_map(x, y, right, id, &position);
    t->x = position.x;
    t->y = position.y;
    t->finger = position.finger;
}

static void sc2_neutralize_pad(ScePadData *pad) {
    memset(pad, 0, sizeof(*pad));
    pad->leftStick.x = pad->leftStick.y = 128;
    pad->rightStick.x = pad->rightStick.y = 128;
    pad->connected = 1;
    pad->quat.w = 1.0f;
}

/* Pad position → d-pad direction (outer 40% of travel only) */
static uint32_t pad_dpad(int16_t x, int16_t y) {
    uint32_t directions = steam_pad_to_dpad(x, y), buttons = 0;
    if (directions & STEAM_DPAD_UP) buttons |= SCE_PAD_BUTTON_UP;
    if (directions & STEAM_DPAD_DOWN) buttons |= SCE_PAD_BUTTON_DOWN;
    if (directions & STEAM_DPAD_LEFT) buttons |= SCE_PAD_BUTTON_LEFT;
    if (directions & STEAM_DPAD_RIGHT) buttons |= SCE_PAD_BUTTON_RIGHT;
    return buttons;
}

/* ── battery ──────────────────────────────────────────────────────────── */

/* Report 0x43 (Linux hid-steam, Ibex): [1] charge state, [2] level %,
 * then voltages, currents and temperature. Warns once below 15% and again
 * below 5% while discharging; charging or getting back above 20% re-arms. */
static void battery_report(uint8_t raw_state, uint8_t raw_level) {
    int lvl = raw_level > 100 ? 100 : raw_level;
    int state = raw_state == 2 ? SC2_BATT_CHARGING :
                raw_state == 4 ? SC2_BATT_FULL : SC2_BATT_DISCHARGING;
    unsigned changes = sc2_status_update_battery(lvl, state);
    if (changes & SC2_STATUS_BATTERY_CHANGED)
        LOG("sc2: battery %d%% (%s)\n", lvl,
            state == SC2_BATT_CHARGING ? "charging" : state == SC2_BATT_FULL ? "charged" : "on battery");
    const char *m = changes & SC2_STATUS_BATTERY_WARN_5
        ? "Puckbridge: Steam Controller battery at 5%. Charge it soon."
        : changes & SC2_STATUS_BATTERY_WARN_15
        ? "Puckbridge: Steam Controller battery low (15%)." : NULL;
    if (m && sc2_notify_fn) sc2_notify_fn(m);
}

void sc2_battery_reset(void) { (void)sc2_status_update_battery(-1, 0); }

static uint32_t profile_input_mask(const steam_input_report_t *report) {
    static const uint8_t profile_input[STEAM_CTL_COUNT] = {
        IN_A, IN_B, IN_X, IN_Y, IN_LB, IN_RB, IN_LT, IN_RT,
        IN_L3, IN_R3, IN_DUP, IN_DDOWN, IN_DLEFT, IN_DRIGHT,
        IN_VIEW, IN_MENU, IN_STEAM, IN_QAM, IN_L4, IN_L5, IN_R4, IN_R5,
        IN_LPAD_CLICK, IN_RPAD_CLICK
    };
    uint32_t mask = 0;
    for (int i = 0; i < STEAM_CTL_COUNT; i++)
        if (report->controls & ((uint32_t)1u << i))
            mask |= (uint32_t)1u << profile_input[i];
    return mask;
}

int sc2_handle_packet(const uint8_t *b, uint32_t len, ScePadData *o, int *link,
                      sc2_input_state_t *input_state) {
    steam_report_t decoded;
    if (!o || !link || !b || !input_state || len == 0) return 0;
    int decoded_ok = steam_report_decode(b, len, &decoded);
    if (decoded_ok != 1) {
        if (decoded.kind != STEAM_REPORT_INPUT) return 0;
        /* A truncated known input frame must release prior controls. Unknown
         * reports are ignored, but malformed input cannot leave stale buttons
         * or touch contacts held in the virtual pad. */
        sc2_neutralize_pad(o);
        *link = 1;
        sc2_status_set_input(0, 1);
        sc2_input_state_reset(input_state);
        sc2_gyro_reset();
        sc2_gyro_set_moving(0);
        return 1;
    }
    if (decoded.kind == STEAM_REPORT_WIRELESS) {
        if (decoded.data.wireless.event == 1) {
            *link = 0; sc2_status_set_input(0, 0);
            sc2_input_state_reset(input_state);
            sc2_gyro_reset();
            sc2_battery_reset();
            sc2_neutralize_pad(o);
            return 1;
        }
        if (decoded.data.wireless.event == 2) *link = 1;
        return 0;
    }
    if (decoded.kind == STEAM_REPORT_BATTERY) {
        battery_report(decoded.data.battery.state, decoded.data.battery.level);
        return 0;
    }
    if (decoded.kind != STEAM_REPORT_INPUT) return 0;
    *link = 1;
    const steam_input_report_t *report = &decoded.data.input;

    sc2_profile_t P;
    uint32_t profile_generation;
    sc2_active_get_versioned(&P, &profile_generation);

    uint8_t lt = steam_trigger_to_u8(report->left_trigger_raw);
    uint8_t rt = steam_trigger_to_u8(report->right_trigger_raw);

    /* 1. physical inputs */
    uint32_t in = profile_input_mask(report);
    if (rt > SC2_TRIG_THRESHOLD) in |= (uint32_t)1u << IN_RT;
    if (lt > SC2_TRIG_THRESHOLD) in |= (uint32_t)1u << IN_LT;
    sc2_status_set_input(in, 1);

    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    int64_t now = (int64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;

    sc2_trigfx_apply(&input_state->trigfx, lt, rt, now, P.trig_fx); /* game's trigger effects, as haptics */

    /* 1a. … is reserved for Puckbridge: hold it 5 s anywhere to pause /
     *     resume. It never reaches bindings, the menu or the PS5. */
    if (in & (1u << IN_QAM)) {
        if (!input_state->qam_since_ms) input_state->qam_since_ms = now;
        else if (!input_state->qam_latched &&
                 now - input_state->qam_since_ms >= SC2_PAUSE_HOLD_MS) {
            input_state->qam_latched = 1;
            sc2_set_paused(!sc2_pause_is_active());
        }
    } else {
        input_state->qam_since_ms = 0;
        input_state->qam_latched = 0;
    }
    in &= ~(1u << IN_QAM);
    if (sc2_pause_is_active()) {
        sc2_bind_reset(&input_state->binding);
        input_state->previous_actions = 0;
        sc2_flick_reset(&input_state->flick);
        return 0;                             /* nothing reaches the PS5 */
    }

#ifndef SC2_RAW_LOG
#define SC2_RAW_LOG 0   /* bit table verified on hardware; set 1 to re-check */
#endif
#if SC2_RAW_LOG
    /* Edge-triggered raw dump: fires only when the decoded physical-input
     * mask changes, so a button press/release logs the exact report bytes
     * next to what we parsed from them. Used to verify/fix the bit table
     * in SET(...) above against real hardware. */
    {
        static uint32_t last_in = 0xFFFFFFFFu;
        if (in != last_in) {
            LOG("sc2: raw b[0..9]=%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x  in=0x%06x\n",
                b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7], b[8], b[9], in);
            last_in = in;
        }
    }
#endif

    /* 1b. Puckbridge menu owns the controller while open */
    if (sc2_menu_filter(&in)) {
        o->leftStick.x = o->leftStick.y = o->rightStick.x = o->rightStick.y = 128;
        o->buttons = 0; o->connected = 1; o->quat.w = 1.0f;
        sc2_bind_reset(&input_state->binding);
        input_state->previous_actions = 0;
        sc2_flick_reset(&input_state->flick);
        sc2_gyro_set_moving(0);
        return 1;
    }

    /* 2. Steam Input-style bindings */
    uint32_t btn = sc2_bind_eval(&input_state->binding, &P, profile_generation, in,
        (report->controls & ((uint32_t)1u << STEAM_CTL_LT)) != 0,
        (report->controls & ((uint32_t)1u << STEAM_CTL_RT)) != 0, now);
    if (input_state->binding.neutral_report) {
        sc2_flick_reset(&input_state->flick);
        input_state->previous_actions = 0;
        sc2_gyro_reset();
        sc2_gyro_set_moving(0);
        sc2_neutralize_pad(o);
        return 1;
    }
    uint32_t act = btn & PB_ACT_MASK;
    if ((act & PB_ACT_MENU) && !(input_state->previous_actions & PB_ACT_MENU)) sc2_menu_toggle();
    input_state->previous_actions = act;
    sc2_gyro_buttons(act);
    btn &= ~PB_ACT_MASK;

    /* 3. Pressure follows the binding engine's evaluated source provenance. */
    uint8_t ol2 = 0, or2 = 0;
    sc2_bind_trigger_pressure(&input_state->binding, lt, rt, &ol2, &or2);
    o->analogButtons.l2 = ol2;
    o->analogButtons.r2 = or2;

    /* 4. sticks */
    int dz = (int)P.deadzone * 32767 / 100;
    int16_t lx = report->left_x, ly = report->left_y;
    int16_t rx = report->right_x, ry = report->right_y;
    if (P.swap_sticks) { int16_t t; t = lx; lx = rx; rx = t; t = ly; ly = ry; ry = t; }

    /* 5. trackpads */
    int lpt = report->left_pad_touched, rpt = report->right_pad_touched;
    int16_t lpx = report->left_pad_x, lpy = report->left_pad_y;
    int16_t rpx = report->right_pad_x, rpy = report->right_pad_y;
    int nf = 0;
    if (lpt) switch (P.lpad) {
        case PAD_TOUCH: pad_touch(&o->touchData.touch[nf++], lpx, lpy, 0, 0); break;
        case PAD_STICK: lx = lpx; ly = lpy; break;
        case PAD_DPAD:  btn |= pad_dpad(lpx, lpy); break;
    }
    if (rpt) switch (P.rpad) {
        case PAD_TOUCH: pad_touch(&o->touchData.touch[nf++], rpx, rpy, 1, 1); break;
        case PAD_STICK: rx = rpx; ry = rpy; break;
        case PAD_DPAD:  btn |= pad_dpad(rpx, rpy); break;
    }
    o->touchData.fingers = (uint8_t)nf;

    o->leftStick.x  = steam_axis_to_u8(lx, dz, 0);
    o->leftStick.y  = steam_axis_to_u8(ly, dz, !P.invert_ly);
    if (P.rstick == RSTICK_FLICK) {                  /* flick stick: turns only */
        int f = sc2_flick_apply(&input_state->flick, &P, rx, ry, now);
        o->rightStick.x = (uint8_t)(128 + f);
        o->rightStick.y = 128;
    } else {
        o->rightStick.x = steam_axis_to_u8(rx, dz, 0);
        o->rightStick.y = steam_axis_to_u8(ry, dz, !P.invert_ry);
    }

    /* 6. gyro aiming: added on top of the right stick */
    int gdx, gdy;
    sc2_gyro_apply(&P, report, now, &gdx, &gdy);
    if (gdx || gdy) {
        int x = o->rightStick.x + gdx, y = o->rightStick.y + gdy;
        o->rightStick.x = (uint8_t)(x < 0 ? 0 : x > 255 ? 255 : x);
        o->rightStick.y = (uint8_t)(y < 0 ? 0 : y > 255 ? 255 : y);
    }
    sc2_gyro_set_moving(gdx || gdy);

    if (ol2) btn |= SCE_PAD_BUTTON_L2; else btn &= ~SCE_PAD_BUTTON_L2;
    if (or2) btn |= SCE_PAD_BUTTON_R2; else btn &= ~SCE_PAD_BUTTON_R2;
    /* keep digital L2/R2 only when the analog value clears the threshold */
    if (ol2 && ol2 <= SC2_TRIG_THRESHOLD) btn &= ~SCE_PAD_BUTTON_L2;
    if (or2 && or2 <= SC2_TRIG_THRESHOLD) btn &= ~SCE_PAD_BUTTON_R2;

    o->buttons   = btn;
    o->connected = 1;
    o->quat.w    = 1.0f;
    sc2_gyro_motion(&P, report, now, o);         /* 7. motion for games */
    return 1;
}
