#include "sc2_profile.h"
#include "game_list.h"
#include "gc_types.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

static int checks;

#define CHECK(condition) do { \
    checks++; \
    if (!(condition)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        exit(1); \
    } \
} while (0)

/* The parser test links the complete profile source file; these target-only
 * game lookup functions are unused by this host test. */
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

typedef struct {
    const char *text;
    const uint32_t *expected;
    pthread_mutex_t *lock;
    pthread_cond_t *condition;
    int *ready;
    int *start;
} parse_thread_t;

static void *parse_concurrently(void *opaque) {
    parse_thread_t *args = opaque;
    pthread_mutex_lock(args->lock);
    ++*args->ready;
    pthread_cond_broadcast(args->condition);
    while (!*args->start) pthread_cond_wait(args->condition, args->lock);
    pthread_mutex_unlock(args->lock);

    for (int iteration = 0; iteration < 5000; ++iteration) {
        sc2_profile_t profile;
        sc2_profile_default(&profile);
        sc2_profile_parse(args->text, &profile);
        if (profile.map[IN_A] != args->expected[0] ||
            profile.map[IN_B] != args->expected[1] ||
            profile.map[IN_X] != args->expected[2] ||
            profile.map[IN_Y] != args->expected[3])
            return (void *)(uintptr_t)1;
    }
    return NULL;
}

static void check_concurrent_combo_parsing(void) {
    static const char *const texts[] = {
        "A=CROSS+CIRCLE\nB=TRIANGLE+SQUARE\nX=L1+R1\nY=UP+DOWN\n",
        "A=UP+LEFT\nB=DOWN+RIGHT\nX=L2+R2\nY=OPTIONS+CREATE\n",
        "A=L3+R3\nB=PS+TOUCHPAD\nX=CROSS+UP\nY=CIRCLE+DOWN\n",
        "A=SQUARE+LEFT\nB=TRIANGLE+RIGHT\nX=CREATE+OPTIONS\nY=L1+L2\n",
    };
    static const uint32_t expected[][4] = {
        {SCE_PAD_BUTTON_CROSS | SCE_PAD_BUTTON_CIRCLE,
         SCE_PAD_BUTTON_TRIANGLE | SCE_PAD_BUTTON_SQUARE,
         SCE_PAD_BUTTON_L1 | SCE_PAD_BUTTON_R1,
         SCE_PAD_BUTTON_UP | SCE_PAD_BUTTON_DOWN},
        {SCE_PAD_BUTTON_UP | SCE_PAD_BUTTON_LEFT,
         SCE_PAD_BUTTON_DOWN | SCE_PAD_BUTTON_RIGHT,
         SCE_PAD_BUTTON_L2 | SCE_PAD_BUTTON_R2,
         SCE_PAD_BUTTON_OPTIONS | SCE_PAD_BUTTON_SHARE},
        {SCE_PAD_BUTTON_L3 | SCE_PAD_BUTTON_R3,
         SCE_PAD_BUTTON_PS | SCE_PAD_BUTTON_TOUCH_PAD,
         SCE_PAD_BUTTON_CROSS | SCE_PAD_BUTTON_UP,
         SCE_PAD_BUTTON_CIRCLE | SCE_PAD_BUTTON_DOWN},
        {SCE_PAD_BUTTON_SQUARE | SCE_PAD_BUTTON_LEFT,
         SCE_PAD_BUTTON_TRIANGLE | SCE_PAD_BUTTON_RIGHT,
         SCE_PAD_BUTTON_SHARE | SCE_PAD_BUTTON_OPTIONS,
         SCE_PAD_BUTTON_L1 | SCE_PAD_BUTTON_L2},
    };
    pthread_t threads[4];
    parse_thread_t args[4];
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
    int ready = 0, start = 0;

    for (size_t i = 0; i < 4; ++i) {
        args[i] = (parse_thread_t){texts[i], expected[i], &lock, &condition,
                                   &ready, &start};
        if (pthread_create(&threads[i], NULL, parse_concurrently, &args[i]) != 0) {
            fprintf(stderr, "FAIL: pthread_create for concurrent parser test\n");
            exit(1);
        }
    }
    pthread_mutex_lock(&lock);
    while (ready != 4) pthread_cond_wait(&condition, &lock);
    start = 1;
    pthread_cond_broadcast(&condition);
    pthread_mutex_unlock(&lock);

    for (size_t i = 0; i < 4; ++i) {
        void *result = NULL;
        if (pthread_join(threads[i], &result) != 0) {
            fprintf(stderr, "FAIL: concurrent profile combo parsing\n");
            exit(1);
        }
        CHECK(result == NULL);
    }
    pthread_cond_destroy(&condition);
    pthread_mutex_destroy(&lock);
}

int main(void) {
    sc2_profile_t p;
    char digits[181];
    char bad[1024];

    sc2_profile_default(&p);
    CHECK(p.long_ms == 400);
    CHECK(p.deadzone == 9);
    CHECK(p.gyro_sens_x == 20);

    sc2_profile_parse(
        "LONG_MS=9999\nDOUBLE_MS=-1\nTURBO_MS=1\nDEADZONE=45\n"
        "GYRO_SENS_X=0\nGYRO_CURVE=999\nTRIGGER_FX=-4\nFLICK_SPEED=1\n"
        "A.turbo=0\nA.toggle=1\nINVERT_LY=2\nMOTION=0\n", &p);
    CHECK(p.long_ms == 2000);
    CHECK(p.double_ms == 100);
    CHECK(p.turbo_ms == 20);
    CHECK(p.deadzone == 40);
    CHECK(p.gyro_sens_x == 1);
    CHECK(p.gyro_curve == 30);
    CHECK(p.trig_fx == 0);
    CHECK(p.flick_speed == 90);
    CHECK(p.turbo[IN_A] == 0);
    CHECK(p.toggle[IN_A] == 1);
    CHECK(p.invert_ly == 1);
    CHECK(p.motion == 0);

    memset(digits, '9', sizeof(digits) - 1);
    digits[sizeof(digits) - 1] = '\0';
    p.turbo[IN_A] = 1;
    p.toggle[IN_A] = 0;
    snprintf(bad, sizeof(bad),
        "LONG_MS=%s\nDOUBLE_MS=200ms\nTURBO_MS=\nDEADZONE=%s\n"
        "A.turbo=not-a-number\nA.toggle=%s\nINVERT_LY=1x\n"
        "GYRO_SENS_X=2147483648\nFLICK_SPEED=-%s\n",
        digits, digits, digits, digits);
    sc2_profile_parse(bad, &p);
    CHECK(p.long_ms == 2000);
    CHECK(p.double_ms == 100);
    CHECK(p.turbo_ms == 20);
    CHECK(p.deadzone == 40);
    CHECK(p.turbo[IN_A] == 1);
    CHECK(p.toggle[IN_A] == 0);
    CHECK(p.invert_ly == 1);
    CHECK(p.gyro_sens_x == 1);
    CHECK(p.flick_speed == 90);

    sc2_profile_parse("LONG_MS=2147483648\nTURBO_MS=-2147483648\n", &p);
    CHECK(p.long_ms == 2000);  /* outside int range is ignored */
    CHECK(p.turbo_ms == 20);   /* in-range signed values are clamped */

    check_concurrent_combo_parsing();

    printf("Steam profile parser checks passed (%d checks)\n", checks);
    return 0;
}
