#include <gtest/gtest.h>

#include "include/containers/lock_free/intrusive/spsc_ring_queue.h"

#include <vector>

template<class T, std::size_t N>
using RingQueue = atom::containers::lock_free::spsc::IntrusiveRingQueue<T, N>;

TEST(TestIntrusiveSPSCRingQueue, TestTryEnqueueAndDequeueOneElement) {
    using Queue = RingQueue<int, 128>;
    // Check tryEnqueue and tryDequeue
    {
        Queue queue;
        std::vector<int> data;
        data.resize(64);

        for (auto i = 0; i < data.size(); ++i) {
            data[i] = i;
            EXPECT_TRUE(queue.tryEnqueue(&data[i]));
        }
        EXPECT_EQ(data.size(), queue.size());

        for (auto i = 0; i < data.size(); ++i) {
            int* actual{nullptr};
            EXPECT_TRUE(queue.tryDequeue(&actual));
            ASSERT_NE(actual, nullptr);

            const auto expected{data[i]};
            EXPECT_EQ(*actual, expected);
        }
        EXPECT_TRUE(queue.empty());
    }
    // Check tryEnqueue and tryDequeue order
    {
        Queue queue;
        for (auto i = 0; i < 64; ++i) {
            EXPECT_TRUE(queue.tryEnqueue(&i));
            EXPECT_EQ(queue.size(), 1);
            int* v{nullptr};
            EXPECT_TRUE(queue.tryDequeue(&v));
            ASSERT_NE(v, nullptr);
            EXPECT_EQ(*v, i);
        }
    }
    // Check tryDequeue from empty ring
    {
        Queue queue;
        EXPECT_TRUE(queue.empty());
        int* v{nullptr};
        EXPECT_FALSE(queue.tryDequeue(&v));
    }
    // Check tryEnqueue to full ring
    {
        Queue queue;
        std::array<int, queue.capacity()> buffer{0};
        EXPECT_TRUE(queue.empty());
        for (auto i = 0; i < queue.capacity(); ++i) {
            auto& element = buffer[i];
            element = i;
            EXPECT_TRUE(queue.tryEnqueue(&element));
        }
        int element{0};
        EXPECT_FALSE(queue.tryEnqueue(&element));
    }
    // Check size
    {
        int element{0};
        Queue queue;
        for (auto i = 0; i < 32; ++i) {
            auto prevSize{queue.size()};
            EXPECT_TRUE(queue.tryEnqueue(&element));
            EXPECT_EQ(prevSize + 1, queue.size());
        }
    }
    // Check fifo order
    {
        Queue queue;
        std::vector<int> values = {10, 20, 30, 40, 50};

        // Add 3
        for (int i = 0; i < 3; ++i) {
            queue.tryEnqueue(&values[i]);
        }

        // Remove 1
        int* ptr = nullptr;
        EXPECT_TRUE(queue.tryDequeue(&ptr));
        EXPECT_EQ(*ptr, 10);

        // Add 2 more
        for (int i = 3; i < 5; ++i) {
            queue.tryEnqueue(&values[i]);
        }

        // Now dequeue all 4 remaining
        std::vector<int> expected = {20, 30, 40, 50};
        for (int exp : expected) {
            EXPECT_TRUE(queue.tryDequeue(&ptr));
            EXPECT_EQ(*ptr, exp);
        }
        EXPECT_TRUE(queue.empty());
    }
    {
        using Queue = RingQueue<int, 4>;
        Queue queue;
        int val = 99;

        for (int round = 0; round < 8; ++round) {
            EXPECT_TRUE(queue.tryEnqueue(&val));
            int* ptr = nullptr;
            EXPECT_TRUE(queue.tryDequeue(&ptr));
            EXPECT_EQ(*ptr, 99);
            EXPECT_TRUE(queue.empty());
        }
    }
}

TEST(TestIntrusiveSPSCRingQueue, TestBulkOperations) {
    using Queue = RingQueue<int, 16>;
    constexpr std::size_t CAP = Queue::CAPACITY;

    auto make_data = [](std::size_t n) {
        std::vector<int> v(n);
        for (std::size_t i = 0; i < n; ++i) v[i] = static_cast<int>(i);
        return v;
    };

    // Check tryEnqueueBulk
    {
        Queue queue;
        EXPECT_TRUE(queue.empty());
        auto data = make_data(8);
        EXPECT_TRUE(queue.tryEnqueueBulk(data.data(), data.size()));
        EXPECT_EQ(queue.size(), data.size());
    }

}
