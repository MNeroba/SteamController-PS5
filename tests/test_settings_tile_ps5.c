/* Host stubs exercise native adapter control flow, never the console ABI. */
#define _POSIX_C_SOURCE 200809L
#define _DARWIN_C_SOURCE
#include <assert.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
static char root[128];static int init_result,register_result,terminate_result;
static int initialized,registered,terminated;
static int fixture_open(const char *path,int flags) { assert(!strcmp(path,"/user"));return open(root,flags); }
int sceAppInstUtilInitialize(void);
int sceAppInstUtilTerminate(void);
int sceAppInstUtilAppInstallTitleDir(const char *,const char *,void *);
static int missing_module,missing_symbol,close_result,module_closes;
static int module_marker;
static void *fixture_dlopen(const char *name,int flags) {
 assert(!strcmp(name,"libSceAppInstUtil.sprx")&&flags==(RTLD_NOW|RTLD_LOCAL));
 return missing_module?NULL:&module_marker;
}
static void *fixture_dlsym(void *module,const char *name) {
 assert(module==&module_marker);if(missing_symbol)return NULL;
 void *address=NULL;
 if(!strcmp(name,"sceAppInstUtilInitialize")){int(*fn)(void)=sceAppInstUtilInitialize;assert(sizeof(fn)==sizeof(address));memcpy(&address,&fn,sizeof(address));}
 else if(!strcmp(name,"sceAppInstUtilTerminate")){int(*fn)(void)=sceAppInstUtilTerminate;memcpy(&address,&fn,sizeof(address));}
 else{assert(!strcmp(name,"sceAppInstUtilAppInstallTitleDir"));int(*fn)(const char*,const char*,void*)=sceAppInstUtilAppInstallTitleDir;memcpy(&address,&fn,sizeof(address));}
 return address;
}
static int fixture_dlclose(void *module) { assert(module==&module_marker);module_closes++;return close_result; }
#define dlopen fixture_dlopen
#define dlsym fixture_dlsym
#define dlclose fixture_dlclose
#define open fixture_open
#include "../payload/settings_tile_ps5.c"
#undef open
#undef dlopen
#undef dlsym
#undef dlclose
int sceAppInstUtilInitialize(void) { initialized++;return init_result; }
int sceAppInstUtilTerminate(void) { terminated++;return terminate_result; }
int sceAppInstUtilAppInstallTitleDir(const char *id,const char *dir,void *unused) {
 assert(!strcmp(id,"SCPS50001")&&!strcmp(dir,"/user/app/")&&!unused);registered++;return register_result;
}
int main(void) {
 strcpy(root,"/tmp/sc-tile-adapter-XXXXXX");assert(mkdtemp(root));char message[256];
 missing_module=1;assert(settings_tile_create(message,sizeof(message))==-1&&initialized==0);
 assert(strstr(message,"unavailable"));missing_module=0;missing_symbol=1;
 assert(settings_tile_create(message,sizeof(message))==-1&&initialized==0&&module_closes==1);
 missing_symbol=0;
 init_result=13;assert(settings_tile_create(message,sizeof(message))==-1);
 assert(initialized==1&&terminated==0&&registered==0&&strstr(message,"initialization failed"));
 init_result=0;assert(settings_tile_create(message,sizeof(message))==-1);
 assert(terminated==1&&registered==0&&strstr(message,"files could not"));
 int user=open(root,O_RDONLY|O_DIRECTORY);assert(user>=0);assert(mkdirat(user,"app",0755)==0);
 register_result=17;assert(settings_tile_create(message,sizeof(message))==-1);
 assert(registered==1&&terminated==2&&strstr(message,"0x00000011"));
 register_result=0;assert(settings_tile_create(message,sizeof(message))==0);
 assert(registered==2&&terminated==3&&strstr(message,"Registration accepted"));
 register_result=17;terminate_result=19;assert(settings_tile_create(message,sizeof(message))==-1&&registered==3);
 assert(strstr(message,"cleanup 0x00000013"));
 int before=initialized;assert(settings_tile_create(message,sizeof(message))==-1&&initialized==before);
 assert(strstr(message,"restart payload")&&retained_module==&module_marker);
 /* Loader-close failure also latches and reports explicit cleanup failure. */
 cleanup_failed=0;retained_module=NULL;terminate_result=0;register_result=0;close_result=1;
 assert(settings_tile_create(message,sizeof(message))==-1&&strstr(message,"module cleanup failed"));
 assert(cleanup_failed&&retained_module==&module_marker);

 int app=openat(user,"app",O_RDONLY|O_DIRECTORY),title=openat(app,SETTINGS_TILE_ID,O_RDONLY|O_DIRECTORY);
 int sys=openat(title,"sce_sys",O_RDONLY|O_DIRECTORY);assert(sys>=0);
 assert(unlinkat(sys,"param.json",0)==0);assert(unlinkat(sys,"icon0.png",0)==0);assert(close(sys)==0);
 assert(unlinkat(title,"sce_sys",AT_REMOVEDIR)==0);assert(unlinkat(title,".install.lock",0)==0);
 assert(unlinkat(title,".steamcontroller-owner",0)==0);assert(close(title)==0);
 assert(unlinkat(app,SETTINGS_TILE_ID,AT_REMOVEDIR)==0);assert(close(app)==0);
 assert(unlinkat(user,"app",AT_REMOVEDIR)==0);assert(close(user)==0);assert(rmdir(root)==0);
 puts("settings tile native adapter: initialization, fixed title, failure retention, cleanup and retry latch passed (host stubs)");
 return 0;
}
