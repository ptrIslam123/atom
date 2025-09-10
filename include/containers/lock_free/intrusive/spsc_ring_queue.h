#ifndef ATOMC_CONTAINERS_LOCK_FREE_SPSC_H
#define ATOMC_CONTAINERS_LOCK_FREE_SPSC_H

#include <atomic>
#include <array>

#include <cstdint>
#include <cstddef>
#include <cassert>

namespace atom::containers::lock_free::spsc {

template<typename T, std::size_t N>
class IntrusiveRingQueue final {
public:
    static_assert(N > 0 && (N & (N - 1)) == 0, "Capacity must be power of two");
    static_assert (N > 1, "Capacity must be > 1");
    static constexpr auto CAPACITY{N};

    using ElementType = T;
    using SizeType = std::size_t;

    IntrusiveRingQueue(const IntrusiveRingQueue&) = delete;
    IntrusiveRingQueue(IntrusiveRingQueue&&) = delete;
    IntrusiveRingQueue& operator=(const IntrusiveRingQueue&) = delete;
    IntrusiveRingQueue& operator=(IntrusiveRingQueue&&) = delete;

    IntrusiveRingQueue() noexcept = default;
    ~IntrusiveRingQueue() noexcept = default;

    bool tryEnqueue(T* ptr) noexcept;
    bool tryEnqueueBulk(T* ptr, std::size_t count, std::size_t& available) noexcept;
    std::size_t enqueueBurst(T* ptr, std::size_t count, std::size_t& available) noexcept;
    bool tryEnqueueBulk(T* ptr, std::size_t count) noexcept;
    std::size_t enqueueBurst(T* ptr, std::size_t count) noexcept;

    bool tryDequeue(T** ptr) noexcept;
    bool tryDequeueBulk(T** ptr, std::size_t count, std::size_t& available) noexcept;
    std::size_t dequeueBurst(T** ptr, std::size_t count, std::size_t& available) noexcept;

    constexpr SizeType size() const noexcept;
    constexpr SizeType capacity() const noexcept;
    constexpr bool empty() const noexcept;

private:
    static constexpr auto CACHELINE_SIZE{64};
    static constexpr auto MASK{CAPACITY - 1};

    constexpr SizeType canEnqueue(SizeType writerIndex, SizeType readerIndex) const noexcept;
    constexpr SizeType canDequeue(SizeType writerIndex, SizeType readerIndex) const noexcept;

    SizeType m_cachedReaderIndex{0};
    SizeType m_cachedWriterIndex{0};

