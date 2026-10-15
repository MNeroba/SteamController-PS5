/* A read-only observer must never produce an ownership receipt. */
#include "vda_pending.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
static unsigned checks;
#define CHECK(x)                                                                                   \
    do {                                                                                           \
        ++checks;                                                                                  \
        assert(x);                                                                                 \
    } while (0)
#define EVENT                                                                                      \
    "<118>#LOGIN MGR# Receive Event : SCE_MBUS_EVENT_DEVICE_ADDED "                                \
    "[DeviceId:0xd030d][type:1][subType:22][capabilityBattery:0]\n"
static void feed(vda_pending_t *s, const char *text) {
    vda_pending_feed(s, text, strlen(text));
}
static void begin(vda_pending_t *s) {
    vda_pending_begin(s, 100, 0);
}
static void readers(void) {
    int pair[2];
    vda_pending_reader_t r;
    CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, pair) == 0);
    CHECK(fcntl(pair[0], F_SETFL, O_NONBLOCK) == 0);
    CHECK(write(pair[1], EVENT, strlen(EVENT)) == (ssize_t)strlen(EVENT));
    CHECK(vda_pending_reader_begin(&r, pair[0], 100) == 0); /* backlog excluded */
    CHECK(vda_pending_reader_poll(&r, 101) == 0 && r.observation.summary.virtual_events == 0);
    CHECK(write(pair[1], EVENT, strlen(EVENT)) == (ssize_t)strlen(EVENT));
    CHECK(vda_pending_reader_poll(&r, 200) == 0);
    CHECK(vda_pending_reader_poll(&r, 3100) == 1 && r.fd == -1);
    CHECK(vda_pending_candidate(&r.observation) == 0xd030d);
    errno = 0;
    CHECK(fcntl(pair[0], F_GETFD) == -1 && errno == EBADF);
    CHECK(close(pair[1]) == 0);
    CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, pair) == 0);
    CHECK(vda_pending_reader_begin(&r, pair[0], 100) == -1 && r.fd == -1); /* blocking fd refused */
    CHECK(close(pair[1]) == 0);
    CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, pair) == 0);
    CHECK(fcntl(pair[0], F_SETFL, O_NONBLOCK) == 0);
    CHECK(vda_pending_reader_begin(&r, pair[0], 100) == 0);
    CHECK(close(pair[1]) == 0);
    CHECK(vda_pending_reader_poll(&r, 200) == 1 &&
          r.observation.summary.status == VDA_PENDING_IO_ERROR);
    CHECK(!vda_pending_candidate(&r.observation));
    CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, pair) == 0);
    CHECK(fcntl(pair[0], F_SETFL, O_NONBLOCK) == 0);
    CHECK(vda_pending_reader_begin(&r, pair[0], 100) == 0);
    char flood[5000];
    for (size_t i = 0; i < sizeof(flood); i++)
        flood[i] = (i & 1) ? '\n' : '.';
    memcpy(flood, EVENT, strlen(EVENT));
    CHECK(write(pair[1], flood, sizeof(flood)) == (ssize_t)sizeof(flood));
    CHECK(vda_pending_reader_poll(&r, 3100) == 1);
    CHECK(r.observation.summary.status == VDA_PENDING_INCOMPLETE &&
          !vda_pending_candidate(&r.observation));
    CHECK(close(pair[1]) == 0);
    errno = EACCES;
    CHECK(vda_pending_reader_begin(&r, -1, 100) == -1);
    CHECK(r.observation.summary.error == EACCES);
    vda_pending_reader_close(&r);
}
int main(void) {
    readers();
    vda_pending_t s;
    begin(&s);
    for (size_t i = 0; i < strlen(EVENT); i++)
        vda_pending_feed(&s, &EVENT[i], 1);
    CHECK(s.summary.virtual_events == 1 && s.summary.distinct_virtuals == 1);
    feed(&s, EVENT);
    CHECK(s.summary.virtual_events == 2 && s.summary.distinct_virtuals == 1);
    CHECK(vda_pending_finish(&s, 3099, 0) == 0);
    CHECK(vda_pending_finish(&s, 3100, 0) == 1);
    CHECK(s.summary.status == VDA_PENDING_UNPROVEN);
    CHECK(vda_pending_candidate(&s) == 0xd030d); /* correlation only, not an ABI proof */
    feed(&s, EVENT);
    CHECK(s.summary.virtual_events == 2);
    begin(&s);
    feed(&s, EVENT);
    feed(&s, "#LOGIN MGR# Receive Event : SCE_MBUS_EVENT_DEVICE_ADDED "
             "[DeviceId:0xd030e][type:1][subType:22][capabilityBattery:0]\n");
    CHECK(vda_pending_finish(&s, 3100, 0) == 1);
    CHECK(s.summary.status == VDA_PENDING_AMBIGUOUS);
    const char *ignored[] = {("[GC] " EVENT),
                             "#LOGIN MGR# Receive Event : SCE_MBUS_EVENT_DEVICE_ADDED "
                             "[DeviceId:0x123][type:1][subType:22][capabilityBattery:1]\n",
                             "#LOGIN MGR# Receive Event : SCE_MBUS_EVENT_DEVICE_ADDED "
                             "[DeviceId:0x123][type:2][subType:22][capabilityBattery:0]\n",
                             "DEVICE_ADDED deviceId=0x123 deviceType=22\n"};
    for (size_t i = 0; i < sizeof(ignored) / sizeof(ignored[0]); i++) {
        begin(&s);
        feed(&s, ignored[i]);
        CHECK(s.summary.virtual_events == 0);
    }
    const char *bad[] = {
        "#LOGIN MGR# Receive Event : SCE_MBUS_EVENT_DEVICE_ADDED "
        "[DeviceId:0x0][type:1][subType:22][capabilityBattery:0]\n",
        "#LOGIN MGR# Receive Event : SCE_MBUS_EVENT_DEVICE_ADDED "
        "[DeviceId:0x10000000000000000][type:1][subType:22][capabilityBattery:0]\n",
        "#LOGIN MGR# Receive Event : SCE_MBUS_EVENT_DEVICE_ADDED "
        "[DeviceId:0x123z][type:1][subType:22][capabilityBattery:0]\n",
        "#LOGIN MGR# Receive Event : SCE_MBUS_EVENT_DEVICE_ADDED "
        "[DeviceId:0x123][type:1][subType:22][subType:22][capabilityBattery:0]\n"};
    for (size_t i = 0; i < sizeof(bad) / sizeof(bad[0]); i++) {
        begin(&s);
        feed(&s, bad[i]);
        CHECK(s.summary.virtual_events == 0);
        CHECK(s.summary.malformed == 1);
    }
    begin(&s);
    char oversized[2048];
    memset(oversized, 'x', sizeof(oversized));
    vda_pending_feed(&s, oversized, sizeof(oversized));
    feed(&s, EVENT);
    CHECK(s.summary.truncated == 1 && s.summary.virtual_events == 0);
    feed(&s, EVENT);
    CHECK(s.summary.virtual_events == 1);
    CHECK(vda_pending_finish(&s, 3100, 0) == 1);
    CHECK(s.summary.status == VDA_PENDING_INCOMPLETE);
    begin(&s);
    feed(&s, EVENT);
    CHECK(vda_pending_finish(&s, 99, 0) == 1);
    CHECK(s.summary.status == VDA_PENDING_CLOCK_ERROR);
    begin(&s);
    CHECK(vda_pending_finish(&s, 200, 1) == 1);
    CHECK(s.summary.status == VDA_PENDING_CANCELLED);
    begin(&s);
    CHECK(vda_pending_finish(&s, 3100, 0) == 1);
    CHECK(s.summary.status == VDA_PENDING_NO_EVENT);
    vda_pending_begin(&s, 100, 1);
    feed(&s, EVENT);
    CHECK(s.summary.virtual_events == 0);
    feed(&s, EVENT);
    CHECK(s.summary.virtual_events == 1); /* stale partial line discarded */
    begin(&s);
    const char nul[] = "bad\0" EVENT;
    vda_pending_feed(&s, nul, sizeof(nul) - 1);
    CHECK(s.summary.virtual_events == 0 && s.summary.malformed == 1);
    CHECK(vda_pending_finish(&s, 3100, 0) == 1);
    CHECK(s.summary.status == VDA_PENDING_INCOMPLETE);
    begin(&s);
    feed(&s, "#LOGIN MGR# Receive Event : SCE_MBUS_EVENT_DEVICE_ADDED "
             "[DeviceId:0xd030d][type:1][subType:22][capabilityBattery:1]\n");
    feed(&s, EVENT);
    CHECK(vda_pending_finish(&s, 3100, 0) == 1);
    CHECK(!vda_pending_candidate(&s));
    begin(&s);
    feed(&s, EVENT);
    feed(&s, "#LOGIN MGR# Receive Event : SCE_MBUS_EVENT_DEVICE_ADDED "
             "[DeviceId:0xd030d][type:1][subType:22][capabilityBattery:1]\n");
    CHECK(vda_pending_finish(&s, 3100, 0) == 1);
    CHECK(!vda_pending_candidate(&s));
    CHECK(strstr(vda_pending_name(VDA_PENDING_UNPROVEN), "ownership unproven"));
    CHECK(!strcmp(vda_pending_name((vda_pending_status_t)-1), "unknown"));
    printf("read-only pending VDA observation: %u checks passed\n", checks);
    return 0;
}
