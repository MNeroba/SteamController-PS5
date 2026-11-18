/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "slot_registry.h"

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    unsigned worker_id;
} worker_args_t;

typedef struct {
    slot_registry_t *registry;
    slot_registry_token_t token;
    slot_registry_token_t other_token;
    int fail;
    int calls;
    int check_other;
} delete_context_t;

static int fake_vda_delete(int32_t handle, void *opaque) {
    delete_context_t *context = (delete_context_t *)opaque;
    slot_registry_entry_t entry;
    int32_t taken = -99;
    context->calls++;
    CHECK(handle >= 0);
    CHECK(slot_registry_snapshot(context->registry, context->token, &entry) ==
          SLOT_REGISTRY_OK);
    CHECK(!entry.virtual_ready && entry.virtual_deleting &&
          entry.virtual_handle == handle);
    CHECK(slot_registry_set_virtual(context->registry, context->token,
                                    handle + 1) == SLOT_REGISTRY_BAD_STATE);
    if (context->check_other)
        CHECK(slot_registry_set_virtual(context->registry, context->other_token,
                                        handle) == SLOT_REGISTRY_HANDLE_IN_USE);
    CHECK(slot_registry_take_virtual(context->registry, context->token,
                                     &taken) == SLOT_REGISTRY_BUSY);
    CHECK(slot_registry_release(context->registry, context->token) ==
          SLOT_REGISTRY_BUSY);
    CHECK(slot_registry_mark_confirmed(context->registry, context->token) ==
          SLOT_REGISTRY_BUSY);
    return context->fail ? -1 : 0;
}

static void *claim_release_worker(void *opaque) {
    worker_args_t *args = (worker_args_t *)opaque;
    char identity[SLOT_REGISTRY_ID_MAX];

    snprintf(identity, sizeof(identity), "usb-worker-%u", args->worker_id);
    for (int i = 0; i < 2000; i++) {
        slot_registry_token_t token;
        if (slot_registry_claim(args->registry, identity, 0x28de,
                                (uint16_t)(0x1302 + args->worker_id),
                                &token) != SLOT_REGISTRY_OK ||
            slot_registry_activate(args->registry, token) != SLOT_REGISTRY_OK ||
            slot_registry_request_release(args->registry, token) != SLOT_REGISTRY_OK ||
            slot_registry_release(args->registry, token) != SLOT_REGISTRY_OK) {
            __atomic_store_n(&thread_failure, 1, __ATOMIC_RELAXED);
            return &thread_failure;
        }
    }
    return NULL;
}

