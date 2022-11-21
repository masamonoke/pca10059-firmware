#include <stddef.h>
#include <stdlib.h>

#include "queue.h"
#include "module/error/runtime_error.h"

#define QUEUE_BUFFER_LEN 1000
#define QUEUE_OBJECTS_LEN 100

static uint16_t* s_buffer_;
static instance_t* s_objects_;
static uint16_t s_current_free_object_idx_;

static bool is_init = false;

void queue_ctx_init(void) {
    if (is_init) {
        RUNTIME_ERROR("Queue is already initialized", -1);
        return;
    }
    buffer_init(&s_buffer_, QUEUE_BUFFER_LEN, &s_objects_,
                QUEUE_OBJECTS_LEN, &s_current_free_object_idx_);
    is_init = true;
}

static void init(void) {
    if (!is_init) {
        queue_ctx_init();
    }
}

instance_t* queue_ctx_alloc_instance(uint16_t len) {
    init();
    return buffer_alloc(&s_buffer_, &s_objects_, len, &s_current_free_object_idx_, QUEUE_BUFFER_LEN);
}


void queue_ctx_dealloc_instance(instance_t* instance) {
    init();
    buffer_dealloc(instance, &s_buffer_, QUEUE_BUFFER_LEN, &s_objects_);
}

uint16_t queue_ctx_instance_poll(instance_t* instance) {
    if (instance->state == FREE) {
        return EMPTY;
    }

    uint16_t data = s_buffer_[instance->start_idx];
    if (data == EMPTY) {
        return EMPTY;
    }
    size_t i;
    for (i = instance->start_idx + 1; i < instance->end_idx + 1; i++) {
        s_buffer_[i - 1] = s_buffer_[i];
    }
    if (instance->last_free_idx != 0) {
        instance->last_free_idx -= 1;
    }
    s_buffer_[i - 1] = EMPTY;
    instance->size--;
    return data;
}

bool queue_ctx_instance_push(instance_t* instance, uint16_t data) {
    if (instance->last_free_idx == instance->end_idx + 1) {
        return false;
    }
    s_buffer_[instance->last_free_idx] = data;
    instance->last_free_idx++;
    instance->size++;
    return true;
}

static void shift_elements_in_chunk(uint16_t end_idx, uint16_t remove_idx) {
    for (size_t i = remove_idx + 1; i < end_idx; i++) {
        s_buffer_[i - 1] = s_buffer_[i];
    }
}

bool queue_ctx_instance_remove_element(instance_t* instance, uint16_t data) {
    for (size_t i = instance->start_idx; i < instance->end_idx; i++) {
        if (s_buffer_[i] == data) {
            s_buffer_[i] = EMPTY;
            shift_elements_in_chunk(instance->end_idx, i);
            instance->size--;
            break;
        }
    }
    return false;
}

uint16_t queue_ctx_peek_instance(instance_t* instance) {
    return s_buffer_[instance->start_idx];
}

bool queue_ctx_is_empty_instance(instance_t* instance) {
    return instance->size == 0;
}

void queue_ctx_empty_instance(instance_t* instance) {
    for (size_t i = instance->start_idx; i < instance->end_idx + 1; i++) {
        s_buffer_[i] = EMPTY;
    }
    instance->last_free_idx = instance->start_idx;
    instance->size = 0;
}

void queue_ctx_empty_half_instance(instance_t* instance) {
    uint16_t half_idx = instance->end_idx - (instance->end_idx - instance->start_idx) / 2 ;
    for (size_t i = (instance->end_idx + 1) / 2; i < instance->end_idx + 1; i++) {
        s_buffer_[i - half_idx] = s_buffer_[i];
    }

    for (size_t i = (instance->end_idx + 1) / 2; i < instance->end_idx + 1; i++) {
        s_buffer_[i] = EMPTY;
    }

    instance->last_free_idx = instance->end_idx - (instance->end_idx - instance->start_idx) / 2;
    instance->size /= 2;
}

void queue_ctx_free(void) {
    free(s_buffer_);
    free(s_objects_);
    s_buffer_ = NULL;
    s_objects_ = NULL;
}

uint16_t queue_ctx_instance_get_size(instance_t* instance) {
    return instance->size;
}