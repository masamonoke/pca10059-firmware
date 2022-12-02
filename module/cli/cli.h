#ifndef MODULE_CLI_CLI
#define MODULE_CLI_CLI
#include <stdint.h>
#include <stdbool.h>

void cli_proceed(char* input);

bool cli_is_there_message(void);

void cli_get_message(char* str, uint16_t* len);

void cli_set_message(char* str, uint16_t len);

#endif /* MODULE_CLI_CLI */
