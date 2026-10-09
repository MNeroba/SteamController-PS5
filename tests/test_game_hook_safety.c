/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "game_hook_safety.h"

#include <stdio.h>
#include <stdlib.h>

static int checks;

#define CHECK(condition) do { \
    checks++; \
    if (!(condition)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        exit(1); \
    } \
} while (0)

int main(void)
{
    const uint64_t gateway = UINT64_C(0x1000);
    const uint64_t original = UINT64_C(0x2000);
    const uint64_t other_hook = UINT64_C(0x3000);

    CHECK(game_hook_slot_restore_action(gateway, gateway, original) ==
          GAME_HOOK_SLOT_RESTORE);
    CHECK(game_hook_slot_restore_action(original, gateway, original) ==
          GAME_HOOK_SLOT_ALREADY_RESTORED);
    CHECK(game_hook_slot_restore_action(other_hook, gateway, original) ==
          GAME_HOOK_SLOT_CONFLICT);
    CHECK(game_hook_slot_restore_action(gateway, 0, original) ==
          GAME_HOOK_SLOT_INVALID);

    for (unsigned failed = 0;
         failed < GAME_HOOK_SHUTDOWN_ROLLBACK_ATTEMPTS; failed++)
        CHECK(game_hook_shutdown_retry(failed));
    CHECK(!game_hook_shutdown_retry(GAME_HOOK_SHUTDOWN_ROLLBACK_ATTEMPTS));

    CHECK(game_hook_audio_lease_active(1, 0, 120));
    CHECK(game_hook_audio_lease_active(1, 120, 120));
    CHECK(!game_hook_audio_lease_active(1, 121, 120));
    CHECK(!game_hook_audio_lease_active(0, 0, 120));

    volatile uint32_t heartbeat = 100;
    volatile uint32_t seen = 99;
    volatile uint32_t age = 1000;
    CHECK(game_hook_heartbeat_age(&heartbeat, &seen, &age, 120) == 0);
    for (unsigned i = 0; i < 119; i++)
        (void)game_hook_heartbeat_age(&heartbeat, &seen, &age, 120);
    CHECK(game_hook_audio_lease_active(1,
          game_hook_heartbeat_age(&heartbeat, &seen, &age, 120), 120));
    CHECK(age == 120);
    CHECK(game_hook_heartbeat_age(&heartbeat, &seen, &age, 120) == 121);
    CHECK(age == 121);
    CHECK(!game_hook_audio_lease_active(1, age, 120));
    heartbeat++;
    CHECK(game_hook_heartbeat_age(&heartbeat, &seen, &age, 120) == 0);
    CHECK(age == 0);

    printf("game hook safety checks passed (%d checks)\n", checks);
    return 0;
}
