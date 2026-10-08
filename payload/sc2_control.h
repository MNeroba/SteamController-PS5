#pragma once

#include <stdint.h>

/* Process-wide control flags shared by the portal, controller and manager
 * threads. Mutations return the previous normalized state. */
int sc2_pause_is_active(void);
int sc2_pause_exchange(int active);

/* A slot assignment is a short-lived lease on the PS5 user-assignment dialog.
 * The token contains a generation, so a delayed USB thread cannot release a
 * newer dialog after its own lease timed out. */
typedef uint64_t sc2_assignment_token_t;

sc2_assignment_token_t sc2_assignment_claim(int slot);
sc2_assignment_token_t sc2_assignment_owner(void);
int sc2_assignment_slot(sc2_assignment_token_t token);
int sc2_assignment_release(sc2_assignment_token_t token);
