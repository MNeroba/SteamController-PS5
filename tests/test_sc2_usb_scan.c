#include "controller_sc2.h"

#include <errno.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

#include "usb_resources.h"

static pthread_mutex_t gate_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t gate_changed = PTHREAD_COND_INITIALIZER;
static int valid_report_written;
static int other_report_written;
static _Thread_local struct usb_fs_endpoint *thread_endpoints;
static _Thread_local int completion_returned;
static _Thread_local int completion_count;
static _Thread_local int thread_fail_close_once;
static _Thread_local int thread_fail_stop_once;
static _Thread_local int thread_scan_mode;
static const uint8_t *descriptor_data;
static uint16_t descriptor_length;
static int descriptor_error;
static int descriptor_reads;

enum { SCAN_MODE_CONCURRENT, SCAN_MODE_MULTIPLEXED, SCAN_MODE_TIMEOUT };

struct scan_call {
    int fd;
    uint8_t address;
    uint8_t addresses[SC2_MAX_EPS];
    int endpoint_count;
    int timeout_ms;
    int mode;
    int result;
    int fail_close_once;
    int fail_stop_once;
    int open_calls;
    int start_calls;
    int complete_calls;
    int stop_calls;
    int close_calls;
    usb_resources_t resources;
};

static _Thread_local struct scan_call *thread_call;

int ioctl(int fd, unsigned long request, ...) {
    va_list args;
    va_start(args, request);
    void *argument = va_arg(args, void *);
    va_end(args);

    if (request == USB_GET_FULL_DESC) {
        struct usb_gen_descriptor *descriptor = argument;
        descriptor_reads++;
        if (descriptor_error != 0) {
            errno = descriptor_error;
            return -1;
        }
        if (!descriptor || !descriptor_data ||
            descriptor_length > descriptor->ugd_maxlen) {
            errno = EINVAL;
            return -1;
        }
        memcpy(descriptor->ugd_data, descriptor_data, descriptor_length);
        descriptor->ugd_actlen = descriptor_length;
        return 0;
    }

    if (request == USB_FS_OPEN) {
        thread_call->open_calls++;
        return 0;
    }
    if (request == USB_FS_START) {
        thread_call->start_calls++;
        return 0;
    }
    if (request == USB_FS_STOP) {
        thread_call->stop_calls++;
        if (thread_fail_stop_once) {
            thread_fail_stop_once = 0;
            errno = EIO;
            return -1;
        }
        return 0;
    }
    if (request == USB_FS_CLOSE) {
        thread_call->close_calls++;
        if (thread_fail_close_once) {
            thread_fail_close_once = 0;
            errno = EIO;
            return -1;
        }
        return 0;
    }
    if (request != USB_FS_COMPLETE) return -1;
    thread_call->complete_calls++;
    if (thread_scan_mode == SCAN_MODE_TIMEOUT) {
        errno = EBUSY;
        return -1;
    }
    if (thread_scan_mode == SCAN_MODE_MULTIPLEXED) {
        int endpoint_index;
        if (completion_count == 0) endpoint_index = 1;
        else if (completion_count == 1) endpoint_index = 0;
        else {
            errno = EBUSY;
            return -1;
        }
        completion_count++;
        struct usb_fs_endpoint *ep = &thread_endpoints[endpoint_index];
        uint8_t *report = ep->ppBuffer[0];
        if (endpoint_index == 1) {
            memset(report, 0, 54);
            report[0] = 0xff;
        } else {
            static const uint8_t input_report[54] = {0x42};
            memcpy(report, input_report, sizeof(input_report));
        }
        ep->pLength[0] = 54;
        ((struct usb_fs_complete *)argument)->ep_index =
            (uint8_t)endpoint_index;
        return 0;
    }
    if (completion_returned) {
        errno = EBUSY;
        return -1;
    }
    completion_returned = 1;

    struct usb_fs_endpoint *ep = &thread_endpoints[0];
    uint8_t *report = ep->ppBuffer[0];
    if (fd == 1) {
        static const uint8_t input_report[54] = {0x42};
        memcpy(report, input_report, sizeof(input_report));
        ep->pLength[0] = sizeof(input_report);

        pthread_mutex_lock(&gate_lock);
        valid_report_written = 1;
        pthread_cond_broadcast(&gate_changed);
        while (!other_report_written)
            pthread_cond_wait(&gate_changed, &gate_lock);
        pthread_mutex_unlock(&gate_lock);
    } else {
        pthread_mutex_lock(&gate_lock);
        while (!valid_report_written)
            pthread_cond_wait(&gate_changed, &gate_lock);
        pthread_mutex_unlock(&gate_lock);

        memset(report, 0, 54);
        report[0] = 0xff;
        ep->pLength[0] = 54;

        pthread_mutex_lock(&gate_lock);
        other_report_written = 1;
        pthread_cond_broadcast(&gate_changed);
        pthread_mutex_unlock(&gate_lock);
    }
    ((struct usb_fs_complete *)argument)->ep_index = 0;
    return 0;
}

