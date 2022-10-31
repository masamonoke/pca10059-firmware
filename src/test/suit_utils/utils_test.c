#include "../../dependency/unity/unity_fixture.h"
#include "../../module/utils/output_utils.h"
#include <time.h>

#include "nrf_delay.h"
#include "boards.h"
#include "bsp.h"

TEST_GROUP(Utils);

TEST_SETUP(Utils) {
    bsp_board_init(BSP_INIT_LEDS);
}

TEST_TEAR_DOWN(Utils) {}

TEST(Utils, Blink) {
    uint32_t yellow_pin = BSP_LED_YELLOW;
    NRF_GPIO_Type* reg = nrf_gpio_pin_port_decode(&yellow_pin);
    uint32_t before = reg->OUTSET;
    blink(yellow_pin);
    uint32_t after = nrf_gpio_pin_port_decode(&yellow_pin)->OUTSET;
    TEST_ASSERT_EQUAL_HEX32(before, after);
}