/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once

#include <stdint.h>

#define USB_RESOURCES_MAX_ENDPOINTS 32

typedef struct {
    int fd;
    int fs_init_attempted;
    int fs_initialized;
    uint32_t opened_endpoints;
    uint32_t active_transfers;
} usb_resources_t;

typedef struct {
    int (*stop_transfer)(int fd, uint8_t endpoint_index, void *context);
    int (*close_endpoint)(int fd, uint8_t endpoint_index, void *context);
    int (*uninit_fs)(int fd, void *context);
    int (*close_fd)(int fd, void *context);
} usb_resources_ops_t;

typedef enum {
    USB_RESOURCES_OK = 0,
    USB_RESOURCES_ERROR = -1,
    USB_RESOURCES_BAD_STATE = -2,
    USB_RESOURCES_BAD_INDEX = -3
} usb_resources_result_t;

void usb_resources_init(usb_resources_t *resources);
int usb_resources_adopt_fd(usb_resources_t *resources, int fd);
int usb_resources_note_fs_init_attempt(usb_resources_t *resources);
int usb_resources_note_fs_init_result(usb_resources_t *resources, int success);
int usb_resources_note_fs_uninit_result(usb_resources_t *resources, int success);
int usb_resources_note_endpoint_open(usb_resources_t *resources,
                                     uint8_t endpoint_index);
int usb_resources_note_transfer_start(usb_resources_t *resources,
                                      uint8_t endpoint_index);
int usb_resources_note_transfer_complete(usb_resources_t *resources,
                                         uint8_t endpoint_index);
int usb_resources_note_transfer_stop(usb_resources_t *resources,
                                     uint8_t endpoint_index);

/* Stop and close tracked endpoints, then uninitialize USB FS only after a
 * successful init, while retaining the file descriptor. Cleanup continues
 * after errors and returns the first failure so callers can log it. */
int usb_resources_finish_fs(usb_resources_t *resources,
                            const usb_resources_ops_t *ops,
                            void *context);

/* Finish USB FS and close the descriptor exactly once. After close_fd is
 * called, ownership is relinquished even if it reports an error, because
 * retrying close can accidentally close a reused descriptor. */
int usb_resources_cleanup(usb_resources_t *resources,
                          const usb_resources_ops_t *ops,
                          void *context);
