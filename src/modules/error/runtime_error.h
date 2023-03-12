#ifndef MODULE_ERROR_RUNTIME_ERROR
#define MODULE_ERROR_RUNTIME_ERROR

void runtime_error(const char* message, int parameter, const char* file, int line);
#define RUNTIME_ERROR(desc, param) runtime_error(desc, param, __FILE__, __LINE__)

#endif /* MODULE_ERROR_RUNTIME_ERROR */
