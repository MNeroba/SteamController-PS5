#include "usb_helpers.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int checks;

#define CHECK(expression) do { \
    checks++; \
    if (!(expression)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expression); \
        exit(1); \
    } \
} while (0)

typedef struct {
    int fail_start_once;
    int fail_complete_once;
    int busy_completions;
    int fail_stop_once;
    int start_calls;
    int complete_calls;
    int stop_calls;
    int close_endpoint_calls;
    int uninit_calls;
    int close_fd_calls;
    uint8_t completed[4];
    int completion_count;
    int completion_status;
    struct usb_fs_endpoint *endpoint;
    const void *observed_buffer;
} fake_usb_t;

static int fake_start(int fd, void *configuration, void *opaque) {
    fake_usb_t *fake = opaque;
    CHECK(fd == 42 && configuration != NULL);
    uint8_t endpoint = ((struct usb_fs_start *)configuration)->ep_index;
    CHECK(endpoint == 1 || endpoint == 2);
    fake->start_calls++;
    if (fake->endpoint && fake->endpoint->ppBuffer) fake->observed_buffer = fake->endpoint->ppBuffer[0];
    if (fake->fail_start_once) {
        fake->fail_start_once = 0;
        errno = EIO;
        return -1;
    }
    return 0;
}

static int fake_complete(int fd, void *configuration,
                         uint8_t *completed_endpoint_index, void *opaque) {
    fake_usb_t *fake = opaque;
    CHECK(fd == 42 && configuration != NULL &&
          completed_endpoint_index != NULL);
    fake->complete_calls++;
    if(fake->endpoint)fake->endpoint->aFrames=1;
    if (fake->fail_complete_once) {
        fake->fail_complete_once = 0;
        errno = EIO;
        return -1;
    }
    if (fake->busy_completions > 0) {
        fake->busy_completions--;
        errno = EBUSY;
        return -1;
    }
    uint8_t endpoint = 1;
    if (fake->completion_count < 4)
        endpoint = fake->completed[fake->completion_count++];
    ((struct usb_fs_complete *)configuration)->ep_index = endpoint;
    *completed_endpoint_index = endpoint;
    if (fake->endpoint) fake->endpoint->status = fake->completion_status;
    return 0;
}

static int fake_stop(int fd, uint8_t endpoint, void *opaque) {
    fake_usb_t *fake = opaque;
    CHECK(fd == 42 && endpoint == 1);
    fake->stop_calls++;
    if (fake->fail_stop_once) {
        fake->fail_stop_once = 0;
        errno = EIO;
        return -1;
    }
    return 0;
}

static int fake_close_endpoint(int fd, uint8_t endpoint, void *opaque) {
    fake_usb_t *fake = opaque;
    CHECK(fd == 42 && (endpoint == 1 || endpoint == 2));
    fake->close_endpoint_calls++;
    return 0;
}

static int fake_uninit(int fd, void *opaque) {
    fake_usb_t *fake = opaque;
    CHECK(fd == 42);
    fake->uninit_calls++;
    return 0;
}

static int fake_close_fd(int fd, void *opaque) {
    fake_usb_t *fake = opaque;
    CHECK(fd == 42);
    fake->close_fd_calls++;
    return 0;
}

static usb_session_ops_t make_ops(void) {
    usb_session_ops_t ops = {
        .start_transfer = fake_start,
        .complete_transfer = fake_complete,
        .resources = {
            .stop_transfer = fake_stop,
            .close_endpoint = fake_close_endpoint,
            .uninit_fs = fake_uninit,
            .close_fd = fake_close_fd
        }
    };
    return ops;
}

static void init_session(usb_resources_t *resources, int endpoints) {
    usb_resources_init(resources);
    CHECK(usb_resources_adopt_fd(resources, 42) == USB_RESOURCES_OK);
    CHECK(usb_resources_note_fs_init_attempt(resources) == USB_RESOURCES_OK);
    CHECK(usb_resources_note_fs_init_result(resources, 1) == USB_RESOURCES_OK);
    for (int i = 1; i <= endpoints; i++)
        CHECK(usb_resources_note_endpoint_open(resources, (uint8_t)i) ==
              USB_RESOURCES_OK);
}

