#include "dependency/unity/unity_fixture.h"
#include "modules/utils/string_utils.h"

TEST_GROUP(StringUtils);

TEST_SETUP(StringUtils) {}

TEST_TEAR_DOWN(StringUtils) {}

TEST(StringUtils, CompareStrings) {
    char s[] = "rgb";
    TEST_ASSERT_TRUE(string_utils_compare_string(s, "rgb"));
    TEST_ASSERT_TRUE(string_utils_compare_string("r", "r"));
    TEST_ASSERT_FALSE(string_utils_compare_string(s, "rb"));
    TEST_ASSERT_FALSE(string_utils_compare_string(s, "RGB"));
    TEST_ASSERT_FALSE(string_utils_compare_string(s, "RgB"));
}

TEST(StringUtils, ParseStringWithNumeric3Args) {
    uint16_t args[3];
    TEST_ASSERT_TRUE(string_utils_parse_string_get_nums("1230 4 203", args, 3));
    TEST_ASSERT_EQUAL(1230, args[0]);
    TEST_ASSERT_EQUAL(4, args[1]);
    TEST_ASSERT_EQUAL(203, args[2]);
}


TEST(StringUtils, ParseStringWithNumeric4Args) {
    uint16_t args[4];
    TEST_ASSERT_TRUE(string_utils_parse_string_get_nums("1230 4 203 1233", args, 4));
    TEST_ASSERT_EQUAL(1230, args[0]);
    TEST_ASSERT_EQUAL(4, args[1]);
    TEST_ASSERT_EQUAL(203, args[2]);
    TEST_ASSERT_EQUAL(1233, args[3]);
}

TEST(StringUtils, CannotParseWithArgsDigitsCountMoreThan5) {
    uint16_t args[4];
    TEST_ASSERT_TRUE(string_utils_parse_string_get_nums("1230 4 203 12335", args, 4));
    TEST_ASSERT_EQUAL(1230, args[0]);
    TEST_ASSERT_EQUAL(4, args[1]);
    TEST_ASSERT_EQUAL(203, args[2]);
    TEST_ASSERT_EQUAL(12335, args[3]);
    TEST_ASSERT_FALSE(string_utils_parse_string_get_nums("1230 4 203 1233533", args, 4));
}

TEST(StringUtils, WhenInvalidDataPassedArgsArrayElementsAllZero) {
    uint16_t args[4];
    TEST_ASSERT_FALSE(string_utils_parse_string_get_nums("1230 4 203 123355", args, 4));
    TEST_ASSERT_EQUAL(0, args[0]);
    TEST_ASSERT_EQUAL(0, args[1]);
    TEST_ASSERT_EQUAL(0, args[2]);
    TEST_ASSERT_EQUAL(0, args[3]);

    TEST_ASSERT_FALSE(string_utils_parse_string_get_nums("123355 4 203 123", args, 4));
    TEST_ASSERT_EQUAL(0, args[0]);
    TEST_ASSERT_EQUAL(0, args[1]);
    TEST_ASSERT_EQUAL(0, args[2]);
    TEST_ASSERT_EQUAL(0, args[3]);

    TEST_ASSERT_FALSE(string_utils_parse_string_get_nums("213 123355 203 123", args, 4));
    TEST_ASSERT_EQUAL(0, args[0]);
    TEST_ASSERT_EQUAL(0, args[1]);
    TEST_ASSERT_EQUAL(0, args[2]);
    TEST_ASSERT_EQUAL(0, args[3]);

    TEST_ASSERT_FALSE(string_utils_parse_string_get_nums("1230 4 123355 123", args, 4));
    TEST_ASSERT_EQUAL(0, args[0]);
    TEST_ASSERT_EQUAL(0, args[1]);
    TEST_ASSERT_EQUAL(0, args[2]);
    TEST_ASSERT_EQUAL(0, args[3]);
}

TEST(StringUtils, LowerCaseString) {
    char str[] = "QWERTYYUI";
    string_utils_to_lower_case(str);
    TEST_ASSERT_EQUAL_STRING("qwertyyui", str);
}

TEST(StringUtils, LowerCaseStringWithNotOnlyLetters) {
    char str[] = "QWERT.,YYUI";
    string_utils_to_lower_case(str);
    TEST_ASSERT_EQUAL_STRING("qwert.,yyui", str);
}

TEST(StringUtils, LowerCaseStringWithLowerLetters) {
    char str[] = "qWeRtYYuI";
    string_utils_to_lower_case(str);
    TEST_ASSERT_EQUAL_STRING("qwertyyui", str);
}

TEST(StringUtils, UpperCase) {
    char str[] = "qwertyyui";
    string_utils_to_upper_case(str);
    TEST_ASSERT_EQUAL_STRING("QWERTYYUI", str);
}

TEST(StringUtils, UpperCaseStringWithNotOnlyLetters) {
    char str[] = "qwert.,yyui";
    string_utils_to_upper_case(str);
    TEST_ASSERT_EQUAL_STRING("QWERT.,YYUI", str);
}

TEST(StringUtils, UpperCaseStringWithUpperLetters) {
    char str[] = "qWeRtYYuI";
    string_utils_to_upper_case(str);
    TEST_ASSERT_EQUAL_STRING("QWERTYYUI", str);
}
