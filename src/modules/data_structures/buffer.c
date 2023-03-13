#include <stdbool.h>
#include <stdlib.h>
#include "buffer.h"

#define BUFFER_EMPTY_DATA 0xFFFF

void buffer_init(uint16_t** buffer, uint16_t buffer_len, instance_t** objects, uint16_t objects_len, uint16_t* current_free_object_idx) {
    *buffer = malloc(sizeof(uint16_t) * buffer_len);
    *objects = malloc(sizeof(instance_t) * objects_len);

    for (size_t i = 0; i < buffer_len; i++) {
        *(*buffer + i) = BUFFER_EMPTY_DATA;
    }

    for (size_t i = 0; i < objects_len; i++) {
        (*(*objects + i)).id = i;
        (*(*objects + i)).start_idx = BUFFER_EMPTY_DATA;
        (*(*objects + i)).end_idx = BUFFER_EMPTY_DATA;
        (*(*objects + i)).state = FREE;
    }

    *current_free_object_idx = 0;
}

static void s_find_free_object_(instance_t** objects, uint16_t* current_free_object_idx) {
    size_t i = 0;
    bool is_found = false;
    while (true) {
        if ((*(*objects + i)).state == FREE) {
            is_found = true;
            break;
        }
        i++;
    }
    if (is_found) {
        *current_free_object_idx = i;
    }
}

instance_t* buffer_alloc(uint16_t** buffer, instance_t** objects, uint16_t len, uint16_t* current_free_object_idx,
                         uint16_t buffer_len) {
    instance_t* instance = (*objects + *current_free_object_idx);

    size_t k = 0;
    while (k != buffer_len) {
        if (*(*buffer + k) == BUFFER_EMPTY_DATA) {
            break;
        }
        k++;
    }
    instance->start_idx = k;
    instance->end_idx = instance->start_idx + len - 1;


    for (size_t i = instance->start_idx; i < instance->end_idx + 1; i++) {
        *(*buffer + i) = 0;
    }

    instance->state = FILLED;
    instance->last_free_idx = instance->start_idx;

    s_find_free_object_(objects, current_free_object_idx);

    return instance;
}

static uint16_t s_shift_chunk_(uint16_t** buffer, uint16_t len, instance_t* object) {
    uint16_t shift = len;
    uint16_t old_start = object->start_idx - 1;
    uint16_t old_end = object->end_idx;
    object->start_idx -= shift + 1;
    object->end_idx -= shift + 1;
    object->last_free_idx -= shift + 1;
    for (size_t i = old_start; i < old_end; i++) {
        *(*buffer + i - shift) = *(*buffer + i);
        *(*buffer + i) = BUFFER_EMPTY_DATA;
    }
    return old_end;
}

static instance_t* s_find_chunk_object_(instance_t** objects, uint16_t buffer_len, uint16_t start) {
    for (size_t i = 0; i < buffer_len; i++) {
        if ((*(*objects + i)).start_idx == start && (*(*objects + i)).state == FILLED) {
            return (*objects + i);
        }
    }
    return NULL;
}

static uint16_t s_find_start_of_next_chunk_(uint16_t** buffer, uint16_t buffer_len, uint16_t end) {
    for (size_t i = end; i < buffer_len; i++) {
        if (*(*buffer + i) != BUFFER_EMPTY_DATA) {
            return i;
        }
    }
    return BUFFER_EMPTY_DATA;
}

static void s_shift_buffer_(uint16_t** buffer, instance_t** objects, uint16_t buffer_len, uint16_t start, uint16_t end) {
    size_t next_chunk_idx_start = end + 1;
    for (size_t i = start; i < buffer_len; i++) {
        instance_t * object = s_find_chunk_object_(objects, buffer_len, next_chunk_idx_start);
        if (object == NULL) {
            continue;
        }
        uint16_t len = end - start;
        end = s_shift_chunk_(buffer, len, object);
        start = end - len;
        next_chunk_idx_start = s_find_start_of_next_chunk_(buffer, buffer_len, end) + 1;
    }
    *(*buffer + next_chunk_idx_start) = BUFFER_EMPTY_DATA;
}

void buffer_dealloc(instance_t* instance, uint16_t** buffer, uint16_t buffer_len, instance_t** objects) {
    for (size_t i = instance->start_idx; i < instance->end_idx; i++) {
        *(*buffer + i) = BUFFER_EMPTY_DATA;
    }
    s_shift_buffer_(buffer, objects, buffer_len, instance->start_idx, instance->end_idx);
    (*(*objects + instance->id)).start_idx = BUFFER_EMPTY_DATA;
    (*(*objects + instance->id)).end_idx = BUFFER_EMPTY_DATA;
    (*(*objects + instance->id)).state = FREE;
    (*(*objects + instance->id)).last_free_idx = BUFFER_EMPTY_DATA;
}