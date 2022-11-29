#include "string_utils.h"

bool string_utils_compare_string(const char* s1, const char*s2) {
    size_t i = 0;
    while (s1[i] != '\0') {
        i++;
        if (s2[i] == '\0' && s1[i] != '\0') {
            return false;
        }
        if (s2[i] != '\0' && s1[i] == '\0') {
            return false;
        }
        if (s1[i] != s2[i]) {
            return false;
        }
    }
    return true;
}

#define MAX_DIGITS 5
bool string_utils_parse_string_get_nums(const char* s, uint16_t* args, uint8_t args_size) {
    size_t i = 0;
    size_t k = 0;
    size_t j = 0;
    uint8_t buff_size = MAX_DIGITS + 1;
    char buff[buff_size];
    bool is_data_invalid = false;
    while (s[i] != '\0' && j != args_size) {
        buff[k] = s[i];
        i++;
        k++;
        if (k > buff_size - 1) {
            is_data_invalid = true;
            break;
        }
        if (k <= buff_size && buff[k - 1] == ' ') {
            uint16_t val = strtol(buff, NULL, 10);
            args[j] = val;
            j++;
            k = 0;
            for (size_t z = 0; z < 4; z++) {
                buff[z] = '\0';
            }
        }
    }
    if (s[i] == '\0') {
        uint16_t val = strtol(buff, NULL, 10);
        args[j] = val;
    }

    if ((args_size - 1 != j && j != args_size) || is_data_invalid == true) {
        for (size_t z = 0; z < args_size; z++) {
            args[z] = 0;
        }
        return false;
    }
    return true;
}

void string_utils_substring(const char* s, uint16_t start, uint16_t end, char* res) {
    size_t i = start;
    size_t k = 0;
    while (i != end + 1) {
        res[k] = s[i];
        i++;
        k++;
    }
}

void string_utils_substring_to_end(const char* s, uint16_t start, char* res) {
    size_t i = start;
    size_t k = 0;
    while (s[i] != '\0') {
        res[k] = s[i];
        i++;
        k++;
    }
}

void string_utils_to_lower_case(char *str) {
    while(*str != '\0') {
        if(*str >= 65 && *str <= 90) {
            *str = (char) (*str + 32);
        }
        str++;
    }
}

void string_utils_to_upper_case(char *str) {
    while(*str != '\0') {
        if(*str >= 97 && *str <= 122) {
            *str = (char) (*str - 32);
        }
        str++;
    }
}