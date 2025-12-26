#include <gtest/gtest.h>

#include "include/memory/allocators/lock_free/fixed/tagged_memory_pool.h"

using namespace atom::memory::allocator::lock_free::fixed;

TEST(TestLockFreeTaggedMemoryPool, Test) {
    struct Id{};
    TaggedMemoryPool<Id, 1024, 1024> pool;
    auto ptr = pool.allocate();
}

TEST(TestLockFreeTaggedMemoryPool, TestAddressTranslation) {
    using namespace __impl_details;
    for (auto i = 0; i < 1000; ++i) {
        EXPECT_EQ(SlotIndexToSelector(SelectorToSlotIndex(i)), i);
    }
}
