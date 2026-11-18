/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "usb_session.h"

#include <errno.h>

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

int usb_session_close_endpoint(usb_resources_t *resources,
                               const usb_session_ops_t *ops,
                               uint8_t endpoint_index, void *context) {
    if (!resources || !ops || !ops->resources.close_endpoint)
        return USB_SESSION_ERROR;
    if (resources->fd < 0 || endpoint_index >= USB_RESOURCES_MAX_ENDPOINTS ||
        !(resources->opened_endpoints & (UINT32_C(1) << endpoint_index)))
        return USB_SESSION_BAD_STATE;
    int result = ops->resources.close_endpoint(resources->fd, endpoint_index,
                                               context);
    if (result != 0) return result;
    result = usb_resources_note_endpoint_close(resources, endpoint_index);
    return result == USB_RESOURCES_OK ? USB_SESSION_OK : USB_SESSION_BAD_STATE;
}

int usb_session_start_transfer(usb_resources_t *resources,
                               const usb_session_ops_t *ops,
                               uint8_t endpoint_index, void *configuration,
                               void *context) {
    if (!resources || !ops || !ops->start_transfer)
        return USB_SESSION_ERROR;
    /* Treat a failed START as possibly issued; cleanup will attempt STOP. */
    int result = usb_resources_note_transfer_start(resources, endpoint_index);
    if (result != USB_RESOURCES_OK) {
        errno = EBUSY;
        return USB_SESSION_BAD_STATE;
    }
    resources->starts++;
    int operation = ops->start_transfer(resources->fd, configuration, context);
    if (operation) usb_session_note_error(resources, errno ? errno : EIO);
    return operation;
}

int usb_session_complete_any_transfer(usb_resources_t *resources,
                                      const usb_session_ops_t *ops,
                                      void *configuration,
                                      uint8_t *completed_endpoint_index,
                                      void *context) {
    if (!resources || !ops || !ops->complete_transfer ||
        !completed_endpoint_index)
        return USB_SESSION_ERROR;
    if (resources->fd < 0 || (resources->active_transfers | resources->retired_transfers) == 0)
        return USB_SESSION_BAD_STATE;
    uint8_t completed = UINT8_MAX;
    int result = ops->complete_transfer(resources->fd, configuration,
                                        &completed, context);
    if (result != 0) { usb_session_note_error(resources, errno ? errno : EIO); return result; }

    if (completed < USB_RESOURCES_MAX_ENDPOINTS && (resources->retired_transfers & (UINT32_C(1) << completed))) {
        resources->late_completions++;
        *completed_endpoint_index = completed;
        return USB_SESSION_CANCELLED_COMPLETION;
    }
    if (completed >= USB_RESOURCES_MAX_ENDPOINTS ||
        usb_resources_note_transfer_complete(resources, completed) !=
            USB_RESOURCES_OK) {
        errno = EPROTO;
        usb_session_note_error(resources, EPROTO);
        return USB_SESSION_BAD_STATE;
    }
    resources->completions++;
    *completed_endpoint_index = completed;
    return USB_SESSION_OK;
}

int usb_session_complete_transfer(usb_resources_t *resources,
                                  const usb_session_ops_t *ops,
                                  uint8_t endpoint_index, void *configuration,
                                  uint8_t *completed_endpoint_index,
                                  void *context) {
    if (!resources || !ops || !completed_endpoint_index)
        return USB_SESSION_ERROR;
    if (resources->fd < 0 || endpoint_index >= USB_RESOURCES_MAX_ENDPOINTS ||
        !(resources->opened_endpoints & (UINT32_C(1) << endpoint_index))) return USB_SESSION_BAD_STATE;
    if (resources->completed_transfers & (UINT32_C(1) << endpoint_index)) {
        (void)usb_resources_take_completion(resources, endpoint_index);
        *completed_endpoint_index=endpoint_index;return USB_SESSION_OK;
    }
    if (!(resources->active_transfers & (UINT32_C(1) << endpoint_index))) return USB_SESSION_BAD_STATE;
    uint8_t completed = UINT8_MAX;
    int result = usb_session_complete_any_transfer(resources, ops,
                                                    configuration, &completed,
                                                    context);
    if (result != USB_SESSION_OK) {
        if (result == USB_SESSION_CANCELLED_COMPLETION) *completed_endpoint_index = completed;
        return result;
    }
    *completed_endpoint_index = completed;
    if (completed == endpoint_index) {
        (void)usb_resources_take_completion(resources, completed);
        return USB_SESSION_OK;
    }
    return USB_SESSION_COMPLETED_OTHER;
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

void usb_input_watchdog_init(usb_input_watchdog_t *w, uint64_t now_ms) {
    if (w) w->last_input_ms = now_ms;
}
void usb_input_watchdog_refresh(usb_input_watchdog_t *w, uint64_t now_ms) {
    if (w) w->last_input_ms = now_ms;
}
int usb_input_watchdog_expired(const usb_input_watchdog_t *w,
                               uint64_t now_ms, unsigned timeout_ms) {
    return !w || !timeout_ms || now_ms < w->last_input_ms ||
        now_ms - w->last_input_ms >= timeout_ms;
}

const char *usb_session_error_name(int error) {
    if(error==EBUSY || error==EAGAIN || error==EWOULDBLOCK)return "pending";
    if(error==ETIMEDOUT)return "timeout";
    if(error==EPROTO || error==EMSGSIZE || error==EINVAL)return "protocol error";
    return "USB failure";
}
void usb_session_note_error(usb_resources_t *r,int e) {
    if(!r || !e || e==EBUSY || e==EAGAIN || e==EWOULDBLOCK)return;
    if(e==ETIMEDOUT)r->timeouts++;
    else if(e==EPROTO || e==EMSGSIZE || e==EINVAL)r->protocol_errors++;
    else r->failures++;
}
