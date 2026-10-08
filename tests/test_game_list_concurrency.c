#include "game_list.h"

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static pthread_mutex_t barrier_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t barrier_changed = PTHREAD_COND_INITIALIZER;
static int readers_at_barrier;

void gc_game_list_test_after_json_read(void) {
    pthread_mutex_lock(&barrier_lock);
    readers_at_barrier++;
    pthread_cond_broadcast(&barrier_changed);
    while (readers_at_barrier < 2)
        pthread_cond_wait(&barrier_changed, &barrier_lock);
    pthread_mutex_unlock(&barrier_lock);
}

struct lookup {
    const char *id;
    char name[GC_GAME_NAME_MAX];
    int result;
};

static void *lookup_name(void *opaque) {
    struct lookup *lookup = opaque;
    lookup->result = gc_game_name(lookup->id, lookup->name, sizeof(lookup->name));
    return NULL;
}

static int ensure_directory(const char *path) {
    if (mkdir(path, 0700) == 0 || errno == EEXIST) return 0;
    perror(path);
    return -1;
}

static int write_title(const char *id, const char *title) {
    char directory[128], path[160];
    snprintf(directory, sizeof(directory), "build/game-list-test/user/%s", id);
    if (ensure_directory(directory) != 0) return -1;
    snprintf(path, sizeof(path), "%s/param.json", directory);
    FILE *file = fopen(path, "w");
    if (!file) { perror(path); return -1; }
    int result = fprintf(file, "{\"titleName\":\"%s\"}", title) < 0 ? -1 : 0;
    if (fclose(file) != 0) result = -1;
    return result;
}

int main(void) {
    const char *id_a = "CUSA00001", *id_b = "CUSA00002";
    if (ensure_directory("build/game-list-test") != 0 ||
        ensure_directory("build/game-list-test/user") != 0 ||
        ensure_directory("build/game-list-test/system") != 0 ||
        write_title(id_a, "Puck Title") != 0 ||
        write_title(id_b, "Wired Title") != 0)
        return 1;

    struct lookup a = {id_a, {0}, -99}, b = {id_b, {0}, -99};
    pthread_t thread_a, thread_b;
    if (pthread_create(&thread_a, NULL, lookup_name, &a) != 0 ||
        pthread_create(&thread_b, NULL, lookup_name, &b) != 0) {
        fprintf(stderr, "failed to start concurrent title lookups\n");
        return 1;
    }
    pthread_join(thread_a, NULL);
    pthread_join(thread_b, NULL);

    unlink("build/game-list-test/user/CUSA00001/param.json");
    unlink("build/game-list-test/user/CUSA00002/param.json");
    rmdir("build/game-list-test/user/CUSA00001");
    rmdir("build/game-list-test/user/CUSA00002");
    rmdir("build/game-list-test/user");
    rmdir("build/game-list-test/system");
    rmdir("build/game-list-test");

    if (a.result != 0 || strcmp(a.name, "Puck Title") != 0 ||
        b.result != 0 || strcmp(b.name, "Wired Title") != 0) {
        fprintf(stderr, "concurrent title lookup contamination: %s=%s, %s=%s\n",
                id_a, a.name, id_b, b.name);
        return 1;
    }
    puts("concurrent game-title lookups keep independent JSON buffers");
    return 0;
}
