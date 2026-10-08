#pragma once

/* Process-wide control flags shared by the portal, controller and manager
 * threads. Mutations return the previous normalized state. */
int sc2_pause_is_active(void);
int sc2_pause_exchange(int active);
