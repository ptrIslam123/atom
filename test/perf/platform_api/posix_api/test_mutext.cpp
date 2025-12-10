#include <gtest/gtest.h>

#if defined(__linux)

#include "include/platform_api/posix_api/mutex.h"
#include "include/utils/measure_time.h"

#include <chrono>
#include <mutex>
#include <iostream>

using namespace atom::platform::posix;
using namespace atom::utils;

TEST(TestPosixMutex, CompareLockUnlock) {
    using ResolutionType = std::chrono::nanoseconds;

    constexpr auto THREAD_COUNT{8 * 2 * 2};
    constexpr auto INCREMENT_COUNT{1024 * 2 * 2};

    ResolutionType stdMutexTimeAccumulator{};
    ResolutionType posixMutexTimeAccumulator{};

    std::uint64_t counter{0};
    std::mutex stdMutex;
    Mutex posixMutex;

    auto doJobWithStdMutex = [&stdMutex, &stdMutexTimeAccumulator, &counter] {
        for (auto i = 0; i < INCREMENT_COUNT; ++i) {
            stdMutexTimeAccumulator += MeasureTimeNanosec([&] {
                stdMutex.lock();
                ++counter;
                stdMutex.unlock();
            });
        }
    };

    auto doJobWithPosixMutex = [&posixMutex, &posixMutexTimeAccumulator, &counter] {
        for (auto i = 0; i < INCREMENT_COUNT; ++i) {
            posixMutexTimeAccumulator += MeasureTimeNanosec([&] {
                posixMutex.lock();
                ++counter;
                posixMutex.unlock();
            });
        }
    };

    auto runThreads = [&counter](auto doJobCallback) {
        counter = 0;
        std::vector<std::thread> threads;
        threads.reserve(THREAD_COUNT);
        for (auto i = 0; i < THREAD_COUNT; ++i) {
            threads.push_back(std::thread{doJobCallback});
        }

        for (auto& th : threads) {
            th.join();
        }
    };

    runThreads(doJobWithStdMutex);
    runThreads(doJobWithPosixMutex);


    auto totalOperations = THREAD_COUNT * INCREMENT_COUNT;

    auto stdAvgTime = stdMutexTimeAccumulator.count() / totalOperations;
    auto posixAvgTime = posixMutexTimeAccumulator.count() / totalOperations;

    auto stdTotalTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(stdMutexTimeAccumulator).count();
    auto posixTotalTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(posixMutexTimeAccumulator).count();

    std::cout << "\t\t\t\t      === MUTEX PERFORMANCE COMPARISON ===" << std::endl;
    std::cout << "\t\t\t\t        Configuration: " << THREAD_COUNT << " threads, " << INCREMENT_COUNT << " operations/thread" << std::endl;
    std::cout << "\t\t\t\t        Total operations: " << totalOperations << std::endl;
    std::cout << "\t\t\t\t      ----------------------------------------" << std::endl;

    std::cout << "\t\t\t\t      std::mutex results:" << std::endl;
    std::cout << "\t\t\t\t        Total time: " << stdTotalTimeMs << " ms" << std::endl;
    std::cout << "\t\t\t\t        Average lock+unlock: " << stdAvgTime << " ns" << std::endl;
    std::cout << "\t\t\t\t        Throughput: " << (totalOperations * 1000.0 / stdTotalTimeMs) << " ops/sec" << std::endl;

    std::cout << "\t\t\t\t      Posix Mutex results:" << std::endl;
    std::cout << "\t\t\t\t        Total time: " << posixTotalTimeMs << " ms" << std::endl;
    std::cout << "\t\t\t\t        Average lock+unlock: " << posixAvgTime << " ns" << std::endl;
    std::cout << "\t\t\t\t        Throughput: " << (totalOperations * 1000.0 / posixTotalTimeMs) << " ops/sec" << std::endl;

    std::cout << "----------------------------------------" << std::endl;
}

#endif
