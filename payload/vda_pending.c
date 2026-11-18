/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "vda_pending.h"
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef VDA_PENDING_TEST
extern ssize_t vda_pending_test_read(int fd, void *buffer, size_t size);
#define vda_reader_read vda_pending_test_read
#else
#define vda_reader_read read
#endif

static void increment(uint32_t *n) {
    if (*n < UINT32_MAX)
        ++*n;
}
static void terminal(vda_pending_t *s, vda_pending_status_t status) {
    s->summary.status = status;
    memset(s->line, 0, sizeof(s->line));
    if (status != VDA_PENDING_UNPROVEN)
        s->first_id = 0;
    s->used = 0;
    s->dropping = 0;
    memset(s->native_ids, 0, sizeof(s->native_ids));
    s->native_count = 0;
}
void vda_pending_begin(vda_pending_t *s, uint64_t now_ms, int discard_partial) {
    if (!s)
        return;
    memset(s, 0, sizeof(*s));
    s->started_ms = now_ms;
    s->summary.status = VDA_PENDING_OBSERVING;
    s->dropping = !!discard_partial;
}
static int hex_digit(char c) {
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}
/* Exact bracket fields, exactly once. Reject suffixes and overflow. */
static int field(const char *line, const char *key, unsigned base, uint64_t *value) {
    const char *p = strstr(line, key);
    if (!p || strstr(p + strlen(key), key))
        return 0;
    p += strlen(key);
    if (base == 16) {
        if (p[0] != '0' || p[1] != 'x')
            return 0;
        p += 2;
    }
    uint64_t n = 0;
    unsigned digits = 0;
    int d;
    while ((d = hex_digit(*p)) >= 0 && (unsigned)d < base) {
        if (n > (UINT64_MAX - (unsigned)d) / base)
            return 0;
        n = n * base + (unsigned)d;
        ++p;
        ++digits;
    }
    if (!digits || *p != ']')
        return 0;
    *value = n;
    return 1;
}
static void observe(vda_pending_t *s) {
    const char *line = s->line;
    if (strstr(line, "[GC]") || strstr(line, "[Ghostpad]") ||
        !strstr(line, "#LOGIN MGR# Receive Event : SCE_MBUS_EVENT_DEVICE_ADDED"))
        return;
    uint64_t id, type, subtype, battery;
    if (!field(line, "[DeviceId:", 16, &id) || !id || !field(line, "[type:", 10, &type) ||
        !field(line, "[subType:", 10, &subtype) ||
        !field(line, "[capabilityBattery:", 10, &battery)) {
        increment(&s->summary.malformed);
        return;
    }
    /* Physical DualSense event in the supplied trace has battery capability1.
     * This filter alone is NOT an ownership receipt, even for battery0. */
    if (type != 1 || subtype != 22)
        return;
    if (battery == 1) {
        if (s->first_id == id)
            increment(&s->summary.malformed);
        for (unsigned i = 0; i < s->native_count; i++)
            if (s->native_ids[i] == id)
                return;
        if (s->native_count == 4) {
            increment(&s->summary.malformed);
            return;
        }
        s->native_ids[s->native_count++] = id;
        return;
    }
    if (battery != 0)
        return;
    for (unsigned i = 0; i < s->native_count; i++)
        if (s->native_ids[i] == id) {
            increment(&s->summary.malformed);
            return;
        }
    increment(&s->summary.virtual_events);
    if (!s->summary.distinct_virtuals) {
        s->first_id = id;
        s->summary.distinct_virtuals = 1;
    } else if (s->first_id != id)
        s->summary.distinct_virtuals = 2; /* 2 means two or more */
}
void vda_pending_feed(vda_pending_t *s, const char *data, size_t size) {
    if (!s || !data || s->summary.status != VDA_PENDING_OBSERVING)
        return;
    for (size_t i = 0; i < size; i++) {
        char c = data[i];
        if (c == '\n') {
            if (!s->dropping) {
                s->line[s->used] = 0;
                observe(s);
            }
            memset(s->line, 0, s->used);
            s->used = 0;
            s->dropping = 0;
        } else if (!s->dropping && c != '\r') {
            if (c == 0) {
                increment(&s->summary.malformed);
                s->dropping = 1;
            } else if (s->used + 1 >= sizeof(s->line)) {
                increment(&s->summary.truncated);
                s->dropping = 1;
            } else
                s->line[s->used++] = c;
        }
    }
}
int vda_pending_finish(vda_pending_t *s, uint64_t now_ms, int cancelled) {
    if (!s)
        return 1;
    if (s->summary.status != VDA_PENDING_OBSERVING)
        return 1;
    if (cancelled)
        terminal(s, VDA_PENDING_CANCELLED);
    else if (now_ms < s->started_ms)
        terminal(s, VDA_PENDING_CLOCK_ERROR);
    else {
        s->summary.elapsed_ms = now_ms - s->started_ms;
        if (s->summary.elapsed_ms < VDA_PENDING_WINDOW_MS)
            return 0;
        if (s->used || s->dropping)
            increment(&s->summary.truncated);
        terminal(s, s->summary.malformed || s->summary.truncated ? VDA_PENDING_INCOMPLETE
                    : s->summary.distinct_virtuals > 1           ? VDA_PENDING_AMBIGUOUS
                    : s->summary.distinct_virtuals               ? VDA_PENDING_UNPROVEN
                                                                 : VDA_PENDING_NO_EVENT);
    }
    return 1;
}
void vda_pending_fail(vda_pending_t *s, vda_pending_status_t status, int error) {
    if (!s)
        return;
    s->summary.error = error;
    terminal(s, status);
}
const char *vda_pending_name(vda_pending_status_t status) {
    static const char *const names[] = {"idle",
                                        "observing (diagnostic only)",
                                        "log unavailable",
                                        "no matching event",
                                        "one virtual event; ownership unproven",
                                        "ambiguous virtual events",
                                        "incomplete observation",
                                        "log I/O error",
                                        "clock error",
                                        "cancelled",
                                        "candidate rejected by native/session guard"};
    return (unsigned)status < sizeof(names) / sizeof(names[0]) ? names[status] : "unknown";
}

