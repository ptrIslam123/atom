#include <gtest/gtest.h>

#include "include/memory/allocators/lock_free/static_memory_pool.h"

#include <vector>
#include <cstdlib>

TEST(TestLockFree, TestAllocator) {
    using namespace atom::memory::allocator::lock_free;
    constexpr auto N = 1024;
    using PoolType = StaticMemoryPool<sizeof(int), N>;
    {
        PoolType pool;
        std::vector<PoolType::Descriptor> descriptors;
        descriptors.reserve(N);

        for (auto i = 0; i < N; ++i) {
            auto mem = pool.allocate();
            ASSERT_NE(mem, nullptr);
            new (&mem.interpretAs<int>()) int{i};
            EXPECT_EQ(mem.interpretAs<int>(), int{i});
            descriptors.push_back(mem);
        }

        for (auto i = 0; i < N; ++i) {
            PoolType::Descriptor mem = descriptors[i];
            pool.deallocate(std::move(mem));
        }
    }
    {
        PoolType pool;
        std::vector<PoolType::Descriptor> d;
        for (auto i = 0; i < 10000; ++i) {
            if (std::rand() % 2 == 0 && d.size() < pool.capacity()) {
                d.push_back(pool.allocate());
                if (d.back() == nullptr) {
                    ASSERT_NE(d.back(), nullptr);
                }
                ASSERT_NE(d.back(), nullptr);
            } else {
                if (d.size() > 0) {
                    pool.deallocate(std::move(d.back()));
                    d.pop_back();
                }
            }
        }
    }
}
