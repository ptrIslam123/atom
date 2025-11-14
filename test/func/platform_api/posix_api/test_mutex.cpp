#include <gtest/gtest.h>

#if defined (__linux)

#include "include/platform_api/posix_api/mutex.h"
#include "include/utils/scoped_guard.h"

#include <vector>
#include <thread>

using namespace atom::platform::posix;
using namespace atom::utils;

TEST(TestClassisMutex, TestLockAndUnlock) {
    constexpr auto THREAD_COUNT{10};
    constexpr auto INCREMENET_COUNT{100};
    std::uint64_t counter{0};
    Mutex mutex;

    auto threadJob = [&mutex, &counter] {
        for (auto i = 0; i < INCREMENET_COUNT; ++i) {
            ScopedGuard lock{[&mutex] { mutex.unlock(); }};
            mutex.lock();
            ++counter;
        }
    };

    std::vector<std::thread> threads;
    threads.reserve(THREAD_COUNT);
    for (auto i = 0; i < THREAD_COUNT; ++i) {
        threads.push_back(std::thread{threadJob});
    }
    for (auto& th : threads) {
        th.join();
    }
    EXPECT_EQ(counter, THREAD_COUNT * INCREMENET_COUNT);
}

TEST(TestClassisMutex, TestTryLock) {
    Mutex mutex;
    EXPECT_TRUE(mutex.tryLock());
    mutex.unlock();
}

TEST(TestClassisMutex, TestReentrant) {
    Mutex mutex;

    mutex.lock();
    EXPECT_FALSE(mutex.tryLock());
    mutex.unlock();
}

#endif
