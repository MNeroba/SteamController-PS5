/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "hid_backend.h"
#include "power_session.h"
#include "slot_lifecycle.h"
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
static int adds, inserts, deletes, closes, fail_add, fail_delete, fail_insert,
    fail_open;
static int32_t returned = 321;
static uint8_t last[HID_REPORT_SIZE];
static int op(void *c) {
  (void)c;
  if (fail_open) {
    errno = EACCES;
    return -1;
  }
  return 17;
}
static int cl(int fd, void *c) {
  (void)c;
  assert(fd == 17);
  closes++;
  return 0;
}
static int request(int fd, unsigned long code, void *ptr, void *c) {
  (void)c;
  assert(fd == 17);
  if (code == HID_ADD) {
    hid_request_t *r = ptr;
    uint8_t *d = r->data;
    adds++;
    assert(r->command == 3 && !r->reserved && d[0] == 3 && d[28] == 123 &&
           d[32] == 1);
    for (unsigned i = 0; i < 168; i++)
      assert(d[i] == (i == 0 ? 3 : i == 28 ? 123 : i == 32 ? 1 : 0));
    *(int32_t *)r->output = returned;
    if (fail_add) {
      errno = EIO;
      return -1;
    }
    return 0;
  }
  if (code == HID_INSERT) {
    hid_request_t *r = ptr;
    inserts++;
    assert(r->command == 321 && !r->output);
    memcpy(last, r->data, HID_REPORT_SIZE);
    if (fail_insert) {
      errno = EAGAIN;
      return -1;
    }
    return 0;
  }
  assert(code == HID_DELETE);
  hid_delete_t *r = ptr;
  assert(r->command == 5 && r->handle == 321);
  deletes++;
  if (fail_delete) {
    errno = EIO;
    return -1;
  }
  return 0;
}
static int auth_calls, deny_at;
static int authorize(int32_t user, void *ctx) {
  (void)ctx;
  assert(user == 123);
  return ++auth_calls == deny_at ? 0 : 1;
}
static const hid_ops_t ops = {op, request, cl, authorize};
static hid_backend_t b = HID_BACKEND_INIT;
static int create(uint32_t i, int32_t *h, int *bind, void *c) {
  (void)i;
  (void)c;
  *bind = 0;
  *h = hid_backend_create(&b, &ops, 123, NULL);
  return *h > 0 ? 0 : -1;
}
static int del(int32_t h, void *c) {
  (void)c;
  return hid_backend_delete(&b, &ops, h, NULL);
}
int main(void) {
  ScePadData p = {0};
  p.quat.w = 1;
  p.leftStick.x = 128;
  p.rightStick.y = 129;
  p.buttons = 0x4000;
  p.analogButtons.r2 = 200;
  uint8_t bytes[HID_REPORT_SIZE];
  assert(!hid_report_encode(&p, bytes));
  assert(bytes[13] == 0x40 && bytes[16] == 128 && bytes[19] == 129 &&
         bytes[21] == 200 && bytes[23] == 1);
  float w;
  memcpy(&w, bytes + 132, 4);
  assert(w == 1);
  p.buttons = 1;
  assert(!hid_report_encode(&p, bytes));
  p.buttons = 0x4000;
  p.buttons = 0x80000000u;
  assert(hid_report_encode(&p, bytes) < 0);
  p.buttons = 0x4000;
  p.quat.w = NAN;
  assert(hid_report_encode(&p, bytes) < 0);
  p.quat.w = 1;
  assert(hid_insert_error_is_transient(EAGAIN));
  assert(hid_insert_error_is_transient(EBUSY));
  assert(hid_insert_error_is_transient(EINTR));
  assert(!hid_insert_error_is_transient(EPERM));
  assert(!hid_insert_error_is_transient(EINVAL));
  assert(!hid_insert_error_is_transient(EIO));
  assert(!hid_insert_error_is_transient(0));
  assert(hid_backend_create(&b, &ops, -1, NULL) < 0 && adds == 0);
  deny_at = auth_calls + 1;
  assert(hid_backend_create(&b, &ops, 123, NULL) < 0 && adds == 0 &&
         b.fd == -1);
  deny_at = auth_calls + 2;
  assert(hid_backend_create(&b, &ops, 123, NULL) < 0 && adds == 0 &&
         b.fd == -1);
  assert(closes == 1 && !b.poisoned);
  closes = 0;
  deny_at = 0;
  fail_open = 1;
  assert(hid_backend_create(&b, &ops, 123, NULL) < 0 && adds == 0);
  fail_open = 0;
  slot_registry_t registry;
  assert(!slot_registry_init(&registry));
  slot_registry_token_t t;
  assert(!slot_registry_claim(&registry, "steam", 0x28de, 0x1304, &t));
  assert(!slot_registry_activate(&registry, t));
  slot_lifecycle_ops_t life = {.create_virtual = create, .delete_virtual = del};
  assert(!slot_lifecycle_attach(&registry, t, &life, NULL, NULL));
  assert(adds == 1);
  assert(hid_backend_create(&b, &ops, 123, NULL) < 0 && adds == 1);
  assert(hid_backend_insert(&b, &ops, 999, &p, NULL) < 0 && inserts == 0);
  assert(!hid_backend_insert(&b, &ops, 321, &p, NULL));
  p.touchData.fingers = 3;
  assert(hid_backend_insert(&b, &ops, 321, &p, NULL) < 0 && errno == EINVAL);
  assert(last[12] == 0 && last[13] == 0 && last[16] == 128 && last[28] == 0);
  float gravity;
  memcpy(&gravity, last + 152, 4);
  assert(gravity == 1.0f);
  p.touchData.fingers = 0;
  p.accel.y = 1;
  assert(!hid_backend_insert(&b, &ops, 321, &p, NULL));
  p.accel.y = 0;
  fail_insert = 1;
  assert(hid_backend_insert(&b, &ops, 321, &p, NULL) < 0);
  fail_insert = 0;
  fail_delete = 1;
  assert(slot_lifecycle_release(&registry, t, del, NULL) != 0);
  assert(b.handle == 321 && closes == 0);
  assert(hid_backend_create(&b, &ops, 123, NULL) < 0 && adds == 1);
  fail_delete = 0;
  assert(!slot_lifecycle_release(&registry, t, del, NULL));
  assert(closes == 1 && b.fd == -1);
  assert(hid_backend_delete(&b, &ops, 321, NULL) < 0);
  slot_registry_destroy(&registry);
  hid_backend_t bad = HID_BACKEND_INIT;
  returned = 0;
  assert(hid_backend_create(&bad, &ops, 123, NULL) < 0 && bad.poisoned);
  int n = adds;
  assert(hid_backend_create(&bad, &ops, 123, NULL) < 0 && adds == n);
  bad = (hid_backend_t)HID_BACKEND_INIT;
  returned = 321;
  fail_add = 1;
  assert(hid_backend_create(&bad, &ops, 123, NULL) < 0 && bad.poisoned);
  fail_add = 0;
  assert(hid_backend_create(&bad, &ops, 123, NULL) < 0);
  assert(closes == 1); /* ambiguous Add never deleted/forgotten */
  power_session_t power = POWER_SESSION_INIT;
  power_session_observe(&power, -1, 500, 1000);
  assert(!power_session_paused(&power));
  power_session_observe(&power, 0, 300, 2000);
  assert(power_session_paused(&power));
  power_session_observe(&power, 0, 1000, 3000);
  assert(power_session_paused(&power));
  power_session_observe(&power, -1, 1000, 4000);
  assert(power_session_paused(&power));
  power_session_observe(&power, 0, 1000, 5000);
  power_session_observe(&power, 0, 1000, 5001);
  assert(power_session_paused(&power));
  power_session_observe(&power, 0, 500, 6000);
  power_session_observe(&power, 0, 1000, 7000);
  power_session_observe(&power, 0, 1000, 8000);
  assert(!power_session_paused(&power));
  power_session_observe(&power, 0, 500, 9000);
  power_session_observe(&power, 0, 42, 10000);
  assert(power_session_paused(&power));
  puts("dev8: HID ABI/encoding/faults/registry cleanup and power consensus "
       "PASS (host only)");
  return 0;
}
