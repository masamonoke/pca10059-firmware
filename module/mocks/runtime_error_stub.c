#include "runtime_error_stub.h"

static const char* message = "No error";
static int parameter = -1;
static const char* file = 0;
static int line = -1;

void runtime_error_stub_reset(void) {
    message = "No error";
    parameter = -1;
}

const char* runtime_error_stub_get_last_error(void) {
    return message;
}

void runtime_error(const char* m, int p, const char* f, int l) {
    message = m;
    parameter = p;
    file = f;
    line = l;
}

int runtime_error_stub_get_last_parameter(void) {
    return parameter;
}