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
#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    int (*initialize)(void);
    int (*terminate)(void);
    int (*install)(const char *,const char *,void *);
    int code;
} tile_api_t;
static int register_exact(void *context) {
    tile_api_t *api=context;
    api->code=api->install(SETTINGS_TILE_ID,"/user/app/",NULL);
    return api->code;
}
static int cleanup_failed;
static void *retained_module;
/* POSIX dlsym function addresses; memcpy avoids a nonportable C pointer cast. */
static int symbol(void *module,const char *name,void *function,size_t size) {
    void *address=dlsym(module,name);
    if(!address||size!=sizeof(address))return -1;
    memcpy(function,&address,size);return 0;
}
static int release_module(void *module) {
    if(dlclose(module)){retained_module=module;cleanup_failed=1;return -1;}
    return 0;
}
int settings_tile_create(char *message,size_t capacity) {
    if(cleanup_failed){snprintf(message,capacity,"Installer cleanup previously failed; restart payload before retrying");return -1;}
    void *module=dlopen("libSceAppInstUtil.sprx",RTLD_NOW|RTLD_LOCAL);
    if(!module){snprintf(message,capacity,"Optional tile installer unavailable; controller payload is unaffected");return -1;}
    tile_api_t api={0};
    if(symbol(module,"sceAppInstUtilInitialize",&api.initialize,sizeof(api.initialize))||
       symbol(module,"sceAppInstUtilTerminate",&api.terminate,sizeof(api.terminate))||
       symbol(module,"sceAppInstUtilAppInstallTitleDir",&api.install,sizeof(api.install))){
        int closed=release_module(module);
        snprintf(message,capacity,"Required tile installer API unavailable; no files changed%s",closed?"; cleanup failed, restart payload":"");return -1;
    }
    int code=api.initialize();
    if(code){
        int closed=release_module(module);
        snprintf(message,capacity,"Tile initialization failed (0x%08x); no files changed%s",(unsigned)code,closed?"; cleanup failed, restart payload":"");return -1;
    }
    int user=open("/user",O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC);
    int app=user<0?-1:openat(user,"app",O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC);
    int result=app<0?SETTINGS_TILE_FILES_FAILED:settings_tile_install(app,settings_icon,sizeof(settings_icon),
        settings_param,sizeof(settings_param),register_exact,&api);
    int file_errno=errno;
    if(app>=0&&close(app))result=SETTINGS_TILE_FILES_FAILED;
    if(user>=0&&close(user))result=SETTINGS_TILE_FILES_FAILED;
    int terminated=api.terminate();
    int closed=0;
    if(terminated){cleanup_failed=1;retained_module=module;}
    else closed=release_module(module);
    if(result==SETTINGS_TILE_REGISTER_FAILED)
        snprintf(message,capacity,"Tile registration failed (0x%08x). Owned assets retained; no global install attempted",(unsigned)api.code);
    else if(result)
        snprintf(message,capacity,"Tile files could not be safely prepared (errno %d). Existing files preserved",file_errno);
    else if(terminated)
        snprintf(message,capacity,"Registration accepted, but installer cleanup failed (0x%08x). Verify the tile on console",(unsigned)terminated);
    else snprintf(message,capacity,"Registration accepted. Check Steam Controller Settings in PS5 Media. FW 13.60 runtime is unverified");
    if(terminated && result && capacity){
        size_t used=strnlen(message,capacity);
        if(used<capacity)snprintf(message+used,capacity-used,"; cleanup 0x%08x failed, restart payload",(unsigned)terminated);
    }
    if(closed && capacity){
        size_t used=strnlen(message,capacity);
        if(used<capacity)snprintf(message+used,capacity-used,"; module cleanup failed, restart payload");
    }
    return result||terminated||closed?-1:0;
}
