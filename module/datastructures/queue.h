#ifndef MODULE_DATASTRUCTURES_QUEUE
#define MODULE_DATASTRUCTURES_QUEUE

#include <stdint.h>
#include <stdbool.h>

#include "buffer.h"

#define EMPTY 0xFFFF

void queue_ctx_init(void);

void queue_ctx_free(void);

instance_t* queue_ctx_alloc_instance(uint16_t len);

void queue_ctx_dealloc_instance(instance_t* instance);

uint16_t queue_ctx_instance_poll(instance_t* instance);

bool queue_ctx_instance_push(instance_t* instance, uint16_t data);

bool queue_ctx_is_empty_instance(instance_t* instance);

bool queue_ctx_instance_remove_element(instance_t* instance, uint16_t data);

uint16_t queue_ctx_peek_instance(instance_t* instance);

void queue_ctx_empty_instance(instance_t* instance);

void queue_ctx_empty_half_instance(instance_t* instance);

uint16_t queue_ctx_instance_get_size(instance_t* instance);

#endif /* MODULE_DATASTRUCTURES_QUEUE */
