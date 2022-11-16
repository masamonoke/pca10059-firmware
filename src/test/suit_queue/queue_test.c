#include "dependency/unity/unity_fixture.h"
#include "module/datastructures/queue.h"
#include "module/mocks/runtime_error_stub.h"

TEST_GROUP(Queue);

static queue_instance_t* _s_queue;

TEST_SETUP(Queue) {
    queue_init();
    _s_queue = queue_alloc(10);
}

TEST_TEAR_DOWN(Queue) {
    queue_dealloc(_s_queue);
    queue_free();
}

TEST(Queue, Poll) {
    queue_push(_s_queue, 1);
    queue_push(_s_queue, 2);
    queue_push(_s_queue, 3);
    queue_push(_s_queue, 4);
    queue_push(_s_queue, 5);
    TEST_ASSERT_EQUAL_HEX16(1, queue_poll(_s_queue));
    TEST_ASSERT_EQUAL_HEX16(2, queue_poll(_s_queue));
    TEST_ASSERT_EQUAL_HEX16(3, queue_poll(_s_queue));
    TEST_ASSERT_EQUAL_HEX16(4, queue_poll(_s_queue));
    TEST_ASSERT_EQUAL_HEX16(5, queue_poll(_s_queue));
} 

TEST(Queue, IsActualEmpty) {
    queue_push(_s_queue, 1);
    queue_push(_s_queue, 2);
    queue_push(_s_queue, 3);
    queue_push(_s_queue, 4);
    queue_push(_s_queue, 5);
    queue_poll(_s_queue);
    queue_poll(_s_queue);
    queue_poll(_s_queue);
    queue_poll(_s_queue);
    queue_poll(_s_queue);
    TEST_ASSERT_TRUE(queue_is_empty(_s_queue));
}

TEST(Queue, Remove) {
    queue_push(_s_queue, 1);
    queue_push(_s_queue, 2);
    queue_push(_s_queue, 3);
    queue_push(_s_queue, 4);
    queue_push(_s_queue, 5);
    queue_remove(_s_queue, 3);
    TEST_ASSERT_EQUAL_HEX16(1, queue_poll(_s_queue));
    TEST_ASSERT_EQUAL_HEX16(2, queue_poll(_s_queue));
    TEST_ASSERT_EQUAL_HEX16(4, queue_poll(_s_queue));
    TEST_ASSERT_EQUAL_HEX16(5, queue_poll(_s_queue));
}

TEST(Queue, FullQueueNotChangingSize) {
    queue_push(_s_queue, 1);
    queue_push(_s_queue, 1);
    queue_push(_s_queue, 1);
    queue_push(_s_queue, 1);
    queue_push(_s_queue, 1);
    queue_push(_s_queue, 1);
    queue_push(_s_queue, 1);
    queue_push(_s_queue, 1);
    queue_push(_s_queue, 1);
    queue_push(_s_queue, 1);
    TEST_ASSERT_EQUAL(10, queue_get_size(_s_queue));
    queue_push(_s_queue, 1);
    TEST_ASSERT_EQUAL(10, queue_get_size(_s_queue));
}

TEST(Queue, NewContextInitThrowsError) {
    queue_init();
    TEST_ASSERT_EQUAL_STRING("Queue context is already initialized", runtime_error_stub_get_last_error());
    TEST_ASSERT_EQUAL(-1, runtime_error_stub_get_last_parameter());
}