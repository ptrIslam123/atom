#ifndef ATOMC_CONTAINERS_LOCK_FREE_MPMC_RING_QUEUE_H
#define ATOMC_CONTAINERS_LOCK_FREE_MPMC_RING_QUEUE_H

#include <span>
#include <array>
#include <atomic>

#include <cstddef>

namespace atom::containers::lock_free::fixed::mpmc {

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

    bool tryEnqueue(std::span<const std::byte> data, SizeType& available) noexcept {

    }

private:
    static constexpr auto CACHELINE_SIZE{64};
    static constexpr auto MASK{N - 1};

    alignas(CACHELINE_SIZE) std::atomic<SizeType> m_writerIndex{0};
    alignas(CACHELINE_SIZE) std::atomic<SizeType> m_readerIndex{0};
    alignas(CACHELINE_SIZE) std::array<std::byte, N> m_buffer;
};

} //! namespace atom::containers::lock_free::fixed::mpmc

#endif //! ATOMC_CONTAINERS_LOCK_FREE_MPMC_RING_QUEUE_H
