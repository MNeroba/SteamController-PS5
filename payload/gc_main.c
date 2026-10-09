/* SPDX-License-Identifier: GPL-3.0-or-later
 * Ghost-Control v5: Multi-controller support
 * USB HID controllers → virtual DualSense devices on PS5
 *
 * Supports up to MAX_SLOTS (4) simultaneous controllers:
 *   - 8BitDo / Nintendo Switch Pro  (VID=057E PID=2009)
 *   - Xbox One S                    (VID=045E PID=02EA)
 *
 * Each detected controller gets its own VDA device + force_bind
 * assignment dialog + VDI injection thread.
 * Hotplug: plug in any time, disconnect any time.
 */

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <signal.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <dev/usb/usb.h>
#include <dev/usb/usb_ioctl.h>
#include <dev/usb/usb_endian.h>

#ifdef __PROSPERO__
#include <ps5/kernel.h>
#include <ps5/klog.h>
#include <ps5/mdbg.h>
#endif

#include "shellui_pad.h"
#include "gc_types.h"
#include "usb_helpers.h"
#include "controller_nintendo.h"
#include "controller_xbox.h"
#include "controller_steam.h"
#include "controller_sc2.h"
#include "sc2_profile.h"
#include "sc2_control.h"
#include "sc2_status.h"
#include "webui.h"
#include "sc2_haptics.h"
#include "sc2_menu.h"
#include "sc2_gyro.h"
#include "ds_handoff.h"
#include "bridge_probe.h"
#include "game_hooks.h"
#include "bridge/wireless_ds4.h"
#include "slot_registry.h"
#include "slot_lifecycle.h"
#include "usb_session.h"
#include "diag_once.h"
#include "runtime_control.h"

/* ── Logging ──────────────────────────────────────────────────────────── */
#define LOG_DIR  "/data/ghostpad"
#define LOG_PATH "/data/ghostpad/gc_status.log"
#define PID_PATH "/data/ghostpad/gc_main.pid"
#define LOG_MAX  480

static pthread_mutex_t g_log_lock = PTHREAD_MUTEX_INITIALIZER;
static int g_log_fd = -1;

void ghostpad_status_log_reset(void) {
    pthread_mutex_lock(&g_log_lock);
    if (g_log_fd >= 0) { close(g_log_fd); g_log_fd = -1; }
    mkdir(LOG_DIR, 0755);
    rename(LOG_PATH, LOG_PATH ".prev");      /* keep the previous run (e.g. after a crash) */
    g_log_fd = open(LOG_PATH, O_WRONLY|O_CREAT|O_TRUNC, 0600);
    pthread_mutex_unlock(&g_log_lock);
}

void ghostpad_status_log(const char *fmt, ...) {
    char buf[LOG_MAX]; va_list ap;
    va_start(ap, fmt); vsnprintf(buf, sizeof(buf)-1, fmt, ap); va_end(ap);
    buf[LOG_MAX-1] = '\0';
    klog_printf("%s", buf);
    pthread_mutex_lock(&g_log_lock);
    if (g_log_fd >= 0) {
        size_t n = strnlen(buf, sizeof(buf));
        write(g_log_fd, buf, n);
        if (n && buf[n-1] != '\n') write(g_log_fd, "\n", 1);
    }
    pthread_mutex_unlock(&g_log_lock);
}
#define gp_log(...) ghostpad_status_log("[GC] " __VA_ARGS__)

/* ── SCE stubs ────────────────────────────────────────────────────────── */
extern int32_t sceUserServiceInitialize(void *params);
extern int32_t sceUserServiceGetInitialUser(int32_t *outUserId);
extern int32_t sceUserServiceGetForegroundUser(int32_t *outUserId);
extern int32_t scePadInit(void);
extern int32_t scePadSetProcessPrivilege(int32_t privilege);
extern int32_t scePadVirtualDeviceAddDevice(void *param, int32_t deviceType);
extern int32_t scePadVirtualDeviceDeleteDevice(int32_t handle);
extern int32_t scePadVirtualDeviceInsertData(int32_t handle, const void *padData);
extern int32_t sceKernelSendNotificationRequest(int unk0, void *req, size_t size, int unk1);

#define VIRTUAL_DEVICE_TYPE_DUALSENSE 3

/* ── Multi-controller slots ───────────────────────────────────────────── */
#define MAX_SLOTS SLOT_REGISTRY_MAX_SLOTS

static slot_registry_t g_slots;
static gc_diag_once_t g_vdi_error_log = GC_DIAG_ONCE_INIT;
/* Serialize AddDevice calls from concurrent controller sessions. A klog
 * DEVICE_ADDED event is not an ownership receipt. */
static pthread_mutex_t g_vda_create_lock = PTHREAD_MUTEX_INITIALIZER;
static int32_t         g_inject_uid = 0x10000000;
static volatile sig_atomic_t g_signal_stop;
static pthread_t g_klog_tid, g_manager_tid;
static int g_klog_started, g_manager_started, g_runtime_shutdown_done;

typedef struct {
    pthread_t thread;
    slot_registry_token_t token;
    int started;
    int done;
} usb_worker_record_t;

static pthread_mutex_t g_usb_workers_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_usb_workers_changed = PTHREAD_COND_INITIALIZER;
static usb_worker_record_t g_usb_workers[MAX_SLOTS];

static void signal_stop_handler(int signal_number) {
    (void)signal_number;
    g_signal_stop = 1;
}

/* ── Notification ─────────────────────────────────────────────────────── */
typedef struct { char _unk[45]; char message[3075]; } NotifyRequest;
static void notify(const char *fmt, ...) {
    NotifyRequest req; va_list ap;
    memset(&req, 0, sizeof(req));
    va_start(ap, fmt); vsnprintf(req.message, sizeof(req.message), fmt, ap); va_end(ap);
    sceKernelSendNotificationRequest(0, &req, sizeof(req), 0);
}

static void notify_str(const char *m) { notify("%s", m); }

/* ── klog capture thread ──────────────────────────────────────────────── */
static uint64_t parse_hex_str(const char *s) {
    uint64_t v = 0;
    while (*s) {
        char c = *s++;
        if      (c >= '0' && c <= '9') v = (v<<4)|(c-'0');
        else if (c >= 'a' && c <= 'f') v = (v<<4)|(c-'a'+10);
        else if (c >= 'A' && c <= 'F') v = (v<<4)|(c-'A'+10);
        else break;
    }
    return v;
}

static void parse_klog_line(const char *line) {
    /* Our own log lines come back through /dev/klog too; one quoting a
     * DEVICE_ADDED line would otherwise be counted as a second device. */
    if (strstr(line, "[GC]") || strstr(line, "[Ghostpad]")) return;
    ds_handoff_klog_line(line);
    if (!strstr(line, "DEVICE_ADDED"))        return;
    if (!strstr(line, "subType:22"))          return;
    if (!strstr(line, "capabilityBattery:0")) return;
    const char *p = strstr(line, "DeviceId:0x");
    if (!p) p = strstr(line, "deviceId=0x");
    if (!p) return;
    p += 11;
    uint64_t id = parse_hex_str(p);
    if (!id) return;
    gp_log("klog: VDA device 0x%llx\n", (unsigned long long)id);
    ds_handoff_note_virtual(id);
}

static void *klog_capture_thread(void *arg) {
    (void)arg;
    char buf[512], line[1024]; size_t line_len = 0;
    int fd = open("/dev/klog", O_RDONLY);
    if (fd < 0) {
        gp_log("klog: /dev/klog errno=%d, trying TCP 127.0.0.1:3232\n", errno);
        struct sockaddr_in sin; int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) return NULL;
        memset(&sin,0,sizeof(sin)); sin.sin_family=AF_INET;
        sin.sin_addr.s_addr=inet_addr("127.0.0.1"); sin.sin_port=htons(3232);
        for (int r=0; r<10 && !runtime_is_stopping(); r++) {
            if (connect(sock,(struct sockaddr*)&sin,sizeof(sin))==0){fd=sock;gp_log("klog: connected\n");break;}
            runtime_sleep_ms(500);
        }
        if (fd<0){close(sock);return NULL;}
    }
    while (!runtime_is_stopping()) {
        if (fd >= FD_SETSIZE) {
            gp_log("klog: descriptor exceeds select capacity; capture stopped\n");
            break;
        }
        fd_set readable;
        FD_ZERO(&readable);
        FD_SET(fd, &readable);
        struct timeval wait = { 0, 250000 };
        int ready = select(fd + 1, &readable, NULL, NULL, &wait);
        if (ready == 0) continue;
        if (ready < 0) {
            if (errno == EINTR) continue;
            gp_log("klog: select failed errno=%d\n", errno);
            break;
        }
        ssize_t len = read(fd, buf, sizeof(buf));
        if (len < 0) { if (errno==EINTR || errno==EAGAIN || errno==EWOULDBLOCK) continue; break; }
        if (len == 0) continue;
        for (ssize_t i=0; i<len; i++) {
            char c = buf[i];
            if (c=='\n'||line_len>=sizeof(line)-1){line[line_len]='\0';parse_klog_line(line);line_len=0;}
            else if (c!='\r') line[line_len++]=c;
        }
    }
    close(fd); return NULL;
}

/* ── VDI injection ────────────────────────────────────────────────────── */
typedef struct {
    steam_legacy_input_state_t steam_input;
    sc2_input_state_t sc2_input;
    uint32_t inject_count;
    int bind_ok;
    int32_t last_vdi_ret;
} usb_session_state_t;

static int slot_virtual_ready(slot_registry_token_t token) {
    slot_registry_entry_t entry;
    return slot_registry_snapshot(&g_slots, token, &entry) == SLOT_REGISTRY_OK &&
           entry.virtual_ready;
}

static void inject_pad(slot_registry_token_t token, int slot,
                       usb_session_state_t *session, const ScePadData *pad) {
    slot_registry_entry_t entry;
    if (!session ||
        slot_registry_snapshot(&g_slots, token, &entry) != SLOT_REGISTRY_OK ||
        !entry.virtual_ready)
        return;
    int32_t h = entry.virtual_handle;
    int vr = scePadVirtualDeviceInsertData(h, pad);
    session->last_vdi_ret = vr;
    uint32_t n = ++session->inject_count;
    if ((n % 600) == 0)
        gp_log("slot[%d] VDI #%u ret=0x%08x\n", slot, n, (uint32_t)vr);
    if (vr != 0 && gc_diag_once_claim(&g_vdi_error_log))
        gp_log("VDI error 0x%08x\n", (uint32_t)vr);
}

