#ifndef ATOMC_CONTAINERS_LOCK_FREE_MPMC_RING_QUEUE_H
#define ATOMC_CONTAINERS_LOCK_FREE_MPMC_RING_QUEUE_H

#include <atomic>
#include <bit>
#include <chrono>
#include <memory>
#include <new>
#include <span>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <utility>

#include <cassert>
#include <cstddef>
#include <cstdint>

namespace atom::containers::lock_free::mpmc {

namespace detail {

/// Cache line size used to isolate indices and slots from false sharing.
inline constexpr std::size_t kCacheLineSize = 64;

} // namespace detail

/**
 * @brief Bounded multi-producer/multi-consumer typed ring queue.
 *
 * Implements the Vyukov MPMC algorithm with one sequence counter per slot.
 * Every slot is padded to a cache line and stores its sequence next to the
 * payload, so a single operation touches one cache line per slot and neither
 * producer nor consumer indices share a line with anything else.
 *
 * A single @c T is never written and read concurrently: producers reserve a
 * contiguous range of logical positions with one relaxed CAS on the writer
 * index, then publish each slot with a release store after the payload has
 * been constructed. Consumers mirror this with the reader index.
 *
 * The queue owns its storage and allocates it once in the constructor; no
 * allocations happen on the hot path.
 *
 * @note Every slot occupies at least one full cache line (64 bytes), regardless
 *       of @c sizeof(T). For small element types the memory overhead per
 *       element is therefore significant (for example a @c RingQueue<std::byte>
 *       uses roughly 64 bytes per slot). Prefer this queue for moderately
 *       sized or larger objects, or accept the padding cost when choosing a
 *       capacity.
 *
 * @tparam T Stored element type. It must be nothrow destructible; individual
 *         operations additionally require nothrow construction (write/emplace)
 *         or nothrow move assignment (read).
 */
template<typename T>
class RingQueue final {
public:
    /** @brief Stored object type. */
    using ValueType = T;

    /** @brief Type used for element counts and logical indices. */
    using SizeType = std::size_t;

    static_assert(std::atomic<SizeType>::is_always_lock_free,
                  "RingQueue requires a lock-free std::atomic<std::size_t>");
    static_assert(std::is_object_v<T>, "T must be an object type");
    static_assert(std::is_nothrow_destructible_v<T>,
                  "T must have a non-throwing destructor");

    RingQueue(const RingQueue&) = delete;
    RingQueue(RingQueue&&) = delete;
    RingQueue& operator=(const RingQueue&) = delete;
    RingQueue& operator=(RingQueue&&) = delete;

    /**
     * @brief Constructs a queue with storage for @p capacity objects.
     *
     * @param capacity Number of slots. Must be a power of two and at least 2.
     *        A capacity of one is not supported: the per-slot sequence scheme
     *        cannot distinguish a written slot from a free slot in that case.
     * @throws std::invalid_argument If @p capacity is not a power of two of at
     *         least 2.
     */
    explicit RingQueue(SizeType capacity);

    /**
     * @brief Destroys all objects still present in the queue.
     *
     * The queue must not be used concurrently with its destruction.
     */
    ~RingQueue() noexcept;

    /**
     * @brief Constructs an object in the queue without blocking.
     *
     * @return @c true on success, @c false if the queue is full.
     */
    template<typename... Args>
    [[nodiscard]] bool tryEmplace(Args&&... args) noexcept;

    /** @brief Copies an object into the queue without blocking. */
    [[nodiscard]] bool tryWrite(const T& data) noexcept;

    /** @brief Moves an object into the queue without blocking. */
    [[nodiscard]] bool tryWrite(T&& data) noexcept;

    /**
     * @brief Copies a span of objects without blocking.
     *
     * The operation is all-or-nothing: on success every object is copied into
     * a contiguous range of slots; on failure the queue is unchanged.
     */
    [[nodiscard]] bool tryWrite(std::span<const T> data) noexcept;

    /**
     * @brief Constructs an object in the queue, waiting until space is free.
     */
    template<typename... Args>
    void emplace(Args&&... args) noexcept;

    /** @brief Copies an object into the queue, waiting until space is free. */
    void write(const T& data) noexcept;

