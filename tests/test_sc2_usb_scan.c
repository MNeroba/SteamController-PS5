#include "controller_sc2.h"

#include <errno.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

static pthread_mutex_t gate_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t gate_changed = PTHREAD_COND_INITIALIZER;
static int valid_report_written;
static int other_report_written;
static _Thread_local struct usb_fs_endpoint *thread_endpoints;
static _Thread_local int completion_returned;

struct scan_call {
    int fd;
    uint8_t address;
    int result;
};

int ioctl(int fd, unsigned long request, ...) {
    va_list args;
    va_start(args, request);
    void *argument = va_arg(args, void *);
    va_end(args);

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
    call->result = sc2_find_active_ep(call->fd, endpoints, &call->address, 1, 5);
    return NULL;
}

int main(void) {
    struct scan_call puck = {1, 0x81, 0};
    struct scan_call wired = {2, 0x85, 0};
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
    puts("SC2 concurrent USB endpoint scan buffers are isolated");
    return 0;
}