static int neutralize_vda_handle(int32_t handle) {
    ScePadData neutral;
    memset(&neutral, 0, sizeof(neutral));
    neutral.leftStick.x = neutral.leftStick.y = 128;
    neutral.rightStick.x = neutral.rightStick.y = 128;
    neutral.connected = 1;
    neutral.quat.w = 1.0f;
    int result = scePadVirtualDeviceInsertData(handle, &neutral);
    if (result != 0)
        gp_log("VDA neutral report failed handle=0x%x ret=0x%08x; delete remains the final neutralization step\n",
               (uint32_t)handle, (uint32_t)result);
    return result;
}

static int runtime_cancel_requested(void *context) {
    (void)context;
    return runtime_is_stopping();
}

/* ── ugen detection ───────────────────────────────────────────────────── */
#define VID_NATIVE  0x2dc8u
#define PID_NATIVE  0x310bu
#define VID_SWITCH  0x057eu
#define PID_SWITCH  0x2009u
#define VID_XBOX    0x045eu
#define PID_XBOX    0x02eau

#define MAX_UGEN_DEVS 64

/* Scan /dev for every ugen* entry (all buses — the PS5's internal USB
 * devices live on ugen0.x / ugen1.x, external controllers on ugen2.x but
 * that has been seen to vary, so we no longer hardcode which bus a
 * controller shows up on). */
static int scan_ugen_devices(char paths[][32], int max) {
    static int last_n = -1, last_err = -1;
    int n = 0, err = 0;
    DIR *d = opendir("/dev");
    if (d) {
        struct dirent *e;
        while (n < max && (e = readdir(d)) != NULL) {
            if (strncmp(e->d_name, "ugen", 4) != 0) continue;
            snprintf(paths[n], 32, "/dev/%s", e->d_name);
            n++;
        }
        closedir(d);
    } else {
        err = errno;
    }

    if (n != last_n || err != last_err) {          /* log only on change */
        if (!d) gp_log("scan: opendir(/dev) failed errno=%d\n", err);
        else {
            gp_log("scan: %d ugen node(s) in /dev\n", n);
            for (int i = 0; i < n; i++) gp_log("scan:   %s\n", paths[i]);
        }
        last_n = n; last_err = err;
    }

    /* Fallback: /dev not listable (or lists nothing) — try paths directly.
     * open() on a missing node just fails with ENOENT, which probe ignores. */
    if (n == 0) {
        for (int bus = 0; bus <= 3 && n < max; bus++)
            for (int dev = 1; dev <= 12 && n < max; dev++)
                snprintf(paths[n++], 32, "/dev/ugen%d.%d", bus, dev);
        if (last_n == 0 && err == last_err) { /* already logged */ }
    }
    return n;
}

/* Log a per-path failure once (so a 2s rescan doesn't spam the log). */
static int ugen_err_logged(const char *path, int code) {
    static char seen[MAX_UGEN_DEVS][32];
    static int  codes[MAX_UGEN_DEVS];
    static int  n_seen = 0;
    for (int i = 0; i < n_seen; i++)
        if (!strcmp(seen[i], path)) { if (codes[i] == code) return 1; codes[i] = code; return 0; }
    if (n_seen < MAX_UGEN_DEVS) {
        strncpy(seen[n_seen], path, 31); seen[n_seen][31] = 0; codes[n_seen] = code; n_seen++;
    }
    return 0;
}

/* Log a given device's VID:PID/product once, not on every 2s scan pass. */
static int ugen_already_logged(const char *path) {
    static char seen[MAX_UGEN_DEVS][32];
    static int  n_seen = 0;
    for (int i = 0; i < n_seen; i++) if (!strcmp(seen[i], path)) return 1;
    if (n_seen < MAX_UGEN_DEVS) { strncpy(seen[n_seen], path, 31); seen[n_seen][31] = 0; n_seen++; }
    return 0;
}

/* Read VID:PID. The PS5 kernel rejects USB_GET_DEVICEINFO (ENOTTY), so try
 * three routes: DEVICEINFO, the standard device descriptor ioctl, and a raw
 * GET_DESCRIPTOR control request. Logs which one worked (once per path). */
static int ugen_err_logged(const char *path, int code);
static int get_vid_pid(int fd, const char *path, uint16_t *vid, uint16_t *pid) {
    struct usb_device_info di; memset(&di,0,sizeof(di));
    if (ioctl(fd,USB_GET_DEVICEINFO,&di) == 0) {
        *vid = di.udi_vendorNo; *pid = di.udi_productNo; return 1;
    }
    int e1 = errno;

    struct usb_device_descriptor dd; memset(&dd,0,sizeof(dd));
    if (ioctl(fd,USB_GET_DEVICE_DESC,&dd) == 0 && dd.bLength >= 12) {
        *vid = UGETW(dd.idVendor); *pid = UGETW(dd.idProduct); return 2;
    }
    int e2 = errno;

    uint8_t raw[18]; memset(raw,0,sizeof(raw));
    struct usb_ctl_request req; memset(&req,0,sizeof(req));
    req.ucr_data = raw;
    req.ucr_request.bmRequestType = 0x80;          /* device-to-host, standard, device */
    req.ucr_request.bRequest      = 0x06;          /* GET_DESCRIPTOR */
    USETW(req.ucr_request.wValue,  0x0100);        /* DEVICE descriptor */
    USETW(req.ucr_request.wIndex,  0);
    USETW(req.ucr_request.wLength, sizeof(raw));
    if (ioctl(fd,USB_DO_REQUEST,&req) == 0 && raw[1] == 0x01) {
        *vid = (uint16_t)(raw[8]  | (raw[9]  << 8));
        *pid = (uint16_t)(raw[10] | (raw[11] << 8));
        return 3;
    }
    int e3 = errno;

    if (!ugen_err_logged(path, 2000 + e1 * 100 + e2 * 10 + e3))
        gp_log("probe: %s no VID/PID (deviceinfo errno=%d, device_desc errno=%d, ctrl_req errno=%d)\n",
               path, e1, e2, e3);
    return 0;
}

/* Probe one /dev/ugen* path to identify controller type.
 * Returns 1 with vid/pid set, 0 if not a known controller.
 *
 * USB_GET_DEVICEINFO is read right after open(), before USB_FS_INIT, so we
 * can identify (and log) a Valve device on ANY bus without needing the
 * endpoint-init dance. The endpoint-guessing fallback (Nintendo/Xbox) stays
 * restricted to ugen2.* — the external bus — so it can never misidentify
 * one of the PS5's own internal USB devices. */
static int probe_one_path(const char *path, uint16_t *out_vid, uint16_t *out_pid) {
    int fd = open(path, O_RDWR|O_NONBLOCK);
    if (fd < 0) {
        int e = errno;
        if (e != ENOENT && !ugen_err_logged(path, e))
            gp_log("probe: %s open failed errno=%d\n", path, e);
        return 0;
    }

    uint16_t vid = 0, pid = 0;
    int how = get_vid_pid(fd, path, &vid, &pid);
    int have_di = how != 0;

    if (have_di && !ugen_already_logged(path)) {
        gp_log("probe: %s VID=%04x PID=%04x (via %s)\n", path, vid, pid,
               how == 1 ? "deviceinfo" : how == 2 ? "device_desc" : "ctrl_req");
    }

    /* Exact VID:PID match first, any bus — endpoint heuristics below would
     * misread the Steam Controller's mouse endpoint (0x82) as an Xbox pad. */
    if (have_di && vid==VID_STEAM &&
        (pid==PID_STEAM_WIRED || pid==PID_SC2_PUCK ||
         pid==PID_SC2_WIRED)) {
        gp_log("probe: %s VID=%04x PID=%04x → Steam Controller\n",
               path,vid,pid);
        *out_vid=VID_STEAM; *out_pid=pid;
        close(fd);
        return 1;
    }

    /* Everything below only ever touches the external bus. */
    if (strncmp(path, "/dev/ugen2.", 11) != 0) { close(fd); return 0; }

    struct usb_fs_endpoint ep;
    struct usb_fs_init ini;
    struct usb_fs_uninit u;
    memset(&ep,0,sizeof(ep)); memset(&ini,0,sizeof(ini));
    ini.pEndpoints=&ep; ini.ep_index_max=1;
    if (ioctl(fd,USB_FS_INIT,&ini)!=0) { close(fd); return 0; }

    int ii=0; ioctl(fd,USB_IFACE_DRIVER_DETACH,&ii);
    ii=1;     ioctl(fd,USB_IFACE_DRIVER_DETACH,&ii);
    ii=2;     ioctl(fd,USB_IFACE_DRIVER_DETACH,&ii);

    struct usb_fs_open po;
    memset(&po,0,sizeof(po)); po.ep_index=0; po.max_bufsize=64; po.max_frames=1;

    int found = 0;

    /* Nintendo: ep=0x81, maxpkt=64 */
    po.ep_no=0x81;
    if (ioctl(fd,USB_FS_OPEN,&po)==0 && po.max_packet_length==64) {
        gp_log("probe: %s ep=0x81 mpkt=%u → Nintendo\n", path,(unsigned)po.max_packet_length);
        struct usb_fs_close pc; memset(&pc,0,sizeof(pc)); pc.ep_index=0; ioctl(fd,USB_FS_CLOSE,&pc);
        *out_vid=VID_SWITCH; *out_pid=PID_SWITCH;
        found = 1;
        goto done;
    }

    /* Xbox One: ep=0x82, maxpkt in (0,64] */
    memset(&po,0,sizeof(po)); po.ep_index=0; po.max_bufsize=64; po.max_frames=1;
    po.ep_no=0x82;
    if (ioctl(fd,USB_FS_OPEN,&po)==0 && po.max_packet_length>0 && po.max_packet_length<=64) {
        gp_log("probe: %s ep=0x82 mpkt=%u → Xbox One\n", path,(unsigned)po.max_packet_length);
        struct usb_fs_close pc; memset(&pc,0,sizeof(pc)); pc.ep_index=0; ioctl(fd,USB_FS_CLOSE,&pc);
        *out_vid=VID_XBOX; *out_pid=PID_XBOX;
        found = 1;
        goto done;
    }

done:
    memset(&u,0,sizeof(u)); ioctl(fd,USB_FS_UNINIT,&u);
    close(fd);
    return found;
}

