#ifndef WEBUI_AUTH_H
#define WEBUI_AUTH_H

#include <pthread.h>
#include <stdint.h>
#include "storage_status.h"

#define WEBUI_TOKEN_HEX_LENGTH 64

int webui_token_is_valid(const char *token);
int webui_auth_matches(const char *authorization, const char *token);
int webui_auth_path_allows(const char *path, const char *authorization,
                           const char *token);
/* Load an existing private 256-bit token or create one. Returns 0 on success,
 * -1 on insecure/malformed storage or I/O failure. `created` is optional. */
int webui_auth_load_or_create(const char *directory,
                              char token[WEBUI_TOKEN_HEX_LENGTH + 1],
                              int *created);
int webui_auth_load_or_create_report(const char *directory,
                                     char token[WEBUI_TOKEN_HEX_LENGTH + 1],
                                     int *created, storage_status_t *status);

/* Session credentials retain all bearer/pairing gates but never touch the
 * failed storage again. Malformed/foreign/insecure existing tokens and entropy
 * failures remain fatal. Operational failures preserve their stage/errno. */
typedef enum { WEBUI_AUTH_FAILED=-1, WEBUI_AUTH_PERSISTENT=0,
               WEBUI_AUTH_SESSION=1 } webui_auth_mode_t;
webui_auth_mode_t webui_auth_begin(const char *directory,
                                  char token[WEBUI_TOKEN_HEX_LENGTH+1],
                                  storage_status_t *status);

#define WEBUI_PAIR_CODE_LENGTH 8
#define WEBUI_PAIR_TTL_MS 300000u
#define WEBUI_PAIR_ATTEMPTS 5u
typedef struct {
    pthread_mutex_t lock;
    uint64_t started_ms;
    char code[WEBUI_PAIR_CODE_LENGTH + 1];
    unsigned attempts;
    int active;
} webui_pairing_t;
/* Initialize before serving requests. A fresh short-lived CSPRNG code is
 * output only for local notification. One redemption, five attempts total;
 * expiry or backward clock disables enrollment until a new local launch.
 * Destroy only after all callers have joined. Neither function logs secrets. */
int webui_pairing_init(webui_pairing_t *pairing, uint64_t now_ms,
                       char code[WEBUI_PAIR_CODE_LENGTH + 1]);
int webui_pairing_redeem(webui_pairing_t *pairing, const char *code,
                         uint64_t now_ms);
void webui_pairing_destroy(webui_pairing_t *pairing);

#endif
