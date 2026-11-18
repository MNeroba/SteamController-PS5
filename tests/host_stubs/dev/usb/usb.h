#pragma once

/* Minimal declarations needed to compile the host gyro state test. */
#include <stdint.h>

struct usb_fs_endpoint {
    void **ppBuffer;
    uint32_t *pLength;
    int nFrames;
    int timeout;
    int flags;
    int aFrames;
    int status;
};

struct usb_fs_open {
    uint8_t ep_index;
    uint8_t ep_no;
    uint32_t max_bufsize;
    uint32_t max_frames;
    uint32_t max_packet_length;
};

struct usb_fs_start { uint8_t ep_index; };
struct usb_fs_complete { uint8_t ep_index; };
struct usb_fs_stop { uint8_t ep_index; };
struct usb_fs_close { uint8_t ep_index; };

struct usb_gen_descriptor {
    void *ugd_data;
    uint32_t ugd_maxlen;
    uint32_t ugd_actlen;
    uint8_t ugd_config_index;
};

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
