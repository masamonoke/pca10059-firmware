#ifndef PURE_C_STACK_H
#define PURE_C_STACK_H

#include <stdint.h>
#include <stdbool.h>
#include "buffer.h"

#define STACK_BUFFER_EMPTY_DATA 0xFFFF

void stack_ctx_init(void);

void stack_ctx_free(void);

instance_t * stack_ctx_alloc_instance(uint16_t len);

void stack_ctx_dealloc_instance(instance_t * instance);

uint16_t stack_ctx_instance_pop(instance_t * instance);

bool stack_ctx_instance_push(instance_t * instance, uint16_t data);

bool stack_ctx_instance_is_empty(instance_t * instance);

void stack_ctx_instance_empty_stack(instance_t * instance);

uint16_t stack_ctx_instance_peek(instance_t* instance);

#endif //PURE_C_STACK_H