/* ── Create VDA for a slot ────────────────────────────────────────────── */
static int32_t create_vda_for_slot(int slot, int *out_bind_ok) {
    struct { int32_t size; int32_t userId; int32_t pad[6]; } vdp;
    pthread_mutex_lock(&g_vda_create_lock);
    memset(&vdp,0,sizeof(vdp)); vdp.size=sizeof(vdp); vdp.userId=1;

    if (out_bind_ok) *out_bind_ok = 0;
    int ret = scePadVirtualDeviceAddDevice(&vdp, VIRTUAL_DEVICE_TYPE_DUALSENSE);
    gp_log("slot[%d] VDA ret=0x%08x\n", slot, (uint32_t)ret);

    /* Only a positive handle returned by this exact AddDevice call can be
     * attributed to this session. Output tokens and asynchronous klog records
     * cannot identify a deletable handle safely. */
    int32_t handle = (ret > 0) ? ret : -1;
    if (handle>=0)
        gp_log("slot[%d] VDI handle=0x%x ready\n", slot, (uint32_t)handle);
    else
        gp_log("slot[%d] ERROR: AddDevice did not return an attributable handle; refusing handle scan\n", slot);
    pthread_mutex_unlock(&g_vda_create_lock);
    return handle;
}

/* Steam Controller actually being used (not just awake): a button, or a
 * stick/trigger well past rest. Wakes the DualSense hand-off. */
static int sc2_pad_in_use(const ScePadData *p) {
    if (p->buttons) return 1;
    int v[4] = { p->leftStick.x, p->leftStick.y, p->rightStick.x, p->rightStick.y };
    for (int i = 0; i < 4; i++) if (v[i] < 128 - 40 || v[i] > 128 + 40) return 1;
    return p->analogButtons.l2 > 40 || p->analogButtons.r2 > 40;
}

typedef struct {
    int slot;
    const char *why;
} vda_delete_context_t;

static int delete_vda_handle(int32_t handle, void *opaque) {
    vda_delete_context_t *context = (vda_delete_context_t *)opaque;
    (void)neutralize_vda_handle(handle);
    int result = scePadVirtualDeviceDeleteDevice(handle);
    if (result != 0)
        gp_log("slot[%d] VDA delete handle=0x%x failed (%s), ret=0x%08x\n",
               context->slot, (uint32_t)handle, context->why, (uint32_t)result);
    return result;
}

static int delete_slot_vda(slot_registry_token_t token, int slot,
                           const char *why) {
    vda_delete_context_t context = { slot, why };
    return slot_registry_delete_virtual(&g_slots, token, delete_vda_handle,
                                        &context);
}

/* ── Lazy virtual pad for the Steam Controller (2026) ───────────────────
 * The Puck stays plugged in while the controller sleeps. Creating the
 * virtual DualSense only while the controller is actually on means the PS5
 * never waits on it (e.g. after rest mode) and a real DualSense keeps
 * working alongside it. */
static int sc2_vda_attach(slot_registry_token_t token, int slot,
                          usb_session_state_t *session) {
    slot_registry_entry_t entry;
    if (slot_registry_snapshot(&g_slots, token, &entry) != SLOT_REGISTRY_OK ||
        entry.state != SLOT_REGISTRY_ACTIVE)
        return 0;
    if (entry.virtual_ready) return 1;

    int bind_ok = 0;
    int32_t h = create_vda_for_slot(slot, &bind_ok);
    if (h < 0) { gp_log("slot[%d] SC2 virtual pad create failed\n", slot); return 0; }
    int result = slot_registry_set_virtual(&g_slots, token, h);
    if (result != SLOT_REGISTRY_OK) {
        /* A failed registration cannot prove whether this handle aliases a
         * device already owned by another slot. Never delete an ambiguous ID. */
        gp_log("slot[%d] SC2 virtual pad was not registered (result=%d); keeping handle untouched\n",
               slot, result);
        return 0;
    }
    session->bind_ok = bind_ok;
    session->last_vdi_ret = -1;
    session->inject_count = 0;
    notify("Puckbridge: Steam Controller connected");
    return 1;
}

static void sc2_vda_detach(slot_registry_token_t token, int slot,
                           usb_session_state_t *session,
                           sc2_assignment_token_t *assignment_token,
                           const char *why) {
    int result = delete_slot_vda(token, slot, why);
    if (result != SLOT_REGISTRY_OK) {
        gp_log("slot[%d] SC2 virtual pad detach deferred (registry result=%d)\n",
               slot, result);
        return;
    }
    if (session) {
        session->bind_ok = 0;
        session->last_vdi_ret = -1;
        session->inject_count = 0;
    }
    if (assignment_token && *assignment_token) {
        sc2_assignment_release(*assignment_token);
        *assignment_token = 0;
    }
    gp_log("slot[%d] SC2 virtual pad removed (%s)\n", slot, why);
}

static int slot_release(slot_registry_token_t token, const char *why) {
    vda_delete_context_t context = { (int)token.index, why };
    int result = slot_lifecycle_release(&g_slots, token, delete_vda_handle,
                                        &context);
    if (result != SLOT_REGISTRY_OK && result != SLOT_REGISTRY_STALE)
        gp_log("slot[%u] release failed (%s), registry result=%d\n",
               token.index, why, result);
    return result;
}

static void retry_releasing_slots(void) {
    for (uint32_t i = 0; i < MAX_SLOTS; i++) {
        slot_registry_entry_t entry;
        if (slot_registry_snapshot_index(&g_slots, i, &entry) == SLOT_REGISTRY_OK &&
            entry.state == SLOT_REGISTRY_RELEASING)
            slot_release(entry.token, "deferred VDA cleanup retry");
    }
}

/* ── USB HID thread ───────────────────────────────────────────────────── */
/* Single-session: receives slot+path+vid+pid, runs until disconnect, then exits.
 * Manager thread handles re-detection after exit. */

typedef struct {
    slot_registry_token_t slot_token;
    sc2_assignment_token_t assignment_token;
    int bind_ok;
} usb_thread_arg_t;

static void usb_worker_mark_done(slot_registry_token_t token) {
    if (token.index >= MAX_SLOTS) return;
    pthread_mutex_lock(&g_usb_workers_lock);
    usb_worker_record_t *record = &g_usb_workers[token.index];
    if (record->started &&
        record->token.generation == token.generation) {
        record->done = 1;
        pthread_cond_broadcast(&g_usb_workers_changed);
    }
    pthread_mutex_unlock(&g_usb_workers_lock);
}

static void reap_finished_usb_workers(void) {
    for (uint32_t i = 0; i < MAX_SLOTS; i++) {
        pthread_t thread;
        slot_registry_token_t token;
        pthread_mutex_lock(&g_usb_workers_lock);
        usb_worker_record_t *record = &g_usb_workers[i];
        int ready = record->started && record->done;
        if (ready) { thread = record->thread; token = record->token; }
        pthread_mutex_unlock(&g_usb_workers_lock);
        if (!ready) continue;
        int result = pthread_join(thread, NULL);
        if (result != 0) {
            gp_log("manager: completed USB worker join failed slot[%u] error=%d\n",
                   i, result);
            continue;
        }
        pthread_mutex_lock(&g_usb_workers_lock);
        record = &g_usb_workers[i];
        if (record->started && record->done &&
            record->token.generation == token.generation) {
            record->started = 0;
            record->done = 0;
        }
        pthread_mutex_unlock(&g_usb_workers_lock);
    }
}

static int join_all_usb_workers(void) {
    for (;;) {
        pthread_t threads[MAX_SLOTS];
        slot_registry_token_t tokens[MAX_SLOTS];
        uint32_t indices[MAX_SLOTS];
        uint32_t count = 0;

        pthread_mutex_lock(&g_usb_workers_lock);
        for (;;) {
            int pending = 0;
            for (uint32_t i = 0; i < MAX_SLOTS; i++)
                if (g_usb_workers[i].started && !g_usb_workers[i].done)
                    pending = 1;
            if (!pending) break;
            pthread_cond_wait(&g_usb_workers_changed, &g_usb_workers_lock);
        }
        for (uint32_t i = 0; i < MAX_SLOTS; i++) {
            if (!g_usb_workers[i].started) continue;
            threads[count] = g_usb_workers[i].thread;
            tokens[count] = g_usb_workers[i].token;
            indices[count++] = i;
        }
        pthread_mutex_unlock(&g_usb_workers_lock);

        if (count == 0) return 0;
        for (uint32_t i = 0; i < count; i++) {
            int result = pthread_join(threads[i], NULL);
            if (result != 0) {
                gp_log("shutdown: USB worker join failed slot[%u] generation=%u error=%d\n",
                       indices[i], tokens[i].generation, result);
                return -1;
            }
        }
        pthread_mutex_lock(&g_usb_workers_lock);
        for (uint32_t i = 0; i < count; i++) {
            usb_worker_record_t *record = &g_usb_workers[indices[i]];
            if (record->started &&
                record->token.generation == tokens[i].generation) {
                record->started = 0;
                record->done = 0;
            }
        }
        pthread_mutex_unlock(&g_usb_workers_lock);
    }
}

static int usb_platform_open(const char *path, int flags, void *context) {
    (void)context;
    return open(path, flags);
}

static int usb_platform_init_fs(int fd, void *configuration, void *context) {
    (void)context;
    return ioctl(fd, USB_FS_INIT, configuration);
}

static int usb_platform_open_endpoint(int fd, void *configuration,
                                      void *context) {
    (void)context;
    return ioctl(fd, USB_FS_OPEN, configuration);
}

static int usb_platform_start_transfer(int fd, void *configuration,
                                       void *context) {
    (void)context;
    return ioctl(fd, USB_FS_START, configuration);
}

static int usb_platform_complete_transfer(int fd, void *configuration,
                                          uint8_t *completed_endpoint_index,
                                          void *context) {
    (void)context;
    if (!completed_endpoint_index) {
        errno = EINVAL;
        return -1;
    }
    int result = ioctl(fd, USB_FS_COMPLETE, configuration);
    if (result == 0)
        *completed_endpoint_index =
            ((struct usb_fs_complete *)configuration)->ep_index;
    return result;
}

