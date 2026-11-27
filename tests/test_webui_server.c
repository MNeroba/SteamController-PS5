/* Real production HTTP/auth/listener on loopback; hardware service stubs.
 * No PS5 device, profile filesystem or notification ABI is exercised. */
#define _POSIX_C_SOURCE 200809L
#define _DARWIN_C_SOURCE
#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
static int storage_write_failure;
static int injected_sync(int fd) {
    if(storage_write_failure){errno=ENOSPC;return -1;}
    return fsync(fd);
}
#define fsync injected_sync
#include "../payload/webui_auth.c"
#undef fsync
static int tile_path_state;
static int bind_failure;
static int injected_bind(int fd, const struct sockaddr *address, socklen_t length);
static char auth_dir[128] __attribute__((unused));
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
static int notifications, mutations, tile_requests, tile_fail;
int settings_tile_create(char *message,size_t capacity) { tile_requests++;snprintf(message,capacity,"host registration fixture");return tile_fail ? -1 : 0; }
int settings_tile_is_registered(void) { return tile_path_state==1 ? 1 : tile_path_state==2 ? -1 : 0; }
static void capture_notification(const char *message) {
    ++notifications; snprintf(notification, sizeof(notification), "%s", message);
}
int gc_game_name(const char *id, char *out, size_t n) { (void)id; if (n) *out=0; return -1; }
static char icon_fixture_path[128];
int gc_game_icon_path(const char *id, char *out, size_t n) {
    if (strcmp(id,"default") || !icon_fixture_path[0]) return -1;
    snprintf(out,n,"%s",icon_fixture_path);return 0;
}
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
void sc2_active_get(sc2_profile_t *p) { memset(p,0,sizeof(*p)); }
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
static int hooks_toggle_calls, hooks_toggle_value, hooks_toggle_result;
int game_hooks_set_runtime_enabled(int enable, char *message, size_t capacity) {
    hooks_toggle_calls++; hooks_toggle_value = enable;
    snprintf(message, capacity, "toggle=%d", enable);
    return hooks_toggle_result;
}
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
    char out[8192];
    assert(loopback_host("localhost")); assert(loopback_host("127.0.0.1"));
    assert(!loopback_host("evil.example")); assert(!loopback_host("localhost.evil"));
    assert(!loopback_host("127.0.0.1.evil")); assert(!loopback_host("localhost:abc"));
    assert(!loopback_host("localhost:1")); assert(!loopback_host("127.0.0.1:1"));
    bind_failure=1; assert(webui_start(capture_notification)==-1);
    bind_failure=0; assert(webui_start(capture_notification)==0);
    struct sockaddr_in local; socklen_t len=sizeof(local);
    assert(getsockname(g_listener,(struct sockaddr*)&local,&len)==0);
    assert(ntohl(local.sin_addr.s_addr)==INADDR_LOOPBACK);
    {
        unsigned bound_port=(unsigned)ntohs(local.sin_port), other_port=bound_port==65535u?1u:bound_port+1u;
        char hostreq[192];
        snprintf(hostreq,sizeof(hostreq),"GET /api/status HTTP/1.1\r\nHost: localhost:%u\r\n\r\n",bound_port);
        assert(request(hostreq,out,sizeof(out))==200);
        snprintf(hostreq,sizeof(hostreq),"GET /api/status HTTP/1.1\r\nHost: localhost:%u\r\n\r\n",other_port);
        assert(request(hostreq,out,sizeof(out))==403);
    }
    assert(pthread_join(g_tile_thread,NULL)==0);g_tile_thread_started=0;
    assert(tile_requests==1);
    assert(request("POST /api/gamehooks?hooks=1 HTTP/1.1\r\nHost: localhost\r\nOrigin: http://localhost\r\nContent-Length: 0\r\n\r\n",out,sizeof(out))==200);
    assert(hooks_toggle_calls == 1 && hooks_toggle_value == 1);
    assert(strstr(out,"toggle=1"));
    hooks_toggle_result = 1;
    assert(request("POST /api/gamehooks?hooks=0 HTTP/1.1\r\nHost: localhost\r\nOrigin: http://localhost\r\nContent-Length: 0\r\n\r\n",out,sizeof(out))==500);
    assert(hooks_toggle_calls == 2 && hooks_toggle_value == 0);
    hooks_toggle_result = 0;
    assert(request("POST /api/gamehooks?hooks=2 HTTP/1.1\r\nHost: localhost\r\nOrigin: http://localhost\r\nContent-Length: 0\r\n\r\n",out,sizeof(out))==400);
    assert(request("GET /api/status HTTP/1.1\r\nHost: localhost\r\n\r\n",out,sizeof(out))==200);
    assert(strstr(out,"X-Content-Type-Options: nosniff"));
    assert(request("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n",out,sizeof(out))==200);
    assert(strstr(out,"X-Frame-Options: DENY"));
    strcpy(icon_fixture_path,"/tmp/sc-webui-icon-XXXXXX");
    int iconfd=mkstemp(icon_fixture_path);assert(iconfd>=0);assert(write(iconfd,"PNG",3)==3);assert(close(iconfd)==0);
    assert(request("GET /api/icon?id=default HTTP/1.1\r\nHost: localhost\r\n\r\n",out,sizeof(out))==200);
    assert(strstr(out,"Content-Type: image/png") && strstr(out,"X-Content-Type-Options: nosniff"));
    assert(unlink(icon_fixture_path)==0);
    strcpy(icon_fixture_path,"/tmp/sc-webui-icon-XXXXXX");
    iconfd=mkstemp(icon_fixture_path);assert(iconfd>=0);
    assert(ftruncate(iconfd,2*1024*1024+1)==0);assert(close(iconfd)==0);
    assert(request("GET /api/icon?id=default HTTP/1.1\r\nHost: localhost\r\n\r\n",out,sizeof(out))==500);
    assert(unlink(icon_fixture_path)==0);icon_fixture_path[0]=0;

    assert(request("GET /api/status HTTP/1.1\r\nHost: evil.example\r\n\r\n",out,sizeof(out))==403);
    assert(request("GET /api/status HTTP/1.1\r\nHost: evil.example\r\nOrigin: http://evil.example\r\n\r\n",out,sizeof(out))==403);
    assert(request("GET /api/status HTTP/1.1\r\nHost: 127.0.0.1.evil\r\n\r\n",out,sizeof(out))==403);
    assert(request("POST /api/settings-tile HTTP/1.1\r\nHost: localhost\r\nOrigin: null\r\nContent-Length: 0\r\n\r\n",out,sizeof(out))==403);

    assert(request("GET /api/status HTTP/1.1\r\nHost: localhost\r\nOrigin: http://evil.example\r\n\r\n",out,sizeof(out))==403);
    assert(request("POST /api/settings-tile HTTP/1.1\r\nHost: localhost\r\nContent-Length: 17\r\n\r\nINSTALL SCPS50001",out,sizeof(out))==403);
    assert(request("POST /api/settings-tile HTTP/1.1\r\nHost: localhost\r\nOrigin: http://evil.example\r\nContent-Type: text/plain;charset=UTF-8\r\nContent-Length: 17\r\n\r\nINSTALL SCPS50001",out,sizeof(out))==403);
    assert(request("GET /api/settings-tile HTTP/1.1\r\nHost: localhost\r\n\r\n",out,sizeof(out))==405);
    assert(request("POST /api/settings-tile HTTP/1.1\r\nHost: localhost\r\nOrigin: http://localhost\r\nContent-Type: text/plain;charset=UTF-8\r\nContent-Length: 3\r\n\r\nBAD",out,sizeof(out))==400);
    assert(request("POST /api/settings-tile HTTP/1.1\r\nHost: localhost\r\nOrigin: http://localhost\r\nContent-Type: text/plain;charset=UTF-8\r\nContent-Length: 17\r\n\r\nINSTALL SCPS50001",out,sizeof(out))==200);
    assert(tile_requests==2); /* explicit retry remains available */
    assert(request("POST /pair HTTP/1.1\r\nHost: localhost\r\nOrigin: http://localhost\r\nContent-Length: 0\r\n\r\n",out,sizeof(out))==404);
    assert(request("GET /does-not-exist HTTP/1.1\r\nHost: localhost\r\n\r\n",out,sizeof(out))==404);
    assert(request("GET /api/diagnostics?path=secrets HTTP/1.1\r\nHost: localhost\r\n\r\n",out,sizeof(out))==400);
    assert(request("GET /api/diagnostics HTTP/1.1\r\nHost: localhost\r\n\r\n",out,sizeof(out))==200);
    int slow=connect_client();assert(send(slow,"G",1,0)==1);
    assert(request("GET /api/status HTTP/1.1\r\nHost: localhost\r\n\r\n",out,sizeof(out))==200);
    assert(webui_stop()==0);close(slow);assert(webui_stop()==0);
    assert(tile_requests==2); /* one automatic attempt and one explicit retry */
    tile_path_state=1; assert(webui_start(capture_notification)==0); assert(pthread_join(g_tile_thread,NULL)==0); g_tile_thread_started=0; assert(webui_stop()==0);
    assert(tile_requests==2); /* existing title is never automatically replaced */
    tile_path_state=2; assert(webui_start(capture_notification)==0); assert(pthread_join(g_tile_thread,NULL)==0); g_tile_thread_started=0; assert(webui_stop()==0);
    assert(tile_requests==2); /* unknown access does not attempt installation */
    tile_path_state=0;tile_fail=1;
    assert(webui_start(capture_notification)==0);assert(pthread_join(g_tile_thread,NULL)==0);g_tile_thread_started=0;
    assert(tile_requests==3 && strstr(notification,"host registration fixture"));
    assert(request("GET /api/status HTTP/1.1\r\nHost: localhost\r\n\r\n",out,sizeof(out))==200);
    assert(webui_stop()==0);
    puts("loopback WebUI, host/origin policy, auto tile and shutdown checks passed");
    return 0;
}

int sc2_store_update(const char *id, sc2_profile_update_fn fn, void *context) {
 sc2_profile_t p; sc2_profile_default(&p); if(fn(&p,context))return -1; return sc2_store_save(id,&p);
}
int sc2_store_recover_default(void) { return -1; }
