#ifndef WEBUI_AUTH_H
#define WEBUI_AUTH_H

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

#endif
