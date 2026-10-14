/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "slot_lifecycle.h"

#include <stdio.h>
#include <stdlib.h>

static int checks;
static int thread_failure;

#define CHECK(condition) do { \
    checks++; \
    if (!(condition)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        exit(1); \
    } \
} while (0)

typedef struct {
    slot_registry_t *registry;
    slot_registry_token_t token;
    int create_result;
    int worker_result;
    int delete_result;
    int32_t create_handle;
    int32_t last_delete_handle;
    int create_calls;
    int worker_calls;
    int delete_calls;
    int bind_ok;
} fake_manager_t;

typedef struct {
    slot_registry_t *registry;
    unsigned worker_id;
} lifecycle_worker_args_t;

static int fake_create_virtual(uint32_t slot, int32_t *out_handle,
                               int *out_bind_ok, void *opaque) {
    fake_manager_t *manager = (fake_manager_t *)opaque;
    CHECK(slot < SLOT_REGISTRY_MAX_SLOTS);
    manager->create_calls++;
    if (manager->create_result != 0) return manager->create_result;
    *out_handle = manager->create_handle;
    *out_bind_ok = manager->bind_ok;
    return 0;
}

static int fake_start_worker(slot_registry_token_t token,
                             int32_t virtual_handle, int bind_ok,
                             void *opaque) {
    fake_manager_t *manager = (fake_manager_t *)opaque;
    slot_registry_entry_t entry;
    manager->worker_calls++;
    CHECK(token.index == manager->token.index);
    CHECK(token.generation == manager->token.generation);
    CHECK(slot_registry_snapshot(manager->registry, token, &entry) ==
          SLOT_REGISTRY_OK);
    CHECK(entry.state == SLOT_REGISTRY_ACTIVE);
    CHECK(virtual_handle == manager->create_handle || virtual_handle == -1);
    CHECK(bind_ok == manager->bind_ok || virtual_handle == -1);
    if (virtual_handle >= 0)
        CHECK(entry.virtual_ready && entry.virtual_handle == virtual_handle);
    else
        CHECK(!entry.virtual_ready);
    return manager->worker_result;
}

static int fake_delete_virtual(int32_t handle, void *opaque) {
    fake_manager_t *manager = (fake_manager_t *)opaque;
    manager->delete_calls++;
    manager->last_delete_handle = handle;
    return manager->delete_result;
}

static int stress_start_worker(slot_registry_token_t token,
                               int32_t virtual_handle, int bind_ok,
                               void *opaque) {
    lifecycle_worker_args_t *args = (lifecycle_worker_args_t *)opaque;
    slot_registry_entry_t entry;
    (void)args->worker_id;
    if (virtual_handle != -1 || bind_ok != 0 ||
        slot_registry_snapshot(args->registry, token, &entry) !=
            SLOT_REGISTRY_OK || entry.state != SLOT_REGISTRY_ACTIVE ||
        entry.token.generation != token.generation)
        __atomic_store_n(&thread_failure, 1, __ATOMIC_RELAXED);
    return 0;
}

static int stress_delete_virtual(int32_t handle, void *opaque) {
    (void)handle;
    (void)opaque;
    __atomic_store_n(&thread_failure, 1, __ATOMIC_RELAXED);
    return -1;
}

static void *lifecycle_worker(void *opaque) {
    lifecycle_worker_args_t *args = (lifecycle_worker_args_t *)opaque;
    slot_lifecycle_ops_t ops = {
        NULL,
        stress_start_worker,
        stress_delete_virtual
    };
    char identity[SLOT_REGISTRY_ID_MAX];
    snprintf(identity, sizeof(identity), "lifecycle-worker-%u",
             args->worker_id);
    for (int i = 0; i < 1000; i++) {
        slot_registry_token_t token;
        int cleanup = SLOT_REGISTRY_OK;
        if (slot_registry_claim(args->registry, identity, 0x28de,
                                (uint16_t)(0x1302 + args->worker_id),
                                &token) != SLOT_REGISTRY_OK ||
            slot_lifecycle_start(args->registry, token, 0, &ops, args,
                                 &cleanup) != SLOT_LIFECYCLE_OK ||
            cleanup != SLOT_REGISTRY_OK ||
            slot_lifecycle_release(args->registry, token,
                                   stress_delete_virtual, args) !=
                SLOT_REGISTRY_OK) {
            __atomic_store_n(&thread_failure, 1, __ATOMIC_RELAXED);
            return &thread_failure;
        }
    }
    return NULL;
}

