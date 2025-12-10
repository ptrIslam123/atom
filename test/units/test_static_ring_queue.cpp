#include <gtest/gtest.h>

#include "include/containers/fixed/ring_queue.h"

using namespace atom::containers::fixed;
using RingQueueType = RingQueue<int, 1024>;

TEST(StaticRingQueueTest, InitialState) {
    RingQueueType queue;
    EXPECT_TRUE(queue.isEmpty());
    EXPECT_FALSE(queue.isFull());
    EXPECT_EQ(queue.size(), 0);
}

TEST(StaticRingQueueTest, TestEnqueueAndDequeue) {
    RingQueueType queue;
    queue.enqueue(int{10});
    EXPECT_EQ(queue.size(), 1);
    EXPECT_EQ(queue.front(), int{10});
    EXPECT_EQ(queue.back(), int{10});

    queue.enqueue(int{20});
    EXPECT_EQ(queue.size(), 2);
    EXPECT_EQ(queue.front(), int{10});
    EXPECT_EQ(queue.back(), int{20});

    queue.dequeue();
    EXPECT_EQ(queue.size(), 1);
    EXPECT_EQ(queue.front(), int{20});
    EXPECT_EQ(queue.back(), int{20});

    queue.dequeue();
    EXPECT_TRUE(queue.isEmpty());
    EXPECT_ANY_THROW(queue.dequeue());
}

TEST(StaticRingQueueTest, TestCapacity) {
    RingQueue<int, 3> queue;
    queue.enqueue(int{10});
    EXPECT_EQ(queue.size(), 1);
    EXPECT_EQ(queue.front(), int{10});
    EXPECT_EQ(queue.back(), int{10});
    EXPECT_FALSE(queue.isFull());

    queue.enqueue(int{20});
    EXPECT_EQ(queue.size(), 2);
    EXPECT_EQ(queue.front(), int{10});
    EXPECT_EQ(queue.back(), int{20});
    EXPECT_FALSE(queue.isFull());

    queue.enqueue(int{30});
    EXPECT_EQ(queue.size(), 3);
    EXPECT_EQ(queue.front(), int{10});
    EXPECT_EQ(queue.back(), int{30});
    EXPECT_TRUE(queue.isFull());

    // rewrite the most old element in queue
    queue.enqueue(int{40});
    EXPECT_EQ(queue.size(), 3);
    EXPECT_EQ(queue.front(), int{20});
    EXPECT_EQ(queue.back(), int{40});
}

TEST(StaticRingQueueTest, EmptyQueueExceptions) {
    RingQueue<int, 2> queue;
    EXPECT_THROW(queue.front(), std::runtime_error);
    EXPECT_THROW(queue.back(), std::runtime_error);
    EXPECT_THROW(queue.dequeue(), std::runtime_error);
}

TEST(StaticRingQueueTest, MoveSemantics) {
    RingQueue<std::unique_ptr<int>, 2> queue;
    auto ptr = std::make_unique<int>(42);
    queue.enqueue(std::move(ptr));
    EXPECT_EQ(ptr, nullptr);
    auto front = std::move(queue.front());
    EXPECT_EQ(*front, 42);
}

TEST(StaticRingQueueTest, ContinuousOverwrite) {
    RingQueue<int, 3> queue;
    for (int i = 0; i < 10; ++i) {
        queue.enqueue(i);
    }
    // After enqueue 10 elemenst we are expected 7, 8, 9
    EXPECT_EQ(queue.front(), 7);
    EXPECT_EQ(queue.back(), 9);
}


TEST(StaticRingQueueTest, TestWithNonTrivialType) {
    static int counter = 0;
    struct Foo {
        explicit Foo(int _id): id(_id) { ++counter; }
        ~Foo() { --counter; }
        int id;
    };
    RingQueue<Foo, 3> queue;
    queue.emplace(int{0});
    EXPECT_EQ(queue.size(), 1);
    EXPECT_EQ(counter, 1);

    queue.dequeue();
    EXPECT_EQ(queue.size(), 0);
    EXPECT_EQ(counter, 0);
}

TEST(StaticRingQueueTest, TestCopyAndMoveQueues) {
    {
        RingQueue<int, 3> queue1, queue2;
        queue1.enqueue(int{10});
        queue1.enqueue(int{20});
        EXPECT_EQ(queue1.size(), 2);
        EXPECT_EQ(queue1.front(), int{10});
        EXPECT_EQ(queue1.back(), int{20});

        queue2 = queue1;
        EXPECT_EQ(queue2.size(), 2);
        EXPECT_EQ(queue2.front(), int{10});
        EXPECT_EQ(queue2.back(), int{20});
    }
    {
        RingQueue<std::string, 3> queue1, queue2;
        queue1.enqueue(std::string{"str1"});
        queue1.enqueue(std::string{"str2"});
        EXPECT_EQ(queue1.size(), 2);
        EXPECT_EQ(queue1.front(), std::string{"str1"});
        EXPECT_EQ(queue1.back(), std::string{"str2"});

        queue2 = queue1;
        EXPECT_EQ(queue2.size(), 2);
        EXPECT_EQ(queue2.front(), std::string{"str1"});
        EXPECT_EQ(queue2.back(), std::string{"str2"});
    }
    {
        RingQueue<std::unique_ptr<int>, 3> queue1, queue2;
        queue1.enqueue(std::make_unique<int>(int{10}));
        queue1.enqueue(std::make_unique<int>(int{20}));
        EXPECT_EQ(queue1.size(), 2);
        EXPECT_EQ(*queue1.front(), int{10});
        EXPECT_EQ(*queue1.back(), int{20});

        queue2 = std::move(queue1);
        EXPECT_EQ(queue2.size(), 2);
        EXPECT_EQ(*queue2.front(), int{10});
        EXPECT_EQ(*queue2.back(), int{20});
    }
}