static int scan_open_endpoint(int fd, void *configuration, void *context) {
    (void)context;
    return ioctl(fd, USB_FS_OPEN, configuration);
}

static int scan_start_transfer(int fd, void *configuration, void *context) {
    (void)context;
    return ioctl(fd, USB_FS_START, configuration);
}

static int scan_complete_transfer(int fd, void *configuration,
                                  uint8_t *completed_endpoint_index,
                                  void *context) {
    (void)context;
    int result = ioctl(fd, USB_FS_COMPLETE, configuration);
    if (result == 0)
        *completed_endpoint_index =
            ((struct usb_fs_complete *)configuration)->ep_index;
    return result;
}

static int scan_stop_transfer(int fd, uint8_t endpoint, void *context) {
    struct usb_fs_stop stop;
    (void)context;
    memset(&stop, 0, sizeof(stop));
    stop.ep_index = endpoint;
    return ioctl(fd, USB_FS_STOP, &stop);
}

static int scan_close_endpoint(int fd, uint8_t endpoint, void *context) {
    struct usb_fs_close close_endpoint;
    (void)context;
    memset(&close_endpoint, 0, sizeof(close_endpoint));
    close_endpoint.ep_index = endpoint;
    return ioctl(fd, USB_FS_CLOSE, &close_endpoint);
}

static const usb_session_ops_t scan_ops = {
    .open_endpoint = scan_open_endpoint,
    .start_transfer = scan_start_transfer,
    .complete_transfer = scan_complete_transfer,
    .resources = {
        .stop_transfer = scan_stop_transfer,
        .close_endpoint = scan_close_endpoint
    }
};

static void *scan_worker(void *opaque) {
    struct scan_call *call = opaque;
    struct usb_fs_endpoint endpoints[SC2_MAX_EPS];
    memset(endpoints, 0, sizeof(endpoints));
    thread_endpoints = endpoints;
    thread_call = call;
    thread_scan_mode = call->mode;
    completion_count = 0;
    completion_returned = 0;
    thread_fail_close_once = call->fail_close_once;
    thread_fail_stop_once = call->fail_stop_once;
    if (call->endpoint_count == 0) {
        call->addresses[0] = call->address;
        call->endpoint_count = 1;
    }
    if (call->timeout_ms == 0) call->timeout_ms = 5;
    call->result = sc2_find_active_ep(call->fd, &call->resources, &scan_ops,
            call, endpoints, call->addresses, call->endpoint_count,
            call->timeout_ms);
    return NULL;
}

static int cleanup_stop(int fd, uint8_t endpoint, void *context) {
    (void)fd; (void)endpoint; (void)context;
    return 0;
}

static int cleanup_endpoint(int fd, uint8_t endpoint, void *context) {
    int *count = context;
    (void)fd; (void)endpoint;
    (*count)++;
    return 0;
}

