#include <gtest/gtest.h>

#include "include/containers/lock_free/queue/fixed/spsc_ring_queue.h"
#include "test/func/exchange_data.h"

#include <atomic>
#include <thread>
#include <memory>
#include <random>

using namespace atom::containers::lock_free::fixed::spsc;

TEST(TestSPSCRingQueue, TestTryEnqueueAndTryDequeue) {
    constexpr auto OPERATIONS{1024 * 1024};

    std::atomic<std::size_t> enqueued{0};
    std::atomic<std::size_t> dequeued{0};

    auto queue = std::make_shared<RingQueue<OPERATIONS>>();

    auto ProducerJob = [&queue, &enqueued, &dequeued] {
        std::array<std::byte, ExchangeData::SIZE> buffer;
        for (std::size_t i = 0; i < OPERATIONS; ++i) {
            ExchangeData::Serialize(ExchangeData::Gen(), buffer);
            const auto result = queue->tryEnqueue(buffer);
            EXPECT_TRUE(result);
            if (result)
                enqueued.fetch_add(1);
        }
    };

    auto ConsumerJob = [&queue, &enqueued, &dequeued] {
        std::size_t prevOrder{0};
        std::array<std::byte, ExchangeData::SIZE> buffer;
        for (std::size_t i = 0; i < OPERATIONS; ++i) {
            while (queue->empty()) {
                std::this_thread::yield();
            }
            const auto result = queue->tryDequeue(buffer);
            EXPECT_TRUE(result);

            ExchangeData data{};
            ExchangeData::Deserialize(buffer, data);

            EXPECT_TRUE(ExchangeData::CheckSum(data));
            EXPECT_TRUE(prevOrder <= data.getOrder());
            prevOrder = data.getOrder();
            if (result)
                dequeued.fetch_add(1);
        }
    };


    std::thread producer{ProducerJob};
    std::thread consumer{ConsumerJob};

    producer.join();
    consumer.join();

    EXPECT_EQ(enqueued.load(), OPERATIONS);
    EXPECT_EQ(dequeued.load(), OPERATIONS);
}

TEST(TestSPSCRingQueue, TestTryEnqueueAndTryDequeueWithSmalRingSize) {
    constexpr auto OPERATIONS{1024 * 64};

    std::atomic<std::size_t> enqueued{0};
    std::atomic<std::size_t> dequeued{0};

    auto queue = std::make_shared<RingQueue<static_cast<std::size_t>(OPERATIONS / 2)>>();

    auto ProducerJob = [&queue, &enqueued, &dequeued] {
        std::array<std::byte, ExchangeData::SIZE> buffer;
        for (std::size_t i = 0; i < OPERATIONS; ++i) {
            ExchangeData::Serialize(ExchangeData::Gen(), buffer);
            while (!queue->tryEnqueue(buffer)) {
                std::this_thread::yield();
            }
            enqueued.fetch_add(1);
        }
    };

    auto ConsumerJob = [&queue, &enqueued, &dequeued] {
        std::size_t prevOrder{0};
        std::array<std::byte, ExchangeData::SIZE> buffer;
        for (std::size_t i = 0; i < OPERATIONS; ++i) {
            while (!queue->tryDequeue(buffer)) {
                std::this_thread::yield();
            }

            ExchangeData data{};
            ExchangeData::Deserialize(buffer, data);
            EXPECT_TRUE(ExchangeData::CheckSum(data));

            EXPECT_TRUE(prevOrder <= data.getOrder());
            prevOrder = data.getOrder();
            dequeued.fetch_add(1);
        }
    };


    std::thread producer{ProducerJob};
    std::thread consumer{ConsumerJob};

    producer.join();
    consumer.join();

    EXPECT_EQ(enqueued.load(), OPERATIONS);
    EXPECT_EQ(dequeued.load(), OPERATIONS);
}


