/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once
#include <stddef.h>
#include <stdint.h>

/* Read-only diagnostic state, NEVER a registration or ownership authority.
 * One owner feeds bounded chunks. Value-only summary is safe to copy into the
 * diagnostic recorder. That summary contains no raw lines or candidate IDs. */
#define VDA_PENDING_WINDOW_MS 3000u
#define VDA_PENDING_LINE_MAX 1024u
typedef enum {
    VDA_PENDING_IDLE,
    VDA_PENDING_OBSERVING,
    VDA_PENDING_UNAVAILABLE,
    VDA_PENDING_NO_EVENT,
    VDA_PENDING_UNPROVEN,
    VDA_PENDING_AMBIGUOUS,
    VDA_PENDING_INCOMPLETE,
    VDA_PENDING_IO_ERROR,
    VDA_PENDING_CLOCK_ERROR,
    VDA_PENDING_CANCELLED,
    VDA_PENDING_REJECTED
} vda_pending_status_t;
typedef struct {
    vda_pending_status_t status;
    uint32_t virtual_events, distinct_virtuals, malformed, truncated;
    uint64_t elapsed_ms;
    int error;
} vda_pending_summary_t;
typedef struct {
    vda_pending_summary_t summary;
    uint64_t started_ms, first_id, native_ids[4];
    unsigned native_count;
    char line[VDA_PENDING_LINE_MAX];
    size_t used;
    int dropping;
} vda_pending_t;
void vda_pending_begin(vda_pending_t *state, uint64_t now_ms, int discard_partial);
void vda_pending_feed(vda_pending_t *state, const char *data, size_t size);
/* Returns 1 once terminal, 0 while observing. Finish erases raw/ID state. */
int vda_pending_finish(vda_pending_t *state, uint64_t now_ms, int cancelled);
void vda_pending_fail(vda_pending_t *state, vda_pending_status_t status, int error);
const char *vda_pending_name(vda_pending_status_t status);

/* Experimental owner-authorized correlation candidate, not a proven Add ABI.
 * Available only after a complete unique window. Native/ambiguous/malformed
 * events never supply a candidate. Caller must retain its registry reservation. */
uint64_t vda_pending_candidate(const vda_pending_t *state);
typedef struct {
    vda_pending_t observation;
    int fd;
    int window_armed;
} vda_pending_reader_t;
typedef int (*vda_pending_fallback_open_fn)(void *context);
/* Takes ownership of fd even on failure. Requires a freshly opened nonblocking
 * stream. Bounded drain must reach EAGAIN before the caller may perform Add. */
int vda_pending_reader_begin(vda_pending_reader_t *reader, int fd, uint64_t now_ms);
int vda_pending_reader_fallback_eligible(const vda_pending_reader_t *reader);
/* Once after drain and immediately before Add: exclude setup/drain time from
 * the minimum observation window, preserving stale partial-line exclusion.
 * Invalid state or a backwards clock closes the reader and fails closed. */
int vda_pending_reader_start_window(vda_pending_reader_t *reader, uint64_t now_ms);
int vda_pending_reader_begin_recover(vda_pending_reader_t *reader, int fd,
                                     vda_pending_fallback_open_fn fallback_open,
                                     void *context, uint64_t now_ms,
                                     int *fallback_attempted, int *primary_error);
int vda_pending_reader_poll(vda_pending_reader_t *reader, uint64_t now_ms);
void vda_pending_reader_close(vda_pending_reader_t *reader);
