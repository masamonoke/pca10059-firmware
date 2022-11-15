#ifndef MODULE_DATASTRUCTURES_QUEUE
#define MODULE_DATASTRUCTURES_QUEUE

#include <stdint.h>
#include <stdbool.h>

#define EMPTY 0xFFFF

typedef struct queue_instance_t queue_instance_t;

void queue_init(void);

void queue_free(void);

queue_instance_t* queue_alloc(uint16_t len);

void queue_dealloc(queue_instance_t* instance);

uint16_t queue_poll(queue_instance_t* instance);

bool queue_push(queue_instance_t* instance, uint16_t data);

bool queue_is_empty(queue_instance_t* instance);

bool queue_remove(queue_instance_t* instance, uint16_t data);

uint16_t queue_peek(queue_instance_t* instance);

void queue_empty(queue_instance_t* instance);

void queue_empty_half(queue_instance_t* instance);

uint16_t queue_get_size(queue_instance_t* instance);

#endif /* MODULE_DATASTRUCTURES_QUEUE */
