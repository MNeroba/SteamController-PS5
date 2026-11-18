/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "hid_backend.h"
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
static void u32(uint8_t *p, uint32_t v) {
  for (int i = 0; i < 4; i++)
    p[i] = (uint8_t)(v >> (8 * i));
}
int main(void) {
  assert(HID_REPORT_SIZE == 168);
  assert(SCE_PAD_BUTTON_CREATE == 1 && SCE_PAD_BUTTON_SHARE == 1 &&
         SCE_PAD_BUTTON_PS == 0x10000);
  assert(offsetof(ScePadData, accel) == 0x1c &&
         offsetof(ScePadData, vel) == 0x28);
  assert(offsetof(ScePadData, touchData) == 0x34 &&
         offsetof(ScePadData, connected) == 0x4c);
  ScePadData p = {0};
  p.buttons = SCE_PAD_BUTTON_SHARE | SCE_PAD_BUTTON_PS |
              SCE_PAD_BUTTON_TOUCH_PAD | SCE_PAD_BUTTON_CROSS;
  p.leftStick.x = 128;
  p.leftStick.y = 0;
  p.rightStick.x = 255;
  p.rightStick.y = 64;
  p.analogButtons.l2 = 12;
  p.analogButtons.r2 = 234;
  p.quat.w = 1;
  p.vel.x = 1;
  p.vel.y = 2;
  p.vel.z = 3;
  p.accel.x = 0.5f;
  p.accel.y = 1;
  p.accel.z = -0.25f;
  p.touchData.fingers = 2;
  p.touchData.touch[0] = (ScePadTouch){960, 540, 0x11, {0}};
  p.touchData.touch[1] = (ScePadTouch){100, 1079, 0x12, {0}};
  uint8_t actual[HID_REPORT_SIZE], golden[168] = {0};
  u32(golden + 12, 0x114001);
  golden[16] = 128;
  golden[18] = 255;
  golden[19] = 64;
  golden[20] = 12;
  golden[21] = 234;
  golden[23] = 1;
  golden[28] = 2;
  golden[32] = 0x11;
  golden[36] = 0xc0;
  golden[37] = 3;
  golden[38] = 0x1c;
  golden[39] = 2;
  golden[40] = 0x12;
  golden[44] = 100;
  golden[46] = 0x37;
  golden[47] = 4;
  u32(golden + 132, 0x3f800000);
  u32(golden + 136, 0x3f800000);
  u32(golden + 140, 0x40000000);
  u32(golden + 144, 0x40400000);
  u32(golden + 148, 0x3f000000);
  u32(golden + 152, 0x3f800000);
  u32(golden + 156, 0xbe800000);
  assert(!hid_report_encode(&p, actual));
  assert(!memcmp(actual, golden, 168));
  p.touchData.fingers = 1;
  p.buttons = 0;
  assert(!hid_report_encode(&p, actual));
  assert(actual[28] == 1 && actual[32] == 0x11);
  for (int i = 40; i < 48; i++)
    assert(actual[i] == 0);
  p.touchData.fingers = 0;
  assert(!hid_report_encode(&p, actual));
  for (int i = 28; i < 120; i++)
    assert(actual[i] == 0);
  p.touchData.fingers = 3;
  assert(hid_report_encode(&p, actual) < 0 && errno == EINVAL);
  p.touchData.fingers = 2;
  p.touchData.touch[1].finger = 0x11;
  assert(hid_report_encode(&p, actual) < 0);
  p.touchData.touch[1].finger = 0x12;
  p.touchData.touch[0].x = 1920;
  assert(hid_report_encode(&p, actual) < 0);
  p.touchData.touch[0].x = 1919;
  p.touchData.touch[0].y = 1080;
  assert(hid_report_encode(&p, actual) < 0);
  p.touchData.touch[0].y = 1079;
  p.touchData.touch[0].finger = 128;
  assert(hid_report_encode(&p, actual) < 0);
  p.touchData.touch[0].finger = 127;
  assert(!hid_report_encode(&p, actual));
  p.vel.y = NAN;
  assert(hid_report_encode(&p, actual) < 0);
  p.vel.y = 2;
  p.accel.x = INFINITY;
  assert(hid_report_encode(&p, actual) < 0);
  p.accel.x = 0.5;
  p.quat.w = -1;
  assert(!hid_report_encode(
      &p, actual)); /* equivalent quaternion sign is preserved */
  float *fields[] = {&p.quat.x, &p.quat.y, &p.quat.z,  &p.quat.w,  &p.vel.x,
                     &p.vel.y,  &p.vel.z,  &p.accel.x, &p.accel.y, &p.accel.z};
  for (unsigned i = 0; i < sizeof(fields) / sizeof(fields[0]); i++) {
    float saved = *fields[i];
    *fields[i] = NAN;
    assert(hid_report_encode(&p, actual) < 0 && errno == EINVAL);
    *fields[i] = -INFINITY;
    assert(hid_report_encode(&p, actual) < 0 && errno == EINVAL);
    *fields[i] = saved;
  }
  assert(hid_report_encode(NULL, actual) < 0 && errno == EINVAL);
  assert(hid_report_encode(&p, NULL) < 0 && errno == EINVAL);
  puts("extended HID: full independent golden frame, native ABI, touch "
       "release/bounds/IDs, motion units and finite validation PASS");
}
