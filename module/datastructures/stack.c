#include "stack.h"
#include <stddef.h>
#include <stdlib.h>

#define STACK_BUFFER_LEN 1000
#define STACK_OBJECTS_LEN 100

static uint16_t* buffer;
static instance_t * objects;
static uint16_t current_free_object_idx;
static bool is_init = false;

void stack_ctx_init(void) {
    buffer_init(&buffer, STACK_BUFFER_LEN, &objects,
                STACK_OBJECTS_LEN, &current_free_object_idx);
    is_init = true;
}

static void init(void) {
    if (!is_init) {
        stack_ctx_init();
    }
}

void stack_ctx_free(void) {
    free(buffer);
    free(objects);
    buffer = NULL;
    objects = NULL;
}

instance_t* stack_ctx_alloc_instance(uint16_t len) {
    init();
    return buffer_alloc(&buffer, &objects, len, &current_free_object_idx, STACK_BUFFER_LEN);
}


void stack_ctx_dealloc_instance(instance_t* instance) {
    init();
    buffer_dealloc(instance, &buffer, STACK_BUFFER_LEN, &objects);
}

uint16_t stack_ctx_instance_pop(instance_t* instance) {
    if (instance->state == FREE) {
        return STACK_BUFFER_EMPTY_DATA;
    }

    uint16_t last_element_idx = instance->last_free_idx - 1;
    uint16_t data = buffer[last_element_idx];
    buffer[last_element_idx] = STACK_BUFFER_EMPTY_DATA;
    instance->last_free_idx--;
    instance->size--;

    return data;
}

bool stack_ctx_instance_push(instance_t* instance, uint16_t data) {
    if (instance->last_free_idx == instance->end_idx) {
        return false;
    }

    buffer[instance->last_free_idx] = data;
    instance->last_free_idx++;
    instance->size++;

    return true;
}

bool stack_ctx_instance_is_empty(instance_t* instance) {
    return instance->size == 0;
}

void stack_ctx_instance_empty_stack(instance_t* instance) {
    for (size_t i = instance->start_idx; i < instance->end_idx + 1; i++) {
        buffer[i] = STACK_BUFFER_EMPTY_DATA;
    }
    instance->last_free_idx = instance->start_idx;
    instance->size = 0;
}

uint16_t stack_ctx_instance_peek(instance_t* instance) {
    if (instance->last_free_idx == 0) {
        return STACK_BUFFER_EMPTY_DATA;
    }
    return buffer[instance->last_free_idx - 1];
}