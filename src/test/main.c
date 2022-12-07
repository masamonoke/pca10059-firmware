#include "suit_gpio_utils/gpio_utils_test_runner.c"
#include "suit_converter/converter_test_runner.c"
#include "suit_queue/queue_test_runner.c"
#include "suit_stack/stack_test_runner.c"
#include "suit_string_utils/string_utils_test_runner.c"
#include "suit_math_utils/math_utils_test_runner.c"

#include "module/io/gpio_utils.h"
#include "module/log/nordic_usb_logging.h"

#include "app_timer.h"

#define DELAY_MS 250

static void run_all_test(void) {
    RUN_TEST_GROUP(GPIOUtils);
    RUN_TEST_GROUP(Converter);
    RUN_TEST_GROUP(Queue);
    RUN_TEST_GROUP(Stack);
    RUN_TEST_GROUP(StringUtils);
    RUN_TEST_GROUP(MathUtils);
}

int main(int argc, const char* argv[]) {
    app_timer_init();
    gpio_utils_init();
    nordic_usb_logging_init();
    //invert of result because function returns 0 when all tests passed
    int is_passed = !UnityMain(argc, argv, run_all_test);
    
    while (true) {
        if (is_passed) {
            gpio_utils_blink_async(LED_GREEN, DELAY_MS);
        } else {
            gpio_utils_blink_async(LED_RED, DELAY_MS);
        }

        nordic_usb_logging_process();
    }
}