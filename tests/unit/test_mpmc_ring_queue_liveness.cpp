#include <gtest/gtest.h>

#include "containers/lock-free/mpmc_ring_queue.h"

#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <span>
#include <thread>
#include <utility>
#include <vector>

namespace {

using atom::containers::lock_free::mpmc::RingQueue;
using namespace std::chrono_literals;

struct Msg {
    int value{};
};

struct Counted {
    static inline std::atomic<int> made{0};

    int value{-1};

    Counted() noexcept { made.fetch_add(1); }
    explicit Counted(int v) noexcept : value(v) { made.fetch_add(1); }
    Counted(const Counted& other) noexcept : value(other.value)
    {
        made.fetch_add(1);
    }
    Counted(Counted&& other) noexcept : value(other.value)
    {
        made.fetch_add(1);
    }
    Counted& operator=(const Counted&) noexcept = default;
    Counted& operator=(Counted&&) noexcept = default;
    ~Counted() noexcept = default;
};

TEST(MpmcLiveness, WriteUnblocksWhenConsumerFreesSpace) {
    RingQueue<int> queue{2};
    ASSERT_TRUE(queue.tryWrite(1));
    ASSERT_TRUE(queue.tryWrite(2));

    std::thread producer([&] { queue.write(3); });
    std::this_thread::sleep_for(50ms);

    int value = 0;
    ASSERT_TRUE(queue.tryRead(value));
    EXPECT_EQ(value, 1);

    producer.join();
    ASSERT_TRUE(queue.tryRead(value));
    EXPECT_EQ(value, 2);
    ASSERT_TRUE(queue.tryRead(value));
    EXPECT_EQ(value, 3);
}

TEST(MpmcLiveness, ReadUnblocksWhenProducerPosts) {
    RingQueue<int> queue{2};
    int received = -1;

    std::thread consumer([&] { queue.read(received); });
    std::this_thread::sleep_for(50ms);

    ASSERT_TRUE(queue.tryWrite(9));
    consumer.join();
    EXPECT_EQ(received, 9);
}

TEST(MpmcLiveness, EmplaceUnblocksAndConstructsExactlyOnce) {
    RingQueue<Counted> queue{2};
    ASSERT_TRUE(queue.tryEmplace(0));
    ASSERT_TRUE(queue.tryEmplace(1));

    Counted out;
    Counted::made.store(0);

    std::thread producer([&] { queue.emplace(7); });
    std::this_thread::sleep_for(50ms);

    ASSERT_TRUE(queue.tryRead(out));
    EXPECT_EQ(out.value, 0);

    producer.join();
    EXPECT_EQ(Counted::made.load(), 1);

    ASSERT_TRUE(queue.tryRead(out));
    EXPECT_EQ(out.value, 1);
    ASSERT_TRUE(queue.tryRead(out));
    EXPECT_EQ(out.value, 7);
}

TEST(MpmcLiveness, CopyWriteUnblocksWhenConsumerFreesSpace) {
    RingQueue<Msg> queue{2};
    ASSERT_TRUE(queue.tryWrite(Msg{0}));
    ASSERT_TRUE(queue.tryWrite(Msg{1}));

    std::thread producer([&] {
        const Msg message{2};
        queue.write(message);
    });
    std::this_thread::sleep_for(50ms);

    Msg out;
    ASSERT_TRUE(queue.tryRead(out));
    EXPECT_EQ(out.value, 0);

    producer.join();
    ASSERT_TRUE(queue.tryRead(out));
    EXPECT_EQ(out.value, 1);
    ASSERT_TRUE(queue.tryRead(out));
    EXPECT_EQ(out.value, 2);
}

TEST(MpmcLiveness, BatchWritersSingleReaderKeepPerProducerOrder) {
    constexpr int kProducers = 2;
    constexpr int kBatches = 20000;
    constexpr int kBatchSize = 4;
    constexpr int kPerProducer = kBatches * kBatchSize;
    constexpr int kTotal = kProducers * kPerProducer;

    RingQueue<int> queue{32};
    std::atomic<bool> corrupt{false};
    std::atomic<long> consumed{0};

    std::thread reader([&] {
        std::array<int, kProducers> next{};
        for (int p = 0; p < kProducers; ++p) {
            next[p] = p * kPerProducer;
        }

        long total = 0;
        int value = 0;
        while (total < kTotal) {
            if (!queue.tryRead(value)) {
                std::this_thread::yield();
                continue;
            }
            const int producer = value / kPerProducer;
            if (producer < 0 || producer >= kProducers ||
                value != next[producer]) {
                corrupt.store(true);
            } else {
                ++next[producer];
            }
            ++total;
            consumed.store(total, std::memory_order_relaxed);
        }
    });

    std::vector<std::thread> producers;
    for (int p = 0; p < kProducers; ++p) {
        producers.emplace_back([&, p] {
            for (int batch = 0; batch < kBatches; ++batch) {
                std::array<int, kBatchSize> values{};
                for (int i = 0; i < kBatchSize; ++i) {
                    values[i] = p * kPerProducer + batch * kBatchSize + i;
                }
                while (!queue.tryWrite(std::span<const int>{values})) {
                    std::this_thread::yield();
                }
            }
        });
    }

    for (auto& producer : producers) {
        producer.join();
    }
    reader.join();

    EXPECT_FALSE(corrupt.load());
    EXPECT_EQ(consumed.load(), kTotal);
}

TEST(MpmcLiveness, SingleWritersBatchReadersLoseNothingAndDuplicateNothing) {
    constexpr int kProducers = 2;
    constexpr int kPerProducer = 20000;
    constexpr int kBatchSize = 4;
    constexpr int kTotal = kProducers * kPerProducer;
    constexpr int kConsumers = 3;

    RingQueue<int> queue{64};
    std::vector<std::atomic<int>> seen(kTotal);
    for (auto& slot : seen) {
        slot.store(0, std::memory_order_relaxed);
    }

    std::atomic<int> producersDone{0};
    std::atomic<long> consumed{0};
    std::atomic<bool> corrupt{false};
    std::vector<std::thread> threads;

    for (int p = 0; p < kProducers; ++p) {
        threads.emplace_back([&, p] {
            for (int i = 0; i < kPerProducer; ++i) {
                const int value = p * kPerProducer + i;
                while (!queue.tryWrite(value)) {
                    std::this_thread::yield();
                }
            }
            producersDone.fetch_add(1, std::memory_order_release);
        });
    }

    for (int c = 0; c < kConsumers; ++c) {
        threads.emplace_back([&] {
            std::array<int, kBatchSize> buffer{};
            for (;;) {
                if (queue.tryRead(std::span<int>{buffer})) {
                    for (int value : buffer) {
                        if (value < 0 || value >= kTotal ||
                            seen[static_cast<std::size_t>(value)].fetch_add(
                                1, std::memory_order_relaxed) != 0) {
                            corrupt.store(true);
                        }
                    }
                    consumed.fetch_add(kBatchSize, std::memory_order_relaxed);
                } else if (producersDone.load(std::memory_order_acquire) ==
                               kProducers &&
                           consumed.load(std::memory_order_relaxed) == kTotal) {
                    break;
                } else {
                    std::this_thread::yield();
                }
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_FALSE(corrupt.load());
    EXPECT_EQ(consumed.load(), kTotal);
    for (int value = 0; value < kTotal; ++value) {
        ASSERT_EQ(seen[static_cast<std::size_t>(value)].load(), 1)
            << "value=" << value;
    }
}

} // namespace
