/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once

#include <stdint.h>
#include <dev/usb/usb.h>
#include <dev/usb/usb_ioctl.h>

#include "usb_session.h"

/* Send one interrupt-OUT transfer through the owning session. COMPLETE may
 * report another endpoint first; its state is updated and polling continues.
 * A failed STOP stays owned so usb_session_cleanup() can retry it. */
int usb_send_out(usb_resources_t *resources, const usb_session_ops_t *ops,
                 struct usb_fs_endpoint *endpoint, uint8_t endpoint_index,
                 const uint8_t *data, uint32_t length, const char *tag,
                 uint32_t poll_interval_us, uint32_t max_polls,
                 void *context);

int usb_send_cmd(usb_resources_t *resources, const usb_session_ops_t *ops,
                 struct usb_fs_endpoint *endpoint, uint8_t endpoint_index,
                 uint8_t first, uint8_t second, uint32_t poll_interval_us,
                 uint32_t max_polls, void *context);
