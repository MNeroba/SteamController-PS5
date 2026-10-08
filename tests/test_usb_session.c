/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "usb_session.h"

#include <dev/usb/usb.h>

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
    int fail_open;
    int fail_init;
    int fail_endpoint_open;
    int fail_start;
    int fail_complete;
    int open_calls;
    int init_calls;
    int endpoint_open_calls;
    int start_calls;
    int complete_calls;
    int stop_calls;
    int endpoint_close_calls;
    int uninit_calls;
    int fd_close_calls;
    int completed_endpoint;
    uint32_t stopped_endpoints;
    uint32_t closed_endpoints;
} fake_session_t;

static int fake_open(const char *path, int flags, void *opaque) {
    fake_session_t *fake = (fake_session_t *)opaque;
    CHECK(path != NULL && flags == 7);
    fake->open_calls++;
    return fake->fail_open ? -1 : 42;
}

static int fake_init(int fd, void *configuration, void *opaque) {
    fake_session_t *fake = (fake_session_t *)opaque;
    CHECK(fd == 42 && configuration != NULL);
    fake->init_calls++;
    return fake->fail_init ? -1 : 0;
}

static int fake_endpoint_open(int fd, void *configuration, void *opaque) {
    fake_session_t *fake = (fake_session_t *)opaque;
    CHECK(fd == 42 && configuration != NULL);
    fake->endpoint_open_calls++;
    return fake->fail_endpoint_open ? -1 : 0;
}

static int fake_start(int fd, void *configuration, void *opaque) {
    fake_session_t *fake = (fake_session_t *)opaque;
    CHECK(fd == 42 && configuration != NULL);
    fake->start_calls++;
    return fake->fail_start ? -1 : 0;
}

static int fake_complete(int fd, void *configuration,
                         uint8_t *completed_endpoint_index, void *opaque) {
    fake_session_t *fake = (fake_session_t *)opaque;
    CHECK(fd == 42 && configuration != NULL);
    fake->complete_calls++;
    ((struct usb_fs_complete *)configuration)->ep_index =
        (uint8_t)fake->completed_endpoint;
    *completed_endpoint_index = (uint8_t)fake->completed_endpoint;
    return fake->fail_complete ? -1 : 0;
}

static int fake_stop(int fd, uint8_t endpoint, void *opaque) {
    fake_session_t *fake = (fake_session_t *)opaque;
    CHECK(fd == 42 && endpoint < USB_RESOURCES_MAX_ENDPOINTS);
    fake->stop_calls++;
    fake->stopped_endpoints |= UINT32_C(1) << endpoint;
    return 0;
}

static int fake_endpoint_close(int fd, uint8_t endpoint, void *opaque) {
    fake_session_t *fake = (fake_session_t *)opaque;
    CHECK(fd == 42 && endpoint < USB_RESOURCES_MAX_ENDPOINTS);
    fake->endpoint_close_calls++;
    fake->closed_endpoints |= UINT32_C(1) << endpoint;
    return 0;
}

static int fake_uninit(int fd, void *opaque) {
    fake_session_t *fake = (fake_session_t *)opaque;
    CHECK(fd == 42);
    fake->uninit_calls++;
    return 0;
}

static int fake_fd_close(int fd, void *opaque) {
    fake_session_t *fake = (fake_session_t *)opaque;
    CHECK(fd == 42);
    fake->fd_close_calls++;
    return 0;
}

static usb_session_ops_t make_ops(void) {
    usb_session_ops_t ops = {
        .open_device = fake_open,
        .init_fs = fake_init,
        .open_endpoint = fake_endpoint_open,
        .start_transfer = fake_start,
        .complete_transfer = fake_complete,
        .resources = {
            .stop_transfer = fake_stop,
            .close_endpoint = fake_endpoint_close,
            .uninit_fs = fake_uninit,
            .close_fd = fake_fd_close
        }
    };
    return ops;
}

