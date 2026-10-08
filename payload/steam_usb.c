#include "steam_usb.h"

#include <string.h>

#define USB_DESC_CONFIG    0x02
#define USB_DESC_INTERFACE 0x04
#define USB_DESC_ENDPOINT  0x05
#define USB_EP_INTERRUPT   0x03

int steam_usb_parse_config(const uint8_t *data, unsigned length,
                           int puck_only, steam_usb_map_t *out) {
    unsigned total, offset, interface_count = 0;
    unsigned current_expected_endpoints = 0, current_endpoint_count = 0;
    uint8_t seen_default_interface[256] = {0};
    int current = -1;
    int have_interface = 0;
    if (!data || !out) return -1;
    memset(out, 0, sizeof(*out));
    if (length < 9 || data[0] < 9 || data[1] != USB_DESC_CONFIG || data[0] > length)
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
            if (descriptor_length < 9) return -1;
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
            if (descriptor_length < 7 || !have_interface) return -1;
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
