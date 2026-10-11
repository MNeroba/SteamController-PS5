/* SPDX-License-Identifier: GPL-3.0-or-later
 * AppInstallTitleDir API reference: SDK v0.43 sample by John Törnblom (2025).
 * Media/deeplink schema reference: owendswang/ps5-web-file-manager.
 * Pins, licenses and owner-supplied image rights: docs/PROVENANCE.md.
 * This is an opt-in registration request, not verified FW compatibility. */
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#ifndef _DARWIN_C_SOURCE
#define _DARWIN_C_SOURCE
#endif
#include "settings_tile.h"
#include "settings_tile_assets.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int sceAppInstUtilInitialize(void);
int sceAppInstUtilTerminate(void);
int sceAppInstUtilAppInstallTitleDir(const char *,const char *,void *);
static int register_exact(void *context) {
    int *code=context;
    *code=sceAppInstUtilAppInstallTitleDir(SETTINGS_TILE_ID,"/user/app/",NULL);
    return *code;
}
static int cleanup_failed;
int settings_tile_create(char *message,size_t capacity) {
    if(cleanup_failed){snprintf(message,capacity,"Installer cleanup previously failed; restart payload before retrying");return -1;}
    int code=sceAppInstUtilInitialize();
    if(code){snprintf(message,capacity,"Tile initialization failed (0x%08x); no files changed",(unsigned)code);return -1;}
    int user=open("/user",O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC);
    int app=user<0?-1:openat(user,"app",O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC);
    int result=app<0?SETTINGS_TILE_FILES_FAILED:settings_tile_install(app,settings_icon,sizeof(settings_icon),
        settings_param,sizeof(settings_param),register_exact,&code);
    int file_errno=errno;
    if(app>=0&&close(app))result=SETTINGS_TILE_FILES_FAILED;
    if(user>=0&&close(user))result=SETTINGS_TILE_FILES_FAILED;
    int terminated=sceAppInstUtilTerminate();
    if(terminated)cleanup_failed=1;
    if(result==SETTINGS_TILE_REGISTER_FAILED)
        snprintf(message,capacity,"Tile registration failed (0x%08x). Owned assets retained; no global install attempted",(unsigned)code);
    else if(result)
        snprintf(message,capacity,"Tile files could not be safely prepared (errno %d). Existing files preserved",file_errno);
    else if(terminated)
        snprintf(message,capacity,"Registration accepted, but installer cleanup failed (0x%08x). Verify the tile on console",(unsigned)terminated);
    else snprintf(message,capacity,"Registration accepted. Check Steam Controller Settings in PS5 Media. FW 13.60 runtime is unverified");
    if(terminated && result && capacity){
        size_t used=strnlen(message,capacity);
        if(used<capacity)snprintf(message+used,capacity-used,"; cleanup 0x%08x failed, restart payload",(unsigned)terminated);
    }
    return result||terminated?-1:0;
}
