#include "steam_protocol.h"
#include "steam_usb.h"

#include <stdio.h>
#include <string.h>

enum { INPUT_SIZE = 64, FUZZ_ROUNDS = 100000 };

static uint32_t rng_state = UINT32_C(0x6d2b79f5);

static uint32_t next_random(void) {
    uint32_t value = rng_state;
    value ^= value << 13;
    value ^= value >> 17;
    value ^= value << 5;
    rng_state = value;
    return value;
}

static int fail(unsigned round, const char *message) {
    fprintf(stderr, "parser property failure at round %u: %s\n", round,
            message);
    return 1;
}

static int fuzz_steam_reports(void) {
    static const uint8_t known_ids[] = {0x42, 0x45, 0x43, 0x79};
    uint8_t bytes[INPUT_SIZE];
    steam_report_t report;

    for (unsigned round = 0; round < FUZZ_ROUNDS; round++) {
        for (size_t i = 0; i < sizeof(bytes); i++)
            bytes[i] = (uint8_t)next_random();
        if ((round & 3u) != 3u)
            bytes[0] = known_ids[round & 3u];
        size_t length = next_random() % (INPUT_SIZE + 1u);

        int result = steam_report_decode(bytes, length, &report);
        if (result < -1 || result > 1)
            return fail(round, "decoder return outside documented range");
        if (result == 1) {
            if ((bytes[0] == 0x42 || bytes[0] == 0x45) &&
                report.kind != STEAM_REPORT_INPUT)
                return fail(round, "input report classified incorrectly");
            if (bytes[0] == 0x43 && report.kind != STEAM_REPORT_BATTERY)
                return fail(round, "battery report classified incorrectly");
            if (bytes[0] == 0x79 && report.kind != STEAM_REPORT_WIRELESS)
                return fail(round, "wireless report classified incorrectly");
            if (report.kind == STEAM_REPORT_INPUT &&
                (report.data.input.wire_length != length ||
                 report.data.input.has_imu != (length >= 46)))
                return fail(round, "input length/IMU boundary is inconsistent");
        } else if ((bytes[0] == 0x42 || bytes[0] == 0x45) && length > 0 &&
                   result == -1 && report.kind != STEAM_REPORT_INPUT) {
            return fail(round, "truncated known input lost neutralization kind");
        }
    }
    return 0;
}

static int fuzz_usb_configurations(void) {
    static const uint8_t simple_config[] = {
        9, 2, 25, 0, 1, 1, 0, 0x80, 50,
        9, 4, 2, 0, 1, 3, 0, 0, 0,
        7, 5, 0x84, 3, 64, 0, 10
    };
    uint8_t bytes[INPUT_SIZE];
    steam_usb_map_t map;

    for (unsigned round = 0; round < FUZZ_ROUNDS; round++) {
        for (size_t i = 0; i < sizeof(bytes); i++)
            bytes[i] = (uint8_t)next_random();
        unsigned length = next_random() % (INPUT_SIZE + 1u);
        if ((round & 1u) == 0) {
            memcpy(bytes, simple_config, sizeof(simple_config));
            length = sizeof(simple_config);
            bytes[next_random() % sizeof(simple_config)] ^=
                (uint8_t)(next_random() | 1u);
        }

        int result = steam_usb_parse_config(bytes, length, round & 1u, &map);
        if (result < -1 || result > STEAM_USB_MAX_INTERFACES)
            return fail(round, "USB parser return outside documented range");
        if (result >= 0 && (result != map.n || map.n > STEAM_USB_MAX_INTERFACES))
            return fail(round, "USB parser interface count is inconsistent");
        if (result >= 0 && (round & 1u)) {
            for (int i = 0; i < map.n; i++) {
                if (map.it[i].iface < 2 || map.it[i].iface > 5)
                    return fail(round, "Puck filter returned an unrelated interface");
            }
        }
    }
    return 0;
}

int main(void) {
    if (fuzz_steam_reports() || fuzz_usb_configurations()) return 1;
    printf("Steam parser properties passed (%u report + %u USB descriptor inputs)\n",
           FUZZ_ROUNDS, FUZZ_ROUNDS);
    return 0;
}