static int cleanup_uninit(int fd, void *context) {
    (void)fd; (void)context;
    return 0;
}

static int cleanup_fd(int fd, void *context) {
    (void)fd; (void)context;
    return 0;
}

static void init_scan_resources(usb_resources_t *resources, int fd) {
    usb_resources_init(resources);
    if (usb_resources_adopt_fd(resources, fd) != USB_RESOURCES_OK ||
        usb_resources_note_fs_init_attempt(resources) != USB_RESOURCES_OK ||
        usb_resources_note_fs_init_result(resources, 1) != USB_RESOURCES_OK) {
        fprintf(stderr, "failed to initialize fake USB scan resources\n");
        exit(1);
    }
}

static int test_usb_map_status(void) {
    static const uint8_t malformed_config[] = {
        9, 2, 12, 0, 1, 1, 0, 0x80, 50
    };
    static const uint8_t puck_interface_excluded[] = {
        9, 2, 25, 0, 1, 1, 0, 0x80, 50,
        9, 4, 6, 0, 1, 3, 0, 0, 0,
        7, 5, 0x86, 3, 64, 0, 10
    };
    static const uint8_t valid_puck_interface[] = {
        9, 2, 25, 0, 1, 1, 0, 0x80, 50,
        9, 4, 2, 0, 1, 3, 0, 0, 0,
        7, 5, 0x84, 3, 64, 0, 10
    };
    uint8_t addresses[SC2_MAX_EPS] = {0};

    descriptor_reads = 0;
    descriptor_data = NULL;
    descriptor_length = 0;
    descriptor_error = EIO;
    sc2_ep_map_t map;
    int result = sc2_ep_map(17, 1, &map);
    if (result != SC2_USB_MAP_UNAVAILABLE || descriptor_reads != 1) {
        fprintf(stderr,
                "unavailable USB map status collapsed: result=%d reads=%d\n",
                result, descriptor_reads);
        return 0;
    }

    descriptor_data = malformed_config;
    descriptor_length = sizeof(malformed_config);
    descriptor_error = 0;
    result = sc2_ep_map(17, 1, &map);
    if (result != SC2_USB_MAP_INVALID || descriptor_reads != 2) {
        fprintf(stderr,
                "invalid USB map status collapsed: result=%d reads=%d\n",
                result, descriptor_reads);
        return 0;
    }

    descriptor_error = ENXIO;
    result = sc2_ep_map(17, 1, &map);
    if (result != SC2_USB_MAP_DEVICE_GONE || descriptor_reads != 3) {
        fprintf(stderr,
                "disconnected USB map was treated as unavailable: result=%d reads=%d\n",
                result, descriptor_reads);
        return 0;
    }

    descriptor_data = puck_interface_excluded;
    descriptor_length = sizeof(puck_interface_excluded);
    descriptor_error = 0;
    result = sc2_ep_map(17, 1, &map);
    int endpoint_count = result < 0 ? result :
        sc2_collect_in_eps(&map, addresses, SC2_MAX_EPS);
    if (result != 0 || endpoint_count != 0 || descriptor_reads != 4) {
        fprintf(stderr,
                "valid map with no Puck endpoint was not distinguished: map=%d endpoints=%d reads=%d\n",
                result, endpoint_count, descriptor_reads);
        return 0;
    }

    descriptor_data = valid_puck_interface;
    descriptor_length = sizeof(valid_puck_interface);
    result = sc2_ep_map(17, 1, &map);
    endpoint_count = result < 0 ? result :
        sc2_collect_in_eps(&map, addresses, SC2_MAX_EPS);
    if (result != 1 || endpoint_count != 1 || addresses[0] != 0x84 ||
        descriptor_reads != 5) {
        fprintf(stderr,
                "valid Puck interface map failed: map=%d endpoints=%d ep=0x%02x reads=%d\n",
                result, endpoint_count, addresses[0], descriptor_reads);
        return 0;
    }
    return 1;
}

