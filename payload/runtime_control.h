/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once

#include <stdint.h>

/* Process-wide stop signal shared by payload workers. Reset only before any
 * worker starts; request_stop() is idempotent and never performs cleanup. */
void runtime_control_reset(void);
void runtime_request_stop(void);
int runtime_is_stopping(void);
/* Interruptible bounded wait for worker loops. */
void runtime_sleep_ms(uint32_t milliseconds);
