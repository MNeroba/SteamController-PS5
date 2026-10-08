/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once

#include "usb_resources.h"

typedef struct {
    int (*open_device)(const char *path, int flags, void *context);
    int (*init_fs)(int fd, void *configuration, void *context);
    int (*open_endpoint)(int fd, void *configuration, void *context);
    int (*start_transfer)(int fd, void *configuration, void *context);
    /* COMPLETE is asynchronous: report the endpoint index it actually returned. */
    int (*complete_transfer)(int fd, void *configuration,
                             uint8_t *completed_endpoint_index,
                             void *context);
    usb_resources_ops_t resources;
} usb_session_ops_t;

enum {
    USB_SESSION_OK = 0,
    /* COMPLETE returned another active endpoint; that completion is tracked. */
    USB_SESSION_COMPLETED_OTHER = 1,
    USB_SESSION_ERROR = -1,
    USB_SESSION_BAD_STATE = -2
};

int usb_session_open(usb_resources_t *resources,
                     const usb_session_ops_t *ops,
                     const char *path, int flags, void *context);
int usb_session_init_fs(usb_resources_t *resources,
                        const usb_session_ops_t *ops,
                        void *configuration, void *context);
int usb_session_uninit_fs(usb_resources_t *resources,
                          const usb_session_ops_t *ops,
                          void *context);
int usb_session_open_endpoint(usb_resources_t *resources,
                              const usb_session_ops_t *ops,
                              uint8_t endpoint_index, void *configuration,
                              void *context);
int usb_session_close_endpoint(usb_resources_t *resources,
                               const usb_session_ops_t *ops,
                               uint8_t endpoint_index, void *context);
int usb_session_start_transfer(usb_resources_t *resources,
                               const usb_session_ops_t *ops,
                               uint8_t endpoint_index, void *configuration,
                               void *context);
/* Complete and account whichever active endpoint the USB FS queue reports. */
int usb_session_complete_any_transfer(usb_resources_t *resources,
                                      const usb_session_ops_t *ops,
                                      void *configuration,
                                      uint8_t *completed_endpoint_index,
                                      void *context);
int usb_session_complete_transfer(usb_resources_t *resources,
                                  const usb_session_ops_t *ops,
                                  uint8_t endpoint_index, void *configuration,
                                  /* Output: endpoint index returned by COMPLETE. */
                                  uint8_t *completed_endpoint_index,
                                  void *context);
int usb_session_stop_transfer(usb_resources_t *resources,
                              const usb_session_ops_t *ops,
                              uint8_t endpoint_index, void *context);
int usb_session_cleanup(usb_resources_t *resources,
                        const usb_session_ops_t *ops,
                        void *context);
