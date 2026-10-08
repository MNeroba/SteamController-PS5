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
static _Thread_local int thread_fail_close_once;

struct scan_call {
    int fd;
    uint8_t address;
    int result;
    int fail_close_once;
    usb_resources_t resources;
};

int ioctl(int fd, unsigned long request, ...) {
    va_list args;
    va_start(args, request);
    void *argument = va_arg(args, void *);
    va_end(args);

    if (request == USB_FS_CLOSE && thread_fail_close_once) {
        thread_fail_close_once = 0;
        errno = EIO;
        return -1;
    }
    if (request == USB_FS_OPEN || request == USB_FS_START ||
        request == USB_FS_STOP || request == USB_FS_CLOSE)
        return 0;
    if (request != USB_FS_COMPLETE) return -1;
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

static void *scan_worker(void *opaque) {
    struct scan_call *call = opaque;
    struct usb_fs_endpoint endpoints[1];
    memset(endpoints, 0, sizeof(endpoints));
    thread_endpoints = endpoints;
    thread_fail_close_once = call->fail_close_once;
    call->result = sc2_find_active_ep(call->fd, &call->resources, endpoints,
                                      &call->address, 1, 5);
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

int main(void) {
    struct scan_call puck = { .fd = 1, .address = 0x81 };
    struct scan_call wired = { .fd = 2, .address = 0x85 };
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

    if (puck.resources.opened_endpoints != 0 ||
        puck.resources.active_transfers != 0 ||
        wired.resources.opened_endpoints != 0 ||
        wired.resources.active_transfers != 0) {
        fprintf(stderr, "successful scan left endpoint resources tracked\n");
        return 1;
    }

    struct scan_call close_failure = {
        .fd = 3, .address = 0x83, .fail_close_once = 1
    };
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
    puts("SC2 concurrent USB endpoint scan buffers are isolated");
    return 0;
}
