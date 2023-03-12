#ifndef MODULE_ERROR_RUNTIME_ERROR_IMPL
#define MODULE_ERROR_RUNTIME_ERROR_IMPL

#include <stdbool.h>

void runtime_error_init(void);

void runtime_error_log_stacktrace(void);

bool runtime_error_is_any_error(void);

bool runtime_error_is_error_just_thrown(void);

#endif /* MODULE_ERROR_RUNTIME_ERROR_IMPL */