    alignas(CACHELINE_SIZE) std::atomic<SizeType> m_writerIndex{0};
    alignas(CACHELINE_SIZE) std::atomic<SizeType> m_readerIndex{0};
    alignas(CACHELINE_SIZE) std::array<T*, CAPACITY> m_buffer;
};

template<typename T, std::size_t N>
bool IntrusiveRingQueue<T, N>::tryEnqueue(T* ptr) noexcept {
    // The caches will reflect the same values ​​of their atomic indexes,
    // only possibly with some delay, and in fact this cannot lead to a problem when adding,
    // because we check that the current atomic index of the writer did not initially coincide
    // with the cached index of the reader; if there is no space (they coincide),
    // we can be sure by rereading the atomic index of the reader and this will be absolutely safe,
    // because the main goal is to prevent the writer's index from overtaking the reader's index,
    // but as I said earlier, the cached index of the reader can be less than or equal
    // to the atomic index of the reader, but in no way greater than it, accordingly,
    // we cannot get the destruction of invariants in the add method.
    assert(ptr);
    const auto writerIndex = m_writerIndex.load(std::memory_order_relaxed);
    const auto nextWriterIndex = (writerIndex + 1) & MASK;

    if (nextWriterIndex != m_cachedReaderIndex) {
        m_buffer[writerIndex] = ptr;
        m_writerIndex.store(nextWriterIndex, std::memory_order_release);
        return true;
    }

    m_cachedReaderIndex = m_readerIndex.load(std::memory_order_acquire);
    if (nextWriterIndex != m_cachedReaderIndex) {
        m_buffer[writerIndex] = ptr;
        m_writerIndex.store(nextWriterIndex, std::memory_order_release);
        return true;
    }
    return false;
}

template<typename T, std::size_t N>
bool IntrusiveRingQueue<T, N>::tryEnqueueBulk(T* ptr, std::size_t count, std::size_t& available) noexcept {
    assert(ptr);
    if (count == 0) [[unlikely]] {
       m_cachedReaderIndex = m_readerIndex.load(std::memory_order_acquire);
       available = canEnqueue(
                   m_writerIndex.load(std::memory_order_relaxed),
                   m_cachedReaderIndex
       );
       return true;
    }

    const auto writerIndex{m_writerIndex.load(std::memory_order_relaxed)};
    available = canEnqueue(writerIndex, m_cachedReaderIndex);
    if (available >= count) {
       for (auto i = 0; i < count; ++i) {
           m_buffer[(writerIndex + i) & MASK] = &ptr[i];
       }
       m_writerIndex.store((writerIndex + count) & MASK, std::memory_order_release);
       return true;
    }

    m_cachedReaderIndex = m_readerIndex.load(std::memory_order_acquire);
    available = canEnqueue(writerIndex, m_cachedReaderIndex);
    if (available >= count) {
       for (auto i = 0; i < count; ++i) {
           m_buffer[(writerIndex + i) & MASK] = &ptr[i];
       }
       m_writerIndex.store((writerIndex + count) & MASK, std::memory_order_release);
       return true;
    }

    return false;
}

template<typename T, std::size_t N>
std::size_t IntrusiveRingQueue<T, N>::enqueueBurst(T* ptr, std::size_t count, std::size_t& available) noexcept {
    assert(ptr);
    assert(count <= capacity());
    if (count == 0) [[unlikely]] {
        m_cachedReaderIndex = m_readerIndex.load(std::memory_order_acquire);
        available = canEnqueue(
                    m_writerIndex.load(std::memory_order_relaxed),
                    m_cachedReaderIndex
        );
        return true;
    }

    const auto writerIndex{m_writerIndex.load(std::memory_order_relaxed)};
    available = canEnqueue(writerIndex, m_cachedReaderIndex);
    if (available >= count) {
        for (auto i = 0; i < count; ++i) {
            m_buffer[(writerIndex + i) & MASK] = ptr[i];
        }
        m_writerIndex.store((writerIndex + count) & MASK, std::memory_order_release);
        return count;
    }

    m_cachedReaderIndex = m_readerIndex.load(std::memory_order_acquire);
    available = canEnqueue(writerIndex, m_cachedReaderIndex);
    count = std::min(count, available);
    for (auto i = 0; i < count; ++i) {
        m_buffer[(writerIndex + i) & MASK] = ptr[i];
    }
    m_writerIndex.store((writerIndex + count) & MASK, std::memory_order_release);
    return count;
}

template<typename T, std::size_t N>
bool IntrusiveRingQueue<T, N>::tryEnqueueBulk(T* ptr, std::size_t count) noexcept {
    [[maybe_unused]] std::size_t available{0};
    return tryEnqueueBulk(ptr, count, available);
}

template<typename T, std::size_t N>
std::size_t IntrusiveRingQueue<T, N>::enqueueBurst(T* ptr, std::size_t count) noexcept {
    [[maybe_unused]] std::size_t available{0};
    return enqueueBurst(ptr, count, available);
}

template<typename T, std::size_t N>
bool IntrusiveRingQueue<T, N>::tryDequeue(T** ptr) noexcept {
    const auto readerIndex{m_readerIndex.load(std::memory_order_relaxed)};

    if (readerIndex != m_cachedWriterIndex) [[likely]] {
        *ptr = m_buffer[readerIndex];
        const auto nextReaderIndex = (readerIndex + 1) & MASK;
        m_readerIndex.store(nextReaderIndex, std::memory_order_release);
        return true;
    }

    m_cachedWriterIndex = m_writerIndex.load(std::memory_order_acquire);
    if (readerIndex != m_cachedWriterIndex) [[likely]] {
        *ptr = m_buffer[readerIndex];
        const auto nextReaderIndex = (readerIndex + 1) & MASK;
        m_readerIndex.store(nextReaderIndex, std::memory_order_release);
        return true;
    }

    return false;
}

template<typename T, std::size_t N>
bool IntrusiveRingQueue<T, N>::tryDequeueBulk(T** ptr, std::size_t count, std::size_t& available) noexcept {
    assert(ptr);
    assert(count <= capacity());

    if (count == 0) [[unlikely]] {
        available = canDequeue(m_cachedWriterIndex, m_cachedReaderIndex);
        return true;
    }

    auto readerIndex{m_readerIndex.load(std::memory_order_relaxed)};
    available = canDequeue(m_cachedWriterIndex, readerIndex);
    if (available >= count) {
        for (auto i = 0; i < count; ++i) {
            ptr[i] = m_buffer[(readerIndex + i) & MASK];
        }
        m_readerIndex.store((readerIndex + count) & MASK, std::memory_order_release);
        return true;
    }

    m_cachedWriterIndex = m_writerIndex.load(std::memory_order_acquire);
    available = canDequeue(m_cachedWriterIndex, readerIndex);
    if (available >= count) {
        for (auto i = 0; i < count; ++i) {
            ptr[i] = m_buffer[(readerIndex + i) & MASK];
        }
        m_readerIndex.store((readerIndex + count) & MASK, std::memory_order_release);
        return true;
    }

    return false;
}

template<typename T, std::size_t N>
std::size_t IntrusiveRingQueue<T, N>::dequeueBurst(T** ptr, std::size_t count, std::size_t& available) noexcept {
    assert(ptr);
    assert(count <= capacity());

    auto readerIndex{m_readerIndex.load(std::memory_order_relaxed)};
    auto i = 0;
    for (; i < count; ++i) {
        if (readerIndex == m_cachedWriterIndex) {
            m_cachedWriterIndex = m_writerIndex.load(std::memory_order_acquire);
            if (readerIndex == m_cachedWriterIndex) {
                break;
            }
        }
        ptr[i] = m_buffer[readerIndex];
        readerIndex = (readerIndex + 1) & MASK;
    }
    m_readerIndex.store(readerIndex, std::memory_order_release);
    available = canDequeue(m_cachedWriterIndex, readerIndex);
    return i;
}

template<typename T, std::size_t N>
constexpr typename IntrusiveRingQueue<T, N>::SizeType
IntrusiveRingQueue<T, N>::size() const noexcept {
    return canDequeue(
                m_writerIndex.load(std::memory_order_acquire),
                m_readerIndex.load(std::memory_order_acquire)
    );
}

template<typename T, std::size_t N>
constexpr typename IntrusiveRingQueue<T, N>::SizeType
IntrusiveRingQueue<T, N>::capacity() const noexcept {
    return CAPACITY - 1;
}

template<typename T, std::size_t N>
constexpr bool IntrusiveRingQueue<T, N>::empty() const noexcept {
    return size() == 0;
}

template<typename T, std::size_t N>
constexpr typename IntrusiveRingQueue<T, N>::SizeType
IntrusiveRingQueue<T, N>::canEnqueue(SizeType writerIndex, SizeType readerIndex) const noexcept {
    return capacity() - canDequeue(writerIndex, readerIndex);
}

template<typename T, std::size_t N>
constexpr typename IntrusiveRingQueue<T, N>::SizeType
IntrusiveRingQueue<T, N>::canDequeue(SizeType writerIndex, SizeType readerIndex) const noexcept {
    if (writerIndex > readerIndex) {
        return writerIndex - readerIndex;
    } else if (writerIndex == readerIndex) {
        return 0;
    } else {
        return readerIndex - writerIndex;
    }
}

} //! namespace atom::containers::lock_free

#endif //! ATOMC_CONTAINERS_LOCK_FREE_SPSC_H