static int usb_platform_stop_transfer(int fd, uint8_t endpoint_index,
                                      void *context) {
    (void)context;
    struct usb_fs_stop stop;
    memset(&stop, 0, sizeof(stop));
    stop.ep_index = endpoint_index;
    return ioctl(fd, USB_FS_STOP, &stop);
}

static int usb_platform_close_endpoint(int fd, uint8_t endpoint_index,
                                       void *context) {
    (void)context;
    struct usb_fs_close close_endpoint;
    memset(&close_endpoint, 0, sizeof(close_endpoint));
    close_endpoint.ep_index = endpoint_index;
    return ioctl(fd, USB_FS_CLOSE, &close_endpoint);
}

static int usb_platform_uninit_fs(int fd, void *context) {
    (void)context;
    struct usb_fs_uninit uninit;
    memset(&uninit, 0, sizeof(uninit));
    return ioctl(fd, USB_FS_UNINIT, &uninit);
}

static int usb_platform_close_fd(int fd, void *context) {
    (void)context;
    return close(fd);
}

static const usb_session_ops_t g_usb_session_ops = {
    .open_device = usb_platform_open,
    .init_fs = usb_platform_init_fs,
    .open_endpoint = usb_platform_open_endpoint,
    .start_transfer = usb_platform_start_transfer,
    .complete_transfer = usb_platform_complete_transfer,
    .resources = {
        .stop_transfer = usb_platform_stop_transfer,
        .close_endpoint = usb_platform_close_endpoint,
        .uninit_fs = usb_platform_uninit_fs,
        .close_fd = usb_platform_close_fd
    }
};

