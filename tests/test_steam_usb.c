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

static void test_sc2_detach_interface_policy(void) {
    steam_usb_map_t map;
    uint8_t interfaces[STEAM_USB_MAX_INTERFACES] = {0};
    CHECK(steam_usb_parse_config(valid_config, sizeof(valid_config), 0, &map) == 4);
    CHECK(steam_usb_collect_detach_interfaces(0x28de, 0x1302, &map,
                                               interfaces,
                                               STEAM_USB_MAX_INTERFACES) == 3);
    CHECK(interfaces[0] == 2 && interfaces[1] == 3 && interfaces[2] == 6);
    CHECK(steam_usb_detach_interface_allowed(0x28de, 0x1302, &map, 2));
    CHECK(!steam_usb_detach_interface_allowed(0x28de, 0x1302, &map, 0));
    CHECK(!steam_usb_detach_interface_allowed(0x045e, 0x02ea, &map, 2));
    CHECK(!steam_usb_detach_interface_allowed(0x28de, 0x1102, &map, 2));
    CHECK(steam_usb_collect_detach_interfaces(0x28de, 0x1102, &map,
                                               interfaces,
                                               STEAM_USB_MAX_INTERFACES) == 0);

    CHECK(steam_usb_parse_config(valid_config, sizeof(valid_config), 1, &map) == 2);
    CHECK(steam_usb_collect_detach_interfaces(0x28de, 0x1304, &map,
                                               interfaces,
                                               STEAM_USB_MAX_INTERFACES) == 2);
    CHECK(interfaces[0] == 2 && interfaces[1] == 3);
    CHECK(steam_usb_collect_detach_interfaces(0x28de, 0x1304, &map,
                                               interfaces, 1) == -1);

    map.it[0].iclass = 0xff;
    CHECK(!steam_usb_detach_interface_allowed(0x28de, 0x1304, &map, 2));
    map.n = STEAM_USB_MAX_INTERFACES + 1;
    CHECK(steam_usb_collect_detach_interfaces(0x28de, 0x1304, &map,
                                               interfaces,
                                               STEAM_USB_MAX_INTERFACES) == -1);

    memset(&map, 0, sizeof(map));
    map.n = 2;
    map.it[0].iface = 2; map.it[0].iclass = 0x03; map.it[0].in_ep = 0x81;
    map.it[1] = map.it[0];
    CHECK(!steam_usb_detach_interface_allowed(0x28de, 0x1302, &map, 2));
    CHECK(steam_usb_collect_detach_interfaces(0x28de, 0x1302, &map,
                                               interfaces,
                                               STEAM_USB_MAX_INTERFACES) == -1);
    map.it[1].iface = 3;
    map.it[1].iclass = 0x02;
    CHECK(!steam_usb_detach_interface_allowed(0x28de, 0x1302, &map, 3));
    map.it[1].iclass = 0x03;
    map.it[1].in_ep = 0;
    CHECK(!steam_usb_detach_interface_allowed(0x28de, 0x1302, &map, 3));
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

static void test_declared_descriptor_counts(void) {
    uint8_t bad_interface_count[sizeof(valid_config)];
    uint8_t bad_endpoint_count[sizeof(valid_config)];
    uint8_t bad_alt_endpoint_count[sizeof(valid_config)];
    static const uint8_t duplicate_default_interface[] = {
        9, 2, 27, 0, 1, 1, 0, 0x80, 50,
        9, 4, 2, 0, 0, 3, 0, 0, 0,
        9, 4, 2, 0, 0, 3, 0, 0, 0
    };
    steam_usb_map_t map;

    memcpy(bad_interface_count, valid_config, sizeof(valid_config));
    bad_interface_count[4] = 3; /* four distinct altsetting-zero interfaces */
    CHECK(steam_usb_parse_config(bad_interface_count,
                                 sizeof(bad_interface_count), 0, &map) == -1);

    memcpy(bad_endpoint_count, valid_config, sizeof(valid_config));
    bad_endpoint_count[22] = 1; /* interface 2 declares two endpoints */
    CHECK(steam_usb_parse_config(bad_endpoint_count,
                                 sizeof(bad_endpoint_count), 0, &map) == -1);

    memcpy(bad_alt_endpoint_count, valid_config, sizeof(valid_config));
    bad_alt_endpoint_count[45] = 0; /* interface 2 altsetting 1 has one */
    CHECK(steam_usb_parse_config(bad_alt_endpoint_count,
                                 sizeof(bad_alt_endpoint_count), 0, &map) == -1);

    CHECK(steam_usb_parse_config(duplicate_default_interface,
                                 sizeof(duplicate_default_interface), 0,
                                 &map) == -1);
}

static void test_interface_map_capacity(void) {
    enum { INTERFACE_COUNT = STEAM_USB_MAX_INTERFACES + 1,
           CONFIG_LENGTH = 9 + INTERFACE_COUNT * 9 };
    uint8_t config[CONFIG_LENGTH] = {
        9, 2, CONFIG_LENGTH, 0, INTERFACE_COUNT, 1, 0, 0x80, 50
    };
    steam_usb_map_t map;

    for (unsigned i = 0; i < INTERFACE_COUNT; i++) {
        unsigned offset = 9 + i * 9;
        config[offset] = 9;
        config[offset + 1] = 4;
        config[offset + 2] = (uint8_t)i;
        config[offset + 3] = 0;
        config[offset + 4] = 0;
        config[offset + 5] = 3;
    }
    CHECK(steam_usb_parse_config(config, sizeof(config), 0, &map) == -1);
}

static void test_fixed_standard_descriptor_lengths(void) {
    static const uint8_t extended_config[] = {
        10, 2, 19, 0, 1, 1, 0, 0x80, 50, 0,
        9, 4, 2, 0, 0, 3, 0, 0, 0
    };
    static const uint8_t extended_interface[] = {
        9, 2, 19, 0, 1, 1, 0, 0x80, 50,
        10, 4, 2, 0, 0, 3, 0, 0, 0, 0
    };
    static const uint8_t extended_endpoint[] = {
        9, 2, 26, 0, 1, 1, 0, 0x80, 50,
        9, 4, 2, 0, 1, 3, 0, 0, 0,
        8, 5, 0x84, 3, 64, 0, 10, 0
    };
    steam_usb_map_t map;

    CHECK(steam_usb_parse_config(extended_config, sizeof(extended_config),
                                 0, &map) == -1);
    CHECK(steam_usb_parse_config(extended_interface, sizeof(extended_interface),
                                 0, &map) == -1);
    CHECK(steam_usb_parse_config(extended_endpoint, sizeof(extended_endpoint),
                                 0, &map) == -1);
}

int main(void) {
    test_complete_configuration_map();
    test_puck_interface_filter();
    test_sc2_detach_interface_policy();
    test_invalid_descriptors();
    test_declared_descriptor_counts();
    test_interface_map_capacity();
    test_fixed_standard_descriptor_lengths();
    if (failures) {
        fprintf(stderr, "%d of %d Steam USB descriptor checks failed\n", failures, checks);
        return 1;
    }
    printf("Steam USB descriptor checks passed (%d checks)\n", checks);
    return 0;
}
