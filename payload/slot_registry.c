/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "slot_registry.h"

#include <stddef.h>
#include <string.h>

static size_t identity_length(const char *identity) {
    size_t length = 0;
    if (!identity) return SLOT_REGISTRY_ID_MAX;
    while (length < SLOT_REGISTRY_ID_MAX && identity[length] != '\0')
        length++;
    return length;
}

static slot_registry_entry_t *find_token_locked(slot_registry_t *registry,
                                                slot_registry_token_t token) {
    slot_registry_entry_t *entry;
    if (token.generation == 0 || token.index >= SLOT_REGISTRY_MAX_SLOTS)
        return NULL;
    entry = &registry->entries[token.index];
    if (entry->state == SLOT_REGISTRY_FREE ||
        entry->token.generation != token.generation)
        return NULL;
    return entry;
}

int slot_registry_init(slot_registry_t *registry) {
    if (!registry) return SLOT_REGISTRY_ERROR;
    memset(registry, 0, sizeof(*registry));
    if (pthread_mutex_init(&registry->mutex, NULL) != 0)
        return SLOT_REGISTRY_ERROR;
    registry->initialized = 1;
    return SLOT_REGISTRY_OK;
}

void slot_registry_destroy(slot_registry_t *registry) {
    if (!registry || !registry->initialized) return;
    pthread_mutex_destroy(&registry->mutex);
    registry->initialized = 0;
}

int slot_registry_claim(slot_registry_t *registry, const char *identity,
                        uint16_t vendor_id, uint16_t product_id,
                        slot_registry_token_t *out_token) {
    size_t length = identity_length(identity);
    int free_index = -1;
    int result = SLOT_REGISTRY_OK;

    if (!registry || !registry->initialized || !out_token ||
        length == 0 || length >= SLOT_REGISTRY_ID_MAX)
        return SLOT_REGISTRY_ERROR;

    pthread_mutex_lock(&registry->mutex);
    for (uint32_t i = 0; i < SLOT_REGISTRY_MAX_SLOTS; i++) {
        slot_registry_entry_t *entry = &registry->entries[i];
        if (entry->state == SLOT_REGISTRY_FREE) {
            if (free_index < 0) free_index = (int)i;
        } else if (strcmp(entry->identity, identity) == 0) {
            result = SLOT_REGISTRY_DUPLICATE;
            goto done;
        }
    }

    if (free_index < 0) {
        result = SLOT_REGISTRY_FULL;
        goto done;
    }

    uint32_t generation = ++registry->next_generation[free_index];
    if (generation == 0) generation = ++registry->next_generation[free_index];

    slot_registry_entry_t *entry = &registry->entries[free_index];
    memset(entry, 0, sizeof(*entry));
    entry->state = SLOT_REGISTRY_RESERVED;
    entry->token.index = (uint32_t)free_index;
    entry->token.generation = generation;
    memcpy(entry->identity, identity, length);
    entry->vendor_id = vendor_id;
    entry->product_id = product_id;
    entry->virtual_handle = -1;
    *out_token = entry->token;

done:
    pthread_mutex_unlock(&registry->mutex);
    return result;
}

int slot_registry_find_identity(slot_registry_t *registry,
                                const char *identity, uint32_t *out_index) {
    size_t length = identity_length(identity);
    if (!registry || !registry->initialized || !out_index ||
        length == 0 || length >= SLOT_REGISTRY_ID_MAX)
        return SLOT_REGISTRY_ERROR;

    pthread_mutex_lock(&registry->mutex);
    for (uint32_t i = 0; i < SLOT_REGISTRY_MAX_SLOTS; i++) {
        slot_registry_entry_t *entry = &registry->entries[i];
        if (entry->state != SLOT_REGISTRY_FREE &&
            strcmp(entry->identity, identity) == 0) {
            *out_index = i;
            pthread_mutex_unlock(&registry->mutex);
            return SLOT_REGISTRY_OK;
        }
    }
    pthread_mutex_unlock(&registry->mutex);
    return SLOT_REGISTRY_ERROR;
}

int slot_registry_activate(slot_registry_t *registry,
                           slot_registry_token_t token) {
    int result = SLOT_REGISTRY_OK;
    if (!registry || !registry->initialized) return SLOT_REGISTRY_ERROR;

    pthread_mutex_lock(&registry->mutex);
    slot_registry_entry_t *entry = find_token_locked(registry, token);
    if (!entry) result = SLOT_REGISTRY_STALE;
    else if (entry->state != SLOT_REGISTRY_RESERVED)
        result = SLOT_REGISTRY_BAD_STATE;
    else entry->state = SLOT_REGISTRY_ACTIVE;
    pthread_mutex_unlock(&registry->mutex);
    return result;
}

