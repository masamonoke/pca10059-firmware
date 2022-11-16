#include <stddef.h>
#include <stdlib.h>

#include "queue.h"
#include "module/error/runtime_error.h"

#define BUFFER_LEN 5000

typedef enum {
    FREE,
    FILLED
} state_t;

struct queue_instance_t {
    uint16_t id;
    uint16_t start_idx;
    uint16_t end_idx;
    uint16_t last_free_idx;
    state_t state;
    uint16_t size;
};

static uint16_t* buffer;
static queue_instance_t* objects;
static uint16_t current_free_object_idx;

static bool is_init = false;

void queue_init(void) {
    if (is_init) {
        RUNTIME_ERROR("Queue context is already initialized", -1);
        return;
    }

    buffer = malloc(sizeof(uint16_t) * BUFFER_LEN);
    objects = malloc(sizeof(queue_instance_t) * BUFFER_LEN);

    for (size_t i = 0; i < BUFFER_LEN; i++) {
        buffer[i] = EMPTY;
    }
    for (size_t i = 0; i < BUFFER_LEN; i++) {
        objects[i].id = i;
        objects[i].start_idx = EMPTY;
        objects[i].end_idx = EMPTY;
        objects[i].state = FREE;
    }
    current_free_object_idx = 0;
    is_init = true;
}

static void init(void) {
    if (!is_init) {
        queue_init();
    }
}

static void find_free_object(void) {
    size_t i = 0;
    bool is_found = false;
    while (true) {
        i++;
        if (objects[i].state == FREE) {
            is_found = true;
            break;
        }
    }
    if (is_found) {
        current_free_object_idx = i;
    }
}

queue_instance_t* queue_alloc(uint16_t len) {

    init();

    queue_instance_t* instance = &objects[current_free_object_idx];

    if (instance->id != 0) {
        queue_instance_t prev_object = objects[instance->id - 1];
        instance->start_idx = prev_object.end_idx + 1;
        instance->end_idx = instance->start_idx + len - 1;
    } else {
        instance->start_idx = 0;
        instance->end_idx = instance->start_idx + len - 1;
    }

    for (size_t i = instance->start_idx; i < instance->end_idx + 1; i++) {
        buffer[i] = 0;
    }

    instance->state = FILLED;
    instance->last_free_idx = instance->start_idx;

    find_free_object();

    return instance;
}

static uint16_t shift_chunk(uint16_t len, queue_instance_t* object) {
    uint16_t shift = len;
    uint16_t old_start = object->start_idx - 1;
    uint16_t old_end = object->end_idx;
    object->start_idx -= shift + 1;
    object->end_idx -= shift + 1;
    object->last_free_idx -= shift + 1;
    for (size_t i = old_start; i < old_end; i++) {
        buffer[i - shift] = buffer[i];
        buffer[i] = EMPTY;
    }
    return old_end;
}

static queue_instance_t* find_chunk_object(uint16_t start) {
    for (size_t i = 0; i < BUFFER_LEN; i++) {
        if (objects[i].start_idx == start && objects[i].state == FILLED) {
            return &objects[i];
        }
    }
    return NULL;
}

static uint16_t find_start_of_next_chunk(uint16_t end) {
    for (size_t i = end; i < BUFFER_LEN; i++) {
        if (buffer[i] != EMPTY) {
            return i;
        }
    }
    return EMPTY;
}

static void shift_buffer(uint16_t start, uint16_t end) {
    size_t next_chunk_idx_start = end + 1;
    for (size_t i = start; i < BUFFER_LEN; i++) {
        queue_instance_t* object = find_chunk_object(next_chunk_idx_start);
        if (object == NULL) {
            continue;
        }
        uint16_t len = end - start;
        end = shift_chunk(len, object);
        start = end - len;
        next_chunk_idx_start = find_start_of_next_chunk(end) + 1;
    }
    buffer[next_chunk_idx_start] = EMPTY;
}

void queue_dealloc(queue_instance_t* instance) {

    init();

    for (size_t i = instance->start_idx; i < instance->end_idx; i++) {
        buffer[i] = EMPTY;
    }
    shift_buffer(instance->start_idx, instance->end_idx);
    objects[instance->id].start_idx = EMPTY;
    objects[instance->id].end_idx = EMPTY;
    objects[instance->id].state = FREE;
    objects[instance->id].last_free_idx = EMPTY;
}

uint16_t queue_poll(queue_instance_t* instance) {
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

bool queue_push(queue_instance_t* instance, uint16_t data) {
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

bool queue_remove(queue_instance_t* instance, uint16_t data) {
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

uint16_t queue_peek(queue_instance_t* instance) {
    return buffer[instance->start_idx];
}

bool queue_is_empty(queue_instance_t* instance) {
    return instance->size == 0;
}

void queue_empty(queue_instance_t* instance) {
    for (size_t i = instance->start_idx; i < instance->end_idx + 1; i++) {
        buffer[i] = EMPTY;
    }
    instance->last_free_idx = instance->start_idx;
    instance->size = 0;
}

void queue_empty_half(queue_instance_t* instance) {
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

void queue_free(void) {
    free(buffer);
    free(objects);
}

uint16_t queue_get_size(queue_instance_t* instance) {
    return instance->size;
}