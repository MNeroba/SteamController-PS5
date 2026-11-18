#include "sc2_menu.h"
#include "sc2_profile.h"
#include "game_list.h"
#include "gc_types.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static unsigned int checks;
static char notification[256];

#define CHECK(condition) do { \
    checks++; \
    if (!(condition)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        exit(1); \
    } \
} while (0)

int gc_game_name(const char *id, char *out, size_t outsz) {
    (void)id;
    if (outsz) out[0] = '\0';
    return 0;
}

int sceLncUtilGetAppIdOfRunningBigApp(void) { return 0; }
int sceLncUtilGetAppTitleId(uint32_t app_id, char *title_id) {
    (void)app_id;
    (void)title_id;
    return -1;
}

void sc2_profile_test_before_commit(const char *id, uint32_t generation) {
    (void)id;
    (void)generation;
}

static void capture_notification(const char *message) {
    snprintf(notification, sizeof(notification), "%s", message);
}

void sc2_haptic_pulse(int pad, uint16_t on_us, uint16_t off_us, uint16_t count) {
    (void)pad; (void)on_us; (void)off_us; (void)count;
}

void sc2_haptic_tick(void) {}

void sc2_menu_test_do_remap(int from, int to);

int main(void) {
    sc2_profile_t profile;
    char fail_tmp[] = "build/profile-menu-test/target.ini.tmp";

    sc2_profile_default(&profile);
    snprintf(profile.name, sizeof(profile.name), "Menu test");
    CHECK(sc2_store_save("target", &profile) == 0);
    sc2_notify_fn = capture_notification;
    sc2_select_override("target");

    notification[0] = '\0';
    sc2_menu_test_do_remap(IN_A, IN_B);
    CHECK(strstr(notification, "Saved to") != NULL);
    CHECK(sc2_store_load("target", &profile) == 0);
    CHECK(profile.map[IN_A] == SCE_PAD_BUTTON_CIRCLE);

    unlink(fail_tmp);
    rmdir(fail_tmp);
    CHECK(mkdir(fail_tmp, 0700) == 0);
    notification[0] = '\0';
    sc2_menu_test_do_remap(IN_A, IN_X);
    CHECK(strstr(notification, "Could not save profile") != NULL);
    CHECK(strstr(notification, "Saved to") == NULL);
    CHECK(sc2_store_load("target", &profile) == 0);
    CHECK(profile.map[IN_A] == SCE_PAD_BUTTON_CIRCLE);

    rmdir(fail_tmp);
    printf("Steam menu profile persistence: %u checks passed\n", checks);
    return 0;
}
