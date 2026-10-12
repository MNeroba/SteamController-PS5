/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "steam_protocol.h"
#include "usb_helpers.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
typedef struct {
    usb_resources_t r;
    struct usb_fs_endpoint ep[2];
    uint8_t sequence[8];
    int offset, count, fail_start, fail_complete, fail_stop, fail_close;
    int short_out, bad_status, bad_frames;
    unsigned starts, stops, closes;
} fake_t;
static int start(int fd, void *v, void *c) {
    fake_t *f = c;
    (void)v;
    assert(fd == 42);
    f->starts++;
    if (f->fail_start) {
        errno = EIO;
        return -1;
    }
    return 0;
}
static int complete(int fd, void *v, uint8_t *idx, void *c) {
    fake_t *f = c;
    (void)v;
    assert(fd == 42);
    if (f->fail_complete) {
        errno = EIO;
        return -1;
    }
    if (f->offset == f->count) {
        errno = EAGAIN;
        return -1;
    }
    *idx = f->sequence[f->offset++];
    if (*idx < 2) {
        struct usb_fs_endpoint *e = &f->ep[*idx];
        e->aFrames = f->bad_frames ? 2 : 1;
        e->status = f->bad_status;
        if (*idx == 0) {
            memset(f->r.frame_data[0], 0, 64);
            f->r.frame_data[0][0] = 0x42;
            f->r.frame_data[0][1] = 1;
            e->pLength[0] = 64;
        } else if (f->short_out)
            e->pLength[0]--;
    }
    return 0;
}
static int stop(int fd, uint8_t ep, void *c) {
    fake_t *f = c;
    assert(fd == 42 && ep < 2);
    f->stops++;
    if (f->fail_stop) {
        errno = EIO;
        return -1;
    }
    return 0;
}
static int close_ep(int fd, uint8_t ep, void *c) {
    fake_t *f = c;
    assert(fd == 42 && ep < 2);
    f->closes++;
    return f->fail_close ? -1 : 0;
}
static int uninit(int fd, void *c) {
    (void)c;
    assert(fd == 42);
    return 0;
}
static int close_fd(int fd, void *c) {
    (void)c;
    assert(fd == 42);
    return 0;
}
static usb_session_ops_t ops = {.start_transfer = start,
                                .complete_transfer = complete,
                                .resources = {.stop_transfer = stop,
                                              .close_endpoint = close_ep,
                                              .uninit_fs = uninit,
                                              .close_fd = close_fd}};
