#include "controller_steam.h"

#include <pthread.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    const uint8_t *report;
    uint8_t expected_x;
    uint8_t expected_y;
    int failed;
} worker_args_t;

static void *run_session(void *opaque) {
    worker_args_t *args = opaque;
    steam_legacy_input_state_t state = {0};
    ScePadData output = {0};

    for (int i = 0; i < 10000; ++i) {
        if (!steam_handle_packet(args->report, 40, &output, &state) ||
            output.leftStick.x != args->expected_x ||
            output.leftStick.y != args->expected_y) {
            args->failed = 1;
            break;
        }
    }
    return NULL;
}

int main(void) {
    uint8_t stick_report[40] = {0};
    uint8_t pad_report[40] = {0};
    ScePadData stick_output = {0}, pad_output = {0};
    steam_legacy_input_state_t stick_session = {0};
    steam_legacy_input_state_t pad_session = {0};

    stick_report[0] = 0x01;
    stick_report[2] = 0x01;
    stick_report[16] = 0x20;   /* left stick X = 20000 */
    stick_report[17] = 0x4e;

    pad_report[0] = 0x01;
    pad_report[2] = 0x01;
    pad_report[10] = 0x88;     /* left pad touched, joystick emulation active */

    if (!steam_handle_packet(stick_report, sizeof(stick_report), &stick_output,
                             &stick_session) ||
        !steam_handle_packet(pad_report, sizeof(pad_report), &pad_output,
                             &pad_session)) {
        fprintf(stderr, "valid legacy Steam reports were rejected\n");
        return 1;
    }
    if (pad_output.leftStick.x != 128 || pad_output.leftStick.y != 128) {
        fprintf(stderr, "legacy Steam session inherited another stick state: x=%u y=%u\n",
                pad_output.leftStick.x, pad_output.leftStick.y);
        return 1;
    }

    worker_args_t workers[2] = {
        {stick_report, 206, 128, 0},
        {pad_report, 128, 128, 0}
    };
    pthread_t threads[2];
    if (pthread_create(&threads[0], NULL, run_session, &workers[0]) != 0) {
        fprintf(stderr, "could not start legacy Steam session workers\n");
        return 1;
    }
    if (pthread_create(&threads[1], NULL, run_session, &workers[1]) != 0) {
        (void)pthread_join(threads[0], NULL);
        fprintf(stderr, "could not start legacy Steam session workers\n");
        return 1;
    }
    int join_first = pthread_join(threads[0], NULL);
    int join_second = pthread_join(threads[1], NULL);
    if (join_first != 0 || join_second != 0 || workers[0].failed || workers[1].failed) {
        fprintf(stderr, "concurrent legacy Steam sessions contaminated stick state\n");
        return 1;
    }

    puts("legacy Steam Controller stick history is session-owned");
    return 0;
}
