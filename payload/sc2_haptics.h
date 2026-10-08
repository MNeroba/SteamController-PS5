#pragma once
#include <stdint.h>
#include <dev/usb/usb.h>
#include <dev/usb/usb_ioctl.h>
#include "slot_registry.h"
#include "usb_session.h"

/* Steam Controller (2026) haptics. Process-wide producers may queue output;
 * only the USB reader that owns the generation-tagged SC2 session may drain it. */
enum { SC2_PAD_LEFT = 0, SC2_PAD_RIGHT = 1, SC2_PAD_BOTH = 2 };

void sc2_haptic_pulse(int pad, uint16_t on_us, uint16_t off_us, uint16_t count);
void sc2_haptic_tick(void);                       /* short UI click, both sides */
void sc2_haptic_rumble_for(uint16_t left, uint16_t right, int ms);
/* Continuous rumble (game vibration): stays until changed; 0,0 stops. */
void sc2_haptic_rumble_set(uint16_t left, uint16_t right);

/* Only one Steam Controller 2026 reader may own haptic output at a time.
 * The USB manager claims its generation-tagged slot token before starting the
 * reader; failed startup abandons it, and the reader releases it at teardown.
 * This is a deliberate single-session support limit until every shared Steam
 * subsystem has an explicit multi-session contract. */
int sc2_haptic_session_claim(slot_registry_token_t owner);
int sc2_haptic_session_is_owner(slot_registry_token_t owner);
void sc2_haptic_session_abandon(slot_registry_token_t owner);

/* USB thread: send pending reports through its tracked session. */
#define SC2_HAP_MAX_OUT 6
/* eps[1] = own interface's interrupt OUT (when out_opened);
 * eps[2 .. 2+n_extra-1] = other interfaces' OUTs (broadcast test, method 4). */
void sc2_haptic_service(slot_registry_token_t owner,
                        usb_resources_t *resources,
                        const usb_session_ops_t *ops,
                        struct usb_fs_endpoint *eps, int out_opened,
                        int iface, int n_extra);
/* Drop queued/session rumble state and send a final rumble stop before the
 * caller cleans up the initialized USB session. A closed session skips I/O. */
int sc2_haptic_session_release(slot_registry_token_t owner,
                               usb_resources_t *resources,
                               const usb_session_ops_t *ops,
                               struct usb_fs_endpoint *eps, int out_opened,
                               int iface, int n_extra);