static void init(fake_t *f) {
    memset(f, 0, sizeof(*f));
    usb_resources_init(&f->r);
    assert(usb_resources_adopt_fd(&f->r, 42) == 0);
    assert(usb_resources_note_fs_init_attempt(&f->r) == 0);
    assert(usb_resources_note_fs_init_result(&f->r, 1) == 0);
    for (int i = 0; i < 2; i++) {
        assert(usb_resources_note_endpoint_open(&f->r, (uint8_t)i) == 0);
        f->r.frame_buffer[i][0] = f->r.frame_data[i];
        f->r.frame_length[i][0] = 64;
        f->ep[i].ppBuffer = f->r.frame_buffer[i];
        f->ep[i].pLength = f->r.frame_length[i];
        f->ep[i].nFrames = 1;
    }
}
static void arm(fake_t *f) {
    int config = 0;
    assert(usb_session_start_transfer(&f->r, &ops, 0, &config, f) == 0);
}
static int output(fake_t *f) {
    const uint8_t pulse[4] = {0x8f, 2, 1, 0};
    return usb_send_out(&f->r, &ops, &f->ep[1], 1, pulse, 4, "feedback", 0, 4, f);
}
static void take_input(fake_t *f) {
    uint8_t i = 255;
    int config = 0;
    assert(usb_session_complete_transfer(&f->r, &ops, 0, &config, &i, f) == 0 && i == 0);
    assert(usb_completed_frame_valid(&f->ep[0], 64));
    assert(f->r.frame_data[0][0] == 0x42);
}
int main(void) {
    fake_t f;
    init(&f);
    arm(&f);
    f.sequence[0] = 0;
    f.sequence[1] = 1;
    f.count = 2;
    assert(output(&f) == 0 && f.r.completed_transfers == 1);
    unsigned starts = f.starts;
    int cfg = 0;
    assert(usb_session_start_transfer(&f.r, &ops, 0, &cfg, &f) == USB_SESSION_BAD_STATE &&
           f.starts == starts);
    take_input(&f);
    assert(f.r.completed_transfers == 0); /* input during init/haptic OUT retained */
    assert(usb_session_cleanup(&f.r, &ops, &f) == 0);
    init(&f);
    arm(&f);
    f.sequence[0] = 1;
    f.sequence[1] = 0;
    f.count = 2;
    assert(output(&f) == 0 && f.r.active_transfers == 1);
    take_input(&f); /* OUT first */
    assert(usb_session_cleanup(&f.r, &ops, &f) == 0);
    init(&f);
    arm(&f);
    f.sequence[0] = 0;
    f.count = 1;
    assert(output(&f) == -ETIMEDOUT);
    assert(f.r.retired_transfers == 2);
    take_input(&f);
    uint8_t old[64];
    memcpy(old, f.r.frame_data[1], 64);
    assert(output(&f) == -EBUSY && !memcmp(old, f.r.frame_data[1], 64));
    arm(&f);
    f.sequence[1] = 1;
    f.sequence[2] = 0;
    f.count = 3;
    uint8_t i = 255;
    assert(usb_session_complete_transfer(&f.r, &ops, 0, &cfg, &i, &f) ==
               USB_SESSION_CANCELLED_COMPLETION &&
           i == 1);
    take_input(&f);
    assert(f.r.late_completions == 1); /* optional output fails, IN continues */
    assert(usb_session_cleanup(&f.r, &ops, &f) == 0);
    init(&f);
    f.fail_start = 1;
    f.fail_stop = 1;
    assert(output(&f) == -EIO && f.r.active_transfers == 2);
    memcpy(old, f.r.frame_data[1], 64);
    assert(output(&f) == -EBUSY && !memcmp(old, f.r.frame_data[1], 64));
    f.fail_close = 1;
    assert(usb_resources_finish_fs(&f.r, &ops.resources, &f) ==
           -1); /* uninit resolves ownership despite individual failure */
    assert(!f.r.fs_initialized);
    assert(usb_session_cleanup(&f.r, &ops, &f) == 0);
    for (int kind = 0; kind < 3; kind++) {
        init(&f);
        f.sequence[0] = 1;
        f.count = 1;
        f.short_out = kind == 0;
        f.bad_status = kind == 1 ? -1 : 0;
        f.bad_frames = kind == 2;
        assert(output(&f) != 0 && f.r.protocol_errors);
        assert(usb_session_cleanup(&f.r, &ops, &f) == 0);
    }
    init(&f);
    arm(&f);
    assert(usb_session_close_endpoint(&f.r,&ops,0,&f)==0);
    assert(f.r.retired_transfers&1u); /* CLOSE of unresolved START is cancellation too */
    assert(usb_resources_note_endpoint_open(&f.r,0)==0);
    assert(usb_session_start_transfer(&f.r,&ops,0,&cfg,&f)==USB_SESSION_BAD_STATE);
    assert(usb_session_cleanup(&f.r,&ops,&f)==0);
    init(&f);
    arm(&f);
    f.sequence[0] = 31;
    f.count = 1;
    assert(usb_session_complete_transfer(&f.r, &ops, 0, &cfg, &i, &f) == USB_SESSION_BAD_STATE &&
           errno == EPROTO);
    f.fail_complete = 1;
    assert(usb_session_complete_transfer(&f.r, &ops, 0, &cfg, &i, &f) == -1 &&
           f.r.active_transfers == 1);
    assert(usb_session_cleanup(&f.r, &ops, &f) == 0);
    /* Disconnect during simultaneous I/O, then a fresh session has no pending data. */
    init(&f);
    arm(&f);
    f.fail_complete = 1;
    f.fail_stop = 1;
    assert(output(&f) == -EIO);
    assert(usb_session_cleanup(&f.r, &ops, &f) == -1);
    init(&f);
    assert(!f.r.completed_transfers && !f.r.retired_transfers && !f.r.active_transfers &&
           !f.r.frame_data[0][0]);
    struct usb_fs_endpoint e = f.ep[0];
    e.aFrames = 1;
    e.pLength[0] = 65;
    assert(!usb_completed_frame_valid(&e, 64));
    e.pLength[0] = 0;
    assert(!usb_completed_frame_valid(&e, 64));
    e.nFrames = 2;
    assert(!usb_completed_frame_valid(&e, 64));
    usb_input_watchdog_t w;
    usb_input_watchdog_init(&w, 1000); /* no refresh from malformed/missing data */
    for (uint64_t t = 1100; t < 1500; t += 100)
        assert(!usb_input_watchdog_expired(&w, t, 500));
    assert(usb_input_watchdog_expired(&w, 1500, 500));
    assert(!strcmp(usb_session_error_name(ETIMEDOUT), "timeout"));
    assert(!strcmp(usb_session_error_name(EPROTO), "protocol error"));
    puts("USB completion ordering/cancellation/lifetime/fault regression checks passed");
    return 0;
}
