#include "suit_blink/blink_test_runner.c"
#include "suit_utils/utils_test_runner.c"
#include "suit_gpio_utils/gpio_utils_test_runner.c"
#include "../../module/utils/output_utils.h"

static void run_all_test(void) {
    RUN_TEST_GROUP(LedDriver);
    RUN_TEST_GROUP(Utils);
    RUN_TEST_GROUP(GPIOUtils);
}

int main(int argc, const char* argv[]) {
    bsp_board_init(BSP_INIT_LEDS);
    int is_passed = !UnityMain(argc, argv, run_all_test);
    while (true) {
        if (is_passed) {
            blink(BSP_LED_GREEN);
        } else {
            blink(BSP_LED_RED);
        }
    }
}