static void *usb_hid_thread(void *arg) {
    usb_thread_arg_t *targ = (usb_thread_arg_t *)arg;
    slot_registry_token_t slot_token = targ->slot_token;
    sc2_assignment_token_t assignment_token = targ->assignment_token;
    usb_session_state_t session;
    memset(&session, 0, sizeof(session));
    session.bind_ok = targ->bind_ok;
    session.last_vdi_ret = -1;
    sc2_input_state_reset(&session.sc2_input);
    free(targ);

    slot_registry_entry_t slot_entry;
    if (slot_registry_snapshot(&g_slots, slot_token, &slot_entry) != SLOT_REGISTRY_OK) {
        sc2_haptic_session_abandon(slot_token);
        sc2_assignment_release(assignment_token);
        usb_worker_mark_done(slot_token);
        return NULL;
    }
    int slot = (int)slot_token.index;
    char dev_path[SLOT_REGISTRY_ID_MAX];
    memcpy(dev_path, slot_entry.identity, sizeof(dev_path));
    dev_path[sizeof(dev_path) - 1] = '\0';
    uint16_t vid = slot_entry.vendor_id, pid = slot_entry.product_id;

    struct usb_fs_endpoint eps[2 + SC2_HAP_MAX_OUT];
    sc2_ep_map_t sc2_map; memset(&sc2_map, 0, sizeof(sc2_map));
    int sc2_n_out = 0;
    struct usb_fs_init     init;
    struct usb_fs_open     fs_open;
    struct usb_fs_start    start;
    struct usb_fs_complete complete;
    uint8_t  buf[64];
    void    *buffers[1]; uint32_t lengths[1];
    int fd = -1, out_opened = 0, sc2_iface = 0;
    usb_resources_t usb_resources;
    usb_resources_init(&usb_resources);
    int usb_ready_notified = 0;

    gp_log("slot[%d] USB thread: %s VID=0x%04x PID=0x%04x\n",
           slot, dev_path, vid, pid);

    /* ── Xbox One: single-pass ─────────────────────────────────────────── */
    if (pid == PID_XBOX) {
        if (usb_session_open(&usb_resources, &g_usb_session_ops, dev_path,
                             O_RDWR, NULL) != 0) {
            gp_log("slot[%d] Xbox open fail errno=%d\n", slot, errno);
            goto exit_slot;
        }
        fd = usb_resources.fd;

        { int ii; for(ii=0;ii<4;ii++){int i2=ii; ioctl(fd,USB_IFACE_DRIVER_DETACH,&i2);} }
        usleep(120000); /* hub settle: give USB hub time to propagate detach */
        memset(eps,0,sizeof(eps)); memset(&init,0,sizeof(init));
        init.pEndpoints=eps; init.ep_index_max=4;
        if (usb_session_init_fs(&usb_resources, &g_usb_session_ops,
                                &init, NULL)!=0){
            gp_log("slot[%d] Xbox FS_INIT fail errno=%d\n",slot,errno);
            goto exit_slot;
        }

        memset(&fs_open,0,sizeof(fs_open));
        fs_open.ep_index=0; fs_open.ep_no=XBOX_EP_IN;
        fs_open.max_bufsize=64; fs_open.max_frames=1;
        if (usb_session_open_endpoint(&usb_resources, &g_usb_session_ops,
                                      0, &fs_open, NULL)!=0){
            gp_log("slot[%d] Xbox IN fail errno=%d\n",slot,errno);
            goto uninit_exit;
        }
        gp_log("slot[%d] Xbox IN ep=0x%02x ok maxpkt=%u\n",
               slot, XBOX_EP_IN, (unsigned)fs_open.max_packet_length);

        buffers[0]=buf; lengths[0]=64;
        eps[0].ppBuffer=buffers; eps[0].pLength=lengths; eps[0].nFrames=1;
        eps[0].timeout=50; eps[0].flags=USB_FS_FLAG_SINGLE_SHORT_OK|USB_FS_FLAG_MULTI_SHORT_OK;

        memset(&fs_open,0,sizeof(fs_open));
        fs_open.ep_index=1; fs_open.ep_no=XBOX_EP_OUT;
        fs_open.max_bufsize=64; fs_open.max_frames=1;
        out_opened = (usb_session_open_endpoint(&usb_resources,
            &g_usb_session_ops, 1, &fs_open, NULL)==0) ? 1 : 0;
        gp_log("slot[%d] Xbox OUT ep=0x%02x opened=%d\n", slot, XBOX_EP_OUT, out_opened);

        xbox_gip_handshake(&usb_resources, &g_usb_session_ops, eps);
        goto main_loop;
    }

    /* ── Steam Controller 2026 (Puck / cable) ──────────────────────────── */
    if (pid == PID_SC2_PUCK || pid == PID_SC2_WIRED) {
        int is_puck = (pid == PID_SC2_PUCK);
        if (usb_session_open(&usb_resources, &g_usb_session_ops, dev_path,
                             O_RDWR, NULL) != 0) {
            gp_log("slot[%d] SC2 open fail errno=%d\n", slot, errno);
            goto exit_slot;
        }
        fd = usb_resources.fd;

        uint8_t addrs[SC2_MAX_EPS];
        int map_status = sc2_ep_map(fd, is_puck, &sc2_map);
        int n_eps = 0;
        if (map_status == SC2_USB_MAP_UNAVAILABLE) {
            /* Preserve the known-device compatibility path only when the
             * kernel refuses GET_FULL_DESC. A parsed-but-empty or malformed
             * map must not be treated as permission to probe guessed endpoints. */
            gp_log("slot[%d] SC2 config descriptor unavailable, trying bounded IN eps 0x81-0x88\n", slot);
            for (int k = 0; k < SC2_MAX_EPS; k++) addrs[k] = (uint8_t)(0x81 + k);
            n_eps = SC2_MAX_EPS;
        } else if (map_status == SC2_USB_MAP_DEVICE_GONE) {
            gp_log("slot[%d] SC2 device disappeared during config read\n", slot);
            goto exit_slot;
        } else if (map_status < 0) {
            gp_log("slot[%d] SC2 config descriptor invalid; refusing endpoint scan\n", slot);
            goto exit_slot;
        } else {
            n_eps = sc2_collect_in_eps(&sc2_map, addrs, SC2_MAX_EPS);
            if (n_eps <= 0) {
                gp_log("slot[%d] SC2 config has no eligible HID IN endpoints\n", slot);
                goto exit_slot;
            }
        }

        { int ii; for(ii=0;ii<8;ii++){int i2=ii; ioctl(fd,USB_IFACE_DRIVER_DETACH,&i2);} }
        usleep(120000);

        struct usb_fs_endpoint sc2_eps[SC2_MAX_EPS];
        memset(sc2_eps,0,sizeof(sc2_eps)); memset(&init,0,sizeof(init));
        init.pEndpoints=sc2_eps; init.ep_index_max=SC2_MAX_EPS;
        if (usb_session_init_fs(&usb_resources, &g_usb_session_ops,
                                &init, NULL)!=0){
            gp_log("slot[%d] SC2 FS_INIT fail errno=%d\n",slot,errno);
            goto exit_slot;
        }

        /* Wait for the controller to wake / pick the puck slot it's paired to */
        int active = 0, told = 0;
        while ((active = sc2_find_active_ep(fd, &usb_resources,
                                             &g_usb_session_ops, NULL,
                                             sc2_eps, addrs, n_eps,
                                             2000, runtime_cancel_requested,
                                             NULL)) == 0 &&
               !runtime_is_stopping()) {
            if (!told) { gp_log("slot[%d] Puck idle — waiting for the controller to turn on\n", slot); told = 1; }
        }
        if (usb_resources.opened_endpoints != 0 ||
            usb_resources.active_transfers != 0) {
            gp_log("slot[%d] SC2 scan cleanup incomplete (opened=0x%x active=0x%x); retrying through session cleanup\n",
                   slot, usb_resources.opened_endpoints,
                   usb_resources.active_transfers);
            goto exit_slot;
        }
        if (usb_session_uninit_fs(&usb_resources, &g_usb_session_ops,
                                  NULL)!=0) {
            gp_log("slot[%d] SC2 scan FS_UNINIT fail errno=%d\n",slot,errno);
            goto exit_slot;
        }
        if (active < 0) {
            if (active == -ENXIO)
                gp_log("slot[%d] SC2 puck removed\n", slot);
            else
                gp_log("slot[%d] SC2 endpoint scan failed (%d)\n", slot,
                       active);
            goto exit_slot;
        }
        if (runtime_is_stopping()) goto exit_slot;
        if (!sc2_pause_is_active()) sc2_vda_attach(slot_token, slot, &session);
        ds_handoff_sc2_session(!sc2_pause_is_active());

        /* Re-init with the single active endpoint at index 0 for the main loop */
        memset(eps,0,sizeof(eps)); memset(&init,0,sizeof(init));
        init.pEndpoints=eps; init.ep_index_max=2 + SC2_HAP_MAX_OUT;
        if (usb_session_init_fs(&usb_resources, &g_usb_session_ops,
                                &init, NULL)!=0) goto exit_slot;
        memset(&fs_open,0,sizeof(fs_open));
        fs_open.ep_index=0; fs_open.ep_no=(uint8_t)active;
        fs_open.max_bufsize=64; fs_open.max_frames=1;
        if (usb_session_open_endpoint(&usb_resources, &g_usb_session_ops,
                                      0, &fs_open, NULL)!=0){
            gp_log("slot[%d] SC2 IN 0x%02x fail errno=%d\n",slot,active,errno);
            goto uninit_exit;
        }
        buffers[0]=buf; lengths[0]=64;
        eps[0].ppBuffer=buffers; eps[0].pLength=lengths; eps[0].nFrames=1;
        eps[0].timeout=200; /* finite timeout lets this USB owner observe shutdown */
        eps[0].flags=USB_FS_FLAG_SINGLE_SHORT_OK|USB_FS_FLAG_MULTI_SHORT_OK;

        /* Haptics: use the OUT endpoint that belongs to the SAME interface as
         * the active IN, read from the descriptor (not guessed). Other slot
         * interfaces' OUTs are opened too, for the broadcast test method. */
        {
            int k = sc2_iface_of_in(&sc2_map, (uint8_t)active);
            uint8_t own_out = 0;
            if (k >= 0) { sc2_iface = sc2_map.it[k].iface; own_out = sc2_map.it[k].out_ep; }
            else        { sc2_iface = active - 0x81; if (sc2_iface < 0) sc2_iface = 0; }
            out_opened = 0;
            if (own_out) {
                memset(&fs_open,0,sizeof(fs_open));
                fs_open.ep_index=1; fs_open.ep_no=own_out;
                fs_open.max_bufsize=64; fs_open.max_frames=1;
                out_opened = (usb_session_open_endpoint(&usb_resources,
                    &g_usb_session_ops, 1, &fs_open, NULL)==0) ? 1 : 0;
            }
            gp_log("slot[%d] SC2 haptics: active IN 0x%02x → iface %d, own OUT 0x%02x\n",
                   slot, active, sc2_iface, own_out);
            if (own_out) gp_log("slot[%d] SC2 haptics: OUT 0x%02x %s\n", slot, own_out,
                                out_opened ? "opened" : "failed to open");
            sc2_n_out = 0;
            for (int j = 0; j < sc2_map.n && sc2_n_out < SC2_HAP_MAX_OUT; j++) {
                uint8_t o = sc2_map.it[j].out_ep;
                if (!o || o == own_out) continue;
                if (is_puck && (sc2_map.it[j].iface < 2 || sc2_map.it[j].iface > 5)) continue;  /* skip pogo pin iface */
                memset(&fs_open,0,sizeof(fs_open));
                fs_open.ep_index=(uint8_t)(2 + sc2_n_out); fs_open.ep_no=o;
                fs_open.max_bufsize=64; fs_open.max_frames=1;
                if (usb_session_open_endpoint(&usb_resources,
                    &g_usb_session_ops, (uint8_t)(2 + sc2_n_out),
                    &fs_open, NULL)==0) {
                    sc2_n_out++;
                    gp_log("slot[%d] SC2 haptics: extra OUT 0x%02x (iface %d) for broadcast test\n",
                           slot, o, sc2_map.it[j].iface);
                }
            }
            sc2_status_set_haptics(1, out_opened ? own_out : -1);
        }
        goto main_loop;
    }

    /* ── Steam Controller: single-pass ─────────────────────────────────── */
    if (pid == PID_STEAM_WIRED) {
        if (usb_session_open(&usb_resources, &g_usb_session_ops, dev_path,
                             O_RDWR, NULL) != 0) {
            gp_log("slot[%d] Steam open fail errno=%d\n", slot, errno);
            goto exit_slot;
        }
        fd = usb_resources.fd;

        /* Detach PS5 HID from keyboard(0) / mouse(1) / gamepad(2) interfaces */
        { int ii; for(ii=0;ii<3;ii++){int i2=ii; ioctl(fd,USB_IFACE_DRIVER_DETACH,&i2);} }
        usleep(120000);

        /* Control transfers before FS mode takes over */
        if (steam_init(fd) != 0)
            notify("Puckbridge: Steam Controller init failed — see klog");

        memset(eps,0,sizeof(eps)); memset(&init,0,sizeof(init));
        init.pEndpoints=eps; init.ep_index_max=1;
        if (usb_session_init_fs(&usb_resources, &g_usb_session_ops,
                                &init, NULL)!=0){
            gp_log("slot[%d] Steam FS_INIT fail errno=%d\n",slot,errno);
            goto exit_slot;
        }
        memset(&fs_open,0,sizeof(fs_open));
        fs_open.ep_index=0; fs_open.ep_no=STEAM_EP_IN;
        fs_open.max_bufsize=64; fs_open.max_frames=1;
        if (usb_session_open_endpoint(&usb_resources, &g_usb_session_ops,
                                      0, &fs_open, NULL)!=0){
            gp_log("slot[%d] Steam IN ep=0x%02x fail errno=%d\n",slot,STEAM_EP_IN,errno);
            goto uninit_exit;
        }
        gp_log("slot[%d] Steam IN ep=0x%02x ok maxpkt=%u\n",
               slot, STEAM_EP_IN, (unsigned)fs_open.max_packet_length);

        buffers[0]=buf; lengths[0]=64;
        eps[0].ppBuffer=buffers; eps[0].pLength=lengths; eps[0].nFrames=1;
        eps[0].timeout=50; eps[0].flags=USB_FS_FLAG_SINGLE_SHORT_OK|USB_FS_FLAG_MULTI_SHORT_OK;
        out_opened = 0;   /* no interrupt OUT; config goes over ep0 */
        goto main_loop;
    }

    /* ── Nintendo: two-pass ────────────────────────────────────────────── */
    if (usb_session_open(&usb_resources, &g_usb_session_ops, dev_path,
                         O_RDWR, NULL) != 0) {
        gp_log("slot[%d] open fail errno=%d\n", slot, errno);
        goto exit_slot;
    }
    fd = usb_resources.fd;

    memset(eps,0,sizeof(eps)); memset(&init,0,sizeof(init));
    init.pEndpoints=eps; init.ep_index_max=1;
    if (usb_session_init_fs(&usb_resources, &g_usb_session_ops,
                            &init, NULL)!=0){
        gp_log("slot[%d] Nintendo FS_INIT p1 fail\n",slot); goto exit_slot;
    }
    { int i0=0,i1=1; ioctl(fd,USB_IFACE_DRIVER_DETACH,&i0); ioctl(fd,USB_IFACE_DRIVER_DETACH,&i1); }
    memset(&fs_open,0,sizeof(fs_open));
    fs_open.ep_index=0; fs_open.ep_no=0x81; fs_open.max_bufsize=64; fs_open.max_frames=1;
    if (usb_session_open_endpoint(&usb_resources, &g_usb_session_ops,
                                  0, &fs_open, NULL)!=0){
        gp_log("slot[%d] Nintendo IN p1 fail errno=%d\n",slot,errno); goto uninit_exit;
    }
    gp_log("slot[%d] Nintendo p1 IN ok maxpkt=%u\n",slot,(unsigned)fs_open.max_packet_length);
    {
        int cleanup_result = usb_session_cleanup(&usb_resources,
            &g_usb_session_ops, NULL);
        fd = -1;
        if (cleanup_result != 0) {
            gp_log("slot[%d] Nintendo p1 cleanup reported an error\n", slot);
            goto exit_slot;
        }
    }

    /* Pass 2: retry DETACH+OPEN up to 5 times to beat usb_hid0 re-attach
     * (real Switch Pro is claimed by PS5 native HID driver after probe releases it) */
    { int detach_try;
      for (detach_try = 0; detach_try < 5; detach_try++) {
        if (usb_session_open(&usb_resources, &g_usb_session_ops, dev_path,
                             O_RDWR, NULL) != 0) {
            gp_log("slot[%d] reopen fail attempt %d\n",slot,detach_try);
            goto exit_slot;
        }
        fd = usb_resources.fd;
        { int i0=0,i1=1,i2=2;
          ioctl(fd,USB_IFACE_DRIVER_DETACH,&i0);
          ioctl(fd,USB_IFACE_DRIVER_DETACH,&i1);
          ioctl(fd,USB_IFACE_DRIVER_DETACH,&i2); }
        memset(eps,0,sizeof(eps)); memset(&init,0,sizeof(init));
        init.pEndpoints=eps; init.ep_index_max=2;
        if (usb_session_init_fs(&usb_resources, &g_usb_session_ops,
                                &init, NULL)!=0){
            int cleanup_result = usb_session_cleanup(&usb_resources,
                &g_usb_session_ops, NULL);
            fd = -1;
            if (cleanup_result != 0) goto exit_slot;
            usleep(50000); continue;
        }
        memset(&fs_open,0,sizeof(fs_open));
        fs_open.ep_index=0; fs_open.ep_no=0x81; fs_open.max_bufsize=64; fs_open.max_frames=1;
        if (usb_session_open_endpoint(&usb_resources, &g_usb_session_ops,
                                      0, &fs_open, NULL)==0) break; /* claimed it */
        int cleanup_result = usb_session_cleanup(&usb_resources,
            &g_usb_session_ops, NULL);
        fd = -1;
        if (cleanup_result != 0) goto exit_slot;
        gp_log("slot[%d] Nintendo p2 OPEN retry %d errno=%d\n",slot,detach_try,errno);
        usleep(50000);
      }
      if (fd < 0) { gp_log("slot[%d] Nintendo p2 give up\n",slot); goto exit_slot; }
    }
    gp_log("slot[%d] Nintendo p2 IN ok maxpkt=%u\n",slot,(unsigned)fs_open.max_packet_length);
    if (fs_open.max_packet_length != 64){ gp_log("slot[%d] wrong maxpkt, reinit\n",slot); goto reinit; }

    buffers[0]=buf; lengths[0]=64;
    eps[0].ppBuffer=buffers; eps[0].pLength=lengths; eps[0].nFrames=1;
    eps[0].timeout=200; eps[0].flags=USB_FS_FLAG_SINGLE_SHORT_OK|USB_FS_FLAG_MULTI_SHORT_OK;

    /* 8BitDo uses ep=0x02; real Nintendo Switch Pro Controller uses ep=0x01 */
    memset(&fs_open,0,sizeof(fs_open));
    fs_open.ep_index=1; fs_open.ep_no=0x02; fs_open.max_bufsize=64; fs_open.max_frames=1;
    out_opened = (usb_session_open_endpoint(&usb_resources,
        &g_usb_session_ops, 1, &fs_open, NULL)==0) ? 1 : 0;
    if (!out_opened) {
        memset(&fs_open,0,sizeof(fs_open));
        fs_open.ep_index=1; fs_open.ep_no=0x01; fs_open.max_bufsize=64; fs_open.max_frames=1;
        out_opened = (usb_session_open_endpoint(&usb_resources,
            &g_usb_session_ops, 1, &fs_open, NULL)==0) ? 1 : 0;
        if (out_opened) gp_log("slot[%d] Nintendo OUT ep=0x01 (real Switch Pro)\n", slot);
    }
    gp_log("slot[%d] Nintendo OUT opened=%d\n", slot, out_opened);
    if (out_opened) {
        (void)usb_send_cmd(&usb_resources, &g_usb_session_ops, &eps[1], 1,
                           0x80, 0x02, 50000, 20, NULL); usleep(30000);
        (void)usb_send_cmd(&usb_resources, &g_usb_session_ops, &eps[1], 1,
                           0x80, 0x04, 50000, 20, NULL); usleep(50000);
        gp_log("slot[%d] Nintendo [80 02]+[80 04] sent\n", slot);
    }

main_loop: ;
    int hs_state = (pid==PID_XBOX || pid==PID_STEAM_WIRED || pid==PID_SC2_PUCK || pid==PID_SC2_WIRED) ? HS_STREAMING : HS_WAIT_81_01;
    int sc2_link = 1;
    /* DualSense took back over after a hand-off: the Steam Controller is
     * parked (no virtual pad, hooks see it as off) until its Steam button is
     * pressed and released, or it's turned off and on. Stage: 0 wait for the
     * Steam button to be up, 1 wait for a press, 2 wait for its release. */
    int sc2_parked = 0, sc2_park_stage = 0, sc2_pause_seen = sc2_pause_is_active();
    uint32_t steam_pkts = 0;
    uint8_t nintendo_seq = 1;

    while (!runtime_is_stopping()) {
        memset(buf,0,64);
        buffers[0]=buf; lengths[0]=64;
        eps[0].ppBuffer=buffers; eps[0].pLength=lengths;
        eps[0].aFrames=0; eps[0].status=0;

        memset(&start,0,sizeof(start)); start.ep_index=0;
        if (usb_session_start_transfer(&usb_resources, &g_usb_session_ops,
                                       0, &start, NULL)!=0) {
            if (errno==EBUSY){
                (void)usb_session_stop_transfer(&usb_resources,
                    &g_usb_session_ops, 0, NULL);
                usleep(5000);
            } else if (errno==ENXIO||errno==ENOTTY){
                gp_log("slot[%d] START errno=%d — device gone\n",slot,errno); goto reinit;
            } else {
                gp_log("slot[%d] START fatal errno=%d\n",slot,errno); goto reinit;
            }
            continue;
        }

        int ok=0, cerr=0, cw=0;
        for(cw=0;cw<60 && !runtime_is_stopping();cw++){
            memset(&complete,0,sizeof(complete));
            uint8_t completed_endpoint = UINT8_MAX;
            int complete_result = usb_session_complete_transfer(
                &usb_resources, &g_usb_session_ops, 0, &complete,
                &completed_endpoint, NULL);
            if(complete_result == USB_SESSION_OK && completed_endpoint == 0){
                ok=1;break;
            }
            if(complete_result == USB_SESSION_COMPLETED_OTHER) continue;
            cerr=errno;
            if(cerr==ENXIO||cerr==ENOTTY){gp_log("slot[%d] COMPLETE errno=%d — gone\n",slot,cerr);goto reinit;}
            if(cerr!=EBUSY) break;
            usleep(500);
        }
        if (runtime_is_stopping()) goto reinit;
        if(!ok){
            (void)usb_session_stop_transfer(&usb_resources,
                &g_usb_session_ops, 0, NULL);
            continue;
        }
        if(lengths[0]<1) continue;

        uint32_t len = lengths[0];

        ScePadData pad; memset(&pad,0,sizeof(pad)); pad.quat.w=1.0f;
        int injected = 0;

        if (pid == PID_XBOX) {
            injected = xbox_handle_packet(&usb_resources, &g_usb_session_ops,
                                          eps, buf, len, &pad);
        } else if (pid == PID_SC2_PUCK || pid == PID_SC2_WIRED) {
            int was = sc2_link;
            injected = sc2_handle_packet(buf, len, &pad, &sc2_link,
                                         &session.sc2_input);
            int paused_now = sc2_pause_is_active();
            if (paused_now != sc2_pause_seen) {             /* paused or resumed (hold … 5 s) */
                sc2_pause_seen = paused_now;
                if (paused_now) ds_handoff_sc2_session(0);
                else if (sc2_link && !sc2_parked) ds_handoff_sc2_session(1);
            }
            if (!was && sc2_link) { sc2_parked = 0; ds_handoff_sc2_session(!sc2_pause_is_active()); }   /* woke up */

            /* DualSense came back after a hand-off: it takes over. Ask the Steam
             * Controller to power off, and park it whether or not it does. */
            if (ds_handoff_take_release() && !sc2_parked && !sc2_pause_is_active()) {
                sc2_parked = 1; sc2_park_stage = 0;
                sc2_power_off(fd, sc2_iface);
                notify("Puckbridge: DualSense in control. Press the Steam button to switch back.");
            }
            if (sc2_pause_is_active() || sc2_parked) {
                /* Paused (hold … 5 s) or the DualSense is in control: release
                 * everything and remove the virtual pad; the hooks see it as
                 * off. Reports are still read to catch the resume / Steam button. */
                game_hooks_feed(NULL, 0);
                sc2_haptic_service(slot_token, &usb_resources,
                    &g_usb_session_ops, eps, out_opened, sc2_iface, sc2_n_out);
                sc2_gyro_service(fd, sc2_iface);
                if (slot_virtual_ready(slot_token)) {
                    ScePadData rel; memset(&rel, 0, sizeof(rel));
                    rel.leftStick.x = rel.leftStick.y = rel.rightStick.x = rel.rightStick.y = 128;
                    rel.connected = 1; rel.quat.w = 1.0f;
                    inject_pad(slot_token, slot, &session, &rel); /* release everything first */
                    sc2_vda_detach(slot_token, slot, &session, &assignment_token,
                                   sc2_pause_is_active() ? "paused" : "DualSense in control");
                }
                if (was && !sc2_link) ds_handoff_sc2_session(0);   /* it did power off */
                sc2_status_snapshot_t status;
                sc2_status_snapshot(&status);
                int steam = sc2_link && (status.live_inputs & (1u << IN_STEAM)) != 0;
                if (sc2_pause_is_active()) ;                    /* only the … hold resumes */
                else if (sc2_park_stage == 0 && !steam) sc2_park_stage = 1;
                else if (sc2_park_stage == 1 &&  steam) sc2_park_stage = 2;
                else if (sc2_park_stage == 2 && !steam) {
                    sc2_parked = 0;
                    ds_handoff_sc2_session(1);                  /* hand-off armed again */
                    gp_log("slot[%d] Steam button: Steam Controller takes control back\n", slot);
                }
                injected = 0;
            } else {
                /* Only real input reports reach the game. Battery/status and
                 * lizard-mode reports leave `pad` zeroed, which reads as a stick
                 * pushed fully up-left. On disconnect nothing is fed and the
                 * bridge is disabled (conn = 0), so native input resumes. */
                /* Only hand off once the Steam Controller really works on its own:
                 * its virtual pad is bound to the user and input is going in. In a
                 * game with merged input it plays through the DualSense, so never. */
                if (injected && sc2_link && sc2_pad_in_use(&pad) &&
                    slot_virtual_ready(slot_token) && session.bind_ok &&
                    session.last_vdi_ret == 0 && !game_hooks_input_active())
                    ds_handoff_sc2_used();
                if (injected) game_hooks_feed(&pad, sc2_link);
                else if (!sc2_link) game_hooks_feed(NULL, 0);
                sc2_haptic_service(slot_token, &usb_resources,
                    &g_usb_session_ops, eps, out_opened, sc2_iface, sc2_n_out);
                sc2_gyro_service(fd, sc2_iface);
                if (was && !sc2_link) {
                    ds_handoff_sc2_session(0);
                    inject_pad(slot_token, slot, &session, &pad); /* release everything first */
                    sc2_vda_detach(slot_token, slot, &session, &assignment_token,
                                   "controller asleep / out of range");
                    notify("Puckbridge: Steam Controller disconnected");
                    injected = 0;
                }
                /* In a hooked game the Steam Controller is merged straight into the
                 * player's controller reads, so the virtual pad is removed there
                 * (it would only take rumble away from the DualSense). It comes
                 * back for the home screen and games that couldn't be hooked. */
                if (game_hooks_input_active()) {
                    if (slot_virtual_ready(slot_token))
                        sc2_vda_detach(slot_token, slot, &session, &assignment_token,
                                       "game uses merged input");
                    injected = 0;
                } else if (sc2_link && !slot_virtual_ready(slot_token))
                    sc2_vda_attach(slot_token, slot, &session);
            }
        } else if (pid == PID_STEAM_WIRED) {
            injected = steam_handle_packet(buf, len, &pad, &session.steam_input);
            if ((++steam_pkts % 1250u) == 0) steam_keepalive(fd);   /* ~every 5s */
        } else {
            injected = nintendo_handle_packet(&usb_resources,
                &g_usb_session_ops, eps, buf, len, &hs_state, &nintendo_seq,
                &pad);
        }

        if (injected > 0) {
            if (!usb_ready_notified) {
                notify("Puckbridge: slot[%d] streaming — controller active", slot);
                usb_ready_notified = 1;
            }
            /* First real button press confirms the assignment — release the gate
             * so the manager can start the next controller's dialog. */
            if (pad.buttons != 0 &&
                slot_registry_mark_confirmed(&g_slots, slot_token) == 1) {
                if (assignment_token) sc2_assignment_release(assignment_token);
                assignment_token = 0;
                gp_log("slot[%d] assignment confirmed (button press)\n", slot);
            }
            inject_pad(slot_token, slot, &session, &pad);
        }
    }

reinit:
    if (usb_ready_notified) { notify("Puckbridge: slot[%d] controller disconnected", slot); usb_ready_notified=0; }
    if (pid == PID_SC2_PUCK || pid == PID_SC2_WIRED) { sc2_status_set_input(0, 0); sc2_status_set_haptics(0, -1); sc2_menu_request_close(); sc2_gyro_reset(); sc2_battery_reset(); ds_handoff_sc2_session(0); }
    goto cleanup_usb;
uninit_exit:
cleanup_usb:
    if (pid == PID_SC2_PUCK || pid == PID_SC2_WIRED)
        (void)sc2_haptic_session_release(slot_token, &usb_resources,
                                         &g_usb_session_ops, eps,
                                         out_opened, sc2_iface, sc2_n_out);
    if (usb_resources.fd >= 0) {
        int cleanup_result = usb_session_cleanup(&usb_resources,
            &g_usb_session_ops, NULL);
        if (cleanup_result != 0)
            gp_log("slot[%d] USB resource cleanup reported %d\n",
                   slot, cleanup_result);
    }
    fd = -1;

exit_slot:
    if (pid == PID_SC2_PUCK || pid == PID_SC2_WIRED)
        sc2_haptic_session_abandon(slot_token);
    if (usb_resources.fd >= 0) {
        int cleanup_result = usb_session_cleanup(&usb_resources,
            &g_usb_session_ops, NULL);
        if (cleanup_result != 0)
            gp_log("slot[%d] USB resource cleanup reported %d\n",
                   slot, cleanup_result);
    }
    gp_log("slot[%d] USB thread exiting — freeing slot\n", slot);
    sc2_assignment_release(assignment_token);
    sc2_input_state_reset(&session.sc2_input);
    slot_release(slot_token, "USB reader exit");
    usb_worker_mark_done(slot_token);
    return NULL;
}

