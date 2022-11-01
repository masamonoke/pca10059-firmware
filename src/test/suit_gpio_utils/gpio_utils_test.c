#include "dependency/unity/unity_fixture.h"
#include "module/io/gpio_utils.h"

TEST_GROUP(GPIOUtils);

TEST_SETUP(GPIOUtils) {}

TEST_TEAR_DOWN(GPIOUtils) {}

TEST(GPIOUtils, IsLedOn) {
    gpio_utils_turn_on_led(LED_YELLOW);
    TEST_ASSERT_TRUE(gpio_utils_is_pin_on(LED_YELLOW));
    gpio_utils_turn_off_led(LED_YELLOW);
}

TEST(GPIOUtils, IsLedOff) {
    gpio_utils_turn_on_led(LED_YELLOW);
    gpio_utils_turn_off_led(LED_YELLOW);
    TEST_ASSERT_TRUE(!gpio_utils_is_pin_on(LED_YELLOW));
}

TEST(GPIOUtils, IsButtonPressed) {
    nrf_gpio_cfg_output(BUTTON);
    nrf_gpio_pin_write(BUTTON, 0);
    TEST_ASSERT_TRUE(gpio_utils_is_button_pressed());
}

TEST(GPIOUtils, IsButtonReleased) {
    TEST_ASSERT_TRUE(gpio_utils_is_button_released());
}

TEST(GPIOUtils, LedInvert) {
    gpio_utils_led_invert(LED_YELLOW);
    TEST_ASSERT_TRUE(gpio_utils_is_pin_on(LED_YELLOW));
    gpio_utils_led_invert(LED_YELLOW);
    TEST_ASSERT_TRUE(!gpio_utils_is_pin_on(LED_YELLOW));
}