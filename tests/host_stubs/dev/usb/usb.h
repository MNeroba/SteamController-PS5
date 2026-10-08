#pragma once

/* Minimal declarations needed to compile the host gyro state test. */
struct usb_fs_endpoint;

struct usb_ctl_request {
    void *ucr_data;
    struct {
        uint8_t bmRequestType;
        uint8_t bRequest;
        uint16_t wValue;
        uint16_t wIndex;
        uint16_t wLength;
    } ucr_request;
};