static slot_lifecycle_ops_t make_ops(void) {
    slot_lifecycle_ops_t ops = {
        fake_create_virtual,
        fake_start_worker,
        fake_delete_virtual
    };
    return ops;
}

static slot_registry_token_t claim(slot_registry_t *registry,
                                   fake_manager_t *manager,
                                   const char *identity) {
    slot_registry_token_t token = { UINT32_MAX, 0 };
    CHECK(slot_registry_claim(registry, identity, 0x28de, 0x1304, &token) ==
          SLOT_REGISTRY_OK);
    manager->token = token;
    return token;
}

int main(void) {
    slot_registry_t registry;
    slot_lifecycle_ops_t ops = make_ops();
    fake_manager_t manager = {0};
    slot_registry_entry_t entry;
    slot_registry_token_t token;
    int cleanup = 999;

    CHECK(slot_registry_init(&registry) == SLOT_REGISTRY_OK);
    manager.registry = &registry;
    manager.last_delete_handle = -1;

    token = claim(&registry, &manager, "create-failure");
    manager.create_result = -1;
    manager.worker_result = 0;
    manager.delete_result = 0;
    manager.create_handle = 10;
    CHECK(slot_lifecycle_start(&registry, token, 1, &ops, &manager,
                               &cleanup) ==
          SLOT_LIFECYCLE_VDA_CREATE_FAILED);
    CHECK(cleanup == SLOT_REGISTRY_OK);
    CHECK(manager.create_calls == 1 && manager.worker_calls == 0);
    CHECK(manager.delete_calls == 0);
    CHECK(slot_registry_count_claimed(&registry) == 0);

    /* A failed create cannot prove no side effect; later scenarios use a
     * separate runtime registry instead of silently retrying that Add. */
    CHECK(registry.creation_blocked);
    slot_registry_destroy(&registry);
    CHECK(slot_registry_init(&registry) == SLOT_REGISTRY_OK);
    token = claim(&registry, &manager, "worker-failure");
    manager.create_result = 0;
    manager.create_handle = 11;
    manager.bind_ok = 1;
    manager.worker_result = -1;
    CHECK(slot_lifecycle_start(&registry, token, 1, &ops, &manager,
                               &cleanup) ==
          SLOT_LIFECYCLE_WORKER_START_FAILED);
    CHECK(cleanup == SLOT_REGISTRY_OK);
    CHECK(manager.worker_calls == 1);
    CHECK(manager.delete_calls == 1 && manager.last_delete_handle == 11);
    CHECK(slot_registry_snapshot(&registry, token, &entry) ==
          SLOT_REGISTRY_STALE);
    CHECK(slot_registry_count_claimed(&registry) == 0);

    token = claim(&registry, &manager, "delete-retry");
    manager.create_handle = 12;
    manager.worker_result = -1;
    manager.delete_result = -1;
    CHECK(slot_lifecycle_start(&registry, token, 1, &ops, &manager,
                               &cleanup) ==
          SLOT_LIFECYCLE_WORKER_START_FAILED);
    CHECK(cleanup == SLOT_REGISTRY_DELETE_FAILED);
    CHECK(slot_registry_snapshot(&registry, token, &entry) ==
          SLOT_REGISTRY_OK);
    CHECK(entry.state == SLOT_REGISTRY_RELEASING);
    CHECK(entry.virtual_ready && entry.virtual_handle == 12);
    CHECK(slot_registry_claim(&registry, "delete-retry", 0x28de, 0x1304,
                              &manager.token) == SLOT_REGISTRY_DUPLICATE);
    CHECK(manager.delete_calls == 2);
    manager.delete_result = 0;
    CHECK(slot_lifecycle_release(&registry, token, fake_delete_virtual,
                                 &manager) == SLOT_REGISTRY_OK);
    CHECK(manager.delete_calls == 3 && manager.last_delete_handle == 12);
    CHECK(slot_registry_count_claimed(&registry) == 0);
    token = claim(&registry, &manager, "delete-retry");
    CHECK(slot_lifecycle_release(&registry, token, fake_delete_virtual,
                                 &manager) == SLOT_REGISTRY_OK);

    token = claim(&registry, &manager, "lazy-worker");
    manager.worker_result = 0;
    int creates_before = manager.create_calls;
    int deletes_before = manager.delete_calls;
    CHECK(slot_lifecycle_start(&registry, token, 0, &ops, &manager,
                               &cleanup) == SLOT_LIFECYCLE_OK);
    CHECK(manager.create_calls == creates_before);
    CHECK(slot_registry_snapshot(&registry, token, &entry) ==
          SLOT_REGISTRY_OK);
    CHECK(entry.state == SLOT_REGISTRY_ACTIVE && !entry.virtual_ready);
    CHECK(slot_lifecycle_release(&registry, token, fake_delete_virtual,
                                 &manager) == SLOT_REGISTRY_OK);
    CHECK(manager.delete_calls == deletes_before);

    slot_registry_token_t owner = claim(&registry, &manager, "existing-owner");
    CHECK(slot_registry_activate(&registry, owner) == SLOT_REGISTRY_OK);
    CHECK(slot_registry_set_virtual(&registry, owner, 41) == SLOT_REGISTRY_OK);
    token = claim(&registry, &manager, "ambiguous-handle");
    manager.create_handle = 41;
    manager.worker_result = 0;
    deletes_before = manager.delete_calls;
    CHECK(slot_lifecycle_start(&registry, token, 1, &ops, &manager,
                               &cleanup) == SLOT_LIFECYCLE_VDA_ATTACH_FAILED);
    CHECK(cleanup == SLOT_REGISTRY_OK);
    CHECK(manager.delete_calls == deletes_before);
    CHECK(slot_registry_snapshot(&registry, owner, &entry) ==
          SLOT_REGISTRY_OK);
    CHECK(entry.virtual_ready && entry.virtual_handle == 41);
    CHECK(slot_lifecycle_release(&registry, owner, fake_delete_virtual,
                                 &manager) == SLOT_REGISTRY_OK);

    CHECK(slot_lifecycle_start(&registry, token, 1, &ops, &manager,
                               &cleanup) == SLOT_LIFECYCLE_INVALID);
    CHECK(slot_registry_count_claimed(&registry) == 0);

    pthread_t workers[SLOT_REGISTRY_MAX_SLOTS];
    lifecycle_worker_args_t worker_args[SLOT_REGISTRY_MAX_SLOTS];
    for (unsigned i = 0; i < SLOT_REGISTRY_MAX_SLOTS; i++) {
        worker_args[i].registry = &registry;
        worker_args[i].worker_id = i;
        CHECK(pthread_create(&workers[i], NULL, lifecycle_worker,
                            &worker_args[i]) == 0);
    }
    for (unsigned i = 0; i < SLOT_REGISTRY_MAX_SLOTS; i++) {
        void *result = NULL;
        CHECK(pthread_join(workers[i], &result) == 0);
        CHECK(result == NULL);
    }
    CHECK(__atomic_load_n(&thread_failure, __ATOMIC_RELAXED) == 0);
    CHECK(slot_registry_count_claimed(&registry) == 0);
    slot_registry_destroy(&registry);
    printf("Slot lifecycle checks passed (%d checks)\n", checks);
    return 0;
}
