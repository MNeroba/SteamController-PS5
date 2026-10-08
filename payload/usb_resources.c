/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "usb_resources.h"

#include <stddef.h>
#include <string.h>

static int endpoint_bit(uint8_t index, uint32_t *out_bit) {
    if (!out_bit || index >= USB_RESOURCES_MAX_ENDPOINTS)
        return USB_RESOURCES_BAD_INDEX;
    *out_bit = UINT32_C(1) << index;
    return USB_RESOURCES_OK;
}

void usb_resources_init(usb_resources_t *resources) {
    if (!resources) return;
    memset(resources, 0, sizeof(*resources));
    resources->fd = -1;
}

int usb_resources_adopt_fd(usb_resources_t *resources, int fd) {
    if (!resources || fd < 0) return USB_RESOURCES_ERROR;
    if (resources->fd >= 0) return USB_RESOURCES_BAD_STATE;
    resources->fd = fd;
    return USB_RESOURCES_OK;
}

int usb_resources_note_fs_init_attempt(usb_resources_t *resources) {
    if (!resources || resources->fd < 0 || resources->fs_init_attempted)
        return USB_RESOURCES_BAD_STATE;
    resources->fs_init_attempted = 1;
    resources->fs_initialized = 0;
    resources->opened_endpoints = 0;
    resources->active_transfers = 0;
    return USB_RESOURCES_OK;
}

int usb_resources_note_fs_init_result(usb_resources_t *resources, int success) {
    if (!resources || resources->fd < 0 || !resources->fs_init_attempted)
        return USB_RESOURCES_BAD_STATE;
    resources->fs_initialized = success ? 1 : 0;
    return USB_RESOURCES_OK;
}

int usb_resources_note_fs_uninit_result(usb_resources_t *resources, int success) {
    if (!resources || resources->fd < 0 || !resources->fs_initialized)
        return USB_RESOURCES_BAD_STATE;
    if (success) {
        resources->fs_init_attempted = 0;
        resources->fs_initialized = 0;
        resources->opened_endpoints = 0;
        resources->active_transfers = 0;
    }
    return USB_RESOURCES_OK;
}

int usb_resources_note_endpoint_open(usb_resources_t *resources,
                                     uint8_t endpoint_index) {
    uint32_t bit;
    int result = endpoint_bit(endpoint_index, &bit);
    if (result != USB_RESOURCES_OK) return result;
    if (!resources || resources->fd < 0 || !resources->fs_initialized)
        return USB_RESOURCES_BAD_STATE;
    if (resources->opened_endpoints & bit) return USB_RESOURCES_BAD_STATE;
    resources->opened_endpoints |= bit;
    return USB_RESOURCES_OK;
}

int usb_resources_note_endpoint_close(usb_resources_t *resources,
                                      uint8_t endpoint_index) {
    uint32_t bit;
    int result = endpoint_bit(endpoint_index, &bit);
    if (result != USB_RESOURCES_OK) return result;
    if (!resources || resources->fd < 0 || !resources->fs_initialized ||
        !(resources->opened_endpoints & bit))
        return USB_RESOURCES_BAD_STATE;
    resources->opened_endpoints &= ~bit;
    resources->active_transfers &= ~bit;
    return USB_RESOURCES_OK;
}

int usb_resources_note_transfer_start(usb_resources_t *resources,
                                      uint8_t endpoint_index) {
    uint32_t bit;
    int result = endpoint_bit(endpoint_index, &bit);
    if (result != USB_RESOURCES_OK) return result;
    if (!resources || resources->fd < 0 || !resources->fs_initialized ||
        !(resources->opened_endpoints & bit))
        return USB_RESOURCES_BAD_STATE;
    if (resources->active_transfers & bit) return USB_RESOURCES_BAD_STATE;
    resources->active_transfers |= bit;
    return USB_RESOURCES_OK;
}

int usb_resources_note_transfer_complete(usb_resources_t *resources,
                                         uint8_t endpoint_index) {
    uint32_t bit;
    int result = endpoint_bit(endpoint_index, &bit);
    if (result != USB_RESOURCES_OK) return result;
    if (!resources || !(resources->opened_endpoints & bit) ||
        !(resources->active_transfers & bit))
        return USB_RESOURCES_BAD_STATE;
    resources->active_transfers &= ~bit;
    return USB_RESOURCES_OK;
}

int usb_resources_note_transfer_stop(usb_resources_t *resources,
                                     uint8_t endpoint_index) {
    uint32_t bit;
    int result = endpoint_bit(endpoint_index, &bit);
    if (result != USB_RESOURCES_OK) return result;
    if (!resources || !(resources->opened_endpoints & bit) ||
        !(resources->active_transfers & bit))
        return USB_RESOURCES_BAD_STATE;
    resources->active_transfers &= ~bit;
    return USB_RESOURCES_OK;
}

static void remember_error(int *first_error, int result) {
    if (*first_error == 0 && result != 0) *first_error = result;
}

int usb_resources_finish_fs(usb_resources_t *resources,
                            const usb_resources_ops_t *ops,
                            void *context) {
    int first_error = 0;
    if (!resources) return USB_RESOURCES_ERROR;
    if (resources->fd < 0) return USB_RESOURCES_OK;
    if (!ops || !ops->stop_transfer || !ops->close_endpoint ||
        !ops->uninit_fs)
        return USB_RESOURCES_ERROR;

    for (int i = USB_RESOURCES_MAX_ENDPOINTS - 1; i >= 0; i--) {
        uint32_t bit = UINT32_C(1) << i;
        if (!(resources->active_transfers & bit)) continue;
        int result = ops->stop_transfer(resources->fd, (uint8_t)i, context);
        remember_error(&first_error, result);
        if (result == 0) resources->active_transfers &= ~bit;
    }

    for (int i = USB_RESOURCES_MAX_ENDPOINTS - 1; i >= 0; i--) {
        uint32_t bit = UINT32_C(1) << i;
        if (!(resources->opened_endpoints & bit)) continue;
        int result = ops->close_endpoint(resources->fd, (uint8_t)i, context);
        remember_error(&first_error, result);
        if (result == 0) {
            resources->opened_endpoints &= ~bit;
            resources->active_transfers &= ~bit;
        }
    }

    if (resources->fs_initialized) {
        int result = ops->uninit_fs(resources->fd, context);
        remember_error(&first_error, result);
        if (result == 0) {
            resources->fs_init_attempted = 0;
            resources->fs_initialized = 0;
            resources->opened_endpoints = 0;
            resources->active_transfers = 0;
        }
    }
    return first_error;
}

int usb_resources_cleanup(usb_resources_t *resources,
                          const usb_resources_ops_t *ops,
                          void *context) {
    int first_error;
    if (!resources) return USB_RESOURCES_ERROR;
    if (resources->fd < 0) return USB_RESOURCES_OK;
    if (!ops || !ops->close_fd) return USB_RESOURCES_ERROR;

    first_error = usb_resources_finish_fs(resources, ops, context);
    int fd = resources->fd;
    int close_result = ops->close_fd(fd, context);
    remember_error(&first_error, close_result);

    resources->fd = -1;
    resources->fs_init_attempted = 0;
    resources->fs_initialized = 0;
    resources->opened_endpoints = 0;
    resources->active_transfers = 0;
    return first_error;
}
