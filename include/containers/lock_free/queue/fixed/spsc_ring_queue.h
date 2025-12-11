#ifndef ATOMC_CONTAINERS_LOCK_FREE_SPSC_RING_QUEUE_H
#define ATOMC_CONTAINERS_LOCK_FREE_SPSC_RING_QUEUE_H

#include <atomic>
#include <array>
#include <span>

#include <cstring>
#include <cstddef>
#include <cassert>

namespace atom::containers::lock_free::fixed::spsc {

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
        available = canEnqueue(writerIndex, m_cachedReaderIndex);
        if (available >= data.size()) {
            copyTo(&m_buffer[writerIndex], data);
            m_writerIndex.store((writerIndex + data.size()) & MASK, std::memory_order_release);
            return true;
        }

        m_cachedReaderIndex = m_readerIndex.load(std::memory_order_acquire);
        available = canEnqueue(writerIndex, m_cachedReaderIndex);
        if (available >= data.size()) {
            copyTo(&m_buffer[writerIndex], data);
            m_writerIndex.store((writerIndex + data.size()) & MASK, std::memory_order_release);
            return true;
        }

        return false;
    }

    SizeType enqueue(std::span<const std::byte> data, SizeType& available) noexcept {
        SizeType enqueued{0};
        const auto writerIndex = m_writerIndex.load(std::memory_order_relaxed);
        available = canEnqueue(writerIndex, m_cachedReaderIndex);
        if (available > 0) {
            enqueued = std::min(available, data.size());
            copyTo(&m_buffer[writerIndex], std::span{data.data(), enqueued});
            m_writerIndex.store((writerIndex + enqueued) & MASK, std::memory_order_release);
            return enqueued;
        }

        m_cachedReaderIndex = m_readerIndex.load(std::memory_order_acquire);
        available = canEnqueue(writerIndex, m_cachedReaderIndex);
        if (available > 0) {
            enqueued = std::min(available, data.size());
            copyTo(&m_buffer[writerIndex], std::span{data.data(), enqueued});
            m_writerIndex.store((writerIndex + enqueued) & MASK, std::memory_order_release);
            return enqueued;
        }

        return enqueued;
    }

    SizeType enqueue(std::span<const std::byte> data) noexcept {
        [[maybe_unused]] SizeType available{};
        return enqueue(data, available);
    }

    bool tryDequeue(std::span<std::byte> data, SizeType& available) noexcept {
        const auto readerIndex = m_readerIndex.load(std::memory_order_relaxed);
        available = canDequeue(m_cachedWriterIndex, readerIndex);
        if (available >= data.size()) {
            copyFrom(&m_buffer[readerIndex], data);
            m_readerIndex.store((readerIndex + data.size()) & MASK, std::memory_order_release);
            return true;
        }

        m_cachedWriterIndex = m_writerIndex.load(std::memory_order_acquire);
        available = canDequeue(m_cachedWriterIndex, readerIndex);
        if (available >= data.size()) {
            copyFrom(&m_buffer[readerIndex], data);
            m_readerIndex.store((readerIndex + data.size()) & MASK, std::memory_order_release);
            return true;
        }

        return false;
    }

    bool tryDequeue(std::span<std::byte> data) noexcept {
        [[maybe_unused]] SizeType available{};
        return tryDequeue(data, available);
    }

    SizeType dequeue(std::span<std::byte> data, SizeType& available) noexcept {
        SizeType dequeued{0};
        const auto readerIndex = m_readerIndex.load(std::memory_order_relaxed);
        available = canDequeue(m_cachedWriterIndex, readerIndex);
        if (available > 0) {
            dequeued = std::min(available, data.size());
            copyFrom(&m_buffer[readerIndex], std::span{data.data(), dequeued});
            m_readerIndex.store((readerIndex + dequeued) & MASK, std::memory_order_release);
            return dequeued;
        }

        m_cachedWriterIndex = m_writerIndex.load(std::memory_order_acquire);
        available = canDequeue(m_cachedWriterIndex, readerIndex);
        if (available > 0) {
            dequeued = std::min(available, data.size());
            copyFrom(&m_buffer[readerIndex], std::span{data.data(), dequeued});
            m_readerIndex.store((readerIndex + dequeued) & MASK, std::memory_order_release);
            return dequeued;
        }

        return dequeued;
    }

    SizeType dequeue(std::span<std::byte> data) noexcept {
        [[maybe_unused]] SizeType available{};
        return dequeue(data, available);
    }

    constexpr SizeType size() const noexcept {
        m_cachedWriterIndex = m_writerIndex.load(std::memory_order_acquire);
        m_cachedReaderIndex = m_readerIndex.load(std::memory_order_acquire);
        return canDequeue(m_cachedWriterIndex, m_cachedReaderIndex);
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

    constexpr void copyTo(std::byte* begin, std::span<const std::byte> data) {
        std::byte* end{m_buffer.data() + m_buffer.size()};
        assert(begin <= end);
        std::size_t remaining{static_cast<std::size_t>(end - begin)};
        if (data.size() <= remaining) {
            std::memcpy(begin, data.data(), data.size());
        } else {
            std::memcpy(begin, data.data(), remaining);
            std::memcpy(m_buffer.data(), data.data() + remaining, data.size() - remaining);
        }
    }

    constexpr void copyFrom(std::byte* begin, std::span<std::byte> data) {
        std::byte* end{m_buffer.data() + m_buffer.size()};
        assert(begin <= end);
        std::size_t remaining{static_cast<std::size_t>(end - begin)};
        if (data.size() <= remaining) {
            std::memcpy(data.data(), begin, data.size());
        } else {
            std::memcpy(data.data(), begin, remaining);
            std::memcpy(data.data() + remaining, m_buffer.data(), data.size() - remaining);
        }
    }

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

    mutable SizeType m_cachedReaderIndex{0};
    mutable SizeType m_cachedWriterIndex{0};

    alignas(CACHELINE_SIZE) std::atomic<SizeType> m_writerIndex{0};
    alignas(CACHELINE_SIZE) std::atomic<SizeType> m_readerIndex{0};
    alignas(CACHELINE_SIZE) std::array<std::byte, N> m_buffer;
};

} //! namespace atom::containers::lock_free::fixed::spsc

#endif //! ATOMC_CONTAINERS_LOCK_FREE_SPSC_RING_QUEUE_H
