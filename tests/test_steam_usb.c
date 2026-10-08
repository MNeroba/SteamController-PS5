#include "steam_usb.h"

#include <stdio.h>
#include <string.h>

static int failures;
static int checks;

#define CHECK(cond) do { \
    checks++; \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        failures++; \
    } \
} while (0)

static const uint8_t valid_config[] = {
    9, 2, 89, 0, 4, 1, 0, 0x80, 50,
    9, 4, 0, 0, 0, 3, 0, 0, 0,
    9, 4, 2, 0, 2, 3, 0, 0, 0,
    7, 5, 0x84, 3, 64, 0, 10,
    7, 5, 0x04, 3, 64, 0, 10,
    9, 4, 2, 1, 1, 3, 0, 0, 0,
    7, 5, 0x89, 3, 64, 0, 10,
    9, 4, 3, 0, 1, 3, 0, 0, 0,
    7, 5, 0x85, 3, 64, 0, 10,
    9, 4, 6, 0, 1, 3, 0, 0, 0,
    7, 5, 0x86, 3, 64, 0, 10
};

static void test_complete_configuration_map(void) {
    steam_usb_map_t map;
    CHECK(steam_usb_parse_config(valid_config, sizeof(valid_config), 0, &map) == 4);
    CHECK(map.n == 4);
    CHECK(map.it[1].iface == 2 && map.it[1].iclass == 3);
    CHECK(map.it[1].in_ep == 0x84 && map.it[1].out_ep == 0x04);
    CHECK(map.it[2].iface == 3 && map.it[2].in_ep == 0x85);
    CHECK(map.it[3].iface == 6 && map.it[3].in_ep == 0x86);
}

static void test_puck_interface_filter(void) {
    steam_usb_map_t map;
    CHECK(steam_usb_parse_config(valid_config, sizeof(valid_config), 1, &map) == 2);
    CHECK(map.n == 2);
    CHECK(map.it[0].iface == 2 && map.it[0].in_ep == 0x84);
    CHECK(map.it[1].iface == 3 && map.it[1].in_ep == 0x85);
}

static void test_invalid_descriptors(void) {
    uint8_t bad_total[] = { 9, 2, 20, 0, 0, 1, 0, 0x80, 50 };
    uint8_t zero_length[] = { 9, 2, 11, 0, 0, 1, 0, 0x80, 50, 0, 4 };
    uint8_t overrun[] = { 9, 2, 11, 0, 0, 1, 0, 0x80, 50, 3, 4 };
    uint8_t short_interface[] = { 9, 2, 11, 0, 1, 1, 0, 0x80, 50, 2, 4 };
    uint8_t short_endpoint[] = {
        9, 2, 22, 0, 1, 1, 0, 0x80, 50,
        9, 4, 2, 0, 1, 3, 0, 0, 0,
        4, 5, 0x84, 3
    };
    uint8_t no_interface[] = { 9, 2, 16, 0, 0, 1, 0, 0x80, 50, 7, 5, 0x84, 3, 64, 0, 10 };
    steam_usb_map_t map;

    CHECK(steam_usb_parse_config(NULL, 0, 0, &map) == -1);
    CHECK(steam_usb_parse_config(valid_config, 8, 0, &map) == -1);
    CHECK(steam_usb_parse_config(valid_config, sizeof(valid_config) - 1, 0, &map) == -1);
    CHECK(steam_usb_parse_config(bad_total, sizeof(bad_total), 0, &map) == -1);
    CHECK(steam_usb_parse_config(zero_length, sizeof(zero_length), 0, &map) == -1);
    CHECK(steam_usb_parse_config(overrun, sizeof(overrun), 0, &map) == -1);
    CHECK(steam_usb_parse_config(short_interface, sizeof(short_interface), 0, &map) == -1);
    CHECK(steam_usb_parse_config(short_endpoint, sizeof(short_endpoint), 0, &map) == -1);
    CHECK(steam_usb_parse_config(no_interface, sizeof(no_interface), 0, &map) == -1);
}

int main(void) {
    test_complete_configuration_map();
    test_puck_interface_filter();
    test_invalid_descriptors();
    if (failures) {
        fprintf(stderr, "%d of %d Steam USB descriptor checks failed\n", failures, checks);
        return 1;
    }
    printf("Steam USB descriptor checks passed (%d checks)\n", checks);
    return 0;
}
