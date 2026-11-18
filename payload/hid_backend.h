/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once
#include "gc_types.h"
#include <stddef.h>
#include <stdint.h>
#define HID_REPORT_SIZE 168
#define HID_ADD 0xc018482aUL
#define HID_INSERT 0x8018482cUL
#define HID_DELETE 0x80104850UL
typedef struct {
  uint32_t command, reserved;
  void *data;
  void *output;
} hid_request_t;
typedef struct {
  uint8_t command, reserved[7];
  uint32_t handle, padding;
} hid_delete_t;
typedef struct {
  int (*open_device)(void *);
  int (*request)(int, unsigned long, void *, void *);
  int (*close_device)(int, void *);
  /* Read-only, fresh foreground/native-pad policy; 1 permits this exact user.
   */
  int (*authorize_user)(int32_t, void *);
} hid_ops_t;
/* Called behind the application's VDA lock; registry remains slot authority. */
typedef struct {
  int fd;
  int32_t handle;
  int poisoned;
} hid_backend_t;
#define HID_BACKEND_INIT {-1, -1, 0}
int hid_backend_create(hid_backend_t *, const hid_ops_t *, int32_t user_id,
                       void *);
int hid_backend_insert(hid_backend_t *, const hid_ops_t *, int32_t,
                       const ScePadData *, void *);
int hid_backend_delete(hid_backend_t *, const hid_ops_t *, int32_t, void *);
/* Transient insert errors mean the pad is not ready yet; retry next frame
 * instead of tearing down the whole session. */
int hid_insert_error_is_transient(int error);
/* Explicit PS5 raw report translation; see docs/development/RAW_HID_REPORT.md.
 */
int hid_report_encode(const ScePadData *, uint8_t[HID_REPORT_SIZE]);
