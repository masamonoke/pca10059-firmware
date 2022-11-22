#include "dependency/unity/unity_fixture.h"
#include "module/data_structures/queue.h"

TEST_GROUP(Queue);

static instance_t* s_queue_;

TEST_SETUP(Queue) {
    s_queue_ = queue_ctx_alloc_instance(10);
}

TEST_TEAR_DOWN(Queue) {
    queue_ctx_dealloc_instance(s_queue_);
}

TEST(Queue, Poll) {
    queue_ctx_instance_push(s_queue_, 1);
    queue_ctx_instance_push(s_queue_, 2);
    queue_ctx_instance_push(s_queue_, 3);
    queue_ctx_instance_push(s_queue_, 4);
    queue_ctx_instance_push(s_queue_, 5);
    TEST_ASSERT_EQUAL_HEX16(1, queue_ctx_instance_poll(s_queue_));
    TEST_ASSERT_EQUAL_HEX16(2, queue_ctx_instance_poll(s_queue_));
    TEST_ASSERT_EQUAL_HEX16(3, queue_ctx_instance_poll(s_queue_));
    TEST_ASSERT_EQUAL_HEX16(4, queue_ctx_instance_poll(s_queue_));
    TEST_ASSERT_EQUAL_HEX16(5, queue_ctx_instance_poll(s_queue_));
} 

TEST(Queue, IsActualEmpty) {
    queue_ctx_instance_push(s_queue_, 1);
    queue_ctx_instance_push(s_queue_, 2);
    queue_ctx_instance_push(s_queue_, 3);
    queue_ctx_instance_push(s_queue_, 4);
    queue_ctx_instance_push(s_queue_, 5);
    queue_ctx_instance_poll(s_queue_);
    queue_ctx_instance_poll(s_queue_);
    queue_ctx_instance_poll(s_queue_);
    queue_ctx_instance_poll(s_queue_);
    queue_ctx_instance_poll(s_queue_);
    TEST_ASSERT_TRUE(queue_ctx_is_empty_instance(s_queue_));
}

TEST(Queue, Remove) {
    queue_ctx_instance_push(s_queue_, 1);
    queue_ctx_instance_push(s_queue_, 2);
    queue_ctx_instance_push(s_queue_, 3);
    queue_ctx_instance_push(s_queue_, 4);
    queue_ctx_instance_push(s_queue_, 5);
    queue_ctx_instance_remove_element(s_queue_, 3);
    TEST_ASSERT_EQUAL_HEX16(1, queue_ctx_instance_poll(s_queue_));
    TEST_ASSERT_EQUAL_HEX16(2, queue_ctx_instance_poll(s_queue_));
    TEST_ASSERT_EQUAL_HEX16(4, queue_ctx_instance_poll(s_queue_));
    TEST_ASSERT_EQUAL_HEX16(5, queue_ctx_instance_poll(s_queue_));
}

TEST(Queue, FullQueueNotChangingSize) {
    queue_ctx_instance_push(s_queue_, 1);
    queue_ctx_instance_push(s_queue_, 1);
    queue_ctx_instance_push(s_queue_, 1);
    queue_ctx_instance_push(s_queue_, 1);
    queue_ctx_instance_push(s_queue_, 1);
    queue_ctx_instance_push(s_queue_, 1);
    queue_ctx_instance_push(s_queue_, 1);
    queue_ctx_instance_push(s_queue_, 1);
    queue_ctx_instance_push(s_queue_, 1);
    queue_ctx_instance_push(s_queue_, 1);
    TEST_ASSERT_EQUAL(10, queue_ctx_instance_get_size(s_queue_));
    queue_ctx_instance_push(s_queue_, 1);
    TEST_ASSERT_EQUAL(10, queue_ctx_instance_get_size(s_queue_));
}