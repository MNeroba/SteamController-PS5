/* SPDX-License-Identifier: GPL-3.0-or-later
 * Independently authored, portable ownership/filesystem transaction. */
#if !defined(__PROSPERO__) && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif
#ifndef _DARWIN_C_SOURCE
#define _DARWIN_C_SOURCE
#endif
#include "settings_tile.h"
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

static const unsigned char owner[] = "SteamController-PS5 settings tile v1\n";
static int directory(int fd) {
    struct stat st;
    return fstat(fd,&st)==0 && S_ISDIR(st.st_mode) && st.st_uid==geteuid() && !(st.st_mode&0022);
}
static int owned_file(int fd) {
    struct stat st;
    return fstat(fd,&st)==0 && S_ISREG(st.st_mode) && st.st_uid==geteuid() && st.st_nlink==1 && !(st.st_mode&0022);
}
static int exact(int dir, const char *name, const unsigned char *data, size_t size) {
    int fd=openat(dir,name,O_RDONLY|O_NOFOLLOW|O_NONBLOCK|O_CLOEXEC);
    if(fd<0)return -1;
    struct stat st;int ok=owned_file(fd)&&fstat(fd,&st)==0&&st.st_size==(off_t)size;
    unsigned char buf[4096];size_t offset=0;
    while(ok&&offset<size){
        size_t amount=size-offset;if(amount>sizeof(buf))amount=sizeof(buf);
        ssize_t n=read(fd,buf,amount);
        if(n<0&&errno==EINTR)continue;
        if(n<=0||memcmp(buf,data+offset,(size_t)n)){ok=0;break;}
        offset+=(size_t)n;
    }
    if(close(fd))ok=0;
    return ok?0:-1;
}
/* O_EXCL + nofollow means cleanup can only unlink a file this call created.
 * On failure leave any existing file/directory untouched. */
static int asset(int dir,const char *name,const unsigned char *data,size_t size,mode_t mode) {
    int fd=openat(dir,name,O_WRONLY|O_CREAT|O_EXCL|O_NOFOLLOW|O_CLOEXEC,mode);
    if(fd<0)return errno==EEXIST?exact(dir,name,data,size):-1;
    int ok=owned_file(fd)&&fchmod(fd,mode)==0;size_t offset=0;
    while(ok&&offset<size){
        ssize_t n=write(fd,data+offset,size-offset);
        if(n<0&&errno==EINTR)continue;
        if(n<=0){ok=0;break;}offset+=(size_t)n;
    }
    if(ok&&fsync(fd))ok=0;
    if(close(fd))ok=0;
    if(!ok){(void)unlinkat(dir,name,0);return -1;}
    return fsync(dir)==0?0:-1;
}
int settings_tile_install(int app_fd,const unsigned char *icon,size_t icon_size,
                          const unsigned char *param,size_t param_size,
                          int (*register_title)(void *),void *context) {
    if(!icon||!param||!register_title||!icon_size||icon_size>1024*1024||
       !param_size||param_size>8192||!directory(app_fd))return SETTINGS_TILE_FILES_FAILED;
    int fresh=mkdirat(app_fd,SETTINGS_TILE_ID,0755)==0;
    if(!fresh&&errno!=EEXIST)return SETTINGS_TILE_FILES_FAILED;
    int title=openat(app_fd,SETTINGS_TILE_ID,O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC);
    int sys=-1,lock=-1,result=SETTINGS_TILE_FILES_FAILED;
    if(title<0)return result;
    if(!directory(title))goto done;
    if(fresh){
        if(fchmod(title,0755)||asset(title,".steamcontroller-owner",owner,sizeof(owner)-1,0600)||fsync(app_fd))goto done;
    }else if(exact(title,".steamcontroller-owner",owner,sizeof(owner)-1))goto done;
    lock=openat(title,".install.lock",O_RDWR|O_CREAT|O_NOFOLLOW|O_NONBLOCK|O_CLOEXEC,0600);
    if(lock<0||!owned_file(lock)||flock(lock,LOCK_EX|LOCK_NB))goto done;
    int fresh_sys=mkdirat(title,"sce_sys",0755)==0;
    if(!fresh_sys&&errno!=EEXIST)goto done;
    sys=openat(title,"sce_sys",O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC);
    if(sys<0||!directory(sys))goto done;
    if(fresh_sys&&fchmod(sys,0755))goto done;
    if(asset(sys,"param.json",param,param_size,0644)||asset(sys,"icon0.png",icon,icon_size,0644)||
       fsync(title)||fsync(app_fd))goto done;
    result=register_title(context)?SETTINGS_TILE_REGISTER_FAILED:0;
done:
    if(sys>=0&&close(sys))result=SETTINGS_TILE_FILES_FAILED;
    if(lock>=0&&close(lock))result=SETTINGS_TILE_FILES_FAILED;
    if(close(title))result=SETTINGS_TILE_FILES_FAILED;
    return result;
}
