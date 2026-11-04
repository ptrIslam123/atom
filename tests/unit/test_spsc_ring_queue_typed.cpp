#include <gtest/gtest.h>

#include "containers/lock-free/spsc_ring_queue.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <stdexcept>
#include <span>

namespace {

using atom::containers::lock_free::spsc::RingQueue;
using namespace std::chrono_literals;

struct Item {
    static inline int live = 0;

    long value{};
    std::byte padding[8]{};

    Item() noexcept { ++live; }
    explicit Item(long v) noexcept : value(v) { ++live; }
    Item(const Item& other) noexcept : value(other.value) { ++live; }
    Item(Item&& other) noexcept : value(other.value) { ++live; }
    Item& operator=(const Item&) = default;
    Item& operator=(Item&&) noexcept = default;
    ~Item() noexcept { --live; }
};

static_assert(sizeof(Item) == 16);

using ItemQueue = RingQueue<Item>;

} // namespace

// Verifies that the constructor accepts a non-null, aligned buffer and valid
// power-of-two capacities which are multiples of sizeof(T).
TEST(TypedRingQueueConstructor, AcceptsValidBufferAndCapacity) {
    alignas(Item) std::byte storage[128]{};

    EXPECT_NO_THROW(((void)ItemQueue{storage, sizeof(Item)}));
    EXPECT_NO_THROW(((void)ItemQueue{storage, 2 * sizeof(Item)}));
    EXPECT_NO_THROW(((void)ItemQueue{storage, 8 * sizeof(Item)}));
}

// Verifies that a null buffer is rejected with std::invalid_argument.
TEST(TypedRingQueueConstructor, RejectsNullBuffer) {
    EXPECT_THROW(((void)ItemQueue{nullptr, 64}), std::invalid_argument);
}

// Verifies that a zero capacity is rejected with std::invalid_argument.
TEST(TypedRingQueueConstructor, RejectsZeroCapacity) {
    alignas(Item) std::byte storage[64]{};

    EXPECT_THROW(((void)ItemQueue{storage, 0}), std::invalid_argument);
}

// Verifies that a non-power-of-two capacity is rejected with
// std::invalid_argument.
TEST(TypedRingQueueConstructor, RejectsNonPowerOfTwoCapacity) {
    alignas(Item) std::byte storage[128]{};

    EXPECT_THROW(((void)ItemQueue{storage, 48}), std::invalid_argument);
    EXPECT_THROW(((void)ItemQueue{storage, 96}), std::invalid_argument);
}

// Verifies that a capacity smaller than sizeof(T) is rejected with
// std::invalid_argument.
TEST(TypedRingQueueConstructor, RejectsCapacitySmallerThanElement) {
    alignas(Item) std::byte storage[64]{};

    EXPECT_THROW(((void)ItemQueue{storage, 8}), std::invalid_argument);
}

// Verifies that a buffer which is not aligned for T is rejected with
// std::invalid_argument.
TEST(TypedRingQueueConstructor, RejectsMisalignedBuffer) {
    alignas(Item) std::byte storage[128]{};

    EXPECT_THROW(((void)ItemQueue{storage + 1, 64}), std::invalid_argument);
}

