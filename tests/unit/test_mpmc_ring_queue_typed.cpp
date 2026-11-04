#include <gtest/gtest.h>

#include "containers/lock-free/mpmc_ring_queue.h"

#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <thread>
#include <vector>

namespace {

using atom::containers::lock_free::mpmc::RingQueue;
using namespace std::chrono_literals;

struct Item {
    static inline std::atomic<int> live{0};

    long value{};
    std::byte padding[8]{};

    Item() noexcept { ++live; }
    explicit Item(long v) noexcept : value(v) { ++live; }
    Item(const Item& other) noexcept : value(other.value) { ++live; }
    Item(Item&& other) noexcept : value(other.value) { ++live; }
    Item& operator=(const Item&) noexcept = default;
    Item& operator=(Item&&) noexcept = default;
    ~Item() noexcept { --live; }
};

static_assert(sizeof(Item) == 16);

using ItemQueue = RingQueue<Item>;

} // namespace

TEST(TypedMpmcRingQueueConstructor, AcceptsPowerOfTwoCapacity) {
    EXPECT_THROW(((void)ItemQueue{1}), std::invalid_argument);
    EXPECT_NO_THROW(((void)ItemQueue{2}));
    EXPECT_NO_THROW(((void)ItemQueue{8}));
}

TEST(TypedMpmcRingQueueConstructor, RejectsInvalidCapacity) {
    EXPECT_THROW(((void)ItemQueue{0}), std::invalid_argument);
    EXPECT_THROW(((void)ItemQueue{3}), std::invalid_argument);
    EXPECT_THROW(((void)ItemQueue{24}), std::invalid_argument);
}

TEST(TypedMpmcRingQueueWrite, TryEmplaceAndTryWriteRoundTrip) {
    {
        ItemQueue queue{4};
        ASSERT_EQ(queue.capacity(), 4);

        const Item lvalue{3};
        EXPECT_TRUE(queue.tryEmplace(1));
        EXPECT_TRUE(queue.tryWrite(Item{2}));
        EXPECT_TRUE(queue.tryWrite(lvalue));

        EXPECT_EQ(queue.size(), 3);
        EXPECT_EQ(queue.availableSpace(), 1);

        Item output;
        EXPECT_TRUE(queue.tryRead(output));
        EXPECT_EQ(output.value, 1);
        EXPECT_TRUE(queue.tryRead(output));
        EXPECT_EQ(output.value, 2);
        EXPECT_TRUE(queue.tryRead(output));
        EXPECT_EQ(output.value, 3);
        EXPECT_TRUE(queue.isEmpty());
    }

    EXPECT_EQ(Item::live.load(), 0);
}

TEST(TypedMpmcRingQueueWrite, BulkTryWriteAndTryReadWrapAround) {
    {
        ItemQueue queue{4};
        ASSERT_EQ(queue.capacity(), 4);

        const std::array<Item, 3> first{Item{0}, Item{1}, Item{2}};
        ASSERT_TRUE(queue.tryWrite(std::span<const Item>{first}));
        EXPECT_EQ(queue.size(), 3);

        std::array<Item, 2> prefix{};
        ASSERT_TRUE(queue.tryRead(std::span<Item>{prefix}));
        EXPECT_EQ(prefix[0].value, 0);
        EXPECT_EQ(prefix[1].value, 1);
        EXPECT_EQ(queue.size(), 1);

        const std::array<Item, 2> second{Item{3}, Item{4}};
        ASSERT_TRUE(queue.tryWrite(std::span<const Item>{second}));
        EXPECT_EQ(queue.size(), 3);

        std::array<Item, 3> remainder{};
        ASSERT_TRUE(queue.tryRead(std::span<Item>{remainder}));
        EXPECT_EQ(remainder[0].value, 2);
        EXPECT_EQ(remainder[1].value, 3);
        EXPECT_EQ(remainder[2].value, 4);
        EXPECT_TRUE(queue.isEmpty());
    }

    EXPECT_EQ(Item::live.load(), 0);
}

TEST(TypedMpmcRingQueueWrite, TryWriteFailsWhenFullAndSucceedsAfterRead) {
    {
        ItemQueue queue{4};
        ASSERT_EQ(queue.capacity(), 4);

        ASSERT_TRUE(queue.tryEmplace(0));
        ASSERT_TRUE(queue.tryEmplace(1));
        ASSERT_TRUE(queue.tryEmplace(2));
        ASSERT_TRUE(queue.tryEmplace(3));

        EXPECT_TRUE(queue.isFull());
        EXPECT_EQ(queue.size(), queue.capacity());
        EXPECT_EQ(queue.availableSpace(), 0);
        EXPECT_FALSE(queue.tryWrite(Item{4}));
        EXPECT_FALSE(queue.tryEmplace(5));

        Item output;
        ASSERT_TRUE(queue.tryRead(output));
        EXPECT_EQ(output.value, 0);
        EXPECT_EQ(queue.availableSpace(), 1);

        ASSERT_TRUE(queue.tryWrite(Item{4}));

        std::array<Item, 4> rest{};
        ASSERT_TRUE(queue.tryRead(std::span<Item>{rest}));
        EXPECT_EQ(rest[0].value, 1);
        EXPECT_EQ(rest[1].value, 2);
        EXPECT_EQ(rest[2].value, 3);
        EXPECT_EQ(rest[3].value, 4);
        EXPECT_TRUE(queue.isEmpty());
    }

    EXPECT_EQ(Item::live.load(), 0);
}