int main(void) {
    slot_registry_t registry;
    slot_registry_token_t first, second, tokens[SLOT_REGISTRY_MAX_SLOTS];
    slot_registry_entry_t entry;
    uint32_t found = UINT32_MAX;
    int32_t handle = -99;
    delete_context_t delete_context;

    CHECK(slot_registry_init(&registry) == SLOT_REGISTRY_OK);
    CHECK(slot_registry_snapshot_index(&registry, 0, &entry) == SLOT_REGISTRY_STALE);
    CHECK(slot_registry_snapshot_index(&registry, SLOT_REGISTRY_MAX_SLOTS,
                                       &entry) == SLOT_REGISTRY_ERROR);
    CHECK(slot_registry_claim(&registry, NULL, 1, 2, &first) == SLOT_REGISTRY_ERROR);
    CHECK(slot_registry_claim(&registry, "", 1, 2, &first) == SLOT_REGISTRY_ERROR);
    CHECK(slot_registry_claim(&registry, "identity-that-is-longer-than-the-slot-key-limit", 1, 2,
                              &first) == SLOT_REGISTRY_ERROR);
    CHECK(slot_registry_claim(&registry, "usb0", 0x28de, 0x1304, NULL) == SLOT_REGISTRY_ERROR);

    CHECK(slot_registry_claim(&registry, "usb0", 0x28de, 0x1304, &first) == SLOT_REGISTRY_OK);
    CHECK(slot_registry_snapshot_index(&registry, first.index, &entry) ==
          SLOT_REGISTRY_OK);
    CHECK(entry.token.generation == first.generation);
    CHECK(first.index == 0);
    CHECK(first.generation != 0);
    CHECK(slot_registry_count_claimed(&registry) == 1);
    CHECK(slot_registry_find_identity(&registry, "usb0", &found) == SLOT_REGISTRY_OK);
    CHECK(found == first.index);
    CHECK(slot_registry_find_identity(&registry, "", &found) == SLOT_REGISTRY_ERROR);
    CHECK(slot_registry_claim(&registry, "usb0", 0x28de, 0x1304, &second) == SLOT_REGISTRY_DUPLICATE);
    CHECK(slot_registry_snapshot(&registry, first, &entry) == SLOT_REGISTRY_OK);
    CHECK(entry.state == SLOT_REGISTRY_RESERVED);
    CHECK(entry.virtual_handle == -1 && entry.virtual_ready == 0);
    CHECK(entry.vendor_id == 0x28de && entry.product_id == 0x1304);
    CHECK(slot_registry_activate(&registry, first) == SLOT_REGISTRY_OK);
    CHECK(slot_registry_activate(&registry, first) == SLOT_REGISTRY_BAD_STATE);
    CHECK(slot_registry_release(&registry, first) == SLOT_REGISTRY_BAD_STATE);
    CHECK(slot_registry_set_virtual(&registry, first, -1) == SLOT_REGISTRY_ERROR);
    CHECK(slot_registry_set_virtual(&registry, first, 41) == SLOT_REGISTRY_OK);
    CHECK(slot_registry_set_virtual(&registry, first, 42) == SLOT_REGISTRY_BAD_STATE);
    CHECK(slot_registry_release(&registry, first) == SLOT_REGISTRY_BUSY);
    CHECK(slot_registry_mark_confirmed(&registry, first) == 1);
    CHECK(slot_registry_mark_confirmed(&registry, first) == 0);
    CHECK(slot_registry_take_virtual(&registry, first, &handle) == SLOT_REGISTRY_OK);
    CHECK(handle == 41);
    CHECK(slot_registry_snapshot(&registry, first, &entry) == SLOT_REGISTRY_OK);
    CHECK(entry.confirmed == 0);
    CHECK(slot_registry_take_virtual(&registry, first, &handle) == SLOT_REGISTRY_OK);
    CHECK(handle == -1);
    delete_context.registry = &registry;
    delete_context.token = first;
    delete_context.fail = 0;
    delete_context.calls = 0;
    CHECK(slot_registry_delete_virtual(&registry, first, fake_vda_delete,
                                       &delete_context) == SLOT_REGISTRY_OK);
    CHECK(delete_context.calls == 0);
    CHECK(slot_registry_request_release(&registry, first) == SLOT_REGISTRY_OK);
    CHECK(slot_registry_release(&registry, first) == SLOT_REGISTRY_OK);
    CHECK(slot_registry_count_claimed(&registry) == 0);
    CHECK(slot_registry_snapshot(&registry, first, &entry) == SLOT_REGISTRY_STALE);
    CHECK(slot_registry_find_identity(&registry, "usb0", &found) == SLOT_REGISTRY_ERROR);

    CHECK(slot_registry_claim(&registry, "usb0", 0x28de, 0x1304, &second) == SLOT_REGISTRY_OK);
    CHECK(second.index == first.index);
    CHECK(second.generation != first.generation);
    CHECK(slot_registry_activate(&registry, first) == SLOT_REGISTRY_STALE);
    CHECK(slot_registry_release(&registry, first) == SLOT_REGISTRY_STALE);
    CHECK(slot_registry_snapshot(&registry, second, &entry) == SLOT_REGISTRY_OK);
    CHECK(entry.state == SLOT_REGISTRY_RESERVED);
    CHECK(slot_registry_set_virtual(&registry, first, 84) == SLOT_REGISTRY_STALE);
    CHECK(slot_registry_set_virtual(&registry, second, 84) == SLOT_REGISTRY_OK);
    delete_context.registry = &registry;
    delete_context.token = first;
    delete_context.fail = 0;
    delete_context.calls = 0;
    CHECK(slot_registry_delete_virtual(&registry, first, fake_vda_delete,
                                       &delete_context) == SLOT_REGISTRY_STALE);
    CHECK(delete_context.calls == 0);
    CHECK(slot_registry_take_virtual(&registry, first, &handle) == SLOT_REGISTRY_STALE);
    CHECK(slot_registry_snapshot(&registry, second, &entry) == SLOT_REGISTRY_OK);
    CHECK(entry.virtual_ready && entry.virtual_handle == 84);
    CHECK(slot_registry_take_virtual(&registry, second, &handle) == SLOT_REGISTRY_OK);
    CHECK(handle == 84);
    CHECK(slot_registry_request_release(&registry, second) == SLOT_REGISTRY_OK);
    CHECK(slot_registry_release(&registry, second) == SLOT_REGISTRY_OK);

    for (uint32_t i = 0; i < SLOT_REGISTRY_MAX_SLOTS; i++) {
        char identity[SLOT_REGISTRY_ID_MAX];
        snprintf(identity, sizeof(identity), "fill-%u", i);
        CHECK(slot_registry_claim(&registry, identity, 1, (uint16_t)i,
                                  &tokens[i]) == SLOT_REGISTRY_OK);
    }
    CHECK(slot_registry_count_claimed(&registry) == SLOT_REGISTRY_MAX_SLOTS);
    CHECK(slot_registry_claim(&registry, "overflow", 1, 1, &first) == SLOT_REGISTRY_FULL);
    CHECK(slot_registry_set_virtual(&registry, tokens[0], 73) == SLOT_REGISTRY_OK);
    CHECK(slot_registry_set_virtual(&registry, tokens[1], 73) == SLOT_REGISTRY_HANDLE_IN_USE);
    delete_context.registry = &registry;
    delete_context.token = tokens[0];
    delete_context.fail = 1;
    delete_context.calls = 0;
    delete_context.other_token = tokens[1];
    delete_context.check_other = 1;
    CHECK(slot_registry_delete_virtual(&registry, tokens[0], fake_vda_delete,
                                       &delete_context) == SLOT_REGISTRY_DELETE_FAILED);
    CHECK(delete_context.calls == 1);
    CHECK(slot_registry_snapshot(&registry, tokens[0], &entry) == SLOT_REGISTRY_OK);
    CHECK(entry.virtual_ready && !entry.virtual_deleting &&
          entry.virtual_handle == 73);
    CHECK(slot_registry_release(&registry, tokens[0]) == SLOT_REGISTRY_BUSY);
    CHECK(slot_registry_request_release(&registry, tokens[0]) == SLOT_REGISTRY_OK);
    CHECK(slot_registry_snapshot_index(&registry, tokens[0].index, &entry) ==
          SLOT_REGISTRY_OK);
    CHECK(entry.state == SLOT_REGISTRY_RELEASING);
    CHECK(slot_registry_take_virtual(&registry, tokens[0], &handle) ==
          SLOT_REGISTRY_BUSY);
    CHECK(slot_registry_mark_confirmed(&registry, tokens[0]) ==
          SLOT_REGISTRY_BUSY);
    CHECK(slot_registry_activate(&registry, tokens[0]) == SLOT_REGISTRY_BAD_STATE);
    CHECK(slot_registry_set_virtual(&registry, tokens[0], 88) ==
          SLOT_REGISTRY_BAD_STATE);
    delete_context.fail = 0;
    CHECK(slot_registry_delete_virtual(&registry, tokens[0], fake_vda_delete,
                                       &delete_context) == SLOT_REGISTRY_OK);
    CHECK(delete_context.calls == 2);
    CHECK(slot_registry_snapshot(&registry, tokens[0], &entry) == SLOT_REGISTRY_OK);
    CHECK(!entry.virtual_ready && !entry.virtual_deleting &&
          entry.virtual_handle == -1);
    for (uint32_t i = 0; i < SLOT_REGISTRY_MAX_SLOTS; i++) {
        CHECK(slot_registry_request_release(&registry, tokens[i]) == SLOT_REGISTRY_OK);
        CHECK(slot_registry_release(&registry, tokens[i]) == SLOT_REGISTRY_OK);
    }

    pthread_t threads[SLOT_REGISTRY_MAX_SLOTS];
    worker_args_t args[SLOT_REGISTRY_MAX_SLOTS];
    for (unsigned i = 0; i < SLOT_REGISTRY_MAX_SLOTS; i++) {
        args[i].registry = &registry;
        args[i].worker_id = i;
        CHECK(pthread_create(&threads[i], NULL, claim_release_worker, &args[i]) == 0);
    }
    for (unsigned i = 0; i < SLOT_REGISTRY_MAX_SLOTS; i++) {
        void *result = NULL;
        CHECK(pthread_join(threads[i], &result) == 0);
        CHECK(result == NULL);
    }
    CHECK(__atomic_load_n(&thread_failure, __ATOMIC_RELAXED) == 0);
    CHECK(slot_registry_count_claimed(&registry) == 0);

    slot_registry_destroy(&registry);
    printf("Slot registry checks passed (%d checks)\n", checks);
    return 0;
}