uint64_t vda_pending_candidate(const vda_pending_t *s) {
    if (!s || s->summary.status != VDA_PENDING_UNPROVEN)
        return 0;
    uint32_t h = (uint32_t)s->first_id;
    return h && h <= INT32_MAX ? s->first_id : 0;
}
void vda_pending_reader_close(vda_pending_reader_t *r) {
    if (!r)
        return;
    int fd = r->fd;
    r->fd = -1;
    if (fd >= 0 && close(fd) != 0)
        vda_pending_fail(&r->observation, VDA_PENDING_IO_ERROR, errno);
}
int vda_pending_reader_begin(vda_pending_reader_t *r, int fd, uint64_t now_ms) {
    if (!r) {
        if (fd >= 0)
            close(fd);
        return -1;
    }
    memset(r, 0, sizeof(*r));
    r->fd = fd;
    vda_pending_begin(&r->observation, now_ms, 0);
    if (fd < 0) {
        vda_pending_fail(&r->observation, VDA_PENDING_UNAVAILABLE, errno);
        return -1;
    }
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0 || !(flags & O_NONBLOCK)) {
        int error = flags < 0 ? errno : EINVAL;
        vda_pending_reader_close(r);
        vda_pending_fail(&r->observation, VDA_PENDING_UNAVAILABLE, error);
        return -1;
    }
    char buffer[1024];
    int partial = 0;
    /* Local patch: 64 x 1 KiB was too small for the existing /dev/klog
     * backlog on real consoles and failed with EOVERFLOW (errno 84 on
     * FreeBSD/PS5, misread as Linux EILSEQ). Drain up to 4 MiB. */
    for (unsigned i = 0; i < 4096; i++) {
        ssize_t n = vda_reader_read(fd, buffer, sizeof(buffer));
        if (n > 0) {
            partial = buffer[n - 1] != '\n';
            continue;
        }
        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            vda_pending_begin(&r->observation, now_ms, partial);
            return 0;
        }
        if (n < 0 && errno == EINTR)
            continue;
        int error = n == 0 ? EPIPE : errno;
        vda_pending_reader_close(r);
        vda_pending_fail(&r->observation, VDA_PENDING_UNAVAILABLE, error);
        return -1;
    }
    vda_pending_reader_close(r);
    vda_pending_fail(&r->observation, VDA_PENDING_UNAVAILABLE, EOVERFLOW);
    return -1;
}
int vda_pending_reader_fallback_eligible(const vda_pending_reader_t *r) {
    return r && r->observation.summary.status == VDA_PENDING_UNAVAILABLE &&
           r->observation.summary.error == EILSEQ;
}
int vda_pending_reader_start_window(vda_pending_reader_t *r, uint64_t now_ms) {
    if (!r) return -1;
    if (r->fd < 0 || r->window_armed ||
        r->observation.summary.status != VDA_PENDING_OBSERVING ||
        r->observation.summary.virtual_events || r->observation.used) {
        vda_pending_reader_close(r);
        vda_pending_fail(&r->observation, VDA_PENDING_REJECTED, EINVAL);
        return -1;
    }
    if (now_ms < r->observation.started_ms) {
        vda_pending_reader_close(r);
        vda_pending_fail(&r->observation, VDA_PENDING_CLOCK_ERROR, EIO);
        return -1;
    }
    r->observation.started_ms = now_ms;
    r->window_armed = 1;
    return 0;
}
int vda_pending_reader_begin_recover(vda_pending_reader_t *r, int fd,
                                     vda_pending_fallback_open_fn fallback_open,
                                     void *context, uint64_t now_ms,
                                     int *fallback_attempted, int *primary_error) {
    if (fallback_attempted)
        *fallback_attempted = 0;
    if (primary_error)
        *primary_error = 0;
    int result = vda_pending_reader_begin(r, fd, now_ms);
    if (!result || !fallback_open || !vda_pending_reader_fallback_eligible(r))
        return result;
    int error = r->observation.summary.error;
    if (primary_error)
        *primary_error = error;
    if (fallback_attempted)
        *fallback_attempted = 1;
    errno = 0;
    int fallback_fd = fallback_open(context);
    if (fallback_fd < 0) {
        error = errno ? errno : EIO;
        vda_pending_fail(&r->observation, VDA_PENDING_UNAVAILABLE, error);
        return -1;
    }
    return vda_pending_reader_begin(r, fallback_fd, now_ms);
}
int vda_pending_reader_poll(vda_pending_reader_t *r, uint64_t now_ms) {
    if (!r)
        return 1;
    if (r->observation.summary.status != VDA_PENDING_OBSERVING)
        return 1;
    if (r->fd < 0) {
        vda_pending_fail(&r->observation, VDA_PENDING_IO_ERROR, EBADF);
        return 1;
    }
    char buffer[512];
    int quiet = 0;
    for (unsigned i = 0; i < 8; i++) {
        ssize_t n = vda_reader_read(r->fd, buffer, sizeof(buffer));
        if (n > 0) {
            vda_pending_feed(&r->observation, buffer, (size_t)n);
            continue;
        }
        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            quiet = 1;
            break;
        }
        if (n < 0 && errno == EINTR)
            continue;
        vda_pending_fail(&r->observation, VDA_PENDING_IO_ERROR, n == 0 ? EPIPE : errno);
        break;
    }
    if (!quiet && r->observation.summary.status == VDA_PENDING_OBSERVING &&
        now_ms >= r->observation.started_ms &&
        now_ms - r->observation.started_ms >= VDA_PENDING_WINDOW_MS)
        vda_pending_fail(&r->observation, VDA_PENDING_INCOMPLETE, EOVERFLOW);
    int done = vda_pending_finish(&r->observation, now_ms, 0);
    if (done)
        vda_pending_reader_close(r);
    return done;
}
