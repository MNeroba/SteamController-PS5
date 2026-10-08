#define _POSIX_C_SOURCE 200809L

#include "webui_peer_policy.h"

#include <arpa/inet.h>
#include <stdio.h>

static int failures;
static int checks;

#define CHECK(condition) do { \
    checks++; \
    if (!(condition)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        failures++; \
    } \
} while (0)

static uint32_t ipv4(const char *text) {
    struct in_addr address;
    if (inet_pton(AF_INET, text, &address) != 1) {
        fprintf(stderr, "invalid IPv4 test fixture: %s\n", text);
        failures++;
        return 0;
    }
    return address.s_addr;
}

static void test_local_ipv4_ranges_are_allowed(void) {
    static const char *const addresses[] = {
        "127.0.0.1", "127.255.255.254", "10.0.0.1", "10.255.255.254",
        "172.16.0.1", "172.31.255.254", "192.168.1.20", "169.254.20.3"
    };
    size_t i;
    for (i = 0; i < sizeof(addresses) / sizeof(addresses[0]); i++)
        CHECK(webui_peer_ipv4_is_lan(ipv4(addresses[i])));
}

static void test_nonlocal_and_reserved_ranges_are_denied(void) {
    static const char *const addresses[] = {
        "0.0.0.0", "8.8.8.8", "100.64.0.1", "172.15.255.254",
        "172.32.0.1", "192.167.1.1", "192.0.2.1", "198.18.0.1",
        "224.0.0.1", "255.255.255.255"
    };
    size_t i;
    for (i = 0; i < sizeof(addresses) / sizeof(addresses[0]); i++)
        CHECK(!webui_peer_ipv4_is_lan(ipv4(addresses[i])));
}

int main(void) {
    test_local_ipv4_ranges_are_allowed();
    test_nonlocal_and_reserved_ranges_are_denied();
    if (failures) {
        fprintf(stderr, "%d/%d webui peer-policy checks failed\n", failures, checks);
        return 1;
    }
    printf("webui peer policy checks passed (%d checks)\n", checks);
    return 0;
}
