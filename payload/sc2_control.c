#include "sc2_control.h"

static int g_pause_active;

int sc2_pause_is_active(void) {
    return __atomic_load_n(&g_pause_active, __ATOMIC_ACQUIRE);
}

int sc2_pause_exchange(int active) {
    active = active ? 1 : 0;
    return __atomic_exchange_n(&g_pause_active, active, __ATOMIC_ACQ_REL);
}
