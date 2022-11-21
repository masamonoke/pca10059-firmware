#include <stddef.h>
#include <stdlib.h>

#include "queue.h"

#define QUEUE_BUFFER_LEN 1000
#define QUEUE_OBJECTS_LEN 100

static uint16_t* buffer;
static instance_t* objects;
static uint16_t current_free_object_idx;

static bool is_init = false;

void queue_ctx_init(void) {
    buffer_init(&buffer, QUEUE_BUFFER_LEN, &objects,
                QUEUE_OBJECTS_LEN, &current_free_object_idx);
    is_init = true;
}

static void init(void) {
    if (!is_init) {
        queue_ctx_init();
    }
}

instance_t* queue_ctx_alloc_instance(uint16_t len) {
    init();
    return buffer_alloc(&buffer, &objects, len, &current_free_object_idx, QUEUE_BUFFER_LEN);
}


void queue_ctx_dealloc_instance(instance_t* instance) {
    init();
    buffer_dealloc(instance, &buffer, QUEUE_BUFFER_LEN, &objects);
}

uint16_t queue_ctx_instance_poll(instance_t* instance) {
    if (instance->state == FREE) {
        return EMPTY;
    }

    uint16_t data = buffer[instance->start_idx];
    if (data == EMPTY) {
        return EMPTY;
    }
    size_t i;
    for (i = instance->start_idx + 1; i < instance->end_idx + 1; i++) {
        buffer[i - 1] = buffer[i];
    }
    if (instance->last_free_idx != 0) {
        instance->last_free_idx -= 1;
    }
    buffer[i - 1] = EMPTY;
    instance->size--;
    return data;
}

bool queue_ctx_instance_push(instance_t* instance, uint16_t data) {
    if (instance->last_free_idx == instance->end_idx + 1) {
        return false;
    }
    buffer[instance->last_free_idx] = data;
    instance->last_free_idx++;
    instance->size++;
    return true;
}

static void shift_elements_in_chunk(uint16_t end_idx, uint16_t remove_idx) {
    for (size_t i = remove_idx + 1; i < end_idx; i++) {
        buffer[i - 1] = buffer[i];
    }
}

bool queue_ctx_instance_remove_element(instance_t* instance, uint16_t data) {
    for (size_t i = instance->start_idx; i < instance->end_idx; i++) {
        if (buffer[i] == data) {
            buffer[i] = EMPTY;
            shift_elements_in_chunk(instance->end_idx, i);
            instance->size--;
            break;
        }
    }
    return false;
}

uint16_t queue_ctx_peek_instance(instance_t* instance) {
    return buffer[instance->start_idx];
}

bool queue_ctx_is_empty_instance(instance_t* instance) {
    return instance->size == 0;
}

void queue_ctx_empty_instance(instance_t* instance) {
    for (size_t i = instance->start_idx; i < instance->end_idx + 1; i++) {
        buffer[i] = EMPTY;
    }
    instance->last_free_idx = instance->start_idx;
    instance->size = 0;
}

void queue_ctx_empty_half_instance(instance_t* instance) {
    uint16_t half_idx = instance->end_idx - (instance->end_idx - instance->start_idx) / 2 ;
    for (size_t i = (instance->end_idx + 1) / 2; i < instance->end_idx + 1; i++) {
        buffer[i - half_idx] = buffer[i];
    }

    for (size_t i = (instance->end_idx + 1) / 2; i < instance->end_idx + 1; i++) {
        buffer[i] = EMPTY;
    }

    instance->last_free_idx = instance->end_idx - (instance->end_idx - instance->start_idx) / 2;
    instance->size /= 2;
}

void queue_ctx_free(void) {
    free(buffer);
    free(objects);
    buffer = NULL;
    objects = NULL;
}

uint16_t queue_ctx_instance_get_size(instance_t* instance) {
    return instance->size;
}