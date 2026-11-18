/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "usb_resources.h"

#include <stdio.h>
#include <stdlib.h>

static int checks;

#define CHECK(condition) do { \
    checks++; \
    if (!(condition)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        exit(1); \
    } \
} while (0)

typedef struct {
    char events[64];
    int event_count;
    int fail_stop_index;
    int fail_close_index;
    int fail_uninit;
    int fail_fd_close;
} fake_usb_t;

static void record(fake_usb_t *usb, char op, uint8_t index) {
    if (usb->event_count < (int)sizeof(usb->events))
        usb->events[usb->event_count++] = (char)(op + (index % 10));
}

static int fake_stop(int fd, uint8_t index, void *opaque) {
    fake_usb_t *usb = (fake_usb_t *)opaque;
    CHECK(fd == 42);
    record(usb, 'S', index);
    return usb->fail_stop_index == (int)index ? -11 : 0;
}

static int fake_close_endpoint(int fd, uint8_t index, void *opaque) {
    fake_usb_t *usb = (fake_usb_t *)opaque;
    CHECK(fd == 42);
    record(usb, 'E', index);
    return usb->fail_close_index == (int)index ? -12 : 0;
}

static int fake_uninit(int fd, void *opaque) {
    fake_usb_t *usb = (fake_usb_t *)opaque;
    CHECK(fd == 42);
    record(usb, 'U', 0);
    return usb->fail_uninit ? -13 : 0;
}

static int fake_close_fd(int fd, void *opaque) {
    fake_usb_t *usb = (fake_usb_t *)opaque;
    CHECK(fd == 42);
    record(usb, 'F', 0);
    return usb->fail_fd_close ? -14 : 0;
}

static usb_resources_ops_t make_ops(void) {
    usb_resources_ops_t ops = {
        fake_stop,
        fake_close_endpoint,
        fake_uninit,
        fake_close_fd
    };
    return ops;
}

int main(void) {
    usb_resources_t resources;
    usb_resources_ops_t ops = make_ops();
    fake_usb_t usb = { .fail_stop_index = -1, .fail_close_index = -1 };

    usb_resources_init(&resources);
    CHECK(resources.fd == -1);
    CHECK(usb_resources_cleanup(&resources, &ops, &usb) == USB_RESOURCES_OK);
    CHECK(usb.event_count == 0);

    CHECK(usb_resources_adopt_fd(&resources, 42) == USB_RESOURCES_OK);
    CHECK(usb_resources_note_fs_init_attempt(&resources) == USB_RESOURCES_OK);
    CHECK(usb_resources_note_fs_init_result(&resources, 0) == USB_RESOURCES_OK);
    CHECK(usb_resources_cleanup(&resources, &ops, &usb) == USB_RESOURCES_OK);
    CHECK(usb.event_count == 1);
    CHECK(usb.events[0] == 'F');
    CHECK(resources.fd == -1 && !resources.fs_init_attempted);
    CHECK(usb_resources_cleanup(&resources, &ops, &usb) == USB_RESOURCES_OK);
    CHECK(usb.event_count == 1);

    usb_resources_init(&resources);
    usb.event_count = 0;
    CHECK(usb_resources_adopt_fd(&resources, 42) == USB_RESOURCES_OK);
    CHECK(usb_resources_note_fs_init_attempt(&resources) == USB_RESOURCES_OK);
    CHECK(usb_resources_note_fs_init_result(&resources, 1) == USB_RESOURCES_OK);
    CHECK(usb_resources_note_endpoint_open(&resources, 0) == USB_RESOURCES_OK);
    CHECK(usb_resources_note_endpoint_open(&resources, 1) == USB_RESOURCES_OK);
    CHECK(usb_resources_note_endpoint_open(&resources, 2) == USB_RESOURCES_OK);
    CHECK(usb_resources_note_transfer_start(&resources, 0) == USB_RESOURCES_OK);
    CHECK(usb_resources_note_transfer_start(&resources, 2) == USB_RESOURCES_OK);
    CHECK(usb_resources_note_transfer_complete(&resources, 0) == USB_RESOURCES_OK);
    CHECK(usb_resources_finish_fs(&resources, &ops, &usb) == USB_RESOURCES_OK);
    CHECK(resources.fd == 42 && !resources.fs_init_attempted);
    CHECK(usb.event_count == 5);
    CHECK(usb.events[0] == 'S' + 2);
    CHECK(usb.events[1] == 'E' + 2 && usb.events[2] == 'E' + 1 &&
          usb.events[3] == 'E');
    CHECK(usb.events[4] == 'U' && resources.active_transfers == 0);
    CHECK(usb_resources_cleanup(&resources, &ops, &usb) == USB_RESOURCES_OK);
    CHECK(usb.events[usb.event_count - 1] == 'F' && resources.fd == -1);

    usb_resources_init(&resources);
    usb.event_count = 0;
    usb.fail_stop_index = 3;
    usb.fail_close_index = -1;
    CHECK(usb_resources_adopt_fd(&resources, 42) == USB_RESOURCES_OK);
    CHECK(usb_resources_note_fs_init_attempt(&resources) == USB_RESOURCES_OK);
    CHECK(usb_resources_note_fs_init_result(&resources, 1) == USB_RESOURCES_OK);
    CHECK(usb_resources_note_endpoint_open(&resources, 3) == USB_RESOURCES_OK);
    CHECK(usb_resources_note_transfer_start(&resources, 3) == USB_RESOURCES_OK);
    CHECK(usb_resources_cleanup(&resources, &ops, &usb) == -11);
    CHECK(usb.event_count == 4);
    CHECK(usb.events[0] == 'S' + 3 && usb.events[1] == 'E' + 3 &&
          usb.events[2] == 'U' && usb.events[3] == 'F');
    CHECK(resources.fd == -1 && resources.opened_endpoints == 0);
    CHECK(usb_resources_cleanup(&resources, &ops, &usb) == USB_RESOURCES_OK);
    CHECK(usb.event_count == 4);

    usb_resources_init(&resources);
    usb.event_count = 0;
    usb.fail_stop_index = -1;
    usb.fail_close_index = -1;
    usb.fail_uninit = 1;
    usb.fail_fd_close = 1;
    CHECK(usb_resources_adopt_fd(&resources, 42) == USB_RESOURCES_OK);
    CHECK(usb_resources_note_fs_init_attempt(&resources) == USB_RESOURCES_OK);
    CHECK(usb_resources_note_fs_init_result(&resources, 1) == USB_RESOURCES_OK);
    CHECK(usb_resources_note_endpoint_open(&resources, 0) == USB_RESOURCES_OK);
    CHECK(usb_resources_cleanup(&resources, &ops, &usb) == -13);
    CHECK(resources.fd == -1);
    int events_after_first_close = usb.event_count;
    CHECK(usb_resources_cleanup(&resources, &ops, &usb) == USB_RESOURCES_OK);
    CHECK(usb.event_count == events_after_first_close);

    usb_resources_init(&resources);
    CHECK(usb_resources_note_fs_init_attempt(&resources) ==
          USB_RESOURCES_BAD_STATE);
    CHECK(usb_resources_note_endpoint_open(&resources, 0) ==
          USB_RESOURCES_BAD_STATE);
    CHECK(usb_resources_note_transfer_start(&resources, 0) ==
          USB_RESOURCES_BAD_STATE);
    CHECK(usb_resources_note_endpoint_open(&resources,
          USB_RESOURCES_MAX_ENDPOINTS) == USB_RESOURCES_BAD_INDEX);

    printf("USB resource lifecycle checks passed (%d checks)\n", checks);
    return 0;
}
