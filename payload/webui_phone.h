#ifndef WEBUI_PHONE_H
#define WEBUI_PHONE_H

#include "webui_auth.h"
#include <stdint.h>

typedef struct {
    uint64_t generation;
    uint32_t peer;
} webui_phone_peer_t;

typedef struct {
    int enabled;
    int paired;
    char host[22]; /* IPv4 literal, colon, port and terminator */
} webui_phone_info_t;

/* A new pairing code revokes the preceding phone credential. Times are monotonic. */
int webui_phone_start(unsigned port, uint64_t now, char code[WEBUI_PAIR_CODE_LENGTH + 1]);
void webui_phone_stop(void);
void webui_phone_info(uint64_t now, webui_phone_info_t *info);
/* The accept loop owns the returned duplicate until it closes it. */
int webui_phone_listener(uint64_t now, uint64_t *generation);
int webui_phone_peer_allowed(webui_phone_peer_t peer, uint64_t now);
int webui_phone_host_matches(webui_phone_peer_t peer, const char *host, uint64_t now);
int webui_phone_authorized(webui_phone_peer_t peer, const char *authorization, uint64_t now);
int webui_phone_pair(webui_phone_peer_t peer, const char *code, uint64_t now,
                      char token[WEBUI_TOKEN_HEX_LENGTH + 1]);

#endif
