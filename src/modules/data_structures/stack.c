#include <stddef.h>
#include <stdlib.h>

#include "stack.h"
#include "modules/error/runtime_error.h"

#define STACK_BUFFER_LEN 1000
#define STACK_OBJECTS_LEN 100

static uint16_t* s_buffer_;
static instance_t * s_objects_;
static uint16_t s_current_free_object_idx_;
static bool is_init = false;

void stack_ctx_init(void) {
    if (is_init) {
        RUNTIME_ERROR("Stack is already initialized", -1);
        return;
    }
    buffer_init(&s_buffer_, STACK_BUFFER_LEN, &s_objects_,
                STACK_OBJECTS_LEN, &s_current_free_object_idx_);
    is_init = true;
}

static void init(void) {
    if (!is_init) {
        stack_ctx_init();
    }
}

void stack_ctx_free(void) {
    free(s_buffer_);
    free(s_objects_);
    s_buffer_ = NULL;
    s_objects_ = NULL;
}

instance_t* stack_ctx_alloc_instance(uint16_t len) {
    init();
    return buffer_alloc(&s_buffer_, &s_objects_, len, &s_current_free_object_idx_, STACK_BUFFER_LEN);
}


void stack_ctx_dealloc_instance(instance_t* instance) {
    init();
    buffer_dealloc(instance, &s_buffer_, STACK_BUFFER_LEN, &s_objects_);
}

uint16_t stack_ctx_instance_pop(instance_t* instance) {
    if (instance->state == FREE) {
        return STACK_BUFFER_EMPTY_DATA;
    }

    uint16_t last_element_idx = instance->last_free_idx - 1;
    uint16_t data = s_buffer_[last_element_idx];
    s_buffer_[last_element_idx] = STACK_BUFFER_EMPTY_DATA;
    instance->last_free_idx--;
    instance->size--;

    return data;
}

bool stack_ctx_instance_push(instance_t* instance, uint16_t data) {
    if (instance->last_free_idx == instance->end_idx) {
        return false;
    }

    s_buffer_[instance->last_free_idx] = data;
    instance->last_free_idx++;
    instance->size++;

    return true;
}

bool stack_ctx_instance_is_empty(instance_t* instance) {
    return instance->size == 0;
}

void stack_ctx_instance_empty_stack(instance_t* instance) {
    for (size_t i = instance->start_idx; i < instance->end_idx + 1; i++) {
        s_buffer_[i] = STACK_BUFFER_EMPTY_DATA;
    }
    instance->last_free_idx = instance->start_idx;
    instance->size = 0;
}

uint16_t stack_ctx_instance_peek(instance_t* instance) {
    if (instance->last_free_idx == 0) {
        return STACK_BUFFER_EMPTY_DATA;
    }
    return s_buffer_[instance->last_free_idx - 1];
}
