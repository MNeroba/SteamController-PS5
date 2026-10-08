/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "usb_session.h"

int usb_session_open(usb_resources_t *resources,
                     const usb_session_ops_t *ops,
                     const char *path, int flags, void *context) {
    if (!resources || !ops || !ops->open_device || !ops->resources.close_fd ||
        !path)
        return USB_SESSION_ERROR;
    if (resources->fd >= 0) return USB_SESSION_BAD_STATE;

    int fd = ops->open_device(path, flags, context);
    if (fd < 0) return fd;
    int result = usb_resources_adopt_fd(resources, fd);
    if (result == USB_RESOURCES_OK) return USB_SESSION_OK;

    /* The descriptor was opened but could not be adopted; relinquish it once. */
    (void)ops->resources.close_fd(fd, context);
    return USB_SESSION_BAD_STATE;
}

int usb_session_init_fs(usb_resources_t *resources,
                        const usb_session_ops_t *ops,
                        void *configuration, void *context) {
    if (!resources || !ops || !ops->init_fs)
        return USB_SESSION_ERROR;
    int result = usb_resources_note_fs_init_attempt(resources);
    if (result != USB_RESOURCES_OK) return USB_SESSION_BAD_STATE;

    int operation_result = ops->init_fs(resources->fd, configuration, context);
    result = usb_resources_note_fs_init_result(resources, operation_result == 0);
    if (result != USB_RESOURCES_OK) return USB_SESSION_BAD_STATE;
    return operation_result;
}

int usb_session_uninit_fs(usb_resources_t *resources,
                          const usb_session_ops_t *ops,
                          void *context) {
    if (!resources || !ops || !ops->resources.uninit_fs)
        return USB_SESSION_ERROR;
    if (resources->fd < 0 || !resources->fs_initialized)
        return USB_SESSION_BAD_STATE;

    int result = ops->resources.uninit_fs(resources->fd, context);
    int note_result = usb_resources_note_fs_uninit_result(resources, result == 0);
    if (note_result != USB_RESOURCES_OK) return USB_SESSION_BAD_STATE;
    return result;
}

int usb_session_open_endpoint(usb_resources_t *resources,
                              const usb_session_ops_t *ops,
                              uint8_t endpoint_index, void *configuration,
                              void *context) {
    if (!resources || !ops || !ops->open_endpoint ||
        !ops->resources.close_endpoint)
        return USB_SESSION_ERROR;
    if (resources->fd < 0 || !resources->fs_initialized ||
        endpoint_index >= USB_RESOURCES_MAX_ENDPOINTS ||
        (resources->opened_endpoints & (UINT32_C(1) << endpoint_index)))
        return USB_SESSION_BAD_STATE;
    int result = ops->open_endpoint(resources->fd, configuration, context);
    if (result != 0) return result;

    result = usb_resources_note_endpoint_open(resources, endpoint_index);
    if (result == USB_RESOURCES_OK) return USB_SESSION_OK;
    (void)ops->resources.close_endpoint(resources->fd, endpoint_index, context);
    return USB_SESSION_BAD_STATE;
}

int usb_session_start_transfer(usb_resources_t *resources,
                               const usb_session_ops_t *ops,
                               uint8_t endpoint_index, void *configuration,
                               void *context) {
    if (!resources || !ops || !ops->start_transfer)
        return USB_SESSION_ERROR;
    /* Treat a failed START as possibly issued; cleanup will attempt STOP. */
    int result = usb_resources_note_transfer_start(resources, endpoint_index);
    if (result != USB_RESOURCES_OK) return USB_SESSION_BAD_STATE;
    return ops->start_transfer(resources->fd, configuration, context);
}

int usb_session_complete_transfer(usb_resources_t *resources,
                                  const usb_session_ops_t *ops,
                                  uint8_t endpoint_index, void *configuration,
                                  void *context) {
    if (!resources || !ops || !ops->complete_transfer)
        return USB_SESSION_ERROR;
    if (resources->fd < 0 || endpoint_index >= USB_RESOURCES_MAX_ENDPOINTS ||
        !(resources->opened_endpoints & (UINT32_C(1) << endpoint_index)) ||
        !(resources->active_transfers & (UINT32_C(1) << endpoint_index)))
        return USB_SESSION_BAD_STATE;
    int result = ops->complete_transfer(resources->fd, configuration, context);
    if (result != 0) return result;

    result = usb_resources_note_transfer_complete(resources, endpoint_index);
    return result == USB_RESOURCES_OK ? USB_SESSION_OK : USB_SESSION_BAD_STATE;
}

int usb_session_stop_transfer(usb_resources_t *resources,
                              const usb_session_ops_t *ops,
                              uint8_t endpoint_index, void *context) {
    if (!resources || !ops || !ops->resources.stop_transfer)
        return USB_SESSION_ERROR;
    if (resources->fd < 0 || endpoint_index >= USB_RESOURCES_MAX_ENDPOINTS ||
        !(resources->opened_endpoints & (UINT32_C(1) << endpoint_index)) ||
        !(resources->active_transfers & (UINT32_C(1) << endpoint_index)))
        return USB_SESSION_BAD_STATE;
    int result = ops->resources.stop_transfer(resources->fd, endpoint_index,
                                              context);
    if (result != 0) return result;

    result = usb_resources_note_transfer_stop(resources, endpoint_index);
    return result == USB_RESOURCES_OK ? USB_SESSION_OK : USB_SESSION_BAD_STATE;
}

int usb_session_cleanup(usb_resources_t *resources,
                        const usb_session_ops_t *ops,
                        void *context) {
    if (!resources || !ops) return USB_SESSION_ERROR;
    return usb_resources_cleanup(resources, &ops->resources, context);
}
