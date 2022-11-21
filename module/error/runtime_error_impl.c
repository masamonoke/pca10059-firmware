#include <stdlib.h>
#include "nrf_log.h"
#include "runtime_error_impl.h"
#include "module/data_structures/stack.h"

typedef struct {
    uint16_t id;
    const char* message;
    int err_code;
} error_t;

static uint8_t id_counter;
static bool is_stack_init = false;
static instance_t* stack;
#define ERRORS_LEN 255
static error_t* s_errors_;
static bool s_is_any_errors = false;

void runtime_error_init(void) {
    stack = stack_ctx_alloc_instance(100);
    s_errors_ = malloc(sizeof(error_t) * ERRORS_LEN);
    is_stack_init = true;
}

void runtime_error(const char* m, int p) {
    if (!is_stack_init) {
        runtime_error_init();
    }

    error_t e = {
            .id = id_counter,
            .message = m,
            .err_code = p
    };
    s_errors_[id_counter] = e;
    id_counter++;
    stack_ctx_instance_push(stack, e.id);
    s_is_any_errors = true;
}

void runtime_error_stacktrace(void) {
    uint8_t id;
    while (!stack_ctx_instance_is_empty(stack)) {
        id = stack_ctx_instance_pop(stack);
        NRF_LOG_INFO("%s\n", s_errors_[id].message);
    }
}