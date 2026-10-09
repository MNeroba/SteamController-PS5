#ifndef STEAM_USB_H
#define STEAM_USB_H

#include <stdint.h>

#define STEAM_USB_MAX_INTERFACES 10
#define STEAM_USB_VENDOR_VALVE 0x28deu
#define STEAM_USB_PID_SC2_WIRED 0x1302u
#define STEAM_USB_PID_SC2_PUCK 0x1304u

typedef struct {
    int n;
    struct { uint8_t iface, in_ep, out_ep, iclass; } it[STEAM_USB_MAX_INTERFACES];
} steam_usb_map_t;

/* Parse one complete USB configuration descriptor. If puck_only is nonzero,
 * retain only Valve Puck controller interfaces 2 through 5. Returns interface
 * count or -1 for a truncated/malformed stream, a nonstandard length for a
 * fixed USB descriptor, descriptor-count mismatch, or a configuration that
 * exceeds the representable interface map. */
int steam_usb_parse_config(const uint8_t *data, unsigned length,
                           int puck_only, steam_usb_map_t *out);

/* USB detach authorization is restricted to the Steam 2026 USB/Puck IDs and
 * unique HID interfaces with a valid interrupt-IN endpoint in a successfully
 * parsed configuration map. Puck authorization is limited to interfaces 2-5. */
int steam_usb_is_sc2_device(uint16_t vendor_id, uint16_t product_id);
int steam_usb_detach_interface_allowed(uint16_t vendor_id, uint16_t product_id,
                                       const steam_usb_map_t *map,
                                       uint8_t interface_number);
int steam_usb_collect_detach_interfaces(uint16_t vendor_id, uint16_t product_id,
                                        const steam_usb_map_t *map,
                                        uint8_t *out, int capacity);

#endif