    /** @brief Moves an object into the queue, waiting until space is free. */
    void write(T&& data) noexcept;

    /**
     * @brief Copies an object into the queue, waiting for at most @p timeout.
     *
     * @return @c true if the write completed, @c false on timeout.
     */
    template<typename Rep, typename Period>
    [[nodiscard]] bool write(const T& data,
                             std::chrono::duration<Rep, Period> timeout) noexcept;

    /**
     * @brief Moves an object into the queue, waiting for at most @p timeout.
     *
     * @return @c true if the write completed, @c false on timeout.
     */
    template<typename Rep, typename Period>
    [[nodiscard]] bool write(T&& data,
                             std::chrono::duration<Rep, Period> timeout) noexcept;

    /**
     * @brief Moves the front object out of the queue without blocking.
     *
     * On success the object is move-assigned into @p data and destroyed in the
     * ring storage.
     *
     * @return @c true on success, @c false if the queue is empty.
     */
    [[nodiscard]] bool tryRead(T& data) noexcept;

    /**
     * @brief Moves a span of objects out without blocking.
     *
     * The operation is all-or-nothing: either every object is moved into
     * @p data, or no objects are removed from the queue.
     */
    [[nodiscard]] bool tryRead(std::span<T> data) noexcept;

    /** @brief Moves the front object out, waiting for data. */
    void read(T& data) noexcept;

    /**
     * @brief Moves the front object out, waiting for at most @p timeout.
     *
     * @return @c true if the read completed, @c false on timeout.
     */
    template<typename Rep, typename Period>
    [[nodiscard]] bool read(T& data,
                            std::chrono::duration<Rep, Period> timeout) noexcept;

    /** @brief Returns the approximate number of objects available to read. */
    [[nodiscard]] SizeType size() const noexcept;

    /** @brief Returns the maximum number of objects the queue can hold. */
    [[nodiscard]] SizeType capacity() const noexcept;

    /** @brief Returns an approximate number of free slots. */
    [[nodiscard]] SizeType availableSpace() const noexcept;

    /** @brief Returns @c true when the queue appears empty. */
    [[nodiscard]] bool isEmpty() const noexcept;

    /** @brief Returns @c true when the queue appears full. */
    [[nodiscard]] bool isFull() const noexcept;

private:
    struct alignas(detail::kCacheLineSize) Slot {
        std::atomic<SizeType> sequence;
        alignas(T) std::byte storage[sizeof(T)];
    };

    [[nodiscard]] static SizeType validateCapacity(SizeType capacity);

    [[nodiscard]] Slot& slotAt(SizeType position) noexcept
    {
        return m_slots[position & m_mask];
    }

    [[nodiscard]] const Slot& slotAt(SizeType position) const noexcept
    {
        return m_slots[position & m_mask];
    }

    [[nodiscard]] bool reserveWrite(SizeType n, SizeType& position) noexcept;
    [[nodiscard]] bool reserveRead(SizeType n, SizeType& position) noexcept;

    template<typename TryOp>
    static void retryForever(TryOp&& op) noexcept
    {
        while (!op()) {
            std::this_thread::yield();
        }
    }

    template<typename TryOp, typename Rep, typename Period>
    static bool retryFor(TryOp&& op,
                         std::chrono::duration<Rep, Period> timeout) noexcept
    {
        const auto start = std::chrono::steady_clock::now();
        for (;;) {
            if (op()) {
                return true;
            }
            if (std::chrono::steady_clock::now() - start >= timeout) {
                return false;
            }
            std::this_thread::yield();
        }
    }

    static T* objectAt(Slot& slot) noexcept
    {
        return std::launder(reinterpret_cast<T*>(slot.storage));
    }

