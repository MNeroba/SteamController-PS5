/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once
#include "runtime_policy.h"
#include "sc2_status.h"
#include "slot_lifecycle.h"
#include "usb_resources.h"
#include "version.h"
#include <stddef.h>
#include <stdint.h>
#define DIAGNOSTICS_REPORT_MAX 24576
#define DIAGNOSTICS_FILENAME "SteamController-PS5-Diagnostics-" STEAMCONTROLLER_VERSION ".txt"
/* Only application-owned memory. No file reader or caller-supplied paths. */
void diagnostics_runtime(const char *stage, int error, uint32_t actual_firmware,
                         const runtime_policy_t *policy);
void diagnostics_startup(const char *stage, int error);
void diagnostics_webui(int status);
void diagnostics_vda_result(int creation, int result);
void diagnostics_users(const user_session_snapshot_t *users);
void diagnostics_log(const char *line, uint64_t monotonic_ms);
void diagnostics_usb_begin(slot_registry_token_t token, const char *path, uint16_t vid,
                           uint16_t pid);
void diagnostics_usb_stage(slot_registry_token_t token, const char *stage, int error);
void diagnostics_usb_endpoints(slot_registry_token_t token, int interface_number, int in_ep,
                               int out_ep);
void diagnostics_usb_input(slot_registry_token_t token, uint32_t length, uint8_t report_id,
                           uint64_t now_ms, int valid);
void diagnostics_usb_snapshot(slot_registry_token_t token, const usb_resources_t *resources,
                              int watchdog_expired);
void diagnostics_virtual(slot_registry_token_t token, const slot_registry_entry_t *entry,
                         const user_session_snapshot_t *users, const slot_recovery_t *recovery);
/* Additional value-copy fields from existing Steam services, not driver mutation. */
typedef struct {
    sc2_status_snapshot_t steam;
    char profile[48];
    int paused, gyro_wanted, gyro_imu, gyro_active, gyro_calibrated;
    int left_trackpad_mode, right_trackpad_mode, mappings, shift;
} diagnostics_steam_t;
/* Returns bytes or -1. Overflow fails closed; strings escaped/redacted. */
int diagnostics_report(char *output, size_t capacity, const diagnostics_steam_t *steam);
