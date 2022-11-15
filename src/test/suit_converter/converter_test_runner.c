#include "converter_test.c"

TEST_GROUP_RUNNER(Converter) {
    RUN_TEST_CASE(Converter, SetWrongRGB);
    RUN_TEST_CASE(Converter, SetRGB);
    RUN_TEST_CASE(Converter, FromRGBtoHSV);
    RUN_TEST_CASE(Converter, SetUpperBoundsHSV);
    RUN_TEST_CASE(Converter, SetLowerBoundsHSV);
    RUN_TEST_CASE(Converter, SetHSVWrongData);
}