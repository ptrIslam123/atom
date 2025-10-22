#include <atomic>
#include <array>
#include <span>

#include <cstdint>
#include <cstddef>
#include <cassert>

namespace atom::containers::lock_free::spsc {

template<std::size_t N>
class RingQueue final {
public:
    static_assert(N > 0 && (N & (N - 1)) == 0, "Capacity must be power of two");
    static_assert (N > 1, "Capacity must be > 1");
    static constexpr auto CAPACITY{N - 1};

    using SizeType = std::size_t;

    RingQueue(const RingQueue&) = delete;
    RingQueue(RingQueue&&) = delete;
    RingQueue& operator=(const RingQueue&) = delete;
    RingQueue& operator=(RingQueue&&) = delete;

    RingQueue() noexcept = default;
    ~RingQueue() noexcept = default;

    bool tryEnqueue(std::span<const std::byte> data) noexcept {
        [[maybe_unused]] SizeType available{};
        return tryEnqueue(data, available);
    }

    bool tryEnqueue(std::span<const std::byte> data, SizeType& available) noexcept {
        const auto writerIndex = m_writerIndex.load(std::memory_order_relaxed);
        const auto readerIndex = m_readerIndex.load(std::memory_order_acquire);

        available = canEnqueue(writerIndex, readerIndex);
        if (available < data.size()) {
            return false;
        }

        auto nextWriteIndex{writerIndex};
        for (SizeType i = 0; i < data.size(); ++i) {
            m_buffer[nextWriteIndex] = data[i];
            nextWriteIndex = (nextWriteIndex + 1) & MASK;
        }
        m_writerIndex.store(nextWriteIndex, std::memory_order_release);
        return true;
    }

    SizeType enqueue(std::span<const std::byte> data, SizeType& available) noexcept {
        const auto writerIndex = m_writerIndex.load(std::memory_order_relaxed);
        const auto readerIndex = m_readerIndex.load(std::memory_order_acquire);

        available = canEnqueue(writerIndex, readerIndex);
        if (available == 0) {
            return 0;
        }

        const SizeType enqueued{std::min(available, data.size())};
        auto nextWriteIndex{writerIndex};
        for (SizeType i = 0; i < enqueued; ++i) {
            m_buffer[nextWriteIndex] = data[i];
            nextWriteIndex = (nextWriteIndex + 1) & MASK;
        }
        m_writerIndex.store(nextWriteIndex, std::memory_order_release);
        return enqueued;
    }

    SizeType enqueue(std::span<const std::byte> data) noexcept {
        [[maybe_unused]] SizeType available{};
        return enqueue(data, available);
    }

    bool tryDequeue(std::span<std::byte> data, SizeType& available) noexcept {
        const auto writerIndex = m_writerIndex.load(std::memory_order_acquire);
        const auto readerIndex = m_readerIndex.load(std::memory_order_relaxed);

        available = canDequeue(writerIndex, readerIndex);
        if (available < data.size()) {
            return false;
        }

        SizeType nextReaderIndex{readerIndex};
        for (SizeType i = 0; i < data.size(); ++i) {
            data[i] = m_buffer[nextReaderIndex];
            nextReaderIndex = (nextReaderIndex + 1) & MASK;
        }
        m_readerIndex.store(nextReaderIndex, std::memory_order_release);
        return true;
    }

    bool tryDequeue(std::span<std::byte> data) noexcept {
        [[maybe_unused]] SizeType available{};
        return tryDequeue(data, available);
    }

    SizeType dequeue(std::span<std::byte> data, SizeType& available) noexcept {
        const auto writerIndex = m_writerIndex.load(std::memory_order_relaxed);
        const auto readerIndex = m_readerIndex.load(std::memory_order_acquire);

        available = canDequeue(writerIndex, readerIndex);
        if (available == 0) {
            return 0;
        }

        const SizeType dequeued{std::min(available, data.size())};
        SizeType nextReaderIndex{readerIndex};
        for (SizeType i = 0; i < dequeued; ++i) {
            data[i] = m_buffer[nextReaderIndex];
            nextReaderIndex = (nextReaderIndex + 1) & MASK;
        }
        m_readerIndex.store(nextReaderIndex, std::memory_order_release);
        return dequeued;
    }

    SizeType dequeue(std::span<std::byte> data) noexcept {
        [[maybe_unused]] SizeType available{};
        return dequeue(data, available);
    }

    constexpr SizeType size() const noexcept {
        const auto writerIndex = m_writerIndex.load(std::memory_order_acquire);
        const auto readerIndex = m_readerIndex.load(std::memory_order_acquire);
        return canDequeue(writerIndex, readerIndex);
    }
    constexpr SizeType capacity() const noexcept {
        return CAPACITY;
    }
    constexpr bool empty() const noexcept {
        return size() == 0;
    }

private:
    static constexpr auto CACHELINE_SIZE{64};
    static constexpr auto MASK{N - 1};

    constexpr SizeType canEnqueue(SizeType writerIndex, SizeType readerIndex) const noexcept {
        assert(writerIndex < N);
        assert(readerIndex < N);

        if (writerIndex >= readerIndex) {
            return (N - writerIndex) + readerIndex - 1;
        } else {
            return readerIndex - writerIndex - 1;
        }
    }

    constexpr SizeType canDequeue(SizeType writerIndex, SizeType readerIndex) const noexcept {
        assert(writerIndex < N);
        assert(readerIndex < N);

        if (writerIndex >= readerIndex) {
            return writerIndex - readerIndex;
        } else {
            return N + writerIndex - readerIndex;
        }
    }

    SizeType m_cachedReaderIndex{0};
    SizeType m_cachedWriterIndex{0};

    alignas(CACHELINE_SIZE) std::atomic<SizeType> m_writerIndex{0};
    alignas(CACHELINE_SIZE) std::atomic<SizeType> m_readerIndex{0};
    alignas(CACHELINE_SIZE) std::array<std::byte, N> m_buffer;
};

} //! namespace atom::containers::lock_free::spsc
