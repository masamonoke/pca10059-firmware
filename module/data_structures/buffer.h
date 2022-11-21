#ifndef PURE_C_BUFFER_H
#define PURE_C_BUFFER_H

#include <stdint.h>

typedef enum {
    FREE,
    FILLED
} state_t;

typedef struct {
    uint16_t id;
    uint16_t start_idx;
    uint16_t end_idx;
    uint16_t last_free_idx;
    state_t state;
    uint16_t size;
} instance_t;

void buffer_init(uint16_t** buffer, uint16_t buffer_len, instance_t** objects, uint16_t objects_len,
                 uint16_t* current_free_object_idx);

instance_t* buffer_alloc(uint16_t** buffer, instance_t** objects, uint16_t len, uint16_t* current_free_object_idx,
                         uint16_t buffer_len);

void buffer_dealloc(instance_t* instance, uint16_t** buffer, uint16_t buffer_len, instance_t** objects);

#endif //PURE_C_BUFFER_H
