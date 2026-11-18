/* controller_xbox.c — Xbox One S GIP controller for Ghost-Control
 * All button bit positions hardware-confirmed on PS5 via live GIP probe.
 * Reference: xboxSeriesSButtonBits.md
 */

#include "controller_xbox.h"
#include "usb_helpers.h"
#include <string.h>
#include <errno.h>
#include <unistd.h>

#ifdef __PROSPERO__
#include <ps5/klog.h>
#define LOG(...) klog_printf("[GC] " __VA_ARGS__)
#else
#define LOG(...) fprintf(stderr, __VA_ARGS__)
#endif

/* Counts GIP INPUT packets — gates Guide button injection at startup */
static uint32_t g_input_count = 0;

/* ── helpers ──────────────────────────────────────────────────────────── */

static uint8_t trig_scale(uint16_t v) {
    if (v > 1023u) v = 1023u;
    return (uint8_t)((v * 255u) / 1023u);
}

#define DEADZONE 7849

static uint8_t stick_x(int16_t v) {
    return (v > DEADZONE || v < -DEADZONE) ? (uint8_t)((v + 32768) >> 8) : 128u;
}

static uint8_t stick_y(int16_t v) {
    return (v > DEADZONE || v < -DEADZONE) ? (uint8_t)(255 - ((v + 32768) >> 8)) : 128u;
}

/* ── input parsing ────────────────────────────────────────────────────── */

/* Parse GIP INPUT (cmd=0x20, 18 bytes) into ScePadData.
 * Hardware-confirmed bit positions — see xboxSeriesSButtonBits.md */
void xbox_parse_input(const uint8_t *b, ScePadData *o) {
    uint8_t  b4   = b[4];
    uint8_t  b5   = b[5];
    uint16_t lt16 = (uint16_t)b[6]  | ((uint16_t)b[7]  << 8);
    uint16_t rt16 = (uint16_t)b[8]  | ((uint16_t)b[9]  << 8);
    int16_t  lx   = (int16_t)((uint16_t)b[10] | ((uint16_t)b[11] << 8));
    int16_t  ly   = (int16_t)((uint16_t)b[12] | ((uint16_t)b[13] << 8));
    int16_t  rx   = (int16_t)((uint16_t)b[14] | ((uint16_t)b[15] << 8));
    int16_t  ry   = (int16_t)((uint16_t)b[16] | ((uint16_t)b[17] << 8));

    uint8_t lt = trig_scale(lt16);
    uint8_t rt = trig_scale(rt16);

    o->leftStick.x      = stick_x(lx);
    o->leftStick.y      = stick_y(ly);
    o->rightStick.x     = stick_x(rx);
    o->rightStick.y     = stick_y(ry);
    o->analogButtons.l2 = lt;
    o->analogButtons.r2 = rt;

    uint32_t btn = 0;

    /* b[4]: system + face buttons */
    if (b4 & 0x04u) btn |= SCE_PAD_BUTTON_OPTIONS;  /* Menu (≡)  → Options  */
    if (b4 & 0x08u) btn |= SCE_PAD_BUTTON_SHARE;    /* View (⧉)  → Share    */
    if (b4 & 0x10u) btn |= SCE_PAD_BUTTON_CROSS;    /* A         → Cross    */
    if (b4 & 0x20u) btn |= SCE_PAD_BUTTON_CIRCLE;   /* B         → Circle   */
    if (b4 & 0x40u) btn |= SCE_PAD_BUTTON_SQUARE;   /* X         → Square   */
    if (b4 & 0x80u) btn |= SCE_PAD_BUTTON_TRIANGLE; /* Y         → Triangle */

    /* b[5]: dpad + bumpers + stick clicks */
    if (b5 & 0x01u) btn |= SCE_PAD_BUTTON_UP;
    if (b5 & 0x02u) btn |= SCE_PAD_BUTTON_DOWN;
    if (b5 & 0x04u) btn |= SCE_PAD_BUTTON_LEFT;
    if (b5 & 0x08u) btn |= SCE_PAD_BUTTON_RIGHT;
    if (b5 & 0x10u) btn |= SCE_PAD_BUTTON_L1;       /* LB → L1 */
    if (b5 & 0x20u) btn |= SCE_PAD_BUTTON_R1;       /* RB → R1 */
    if (b5 & 0x40u) btn |= SCE_PAD_BUTTON_L3;       /* LS → L3 */
    if (b5 & 0x80u) btn |= SCE_PAD_BUTTON_R3;       /* RS → R3 */

    /* Triggers: analog + digital threshold */
    if (lt > 16u) btn |= SCE_PAD_BUTTON_L2;
    if (rt > 16u) btn |= SCE_PAD_BUTTON_R2;

    o->buttons   = btn;
    o->connected = 1;
    o->quat.w    = 1.0f;
}

/* ── GIP protocol ─────────────────────────────────────────────────────── */

