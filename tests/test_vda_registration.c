/* Host contracts for the production observer -> existing lifecycle/registry.
 * This is not validation of the PS5 Add/VDI/MBus ABI. */
#include "vda_pending.h"
#include "vda_bind_policy.h"
#include "slot_lifecycle.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned checks;
#define CHECK(x)                                                                                   \
    do {                                                                                           \
        ++checks;                                                                                  \
        assert(x);                                                                                 \
    } while (0)
#define VIRTUAL                                                                                    \
    "#LOGIN MGR# Receive Event : SCE_MBUS_EVENT_DEVICE_ADDED "                                     \
    "[DeviceId:0x70107][type:1][subType:22][capabilityBattery:0]\n"
typedef struct {
    unsigned adds, deletes, workers;
    int fail_delete;
} fake_t;
static int add(uint32_t slot, int32_t *h, int *bind, void *ctx) {
    (void)slot;
    fake_t *f = ctx;
    f->adds++;
    *h = -1;
    *bind = 0;
    return SLOT_LIFECYCLE_CREATE_PENDING;
}
static int del(int32_t h, void *ctx) {
    fake_t *f = ctx;
    CHECK(h == 0x70107);
    f->deletes++;
    return f->fail_delete ? -1 : 0;
}
static int start(slot_registry_token_t t, int32_t h, int bind, void *ctx) {
    (void)t;
    CHECK(h == -1 && !bind);
    ((fake_t *)ctx)->workers++;
    return 0;
}
static const slot_lifecycle_ops_t ops = {
    .create_virtual = add, .delete_virtual = del, .start_worker = start};
static void registration(int cancel, int release, int fail) {
    slot_registry_t r;
    slot_registry_token_t t;
    slot_registry_entry_t e;
    fake_t f = {0};
    CHECK(slot_registry_init(&r) == 0);
    CHECK(slot_registry_claim(&r, "puck", 0x28de, 0x1304, &t) == 0);
    int cleanup = -1;
    CHECK(slot_lifecycle_start(&r, t, 1, &ops, &f, &cleanup) == 0 && !cleanup);
    CHECK(f.adds == 1 && f.workers == 1); /* USB reader starts during pending, no restart */
    CHECK(slot_registry_snapshot(&r, t, &e) == 0 && e.virtual_creating && !e.virtual_ready);
    for (unsigned i = 0; i < 32; i++)
        CHECK(slot_lifecycle_attach(&r, t, &ops, &f, NULL) == SLOT_REGISTRY_BUSY);
    if (release)
        CHECK(slot_registry_request_release(&r, t) == 0);
    vda_pending_t event;
    vda_pending_begin(&event, 100, 0);
    vda_pending_feed(&event, VIRTUAL, strlen(VIRTUAL));
    CHECK(!vda_pending_candidate(&event)); /* never early-promote before full window */
    CHECK(vda_pending_finish(&event, 3100, 0) == 1);
    uint64_t id = vda_pending_candidate(&event);
    CHECK(id == 0x70107);
    f.fail_delete = fail;
    int result = slot_lifecycle_finish_pending(&r, t, id, cancel, &ops, &f);
    CHECK(result == ((cancel || release) && fail ? SLOT_REGISTRY_DELETE_FAILED : 0));
    CHECK(slot_registry_snapshot(&r, t, &e) == 0 && !e.virtual_creating &&
          e.token.generation == t.generation);
    if (cancel || release) {
        CHECK(f.deletes == 1);
        CHECK(e.virtual_ready == !!fail);
        CHECK(e.virtual_retiring == !!fail);
        if (fail)
            CHECK(slot_lifecycle_attach(&r, t, &ops, &f, NULL) == SLOT_REGISTRY_BUSY);
    } else
        CHECK(e.virtual_ready && e.virtual_correlated && e.virtual_device_id == id &&
              !e.virtual_retiring);
    CHECK(f.adds == 1 && f.workers == 1);
    slot_registry_token_t reconnect={0};
    if(fail) {
        CHECK(slot_registry_claim(&r,"reconnect",0x28de,0x1302,&reconnect)==0);
        CHECK(slot_registry_activate(&r,reconnect)==0);
        CHECK(slot_registry_begin_virtual_create(&r,reconnect)==SLOT_REGISTRY_BUSY);
    }
    f.fail_delete = 0;
    CHECK(slot_lifecycle_release(&r, t, del, &f) == 0);
    CHECK(slot_registry_finish_correlated_create(&r, t, id, 0) == SLOT_REGISTRY_STALE);
    if(fail) {
        CHECK(slot_registry_begin_virtual_create(&r,reconnect)==0);
        CHECK(slot_registry_finish_virtual_create(&r,reconnect,-1)==SLOT_REGISTRY_ERROR);
        CHECK(slot_lifecycle_release(&r,reconnect,del,&f)==0);
    }
    slot_registry_destroy(&r);
}
int main(void) {
    registration(0, 0, 0);
    registration(1, 0, 0);
    registration(0, 1, 0);
    registration(1, 0, 1);
    registration(0, 1, 1);
    int32_t one[] = {123, -1, -1, -1}, two[] = {123, 456, -1, -1}, dupe[] = {123, 123, -1, -1};
    CHECK(vda_bind_target(0, one, 0, 123, 0, 0) == 123);
    CHECK(vda_bind_target(-1, one, 0, 123, 0, 0) < 0);
    CHECK(vda_bind_target(0, one, -1, 123, 0, 0) < 0);
    CHECK(vda_bind_target(0, one, 0, 456, 0, 0) < 0);
    CHECK(vda_bind_target(0, two, 0, 123, 0, 0) < 0);
    CHECK(vda_bind_target(0, dupe, 0, 123, 0, 0) < 0);
    CHECK(vda_bind_target(0, one, 0, 123, 0, 1) < 0);  /* connected native DualSense protected */
    CHECK(vda_bind_target(0, one, 0, 123, -1, 0) < 0); /* unknown is not absent */
    CHECK(vda_bind_target(0, NULL, 0, 123, 0, 0) < 0);
    CHECK(vda_bind_target(0, one, 0, -1, 0, 0) < 0);
    printf("pending VDA registration/native protection: %u checks passed\n", checks);
    return 0;
}
