/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "slot_lifecycle.h"
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
typedef struct {
    int creates, deletes, fail_delete, release_during_create;
    int32_t next;
    slot_registry_t *r;
    slot_registry_token_t t;
} fake_t;
static int create(uint32_t slot, int32_t *h, int *bind, void *ctx) {
    fake_t *f = ctx;
    assert(slot == f->t.index);
    f->creates++;
    *h = ++f->next;
    *bind = 0;
    if (f->release_during_create)
        assert(slot_registry_request_release(f->r, f->t) == 0);
    return 0;
}
static int del(int32_t h, void *ctx) {
    fake_t *f = ctx;
    assert(h > 0);
    f->deletes++;
    return f->fail_delete ? -1 : 0;
}
static void *race(void *ctx) {
    fake_t *f = ctx;
    if (slot_registry_begin_virtual_create(f->r, f->t) == 0)
        return (void *)1;
    return 0;
}
int main(void) {
    slot_registry_t r;
    assert(slot_registry_init(&r) == 0);
    slot_registry_token_t t;
    assert(slot_registry_claim(&r, "/dev/ugen1.1", 0x28de, 0x1304, &t) == 0);
    assert(slot_registry_activate(&r, t) == 0);
    fake_t f = {.r = &r, .t = t, .next = 30};
    slot_lifecycle_ops_t ops = {.create_virtual = create, .delete_virtual = del};
    slot_recovery_t recovery = {0};
    user_session_snapshot_t u = {.read_ok = 1, .can_register = 1};
    assert(slot_lifecycle_recover(&r, t, &recovery, &u, 1, &ops, &f) == 0 && f.creates == 0);
    assert(slot_lifecycle_attach(&r, t, &ops, &f, 0) == 0); /* startup at login screen */
    slot_registry_entry_t e;
    assert(slot_registry_snapshot(&r, t, &e) == 0 && e.virtual_handle == 31);
    u.logout_epoch = 1;
    u.logged_out = 1;
    u.can_register = 0;
    f.fail_delete = 1;
    assert(slot_lifecycle_recover(&r, t, &recovery, &u, 1, &ops, &f) ==
           SLOT_REGISTRY_DELETE_FAILED);
    assert(f.creates == 1 && recovery.pending);
    assert(slot_registry_snapshot(&r, t, &e) == 0 && e.virtual_handle == 31 && e.virtual_ready);
    assert(slot_lifecycle_attach(&r, t, &ops, &f, 0) == SLOT_REGISTRY_BUSY); /* no duplicate */
    f.fail_delete = 0;
    assert(slot_lifecycle_recover(&r, t, &recovery, &u, 1, &ops, &f) == 1);
    for (int i = 0; i < 10; i++)
        assert(slot_lifecycle_recover(&r, t, &recovery, &u, 1, &ops, &f) == 1);
    assert(f.creates == 1);
    u.logged_out = 0;
    u.can_register = 1;
    assert(slot_lifecycle_recover(&r, t, &recovery, &u, 1, &ops, &f) == 0 && f.creates == 2 &&
           recovery.recovered == 1);
    assert(slot_registry_snapshot(&r, t, &e) == 0 && e.token.generation == t.generation &&
           e.virtual_handle == 32);
    /* A paused/released controller never resurrects. Explicit resume is separate. */
    u.logout_epoch = 2;
    assert(slot_lifecycle_recover(&r, t, &recovery, &u, 0, &ops, &f) == 0 && f.creates == 2);
    assert(slot_registry_delete_virtual(&r, t, del, &f) == 0);
    assert(slot_lifecycle_recover(&r, t, &recovery, &u, 1, &ops, &f) == 0 && f.creates == 2);
    /* A pending AddDevice reserves its generation even when release races. */
    assert(slot_registry_begin_virtual_create(&r, t) == 0);
    assert(slot_lifecycle_recover(&r, t, &recovery,
                                  &(user_session_snapshot_t){.logout_epoch = 3, .can_register = 1},
                                  1, &ops, &f) == SLOT_REGISTRY_BUSY);
    assert(slot_registry_request_release(&r, t) == 0);
    assert(slot_registry_release(&r, t) == SLOT_REGISTRY_BUSY);
    assert(slot_registry_finish_virtual_create(&r, t, 55) == 0);
    assert(slot_lifecycle_release(&r, t, del, &f) == 0);
    assert(slot_registry_claim(&r, "new-session", 0x28de, 0x1302, &f.t) == 0);
    assert(slot_registry_activate(&r, f.t) == 0);
    assert(f.t.generation != t.generation);
    assert(slot_lifecycle_recover(&r, t, &recovery, &u, 1, &ops, &f) ==
           SLOT_LIFECYCLE_INVALID); /* stale generation */
    pthread_t threads[12];
    int winners = 0;
    for (int i = 0; i < 12; i++)
        assert(pthread_create(&threads[i], 0, race, &f) == 0);
    for (int i = 0; i < 12; i++) {
        void *win;
        assert(pthread_join(threads[i], &win) == 0);
        winners += win != 0;
    }
    assert(winners == 1);
    assert(slot_registry_finish_virtual_create(&r, f.t, -1) == SLOT_REGISTRY_ERROR);
    f.release_during_create = 1;
    assert(slot_lifecycle_attach(&r, f.t, &ops, &f, 0) == 0);
    assert(slot_registry_snapshot(&r, f.t, &e) == 0 && e.state == SLOT_REGISTRY_RELEASING &&
           e.virtual_ready);
    assert(slot_lifecycle_release(&r, f.t, del, &f) ==
           0); /* Ambiguous receipt neither deletes another owner's handle nor loops at shutdown. */
    slot_registry_token_t a, b;
    assert(slot_registry_claim(&r, "owner", 0x28de, 0x1302, &a) == 0);
    assert(slot_registry_activate(&r, a) == 0);
    assert(slot_registry_set_virtual(&r, a, 99) == 0);
    assert(slot_registry_claim(&r, "alias", 0x28de, 0x1304, &b) == 0);
    assert(slot_registry_activate(&r, b) == 0);
    assert(slot_registry_begin_virtual_create(&r, b) == 0);
    assert(slot_registry_finish_virtual_create(&r, b, 99) == SLOT_REGISTRY_HANDLE_IN_USE);
    assert(slot_lifecycle_release(&r, b, del, &f) == 0); /* owns no independently proven handle */
    assert(slot_lifecycle_release(&r, a, del, &f) == 0);
    assert(slot_registry_claim(&r, "blocked", 0x28de, 0x1302, &b) == 0);
    assert(slot_registry_activate(&r, b) == 0);
    assert(slot_registry_begin_virtual_create(&r, b) ==
           SLOT_REGISTRY_BUSY); /* inhibit all new Add */
    assert(slot_lifecycle_release(&r, b, del, &f) == 0);
    slot_registry_destroy(&r);
    puts("owned slot recovery/pending registration/race checks passed");
    return 0;
}
