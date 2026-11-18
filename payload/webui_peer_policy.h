#ifndef WEBUI_PEER_POLICY_H
#define WEBUI_PEER_POLICY_H

#include <stdint.h>

/* The argument is an IPv4 address in network byte order. */
int webui_peer_ipv4_is_lan(uint32_t address_be);

#endif
