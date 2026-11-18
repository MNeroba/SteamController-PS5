# Dev.8 raw HID report contract

Reviewed 2026-11-03. Evidence: VERIFIED_CODE (pinned public implementations), VERIFIED_HOST_TEST (synthetic golden frame and fault injection). Our PS5 execution is NOT_RUN; none of these sources proves compatibility with our target FW13.60.

## Sources and confidence

- [ProsperoSunshine pad.c, lines1–155](https://github.com/Darkatek7/ProsperoSunshine/blob/3574d60989efe7d0b2410e2ff137cd1c4d490c60/src/platform/ps5/pad.c#L1): complete descriptor, touch and motion writer. [Public ABI investigation](https://github.com/Darkatek7/ProsperoSunshine/blob/3574d60989efe7d0b2410e2ff137cd1c4d490c60/docs/re/pad-input.md) describes insertion/read-side agreement. SPDX GPL-3.0-only; no Sony binary, proprietary header or disassembly was acquired or redistributed here.
- [ProsperoSunshine STATUS, lines204–214](https://github.com/Darkatek7/ProsperoSunshine/blob/3574d60989efe7d0b2410e2ff137cd1c4d490c60/STATUS.md#L204): UPSTREAM_CLAIM of FW13.00 title-side readback, two independent contacts, release, angular velocity in rad/s and acceleration in g. Lines78/128 describe Create/PS behavior. Real game consumption, axis signs and non-neutral quaternion response remain unproven for this project.
- [ps5upload vpad.c](https://github.com/phantomptr/ps5upload/blob/efcd6fd4a2ce416b226c9a28ef16464ad7fbcb73/payload/src/vpad.c) and [self-tests](https://github.com/phantomptr/ps5upload/blob/efcd6fd4a2ce416b226c9a28ef16464ad7fbcb73/payload/tests/vpad_selftest.c): same full168-byte layout; file SPDX GPL-3.0-only. Its credits identify the same lineage: corroborating implementation, not independent hardware evidence.
- [OpenOrbis public pad.h](https://github.com/OpenOrbis/OpenOrbis-PS4-Toolchain/blob/0a1aaf9dd4a92695538bdeb09fb056d06dd11725/include/orbis/_types/pad.h): PS4 native read-structure order agrees with the PS5 source: quaternion, acceleration, angular velocity, touch. PS4 agreement alone is not PS5 ABI proof.
- The older FGG/AnyController160-byte descriptions and ps5-mcp84c0f7b316772b13cd62e0121c154afc3ed2482c partial description omit extended fields. SunshinePS55bb544695d88342d8dfde92e823865ffc66635dd explicitly leaves them provisional. Those negative findings remain in the earlier comparison.

This adapter and regression are independently authored from interface facts; no upstream parser/server/controller implementation was copied. See ../PROVENANCE.md for lineage and licensing.

## Wire layout (little endian)

Add ioctl0xC018482A uses a24-byte header: u32 class3, u32zero, descriptor pointer, output signed-handle pointer. Its168-byte descriptor has class3 at0, **actual confirmed account ID** u32 at0x1c and subtype1 at0x20. Never substitute1, an initial account, a filesystem directory or an unverified foreground account. A successful syscall plus positive output handle establishes our receipt; failed/ambiguous Add retains unresolved ownership and prohibits retries.

Insert ioctl0x8018482C uses a24-byte header: positive owned handle, zero, report pointer, eight zero bytes. The report is **168 bytes**, distinct from native ScePadData120 bytes:

| Offset | Field | Encoding |
| --- | --- | --- |
| 0x00–0x0b | Header | Zero; do not synthesize kernel timestamps |
| 0x0c | Buttons | u32; Create/Share bit0, PS bit16, touchpad click bit20 |
| 0x10–0x13 | Sticks | lx,ly,rx,ry u8 |
| 0x14–0x15 | Triggers | l2,r2 u8 |
| 0x17 | Touch dataset selector | 1 (not a connection flag) |
| 0x1c | Contact count | u8,0..2 |
| 0x20,0x28 | Contact records | 8 bytes: id u8,three zero bytes,x LEu16,y LEu16 |
| 0x30–0x77 | Other touch datasets | Zero |
| 0x78–0x87 | Orientation | float32 x,y,z,w |
| 0x88–0x93 | Angular velocity | float32 x,y,z; rad/s |
| 0x94–0x9f | Acceleration | float32 x,y,z; g |
| 0xa0–0xa7 | Tail | Zero |

Unused bytes are zero. Contacts retain canonical IDs0..127, distinct while active; x0..1919,y0..1079. The standard1920-wide extent is an upstream device assumption corroborated by midpoint readback, not a measured resolution query on our console. Lift reduces count and zeros removed records. Pressure has no established PS5 field and remains canonical Steam state only. No arbitrary raw memcpy of ScePadData into this report.

All10 floats must be finite; unknown button bits, invalid contacts/count/IDs/coordinates fail before submission. Quaternion sign is preserved. Existing steam_motion supplies PS5-axis rad/s and g; this adapter does not rescale them again. Neutral is centered sticks, released controls/contacts, identity quaternion, zero gyro and gravity(0,1,0). Actual zero acceleration is preserved when supplied, including freefall. On invalid input the backend attempts neutral and runtime requests coordinated owned shutdown.

Native ScePadData uses acceleration at0x1c and angular velocity at0x28, opposite the prior local declaration. Static assertions enforce these offsets, touch0x34 and connected0x4c; named semantic access remains unchanged.

## Account and native-controller safety

ProsperoSunshine STATUS's known-limits section reports native-controller removal when a virtual pad was registered to the same account, even without explicit host disconnect. Therefore its account-directory scan/automatic registration is **not adopted**.

Direct HID remains opt-in, incompatible with handoff/hooks/bridge. Before registry reservation, require one valid logged-in user, matching foreground and a successful read of a disconnected native pad. Missing handle, API error, connected DualSense, malformed/multiple users or login screen defer with no Add. Check again before opening and immediately before the creation ioctl. Recovery may delete our old receipt, but must defer re-creation under the same policy. User IDs never enter logs. No native Open/disconnect/unbind, Shell patch or fallback backend.

An account/native-state change between the last read and ioctl cannot be made atomic by these APIs. A changed check after reservation fails conservatively and can inhibit creation until restart; it never guesses deletion or retries an ambiguous Add. If the platform cannot provide a readable disconnected native handle, this strict mode remains waiting. This is a documented safe limitation, not grounds to weaken native protection. Physical controlled assignment/coexistence tests remain mandatory before broader enabling.

## Verification

`tests/test_hid_report_full.c` first failed on report-size160, before correction. It now checks an independently constructed complete168-byte golden frame, native layout, Create/PS separation, contact lift/bounds/IDs, all finite-value failure paths and nulls. `test_dev8_backend` covers exact account descriptor, authorization denial before/after open, neutral gravity, wrong handles, failed Add/Insert/Delete, retained ownership and power consensus. Existing user/registry/recovery/USB suites remain authoritative. See project-memory/TEST_LOG.md for actual command results. Synthetic tests are not captured PS5 packets or hardware tests.
