/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef SETTINGS_TILE_H
#define SETTINGS_TILE_H
#include <stddef.h>
#define SETTINGS_TILE_ID "SCPS50001"
#define SETTINGS_TILE_CONFIRM "INSTALL " SETTINGS_TILE_ID
#define SETTINGS_TILE_FILES_FAILED (-1)
#define SETTINGS_TILE_REGISTER_FAILED (-2)
/* app_fd is an already validated /user/app directory (temp directory on host).
 * Never overwrites existing bytes. Failed registration retains owned assets,
 * because the platform may already have partially registered the title. */
int settings_tile_install(int app_fd, const unsigned char *icon, size_t icon_size,
                          const unsigned char *param, size_t param_size,
                          int (*register_title)(void *), void *context);
/* 1 only when a previous registration succeeded (marker) and the embedded
 * assets still match byte-for-byte. Used to skip automatic re-registration
 * without confusing staged files for a registered tile. */
int settings_tile_installed(int app_fd, const unsigned char *icon, size_t icon_size,
                            const unsigned char *param, size_t param_size);
/* Native adapter, called only by an authenticated explicit POST. */
int settings_tile_create(char *message, size_t capacity);
/* Native adapter: 1 = registered and current, 0 = absent/stale,
 * -1 = state cannot be determined; never install on -1. */
int settings_tile_is_registered(void);
#endif
