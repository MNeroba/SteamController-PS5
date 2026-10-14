#include "steam_usb.h"

#include <string.h>

#define USB_DESC_CONFIG    0x02
#define USB_DESC_INTERFACE 0x04
#define USB_DESC_ENDPOINT  0x05
#define USB_EP_INTERRUPT   0x03

int steam_usb_device_identity(const uint8_t *data, unsigned length,
                              uint16_t *vendor_id, uint16_t *product_id) {
    if (!data || !vendor_id || !product_id || length != 18 ||
        data[0] != 18 || data[1] != 1) return 0;
    *vendor_id = (uint16_t)(data[8] | ((uint16_t)data[9] << 8));
    *product_id = (uint16_t)(data[10] | ((uint16_t)data[11] << 8));
    return 1;
}
int steam_usb_identity_matches(uint16_t expected_vid, uint16_t expected_pid,
                               uint16_t actual_vid, uint16_t actual_pid) {
    return steam_usb_is_sc2_device(expected_vid,expected_pid) &&
        expected_vid==actual_vid && expected_pid==actual_pid;
}

int steam_usb_parse_config(const uint8_t *data, unsigned length,
                           int puck_only, steam_usb_map_t *out) {
    unsigned total, offset, interface_count = 0;
    unsigned current_expected_endpoints = 0, current_endpoint_count = 0;
    uint8_t seen_default_interface[256] = {0};
    int current = -1;
    int have_interface = 0;
    if (!data || !out) return -1;
    memset(out, 0, sizeof(*out));
    /* USB 2.0 configuration descriptors have a fixed nine-byte prefix. */
    if (length < 9 || data[0] != 9 || data[1] != USB_DESC_CONFIG)
        return -1;
    total = (unsigned)data[2] | ((unsigned)data[3] << 8);
    if (total < data[0] || total > length) return -1;

    offset = data[0];
    while (offset < total) {
        unsigned descriptor_length;
        uint8_t type;
        if (total - offset < 2) return -1;
        descriptor_length = data[offset];
        type = data[offset + 1];
        if (descriptor_length < 2 || descriptor_length > total - offset) return -1;

        if (type == USB_DESC_INTERFACE) {
            uint8_t iface, alt;
            /* Standard interface descriptors are exactly nine bytes. */
            if (descriptor_length != 9) return -1;
            if (have_interface &&
                current_endpoint_count != current_expected_endpoints)
                return -1;
            have_interface = 1;
            current = -1;
            iface = data[offset + 2];
            alt = data[offset + 3];
            current_expected_endpoints = data[offset + 4];
            current_endpoint_count = 0;
            if (alt == 0) {
                if (seen_default_interface[iface]) return -1;
                seen_default_interface[iface] = 1;
                interface_count++;
                if (interface_count > data[4]) return -1;
                if (!puck_only || (iface >= 2 && iface <= 5)) {
                    if (out->n >= STEAM_USB_MAX_INTERFACES) return -1;
                    current = out->n++;
                    out->it[current].iface = iface;
                    out->it[current].iclass = data[offset + 5];
                }
            }
        } else if (type == USB_DESC_ENDPOINT) {
            uint8_t address, attributes;
            /* Standard endpoint descriptors are exactly seven bytes. */
            if (descriptor_length != 7 || !have_interface) return -1;
            current_endpoint_count++;
            if (current_endpoint_count > current_expected_endpoints)
                return -1;
            if (current >= 0) {
                address = data[offset + 2];
                attributes = data[offset + 3];
                if ((attributes & 0x03) == USB_EP_INTERRUPT) {
                    if (address & 0x80) {
                        if (!out->it[current].in_ep) out->it[current].in_ep = address;
                    } else if (!out->it[current].out_ep) {
                        out->it[current].out_ep = address;
                    }
                }
            }
        }
        offset += descriptor_length;
    }
    if (have_interface && current_endpoint_count != current_expected_endpoints)
        return -1;
    if (interface_count != data[4]) return -1;
    return out->n;
}

int steam_usb_is_sc2_device(uint16_t vendor_id, uint16_t product_id) {
    return vendor_id == STEAM_USB_VENDOR_VALVE &&
           (product_id == STEAM_USB_PID_SC2_WIRED ||
            product_id == STEAM_USB_PID_SC2_PUCK);
}

static int usb_endpoint_address_valid(uint8_t address, int input) {
    return (address & 0x0fu) != 0 && (address & 0x70u) == 0 &&
           (((address & 0x80u) != 0) == (input != 0));
}

static int detach_map_is_well_formed(const steam_usb_map_t *map) {
    if (!map || map->n <= 0 || map->n > STEAM_USB_MAX_INTERFACES)
        return 0;
    for (int i = 0; i < map->n; ++i) {
        if (map->it[i].in_ep &&
            !usb_endpoint_address_valid(map->it[i].in_ep, 1))
            return 0;
        if (map->it[i].out_ep &&
            !usb_endpoint_address_valid(map->it[i].out_ep, 0))
            return 0;
        for (int j = i + 1; j < map->n; ++j)
            if (map->it[i].iface == map->it[j].iface)
                return 0;
    }
    return 1;
}

int steam_usb_detach_interface_allowed(uint16_t vendor_id, uint16_t product_id,
                                       const steam_usb_map_t *map,
                                       uint8_t interface_number) {
    if (!steam_usb_is_sc2_device(vendor_id, product_id) ||
        !detach_map_is_well_formed(map) ||
        (product_id == STEAM_USB_PID_SC2_PUCK &&
         (interface_number < 2 || interface_number > 5)))
        return 0;
    for (int i = 0; i < map->n; ++i) {
        if (map->it[i].iface == interface_number &&
            map->it[i].iclass == 0x03 &&
            map->it[i].in_ep != 0)
            return 1;
    }
    return 0;
}

int steam_usb_collect_detach_interfaces(uint16_t vendor_id, uint16_t product_id,
                                        const steam_usb_map_t *map,
                                        uint8_t *out, int capacity) {
    if (!steam_usb_is_sc2_device(vendor_id, product_id))
        return 0;
    if (!map || !out || capacity <= 0 || !detach_map_is_well_formed(map))
        return -1;

    uint8_t selected[STEAM_USB_MAX_INTERFACES];
    int count = 0;
    for (int i = 0; i < map->n; ++i) {
        uint8_t iface = map->it[i].iface;
        if (!steam_usb_detach_interface_allowed(vendor_id, product_id,
                                                map, iface))
            continue;
        selected[count++] = iface;
    }
    if (count > capacity)
        return -1;
    memcpy(out, selected, (size_t)count);
    return count;
}
