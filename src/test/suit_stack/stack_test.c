#include "dependency/unity/unity_fixture.h"
#include "module/data_structures/stack.h"

TEST_GROUP(Stack);

static instance_t* s_stack_;

TEST_SETUP(Stack) {
    s_stack_ = stack_ctx_alloc_instance(10);
}

TEST_TEAR_DOWN(Stack) {
    stack_ctx_dealloc_instance(s_stack_);
}

TEST(Stack, Push) {
    stack_ctx_instance_push(s_stack_, 33);
    uint16_t res = stack_ctx_instance_peek(s_stack_);
    TEST_ASSERT_EQUAL_HEX16(33, res);
}

TEST(Stack, Pop) {
    stack_ctx_instance_push(s_stack_, 33);
    uint16_t res = stack_ctx_instance_pop(s_stack_);
    TEST_ASSERT_EQUAL_HEX16(33, res);
}