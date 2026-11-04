#include <gtest/gtest.h>

#include "containers/lock-free/spsc_ring_queue.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
#include <stdexcept>

namespace {

using atom::containers::lock_free::spsc::RingQueue;
using ByteQueue = RingQueue<std::byte>;
using namespace std::chrono_literals;

bool enqueue(ByteQueue& queue, std::span<const std::byte> data) {
    return queue.tryWrite(
        data.size(),
        [data](std::span<std::byte> first,
               std::span<std::byte> second) noexcept {
            std::memcpy(first.data(), data.data(), first.size());
            std::memcpy(second.data(), data.data() + first.size(),
                        second.size());
        });
}

bool dequeue(ByteQueue& queue, std::span<std::byte> data) {
    return queue.tryRead(
        data.size(),
        [data](std::span<std::byte> first,
               std::span<std::byte> second) noexcept {
            std::memcpy(data.data(), first.data(), first.size());
            std::memcpy(data.data() + first.size(), second.data(),
                        second.size());
        });
}

} // namespace

// Verifies that the constructor accepts a non-null buffer and every valid
// power-of-two capacity.
TEST(ByteRingQueueConstructor, AcceptsValidBufferAndCapacity) {
    alignas(64) std::byte storage[64]{};

    EXPECT_NO_THROW(((void)ByteQueue{storage, 1}));
    EXPECT_NO_THROW(((void)ByteQueue{storage, 2}));
    EXPECT_NO_THROW(((void)ByteQueue{storage, 8}));
    EXPECT_NO_THROW(((void)ByteQueue{storage, sizeof storage}));
}

// Verifies that a null buffer is rejected with std::invalid_argument.
TEST(ByteRingQueueConstructor, RejectsNullBuffer) {
    EXPECT_THROW(((void)ByteQueue{nullptr, 8}), std::invalid_argument);
}

// Verifies that a zero capacity is rejected with std::invalid_argument.
TEST(ByteRingQueueConstructor, RejectsZeroCapacity) {
    alignas(64) std::byte storage[8]{};

    EXPECT_THROW(((void)ByteQueue{storage, 0}), std::invalid_argument);
}

// Verifies that non-power-of-two capacities are rejected with
// std::invalid_argument.
TEST(ByteRingQueueConstructor, RejectsNonPowerOfTwoCapacity) {
    alignas(64) std::byte storage[64]{};

    EXPECT_THROW(((void)ByteQueue{storage, 3}), std::invalid_argument);
    EXPECT_THROW(((void)ByteQueue{storage, 7}), std::invalid_argument);
    EXPECT_THROW(((void)ByteQueue{storage, 12}), std::invalid_argument);
    EXPECT_THROW(((void)ByteQueue{storage, 63}), std::invalid_argument);
}

// Verifies that tryWrite(n, func) stores bytes which tryRead reads back
// unchanged, and that size/availableSpace are updated accordingly.
TEST(ByteRingQueueWrite, TryWriteRoundTrip) {
    alignas(64) std::byte storage[16]{};
    ByteQueue queue(storage, sizeof storage);

    const std::array<std::byte, 5> input{
        std::byte{0x10}, std::byte{0x20}, std::byte{0x30},
        std::byte{0x40}, std::byte{0x50}};

    ASSERT_TRUE(enqueue(queue, input));
    EXPECT_EQ(queue.size(), input.size());
    EXPECT_EQ(queue.availableSpace(), sizeof storage - input.size());

    std::array<std::byte, 5> output{};
    ASSERT_TRUE(dequeue(queue, output));

    EXPECT_TRUE(std::equal(input.begin(), input.end(), output.begin()));
    EXPECT_TRUE(queue.isEmpty());
}

// Verifies that tryWrite and tryRead split data into two spans when the
// operation crosses the physical ring boundary, preserving byte order.
TEST(ByteRingQueueWrite, TryWriteAndTryReadCanSplitAcrossBoundary) {
    alignas(64) std::byte storage[8]{};
    ByteQueue queue(storage, sizeof storage);

    const std::array<std::byte, 6> firstWrite{
        std::byte{1}, std::byte{2}, std::byte{3},
        std::byte{4}, std::byte{5}, std::byte{6}};
    ASSERT_TRUE(enqueue(queue, firstWrite));

    std::array<std::byte, 4> prefix{};
    ASSERT_TRUE(dequeue(queue, prefix));

    const std::array<std::byte, 5> secondWrite{
        std::byte{7}, std::byte{8}, std::byte{9},
        std::byte{10}, std::byte{11}};

    std::size_t firstSize = 0;
    std::size_t secondSize = 0;
    ASSERT_TRUE(queue.tryWrite(
        secondWrite.size(),
        [&](std::span<std::byte> first,
            std::span<std::byte> second) noexcept {
            firstSize = first.size();
            secondSize = second.size();
            std::memcpy(first.data(), secondWrite.data(), first.size());
            std::memcpy(second.data(), secondWrite.data() + first.size(),
                        second.size());
        }));

    EXPECT_EQ(firstSize, 2);
    EXPECT_EQ(secondSize, 3);

    const std::array<std::byte, 7> expected{
        firstWrite[4], firstWrite[5], secondWrite[0], secondWrite[1],
        secondWrite[2], secondWrite[3], secondWrite[4]};

    std::array<std::byte, 7> output{};
    ASSERT_TRUE(dequeue(queue, output));

    EXPECT_TRUE(std::equal(expected.begin(), expected.end(), output.begin()));
    EXPECT_TRUE(queue.isEmpty());
}

