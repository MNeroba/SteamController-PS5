/* SPDX-License-Identifier: GPL-3.0-or-later
 * AppInstallTitleDir API reference: SDK v0.43 sample by John Törnblom (2025).
 * Media/deeplink schema reference: owendswang/ps5-web-file-manager.
 * FW 13.60 export NIDs: xdCloudy/PS5-13.60-Function-Stub-Library.
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
#ifdef __PROSPERO__
#include <ps5/kernel.h>
#endif

/* System SPRX exports are NID-indexed; public C names are not guaranteed.
 * Resolve by the firmware NID first and fall back to the exported name. */
#define TILE_NID_INIT  "540lotO7oHE"
#define TILE_NID_TERM  "kLLazhNh6d4"
#define TILE_NID_TITLE "Wudg3Xe3heE"

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

/* POSIX dlsym function addresses; memcpy avoids a nonportable C pointer cast. */
static int symbol(void *module,const char *name,void *function,size_t size) {
    void *address=dlsym(module,name);
    if(!address||size!=sizeof(address))return -1;
    memcpy(function,&address,size);return 0;
}
#ifndef SETTINGS_TILE_TEST
static int nid_symbol(const char *nid,void *function,size_t size) {
#ifdef __PROSPERO__
    uint32_t handle=0;
    if(kernel_dynlib_handle(-1,"libSceAppInstUtil.sprx",&handle)!=0)return -1;
    void *address=(void*)kernel_dynlib_resolve(-1,handle,nid);
    if(!address||size!=sizeof(address))return -1;
    memcpy(function,&address,size);return 0;
#else
    (void)nid;(void)function;(void)size;return -1;
#endif
}
#define TILE_NID_LOOKUP(nid,fn) nid_symbol((nid),(fn),sizeof(fn))
#else
/* Host tests inject NID lookups without entering the kernel ABI. */
int settings_tile_test_nid_lookup(const char *nid,void *function,size_t size);
#define TILE_NID_LOOKUP(nid,fn) settings_tile_test_nid_lookup((nid),(fn),sizeof(fn))
#endif

static int resolve(void *module,const char *nid,const char *name,void *function,
                   size_t size) {
    return TILE_NID_LOOKUP(nid,function)==0 ? 0
                                           : symbol(module,name,function,size);
}

/* The installer module is a system SPRX. Keeping it mapped for the payload
 * lifetime avoids an unload that can fail and must not disable later retries:
 * registration is a user-visible, retryable action and no files are changed
 * when the capability probe fails. */
static int append(char *message,size_t capacity,const char *text) {
    size_t used=strnlen(message,capacity);
    if(used>=capacity)return 0;
    int n=snprintf(message+used,capacity-used,"%s",text);
    return n>0&&(size_t)n<capacity-used;
}

int settings_tile_is_registered(void) {
    int user=open("/user",O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC);
    if(user<0)return errno==ENOENT?0:-1;
    int app=openat(user,"app",O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC);
    int saved=errno;close(user);
    if(app<0)return saved==ENOENT?0:-1;
    int result=settings_tile_installed(app,settings_icon,sizeof(settings_icon),
                                       settings_param,sizeof(settings_param));
    close(app);
    return result?1:0;
}

int settings_tile_create(char *message,size_t capacity) {
    if(!message||!capacity)return -1;
    message[0]='\0';
    void *module=dlopen("libSceAppInstUtil.sprx",RTLD_NOW|RTLD_LOCAL);
    if(!module){snprintf(message,capacity,"Optional tile installer unavailable; controller payload is unaffected");return -1;}
    tile_api_t api={0};
    int have_init=resolve(module,TILE_NID_INIT,"sceAppInstUtilInitialize",
                          &api.initialize,sizeof(api.initialize))==0;
    int have_term=resolve(module,TILE_NID_TERM,"sceAppInstUtilTerminate",
                          &api.terminate,sizeof(api.terminate))==0;
    int have_title=resolve(module,TILE_NID_TITLE,"sceAppInstUtilAppInstallTitleDir",
                           &api.install,sizeof(api.install))==0;
    if(!have_init||!have_term||!have_title){
        snprintf(message,capacity,
                 "Required tile installer API unavailable (%s%s%s); no files changed; retry is safe",
                 have_init?"":"initialize ",have_term?"":"terminate ",have_title?"":"install");
        return -1;
    }
    int code=api.initialize();
    if(code){
        snprintf(message,capacity,"Tile initialization failed (0x%08x); no files changed; retry is safe",(unsigned)code);
        return -1;
    }
    int user=open("/user",O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC);
    int app=user<0?-1:openat(user,"app",O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC);
    int file_errno=errno;
    int result=app<0?SETTINGS_TILE_FILES_FAILED:settings_tile_install(app,settings_icon,sizeof(settings_icon),
        settings_param,sizeof(settings_param),register_exact,&api);
    if(app>=0&&close(app))result=SETTINGS_TILE_FILES_FAILED;
    if(user>=0&&close(user))result=SETTINGS_TILE_FILES_FAILED;
    int terminated=api.terminate();
    if(result==SETTINGS_TILE_REGISTER_FAILED)
        snprintf(message,capacity,"Tile registration failed (0x%08x). Owned assets retained; no global install attempted",(unsigned)api.code);
    else if(result)
        snprintf(message,capacity,"Tile files could not be safely prepared (errno %d). Existing files preserved",file_errno);
    else if(terminated)
        snprintf(message,capacity,"Registration accepted, but installer cleanup failed (0x%08x). Verify the tile on console",(unsigned)terminated);
    else snprintf(message,capacity,"Registration accepted. Check Steam Controller Settings in PS5 Media. FW 13.60 runtime is unverified");
    if(terminated && result) append(message,capacity,"; installer cleanup also failed");
    /* A failed Terminate/unload never invalidates an accepted registration and
     * never blocks a later retry. */
    return result?-1:0;
}
