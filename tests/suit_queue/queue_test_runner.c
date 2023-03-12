#include "queue_test.c"

TEST_GROUP_RUNNER(Queue) {
    RUN_TEST_CASE(Queue, Poll);
    RUN_TEST_CASE(Queue, IsActualEmpty);
    RUN_TEST_CASE(Queue, Remove);
    RUN_TEST_CASE(Queue, FullQueueNotChangingSize);
}