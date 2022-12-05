#ifndef MODULE_APP_HSV_EDITOR_CLI_CLI
#define MODULE_APP_HSV_EDITOR_CLI_CLI
#include <stdint.h>
#include <stdbool.h>

void cli_proceed(const char* input);

bool cli_is_there_message(void);

void cli_get_message(char* str);

void cli_set_message(const char* str);

#endif /* MODULE_APP_HSV_EDITOR_CLI_CLI */
