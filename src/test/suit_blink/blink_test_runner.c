#include "blink_test.c"

TEST_GROUP_RUNNER(LedDriver) {
    RUN_TEST_CASE(LedDriver, LedOn);
    RUN_TEST_CASE(LedDriver, LedOff);
}