#include "dependency/unity/unity_fixture.h"
#include "module/utils/math_utils.h"

TEST_GROUP(MathUtils);

TEST_SETUP(MathUtils) {}

TEST_TEAR_DOWN(MathUtils) {}

TEST(MathUtils, ClampUpper) {
    TEST_ASSERT_EQUAL(100, math_utils_clamp_int(101, 100, 0));
}

TEST(MathUtils, ClampLower) {
    TEST_ASSERT_EQUAL(0, math_utils_clamp_int(-1, 100, 0));
}

TEST(MathUtils, ClampValueInRangeUnchanged) {
    TEST_ASSERT_EQUAL(50, math_utils_clamp_int(50, 100, 0));
}