typedef struct {
    vda_delete_context_t deletion;
    sc2_assignment_token_t assignment_token;
    int32_t virtual_handle;
} manager_start_context_t;

static int manager_create_virtual(uint32_t slot, int32_t *out_handle,
                                  int *out_bind_ok, void *opaque) {
    manager_start_context_t *context = (manager_start_context_t *)opaque;
    int bind_ok = 0;
    int32_t handle = create_vda_for_slot((int)slot, &bind_ok);
    context->virtual_handle = handle;
    *out_handle = handle;
    *out_bind_ok = bind_ok;
    return handle < 0 ? -1 : 0;
}

static int manager_start_usb_worker(slot_registry_token_t token,
                                    int32_t virtual_handle, int bind_ok,
                                    void *opaque) {
    manager_start_context_t *context = (manager_start_context_t *)opaque;
    usb_thread_arg_t *arg = malloc(sizeof(*arg));
    if (!arg) {
        gp_log("manager: malloc fail for slot[%u]\n", token.index);
        return -1;
    }
    arg->slot_token = token;
    arg->assignment_token = context->assignment_token;
    arg->bind_ok = bind_ok;

    if (token.index >= MAX_SLOTS || runtime_is_stopping()) {
        free(arg);
        return -1;
    }

    pthread_mutex_lock(&g_usb_workers_lock);
    usb_worker_record_t *record = &g_usb_workers[token.index];
    if (record->started) {
        if (!record->done) {
            pthread_mutex_unlock(&g_usb_workers_lock);
            gp_log("manager: prior USB worker still owns slot[%u]\n", token.index);
            free(arg);
            return -1;
        }
        pthread_t previous = record->thread;
        record->started = 0;
        pthread_mutex_unlock(&g_usb_workers_lock);
        int join_result = pthread_join(previous, NULL);
        if (join_result != 0) {
            gp_log("manager: completed USB worker join failed slot[%u] error=%d\n",
                   token.index, join_result);
            free(arg);
            return -1;
        }
        pthread_mutex_lock(&g_usb_workers_lock);
        record = &g_usb_workers[token.index];
    }
    if (runtime_is_stopping()) {
        pthread_mutex_unlock(&g_usb_workers_lock);
        free(arg);
        return -1;
    }
    record->token = token;
    record->started = 1;
    record->done = 0;
    int create_result = pthread_create(&record->thread, NULL, usb_hid_thread, arg);
    if (create_result != 0) {
        record->started = 0;
        record->done = 0;
        pthread_mutex_unlock(&g_usb_workers_lock);
        gp_log("manager: pthread_create fail slot[%u] error=%d\n",
               token.index, create_result);
        free(arg);
        return -1;
    }
    pthread_mutex_unlock(&g_usb_workers_lock);
    gp_log("manager: slot[%u] USB thread started handle=0x%x\n",
           token.index, (uint32_t)virtual_handle);
    notify("Puckbridge: slot[%u] ready — press a button to assign", token.index);
    return 0;
}

