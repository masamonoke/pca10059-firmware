#include "gpio_utils_test.c"

TEST_GROUP_RUNNER(GPIOUtils) {
    RUN_TEST_CASE(GPIOUtils, IsLedOn);
    RUN_TEST_CASE(GPIOUtils, IsLedOff);
    RUN_TEST_CASE(GPIOUtils, IsButtonPressed);
    RUN_TEST_CASE(GPIOUtils, IsButtonReleased);
    RUN_TEST_CASE(GPIOUtils, LedInvert);
}