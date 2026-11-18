/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "power_session.h"
int power_session_paused(const power_session_t *p) {
  return atomic_load(&p->paused);
}
void power_session_observe(power_session_t *p, int result, uint64_t bits,
                           uint64_t ms) {
  int spaced = ms >= p->last_ms && ms - p->last_ms >= 500;
  p->last_ms = ms;
  if (result || (bits & 65535) != 1000) {
    p->working = 0;
    if (!result && ((bits & 65535) == 300 || (bits & 65535) == 500))
      atomic_store(&p->paused, 1);
    return;
  }
  if (!power_session_paused(p))
    return;
  if (!spaced) {
    p->working = 0;
    return;
  }
  if (++p->working >= 2) {
    p->working = 0;
    atomic_store(&p->paused, 0);
  }
}