int slot_registry_snapshot(slot_registry_t *registry,
                           slot_registry_token_t token,
                           slot_registry_entry_t *out_entry) {
    int result = SLOT_REGISTRY_OK;
    if (!registry || !registry->initialized || !out_entry)
        return SLOT_REGISTRY_ERROR;

    pthread_mutex_lock(&registry->mutex);
    slot_registry_entry_t *entry = find_token_locked(registry, token);
    if (!entry) result = SLOT_REGISTRY_STALE;
    else *out_entry = *entry;
    pthread_mutex_unlock(&registry->mutex);
    return result;
}

int slot_registry_snapshot_index(slot_registry_t *registry, uint32_t index,
                                 slot_registry_entry_t *out_entry) {
    if (!registry || !registry->initialized || !out_entry ||
        index >= SLOT_REGISTRY_MAX_SLOTS)
        return SLOT_REGISTRY_ERROR;

    pthread_mutex_lock(&registry->mutex);
    slot_registry_entry_t *entry = &registry->entries[index];
    int result = entry->state == SLOT_REGISTRY_FREE ? SLOT_REGISTRY_STALE :
                 SLOT_REGISTRY_OK;
    if (result == SLOT_REGISTRY_OK) *out_entry = *entry;
    pthread_mutex_unlock(&registry->mutex);
    return result;
}

int slot_registry_request_release(slot_registry_t *registry,
                                  slot_registry_token_t token) {
    int result = SLOT_REGISTRY_OK;
    if (!registry || !registry->initialized) return SLOT_REGISTRY_ERROR;

    pthread_mutex_lock(&registry->mutex);
    slot_registry_entry_t *entry = find_token_locked(registry, token);
    if (!entry) result = SLOT_REGISTRY_STALE;
    else entry->state = SLOT_REGISTRY_RELEASING;
    pthread_mutex_unlock(&registry->mutex);
    return result;
}

int slot_registry_set_virtual(slot_registry_t *registry,
                              slot_registry_token_t token,
                              int32_t virtual_handle) {
    int result = SLOT_REGISTRY_OK;
    if (!registry || !registry->initialized || virtual_handle < 0)
        return SLOT_REGISTRY_ERROR;

    pthread_mutex_lock(&registry->mutex);
    slot_registry_entry_t *entry = find_token_locked(registry, token);
    if (!entry) result = SLOT_REGISTRY_STALE;
    else if (entry->state == SLOT_REGISTRY_RELEASING)
        result = SLOT_REGISTRY_BAD_STATE;
    else if (entry->virtual_ready || entry->virtual_deleting || entry->virtual_creating || entry->virtual_blocked)
        result = SLOT_REGISTRY_BAD_STATE;
    else {
        for (uint32_t i = 0; i < SLOT_REGISTRY_MAX_SLOTS; i++) {
            slot_registry_entry_t *other = &registry->entries[i];
            if (other != entry && other->state != SLOT_REGISTRY_FREE &&
                (other->virtual_ready || other->virtual_deleting) &&
                other->virtual_handle == virtual_handle) {
                result = SLOT_REGISTRY_HANDLE_IN_USE;
                break;
            }
        }
    }
    if (result == SLOT_REGISTRY_OK) {
        entry->virtual_handle = virtual_handle;
        entry->virtual_ready = 1;
        entry->confirmed = 0;
    }
    pthread_mutex_unlock(&registry->mutex);
    return result;
}

int slot_registry_take_virtual(slot_registry_t *registry,
                               slot_registry_token_t token,
                               int32_t *out_handle) {
    int result = SLOT_REGISTRY_OK;
    if (!registry || !registry->initialized || !out_handle)
        return SLOT_REGISTRY_ERROR;

    pthread_mutex_lock(&registry->mutex);
    slot_registry_entry_t *entry = find_token_locked(registry, token);
    if (!entry) result = SLOT_REGISTRY_STALE;
    else if (entry->state == SLOT_REGISTRY_RELEASING || entry->virtual_deleting || entry->virtual_creating)
        result = SLOT_REGISTRY_BUSY;
    else {
        *out_handle = entry->virtual_ready ? entry->virtual_handle : -1;
        entry->virtual_ready = 0;
        entry->virtual_handle = -1;
        entry->confirmed = 0;
    }
    pthread_mutex_unlock(&registry->mutex);
    return result;
}

int slot_registry_delete_virtual(slot_registry_t *registry,
                                 slot_registry_token_t token,
                                 slot_registry_delete_fn delete_fn,
                                 void *context) {
    int32_t handle = -1;
    int result = SLOT_REGISTRY_OK;
    if (!registry || !registry->initialized || !delete_fn)
        return SLOT_REGISTRY_ERROR;

    pthread_mutex_lock(&registry->mutex);
    slot_registry_entry_t *entry = find_token_locked(registry, token);
    if (!entry) result = SLOT_REGISTRY_STALE;
    else if (entry->virtual_deleting || entry->virtual_creating) result = SLOT_REGISTRY_BUSY;
    else if (entry->virtual_ready) {
        handle = entry->virtual_handle;
        entry->virtual_ready = 0;
        entry->virtual_deleting = 1;
        entry->confirmed = 0;
    } else {
        entry->confirmed = 0;
    }
    pthread_mutex_unlock(&registry->mutex);
    if (result != SLOT_REGISTRY_OK || handle < 0) return result;

    int delete_result = delete_fn(handle, context);

    pthread_mutex_lock(&registry->mutex);
    entry = find_token_locked(registry, token);
    if (!entry) result = SLOT_REGISTRY_STALE;
    else if (!entry->virtual_deleting) result = SLOT_REGISTRY_BAD_STATE;
    else {
        entry->virtual_deleting = 0;
        if (delete_result != 0) {
            entry->virtual_ready = 1;
            result = SLOT_REGISTRY_DELETE_FAILED;
        } else {
            entry->virtual_handle = -1;
        }
    }
    pthread_mutex_unlock(&registry->mutex);
    return result;
}

