#define _POSIX_C_SOURCE 200809L
#ifndef _DARWIN_C_SOURCE
#define _DARWIN_C_SOURCE
#endif
#include "settings_tile.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>
static int calls, fail_register;
static int register_tile(void *ctx) { assert(ctx == &calls); calls++; return fail_register; }
static const unsigned char icon[] = "image-fixture";
static const unsigned char param[] = "metadata-fixture";
static int install(int fd) { return settings_tile_install(fd, icon, sizeof(icon), param, sizeof(param), register_tile, &calls); }
int main(void) {
 char root[]="/tmp/sc-tile-XXXXXX"; assert(mkdtemp(root));
 int fd=open(root,O_RDONLY|O_DIRECTORY);assert(fd>=0);
 mode_t oldmask=umask(0077);
 assert(install(fd)==0&&calls==1);
 struct stat readable;assert(fstatat(fd,SETTINGS_TILE_ID "/sce_sys",&readable,0)==0);
 assert((readable.st_mode&0777)==0755);
 assert(fstatat(fd,SETTINGS_TILE_ID "/sce_sys/icon0.png",&readable,0)==0);
 assert((readable.st_mode&0777)==0644);umask(oldmask);assert(install(fd)==0&&calls==2);
 fail_register=123;assert(install(fd)==SETTINGS_TILE_REGISTER_FAILED&&calls==3);
 fail_register=0;assert(install(fd)==0&&calls==4);
 int title=openat(fd,SETTINGS_TILE_ID,O_RDONLY|O_DIRECTORY);assert(title>=0);
 int lock=openat(title,".install.lock",O_RDWR);assert(lock>=0);assert(flock(lock,LOCK_EX|LOCK_NB)==0);
 assert(install(fd)==SETTINGS_TILE_FILES_FAILED&&calls==4);assert(close(lock)==0);
 int sys=openat(title,"sce_sys",O_RDONLY|O_DIRECTORY);assert(sys>=0);
 assert(unlinkat(sys,"icon0.png",0)==0);assert(symlinkat("/etc/passwd",sys,"icon0.png")==0);
 assert(install(fd)==SETTINGS_TILE_FILES_FAILED&&calls==4);assert(unlinkat(sys,"icon0.png",0)==0);
 assert(mkfifoat(sys,"icon0.png",0600)==0);assert(install(fd)==SETTINGS_TILE_FILES_FAILED&&calls==4);
 assert(unlinkat(sys,"icon0.png",0)==0);assert(install(fd)==0&&calls==5);
 int image=openat(sys,"icon0.png",O_WRONLY);assert(image>=0);assert(write(image,"x",1)==1);assert(close(image)==0);
 assert(install(fd)==SETTINGS_TILE_FILES_FAILED&&calls==5);
 assert(unlinkat(sys,"icon0.png",0)==0);assert(unlinkat(sys,"param.json",0)==0);assert(close(sys)==0);
 assert(unlinkat(title,"sce_sys",AT_REMOVEDIR)==0);assert(unlinkat(title,".install.lock",0)==0);
 assert(unlinkat(title,".steamcontroller-owner",0)==0);assert(close(title)==0);
 /* Existing unowned directory and symlink must never be claimed or removed. */
 assert(install(fd)==SETTINGS_TILE_FILES_FAILED&&calls==5);
 assert(unlinkat(fd,SETTINGS_TILE_ID,AT_REMOVEDIR)==0);assert(symlinkat("/tmp",fd,SETTINGS_TILE_ID)==0);
 assert(install(fd)==SETTINGS_TILE_FILES_FAILED&&calls==5);assert(unlinkat(fd,SETTINGS_TILE_ID,0)==0);
 assert(settings_tile_install(fd,icon,0,param,sizeof(param),register_tile,&calls)==SETTINGS_TILE_FILES_FAILED);
 assert(install(-1)==SETTINGS_TILE_FILES_FAILED);assert(close(fd)==0);assert(rmdir(root)==0);
 puts("settings tile: ownership, exact assets, lock, retry, registration failure and hostile files passed");
 return 0;
}
