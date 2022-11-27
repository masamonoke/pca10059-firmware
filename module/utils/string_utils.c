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

bool string_utils_parse_string_get_nums(const char* s, uint16_t* a, uint8_t len) {
    size_t i = 0;
    size_t k = 0;
    size_t j = 0;
    char buff[4];
    while (s[i] != '\0' && j != len) {
        buff[k] = s[i];
        i++;
        k++;
        if (k <= 4 && buff[k - 1] == ' ') {
            uint16_t val = strtol(buff, NULL, 10);
            a[j] = val;
            j++;
            k = 0;
            for (size_t z = 0; z < 4; z++) {
                buff[z] = '\0';
            }
        }
    }
    if (s[i] == '\0') {
        uint16_t val = strtol(buff, NULL, 10);
        a[j] = val;
    }
    
    if (len - 1 != j && j != len) {
        for (size_t z = j + 1; z < len; z++) {
            a[z] = 0;
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