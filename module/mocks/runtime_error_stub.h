#ifndef MODULE_MOCKS_RUNTIME_ERROR_STUB
#define MODULE_MOCKS_RUNTIME_ERROR_STUB

void runtime_error_stub_reset(void);

const char* runtime_error_stub_get_last_error(void);

int runtime_error_stub_get_last_parameter(void);

#endif /* MODULE_MOCKS_RUNTIME_ERROR_STUB */
