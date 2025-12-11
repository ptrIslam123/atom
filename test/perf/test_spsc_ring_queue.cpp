#include <gtest/gtest.h>

#include "include/containers/lock_free/queue/fixed/spsc_ring_queue.h"
#include "include/containers/fixed/ring_queue.h"
#include "test/func/exchange_data.h"

#include <atomic>
#include <thread>
#include <vector>
#include <memory>
#include <random>
#include <chrono>
#include <iostream>
#include <iomanip>

using namespace atom::containers;
using namespace atom::containers::lock_free::spsc::fixed;

TEST(TestPerfSPSCRingQueue, Test) {
    using ResolutionType = std::chrono::nanoseconds;
    ResolutionType ringPerfAccumulator{};

    ResolutionType lockedRingProducerPerfAccumulator{};
    ResolutionType lockedRingConsumerPerfAccumulator{};

    ResolutionType spscProducerPerfAccumulator{};
    ResolutionType spscConsumerPerfAccumulator{};

    constexpr auto OPERATIONS{1024 * 64};

    // Locked ring queue
    {
        using ExchangeDataBufferType = std::array<std::byte, ExchangeData::SIZE>;
        fixed::RingQueue<ExchangeDataBufferType, OPERATIONS> queue;
        std::mutex mutex;

        auto ProducerJob = [&queue, &mutex, &lockedRingProducerPerfAccumulator] {
            ExchangeDataBufferType buffer;
            bool result{false};
            for (std::size_t i = 0; i < OPERATIONS; ++i) {
                ExchangeData::Serialize(ExchangeData::Gen(), buffer);
                for (;;) {
                    {
                        std::lock_guard lock{mutex};
                        auto __start = std::chrono::high_resolution_clock::now();
                        if (!queue.isFull()) {
                            queue.enqueue(buffer);
                            result = true;
                        }
                        auto __end = std::chrono::high_resolution_clock::now();
                        lockedRingProducerPerfAccumulator += std::chrono::duration_cast<ResolutionType>(__end - __start);
                    }
                    if (result) {
                        break;
                    }
                    std::this_thread::yield();
                }
            }
        };

        auto ConsumerJob = [&queue, &lockedRingConsumerPerfAccumulator] {
            ExchangeDataBufferType buffer;
            bool result{false};
            for (std::size_t i = 0; i < OPERATIONS; ++i) {
                for (;;) {
                    {
                       auto __start = std::chrono::high_resolution_clock::now();
                       if (!queue.isEmpty()) {
                           buffer = queue.front();
                           queue.dequeue();
                           result = true;
                       }
                       auto __end = std::chrono::high_resolution_clock::now();
                       lockedRingConsumerPerfAccumulator += std::chrono::duration_cast<ResolutionType>(__end - __start);
                    }
                    if (result) {
                        break;
                    }
                    std::this_thread::yield();
                }
                ExchangeData data{};
                ExchangeData::Deserialize(buffer, data);
            }
        };

        std::thread producer{ProducerJob};
        std::thread consumer{ConsumerJob};

        producer.join();
        consumer.join();
    }

    // SPSC ring queue
    {
        auto queue = std::make_shared<RingQueue<OPERATIONS>>();

        auto ProducerJob = [&queue, &spscProducerPerfAccumulator] {
            std::array<std::byte, ExchangeData::SIZE> buffer;
            bool result{};
            for (std::size_t i = 0; i < OPERATIONS; ++i) {
                ExchangeData::Serialize(ExchangeData::Gen(), buffer);
                for (;;) {
                    {
                        auto __start = std::chrono::high_resolution_clock::now();
                        result = queue->tryEnqueue(buffer);
                        auto __end = std::chrono::high_resolution_clock::now();
                        spscProducerPerfAccumulator += std::chrono::duration_cast<ResolutionType>(__end - __start);
                    }
                    if (result) {
                        break;
                    }
                    std::this_thread::yield();
                }
            }
        };

        auto ConsumerJob = [&queue, &spscConsumerPerfAccumulator] {
            std::array<std::byte, ExchangeData::SIZE> buffer;
            bool result{};
            for (std::size_t i = 0; i < OPERATIONS; ++i) {
                for (;;) {
                    {
                       auto __start = std::chrono::high_resolution_clock::now();
                       result = queue->tryDequeue(buffer);
                       auto __end = std::chrono::high_resolution_clock::now();
                       spscConsumerPerfAccumulator += std::chrono::duration_cast<ResolutionType>(__end - __start);
                    }
                    if (result) {
                        break;
                    }
                    std::this_thread::yield();
                }
                ExchangeData data{};
                ExchangeData::Deserialize(buffer, data);
            }
        };

        std::thread producer{ProducerJob};
        std::thread consumer{ConsumerJob};

        producer.join();
        consumer.join();
    }

    const auto averageLockedRingProducerPerf{lockedRingProducerPerfAccumulator / OPERATIONS};
    const auto averageLockedRingConsumerPerf{lockedRingConsumerPerfAccumulator / OPERATIONS};

    const auto avergareSPSCProducerPerf{spscProducerPerfAccumulator / OPERATIONS};
    const auto averageSPSCConsumerPerf{spscConsumerPerfAccumulator / OPERATIONS};


    std::cout << "\t\t\t\t\t\t\t OPERATION: " << OPERATIONS << "\n\n";

    std::cout << "\t\t\t\t\t (locked ring producer) total:  " << lockedRingProducerPerfAccumulator << ", average: " << averageLockedRingProducerPerf <<  "\n";
    std::cout << "\t\t\t\t\t (locked ring consumer) total:  " << lockedRingConsumerPerfAccumulator << ", average: " << averageLockedRingConsumerPerf << "\n";
    std::cout << "\n";

    std::cout << "\t\t\t\t\t (spsc producer) total:         " << spscProducerPerfAccumulator << ", average: " << avergareSPSCProducerPerf <<  "\n";
    std::cout << "\t\t\t\t\t (spsc consumer) total:         " << spscConsumerPerfAccumulator << ", average: " << averageSPSCConsumerPerf << "\n";
    std::cout << "\n";

    const auto producerRatio = static_cast<double>(averageLockedRingProducerPerf.count()) / avergareSPSCProducerPerf.count();
    const auto consumerRatio = static_cast<double>(averageLockedRingConsumerPerf.count()) / averageSPSCConsumerPerf.count();
    const auto totalLocked = lockedRingProducerPerfAccumulator + lockedRingConsumerPerfAccumulator;
    const auto totalSPSC = spscProducerPerfAccumulator + spscConsumerPerfAccumulator;
    const auto totalRatio = static_cast<double>(totalLocked.count()) / totalSPSC.count();

    std::cout << "\t\t\t\t\t\t\t === PERFORMANCE RATIOS ===\n";
    std::cout << "\t\t\t\t\t Producer: SPSC is " << std::fixed << std::setprecision(2) << producerRatio << "x faster\n";
    std::cout << "\t\t\t\t\t Consumer: SPSC is " << std::fixed << std::setprecision(2) << consumerRatio << "x faster\n";
    std::cout << "\t\t\t\t\t Overall:  SPSC is " << std::fixed << std::setprecision(2) << totalRatio << "x faster\n";
    std::cout << "\n";

    std::cout << "\t\t\t\t\t\t\t === PERCENTAGE IMPROVEMENT ===\n";
    std::cout << "\t\t\t\t\t Producer improvement: " << std::fixed << std::setprecision(1) << ((producerRatio - 1) * 100) << "%\n";
    std::cout << "\t\t\t\t\t Consumer improvement: " << std::fixed << std::setprecision(1) << ((consumerRatio - 1) * 100) << "%\n";
    std::cout << "\t\t\t\t\t Overall improvement:  " << std::fixed << std::setprecision(1) << ((totalRatio - 1) * 100) << "%\n";
    std::cout << std::flush;
}
