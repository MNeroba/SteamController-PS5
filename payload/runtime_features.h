/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once
#include <stddef.h>

/* Owner-authorized runtime toggles for policy-gated services that already
 * have a worker/observer. Both persist the complete runtime policy through the
 * private writer, start or stop the service immediately, and revert policy and
 * service state if either step fails. Return 0 on success, -1 on failure and
 * write a human-readable message for the portal. */
int runtime_feature_set_handoff(int on, char *message, size_t capacity);
int runtime_feature_set_bridge(int on, char *message, size_t capacity);