int main(void) {
    if (!test_usb_map_status()) return 1;

    struct scan_call puck = {
        .fd = 1, .address = 0x81, .endpoint_count = 1, .timeout_ms = 5
    };
    struct scan_call wired = {
        .fd = 2, .address = 0x85, .endpoint_count = 1, .timeout_ms = 5
    };
    puck.addresses[0] = puck.address;
    wired.addresses[0] = wired.address;
    init_scan_resources(&puck.resources, puck.fd);
    init_scan_resources(&wired.resources, wired.fd);
    pthread_t puck_thread, wired_thread;

    if (pthread_create(&puck_thread, NULL, scan_worker, &puck) != 0 ||
        pthread_create(&wired_thread, NULL, scan_worker, &wired) != 0) {
        fprintf(stderr, "failed to create fake USB scan workers\n");
        return 1;
    }
    pthread_join(puck_thread, NULL);
    pthread_join(wired_thread, NULL);

    if (puck.result != puck.address || wired.result != 0) {
        fprintf(stderr,
                "per-session scan buffers were contaminated: Puck=0x%x (wanted 0x%x), wired=0x%x\n",
                puck.result, puck.address, wired.result);
        return 1;
    }
    if (puck.complete_calls != 1 || wired.complete_calls != 1) {
        fprintf(stderr, "concurrent scans did not use the injected COMPLETE callbacks\n");
        return 1;
    }

    if (puck.resources.opened_endpoints != 0 ||
        puck.resources.active_transfers != 0 ||
        wired.resources.opened_endpoints != 0 ||
        wired.resources.active_transfers != 0) {
        fprintf(stderr, "successful scan left endpoint resources tracked\n");
        return 1;
    }

    struct scan_call close_failure = {
        .fd = 3, .address = 0x83, .endpoint_count = 1, .timeout_ms = 5,
        .fail_close_once = 1
    };
    close_failure.addresses[0] = close_failure.address;
    init_scan_resources(&close_failure.resources, close_failure.fd);
    pthread_t failure_thread;
    if (pthread_create(&failure_thread, NULL, scan_worker, &close_failure) != 0) {
        fprintf(stderr, "failed to create USB close-failure scan worker\n");
        return 1;
    }
    pthread_join(failure_thread, NULL);
    if (close_failure.result != -EIO ||
        close_failure.resources.opened_endpoints != 1u ||
        close_failure.resources.active_transfers != 0) {
        fprintf(stderr,
                "close failure lost lifecycle ownership: result=%d opened=0x%x active=0x%x\n",
                close_failure.result,
                close_failure.resources.opened_endpoints,
                close_failure.resources.active_transfers);
        return 1;
    }

    int retry_close_calls = 0;
    usb_resources_ops_t cleanup_ops = {
        .stop_transfer = cleanup_stop,
        .close_endpoint = cleanup_endpoint,
        .uninit_fs = cleanup_uninit,
        .close_fd = cleanup_fd
    };
    if (usb_resources_cleanup(&close_failure.resources, &cleanup_ops,
                              &retry_close_calls) != USB_RESOURCES_OK ||
        retry_close_calls != 1) {
        fprintf(stderr, "tracked endpoint close was not retried by session cleanup\n");
        return 1;
    }

    struct scan_call stop_close_failure = {
        .fd = 6, .endpoint_count = 1, .timeout_ms = 5,
        .mode = SCAN_MODE_TIMEOUT, .fail_stop_once = 1,
        .fail_close_once = 1
    };
    stop_close_failure.addresses[0] = 0x86;
    init_scan_resources(&stop_close_failure.resources, stop_close_failure.fd);
    (void)scan_worker(&stop_close_failure);
    if (stop_close_failure.result != -EIO ||
        stop_close_failure.stop_calls != 1 ||
        stop_close_failure.close_calls != 1 ||
        stop_close_failure.resources.opened_endpoints != 1u ||
        stop_close_failure.resources.active_transfers != 1u) {
        fprintf(stderr,
                "failed STOP/CLOSE lost scanner ownership: result=%d opened=0x%x active=0x%x\n",
                stop_close_failure.result,
                stop_close_failure.resources.opened_endpoints,
                stop_close_failure.resources.active_transfers);
        return 1;
    }
    int retry_stop_close_calls = 0;
    if (usb_resources_cleanup(&stop_close_failure.resources, &cleanup_ops,
                              &retry_stop_close_calls) != USB_RESOURCES_OK ||
        retry_stop_close_calls != 1 ||
        stop_close_failure.resources.opened_endpoints != 0 ||
        stop_close_failure.resources.active_transfers != 0) {
        fprintf(stderr, "session cleanup did not retry failed scan STOP/CLOSE\n");
        return 1;
    }

    struct scan_call multiplexed = {
        .fd = 4, .endpoint_count = 2, .timeout_ms = 10,
        .mode = SCAN_MODE_MULTIPLEXED
    };
    multiplexed.addresses[0] = 0x81;
    multiplexed.addresses[1] = 0x82;
    init_scan_resources(&multiplexed.resources, multiplexed.fd);
    (void)scan_worker(&multiplexed);
    if (multiplexed.result != multiplexed.addresses[0] ||
        multiplexed.open_calls != 2 || multiplexed.start_calls != 2 ||
        multiplexed.complete_calls != 2 || multiplexed.stop_calls != 0 ||
        multiplexed.close_calls != 2 ||
        multiplexed.resources.opened_endpoints != 0 ||
        multiplexed.resources.active_transfers != 0) {
        fprintf(stderr,
                "multiplexed COMPLETE did not account the returned endpoint first: result=0x%x open=%d start=%d complete=%d stop=%d close=%d state=%x/%x\n",
                multiplexed.result, multiplexed.open_calls,
                multiplexed.start_calls, multiplexed.complete_calls,
                multiplexed.stop_calls, multiplexed.close_calls,
                multiplexed.resources.opened_endpoints,
                multiplexed.resources.active_transfers);
        return 1;
    }

    struct scan_call competing_transfer = {
        .fd = 5, .address = 0x84, .endpoint_count = 1, .timeout_ms = 5
    };
    competing_transfer.addresses[0] = competing_transfer.address;
    init_scan_resources(&competing_transfer.resources, competing_transfer.fd);
    if (usb_resources_note_endpoint_open(&competing_transfer.resources, 3) !=
            USB_RESOURCES_OK ||
        usb_resources_note_transfer_start(&competing_transfer.resources, 3) !=
            USB_RESOURCES_OK) {
        fprintf(stderr, "failed to set up competing transfer ownership check\n");
        return 1;
    }
    (void)scan_worker(&competing_transfer);
    if (competing_transfer.result != USB_RESOURCES_BAD_STATE ||
        competing_transfer.open_calls != 0 ||
        competing_transfer.start_calls != 0 ||
        competing_transfer.complete_calls != 0 ||
        competing_transfer.resources.opened_endpoints != (UINT32_C(1) << 3) ||
        competing_transfer.resources.active_transfers != (UINT32_C(1) << 3)) {
        fprintf(stderr, "SC2 scan consumed or changed an existing USB transfer\n");
        return 1;
    }
    int competing_cleanup_closes = 0;
    if (usb_resources_cleanup(&competing_transfer.resources, &cleanup_ops,
                              &competing_cleanup_closes) != USB_RESOURCES_OK ||
        competing_cleanup_closes != 1) {
        fprintf(stderr, "competing transfer ownership was not cleanable\n");
        return 1;
    }
    puts("SC2 USB scan injection, multiplexing, and ownership checks passed");
    return 0;
}