// Verifies that tryWrite returns false and does not invoke the callback when
// the ring is full, and succeeds again after the consumer frees space.
TEST(ByteRingQueueWrite, TryWriteFailsWhenFullAndSucceedsAfterRead) {
    alignas(64) std::byte storage[8]{};
    ByteQueue queue(storage, sizeof storage);

    std::array<std::byte, 8> input{};
    for (std::size_t i = 0; i < input.size(); ++i) {
        input[i] = static_cast<std::byte>(i + 1);
    }

    ASSERT_TRUE(enqueue(queue, input));
    EXPECT_TRUE(queue.isFull());
    EXPECT_EQ(queue.size(), input.size());
    EXPECT_EQ(queue.availableSpace(), 0);

    bool callbackInvoked = false;
    EXPECT_FALSE(queue.tryWrite(
        1,
        [&](std::span<std::byte> first,
            std::span<std::byte> second) noexcept {
            callbackInvoked = true;
            first[0] = std::byte{0xff};
            (void)second;
        }));
    EXPECT_FALSE(callbackInvoked);
    EXPECT_EQ(queue.size(), input.size());

    std::array<std::byte, 4> prefix{};
    ASSERT_TRUE(dequeue(queue, prefix));
    EXPECT_FALSE(queue.isFull());
    EXPECT_EQ(queue.availableSpace(), prefix.size());

    const std::array<std::byte, 4> suffix{
        std::byte{21}, std::byte{22}, std::byte{23}, std::byte{24}};
    ASSERT_TRUE(enqueue(queue, suffix));

    std::array<std::byte, 8> output{};
    ASSERT_TRUE(dequeue(queue, output));

    const std::array<std::byte, 8> expected{
        std::byte{5}, std::byte{6}, std::byte{7}, std::byte{8},
        std::byte{21}, std::byte{22}, std::byte{23}, std::byte{24}};
    EXPECT_TRUE(std::equal(expected.begin(), expected.end(), output.begin()));
    EXPECT_TRUE(queue.isEmpty());
}

#ifndef NDEBUG
// Verifies (debug builds only) that writing more than capacity violates the
// precondition and triggers the assert.
TEST(ByteRingQueueWriteDeathTest, RejectsWriteLargerThanCapacity) {
    alignas(64) std::byte storage[8]{};
    ByteQueue queue(storage, sizeof storage);

    EXPECT_DEATH(
        (void)queue.tryWrite(
            9,
            [](std::span<std::byte>, std::span<std::byte>) noexcept {}),
        "");
}
#endif

// Verifies that read with a timeout returns false after the deadline when the
// queue is empty, without invoking the callback.
TEST(ByteRingQueueTimeout, ReadReturnsFalseAfterTimeoutWhenEmpty) {
    alignas(64) std::byte storage[8]{};
    ByteQueue queue(storage, sizeof storage);

    std::array<std::byte, 1> output{};
    bool callbackInvoked = false;

    const auto start = std::chrono::steady_clock::now();
    const bool result = queue.read(
        1,
        [&](std::span<std::byte> first,
            std::span<std::byte> second) noexcept {
            callbackInvoked = true;
            output[0] = first[0];
            (void)second;
        },
        20ms);
    const auto elapsed = std::chrono::steady_clock::now() - start;

    EXPECT_FALSE(result);
    EXPECT_FALSE(callbackInvoked);
    EXPECT_GE(elapsed, 15ms);
    EXPECT_TRUE(queue.isEmpty());
}

// Verifies that write with a timeout returns false after the deadline when
// the queue is full, without invoking the callback or altering the contents.
TEST(ByteRingQueueTimeout, WriteReturnsFalseAfterTimeoutWhenFull) {
    alignas(64) std::byte storage[8]{};
    ByteQueue queue(storage, sizeof storage);

    std::array<std::byte, 8> input{};
    ASSERT_TRUE(enqueue(queue, input));
    ASSERT_TRUE(queue.isFull());

    bool callbackInvoked = false;
    const auto start = std::chrono::steady_clock::now();
    const bool result = queue.write(
        1,
        [&](std::span<std::byte> first,
            std::span<std::byte> second) noexcept {
            callbackInvoked = true;
            first[0] = std::byte{0xff};
            (void)second;
        },
        20ms);
    const auto elapsed = std::chrono::steady_clock::now() - start;

    EXPECT_FALSE(result);
    EXPECT_FALSE(callbackInvoked);
    EXPECT_GE(elapsed, 15ms);
    EXPECT_EQ(queue.size(), input.size());

    std::array<std::byte, 8> output{};
    ASSERT_TRUE(dequeue(queue, output));
    EXPECT_TRUE(std::equal(input.begin(), input.end(), output.begin()));
}

// Verifies that timeout write/read complete successfully when space/data are
// already available.
TEST(ByteRingQueueTimeout, TimeoutOperationsCompleteWhenPossible) {
    alignas(64) std::byte storage[8]{};
    ByteQueue queue(storage, sizeof storage);

    const std::array<std::byte, 4> input{
        std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}};

    ASSERT_TRUE(queue.write(
        input.size(),
        [&](std::span<std::byte> first,
            std::span<std::byte> second) noexcept {
            std::memcpy(first.data(), input.data(), first.size());
            std::memcpy(second.data(), input.data() + first.size(),
                        second.size());
        },
        1s));

    std::array<std::byte, 4> output{};
    ASSERT_TRUE(queue.read(
        output.size(),
        [&](std::span<std::byte> first,
            std::span<std::byte> second) noexcept {
            std::memcpy(output.data(), first.data(), first.size());
            std::memcpy(output.data() + first.size(), second.data(),
                        second.size());
        },
        1s));

    EXPECT_TRUE(std::equal(input.begin(), input.end(), output.begin()));
    EXPECT_TRUE(queue.isEmpty());
}
