#pragma once
#include <stddef.h>
#include "gc_types.h"

enum {
    GAME_HOOKS_STOP_OK = 0,
    /* Worker joined; target-resident gateway could not be fully restored and
     * will remain mapped until target exit or a later safe cleanup attempt. */
    GAME_HOOKS_STOP_ROLLBACK_PENDING = 1
};
/* In-game Steam Controller support via the game's own pad imports
 * (PoorDS4-style): input merged into the player's controller reads, and the
 * game's vibration forwarded to the Steam Controller. */
void game_hooks_start(void);
int  game_hooks_stop(void);                /* joins after bounded rollback; see result */
void game_hooks_feed(const ScePadData *pad, int connected);   /* from the SC2 thread */
int  game_hooks_input_active(void);    /* hooks installed and merging input */
void game_hooks_set(int input_on, int vibe_on, int mode);   /* -1 = unchanged */
int  game_hooks_json(char *out, size_t n);
void game_hooks_set_audio(int on);
