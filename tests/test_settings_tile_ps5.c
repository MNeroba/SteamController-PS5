/* Host stubs exercise native adapter control flow, never the console ABI. */
#define _POSIX_C_SOURCE 200809L
#define _DARWIN_C_SOURCE
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
static char root[128];static int init_result,register_result,terminate_result;
static int initialized,registered,terminated;
static int fixture_open(const char *path,int flags) { assert(!strcmp(path,"/user"));return open(root,flags); }
#define open fixture_open
#include "../payload/settings_tile_ps5.c"
#undef open
int sceAppInstUtilInitialize(void) { initialized++;return init_result; }
int sceAppInstUtilTerminate(void) { terminated++;return terminate_result; }
int sceAppInstUtilAppInstallTitleDir(const char *id,const char *dir,void *unused) {
 assert(!strcmp(id,"SCPS50001")&&!strcmp(dir,"/user/app/")&&!unused);registered++;return register_result;
}
int main(void) {
 strcpy(root,"/tmp/sc-tile-adapter-XXXXXX");assert(mkdtemp(root));char message[256];
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
 assert(strstr(message,"restart payload"));
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
