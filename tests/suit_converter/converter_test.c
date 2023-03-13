#include "dependency/unity/unity_fixture.h"
#include "modules/color/converter.h"

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

    //magenta
    rgb = converter_get_rgb_data(255, 0, 255);
    hsv = converter_to_hsv_from_rgb(rgb);
    TEST_ASSERT_EQUAL_HEX16(300, hsv.hue);
    TEST_ASSERT_EQUAL_HEX16(100, hsv.saturation);
    TEST_ASSERT_EQUAL_HEX16(100, hsv.value);

    //cyan
    rgb = converter_get_rgb_data(0, 255, 255);
    hsv = converter_to_hsv_from_rgb(rgb);
    TEST_ASSERT_EQUAL_HEX16(180, hsv.hue);
    TEST_ASSERT_EQUAL_HEX16(100, hsv.saturation);
    TEST_ASSERT_EQUAL_HEX16(100, hsv.value);

    //white
    rgb = converter_get_rgb_data(255, 255, 255);
    hsv = converter_to_hsv_from_rgb(rgb);
    TEST_ASSERT_EQUAL_HEX16(0, hsv.hue);
    TEST_ASSERT_EQUAL_HEX16(0, hsv.saturation);
    TEST_ASSERT_EQUAL_HEX16(100, hsv.value);

    rgb = converter_get_rgb_data(0, 139, 139);
    hsv = converter_to_hsv_from_rgb(rgb);
    TEST_ASSERT_EQUAL_HEX16(180, hsv.hue);
    TEST_ASSERT_EQUAL_HEX16(100, hsv.saturation);
    //actually 54.5, but doing ceil
    TEST_ASSERT_EQUAL_HEX16(55, hsv.value);

    rgb = converter_get_rgb_data(45, 235, 34);
    hsv = converter_to_hsv_from_rgb(rgb);
    TEST_ASSERT_EQUAL_HEX16(117, hsv.hue);
    //actually 85.5
    TEST_ASSERT_EQUAL_HEX16(86, hsv.saturation);
    //actually 92.2
    TEST_ASSERT_EQUAL_HEX16(93, hsv.value);

    rgb = converter_get_rgb_data(255, 255, 34);
    hsv = converter_to_hsv_from_rgb(rgb);
    TEST_ASSERT_EQUAL_HEX16(60, hsv.hue);
    TEST_ASSERT_EQUAL_HEX16(87, hsv.saturation);
    TEST_ASSERT_EQUAL_HEX16(100, hsv.value);

    rgb = converter_get_rgb_data(186, 255, 234);
    hsv = converter_to_hsv_from_rgb(rgb);
    TEST_ASSERT_EQUAL_HEX16(162, hsv.hue);
    TEST_ASSERT_EQUAL_HEX16(28, hsv.saturation);
    TEST_ASSERT_EQUAL_HEX16(100, hsv.value);

    rgb = converter_get_rgb_data(255, 255, 0);
    hsv = converter_to_hsv_from_rgb(rgb);
    TEST_ASSERT_EQUAL_HEX16(60, hsv.hue);
    TEST_ASSERT_EQUAL_HEX16(100, hsv.saturation);
    TEST_ASSERT_EQUAL_HEX16(100, hsv.value);
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
