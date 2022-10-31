#include "../../dependency/unity/unity_fixture.h"
#include "../../module/utils/output_utils.h"

#include "nrf_delay.h"
#include "boards.h"
#include "bsp.h"

TEST_GROUP(LedDriver);


TEST_SETUP(LedDriver) {
    bsp_board_init(BSP_INIT_LEDS);
}

TEST_TEAR_DOWN(LedDriver) {
    uint32_t yellow_pin = BSP_LED_YELLOW;
    bsp_board_led_invert(yellow_pin);
}

TEST(LedDriver, LedOn) {
    uint32_t yellow_pin = BSP_LED_YELLOW;
    NRF_GPIO_Type* reg = nrf_gpio_pin_port_decode(&yellow_pin);
    uint32_t before_toggle_outset = reg->OUTSET;
    bsp_board_led_invert(yellow_pin);
    reg = nrf_gpio_pin_port_decode(&yellow_pin);
    uint32_t after_toggle_outset = reg->OUTSET;
    TEST_ASSERT_NOT_EQUAL(before_toggle_outset, after_toggle_outset);
}

TEST(LedDriver, LedOff) {
    uint32_t yellow_pin = BSP_LED_YELLOW;
    bsp_board_led_invert(yellow_pin);
    NRF_GPIO_Type* reg = nrf_gpio_pin_port_decode(&yellow_pin);
    uint32_t after_toggle_outset1 = reg->OUTSET;
    bsp_board_led_invert(yellow_pin);
    uint32_t after_toggle_outset2 = reg->OUTSET;
    TEST_ASSERT_NOT_EQUAL(after_toggle_outset1, after_toggle_outset2);
    bsp_board_led_invert(yellow_pin);
}