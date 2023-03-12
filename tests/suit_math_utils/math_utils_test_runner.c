#include "math_utils_test.c"

TEST_GROUP_RUNNER(MathUtils) {
    RUN_TEST_CASE(MathUtils, ClampUpper);
    RUN_TEST_CASE(MathUtils, ClampLower);
    RUN_TEST_CASE(MathUtils, ClampValueInRangeUnchanged);
}