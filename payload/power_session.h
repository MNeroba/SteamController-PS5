/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once
#include <stdatomic.h>
#include <stdint.h>
typedef struct {
  atomic_int paused;
  unsigned working;
  uint64_t last_ms;
} power_session_t;
#define POWER_SESSION_INIT {ATOMIC_VAR_INIT(0), 0, 0}
/* Manager is the sole observer; USB readers only load paused atomically. */
void power_session_observe(power_session_t *, int, uint64_t, uint64_t);
int power_session_paused(const power_session_t *);
