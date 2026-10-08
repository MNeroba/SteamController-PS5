#include "webui_peer_policy.h"

#include <arpa/inet.h>

int webui_peer_ipv4_is_lan(uint32_t address_be) {
    uint32_t address = ntohl(address_be);
    unsigned first = (unsigned)(address >> 24);
    unsigned second = (unsigned)((address >> 16) & 0xffu);

    if (first == 127u || first == 10u) return 1;
    if (first == 172u && second >= 16u && second <= 31u) return 1;
    if (first == 192u && second == 168u) return 1;
    if (first == 169u && second == 254u) return 1;
    return 0;
}