int main(void) {
    usb_resources_t resources;
    usb_session_ops_t ops = make_ops();
    struct usb_fs_endpoint endpoints[3] = {0};
    const uint8_t report[] = {0x80, 1, 2, 3};
    fake_usb_t fake = {0};
    init_session(&resources, 2);
    fake.endpoint = &endpoints[1];
    fake.completed[0] = 2;
    fake.completed[1] = 1;
    fake.completion_count = 0;
    struct usb_fs_start extra_start = {.ep_index = 2};
    CHECK(usb_session_start_transfer(&resources, &ops, 2, &extra_start,
                                     &fake) == 0);
    CHECK(usb_send_out(&resources, &ops, &endpoints[1], 1, report,
                       sizeof(report), "two-endpoint", 0, 3, &fake) == 0);
    CHECK(fake.complete_calls == 2);
    CHECK(resources.active_transfers == 0);
    CHECK(fake.stop_calls == 0);

    fake = (fake_usb_t){ .fail_complete_once = 1, .fail_stop_once = 1, .endpoint = &endpoints[1] };
    init_session(&resources, 1);
    CHECK(usb_send_out(&resources, &ops, &endpoints[1], 1, report,
                       sizeof(report), "stop-retry", 0, 2, &fake) == -EIO);
    CHECK(fake.observed_buffer != report); /* The caller may return immediately. */
    CHECK(resources.active_transfers == (UINT32_C(1) << 1));
    CHECK(fake.start_calls == 1 && fake.stop_calls == 1);
    CHECK(usb_send_out(&resources, &ops, &endpoints[1], 1, report,
                       sizeof(report), "owned-transfer", 0, 2, &fake) == -EBUSY);
    CHECK(fake.start_calls == 1 && fake.stop_calls == 1);
    CHECK(endpoints[1].ppBuffer[0] == fake.observed_buffer);
    CHECK(memcmp(fake.observed_buffer, report, sizeof(report)) == 0);
    CHECK(endpoints[1].pLength[0] == sizeof(report));
    endpoints[1].aFrames = 1;
    CHECK(usb_completed_frame_valid(&endpoints[1], 64));
    endpoints[1].status = EPIPE;
    CHECK(!usb_completed_frame_valid(&endpoints[1], 64));
    endpoints[1].status = 0; endpoints[1].aFrames = 0;
    CHECK(!usb_completed_frame_valid(&endpoints[1], 64));
    endpoints[1].aFrames = 1; endpoints[1].pLength[0] = 65;
    CHECK(!usb_completed_frame_valid(&endpoints[1], 64));
    CHECK(usb_session_cleanup(&resources, &ops, &fake) == 0);
    CHECK(fake.stop_calls == 2 && resources.fd == -1);

    fake = (fake_usb_t){ .fail_start_once = 1, .endpoint = &endpoints[1] };
    init_session(&resources, 1);
    CHECK(usb_send_out(&resources, &ops, &endpoints[1], 1, report,
                       sizeof(report), "start-stop", 0, 2, &fake) == -EIO);
    CHECK(resources.active_transfers == 0 && fake.stop_calls == 1);
    CHECK(usb_session_cleanup(&resources, &ops, &fake) == 0);

    fake = (fake_usb_t){ .endpoint=&endpoints[1], .completed={1}, .completion_status=INT_MAX };
    init_session(&resources,1);
    CHECK(usb_send_out(&resources,&ops,&endpoints[1],1,report,
                       sizeof(report),"invalid-status",0,2,&fake)==-EPROTO);
    CHECK(resources.protocol_errors==1 && resources.active_transfers==0);
    CHECK(usb_session_cleanup(&resources,&ops,&fake)==0);
    printf("USB OUT session checks passed (%d checks)\n", checks);
    return 0;
}