int slot_registry_mark_confirmed(slot_registry_t *registry,
                                 slot_registry_token_t token) {
    int result = SLOT_REGISTRY_OK;
    if (!registry || !registry->initialized) return SLOT_REGISTRY_ERROR;

    pthread_mutex_lock(&registry->mutex);
    slot_registry_entry_t *entry = find_token_locked(registry, token);
    if (!entry) result = SLOT_REGISTRY_STALE;
    else if (entry->state == SLOT_REGISTRY_RELEASING || entry->virtual_deleting || entry->virtual_creating)
        result = SLOT_REGISTRY_BUSY;
    else if (entry->confirmed) result = 0;
    else {
        entry->confirmed = 1;
        result = 1;
    }
    pthread_mutex_unlock(&registry->mutex);
    return result;
}

int slot_registry_release(slot_registry_t *registry,
                          slot_registry_token_t token) {
    int result = SLOT_REGISTRY_OK;
    if (!registry || !registry->initialized) return SLOT_REGISTRY_ERROR;

    pthread_mutex_lock(&registry->mutex);
    slot_registry_entry_t *entry = find_token_locked(registry, token);
    if (!entry) result = SLOT_REGISTRY_STALE;
    else if (entry->virtual_ready || entry->virtual_deleting || entry->virtual_creating)
        result = SLOT_REGISTRY_BUSY;
    else if (entry->state != SLOT_REGISTRY_RELEASING)
        result = SLOT_REGISTRY_BAD_STATE;
    else memset(entry, 0, sizeof(*entry));
    pthread_mutex_unlock(&registry->mutex);
    return result;
}

uint32_t slot_registry_count_claimed(slot_registry_t *registry) {
    uint32_t count = 0;
    if (!registry || !registry->initialized) return 0;

    pthread_mutex_lock(&registry->mutex);
    for (uint32_t i = 0; i < SLOT_REGISTRY_MAX_SLOTS; i++)
        if (registry->entries[i].state != SLOT_REGISTRY_FREE) count++;
    pthread_mutex_unlock(&registry->mutex);
    return count;
}

int slot_registry_begin_virtual_create(slot_registry_t *r, slot_registry_token_t t) {
    if(!r || !r->initialized)return SLOT_REGISTRY_ERROR;
    pthread_mutex_lock(&r->mutex);
    slot_registry_entry_t *e=find_token_locked(r,t);int rc=SLOT_REGISTRY_OK;
    if(!e)rc=SLOT_REGISTRY_STALE;
    else if(r->creation_blocked || (e->state!=SLOT_REGISTRY_ACTIVE && e->state!=SLOT_REGISTRY_RESERVED) || e->virtual_ready || e->virtual_deleting || e->virtual_creating || e->virtual_blocked)rc=SLOT_REGISTRY_BUSY;
    else e->virtual_creating=1;
    pthread_mutex_unlock(&r->mutex);return rc;
}
int slot_registry_finish_virtual_create(slot_registry_t *r,slot_registry_token_t t,int32_t h) {
    if(!r || !r->initialized)return SLOT_REGISTRY_ERROR;
    pthread_mutex_lock(&r->mutex);
    slot_registry_entry_t *e=find_token_locked(r,t);int rc=SLOT_REGISTRY_OK;
    if(!e)rc=SLOT_REGISTRY_STALE;
    else if(!e->virtual_creating)rc=SLOT_REGISTRY_BAD_STATE;
    else {
        e->virtual_creating=0;
        if(h<=0)rc=SLOT_REGISTRY_ERROR;
        else {
            for(unsigned i=0;i<SLOT_REGISTRY_MAX_SLOTS;i++) {
                slot_registry_entry_t *other=&r->entries[i];
                if(other!=e && other->state!=SLOT_REGISTRY_FREE && (other->virtual_ready || other->virtual_deleting) && other->virtual_handle==h) {
                    e->virtual_blocked=1;r->creation_blocked=1;rc=SLOT_REGISTRY_HANDLE_IN_USE;break;
                }
            }
            if(rc==SLOT_REGISTRY_OK) {e->virtual_handle=h;e->virtual_ready=1;e->confirmed=0;}
        }
    }
    pthread_mutex_unlock(&r->mutex);return rc;
}
