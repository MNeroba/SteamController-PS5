#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif
#include "webui_phone.h"
#include "webui_peer_policy.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#define PHONE_SESSION_MS (60u * 60u * 1000u)

static pthread_mutex_t phone_lock = PTHREAD_MUTEX_INITIALIZER;
static struct {
    int listener;
    uint64_t generation;
    uint64_t started;
    uint32_t address, mask, paired_peer;
    char host[22];
    char token[WEBUI_TOKEN_HEX_LENGTH + 1];
    webui_pairing_t pairing;
} phone = {.listener = -1};

static int private_ipv4(uint32_t address) {
    unsigned first = ntohl(address) >> 24;
    return webui_peer_ipv4_is_lan(address) && first != 127;
}

static int usable_subnet(uint32_t address, uint32_t mask) {
    uint32_t inverse = ~ntohl(mask);
    uint32_t host = ntohl(address) & inverse;
    return private_ipv4(address) && mask && inverse > 1 &&
           (inverse & (inverse + 1)) == 0 && host && host != inverse;
}

static void stop_locked(void) {
    if (phone.listener < 0) return;
    shutdown(phone.listener, SHUT_RDWR);
    close(phone.listener);
    phone.listener = -1;
    ++phone.generation;
    webui_pairing_destroy(&phone.pairing);
    memset(phone.token, 0, sizeof(phone.token));
    phone.paired_peer = 0;
}

static int active_locked(uint64_t now) {
    /* Another thread can open a session after the caller sampled the clock.
     * An older observation must never close that newer session. */
    if (phone.listener >= 0 && now < phone.started) return 0;
    if (phone.listener >= 0 && now - phone.started >= PHONE_SESSION_MS) stop_locked();
    return phone.listener >= 0;
}

static int bind_private(unsigned port, uint32_t *address, uint32_t *mask, char host[22]) {
    struct ifaddrs *interfaces = NULL;
    if (!port || port > 65535 || getifaddrs(&interfaces) != 0) return -1;
    int listener = -1;
    for (struct ifaddrs *it = interfaces; it; it = it->ifa_next) {
        if (!(it->ifa_flags & IFF_UP) || (it->ifa_flags & (IFF_LOOPBACK | IFF_POINTOPOINT)) ||
            !it->ifa_addr || !it->ifa_netmask || it->ifa_addr->sa_family != AF_INET ||
            it->ifa_netmask->sa_family != AF_INET) continue;
        const struct sockaddr_in *ip = (const struct sockaddr_in *)it->ifa_addr;
        const struct sockaddr_in *net = (const struct sockaddr_in *)it->ifa_netmask;
        if (!usable_subnet(ip->sin_addr.s_addr, net->sin_addr.s_addr)) continue;
        char literal[INET_ADDRSTRLEN];
        if (!inet_ntop(AF_INET, &ip->sin_addr, literal, sizeof(literal))) continue;
        struct sockaddr_in bind_to = *ip;
        bind_to.sin_port = htons((uint16_t)port);
        int fd = socket(AF_INET, SOCK_STREAM, 0), one = 1;
        if (fd < 0) continue;
        if (fd < FD_SETSIZE && fcntl(fd, F_SETFL, O_NONBLOCK) == 0 &&
            setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) == 0 &&
            bind(fd, (struct sockaddr *)&bind_to, sizeof(bind_to)) == 0 && listen(fd, 4) == 0) {
            listener = fd;
            *address = ip->sin_addr.s_addr;
            *mask = net->sin_addr.s_addr;
            snprintf(host, 22, "%s:%u", literal, port);
            break;
        }
        close(fd);
    }
    freeifaddrs(interfaces);
    return listener;
}

int webui_phone_start(unsigned port, uint64_t now, char code[WEBUI_PAIR_CODE_LENGTH + 1]) {
    pthread_mutex_lock(&phone_lock);
    int result = -1;
    if (active_locked(now)) {
        result = webui_pairing_refresh(&phone.pairing, now, code);
    } else if (phone.listener < 0) {
        int fd = bind_private(port, &phone.address, &phone.mask, phone.host);
        if (fd >= 0) {
            result = webui_pairing_init(&phone.pairing, now, code);
            if (result == 0) {
                phone.listener = fd;
                ++phone.generation;
            } else close(fd);
        }
    }
    if (result == 0) {
        phone.started = now;
        phone.paired_peer = 0;
        memset(phone.token, 0, sizeof(phone.token));
    }
    pthread_mutex_unlock(&phone_lock);
    return result;
}

void webui_phone_stop(void) {
    pthread_mutex_lock(&phone_lock);
    stop_locked();
    pthread_mutex_unlock(&phone_lock);
}

void webui_phone_info(uint64_t now, webui_phone_info_t *info) {
    memset(info, 0, sizeof(*info));
    pthread_mutex_lock(&phone_lock);
    if (active_locked(now)) {
        info->enabled = 1;
        info->paired = phone.paired_peer != 0;
        memcpy(info->host, phone.host, sizeof(info->host));
    }
    pthread_mutex_unlock(&phone_lock);
}

int webui_phone_listener(uint64_t now, uint64_t *generation) {
    pthread_mutex_lock(&phone_lock);
    int fd = active_locked(now) ? dup(phone.listener) : -1;
    if (fd >= FD_SETSIZE) { close(fd); fd = -1; }
    *generation = phone.generation;
    pthread_mutex_unlock(&phone_lock);
    return fd;
}

static int peer_locked(webui_phone_peer_t peer, uint64_t now) {
    return active_locked(now) && peer.generation == phone.generation &&
           usable_subnet(peer.peer, phone.mask) &&
           (peer.peer & phone.mask) == (phone.address & phone.mask);
}

int webui_phone_peer_allowed(webui_phone_peer_t peer, uint64_t now) {
    pthread_mutex_lock(&phone_lock);
    int ok = peer_locked(peer, now);
    pthread_mutex_unlock(&phone_lock);
    return ok;
}

int webui_phone_host_matches(webui_phone_peer_t peer, const char *host, uint64_t now) {
    pthread_mutex_lock(&phone_lock);
    int ok = peer_locked(peer, now) && !strcmp(host, phone.host);
    pthread_mutex_unlock(&phone_lock);
    return ok;
}

int webui_phone_authorized(webui_phone_peer_t peer, const char *authorization, uint64_t now) {
    pthread_mutex_lock(&phone_lock);
    int ok = peer_locked(peer, now) && phone.paired_peer == peer.peer &&
             webui_auth_matches(authorization, phone.token);
    pthread_mutex_unlock(&phone_lock);
    return ok;
}

int webui_phone_pair(webui_phone_peer_t peer, const char *code, uint64_t now,
                      char token[WEBUI_TOKEN_HEX_LENGTH + 1]) {
    pthread_mutex_lock(&phone_lock);
    int ok = peer_locked(peer, now) && !phone.paired_peer &&
             webui_pairing_redeem(&phone.pairing, code, now) &&
             webui_auth_session_token(phone.token, NULL) == 0;
    if (ok) {
        phone.paired_peer = peer.peer;
        memcpy(token, phone.token, sizeof(phone.token));
    }
    pthread_mutex_unlock(&phone_lock);
    return ok;
}