    SizeType m_capacity;
    SizeType m_mask;
    std::unique_ptr<Slot[]> m_slots;
    alignas(detail::kCacheLineSize) std::atomic<SizeType> m_writerIdx{0};
    alignas(detail::kCacheLineSize) std::atomic<SizeType> m_readerIdx{0};
};

template<typename T>
RingQueue<T>::SizeType RingQueue<T>::validateCapacity(SizeType capacity)
{
    if (capacity < 2 || !std::has_single_bit(capacity)) [[unlikely]] {
        throw std::invalid_argument(
            "RingQueue: capacity must be a power of two of at least 2");
    }
    return capacity;
}

template<typename T>
RingQueue<T>::RingQueue(SizeType capacity)
    : m_capacity(validateCapacity(capacity))
    , m_mask(m_capacity - 1)
    , m_slots(std::make_unique<Slot[]>(m_capacity))
{
    for (SizeType i = 0; i < m_capacity; ++i) {
        m_slots[i].sequence.store(i, std::memory_order_relaxed);
    }
}

template<typename T>
RingQueue<T>::~RingQueue() noexcept
{
    if constexpr (!std::is_trivially_destructible_v<T>) {
        auto reader = m_readerIdx.load(std::memory_order_relaxed);
        const auto writer = m_writerIdx.load(std::memory_order_relaxed);
        while (reader != writer) {
            std::destroy_at(objectAt(slotAt(reader)));
            ++reader;
        }
    }
}

template<typename T>
bool RingQueue<T>::reserveWrite(SizeType n, SizeType& position) noexcept
{
    for (;;) {
        position = m_writerIdx.load(std::memory_order_relaxed);
        bool retry = false;

        for (SizeType i = 0; i < n; ++i) {
            const auto sequence =
                slotAt(position + i).sequence.load(std::memory_order_acquire);
            const auto diff = static_cast<std::intptr_t>(sequence) -
                              static_cast<std::intptr_t>(position + i);

            if (diff == 0) [[likely]] {
                continue;
            }
            if (diff < 0) [[unlikely]] {
                return false;
            }
            retry = true;
            break;
        }

        if (retry) [[unlikely]] {
            continue;
        }

        if (m_writerIdx.compare_exchange_weak(
                position, position + n, std::memory_order_relaxed,
                std::memory_order_relaxed)) [[likely]] {
            return true;
        }
    }
}

template<typename T>
bool RingQueue<T>::reserveRead(SizeType n, SizeType& position) noexcept
{
    for (;;) {
        position = m_readerIdx.load(std::memory_order_relaxed);
        bool retry = false;

        for (SizeType i = 0; i < n; ++i) {
            const auto sequence =
                slotAt(position + i).sequence.load(std::memory_order_acquire);
            const auto diff = static_cast<std::intptr_t>(sequence) -
                              static_cast<std::intptr_t>(position + i + 1);

            if (diff == 0) [[likely]] {
                continue;
            }
            if (diff < 0) [[unlikely]] {
                return false;
            }
            retry = true;
            break;
        }

        if (retry) [[unlikely]] {
            continue;
        }

        if (m_readerIdx.compare_exchange_weak(
                position, position + n, std::memory_order_relaxed,
                std::memory_order_relaxed)) [[likely]] {
            return true;
        }
    }
}

template<typename T>
template<typename... Args>
bool RingQueue<T>::tryEmplace(Args&&... args) noexcept
{
    static_assert(std::is_nothrow_constructible_v<T, Args...>,
                  "T must have a non-throwing constructor for these arguments");

    SizeType position = 0;
    if (!reserveWrite(1, position)) [[unlikely]] {
        return false;
    }

    Slot& slot = slotAt(position);
    std::construct_at(reinterpret_cast<T*>(slot.storage),
                      std::forward<Args>(args)...);
    slot.sequence.store(position + 1, std::memory_order_release);
    return true;
}

template<typename T>
bool RingQueue<T>::tryWrite(const T& data) noexcept
{
    static_assert(std::is_nothrow_copy_constructible_v<T>,
                  "T must have a non-throwing copy constructor");
    return tryEmplace(data);
}

template<typename T>
bool RingQueue<T>::tryWrite(T&& data) noexcept
{
    static_assert(std::is_nothrow_move_constructible_v<T>,
                  "T must have a non-throwing move constructor");
    return tryEmplace(std::move(data));
}

template<typename T>
bool RingQueue<T>::tryWrite(std::span<const T> data) noexcept
{
    static_assert(std::is_nothrow_copy_constructible_v<T>,
                  "T must have a non-throwing copy constructor");

    const SizeType n = data.size();
    if (n == 0) [[unlikely]] {
        return true;
    }
    if (n > m_capacity) [[unlikely]] {
        return false;
    }

    SizeType position = 0;
    if (!reserveWrite(n, position)) [[unlikely]] {
        return false;
    }

    for (SizeType i = 0; i < n; ++i) {
        Slot& slot = slotAt(position + i);
        std::construct_at(reinterpret_cast<T*>(slot.storage), data[i]);
        slot.sequence.store(position + i + 1, std::memory_order_release);
    }
    return true;
}

template<typename T>
template<typename... Args>
void RingQueue<T>::emplace(Args&&... args) noexcept
{
    retryForever([&] {
        return tryEmplace(std::forward<Args>(args)...);
    });
}

template<typename T>
void RingQueue<T>::write(const T& data) noexcept
{
    emplace(data);
}

template<typename T>
void RingQueue<T>::write(T&& data) noexcept
{
    emplace(std::move(data));
}

template<typename T>
template<typename Rep, typename Period>
bool RingQueue<T>::write(const T& data,
                         std::chrono::duration<Rep, Period> timeout) noexcept
{
    return retryFor([&] { return tryWrite(data); }, timeout);
}

template<typename T>
template<typename Rep, typename Period>
bool RingQueue<T>::write(T&& data,
                         std::chrono::duration<Rep, Period> timeout) noexcept
{
    return retryFor([&] { return tryWrite(std::move(data)); }, timeout);
}

template<typename T>
bool RingQueue<T>::tryRead(T& data) noexcept
{
    static_assert(std::is_nothrow_move_assignable_v<T>,
                  "T must have a non-throwing move assignment");

    SizeType position = 0;
    if (!reserveRead(1, position)) [[unlikely]] {
        return false;
    }

    Slot& slot = slotAt(position);
    T* object = objectAt(slot);
    data = std::move(*object);
    std::destroy_at(object);
    slot.sequence.store(position + m_capacity, std::memory_order_release);
    return true;
}

template<typename T>
bool RingQueue<T>::tryRead(std::span<T> data) noexcept
{
    static_assert(std::is_nothrow_move_assignable_v<T>,
                  "T must have a non-throwing move assignment");

    const SizeType n = data.size();
    if (n == 0) [[unlikely]] {
        return true;
    }
    if (n > m_capacity) [[unlikely]] {
        return false;
    }

    SizeType position = 0;
    if (!reserveRead(n, position)) [[unlikely]] {
        return false;
    }

    for (SizeType i = 0; i < n; ++i) {
        Slot& slot = slotAt(position + i);
        T* object = objectAt(slot);
        data[i] = std::move(*object);
        std::destroy_at(object);
        slot.sequence.store(position + i + m_capacity,
                            std::memory_order_release);
    }
    return true;
}

template<typename T>
void RingQueue<T>::read(T& data) noexcept
{
    retryForever([&] { return tryRead(data); });
}

template<typename T>
template<typename Rep, typename Period>
bool RingQueue<T>::read(T& data,
                        std::chrono::duration<Rep, Period> timeout) noexcept
{
    return retryFor([&] { return tryRead(data); }, timeout);
}

template<typename T>
RingQueue<T>::SizeType RingQueue<T>::size() const noexcept
{
    // The reader index is sampled first and only moves forward, so the writer
    // index loaded afterwards is always >= it (no underflow). Readers may
    // advance between the loads, so clamp to keep the result within capacity.
    const auto reader = m_readerIdx.load(std::memory_order_acquire);
    const auto writer = m_writerIdx.load(std::memory_order_acquire);
    const auto pending = writer - reader;
    return pending < m_capacity ? pending : m_capacity;
}

template<typename T>
RingQueue<T>::SizeType RingQueue<T>::capacity() const noexcept
{
    return m_capacity;
}

template<typename T>
RingQueue<T>::SizeType RingQueue<T>::availableSpace() const noexcept
{
    return m_capacity - size();
}

template<typename T>
bool RingQueue<T>::isEmpty() const noexcept
{
    return size() == 0;
}

template<typename T>
bool RingQueue<T>::isFull() const noexcept
{
    return size() == m_capacity;
}

} // namespace atom::containers::lock_free::mpmc

#endif // ATOMC_CONTAINERS_LOCK_FREE_MPMC_RING_QUEUE_H
