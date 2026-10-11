/* Real production HTTP/auth/listener on loopback; hardware service stubs.
 * No PS5 device, profile filesystem or notification ABI is exercised. */
#define _POSIX_C_SOURCE 200809L
#define _DARWIN_C_SOURCE
#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
static int bind_failure;
static int injected_bind(int fd, const struct sockaddr *address, socklen_t length);
static char auth_dir[128];
#define WEBUI_AUTH_DIRECTORY auth_dir
#define WEBUI_PORT 0
#define bind injected_bind
#include "../payload/webui.c"
#undef bind
static int injected_bind(int fd, const struct sockaddr *address, socklen_t length) {
    if (bind_failure) { errno = EADDRINUSE; return -1; }
    return bind(fd, address, length);
}

static char notification[256];
static int notifications, mutations;
static void capture_notification(const char *message) {
    ++notifications; snprintf(notification, sizeof(notification), "%s", message);
}
int gc_game_name(const char *id, char *out, size_t n) { (void)id; if (n) *out=0; return -1; }
int gc_game_icon_path(const char *id, char *out, size_t n) { (void)id;(void)out;(void)n;return -1; }
int gc_game_list(gc_game_t *out, int n) { (void)out;(void)n;return 0; }
void sc2_set_paused(int on) { (void)on; }
int sc2_menu_is_open(void) { return 0; }
void sc2_gyro_status(sc2_gyro_status_t *s) { memset(s,0,sizeof(*s)); }
void sc2_haptic_pulse(int p,uint16_t a,uint16_t b,uint16_t c) { (void)p;(void)a;(void)b;(void)c; }
void sc2_haptic_tick(void) {}
void sc2_haptic_rumble_for(uint16_t l,uint16_t r,int ms) { (void)l;(void)r;(void)ms; }
void sc2_select_status(char *t,char *a,char *o) { *t=0;strcpy(a,"default");*o=0; }
void sc2_select_title_name(char *o,size_t n) { if(n)*o=0; }
void sc2_select_active_name(char *o,size_t n) { if(n)*o=0; }
int sc2_store_valid_id(const char *id) { return !strcmp(id,"default"); }
void sc2_store_list(void (*cb)(const char*,const char*,void*),void *u) { cb("default","Default",u); }
int sc2_store_load(const char *id,sc2_profile_t *p) { (void)id; memset(p,0,sizeof(*p));return 0; }
void sc2_profile_default(sc2_profile_t *p) { memset(p,0,sizeof(*p)); }
int sc2_profile_parse_checked(const char *t,sc2_profile_t *p,int c) { (void)t;(void)p;(void)c;return 0; }
int sc2_store_save(const char *id,const sc2_profile_t *p) { (void)id;(void)p;++mutations;return 0; }
int sc2_profile_format(const sc2_profile_t *p,char *o,size_t n) { (void)p;return snprintf(o,n,"name=Default\n"); }
int sc2_store_delete(const char *id) { (void)id;++mutations;return 0; }
void sc2_select_reload(void) {}
void sc2_select_override(const char *id) { (void)id; }
int game_hooks_set(int a,int b,int c) { (void)a;(void)b;(void)c; return 0; }
int game_hooks_set_audio(int a) { (void)a; return 0; }
int game_hooks_configure(int a,int b,int c,int d) { (void)a;(void)b;(void)c;(void)d; return 0; }
int game_hooks_json(char *o,size_t n) { return snprintf(o,n,"{}"); }
int ds_handoff_set_enabled(int on) { return on?-1:0; }
int ds_handoff_json(char *o,size_t n) { return snprintf(o,n,"{}"); }
void bridge_probe_request(void) {}
int bridge_probe_json(char *o,size_t n) { return snprintf(o,n,"{}"); }

