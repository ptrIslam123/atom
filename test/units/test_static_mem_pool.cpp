#include <gtest/gtest.h>
#include <memory>

#include "include/memory/allocators/lock_free/static_memory_pool.h"

using namespace atom::memory::allocator::lock_free;

constexpr std::size_t ID = 0;
constexpr std::size_t BLOCK_SIZE = 1024;
constexpr std::size_t CAPACITY = 1024 * 1024;
constexpr std::size_t CACHE_SIZE = static_cast<std::size_t>(CAPACITY / 4);
using StaticMemPool = StaticMemoryPool<ID, BLOCK_SIZE, CAPACITY, CACHE_SIZE>;

TEST(TestStaticMemPool, SimpleTest) {
    std::byte* mem = nullptr;
    auto pool = std::make_unique<StaticMemPool>();

    for (auto i = 0; i < 10; ++i) {
        ASSERT_NO_THROW(mem = pool->allocate());
        ASSERT_NE(mem, nullptr);
        ASSERT_NO_THROW(pool->deallocate(mem));
    }
}
