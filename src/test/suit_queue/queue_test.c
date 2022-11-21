#include "dependency/unity/unity_fixture.h"
#include "module/datastructures/queue.h"
#include "module/mocks/runtime_error_stub.h"

TEST_GROUP(Queue);

static instance_t* _s_queue;

TEST_SETUP(Queue) {
    _s_queue = queue_ctx_alloc_instance(10);
}

TEST_TEAR_DOWN(Queue) {
    queue_ctx_dealloc_instance(_s_queue);
}

TEST(Queue, Poll) {
    queue_ctx_instance_push(_s_queue, 1);
    queue_ctx_instance_push(_s_queue, 2);
    queue_ctx_instance_push(_s_queue, 3);
    queue_ctx_instance_push(_s_queue, 4);
    queue_ctx_instance_push(_s_queue, 5);
    TEST_ASSERT_EQUAL_HEX16(1, queue_ctx_instance_poll(_s_queue));
    TEST_ASSERT_EQUAL_HEX16(2, queue_ctx_instance_poll(_s_queue));
    TEST_ASSERT_EQUAL_HEX16(3, queue_ctx_instance_poll(_s_queue));
    TEST_ASSERT_EQUAL_HEX16(4, queue_ctx_instance_poll(_s_queue));
    TEST_ASSERT_EQUAL_HEX16(5, queue_ctx_instance_poll(_s_queue));
} 

TEST(Queue, IsActualEmpty) {
    queue_ctx_instance_push(_s_queue, 1);
    queue_ctx_instance_push(_s_queue, 2);
    queue_ctx_instance_push(_s_queue, 3);
    queue_ctx_instance_push(_s_queue, 4);
    queue_ctx_instance_push(_s_queue, 5);
    queue_ctx_instance_poll(_s_queue);
    queue_ctx_instance_poll(_s_queue);
    queue_ctx_instance_poll(_s_queue);
    queue_ctx_instance_poll(_s_queue);
    queue_ctx_instance_poll(_s_queue);
    TEST_ASSERT_TRUE(queue_ctx_is_empty_instance(_s_queue));
}

TEST(Queue, Remove) {
    queue_ctx_instance_push(_s_queue, 1);
    queue_ctx_instance_push(_s_queue, 2);
    queue_ctx_instance_push(_s_queue, 3);
    queue_ctx_instance_push(_s_queue, 4);
    queue_ctx_instance_push(_s_queue, 5);
    queue_ctx_instance_remove_element(_s_queue, 3);
    TEST_ASSERT_EQUAL_HEX16(1, queue_ctx_instance_poll(_s_queue));
    TEST_ASSERT_EQUAL_HEX16(2, queue_ctx_instance_poll(_s_queue));
    TEST_ASSERT_EQUAL_HEX16(4, queue_ctx_instance_poll(_s_queue));
    TEST_ASSERT_EQUAL_HEX16(5, queue_ctx_instance_poll(_s_queue));
}

TEST(Queue, FullQueueNotChangingSize) {
    queue_ctx_instance_push(_s_queue, 1);
    queue_ctx_instance_push(_s_queue, 1);
    queue_ctx_instance_push(_s_queue, 1);
    queue_ctx_instance_push(_s_queue, 1);
    queue_ctx_instance_push(_s_queue, 1);
    queue_ctx_instance_push(_s_queue, 1);
    queue_ctx_instance_push(_s_queue, 1);
    queue_ctx_instance_push(_s_queue, 1);
    queue_ctx_instance_push(_s_queue, 1);
    queue_ctx_instance_push(_s_queue, 1);
    TEST_ASSERT_EQUAL(10, queue_ctx_instance_get_size(_s_queue));
    queue_ctx_instance_push(_s_queue, 1);
    TEST_ASSERT_EQUAL(10, queue_ctx_instance_get_size(_s_queue));
}