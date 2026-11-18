/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "hid_backend.h"
#include <errno.h>

#include <string.h>
_Static_assert(sizeof(float) == 4, "HID float ABI");
#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error HID report adapter requires little endian
#endif
_Static_assert(sizeof(void *) == 8, "HID ABI requires 64-bit pointers");
_Static_assert(sizeof(hid_request_t) == 24, "HID request ABI");
_Static_assert(offsetof(hid_request_t, data) == 8, "HID data offset");
_Static_assert(offsetof(hid_request_t, output) == 16, "HID output offset");
_Static_assert(sizeof(hid_delete_t) == 16, "HID delete ABI");
_Static_assert(offsetof(hid_delete_t, handle) == 8, "HID handle offset");
static int finite_float(float v) {
  uint32_t bits;
  memcpy(&bits, &v, 4);
  return (bits & 0x7f800000u) != 0x7f800000u;
}
static int valid(const hid_ops_t *o) {
  return o && o->open_device && o->request && o->close_device &&
         o->authorize_user;
}
int hid_report_encode(const ScePadData *p, uint8_t out[HID_REPORT_SIZE]) {
  if (!p || !out) {
    errno = EINVAL;
    return -1;
  }
  memset(out, 0, HID_REPORT_SIZE);
  if (p->buttons & ~UINT32_C(0x0011ffff)) {
    errno = ENOTSUP;
    return -1;
  }
  if (!finite_float(p->quat.x) || !finite_float(p->quat.y) ||
      !finite_float(p->quat.z) || !finite_float(p->quat.w) ||
      !finite_float(p->vel.x) || !finite_float(p->vel.y) ||
      !finite_float(p->vel.z) || !finite_float(p->accel.x) ||
      !finite_float(p->accel.y) || !finite_float(p->accel.z) ||
      p->touchData.fingers > 2) {
    errno = EINVAL;
    return -1;
  }
  for (unsigned i = 0; i < p->touchData.fingers; i++) {
    const ScePadTouch *t = &p->touchData.touch[i];
    if (t->finger > 127 || t->x >= 1920 || t->y >= 1080 ||
        (i && t->finger == p->touchData.touch[0].finger)) {
      errno = EINVAL;
      return -1;
    }
  }
  memcpy(out + 12, &p->buttons, 4);
  out[16] = p->leftStick.x;
  out[17] = p->leftStick.y;
  out[18] = p->rightStick.x;
  out[19] = p->rightStick.y;
  out[20] = p->analogButtons.l2;
  out[21] = p->analogButtons.r2;
  out[23] = 1;
  /* Dataset 1 selected at +23; unused datasets and released contacts stay zero.
   */
  out[28] = p->touchData.fingers;
  for (unsigned i = 0; i < p->touchData.fingers; i++) {
    const ScePadTouch *t = &p->touchData.touch[i];
    uint8_t *record = out + 32 + 8 * i;
    record[0] = t->finger;
    memcpy(record + 4, &t->x, 2);
    memcpy(record + 6, &t->y, 2);
  }
  memcpy(out + 120, &p->quat, 16);
  memcpy(out + 136, &p->vel, 12); /* rad/s, already converted by steam_motion */
  memcpy(out + 148, &p->accel, 12); /* g */
  return 0;
}
int hid_backend_create(hid_backend_t *b, const hid_ops_t *o, int32_t user_id,
                       void *ctx) {
  if (!b || !valid(o) || user_id <= 0) {
    errno = EINVAL;
    return -1;
  }
  if (b->fd >= 0 || b->poisoned) {
    errno = EBUSY;
    return -1;
  }
  if (o->authorize_user(user_id, ctx) != 1) {
    errno = EAGAIN;
    return -1;
  }
  b->fd = o->open_device(ctx);
  if (b->fd < 0)
    return -1;
  /* Opening may block: recheck immediately before the only creation ioctl. */
  if (o->authorize_user(user_id, ctx) != 1) {
    int fd = b->fd;
    b->fd = -1;
    (void)o->close_device(fd, ctx);
    errno = EAGAIN;
    return -1;
  }
  uint8_t desc[168] = {0};
  int32_t handle = -1;
  desc[0] = 3;
  memcpy(desc + 28, &user_id, 4);
  desc[32] = 1;
  hid_request_t req = {3, 0, desc, &handle};
  errno = 0;
  int r = o->request(b->fd, HID_ADD, &req, ctx);
  if (r || handle <= 0) {
    b->poisoned = 1;
    errno = r ? (errno ? errno : EIO) : EPROTO;
    return -1;
  }
  b->handle = handle;
  return handle;
}
int hid_backend_insert(hid_backend_t *b, const hid_ops_t *o, int32_t handle,
                       const ScePadData *p, void *ctx) {
  if (!b || !valid(o) || b->poisoned || b->fd < 0 || handle <= 0 ||
      b->handle != handle) {
    errno = EPERM;
    return -1;
  }
  uint8_t report[HID_REPORT_SIZE];
  int result = hid_report_encode(p, report), saved = errno;
  if (result) {
    ScePadData neutral = {0};
    neutral.leftStick.x = neutral.leftStick.y = 128;
    neutral.rightStick.x = neutral.rightStick.y = 128;
    neutral.quat.w = 1;
    neutral.accel.y = 1;
    (void)hid_report_encode(&neutral, report);
  }
  hid_request_t req = {(uint32_t)handle, 0, report, NULL};
  int sent = o->request(b->fd, HID_INSERT, &req, ctx);
  if (result) {
    errno = saved;
    return -1;
  }
  return sent;
}
int hid_insert_error_is_transient(int error) {
  return error == EAGAIN || error == EBUSY || error == EINTR;
}
int hid_backend_delete(hid_backend_t *b, const hid_ops_t *o, int32_t handle,
                       void *ctx) {
  if (!b || !valid(o) || b->poisoned || b->fd < 0 || handle <= 0 ||
      b->handle != handle) {
    errno = EPERM;
    return -1;
  }
  ScePadData neutral = {0};
  neutral.leftStick.x = neutral.leftStick.y = 128;
  neutral.rightStick.x = neutral.rightStick.y = 128;
  neutral.quat.w = 1;
  neutral.accel.y = 1;
  (void)hid_backend_insert(b, o, handle, &neutral, ctx);
  hid_delete_t req = {0};
  req.command = 5;
  req.handle = (uint32_t)handle;
  int r = o->request(b->fd, HID_DELETE, &req, ctx);
  if (r)
    return r;
  int fd = b->fd;
  b->fd = -1;
  b->handle = -1;
  /* Successful deletion retires ownership even if close reports failure. No fd
   * retry. */
  (void)o->close_device(fd, ctx);
  return 0;
}