// Verifies that tryEmplace/tryWrite construct objects which tryRead reads back
// unchanged, and that size/availableSpace are updated accordingly.
TEST(TypedRingQueueWrite, TryEmplaceAndTryWriteRoundTrip) {
    {
        alignas(Item) std::byte storage[64]{};
        ItemQueue queue(storage, sizeof storage);
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

    EXPECT_EQ(Item::live, 0);
}

// Verifies that bulk tryWrite/tryRead split objects into two non-contiguous
// spans at the physical ring boundary while preserving element order.
TEST(TypedRingQueueWrite, BulkTryWriteAndTryReadSplitAcrossBoundary) {
    {
        alignas(Item) std::byte storage[64]{};
        ItemQueue queue(storage, sizeof storage);
        ASSERT_EQ(queue.capacity(), 4);

        const std::array<Item, 3> first{Item{0}, Item{1}, Item{2}};
        ASSERT_TRUE(queue.tryWrite(std::span<const Item>{first}));
        EXPECT_EQ(queue.size(), 3);

        std::array<Item, 2> prefix{};
        ASSERT_TRUE(queue.tryRead(std::span<Item>{prefix}));
        EXPECT_EQ(prefix[0].value, 0);
        EXPECT_EQ(prefix[1].value, 1);
        EXPECT_EQ(queue.size(), 1);

        // Writer is at slot 3; two more objects must wrap around the end.
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

    EXPECT_EQ(Item::live, 0);
}

// Verifies that tryEmplace/tryWrite return false when the ring is full, that
// tryWrite never invokes a callback (there is none for objects), and that
// writing succeeds again after the consumer frees space.
TEST(TypedRingQueueWrite, TryWriteFailsWhenFullAndSucceedsAfterRead) {
    {
        alignas(Item) std::byte storage[64]{};
        ItemQueue queue(storage, sizeof storage);
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
        EXPECT_EQ(queue.size(), queue.capacity());

        Item prefix;
        ASSERT_TRUE(queue.tryRead(prefix));
        EXPECT_EQ(prefix.value, 0);
        EXPECT_EQ(queue.availableSpace(), 1);

        ASSERT_TRUE(queue.tryWrite(Item{4}));

        std::array<Item, 4> output{};
        ASSERT_TRUE(queue.tryRead(std::span<Item>{output}));
        EXPECT_EQ(output[0].value, 1);
        EXPECT_EQ(output[1].value, 2);
        EXPECT_EQ(output[2].value, 3);
        EXPECT_EQ(output[3].value, 4);
        EXPECT_TRUE(queue.isEmpty());
    }

    EXPECT_EQ(Item::live, 0);
}

// Verifies that bulk tryWrite rejects a span larger than the queue capacity
// without changing the queue contents.
TEST(TypedRingQueueWrite, BulkTryWriteRejectsSpanLargerThanCapacity) {
    {
        alignas(Item) std::byte storage[64]{};
        ItemQueue queue(storage, sizeof storage);
        ASSERT_EQ(queue.capacity(), 4);

        const std::array<Item, 5> tooLarge{};

        EXPECT_FALSE(queue.tryWrite(std::span<const Item>{tooLarge}));
        EXPECT_TRUE(queue.isEmpty());
        EXPECT_EQ(queue.size(), 0);
    }

    EXPECT_EQ(Item::live, 0);
}

#ifndef NDEBUG
// Verifies (debug builds only) that a blocking bulk write larger than the
// capacity violates the precondition and triggers the assert.
TEST(TypedRingQueueWriteDeathTest, BulkWriteLargerThanCapacityViolatesPrecondition) {
    alignas(Item) std::byte storage[64]{};
    ItemQueue queue(storage, sizeof storage);
    const std::array<Item, 5> tooLarge{};

    EXPECT_DEATH(
        (void)queue.write(std::span<const Item>{tooLarge}),
        "");
}
#endif

// Verifies that typed read with a timeout returns false after the deadline
// when the queue is empty.
TEST(TypedRingQueueTimeout, ReadReturnsFalseAfterTimeoutWhenEmpty) {
    alignas(Item) std::byte storage[64]{};
    ItemQueue queue(storage, sizeof storage);

    Item output;
    const auto start = std::chrono::steady_clock::now();
    const bool result = queue.read(output, 20ms);
    const auto elapsed = std::chrono::steady_clock::now() - start;

    EXPECT_FALSE(result);
    EXPECT_GE(elapsed, 15ms);
    EXPECT_TRUE(queue.isEmpty());
}

// Verifies that typed write with a timeout returns false after the deadline
// when the queue is full without modifying the queued objects.
TEST(TypedRingQueueTimeout, WriteReturnsFalseAfterTimeoutWhenFull) {
    {
        alignas(Item) std::byte storage[64]{};
        ItemQueue queue(storage, sizeof storage);
        ASSERT_EQ(queue.capacity(), 4);

        ASSERT_TRUE(queue.tryEmplace(0));
        ASSERT_TRUE(queue.tryEmplace(1));
        ASSERT_TRUE(queue.tryEmplace(2));
        ASSERT_TRUE(queue.tryEmplace(3));
        ASSERT_TRUE(queue.isFull());

        const Item extra{4};
        const auto start = std::chrono::steady_clock::now();
        const bool result = queue.write(extra, 20ms);
        const auto elapsed = std::chrono::steady_clock::now() - start;

        EXPECT_FALSE(result);
        EXPECT_GE(elapsed, 15ms);
        EXPECT_EQ(queue.size(), queue.capacity());

        std::array<Item, 4> output{};
        ASSERT_TRUE(queue.tryRead(std::span<Item>{output}));
        EXPECT_EQ(output[0].value, 0);
        EXPECT_EQ(output[1].value, 1);
        EXPECT_EQ(output[2].value, 2);
        EXPECT_EQ(output[3].value, 3);
    }

    EXPECT_EQ(Item::live, 0);
}

// Verifies that typed timeout write/read complete successfully when space and
// data are already available.
TEST(TypedRingQueueTimeout, TimeoutOperationsCompleteWhenPossible) {
    {
        alignas(Item) std::byte storage[64]{};
        ItemQueue queue(storage, sizeof storage);

        ASSERT_TRUE(queue.write(Item{1}, 1s));

        Item output;
        ASSERT_TRUE(queue.read(output, 1s));
        EXPECT_EQ(output.value, 1);
        EXPECT_TRUE(queue.isEmpty());
    }

    EXPECT_EQ(Item::live, 0);
}
