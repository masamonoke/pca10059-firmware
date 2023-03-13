#include "string_utils_test.c"

TEST_GROUP_RUNNER(StringUtils) {
    RUN_TEST_CASE(StringUtils, CompareStrings);
    RUN_TEST_CASE(StringUtils, ParseStringWithNumeric3Args);
    RUN_TEST_CASE(StringUtils, ParseStringWithNumeric4Args);
    RUN_TEST_CASE(StringUtils, CannotParseWithArgsDigitsCountMoreThan5);
    RUN_TEST_CASE(StringUtils, WhenInvalidDataPassedArgsArrayElementsAllZero);
    RUN_TEST_CASE(StringUtils, LowerCaseString);
    RUN_TEST_CASE(StringUtils, LowerCaseStringWithNotOnlyLetters);
    RUN_TEST_CASE(StringUtils, LowerCaseStringWithLowerLetters);
    RUN_TEST_CASE(StringUtils, UpperCase);
    RUN_TEST_CASE(StringUtils, UpperCaseStringWithNotOnlyLetters);
    RUN_TEST_CASE(StringUtils, UpperCaseStringWithUpperLetters);
}