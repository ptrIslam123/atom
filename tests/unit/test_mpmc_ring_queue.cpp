#include <gtest/gtest.h>

#include "containers/lock-free/mpmc_ring_queue.h"

#include <atomic>
#include <chrono>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <thread>
#include <vector>

namespace {

using atom::containers::lock_free::mpmc::RingQueue;
using ByteQueue = RingQueue<std::byte>;
using namespace std::chrono_literals;

} // namespace

TEST(MpmcRingQueueConstructor, RejectsInvalidCapacity) {
    EXPECT_THROW(((void)ByteQueue{0}), std::invalid_argument);
    EXPECT_THROW(((void)ByteQueue{1}), std::invalid_argument);
    EXPECT_THROW(((void)ByteQueue{3}), std::invalid_argument);
    EXPECT_THROW(((void)ByteQueue{24}), std::invalid_argument);
    EXPECT_NO_THROW(((void)ByteQueue{2}));
    EXPECT_NO_THROW(((void)ByteQueue{8}));
}

TEST(MpmcRingQueueWrite, SingleAndBulkRoundTrip) {
    ByteQueue queue{8};

    EXPECT_TRUE(queue.tryWrite(std::byte{1}));
    EXPECT_TRUE(queue.tryWrite(std::byte{2}));

    const std::byte bulk[]{std::byte{3}, std::byte{4}, std::byte{5}};
    ASSERT_TRUE(queue.tryWrite(std::span<const std::byte>{bulk}));
    EXPECT_EQ(queue.size(), 5);

    std::byte first{};
    ASSERT_TRUE(queue.tryRead(first));
    EXPECT_EQ(first, std::byte{1});

    std::byte rest[4]{};
    ASSERT_TRUE(queue.tryRead(std::span<std::byte>{rest}));
    EXPECT_EQ(rest[0], std::byte{2});
    EXPECT_EQ(rest[1], std::byte{3});
    EXPECT_EQ(rest[2], std::byte{4});
    EXPECT_EQ(rest[3], std::byte{5});
    EXPECT_TRUE(queue.isEmpty());
}

TEST(MpmcRingQueueWrite, RejectsBulkLargerThanCapacity) {
    ByteQueue queue{4};
    const std::byte tooLarge[5]{};

    EXPECT_FALSE(queue.tryWrite(std::span<const std::byte>{tooLarge}));
    EXPECT_TRUE(queue.isEmpty());
}

TEST(MpmcRingQueueMetrics, StayWithinCapacityUnderConcurrentChurn) {
    constexpr std::size_t kCapacity = 64;
    ByteQueue queue{kCapacity};

    std::atomic<bool> stop{false};
    std::atomic<bool> outOfRange{false};
    std::vector<std::thread> threads;

    for (int i = 0; i < 2; ++i) {
        threads.emplace_back([&] {
            const std::byte chunk[8]{};
            while (!stop.load(std::memory_order_relaxed)) {
                (void)queue.tryWrite(std::span<const std::byte>{chunk});
            }
        });
        threads.emplace_back([&] {
            std::byte out[8]{};
            while (!stop.load(std::memory_order_relaxed)) {
                (void)queue.tryRead(std::span<std::byte>{out});
            }
        });
    }

    threads.emplace_back([&] {
        while (!stop.load(std::memory_order_relaxed)) {
            const auto size = queue.size();
            const auto space = queue.availableSpace();
            if (size > kCapacity || space > kCapacity) {
                outOfRange.store(true);
            }
        }
    });

    std::this_thread::sleep_for(250ms);
    stop.store(true);
    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_FALSE(outOfRange.load());
}
