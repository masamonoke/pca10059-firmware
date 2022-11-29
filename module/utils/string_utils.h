#ifndef MODULE_UTILS_STRING_UTILS
#define MODULE_UTILS_STRING_UTILS

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

bool string_utils_compare_string(const char* s1, const char*s2);

bool string_utils_parse_string_get_nums(const char* s, uint16_t* a, uint8_t len);

void string_utils_substring(const char* s, uint16_t start, uint16_t end, char* res);

void string_utils_substring_to_end(const char* s, uint16_t start, char* res);

void string_utils_to_lower_case(char *str);

void string_utils_to_upper_case(char *str);

#endif /* MODULE_UTILS_STRING_UTILS */
