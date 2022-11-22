#include "stack_test.c"

TEST_GROUP_RUNNER(Stack) {
    RUN_TEST_CASE(Stack, Push);
    RUN_TEST_CASE(Stack, Pop);
}