TEST(TypedMpmcRingQueueWrite, BulkTryWriteRejectsSpanLargerThanCapacity) {
    {
        ItemQueue queue{4};
        const std::array<Item, 5> tooLarge{};

        EXPECT_FALSE(queue.tryWrite(std::span<const Item>{tooLarge}));
        EXPECT_TRUE(queue.isEmpty());
    }

    EXPECT_EQ(Item::live.load(), 0);
}

TEST(TypedMpmcRingQueueTimeout, ReadReturnsFalseAfterTimeoutWhenEmpty) {
    ItemQueue queue{4};

    Item output;
    const auto start = std::chrono::steady_clock::now();
    const bool result = queue.read(output, 20ms);
    const auto elapsed = std::chrono::steady_clock::now() - start;

    EXPECT_FALSE(result);
    EXPECT_GE(elapsed, 15ms);
    EXPECT_TRUE(queue.isEmpty());
}

TEST(TypedMpmcRingQueueTimeout, WriteReturnsFalseAfterTimeoutWhenFull) {
    {
        ItemQueue queue{4};
        ASSERT_EQ(queue.capacity(), 4);

        for (int i = 0; i < 4; ++i) {
            ASSERT_TRUE(queue.tryEmplace(i));
        }
        ASSERT_TRUE(queue.isFull());

        const Item extra{4};
        const auto start = std::chrono::steady_clock::now();
        const bool result = queue.write(extra, 20ms);
        const auto elapsed = std::chrono::steady_clock::now() - start;

        EXPECT_FALSE(result);
        EXPECT_GE(elapsed, 15ms);
        EXPECT_EQ(queue.size(), queue.capacity());
    }

    EXPECT_EQ(Item::live.load(), 0);
}

TEST(TypedMpmcRingQueueTimeout, TimeoutOperationsCompleteWhenPossible) {
    {
        ItemQueue queue{4};

        ASSERT_TRUE(queue.write(Item{1}, 1s));

        Item output;
        ASSERT_TRUE(queue.read(output, 1s));
        EXPECT_EQ(output.value, 1);
        EXPECT_TRUE(queue.isEmpty());
    }

    EXPECT_EQ(Item::live.load(), 0);
}

TEST(TypedMpmcRingQueueConcurrency, DeliversObjectsExactlyOnce) {
    constexpr int kProducers = 2;
    constexpr int kConsumers = 2;
    constexpr long kPerProducer = 10000;
    constexpr long kTotal = kProducers * kPerProducer;

    {
        ItemQueue queue{8};
        ASSERT_GE(queue.capacity(), 8);

        std::vector<std::atomic<int>> seen(static_cast<std::size_t>(kTotal));
        for (auto& slot : seen) {
            slot.store(0, std::memory_order_relaxed);
        }

        std::atomic<bool> corrupt{false};
        std::atomic<long> consumed{0};
        std::atomic<int> producersDone{0};
        std::vector<std::thread> threads;

        for (int producer = 0; producer < kProducers; ++producer) {
            threads.emplace_back([&, producer] {
                for (long sequence = 0; sequence < kPerProducer; ++sequence) {
                    const long value = producer * kPerProducer + sequence;
                    while (!queue.tryWrite(Item{value})) {
                    }
                }
                producersDone.fetch_add(1, std::memory_order_release);
            });
        }

        for (int consumer = 0; consumer < kConsumers; ++consumer) {
            threads.emplace_back([&] {
                Item item;
                for (;;) {
                    const bool done = producersDone.load(
                                          std::memory_order_acquire) ==
                                      kProducers;
                    if (!queue.tryRead(item)) {
                        if (done) {
                            break;
                        }
                        continue;
                    }
                    if (item.value < 0 || item.value >= kTotal ||
                        seen[static_cast<std::size_t>(item.value)].fetch_add(
                            1, std::memory_order_relaxed) != 0) {
                        corrupt.store(true);
                    }
                    consumed.fetch_add(1);
                }
            });
        }

        for (auto& thread : threads) {
            thread.join();
        }

        EXPECT_FALSE(corrupt.load());
        EXPECT_EQ(consumed.load(), kTotal);
        for (long value = 0; value < kTotal; ++value) {
            ASSERT_EQ(seen[static_cast<std::size_t>(value)].load(), 1)
                << "value=" << value;
        }
    }

    EXPECT_EQ(Item::live.load(), 0);
}