static int read_one(usb_resources_t *resources, const usb_session_ops_t *ops,
                    struct usb_fs_endpoint *eps,
                    uint8_t *buf, uint32_t timeout_ms) {
    void *b[1] = {buf}; uint32_t l[1] = {64};
    eps[0].ppBuffer = b; eps[0].pLength = l; eps[0].nFrames   = 1;
    eps[0].timeout  = timeout_ms; eps[0].aFrames = 0; eps[0].status = 0;
    eps[0].flags    = USB_FS_FLAG_SINGLE_SHORT_OK | USB_FS_FLAG_MULTI_SHORT_OK;

    struct usb_fs_start st; memset(&st, 0, sizeof(st)); st.ep_index = 0;
    int start_result = usb_session_start_transfer(resources, ops, 0, &st, NULL);
    if (start_result != 0) {
        int saved_errno = errno ? errno : EIO;
        if (start_result != USB_SESSION_BAD_STATE &&
            usb_session_stop_transfer(resources, ops, 0, NULL) != 0)
            LOG("Xbox handshake START cleanup STOP failed errno=%d\n", errno);
        return -saved_errno;
    }

    int polls = (int)((timeout_ms + 300) / 50) + 1;
    for (int w = 0; w < polls; w++) {
        struct usb_fs_complete co; memset(&co, 0, sizeof(co));
        uint8_t completed_endpoint = UINT8_MAX;
        int result = usb_session_complete_transfer(resources, ops, 0, &co,
                                                   &completed_endpoint, NULL);
        if (result == USB_SESSION_OK && completed_endpoint == 0) {
            if (eps[0].aFrames == 0 || l[0] == 0) return 0;
            return (int)l[0];
        }
        if (result == USB_SESSION_COMPLETED_OTHER) continue;
        if (errno != EBUSY) {
            int saved_errno = errno ? errno : EIO;
            if (usb_session_stop_transfer(resources, ops, 0, NULL) != 0)
                LOG("Xbox handshake COMPLETE cleanup STOP failed errno=%d\n",
                    errno);
            return -saved_errno;
        }
        usleep(50000);
    }
    if (usb_session_stop_transfer(resources, ops, 0, NULL) != 0) {
        int saved_errno = errno ? errno : EIO;
        LOG("Xbox handshake timeout STOP failed errno=%d\n", saved_errno);
        return -saved_errno;
    }
    return 0;
}

static void send_ack(usb_resources_t *resources, const usb_session_ops_t *ops,
                     struct usb_fs_endpoint *eps, uint8_t orig_seq) {
    uint8_t ack[8] = {GIP_CMD_ACK, 0x00, 0x00, 0x04,
                      orig_seq, GIP_CMD_ANNOUNCE, 0x00, 0x00};
    (void)usb_send_out(resources, ops, &eps[1], 1, ack, 8, "ack", 50000,
                       20, NULL);
}

void xbox_gip_handshake(usb_resources_t *resources,
                        const usb_session_ops_t *ops,
                        struct usb_fs_endpoint *eps) {
    static const uint8_t power[] = {0x05, 0x20, 0x00, 0x01, 0x00};
    uint8_t buf[64];
    int announced = 0;

    g_input_count = 0;

    /* Pass 0: wait for ANNOUNCE; pass 1: send hello to re-trigger */
    for (int pass = 0; pass < 2 && !announced; pass++) {
        if (pass == 1) {
            uint8_t hello[4] = {GIP_CMD_ACK, 0x00, 0x00, 0x00};
            (void)usb_send_out(resources, ops, &eps[1], 1, hello, 4,
                               "hello", 50000, 20, NULL);
        }
        /* Pass 0: 3 reads — if ANNOUNCE was already sent during probe, we miss it fast.
         * Pass 1: send hello to re-trigger, wait longer. Handles both direct + hub. */
        int reads = (pass == 0) ? 3 : 60;
        for (int i = 0; i < reads && !announced; i++) {
            int n = read_one(resources, ops, eps, buf, 150);
            if (n < 0) {
                LOG("Xbox handshake read failed errno=%d\n", -n);
                return;
            }
            if (n > 0 && buf[0] == GIP_CMD_ANNOUNCE) {
                send_ack(resources, ops, eps, buf[1]);
                announced = 1;
            } else if (n > 0 && buf[0] == GIP_CMD_INPUT) {
                announced = 1;
            }
        }
    }

    (void)usb_send_out(resources, ops, &eps[1], 1, power, 5,
                       "power", 50000, 20, NULL);
    LOG("Xbox handshake done (announced=%d)\n", announced);
}

int xbox_handle_packet(usb_resources_t *resources,
                       const usb_session_ops_t *ops,
                       struct usb_fs_endpoint *eps,
                       const uint8_t *buf, uint32_t len,
                       ScePadData *out_pad) {
    (void)eps;
    uint8_t cmd = buf[0];

    /* Player input */
    if (cmd == GIP_CMD_INPUT && len >= 18) {
        g_input_count++;
        xbox_parse_input(buf, out_pad);
        return 1;
    }

    /* Guide button (Xbox logo): cmd=0x07, b[4]=0x01 pressed, 0x00 released.
     * Controller auto-sends this at connect — gate behind 10 INPUT packets
     * so the startup auto-send does not inject a PS button press. */
    if (cmd == 0x07) {
        out_pad->connected    = 1;
        out_pad->quat.w       = 1.0f;
        out_pad->leftStick.x  = 128;
        out_pad->leftStick.y  = 128;
        out_pad->rightStick.x = 128;
        out_pad->rightStick.y = 128;
        if (g_input_count > 10 && len >= 5 && (buf[4] & 0x01u))
            out_pad->buttons = SCE_PAD_BUTTON_PS;
        return 1;
    }

    /* Re-announce during streaming */
    if (cmd == GIP_CMD_ANNOUNCE && len >= 4) {
        static const uint8_t power[] = {0x05, 0x20, 0x00, 0x01, 0x00};
        send_ack(resources, ops, eps, buf[1]);
        (void)usb_send_out(resources, ops, &eps[1], 1, power, 5,
                           "repower", 50000, 20, NULL);
        return 0;
    }

    return 0;
}