int main(void) {
    usb_resources_t resources;
    usb_resources_init(&resources);
    usb_session_ops_t ops = make_ops();
    fake_session_t fake = { .fail_open = 1 };
    int config = 1;

    CHECK(usb_session_open(&resources, &ops, "/dev/fake", 7, &fake) == -1);
    CHECK(resources.fd == -1 && fake.fd_close_calls == 0);

    fake = (fake_session_t){ .fail_init = 1 };
    CHECK(usb_session_open(&resources, &ops, "/dev/fake", 7, &fake) == 0);
    CHECK(usb_session_init_fs(&resources, &ops, &config, &fake) == -1);
    CHECK(resources.fs_init_attempted && !resources.fs_initialized);
    CHECK(usb_session_cleanup(&resources, &ops, &fake) == 0);
    CHECK(fake.uninit_calls == 0 && fake.fd_close_calls == 1);

    usb_resources_init(&resources);
    fake = (fake_session_t){ .fail_endpoint_open = 1 };
    CHECK(usb_session_open(&resources, &ops, "/dev/fake", 7, &fake) == 0);
    CHECK(usb_session_init_fs(&resources, &ops, &config, &fake) == 0);
    CHECK(usb_session_open_endpoint(&resources, &ops, 0, &config, &fake) == -1);
    CHECK(resources.opened_endpoints == 0);
    CHECK(usb_session_cleanup(&resources, &ops, &fake) == 0);
    CHECK(fake.endpoint_close_calls == 0 && fake.uninit_calls == 1);

    usb_resources_init(&resources);
    fake = (fake_session_t){ .fail_start = 1 };
    CHECK(usb_session_open(&resources, &ops, "/dev/fake", 7, &fake) == 0);
    CHECK(usb_session_init_fs(&resources, &ops, &config, &fake) == 0);
    CHECK(usb_session_open_endpoint(&resources, &ops, 0, &config, &fake) == 0);
    CHECK(usb_session_start_transfer(&resources, &ops, 0, &config, &fake) == -1);
    CHECK(resources.active_transfers == 1);
    CHECK(usb_session_cleanup(&resources, &ops, &fake) == 0);
    CHECK(fake.stop_calls == 1 && fake.endpoint_close_calls == 1);

    usb_resources_init(&resources);
    fake = (fake_session_t){ .fail_complete = 1 };
    CHECK(usb_session_open(&resources, &ops, "/dev/fake", 7, &fake) == 0);
    CHECK(usb_session_init_fs(&resources, &ops, &config, &fake) == 0);
    CHECK(usb_session_open_endpoint(&resources, &ops, 0, &config, &fake) == 0);
    CHECK(usb_session_start_transfer(&resources, &ops, 0, &config, &fake) == 0);
    uint8_t completed_endpoint = UINT8_MAX;
    CHECK(usb_session_complete_transfer(&resources, &ops, 0, &config,
                                        &completed_endpoint, &fake) == -1);
    CHECK(resources.active_transfers == 1);
    CHECK(usb_session_cleanup(&resources, &ops, &fake) == 0);
    CHECK(fake.stop_calls == 1);

    usb_resources_init(&resources);
    fake = (fake_session_t){0};
    CHECK(usb_session_open(&resources, &ops, "/dev/fake", 7, &fake) == 0);
    CHECK(usb_session_init_fs(&resources, &ops, &config, &fake) == 0);
    CHECK(usb_session_open_endpoint(&resources, &ops, 0, &config, &fake) == 0);
    CHECK(usb_session_start_transfer(&resources, &ops, 0, &config, &fake) == 0);
    CHECK(usb_session_complete_transfer(&resources, &ops, 0, &config,
                                        &completed_endpoint, &fake) == 0);
    CHECK(resources.active_transfers == 0);
    CHECK(usb_session_cleanup(&resources, &ops, &fake) == 0);
    CHECK(fake.stop_calls == 0 && fake.fd_close_calls == 1);

    /* COMPLETE returns the index that finished; it is not a request for the
     * endpoint the caller hoped to complete. A different endpoint can finish
     * first and must remain accurately represented in the shared tracker. */
    usb_resources_init(&resources);
    fake = (fake_session_t){0};
    CHECK(usb_session_open(&resources, &ops, "/dev/fake", 7, &fake) == 0);
    CHECK(usb_session_init_fs(&resources, &ops, &config, &fake) == 0);
    CHECK(usb_session_open_endpoint(&resources, &ops, 0, &config, &fake) == 0);
    CHECK(usb_session_open_endpoint(&resources, &ops, 1, &config, &fake) == 0);
    CHECK(usb_session_start_transfer(&resources, &ops, 0, &config, &fake) == 0);
    CHECK(usb_session_start_transfer(&resources, &ops, 1, &config, &fake) == 0);
    struct usb_fs_complete completion = {0};
    fake.completed_endpoint = 1;
    CHECK(usb_session_complete_transfer(&resources, &ops, 0, &completion,
                                        &completed_endpoint, &fake) ==
          USB_SESSION_COMPLETED_OTHER);
    CHECK(completed_endpoint == 1);
    CHECK(completion.ep_index == 1);
    CHECK(resources.active_transfers == UINT32_C(1));
    fake.completed_endpoint = 0;
    CHECK(usb_session_complete_transfer(&resources, &ops, 0, &completion,
                                        &completed_endpoint, &fake) ==
          USB_SESSION_OK);
    CHECK(resources.active_transfers == 0);
    CHECK(usb_session_cleanup(&resources, &ops, &fake) == 0);
    CHECK(fake.stopped_endpoints == 0);
    CHECK(fake.closed_endpoints == (UINT32_C(1) << 0 | UINT32_C(1) << 1));

    printf("USB session fault-injection checks passed (%d checks)\n", checks);
    return 0;
}
