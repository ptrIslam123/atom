#include <gtest/gtest.h>

#include "include/containers/lock_free/mpsc_ring_queue.h"

#include <thread>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>

using namespace atom::containers::lock_free::mpsc;

TEST(TestMPSCRingQueue, MultipleProducersCorrectness) {
    constexpr auto PRODUCERS{16};
    constexpr auto OPERATIONS_PER_PRODUCER{1024};
    constexpr auto TOTAL_OPERATIONS{PRODUCERS * OPERATIONS_PER_PRODUCER};
    constexpr auto QUEUE_SIZE{1024};

    RingQueue<int, QUEUE_SIZE> sharedQueue;

    // Данные для продюсеров (каждый продюсер будет отправлять свой диапазон)
    std::vector<std::vector<int>> producersData(PRODUCERS);
    for (int p = 0; p < PRODUCERS; ++p) {
        for (int i = 0; i < OPERATIONS_PER_PRODUCER; ++i) {
            producersData[p].push_back(p * 10000 + i); // Уникальные ID
        }
    }

    // Данные полученные потребителем
    std::vector<int> consumerData;
    consumerData.reserve(TOTAL_OPERATIONS);

    std::atomic<bool> producersDone{false};
    std::atomic<int> activeProducers{PRODUCERS};

    // Producer job
    auto producerExec = [&](int producerId) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(10, 100);

        for (int value : producersData[producerId]) {
            // Используем tryEnqueue с retry для надежности
            while (!sharedQueue.tryEnqueue(value)) {
                std::this_thread::yield();
            }
            // Случайная задержка для создания конкуренции
            std::this_thread::sleep_for(std::chrono::microseconds{dist(gen)});
        }
        --activeProducers;
    };

    // Consumer job
    auto consumerExec = [&]() {
        int value{0};
        while (consumerData.size() < TOTAL_OPERATIONS) {
            if (sharedQueue.tryDequeue(value)) {
                consumerData.push_back(value);
            } else {
                // Если данные есть, но очередь пуста - ждем
                std::this_thread::yield();
            }
        }
    };

    // Запускаем потребителя
    std::thread consumer{consumerExec};

    // Запускаем продюсеров
    std::vector<std::thread> producers;
    producers.reserve(PRODUCERS);
    for (int p = 0; p < PRODUCERS; ++p) {
        producers.emplace_back(producerExec, p);
    }

    // Ждем завершения продюсеров
    for (auto& t : producers) {
        t.join();
    }

    // Ждем завершения потребителя
    consumer.join();

    // ========================================================================
    // ПРОВЕРКИ
    // ========================================================================

    // 1. Проверяем, что получено правильное количество данных
    EXPECT_EQ(consumerData.size(), TOTAL_OPERATIONS);

    // 2. Проверяем, что все данные от всех продюсеров получены
    // Сортируем для сравнения
    std::vector<int> expectedData;
    expectedData.reserve(TOTAL_OPERATIONS);
    for (const auto& data : producersData) {
        expectedData.insert(expectedData.end(), data.begin(), data.end());
    }
    std::sort(expectedData.begin(), expectedData.end());

    std::vector<int> receivedData = consumerData;
    std::sort(receivedData.begin(), receivedData.end());

    EXPECT_EQ(receivedData, expectedData);

    // 3. Проверяем, что порядок внутри каждого продюсера сохранен
    // Для каждого продюсера проверяем, что его данные пришли в правильном порядке
    for (int p = 0; p < PRODUCERS; ++p) {
        std::vector<int> receivedFromProducer;
        for (int value : consumerData) {
            if (value >= p * 10000 && value < (p + 1) * 10000) {
                receivedFromProducer.push_back(value);
            }
        }

        // Проверяем, что данные этого продюсера пришли в правильном порядке
        for (size_t i = 0; i < receivedFromProducer.size(); ++i) {
            EXPECT_EQ(receivedFromProducer[i], p * 10000 + i)
            << "Producer " << p << " data out of order at position " << i;
        }
    }

    // 4. Очередь должна быть пуста
    EXPECT_TRUE(sharedQueue.empty());
    EXPECT_EQ(sharedQueue.size(), 0);
}
