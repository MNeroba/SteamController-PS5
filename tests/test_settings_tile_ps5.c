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
static int missing_module,missing_symbol,module_marker;
static int nid_available;                    /* 1: all three NID lookups succeed */
int settings_tile_test_nid_lookup(const char *nid,void *function,size_t size);
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
#define dlopen fixture_dlopen
#define dlsym fixture_dlsym
#define open fixture_open
#define SETTINGS_TILE_TEST
#include "../payload/settings_tile_ps5.c"
#undef open
#undef dlopen
#undef dlsym
int settings_tile_test_nid_lookup(const char *nid,void *function,size_t size) {
 if(!nid_available)return -1;
 void *address=NULL;
 if(!strcmp(nid,TILE_NID_INIT)){int(*fn)(void)=sceAppInstUtilInitialize;memcpy(&address,&fn,sizeof(address));}
 else if(!strcmp(nid,TILE_NID_TERM)){int(*fn)(void)=sceAppInstUtilTerminate;memcpy(&address,&fn,sizeof(address));}
 else if(!strcmp(nid,TILE_NID_TITLE)){int(*fn)(const char*,const char*,void*)=sceAppInstUtilAppInstallTitleDir;memcpy(&address,&fn,sizeof(address));}
 else return -1;
 if(size!=sizeof(address))return -1;
 memcpy(function,&address,size);return 0;
}
int sceAppInstUtilInitialize(void) { initialized++;return init_result; }
int sceAppInstUtilTerminate(void) { terminated++;return terminate_result; }
int sceAppInstUtilAppInstallTitleDir(const char *id,const char *dir,void *unused) {
 assert(!strcmp(id,"SCPS50001")&&!strcmp(dir,"/user/app/")&&!unused);registered++;return register_result;
}
int main(void) {
 strcpy(root,"/tmp/sc-tile-adapter-XXXXXX");assert(mkdtemp(root));char message[256];

 /* Missing module and missing API must fail without latching a retry out. */
 missing_module=1;
 assert(settings_tile_create(message,sizeof(message))==-1&&initialized==0);
 assert(strstr(message,"unavailable"));
 missing_module=0;missing_symbol=1;nid_available=0;
 assert(settings_tile_create(message,sizeof(message))==-1&&initialized==0);
 assert(strstr(message,"Required tile installer API unavailable"));
 assert(strstr(message,"initialize")&&strstr(message,"retry is safe"));

 /* Prepare the owned /user/app skeleton. */
 int user=open(root,O_RDONLY|O_DIRECTORY);assert(user>=0);assert(mkdirat(user,"app",0755)==0);

 /* NID-only SPRX: public names are absent but registration still succeeds. */
 nid_available=1;
 assert(settings_tile_create(message,sizeof(message))==0);
 assert(initialized==1&&registered==1&&terminated==1&&strstr(message,"Registration accepted"));

 /* Name fallback when NID resolution is unavailable. */
 nid_available=0;missing_symbol=0;
 assert(settings_tile_create(message,sizeof(message))==0&&registered==2);

 /* Initialization failure is retryable, not latched. */
 init_result=13;
 assert(settings_tile_create(message,sizeof(message))==-1);
 assert(strstr(message,"initialization failed")&&initialized==3&&terminated==2);
 init_result=0;

 /* Registration failure retains the error code. */
 register_result=17;
 assert(settings_tile_create(message,sizeof(message))==-1);
 assert(strstr(message,"0x00000011"));

 /* Accepted registration with failed installer cleanup stays a success and
  * never blocks the next attempt. */
 register_result=0;terminate_result=19;
 assert(settings_tile_create(message,sizeof(message))==0);
 assert(strstr(message,"Registration accepted")&&strstr(message,"cleanup failed"));
 terminate_result=0;
 assert(settings_tile_create(message,sizeof(message))==0&&registered==5);
 assert(strstr(message,"Registration accepted"));

 int app=openat(user,"app",O_RDONLY|O_DIRECTORY),title=openat(app,SETTINGS_TILE_ID,O_RDONLY|O_DIRECTORY);
 int sys=openat(title,"sce_sys",O_RDONLY|O_DIRECTORY);assert(sys>=0);
 assert(unlinkat(sys,"param.json",0)==0);assert(unlinkat(sys,"icon0.png",0)==0);assert(close(sys)==0);
 assert(unlinkat(title,"sce_sys",AT_REMOVEDIR)==0);assert(unlinkat(title,".install.lock",0)==0);
 assert(unlinkat(title,".steamcontroller-registered",0)==0);
 assert(unlinkat(title,".steamcontroller-owner",0)==0);assert(close(title)==0);
 assert(unlinkat(app,SETTINGS_TILE_ID,AT_REMOVEDIR)==0);assert(close(app)==0);
 assert(unlinkat(user,"app",AT_REMOVEDIR)==0);assert(close(user)==0);assert(rmdir(root)==0);
 puts("settings tile native adapter: NID/name resolution, retry-safe failures and cleanup warning passed (host stubs)");
 return 0;
}
