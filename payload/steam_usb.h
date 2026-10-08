#ifndef STEAM_USB_H
#define STEAM_USB_H

#include <stdint.h>

#define STEAM_USB_MAX_INTERFACES 10

typedef struct {
    int n;
    struct { uint8_t iface, in_ep, out_ep, iclass; } it[STEAM_USB_MAX_INTERFACES];
} steam_usb_map_t;

/* Parse one complete USB configuration descriptor. If puck_only is nonzero,
 * retain only Valve Puck controller interfaces 2 through 5. Returns interface
 * count or -1 for a truncated/malformed stream, descriptor-count mismatch, or
 * a configuration that exceeds the representable interface map. */
int steam_usb_parse_config(const uint8_t *data, unsigned length,
                           int puck_only, steam_usb_map_t *out);

#endif
