#include "dependency/unity/unity_fixture.h"
#include "module/color/converter.h"
#include "module/mocks/runtime_error_stub.h"

TEST_GROUP(Converter);

TEST_SETUP(Converter) {}

TEST_TEAR_DOWN(Converter) {}

TEST(Converter, SetRGB) {
    rgb_t rgb = converter_get_rgb_data(12, 0, 255);
    TEST_ASSERT_EQUAL_HEX8(12, rgb.red);
    TEST_ASSERT_EQUAL_HEX8(0, rgb.green);
    TEST_ASSERT_EQUAL_HEX8(255, rgb.blue);
}

TEST(Converter, SetWrongRGB) {
    rgb_t rgb = converter_get_rgb_data(-1, -1, -1);
    TEST_ASSERT_TRUE(rgb.red <= 255);
    TEST_ASSERT_TRUE(rgb.green <= 255);
    TEST_ASSERT_TRUE(rgb.blue <= 255);
    TEST_ASSERT_EQUAL_HEX8(255, rgb.red);
    TEST_ASSERT_EQUAL_HEX8(255, rgb.green);
    TEST_ASSERT_EQUAL_HEX8(255, rgb.blue);
}

TEST(Converter, FromRGBtoHSV) {
    rgb_t rgb = converter_get_rgb_data(232, 123, 25);
    hsv_t hsv = converter_to_hsv_from_rgb(rgb);
    TEST_ASSERT_EQUAL_HEX16(29, hsv.hue);
    TEST_ASSERT_EQUAL_HEX16(90, hsv.saturation);
    TEST_ASSERT_EQUAL_HEX16(91, hsv.value);
}

TEST(Converter, SetUpperBoundsHSV) {
    hsv_t hsv = converter_get_hsv_data(360, 100, 100);
    TEST_ASSERT_EQUAL_HEX16(360, hsv.hue);
    TEST_ASSERT_EQUAL_HEX16(100, hsv.saturation);
    TEST_ASSERT_EQUAL_HEX16(100, hsv.value);
}

TEST(Converter, SetLowerBoundsHSV) {
    hsv_t hsv = converter_get_hsv_data(0, 0, 0);
    TEST_ASSERT_EQUAL_HEX16(0, hsv.hue);
    TEST_ASSERT_EQUAL_HEX16(0, hsv.saturation);
    TEST_ASSERT_EQUAL_HEX16(0, hsv.value);
}

TEST(Converter, SetHSVWrongData) {
    hsv_t hsv = converter_get_hsv_data(-1, -1, -1);
    TEST_ASSERT_EQUAL_HEX16(360, hsv.hue);
    TEST_ASSERT_EQUAL_HEX16(100, hsv.saturation);
    TEST_ASSERT_EQUAL_HEX16(100, hsv.value);
}
