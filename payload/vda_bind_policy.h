/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once
#include <stdint.h>
static inline int vda_user_list_valid(const int32_t users[4]) {
    if (!users)
        return 0;
    for (unsigned i = 0; i < 4; i++) {
        if (users[i] == -1)
            continue;
        if (users[i] <= 0)
            return 0;
        for (unsigned j = 0; j < i; j++)
            if (users[j] == users[i])
                return 0;
    }
    return 1;
}
/* A native read error/unknown state is NOT an absent controller. Only a fresh
 * exactly-one-user list with matching foreground and a successfully read,
 * disconnected native pad permits explicit Steam/PS-triggered local binding.
 * No native operation is performed by this pure policy. */
static inline int32_t vda_bind_target(int list_result, const int32_t users[4],
                                      int foreground_result, int32_t foreground,
                                      int native_read_result, int native_connected) {
    if (list_result || foreground_result || native_read_result || native_connected ||
        !vda_user_list_valid(users) || foreground <= 0)
        return -1;
    unsigned count = 0;
    for (unsigned i = 0; i < 4; i++) {
        if (users[i] == -1)
            continue;
        if (users[i] <= 0 || users[i] != foreground)
            return -1;
        count++;
    }
    return count == 1 ? foreground : -1;
}