/* ── Controller manager thread ────────────────────────────────────────── */
static void *controller_manager_thread(void *arg) {
    (void)arg;
    int scan = 0;
    gp_log("Manager thread started (MAX_SLOTS=%d)\n", MAX_SLOTS);

    while (!runtime_is_stopping()) {
        reap_finished_usb_workers();
        if (runtime_is_stopping()) break;
        static char devs[MAX_UGEN_DEVS][32];
        int n_devs = scan_ugen_devices(devs, MAX_UGEN_DEVS);
        retry_releasing_slots();

        for (int i = 0; i < n_devs; i++) {
            if (runtime_is_stopping()) break;
            const char *path = devs[i];

            /* Serialize assignment: if a controller is still awaiting the user's
             * button press to confirm its dialog, do not start another one. */
            if (sc2_assignment_owner() != 0) break;

            /* Identity is unique only for this live USB session. The registry
             * owns both duplicate detection and free-slot selection atomically. */
            uint32_t existing_slot;
            if (slot_registry_find_identity(&g_slots, path, &existing_slot) ==
                SLOT_REGISTRY_OK)
                continue;

            /* Try to identify controller */
            uint16_t vid=0, pid=0;
            if (!probe_one_path(path, &vid, &pid)) continue;

            slot_registry_token_t slot_token;
            int claim_result = slot_registry_claim(&g_slots, path, vid, pid,
                                                  &slot_token);
            if (claim_result != SLOT_REGISTRY_OK) {
                if (claim_result == SLOT_REGISTRY_FULL && (scan % 5) == 0)
                    gp_log("manager: all %d slots full\n", MAX_SLOTS);
                continue;
            }
            int slot = (int)slot_token.index;

            const char *name =
                (vid==VID_SWITCH && pid==PID_SWITCH) ? "Nintendo Switch Pro / 8BitDo" :
                (vid==VID_NATIVE && pid==PID_NATIVE) ? "8BitDo Native" :
                (vid==VID_XBOX   && pid==PID_XBOX)   ? "Xbox One S" :
                (vid==VID_STEAM  && pid==PID_STEAM_WIRED) ? "Steam Controller (2015)" :
                (vid==VID_STEAM  && pid==PID_SC2_PUCK)    ? "Steam Controller Puck" :
                (vid==VID_STEAM  && pid==PID_SC2_WIRED)   ? "Steam Controller (2026)" : "Unknown";

            gp_log("manager: %s at %s → slot[%d]\n", name, path, slot);
            int lazy = (vid==VID_STEAM && (pid==PID_SC2_PUCK || pid==PID_SC2_WIRED));
            if (!lazy) notify("Puckbridge: %s detected — assign user on screen", name);

            int owns_sc2_haptics = 0;
            if (pid == PID_SC2_PUCK || pid == PID_SC2_WIRED) {
                if (!sc2_haptic_session_claim(slot_token)) {
                    if ((scan % 5) == 0)
                        gp_log("manager: Steam Controller at %s skipped; slot[%u] already owns the single SC2 session\n",
                               path, slot_token.index);
                    slot_release(slot_token, "another Steam Controller session owns haptics");
                    continue;
                }
                owns_sc2_haptics = 1;
            }

            /* The assignment lease serializes the PS5 dialog. Slot ownership
             * remains generation-tagged even if that dialog times out. */
            sc2_assignment_token_t assignment_token = 0;
            if (!lazy) {
                assignment_token = sc2_assignment_claim(slot);
                if (assignment_token == 0) {
                    gp_log("manager: assignment lease unavailable for slot[%d]\n", slot);
                    slot_release(slot_token, "assignment lease unavailable");
                    continue;
                }
            }

            manager_start_context_t start_context = {
                { slot, "manager startup rollback" }, assignment_token, -1
            };
            slot_lifecycle_ops_t start_ops = {
                manager_create_virtual,
                manager_start_usb_worker,
                delete_vda_handle
            };
            int cleanup_result = SLOT_REGISTRY_OK;
            int start_result = slot_lifecycle_start(
                &g_slots, slot_token, !lazy, &start_ops, &start_context,
                &cleanup_result);
            if (start_result != SLOT_LIFECYCLE_OK) {
                if (owns_sc2_haptics)
                    sc2_haptic_session_abandon(slot_token);
                if (assignment_token)
                    sc2_assignment_release(assignment_token);
                if (start_result == SLOT_LIFECYCLE_VDA_ATTACH_FAILED) {
                    gp_log("manager: slot[%d] VDA registration failed (handle=0x%x); not deleting an unowned/ambiguous handle\n",
                           slot, (uint32_t)start_context.virtual_handle);
                } else {
                    gp_log("manager: slot[%d] startup transaction failed (stage=%d, cleanup=%d)\n",
                           slot, start_result, cleanup_result);
                }
                if (cleanup_result != SLOT_REGISTRY_OK &&
                    cleanup_result != SLOT_REGISTRY_STALE)
                    gp_log("manager: slot[%d] startup cleanup deferred (registry result=%d)\n",
                           slot, cleanup_result);
            }
            /* One controller per scan pass — assignment gate blocks the rest
             * until the user confirms this one with a button press. */
            break;
        }

        /* Assignment timeout: if the user never presses a button, release the
         * gate after ~30s so the queue does not stall forever. */
        static int assign_wait = 0;
        static sc2_assignment_token_t assign_wait_token = 0;
        sc2_assignment_token_t current_assignment = sc2_assignment_owner();
        if (current_assignment != 0) {
            if (current_assignment != assign_wait_token) {
                assign_wait_token = current_assignment;
                assign_wait = 0;
            }
            if (++assign_wait > 15) {  /* 15 * 2s = 30s */
                int assign_slot = sc2_assignment_slot(current_assignment);
                if (sc2_assignment_release(current_assignment))
                    gp_log("manager: assignment timeout slot[%d] — releasing gate\n", assign_slot);
                assign_wait = 0;
                assign_wait_token = 0;
            }
        } else {
            assign_wait = 0;
            assign_wait_token = 0;
        }

        scan++;
        if ((scan % 5) == 0) {
            /* Log active slots every 10s */
            uint32_t active = slot_registry_count_claimed(&g_slots);
            if (active == 0 && (scan % 10) == 0)
                gp_log("manager: scan #%d — no controllers\n", scan);
        }
        runtime_sleep_ms(2000);
    }
    gp_log("manager: stopping scan; waiting for USB owners to clean up\n");
    if (join_all_usb_workers() != 0)
        gp_log("manager: one or more USB workers could not be joined\n");
    return NULL;
}

