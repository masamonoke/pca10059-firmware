#include "math_utils.h"

int math_utils_clamp_int(int x, int upper, int lower) {
    int v = math_utils_max_int(x, lower);
    return math_utils_min_int(v, upper);
}

int math_utils_max_int(int a, int b) {
    return a > b ? a : b;
}

int math_utils_min_int(int a, int b) {
    return a < b ? a : b;
}