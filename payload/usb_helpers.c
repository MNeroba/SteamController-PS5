/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "usb_helpers.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifdef __PROSPERO__
extern void ghostpad_status_log(const char *fmt, ...);
#define LOG(...) ghostpad_status_log(__VA_ARGS__)
#else
#define LOG(...) fprintf(stderr, __VA_ARGS__)
#endif

static int stop_after_error(usb_resources_t *resources,
                            const usb_session_ops_t *ops,
                            uint8_t endpoint_index, const char *tag,
                            int primary_error, void *context) {
    int stop_result = usb_session_stop_transfer(resources, ops,
                                                endpoint_index, context);
    if (stop_result != 0)
        LOG("OUT %s STOP cleanup failed errno=%d; session retains endpoint %u\n",
            tag ? tag : "?", errno, endpoint_index);
    return -primary_error;
}

int usb_send_out(usb_resources_t *resources, const usb_session_ops_t *ops,
                 struct usb_fs_endpoint *endpoint, uint8_t endpoint_index,
                 const uint8_t *data, uint32_t length, const char *tag,
                 uint32_t poll_interval_us, uint32_t max_polls,
                 void *context) {
    if (!resources || !ops || !endpoint || !data || length == 0 || length > USB_RESOURCES_FRAME_CAPACITY ||
        endpoint_index >= USB_RESOURCES_MAX_ENDPOINTS || max_polls == 0 ||
        poll_interval_us > 1000000u) {
        errno = EINVAL;
        return -EINVAL;
    }

    /* Refuse before mutating metadata referenced by an earlier START. */
    uint32_t bit = UINT32_C(1) << endpoint_index;
    if (resources->fd < 0 || !resources->fs_initialized ||
        !(resources->opened_endpoints & bit) || (resources->active_transfers | resources->completed_transfers | resources->retired_transfers) & bit) {
        errno = EBUSY; return -EBUSY;
    }
    memcpy(resources->frame_data[endpoint_index], data, length);
    resources->frame_buffer[endpoint_index][0] = resources->frame_data[endpoint_index];
    resources->frame_length[endpoint_index][0] = length;
    endpoint->ppBuffer = resources->frame_buffer[endpoint_index];
    endpoint->pLength = resources->frame_length[endpoint_index];
    endpoint->nFrames = 1;
    endpoint->timeout = 150;
    endpoint->flags = 0;
    endpoint->aFrames = 0;
    endpoint->status = 0;

    struct usb_fs_start start;
    memset(&start, 0, sizeof(start));
    start.ep_index = endpoint_index;
    int result = usb_session_start_transfer(resources, ops, endpoint_index,
                                            &start, context);
    if (result != 0) {
        int saved_errno = errno ? errno : EIO;
        /* BAD_STATE means no new START was issued; stopping here could steal
         * an operation owned by an earlier caller on the same endpoint. */
        if (result == USB_SESSION_BAD_STATE) return -saved_errno;
        return stop_after_error(resources, ops, endpoint_index, tag,
                                saved_errno, context);
    }

    for (uint32_t poll = 0; poll < max_polls; poll++) {
        struct usb_fs_complete complete;
        memset(&complete, 0, sizeof(complete));
        uint8_t completed_endpoint = UINT8_MAX;
        result = usb_session_complete_transfer(resources, ops, endpoint_index,
                                               &complete,
                                               &completed_endpoint, context);
        if (result == USB_SESSION_OK) {
            if (!usb_completed_frame_valid(endpoint, length) || endpoint->pLength[0] != length) {
                usb_session_note_error(resources, EPROTO);
                return endpoint->status > 0 && endpoint->status <= INT_MAX-1000
                    ? -1000 - endpoint->status : -EPROTO;
            }
            return 0;
        }
        if (result == USB_SESSION_COMPLETED_OTHER || result == USB_SESSION_CANCELLED_COMPLETION) continue;

        int saved_errno = errno ? errno : EIO;
        if (saved_errno != EBUSY && saved_errno != EAGAIN && saved_errno != EWOULDBLOCK && saved_errno != ETIMEDOUT) {
            LOG("OUT %s COMPLETE fail errno=%d\n", tag ? tag : "?",
                saved_errno);
            return stop_after_error(resources, ops, endpoint_index, tag,
                                    saved_errno, context);
        }
        if (poll_interval_us != 0) usleep(poll_interval_us);
    }

    usb_session_note_error(resources, ETIMEDOUT);
    LOG("OUT %s timeout after %u polls\n", tag ? tag : "?", max_polls);
    return stop_after_error(resources, ops, endpoint_index, tag, ETIMEDOUT,
                            context);
}

int usb_send_cmd(usb_resources_t *resources, const usb_session_ops_t *ops,
                 struct usb_fs_endpoint *endpoint, uint8_t endpoint_index,
                 uint8_t first, uint8_t second, uint32_t poll_interval_us,
                 uint32_t max_polls, void *context) {
    uint8_t buffer[2] = { first, second };
    char tag[8];
    snprintf(tag, sizeof(tag), "%02x%02x", first, second);
    return usb_send_out(resources, ops, endpoint, endpoint_index, buffer,
                        sizeof(buffer), tag, poll_interval_us, max_polls,
                        context);
}