/* ── Credential elevation ─────────────────────────────────────────────── */
static void elevate_credentials(void) {
    pid_t p = getpid();
    uint8_t caps[16]; memset(caps,0xff,sizeof(caps));
    kernel_set_ucred_authid(p, 0x3800000000010003l);
    kernel_set_ucred_caps(p, caps);
}

static int release_owned_slots(void) {
    for (int attempt = 0; attempt < 5; attempt++) {
        for (uint32_t i = 0; i < MAX_SLOTS; i++) {
            slot_registry_entry_t entry;
            if (slot_registry_snapshot_index(&g_slots, i, &entry) !=
                SLOT_REGISTRY_OK)
                continue;
            int result = slot_release(entry.token, "central runtime shutdown");
            if (result != SLOT_REGISTRY_OK && result != SLOT_REGISTRY_STALE)
                gp_log("shutdown: owned slot[%u] rollback retry %d returned %d\n",
                       i, attempt + 1, result);
        }
        if (slot_registry_count_claimed(&g_slots) == 0) return 0;
        usleep(250000);
    }
    return -1;
}

static void remove_pid_file_if_owned(void) {
    char contents[32] = {0};
    int fd = open(PID_PATH, O_RDONLY);
    if (fd < 0) return;
    ssize_t length = read(fd, contents, sizeof(contents) - 1);
    close(fd);
    if (length > 0 && (pid_t)atoi(contents) == getpid())
        (void)unlink(PID_PATH);
}

static int runtime_shutdown(void) {
    int safe_to_release = 1;
    if (g_runtime_shutdown_done) return 0;

    /* Close the only worker that can disconnect a physical DualSense before
     * the shared stop request. The module gate prevents new hand-off calls. */
    if (ds_handoff_stop() != 0) {
        gp_log("shutdown: DualSense hand-off worker join failed\n");
        safe_to_release = 0;
    }
    runtime_request_stop();

    /* Stop request producers and one-shot system workers before USB teardown. */
    if (webui_stop() != 0) {
        gp_log("shutdown: WebUI listener join failed\n");
        safe_to_release = 0;
    }
    if (bridge_probe_stop() != 0) {
        gp_log("shutdown: read-only bridge probe join failed\n");
        safe_to_release = 0;
    }

    /* The manager joins each reader only after its owner stopped USB transfers,
     * released haptics, closed endpoints and file descriptors, and neutralized
     * / deleted its registered virtual device. */
    if (g_manager_started) {
        int result = pthread_join(g_manager_tid, NULL);
        if (result == 0) g_manager_started = 0;
        else {
            gp_log("shutdown: manager join failed error=%d\n", result);
            safe_to_release = 0;
        }
    }
    if (!g_manager_started && join_all_usb_workers() != 0)
        safe_to_release = 0;

    /* Game hooks own remote import changes and must complete their rollback
     * before their status or shared state is allowed to go away. */
    if (game_hooks_stop() != 0) {
        gp_log("shutdown: game-hook worker join / rollback failed\n");
        safe_to_release = 0;
    }
    if (sc2_select_stop() != 0) {
        gp_log("shutdown: profile watcher join failed\n");
        safe_to_release = 0;
    }
    if (g_klog_started) {
        int result = pthread_join(g_klog_tid, NULL);
        if (result == 0) g_klog_started = 0;
        else {
            gp_log("shutdown: klog capture join failed error=%d\n", result);
            safe_to_release = 0;
        }
    }

    if (!safe_to_release) {
        gp_log("shutdown: retaining shared state and registry until all owners join\n");
        return -1;
    }
    if (release_owned_slots() != 0) {
        gp_log("shutdown: virtual-device rollback still pending; registry retained\n");
        return -1;
    }

    slot_registry_destroy(&g_slots);
    remove_pid_file_if_owned();
    gp_log("shutdown: all owned devices removed and workers joined\n");
    pthread_mutex_lock(&g_log_lock);
    if (g_log_fd >= 0) { close(g_log_fd); g_log_fd = -1; }
    pthread_mutex_unlock(&g_log_lock);
    g_runtime_shutdown_done = 1;
    return 0;
}

/* ── main ─────────────────────────────────────────────────────────────── */
int main(void) {
    int32_t userId=-1, fgUser=-1; int ret;
    int startup_result = 0;
    struct sigaction stop_action;

    runtime_control_reset();
    memset(&stop_action, 0, sizeof(stop_action));
    stop_action.sa_handler = signal_stop_handler;
    sigemptyset(&stop_action.sa_mask);

    ghostpad_status_log_reset();
    if (slot_registry_init(&g_slots) != SLOT_REGISTRY_OK) {
        gp_log("slot registry initialization failed\n");
        return 1;
    }
    if (sigaction(SIGTERM, &stop_action, NULL) != 0 ||
        sigaction(SIGINT, &stop_action, NULL) != 0) {
        gp_log("signal handler installation failed errno=%d\n", errno);
        slot_registry_destroy(&g_slots);
        return 1;
    }
    gp_log("Ghost-Control v5 starting — %d slots\n", MAX_SLOTS);
    notify("PuckbridgePS5 (built on Ghostcontrol by StonedModder) — plug in controllers now");

    /* Kill previous instance */
    { int pfd=open(PID_PATH,O_RDONLY);
      if(pfd>=0){char pb[16]={0};read(pfd,pb,15);close(pfd);
        pid_t old=(pid_t)atoi(pb);
        if(old>0&&old!=getpid()){gp_log("Killing prev pid=%d\n",old);kill(old,SIGTERM);usleep(600000);}
      }
      int pfd2=open(PID_PATH,O_WRONLY|O_CREAT|O_TRUNC,0600);
      if(pfd2>=0){char pb[16];snprintf(pb,sizeof(pb),"%d",getpid());write(pfd2,pb,strlen(pb));close(pfd2);}
    }

    sceUserServiceInitialize(NULL);
    sceUserServiceGetInitialUser(&userId);
    sceUserServiceGetForegroundUser(&fgUser);
    gp_log("userId=0x%08x fgUser=0x%08x\n", (uint32_t)userId, (uint32_t)fgUser);
    g_inject_uid = (fgUser > 0) ? fgUser : userId;
    if ((uint32_t)userId<0x10000000u||(uint32_t)userId>0x1000000Fu) userId=0x10000000;
    gp_log("inject_uid=0x%08x\n", (uint32_t)g_inject_uid);

    elevate_credentials();

    ret=scePadInit(); gp_log("scePadInit: 0x%08x\n", ret);
    ret=scePadSetProcessPrivilege(1); gp_log("scePadSetProcessPrivilege: 0x%08x\n", ret);

    /* Start klog capture thread first — must be running before any VDA call */
    if (pthread_create(&g_klog_tid, NULL, klog_capture_thread, NULL)==0) {
        g_klog_started = 1;
        gp_log("klog thread started\n");
    } else {
        gp_log("klog: failed to start capture thread; VDA identity binding is degraded\n");
    }
    runtime_sleep_ms(300); /* let klog thread connect before first VDA */

    /* Remap portal + per-game profile switching */
    sc2_notify_fn = notify_str;
    if (!g_signal_stop && !runtime_is_stopping()) sc2_select_start();
    if (!g_signal_stop && !runtime_is_stopping()) bridge_probe_start(g_inject_uid);
    if (!g_signal_stop && !runtime_is_stopping()) game_hooks_start();
    if (!g_signal_stop && !runtime_is_stopping()) ds_handoff_start();
    if (!g_signal_stop && !runtime_is_stopping()) {
        webui_start();
        notify("Puckbridge: remap portal on port %d", WEBUI_PORT);
    }

    /* Start controller manager — handles all detection, VDA creation, USB threads */
    if (!g_signal_stop && !runtime_is_stopping() &&
        pthread_create(&g_manager_tid, NULL, controller_manager_thread, NULL)==0) {
        g_manager_started = 1;
        gp_log("Manager thread started\n");
    } else {
        if (!g_signal_stop && !runtime_is_stopping()) {
            gp_log("manager: failed to start; no USB controllers will be managed\n");
            startup_result = 1;
        }
    }

    /* Keep-alive until SIGINT/SIGTERM; handlers only set this signal-safe flag. */
    uint32_t tick = 0;
    while (g_manager_started && !g_signal_stop && !runtime_is_stopping()) {
        usleep(1000000);
        if (g_signal_stop || runtime_is_stopping()) break;
        tick++;
        if (tick % 10 == 0) {
            uint32_t active = slot_registry_count_claimed(&g_slots);
            gp_log("alive tick=%u active_slots=%u\n", tick, active);
        }
    }
    while (runtime_shutdown() != 0) {
        gp_log("shutdown: waiting before retrying owner cleanup\n");
        sleep(1);
    }
    return startup_result;
}