static int connect_client(void) {
    struct sockaddr_in address; socklen_t n=sizeof(address);
    assert(getsockname(g_listener,(struct sockaddr*)&address,&n)==0);
    address.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
    int fd=socket(AF_INET,SOCK_STREAM,0);assert(fd>=0);
    assert(connect(fd,(struct sockaddr*)&address,n)==0);
    struct timeval timeout={3,0};
    assert(setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout))==0);
    return fd;
}
static int request(const char *wire,char *out,size_t cap) {
    int fd=connect_client();assert(send(fd,wire,strlen(wire),0)==(ssize_t)strlen(wire));
    size_t used=0;ssize_t n;
    while(used+1<cap&&(n=recv(fd,out+used,cap-1-used,0))>0)used+=(size_t)n;
    out[used]=0;close(fd);int code=0;assert(sscanf(out,"HTTP/1.1 %d",&code)==1);return code;
}
int main(void) {
    strcpy(auth_dir,"/tmp/sc-webui-service-XXXXXX");assert(mkdtemp(auth_dir));
    runtime_control_reset();
    bind_failure = 1;
    assert(webui_start(capture_notification)==-1);
    assert(notifications==1&&!strstr(notification,"ready")&&!g_webui_thread_started);
    bind_failure = 0; notifications = 0;
    assert(webui_start(capture_notification)==0);
    assert(notifications==1&&strstr(notification,"WebUI ready"));
    assert(!strstr(notification,g_webui_token));
    char code[9];const char *text=strstr(notification,"Pairing code: ");assert(text);
    memcpy(code,text+14,8);code[8]=0;
    char out[8192],wire[512];
    assert(request("GET /api/status HTTP/1.1\r\nHost: localhost\r\n\r\n",out,sizeof(out))==401);
    assert(!strstr(out,g_webui_token));
    /* Unauthenticated body is deliberately never sent: must reject early. */
    assert(request("POST /api/profile?id=default HTTP/1.1\r\nHost: localhost\r\nContent-Length: 30000\r\n\r\n",out,sizeof(out))==401);
    assert(mutations==0);
    snprintf(wire,sizeof(wire),"POST /pair HTTP/1.1\r\nHost: localhost\r\nOrigin: http://evil.example\r\nContent-Type: text/plain;charset=UTF-8\r\nContent-Length: 8\r\n\r\n%s",code);
    assert(request(wire,out,sizeof(out))==403);
    snprintf(wire,sizeof(wire),"POST /pair HTTP/1.1\r\nHost: localhost\r\nOrigin: http://localhost\r\nContent-Type: text/plain;charset=UTF-8\r\nContent-Length: 8\r\n\r\n%s",code);
    assert(request(wire,out,sizeof(out))==200&&strstr(out,g_webui_token));
    assert(request(wire,out,sizeof(out))==403&&!strstr(out,g_webui_token));
    snprintf(wire,sizeof(wire),"GET /api/status HTTP/1.1\r\nHost: localhost\r\nAuthorization: Bearer %s\r\n\r\n",g_webui_token);
    assert(request(wire,out,sizeof(out))==200&&strstr(out,"connected"));
    snprintf(wire,sizeof(wire),"POST /api/profile?id=default HTTP/1.1\r\nHost: localhost\r\nAuthorization: Bearer %s\r\nContent-Type: text/plain;charset=UTF-8\r\nContent-Length: 5\r\n\r\nA=PS\n",g_webui_token);
    assert(request(wire,out,sizeof(out))==200&&mutations==1);
    snprintf(wire,sizeof(wire),"GET /api/status HTTP/1.1\r\nHost: localhost\r\nAuthorization: Bearer %s\r\n\r\n",g_webui_token);
    /* A concurrent slow untrusted client does not block authorized APIs. */
    int slow=connect_client();assert(send(slow,"G",1,0)==1);
    assert(request(wire,out,sizeof(out))==200);
    assert(webui_stop()==0);close(slow);assert(webui_stop()==0);
    /* Retain credentials across payload relaunch, issue a fresh pairing code. */
    char token[65];int created;assert(webui_auth_load_or_create(auth_dir,token,&created)==0&&!created);
    assert(webui_start(capture_notification)==0&&strcmp(token,g_webui_token)==0);
    assert(webui_stop()==0);
    char path[160];snprintf(path,sizeof(path),"%s/webui.token",auth_dir);
    int f=open(path,O_WRONLY|O_TRUNC);assert(f>=0);assert(write(f,"bad",3)==3);close(f);
    int before=notifications;assert(webui_start(capture_notification)==-1);
    assert(notifications==before+1&&!strstr(notification,"ready"));
    assert(unlink(path)==0);assert(rmdir(auth_dir)==0);
    puts("production WebUI first-launch, authentication, concurrency and lifecycle checks passed");
    return 0;
}

int sc2_store_update(const char *id, sc2_profile_update_fn fn, void *context) {
 sc2_profile_t p; sc2_profile_default(&p); if(fn(&p,context))return -1; return sc2_store_save(id,&p);
}
int sc2_store_recover_default(void) { return -1; }