TEST(TestSPSCRingQueue, CorrectnessUnderConsumerContention) {
    using DelayResolutionType = std::chrono::nanoseconds;
    constexpr auto OPERATIONS{1024 * 64};

    constexpr auto PRODUCER_MIN_DELAY{0};
    constexpr auto PRODUCER_MAX_DELAY{100};

    constexpr auto CONSUMER_MIN_DELAY{0};
    constexpr auto CONSUMER_MAX_DELAY{PRODUCER_MAX_DELAY * 5};

    std::atomic<std::size_t> enqueued{0};
    std::atomic<std::size_t> dequeued{0};

    auto queue = std::make_shared<RingQueue<static_cast<std::size_t>(OPERATIONS / 2)>>();

    auto ProducerJob = [&queue, &enqueued, &dequeued] {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(PRODUCER_MIN_DELAY, PRODUCER_MAX_DELAY);

        std::array<std::byte, ExchangeData::SIZE> buffer;
        for (std::size_t i = 0; i < OPERATIONS; ++i) {
            ExchangeData::Serialize(ExchangeData::Gen(), buffer);
            while (!queue->tryEnqueue(buffer)) {
                std::this_thread::yield();
            }
            enqueued.fetch_add(1);
            std::this_thread::sleep_for(DelayResolutionType{dist(gen)});
        }
    };

    auto ConsumerJob = [&queue, &enqueued, &dequeued] {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(CONSUMER_MIN_DELAY, CONSUMER_MAX_DELAY);

        std::size_t prevOrder{0};
        std::array<std::byte, ExchangeData::SIZE> buffer;
        for (std::size_t i = 0; i < OPERATIONS; ++i) {
            while (!queue->tryDequeue(buffer)) {
                std::this_thread::yield();
            }

            ExchangeData data{};
            ExchangeData::Deserialize(buffer, data);
            EXPECT_TRUE(ExchangeData::CheckSum(data));

            EXPECT_TRUE(prevOrder <= data.getOrder());
            prevOrder = data.getOrder();
            dequeued.fetch_add(1);
            std::this_thread::sleep_for(DelayResolutionType{dist(gen)});
        }
    };


    std::thread producer{ProducerJob};
    std::thread consumer{ConsumerJob};

    producer.join();
    consumer.join();

    EXPECT_EQ(enqueued.load(), OPERATIONS);
    EXPECT_EQ(dequeued.load(), OPERATIONS);
}


TEST(TestSPSCRingQueue, CorrectnessUnderProducerContention) {
    using DelayResolutionType = std::chrono::nanoseconds;
    constexpr auto OPERATIONS{1024 * 64};

    constexpr auto CONSUMER_MIN_DELAY{0};
    constexpr auto CONSUMER_MAX_DELAY{100};

    constexpr auto PRODUCER_MIN_DELAY{0};
    constexpr auto PRODUCER_MAX_DELAY{CONSUMER_MAX_DELAY * 5};

    std::atomic<std::size_t> enqueued{0};
    std::atomic<std::size_t> dequeued{0};

    auto queue = std::make_shared<RingQueue<static_cast<std::size_t>(OPERATIONS / 2)>>();

    auto ProducerJob = [&queue, &enqueued, &dequeued] {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(PRODUCER_MIN_DELAY, PRODUCER_MAX_DELAY);

        std::array<std::byte, ExchangeData::SIZE> buffer;
        for (std::size_t i = 0; i < OPERATIONS; ++i) {
            ExchangeData::Serialize(ExchangeData::Gen(), buffer);
            while (!queue->tryEnqueue(buffer)) {
                std::this_thread::yield();
            }
            enqueued.fetch_add(1);
            std::this_thread::sleep_for(DelayResolutionType{dist(gen)});
        }
    };

    auto ConsumerJob = [&queue, &enqueued, &dequeued] {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(CONSUMER_MIN_DELAY, CONSUMER_MAX_DELAY);

        std::size_t prevOrder{0};
        std::array<std::byte, ExchangeData::SIZE> buffer;
        for (std::size_t i = 0; i < OPERATIONS; ++i) {
            while (!queue->tryDequeue(buffer)) {
                std::this_thread::yield();
            }

            ExchangeData data{};
            ExchangeData::Deserialize(buffer, data);
            EXPECT_TRUE(ExchangeData::CheckSum(data));

            EXPECT_TRUE(prevOrder <= data.getOrder());
            prevOrder = data.getOrder();
            dequeued.fetch_add(1);
            std::this_thread::sleep_for(DelayResolutionType{dist(gen)});
        }
    };


    std::thread producer{ProducerJob};
    std::thread consumer{ConsumerJob};

    producer.join();
    consumer.join();

    EXPECT_EQ(enqueued.load(), OPERATIONS);
    EXPECT_EQ(dequeued.load(), OPERATIONS);
}
