#include <gtest/gtest.h>

#include "containers/lock-free/mpmc_ring_queue.h"

#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <span>
#include <stdexcept>

namespace {

using atom::containers::lock_free::mpmc::RingQueue;

struct Tracked {
    static inline std::atomic<int> live{0};
    static inline std::atomic<int> copies{0};
    static inline std::atomic<int> moves{0};

    int value{-1};

    Tracked() noexcept { live.fetch_add(1); }
    explicit Tracked(int v) noexcept : value(v) { live.fetch_add(1); }
    Tracked(const Tracked& other) noexcept : value(other.value)
    {
        live.fetch_add(1);
        copies.fetch_add(1);
    }
    Tracked(Tracked&& other) noexcept : value(other.value)
    {
        live.fetch_add(1);
        moves.fetch_add(1);
    }
    Tracked& operator=(const Tracked&) noexcept = default;
    Tracked& operator=(Tracked&&) noexcept = default;
    ~Tracked() noexcept { live.fetch_sub(1); }

    static void reset() noexcept
    {
        live.store(0);
        copies.store(0);
        moves.store(0);
    }
};

using Queue = RingQueue<Tracked>;

class SingleThreadedTest : public ::testing::Test {
protected:
    void SetUp() override { Tracked::reset(); }
};

TEST(MpmcSingleThreadedConstructor, AcceptsPowersOfTwoAndRejectsOthers) {
    EXPECT_NO_THROW(((void)RingQueue<int>{2}));
    EXPECT_NO_THROW(((void)RingQueue<int>{1024}));
    EXPECT_EQ(RingQueue<int>{1024}.capacity(), 1024u);

    EXPECT_THROW(((void)RingQueue<int>{0}), std::invalid_argument);
    EXPECT_THROW(((void)RingQueue<int>{1}), std::invalid_argument);
    EXPECT_THROW(((void)RingQueue<int>{3}), std::invalid_argument);
    EXPECT_THROW(((void)RingQueue<int>{5}), std::invalid_argument);
    EXPECT_THROW(((void)RingQueue<int>{6}), std::invalid_argument);
    EXPECT_THROW(((void)RingQueue<int>{7}), std::invalid_argument);
    EXPECT_THROW(((void)RingQueue<int>{24}), std::invalid_argument);
}

TEST_F(SingleThreadedTest, MetricsOnEmptyQueue) {
    Queue queue{4};

    EXPECT_EQ(queue.size(), 0u);
    EXPECT_EQ(queue.capacity(), 4u);
    EXPECT_EQ(queue.availableSpace(), 4u);
    EXPECT_TRUE(queue.isEmpty());
    EXPECT_FALSE(queue.isFull());
}

TEST_F(SingleThreadedTest, MetricsTrackEveryOperation) {
    Queue queue{4};

    ASSERT_TRUE(queue.tryEmplace(1));
    EXPECT_EQ(queue.size(), 1u);
    EXPECT_EQ(queue.availableSpace(), 3u);
    EXPECT_FALSE(queue.isEmpty());

    ASSERT_TRUE(queue.tryEmplace(2));
    ASSERT_TRUE(queue.tryEmplace(3));
    ASSERT_TRUE(queue.tryEmplace(4));
    EXPECT_EQ(queue.size(), 4u);
    EXPECT_EQ(queue.availableSpace(), 0u);
    EXPECT_TRUE(queue.isFull());

    Tracked out;
    ASSERT_TRUE(queue.tryRead(out));
    EXPECT_EQ(out.value, 1);
    EXPECT_EQ(queue.size(), 3u);
    EXPECT_EQ(queue.availableSpace(), 1u);
    EXPECT_FALSE(queue.isFull());
}

TEST_F(SingleThreadedTest, TryEmplaceRoundTripPreservesFifo) {
    Queue queue{4};

    ASSERT_TRUE(queue.tryEmplace(10));
    ASSERT_TRUE(queue.tryEmplace(11));
    ASSERT_TRUE(queue.tryEmplace(12));

    Tracked out;
    ASSERT_TRUE(queue.tryRead(out));
    EXPECT_EQ(out.value, 10);
    ASSERT_TRUE(queue.tryRead(out));
    EXPECT_EQ(out.value, 11);
    ASSERT_TRUE(queue.tryRead(out));
    EXPECT_EQ(out.value, 12);
    EXPECT_TRUE(queue.isEmpty());
    EXPECT_EQ(Tracked::live.load(), 1);
}

TEST_F(SingleThreadedTest, TryEmplaceOnFullQueueDoesNotConstruct) {
    Queue queue{2};
    ASSERT_TRUE(queue.tryEmplace(1));
    ASSERT_TRUE(queue.tryEmplace(2));

    const int liveBefore = Tracked::live.load();
    EXPECT_FALSE(queue.tryEmplace(3));
    EXPECT_EQ(Tracked::live.load(), liveBefore);
    EXPECT_EQ(queue.size(), 2u);
}

TEST_F(SingleThreadedTest, TryWriteCopyUsesCopyAndFailsWithoutCopying) {
    Queue queue{2};
    Tracked source{7};

    const int copiesBefore = Tracked::copies.load();
    ASSERT_TRUE(queue.tryWrite(source));
    ASSERT_TRUE(queue.tryEmplace(99));
    EXPECT_EQ(Tracked::copies.load(), copiesBefore + 1);

    const int copiesAfterSuccess = Tracked::copies.load();
    Tracked overflow{8};
    EXPECT_FALSE(queue.tryWrite(overflow));
    EXPECT_EQ(Tracked::copies.load(), copiesAfterSuccess);

    Tracked out;
    ASSERT_TRUE(queue.tryRead(out));
    EXPECT_EQ(out.value, 7);
}

TEST_F(SingleThreadedTest, TryWriteMoveLeavesSourceIntactOnFailure) {
    Queue queue{2};
    ASSERT_TRUE(queue.tryEmplace(1));
    ASSERT_TRUE(queue.tryEmplace(2));

    Tracked source{42};
    const int movesBefore = Tracked::moves.load();
    EXPECT_FALSE(queue.tryWrite(std::move(source)));
    EXPECT_EQ(source.value, 42);
    EXPECT_EQ(Tracked::moves.load(), movesBefore);

    Tracked out;
    ASSERT_TRUE(queue.tryRead(out));
    EXPECT_EQ(out.value, 1);
    ASSERT_TRUE(queue.tryRead(out));
    EXPECT_EQ(out.value, 2);

    ASSERT_TRUE(queue.tryWrite(std::move(source)));
    ASSERT_TRUE(queue.tryRead(out));
    EXPECT_EQ(out.value, 42);
}

TEST_F(SingleThreadedTest, TryWriteEmptySpanIsNoOp) {
    Queue queue{4};
    const std::span<const Tracked> empty{};

    EXPECT_TRUE(queue.tryWrite(empty));
    EXPECT_TRUE(queue.isEmpty());
}

TEST_F(SingleThreadedTest, TryWriteSpanLargerThanCapacityIsRejected) {
    Queue queue{4};
    const std::array<Tracked, 5> source{Tracked{0}, Tracked{1}, Tracked{2},
                                        Tracked{3}, Tracked{4}};

    EXPECT_FALSE(queue.tryWrite(std::span<const Tracked>{source}));
    EXPECT_TRUE(queue.isEmpty());
}

TEST_F(SingleThreadedTest, TryWriteSpanExactlyCapacityFillsQueue) {
    Queue queue{4};
    const std::array<Tracked, 4> source{Tracked{0}, Tracked{1}, Tracked{2},
                                        Tracked{3}};

    ASSERT_TRUE(queue.tryWrite(std::span<const Tracked>{source}));
    EXPECT_EQ(queue.size(), 4u);
    EXPECT_TRUE(queue.isFull());

    std::array<Tracked, 4> out;
    ASSERT_TRUE(queue.tryRead(std::span<Tracked>{out}));
    for (int i = 0; i < 4; ++i) {
        EXPECT_EQ(out[i].value, i);
    }
    EXPECT_TRUE(queue.isEmpty());
}

TEST_F(SingleThreadedTest, TryWriteSpanPartialFreeFailsAtomically) {
    Queue queue{4};
    ASSERT_TRUE(queue.tryEmplace(10));
    ASSERT_TRUE(queue.tryEmplace(11));

    const std::array<Tracked, 3> source{Tracked{1}, Tracked{2}, Tracked{3}};
    const int copiesBefore = Tracked::copies.load();

    EXPECT_FALSE(queue.tryWrite(std::span<const Tracked>{source}));
    EXPECT_EQ(Tracked::copies.load(), copiesBefore);
    EXPECT_EQ(queue.size(), 2u);

    Tracked out;
    ASSERT_TRUE(queue.tryRead(out));
    EXPECT_EQ(out.value, 10);
    ASSERT_TRUE(queue.tryRead(out));
    EXPECT_EQ(out.value, 11);
    EXPECT_TRUE(queue.isEmpty());
}

TEST_F(SingleThreadedTest, TryWriteSpanWrapsAround) {
    Queue queue{4};
    const std::array<Tracked, 3> first{Tracked{0}, Tracked{1}, Tracked{2}};
    ASSERT_TRUE(queue.tryWrite(std::span<const Tracked>{first}));

    std::array<Tracked, 2> prefix;
    ASSERT_TRUE(queue.tryRead(std::span<Tracked>{prefix}));
    EXPECT_EQ(prefix[0].value, 0);
    EXPECT_EQ(prefix[1].value, 1);

    const std::array<Tracked, 2> second{Tracked{3}, Tracked{4}};
    ASSERT_TRUE(queue.tryWrite(std::span<const Tracked>{second}));

    std::array<Tracked, 3> rest;
    ASSERT_TRUE(queue.tryRead(std::span<Tracked>{rest}));
    EXPECT_EQ(rest[0].value, 2);
    EXPECT_EQ(rest[1].value, 3);
    EXPECT_EQ(rest[2].value, 4);
    EXPECT_TRUE(queue.isEmpty());
}

TEST_F(SingleThreadedTest, TryReadEmptyDoesNotModifyOutput) {
    Queue queue{4};
    Tracked out{99};

    const int movesBefore = Tracked::moves.load();
    EXPECT_FALSE(queue.tryRead(out));
    EXPECT_EQ(out.value, 99);
    EXPECT_EQ(Tracked::moves.load(), movesBefore);
}

TEST_F(SingleThreadedTest, TryReadDestroysStoredObject) {
    Queue queue{4};
    ASSERT_TRUE(queue.tryEmplace(5));
    EXPECT_EQ(Tracked::live.load(), 1);

    Tracked out;
    EXPECT_EQ(Tracked::live.load(), 2);
    ASSERT_TRUE(queue.tryRead(out));
    EXPECT_EQ(Tracked::live.load(), 1);
    EXPECT_EQ(out.value, 5);
}

TEST_F(SingleThreadedTest, TryReadEmptySpanIsNoOp) {
    Queue queue{4};
    const std::span<Tracked> empty{};

    EXPECT_TRUE(queue.tryRead(empty));
    EXPECT_TRUE(queue.isEmpty());
}

TEST_F(SingleThreadedTest, TryReadSpanLargerThanCapacityIsRejected) {
    Queue queue{2};
    ASSERT_TRUE(queue.tryEmplace(1));
    std::array<Tracked, 3> out;

    EXPECT_FALSE(queue.tryRead(std::span<Tracked>{out}));
    EXPECT_EQ(queue.size(), 1u);
}

TEST_F(SingleThreadedTest, TryReadSpanInsufficientFailsAtomically) {
    Queue queue{4};
    ASSERT_TRUE(queue.tryEmplace(1));
    ASSERT_TRUE(queue.tryEmplace(2));

    std::array<Tracked, 3> out;
    EXPECT_FALSE(queue.tryRead(std::span<Tracked>{out}));
    EXPECT_EQ(queue.size(), 2u);
    for (const auto& item : out) {
        EXPECT_EQ(item.value, -1);
    }

    Tracked single;
    ASSERT_TRUE(queue.tryRead(single));
    EXPECT_EQ(single.value, 1);
    ASSERT_TRUE(queue.tryRead(single));
    EXPECT_EQ(single.value, 2);
}

TEST_F(SingleThreadedTest, TryReadSpanWrapsAround) {
    Queue queue{4};
    const std::array<Tracked, 4> data{Tracked{0}, Tracked{1}, Tracked{2},
                                      Tracked{3}};
    ASSERT_TRUE(queue.tryWrite(std::span<const Tracked>{data}));

    std::array<Tracked, 2> first;
    ASSERT_TRUE(queue.tryRead(std::span<Tracked>{first}));
    EXPECT_EQ(first[0].value, 0);
    EXPECT_EQ(first[1].value, 1);

    const std::array<Tracked, 2> more{Tracked{4}, Tracked{5}};
    ASSERT_TRUE(queue.tryWrite(std::span<const Tracked>{more}));

    std::array<Tracked, 4> rest;
    ASSERT_TRUE(queue.tryRead(std::span<Tracked>{rest}));
    EXPECT_EQ(rest[0].value, 2);
    EXPECT_EQ(rest[1].value, 3);
    EXPECT_EQ(rest[2].value, 4);
    EXPECT_EQ(rest[3].value, 5);
    EXPECT_TRUE(queue.isEmpty());
}

TEST_F(SingleThreadedTest, BulkFullCapacityFromWrappedIndices) {
    Queue queue{4};
    ASSERT_TRUE(queue.tryEmplace(0));
    ASSERT_TRUE(queue.tryEmplace(1));

    Tracked out;
    ASSERT_TRUE(queue.tryRead(out));
    ASSERT_TRUE(queue.tryRead(out));
    EXPECT_TRUE(queue.isEmpty());

    const std::array<Tracked, 4> source{Tracked{10}, Tracked{11},
                                        Tracked{12}, Tracked{13}};
    ASSERT_TRUE(queue.tryWrite(std::span<const Tracked>{source}));
    EXPECT_TRUE(queue.isFull());

    std::array<Tracked, 4> drained;
    ASSERT_TRUE(queue.tryRead(std::span<Tracked>{drained}));
    for (int i = 0; i < 4; ++i) {
        EXPECT_EQ(drained[i].value, 10 + i);
    }
    EXPECT_TRUE(queue.isEmpty());
}

TEST_F(SingleThreadedTest, TimeoutWriteFailsWhenFull) {
    Queue queue{2};
    ASSERT_TRUE(queue.tryEmplace(1));
    ASSERT_TRUE(queue.tryEmplace(2));

    Tracked source{3};
    EXPECT_FALSE(queue.write(source, std::chrono::milliseconds{0}));
    EXPECT_EQ(queue.size(), 2u);
}

TEST_F(SingleThreadedTest, TimeoutWriteLeavesMoveSourceOnTimeout) {
    Queue queue{2};
    ASSERT_TRUE(queue.tryEmplace(1));
    ASSERT_TRUE(queue.tryEmplace(2));

    Tracked source{2};
    EXPECT_FALSE(queue.write(std::move(source), std::chrono::milliseconds{0}));
    EXPECT_EQ(source.value, 2);
}

TEST_F(SingleThreadedTest, TimeoutWriteSucceedsWhenSpaceAvailable) {
    Queue queue{2};

    Tracked source{2};
    EXPECT_TRUE(queue.write(source, std::chrono::seconds{1}));

    Tracked out;
    ASSERT_TRUE(queue.tryRead(out));
    EXPECT_EQ(out.value, 2);
}

TEST_F(SingleThreadedTest, TimeoutReadFailsWhenEmpty) {
    Queue queue{2};
    Tracked out{55};

    EXPECT_FALSE(queue.read(out, std::chrono::milliseconds{0}));
    EXPECT_EQ(out.value, 55);
}

TEST_F(SingleThreadedTest, TimeoutReadSucceedsWhenDataAvailable) {
    Queue queue{2};
    ASSERT_TRUE(queue.tryEmplace(3));

    Tracked out;
    EXPECT_TRUE(queue.read(out, std::chrono::seconds{1}));
    EXPECT_EQ(out.value, 3);
}

TEST_F(SingleThreadedTest, DestructorDestroysRemainingObjects) {
    {
        Queue queue{4};
        ASSERT_TRUE(queue.tryEmplace(1));
        ASSERT_TRUE(queue.tryEmplace(2));
        ASSERT_TRUE(queue.tryEmplace(3));
        EXPECT_EQ(Tracked::live.load(), 3);
    }
    EXPECT_EQ(Tracked::live.load(), 0);
}

TEST_F(SingleThreadedTest, LongRunOfWrappedOperationsPreservesFifo) {
    constexpr long kOperations = 20000;
    Queue queue{8};

    long expected = 0;
    for (long value = 0; value < kOperations; ++value) {
        ASSERT_TRUE(queue.tryWrite(Tracked{static_cast<int>(value)}));
        if (queue.isFull()) {
            Tracked out;
            ASSERT_TRUE(queue.tryRead(out));
            EXPECT_EQ(out.value, static_cast<int>(expected));
            ++expected;
        }
    }
    while (!queue.isEmpty()) {
        Tracked out;
        ASSERT_TRUE(queue.tryRead(out));
        EXPECT_EQ(out.value, static_cast<int>(expected));
        ++expected;
    }
    EXPECT_EQ(expected, kOperations);
    EXPECT_EQ(Tracked::live.load(), 0);
}

TEST_F(SingleThreadedTest, MinimumCapacitySingleElementOperations) {
    Queue queue{2};
    Tracked out;

    EXPECT_FALSE(queue.tryRead(out));

    ASSERT_TRUE(queue.tryEmplace(1));
    ASSERT_TRUE(queue.tryEmplace(2));
    EXPECT_TRUE(queue.isFull());
    EXPECT_FALSE(queue.tryEmplace(3));

    ASSERT_TRUE(queue.tryRead(out));
    EXPECT_EQ(out.value, 1);
    ASSERT_TRUE(queue.tryRead(out));
    EXPECT_EQ(out.value, 2);
    EXPECT_TRUE(queue.isEmpty());
    EXPECT_FALSE(queue.tryRead(out));
}

TEST_F(SingleThreadedTest, MinimumCapacityBulkOperations) {
    Queue queue{2};
    const std::array<Tracked, 2> source{Tracked{7}, Tracked{8}};

    ASSERT_TRUE(queue.tryWrite(std::span<const Tracked>{source}));
    EXPECT_TRUE(queue.isFull());

    std::array<Tracked, 2> out;
    ASSERT_TRUE(queue.tryRead(std::span<Tracked>{out}));
    EXPECT_EQ(out[0].value, 7);
    EXPECT_EQ(out[1].value, 8);
    EXPECT_TRUE(queue.isEmpty());
}

TEST_F(SingleThreadedTest, MinimumCapacityReusesSlotsRepeatedly) {
    Queue queue{2};
    Tracked out;

    for (int i = 0; i < 1000; ++i) {
        ASSERT_TRUE(queue.tryEmplace(i));
        ASSERT_TRUE(queue.tryRead(out));
        EXPECT_EQ(out.value, i);
    }
    EXPECT_TRUE(queue.isEmpty());
}

TEST(MpmcSingleThreadedTypes, MoveOnlyUniquePtrRoundTrips) {
    RingQueue<std::unique_ptr<int>> queue{2};

    auto first = std::make_unique<int>(42);
    ASSERT_TRUE(queue.tryWrite(std::move(first)));
    EXPECT_EQ(first, nullptr);

    std::unique_ptr<int> out;
    ASSERT_TRUE(queue.tryRead(out));
    ASSERT_NE(out, nullptr);
    EXPECT_EQ(*out, 42);

    ASSERT_TRUE(queue.tryWrite(std::make_unique<int>(1)));
    ASSERT_TRUE(queue.tryWrite(std::make_unique<int>(2)));

    auto intact = std::make_unique<int>(9);
    EXPECT_FALSE(queue.tryWrite(std::move(intact)));
    ASSERT_NE(intact, nullptr);
    EXPECT_EQ(*intact, 9);
}

TEST(MpmcSingleThreadedTypes, LargeObjectRoundTrips) {
    struct alignas(64) Big {
        std::array<std::uint64_t, 16> words{};
    };

    RingQueue<Big> queue{2};
    Big input;
    input.words[0] = 0xDEADBEEF;
    input.words[15] = 0x12345678;

    ASSERT_TRUE(queue.tryWrite(input));

    Big output;
    ASSERT_TRUE(queue.tryRead(output));
    EXPECT_EQ(output.words[0], 0xDEADBEEF);
    EXPECT_EQ(output.words[15], 0x12345678);
}

TEST(MpmcSingleThreadedTypes, OverAlignedObjectRoundTrips) {
    struct alignas(128) Aligned {
        int value{};
    };

    RingQueue<Aligned> queue{2};
    ASSERT_TRUE(queue.tryEmplace(Aligned{11}));

    Aligned output;
    ASSERT_TRUE(queue.tryRead(output));
    EXPECT_EQ(output.value, 11);
}

} // namespace
