/* Host-only stdin adapter to the production profile parser/formatter.
 * It touches no PS5 ABI, store, console, or controller. */
#include "sc2_profile.h"
#include "game_list.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int gc_game_name(const char *id, char *out, size_t size) {
    (void)id;
    if (size) out[0] = '\0';
    return 0;
}
int sceLncUtilGetAppIdOfRunningBigApp(void) { return 0; }
int sceLncUtilGetAppTitleId(uint32_t id, char *title) {
    (void)id; (void)title;
    return -1;
}

int main(void) {
    char input[65538], output[SC2_PROFILE_CAPACITY];
    size_t length = fread(input, 1, sizeof(input) - 1, stdin);
    if (ferror(stdin) || !feof(stdin)) return 1;
    char *separator = memchr(input, '\0', length);
    if (!separator) return 1;
    input[length] = '\0';
    sc2_profile_t profile;
    sc2_profile_default(&profile);
    sc2_profile_parse(input, &profile);
    if (sc2_profile_parse_checked(separator + 1, &profile, 0) != 0) return 1;
    int bytes = sc2_profile_format(&profile, output, sizeof(output));
    if (bytes < 0 || (size_t)bytes >= sizeof(output)) return 1;
    return fwrite(output, 1, (size_t)bytes, stdout) == (size_t)bytes ? 0 : 1;
}
