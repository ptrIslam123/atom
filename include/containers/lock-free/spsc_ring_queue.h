#ifndef ATOMC_CONTAINERS_LOCK_FREE_SPSC_RING_QUEUE_H
#define ATOMC_CONTAINERS_LOCK_FREE_SPSC_RING_QUEUE_H

#include <algorithm>
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
#include <cstring>

namespace atom::containers::lock_free::spsc {

/**
 * @brief Single-producer/single-consumer lock-free ring queue.
 *
 * The queue uses caller-provided, contiguous storage and a monotonically
 * increasing index for every producer and consumer operation. Indices are not
 * wrapped; the physical offset is obtained with a bit mask. This keeps the
 * fast paths free of division and makes full/empty detection unambiguous.
 *
 * The byte specialization is the low-level engine. It exposes the ring as one
 * or two contiguous spans so that data which crosses the end of the physical
 * buffer can still be written or read with a single operation.
 *
 * @tparam T Stored element type. Only the @c std::byte specialization is
 *         defined as a low-level engine; all other types use the typed
 *         specialization below.
 */
template<typename T>
class RingQueue;

/**
 * @brief Byte-oriented SPSC ring queue over externally owned storage.
 *
 * The queue is lock-free for any single producer and any single consumer.
 * @c tryWrite() must only be called by the producer thread, and @c tryRead()
 * must only be called by the consumer thread. Query methods may be called from
 * either thread but return an approximate snapshot.
 *
 * The buffer is not owned by the queue and must remain valid for the entire
 * lifetime of the queue.
 *
 * Callbacks are invoked as @c func(first, second, queueSize), where
 * @c queueSize is the approximate number of bytes currently queued before the
 * operation. Callbacks accepting only @c func(first, second) remain supported.
 *
 * @warning The storage must be at least 1 byte long and its size must be a
 *          power of two.
 *
 * @note Except for the validating constructor, member functions are
 *       @c noexcept. Invalid constructor arguments throw
 *       @c std::invalid_argument.
 */
template<>
class RingQueue<std::byte> {
public:
    /** @brief Type used for logical indices and byte counts. */
    using SizeType = std::uintmax_t;

    static_assert(std::atomic<SizeType>::is_always_lock_free,
                  "atomic<uintmax_t> must be lock-free");

    RingQueue(const RingQueue&) = delete;
    RingQueue(RingQueue&&) = delete;
    RingQueue& operator=(const RingQueue&) = delete;
    RingQueue& operator=(RingQueue&&) = delete;

    /**
     * @brief Constructs a queue over an existing byte buffer.
     *
     * @param buffer Non-null storage used by the queue.
     * @param capacity Number of bytes in @p buffer. Must be a power of two.
     * @throws std::invalid_argument If @p buffer is null, or @p capacity is
     *         zero or not a power of two.
     */
    RingQueue(std::byte* buffer, std::size_t capacity);

    ~RingQueue() noexcept = default;

    /**
     * @brief Writes exactly @p n bytes, waiting until enough space is free.
     *
     * This function spins with @c std::this_thread::yield() while the queue is
     * full. It must only be called from the producer thread.
     *
     * @param n Number of bytes to write. Must not exceed @c capacity().
     * @param func Callable invoked as @c func(first, second, queueSize) or
     *             @c func(first, second). The two spans together describe
     *             exactly @p n continuous bytes of writable
     *             uninitialized storage. Either span may be empty, but the
     *             first span is non-empty when @p n is non-zero.
     */
    template<typename Func>
    void write(SizeType n, Func&& func) noexcept;

    /**
     * @brief Writes exactly @p n bytes, waiting for at most @p timeout.
     *
     * The operation is attempted at least once. If it cannot complete before
     * the timeout expires, the function returns @c false without writing.
     *
     * @param n Number of bytes to write.
     * @param func Callable invoked as @c func(first, second, queueSize) or
     *             @c func(first, second).
     * @param timeout Maximum time to wait for free space.
     * @return @c true if the write completed, @c false on timeout.
     */
    template<typename Rep, typename Period, typename Func>
    [[nodiscard]] bool write(SizeType n, Func&& func,
                             std::chrono::duration<Rep, Period> timeout) noexcept;

    /**
     * @brief Attempts to write exactly @p n bytes without blocking.
     *
     * The callback is invoked only when enough space is available. The write
     * becomes visible to the consumer only after the callback returns.
     *
     * @param n Number of bytes to write.
     * @param func Callable invoked as @c func(first, second, queueSize) or
     *             @c func(first, second).
     * @return @c true on success, @c false if the queue does not currently
     *         have enough space or @p n exceeds @c capacity().
     */
    template<typename Func>
    [[nodiscard]] bool tryWrite(SizeType n, Func&& func)
        noexcept;

    /**
     * @brief Attempts to copy a byte span into the queue without blocking.
     *
     * This is a convenience wrapper around the callback-based operation.
     *
     * @param data Bytes to copy into the queue.
     * @return @c true on success, @c false if the queue does not currently
     *         have enough space or @p data is larger than @c capacity().
     */
    [[nodiscard]] bool tryWrite(std::span<const std::byte> data) noexcept;

    /**
     * @brief Writes all bytes from a span, waiting until enough space is free.
     *
     * @param data Bytes to copy into the queue.
     */
    void write(std::span<const std::byte> data) noexcept;

    /**
     * @brief Writes all bytes from a span, waiting for at most @p timeout.
     *
     * @param data Bytes to copy into the queue.
     * @param timeout Maximum time to wait for free space.
     * @return @c true if the write completed, @c false on timeout.
     */
    template<typename Rep, typename Period>
    [[nodiscard]] bool write(std::span<const std::byte> data,
                             std::chrono::duration<Rep, Period> timeout) noexcept;

    /**
     * @brief Attempts to read exactly @p n bytes without blocking.
     *
     * The callback is invoked only when enough data is available. The bytes
     * remain owned by the queue until the callback returns; the consumer index
     * is advanced afterwards.
     *
     * @param n Number of bytes to read.
     * @param func Callable invoked as @c func(first, second, queueSize) or
     *             @c func(first, second).
     * @return @c true on success, @c false if fewer than @p n bytes are
     *         currently available or @p n exceeds @c capacity().
     */
    template<typename Func>
    [[nodiscard]] bool tryRead(SizeType n, Func&& func) noexcept;

    /**
     * @brief Reads exactly @p n bytes, waiting until enough data is available.
     *
     * This function spins with @c std::this_thread::yield() while the queue is
     * empty. It must only be called from the consumer thread.
     *
     * @param n Number of bytes to read. Must not exceed @c capacity().
     * @param func Callable invoked as @c func(first, second, queueSize) or
     *             @c func(first, second). The two spans together describe
     *             exactly @p n continuous bytes of readable
     *             storage. Either span may be empty, but the first span is
     *             non-empty when @p n is non-zero.
     */
    template<typename Func>
    void read(SizeType n, Func&& func) noexcept;

    /**
     * @brief Reads exactly @p n bytes, waiting for at most @p timeout.
     *
     * The operation is attempted at least once. If it cannot complete before
     * the timeout expires, the function returns @c false without consuming
     * any data.
     *
     * @param n Number of bytes to read.
     * @param func Callable invoked as @c func(first, second, queueSize) or
     *             @c func(first, second).
     * @param timeout Maximum time to wait for data.
     * @return @c true if the read completed, @c false on timeout.
     */
    template<typename Rep, typename Period, typename Func>
    [[nodiscard]] bool read(SizeType n, Func&& func,
                            std::chrono::duration<Rep, Period> timeout) noexcept;

    /**
     * @brief Attempts to copy bytes from the queue into a span without
     *        blocking.
     *
     * This is a convenience wrapper around the callback-based operation.
     *
     * @param data Destination buffer.
     * @return @c true on success, @c false if fewer than @c data.size() bytes
     *         are currently available or @p data is larger than
     *         @c capacity().
     */
    [[nodiscard]] bool tryRead(std::span<std::byte> data) noexcept;

    /**
     * @brief Reads exactly @p n bytes into a span, waiting for enough data.
     *
     * Only the first @p n bytes of @p data are written. The operation is
     * all-or-nothing and must only be used when @p n does not exceed both
     * @c capacity() and @c data.size().
     *
     * @param data Destination buffer.
     * @param n Number of bytes to read.
     */
    void read(std::span<std::byte> data, SizeType n) noexcept;

    /**
     * @brief Reads @c data.size() bytes into a span, waiting for enough data.
     *
     * @param data Destination buffer.
     */
    void read(std::span<std::byte> data) noexcept;

    /**
     * @brief Reads @c data.size() bytes into a span, waiting for at most
     *        @p timeout.
     *
     * @param data Destination buffer.
     * @param timeout Maximum time to wait for data.
     * @return @c true if the read completed, @c false on timeout.
     */
    template<typename Rep, typename Period>
    [[nodiscard]] bool read(std::span<std::byte> data,
                            std::chrono::duration<Rep, Period> timeout) noexcept;

    /**
     * @brief Reads exactly @p n bytes into a span, waiting for at most
     *        @p timeout.
     *
     * Only the first @p n bytes of @p data are written. The operation is
     * attempted at least once.
     *
     * @param data Destination buffer.
     * @param n Number of bytes to read.
     * @param timeout Maximum time to wait for data.
     * @return @c true if the read completed, @c false on timeout.
     */
    template<typename Rep, typename Period>
    [[nodiscard]] bool read(std::span<std::byte> data, SizeType n,
                            std::chrono::duration<Rep, Period> timeout) noexcept;

    /**
     * @brief Returns an approximate number of bytes available for reading.
     *
     * The result is a snapshot and may be stale as soon as it is returned.
     */
    [[nodiscard]] SizeType size() const noexcept;

    /** @brief Returns the total number of bytes in the backing buffer. */
    [[nodiscard]] std::size_t capacity() const noexcept;

    /**
     * @brief Returns an approximate number of bytes that can still be written.
     *
     * The result is a snapshot and may be stale as soon as it is returned.
     */
    [[nodiscard]] SizeType availableSpace() const noexcept;

    /** @brief Returns @c true when no bytes are available for reading. */
    [[nodiscard]] bool isEmpty() const noexcept;

    /** @brief Returns @c true when no more bytes can be written. */
    [[nodiscard]] bool isFull() const noexcept;

private:
    [[nodiscard]] static std::span<std::byte>
    validateBuffer(std::byte* buffer, std::size_t capacity);

    static constexpr std::size_t CACHELINE_SIZE{64};

    struct alignas(CACHELINE_SIZE) ProducerData {
        std::atomic<SizeType> index{0};
        SizeType cachedReader{0};
    };

    struct alignas(CACHELINE_SIZE) ConsumerData {
        std::atomic<SizeType> index{0};
        SizeType cachedWriter{0};
    };

    ProducerData m_producer;
    ConsumerData m_consumer;
    std::span<std::byte> m_buffer;
    std::size_t m_mask;
};

/**
 * @brief Typed SPSC ring queue built on top of the byte-oriented engine.
 *
 * Objects are constructed directly in the ring storage and destroyed when they
 * are dequeued or when the queue is destroyed. The storage must stay valid for
 * the lifetime of the queue and must be aligned for @c T.
 *
 * @tparam T Object type stored in the queue. The queue supports non-trivial
 *         types whose constructor, move assignment, and destructor obey the
 *         documented @c noexcept requirements.
 *
 * @note @c sizeof(T) must be a power of two, and the supplied buffer capacity
 *       must be a power-of-two multiple of @c sizeof(T). This keeps the
 *       logical capacity a power of two, so the fast bit-mask indexing path
 *       can be used. Bulk operations may split their destination into two
 *       non-contiguous spans, but an individual @c T is never split.
 *
 * @note Except for the validating constructor, member functions are
 *       @c noexcept. Invalid constructor arguments throw
 *       @c std::invalid_argument.
 */
template<typename T>
class RingQueue final : private RingQueue<std::byte> {
public:
    /** @brief Stored object type. */
    using ValueType = T;

    /** @brief Type used for element counts and logical indices. */
    using SizeType = typename RingQueue<std::byte>::SizeType;

    static_assert(std::is_object_v<T>, "T must be an object type");
    static_assert(std::has_single_bit(sizeof(T)), "sizeof(T) must be a power of two");
    static_assert(std::is_constructible_v<T>, "T must be constructible");
    static_assert(std::is_nothrow_constructible_v<T>,
                  "T must have a non-throwing constructor");
    static_assert(std::is_nothrow_copy_constructible_v<T>,
                  "T must have a non-throwing copy constructor");
    static_assert(std::is_nothrow_move_constructible_v<T>,
                  "T must have a non-throwing move constructor");
    static_assert(std::is_nothrow_move_assignable_v<T>,
                  "T must have a non-throwing move assignment");
    static_assert(std::is_destructible_v<T>, "T must be destructible");
    static_assert(std::is_nothrow_destructible_v<T>,
                  "T must have a non-throwing destructor");

    RingQueue(const RingQueue&) = delete;
    RingQueue(RingQueue&&) = delete;
    RingQueue& operator=(const RingQueue&) = delete;
    RingQueue& operator=(RingQueue&&) = delete;

    /**
     * @brief Constructs a typed queue over an existing byte buffer.
     *
     * @param buffer Storage used by the queue. It must be aligned for @c T.
     * @param capacity Number of bytes in @p buffer. It must be a power of two,
     *                 at least @c sizeof(T), and a multiple of @c sizeof(T).
     * @throws std::invalid_argument If a constructor precondition is violated.
     */
    RingQueue(std::byte* buffer, std::size_t capacity);

    /**
     * @brief Destroys all objects still present in the queue.
     *
     * The queue must not be used concurrently with its destruction.
     */
    ~RingQueue() noexcept;

    /**
     * @brief Constructs an object in the queue without blocking.
     *
     * @param args Arguments forwarded to the constructor of @c T.
     * @return @c true on success, @c false if the queue is full.
     */
    template<typename... Args>
    [[nodiscard]] bool tryEmplace(Args&&... args) noexcept;

    /**
     * @brief Copies an object into the queue without blocking.
     *
     * @return @c true on success, @c false if the queue is full.
     */
    [[nodiscard]] bool tryWrite(const T& data) noexcept;

    /**
     * @brief Moves an object into the queue without blocking.
     *
     * @return @c true on success, @c false if the queue is full.
     */
    [[nodiscard]] bool tryWrite(T&& data) noexcept;

    /**
     * @brief Copies a span of objects into the queue without blocking.
     *
     * The operation is all-or-nothing: either every object is copied, or the
     * queue is left unchanged.
     *
     * @return @c true on success, @c false if there is not enough free space
     *         or @p data is larger than @c capacity().
     */
    [[nodiscard]] bool tryWrite(std::span<const T> data) noexcept;

    /**
     * @brief Copies a span of objects into the queue, waiting for free space.
     *
     * The operation is all-or-nothing and must only be used when
     * @c data.size() does not exceed @c capacity().
     *
     */
    void write(std::span<const T> data) noexcept;

    /**
     * @brief Copies a span of objects into the queue, waiting for at most
     *        @p timeout.
     *
     * The operation is attempted at least once. Timeout is only applied to
     * waiting for free space; an oversized span violates the precondition.
     *
     * @return @c true if the write completed, @c false on timeout.
     */
    template<typename Rep, typename Period>
    [[nodiscard]] bool write(std::span<const T> data,
                             std::chrono::duration<Rep, Period> timeout) noexcept;

    /**
     * @brief Constructs an object in the queue, waiting until space is free.
     *
     * @param args Arguments forwarded to the constructor of @c T.
     */
    template<typename... Args>
    void emplace(Args&&... args) noexcept;

    /**
     * @brief Copies an object into the queue, waiting until space is free.
     */
    void write(const T& data) noexcept;

    /**
     * @brief Copies an object into the queue, waiting for at most @p timeout.
     *
     * @return @c true if the write completed, @c false on timeout.
     */
    template<typename Rep, typename Period>
    [[nodiscard]] bool write(const T& data,
                             std::chrono::duration<Rep, Period> timeout) noexcept;

    /**
     * @brief Moves an object into the queue, waiting until space is free.
     */
    void write(T&& data) noexcept;

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
     * @brief Moves the front object out of the queue, waiting for data.
     */
    void read(T& data) noexcept;

    /**
     * @brief Moves the front object out of the queue, waiting for at most
     *        @p timeout.
     *
     * @return @c true if the read completed, @c false on timeout.
     */
    template<typename Rep, typename Period>
    [[nodiscard]] bool read(T& data,
                            std::chrono::duration<Rep, Period> timeout) noexcept;

    /**
     * @brief Moves a span of objects out of the queue without blocking.
     *
     * The operation is all-or-nothing: either every object is moved into
     * @p data and destroyed in the ring, or no objects are removed from the
     * queue.
     *
     * @return @c true on success, @c false if there are not enough objects or
     *         @p data is larger than @c capacity().
     */
    [[nodiscard]] bool tryRead(std::span<T> data) noexcept;

    /**
     * @brief Moves a span of objects out of the queue, waiting for data.
     *
     * The operation is all-or-nothing and must only be used when
     * @c data.size() does not exceed @c capacity().
     *
     */
    void read(std::span<T> data) noexcept;

    /**
     * @brief Moves a span of objects out of the queue, waiting for at most
     *        @p timeout.
     *
     * The operation is attempted at least once. Timeout is only applied to
     * waiting for data; an oversized span violates the precondition.
     *
     * @return @c true if the read completed, @c false on timeout.
     */
    template<typename Rep, typename Period>
    [[nodiscard]] bool read(std::span<T> data,
                            std::chrono::duration<Rep, Period> timeout) noexcept;

    /**
     * @brief Reads raw bytes from the queue without blocking.
     *
     * This is available only for trivially destructible @c T, because raw
     * reads advance the consumer index without running destructors. The byte
     * count must be a whole number of objects.
     *
     * @param n Number of bytes to read. Must be a multiple of @c sizeof(T).
     * @param func Callable invoked as @c func(first, second, queueSize) or
     *             @c func(first, second).
     * @return @c true on success, @c false if @p n is not a multiple of
     *         @c sizeof(T) or fewer than @p n bytes are available.
     */
    template<typename Func>
    [[nodiscard]] bool tryRead(SizeType n, Func&& func)
        noexcept;

    /** @brief Returns the approximate number of objects available to read. */
    [[nodiscard]] SizeType size() const noexcept;

    /** @brief Returns the maximum number of objects the queue can hold. */
    [[nodiscard]] SizeType capacity() const noexcept;

    /**
     * @brief Returns an approximate number of objects that can still be
     *        written.
     *
     * The result is a snapshot and may be stale as soon as it is returned.
     */
    [[nodiscard]] SizeType availableSpace() const noexcept;

    /** @brief Returns @c true when the queue contains no objects. */
    [[nodiscard]] bool isEmpty() const noexcept;

    /** @brief Returns @c true when the queue cannot accept another object. */
    [[nodiscard]] bool isFull() const noexcept;

private:
    using Base = RingQueue<std::byte>;

    [[nodiscard]] static std::size_t
    validateCapacity(std::byte* buffer, std::size_t capacity);
};

inline std::span<std::byte>
RingQueue<std::byte>::validateBuffer(std::byte* buffer,
                                     std::size_t capacity)
{
    if (buffer == nullptr) {
        throw std::invalid_argument("RingQueue: buffer must not be null");
    }
    if (capacity == 0 || (capacity & (capacity - 1)) != 0) {
        throw std::invalid_argument(
            "RingQueue: capacity must be a non-zero power of two");
    }
    return std::span<std::byte>{buffer, capacity};
}

inline RingQueue<std::byte>::RingQueue(std::byte* buffer,
                                       std::size_t capacity)
    : m_buffer(validateBuffer(buffer, capacity))
    , m_mask(capacity - 1)
{
}

template<typename Func>
void RingQueue<std::byte>::write(SizeType n, Func&& func) noexcept
{
    while (!tryWrite(n, func)) {
        std::this_thread::yield();
    }
}

template<typename Rep, typename Period, typename Func>
bool RingQueue<std::byte>::write(SizeType n, Func&& func,
                                 std::chrono::duration<Rep, Period> timeout) noexcept
{
    const auto start = std::chrono::steady_clock::now();
    for (;;) {
        if (tryWrite(n, func)) {
            return true;
        }
        if (std::chrono::steady_clock::now() - start >= timeout) {
            return false;
        }
        std::this_thread::yield();
    }
}

template<typename Func>
bool RingQueue<std::byte>::tryWrite(SizeType n, Func&& func)
    noexcept
{
    assert(n > 0 && n <= capacity());
    const auto writerIdx = m_producer.index.load(std::memory_order_relaxed);
    auto& cachedReaderIdx = m_producer.cachedReader;

    const auto tryCommit = [&]() -> bool {
        const auto used = writerIdx - cachedReaderIdx;
        if (capacity() - used < n) [[unlikely]] {
            return false;
        }

        const auto offset = static_cast<std::size_t>(writerIdx & m_mask);
        const auto first = std::min<SizeType>(n, static_cast<SizeType>(capacity() - offset));
        const auto second = n - first;

        detail::invokeCallback(
            func,
            std::span<std::byte>{m_buffer.data() + offset,
                                 static_cast<std::size_t>(first)},
            std::span<std::byte>{m_buffer.data(),
                                 static_cast<std::size_t>(second)},
            used);

        m_producer.index.store(writerIdx + n, std::memory_order_release);
        return true;
    };

    if (tryCommit()) [[likely]] {
        return true;
    }

    cachedReaderIdx = m_consumer.index.load(std::memory_order_acquire);
    return tryCommit();
}

template<typename Func>
bool RingQueue<std::byte>::tryRead(SizeType n, Func&& func)
    noexcept
{
    assert(n > 0 && n <= capacity());
    const auto readerIdx = m_consumer.index.load(std::memory_order_relaxed);
    auto& cachedWriterIdx = m_consumer.cachedWriter;

    const auto tryCommit = [&]() -> bool {
        const auto queued = cachedWriterIdx - readerIdx;
        if (queued < n) [[unlikely]] {
            return false;
        }

        const auto offset = static_cast<std::size_t>(readerIdx & m_mask);
        const auto first = std::min<SizeType>(
            n, static_cast<SizeType>(capacity() - offset));
        const auto second = n - first;

        detail::invokeCallback(
            func,
            std::span<std::byte>{m_buffer.data() + offset,
                                 static_cast<std::size_t>(first)},
            std::span<std::byte>{m_buffer.data(),
                                 static_cast<std::size_t>(second)},
            queued);

        m_consumer.index.store(readerIdx + n, std::memory_order_release);
        return true;
    };

    if (tryCommit()) [[likely]] {
        return true;
    }

    cachedWriterIdx = m_producer.index.load(std::memory_order_acquire);
    return tryCommit();
}

template<typename Func>
void RingQueue<std::byte>::read(SizeType n, Func&& func) noexcept
{
    while (!tryRead(n, func)) {
        std::this_thread::yield();
    }
}

template<typename Rep, typename Period, typename Func>
bool RingQueue<std::byte>::read(SizeType n, Func&& func,
                                std::chrono::duration<Rep, Period> timeout) noexcept
{
    const auto start = std::chrono::steady_clock::now();
    for (;;) {
        if (tryRead(n, func)) {
            return true;
        }
        if (std::chrono::steady_clock::now() - start >= timeout) {
            return false;
        }
        std::this_thread::yield();
    }
}

inline bool
RingQueue<std::byte>::tryWrite(std::span<const std::byte> data) noexcept
{
    return tryWrite(data.size(), [data](std::span<std::byte> first,
                                        std::span<std::byte> second) noexcept {
        std::memcpy(first.data(), data.data(), first.size());
        std::memcpy(second.data(), data.data() + first.size(), second.size());
    });
}

inline void
RingQueue<std::byte>::write(std::span<const std::byte> data) noexcept
{
    while (!tryWrite(data)) {
        std::this_thread::yield();
    }
}

template<typename Rep, typename Period>
inline bool
RingQueue<std::byte>::write(std::span<const std::byte> data,
                            std::chrono::duration<Rep, Period> timeout) noexcept
{
    const auto start = std::chrono::steady_clock::now();
    for (;;) {
        if (tryWrite(data)) {
            return true;
        }
        if (std::chrono::steady_clock::now() - start >= timeout) {
            return false;
        }
        std::this_thread::yield();
    }
}

inline bool
RingQueue<std::byte>::tryRead(std::span<std::byte> data) noexcept
{
    return tryRead(data.size(), [data](std::span<std::byte> first,
                                       std::span<std::byte> second) noexcept {
        std::memcpy(data.data(), first.data(), first.size());
        std::memcpy(data.data() + first.size(), second.data(), second.size());
    });
}

inline void
RingQueue<std::byte>::read(std::span<std::byte> data, SizeType n) noexcept
{
    while (!tryRead(n, [data](std::span<std::byte> first,
                              std::span<std::byte> second) noexcept {
        std::memcpy(data.data(), first.data(), first.size());
        std::memcpy(data.data() + first.size(), second.data(), second.size());
    })) {
        std::this_thread::yield();
    }
}

template<typename Rep, typename Period>
inline bool
RingQueue<std::byte>::read(std::span<std::byte> data, SizeType n,
                           std::chrono::duration<Rep, Period> timeout) noexcept
{
    const auto start = std::chrono::steady_clock::now();
    for (;;) {
        if (tryRead(n, [data](std::span<std::byte> first,
                              std::span<std::byte> second) noexcept {
            std::memcpy(data.data(), first.data(), first.size());
            std::memcpy(data.data() + first.size(), second.data(),
                        second.size());
        })) {
            return true;
        }
        if (std::chrono::steady_clock::now() - start >= timeout) {
            return false;
        }
        std::this_thread::yield();
    }
}

inline void
RingQueue<std::byte>::read(std::span<std::byte> data) noexcept
{
    read(data, data.size());
}

template<typename Rep, typename Period>
inline bool
RingQueue<std::byte>::read(std::span<std::byte> data,
                           std::chrono::duration<Rep, Period> timeout) noexcept
{
    return read(data, data.size(), timeout);
}

inline RingQueue<std::byte>::SizeType
RingQueue<std::byte>::size() const noexcept
{
    const auto readerIdx = m_consumer.index.load(std::memory_order_acquire);
    const auto writerIdx = m_producer.index.load(std::memory_order_acquire);
    return writerIdx - readerIdx;
}

inline std::size_t RingQueue<std::byte>::capacity() const noexcept
{
    return m_buffer.size();
}

inline RingQueue<std::byte>::SizeType
RingQueue<std::byte>::availableSpace() const noexcept
{
    return capacity() - size();
}

inline bool RingQueue<std::byte>::isEmpty() const noexcept
{
    return size() == 0;
}

inline bool RingQueue<std::byte>::isFull() const noexcept
{
    return size() == capacity();
}

template<typename T>
std::size_t RingQueue<T>::validateCapacity(std::byte* buffer,
                                            std::size_t capacity)
{
    if (buffer == nullptr) {
        throw std::invalid_argument("RingQueue: buffer must not be null");
    }
    if (capacity == 0 || (capacity & (capacity - 1)) != 0) {
        throw std::invalid_argument(
            "RingQueue: capacity must be a non-zero power of two");
    }
    if (capacity < sizeof(T)) {
        throw std::invalid_argument(
            "RingQueue: capacity is smaller than sizeof(T)");
    }
    if (capacity % sizeof(T) != 0) {
        throw std::invalid_argument(
            "RingQueue: capacity must be a multiple of sizeof(T)");
    }
    if (reinterpret_cast<std::uintptr_t>(buffer) % alignof(T) != 0) {
        throw std::invalid_argument(
            "RingQueue: buffer is not aligned for T");
    }
    return capacity;
}

template<typename T>
RingQueue<T>::RingQueue(std::byte* buffer, std::size_t capacity)
    : Base(buffer, validateCapacity(buffer, capacity))
{
}

template<typename T>
RingQueue<T>::~RingQueue() noexcept
{
    auto remaining = Base::size() / sizeof(T);
    while (remaining-- > 0) {
        const auto destroyed =
            Base::tryRead(sizeof(T),
                          [](std::span<std::byte> first,
                             std::span<std::byte>) noexcept {
                              std::destroy_at(
                                  std::launder(reinterpret_cast<T*>(first.data())));
                          });
        assert(destroyed);
        static_cast<void>(destroyed);
    }
}

template<typename T>
template<typename... Args>
bool RingQueue<T>::tryEmplace(Args&&... args)
    noexcept
{
    const auto construct = [&](std::span<std::byte> first,
                               std::span<std::byte>)
        noexcept {
        std::construct_at(reinterpret_cast<T*>(first.data()),
                          std::forward<Args>(args)...);
    };
    return Base::tryWrite(sizeof(T), construct);
}

template<typename T>
bool RingQueue<T>::tryWrite(const T& data)
    noexcept
{
    static_assert(std::is_copy_constructible_v<T>,
                  "T must be copy constructible");
    return tryEmplace(data);
}

template<typename T>
bool RingQueue<T>::tryWrite(T&& data)
    noexcept
{
    static_assert(std::is_move_constructible_v<T>,
                  "T must be move constructible");
    return tryEmplace(std::move(data));
}

template<typename T>
template<typename... Args>
void RingQueue<T>::emplace(Args&&... args) noexcept
{
    while (!tryEmplace(std::forward<Args>(args)...)) {
        std::this_thread::yield();
    }
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
    const auto start = std::chrono::steady_clock::now();
    for (;;) {
        if (tryWrite(data)) {
            return true;
        }
        if (std::chrono::steady_clock::now() - start >= timeout) {
            return false;
        }
        std::this_thread::yield();
    }
}

template<typename T>
template<typename Rep, typename Period>
bool RingQueue<T>::write(T&& data,
                         std::chrono::duration<Rep, Period> timeout) noexcept
{
    const auto start = std::chrono::steady_clock::now();
    for (;;) {
        if (tryWrite(std::move(data))) {
            return true;
        }
        if (std::chrono::steady_clock::now() - start >= timeout) {
            return false;
        }
        std::this_thread::yield();
    }
}

template<typename T>
bool RingQueue<T>::tryWrite(std::span<const T> data)
    noexcept
{
    static_assert(std::is_copy_constructible_v<T>,
                  "T must be copy constructible");
    if (data.size() > capacity()) [[unlikely]] {
        return false;
    }
    if (data.empty()) [[unlikely]] {
        return true;
    }

    const auto bytes = static_cast<SizeType>(data.size()) * sizeof(T);
    const auto construct = [&](std::span<std::byte> first, std::span<std::byte> second) noexcept {
        const auto firstCount = first.size() / sizeof(T);
        const auto secondCount = second.size() / sizeof(T);

        auto* firstObjects = reinterpret_cast<T*>(first.data());
        auto* secondObjects = reinterpret_cast<T*>(second.data());

        for (std::size_t i = 0; i < firstCount; ++i) {
            std::construct_at(firstObjects + i, data[i]);
        }
        for (std::size_t i = 0; i < secondCount; ++i) {
            std::construct_at(secondObjects + i, data[firstCount + i]);
        }
    };

    return Base::tryWrite(bytes, construct);
}

template<typename T>
void RingQueue<T>::write(std::span<const T> data) noexcept
{
    assert(data.size() <= capacity());
    while (!tryWrite(data)) {
        std::this_thread::yield();
    }
}

template<typename T>
template<typename Rep, typename Period>
bool RingQueue<T>::write(std::span<const T> data,
                         std::chrono::duration<Rep, Period> timeout) noexcept
{
    assert(data.size() <= capacity());

    const auto start = std::chrono::steady_clock::now();
    for (;;) {
        if (tryWrite(data)) {
            return true;
        }
        if (std::chrono::steady_clock::now() - start >= timeout) {
            return false;
        }
        std::this_thread::yield();
    }
}

template<typename T>
bool RingQueue<T>::tryRead(T& data)
    noexcept
{
    static_assert(std::is_move_assignable_v<T>,
                  "T must be move assignable");

    const auto consume = [&](std::span<std::byte> first,
                             std::span<std::byte>)
        noexcept {
        auto* object = std::launder(reinterpret_cast<T*>(first.data()));
        data = std::move(*object);
        std::destroy_at(object);
    };

    return Base::tryRead(sizeof(T), consume);
}

template<typename T>
void RingQueue<T>::read(T& data)
    noexcept
{
    while (!tryRead(data)) {
        std::this_thread::yield();
    }
}

template<typename T>
template<typename Rep, typename Period>
bool RingQueue<T>::read(T& data,
                        std::chrono::duration<Rep, Period> timeout) noexcept
{
    const auto start = std::chrono::steady_clock::now();
    for (;;) {
        if (tryRead(data)) {
            return true;
        }
        if (std::chrono::steady_clock::now() - start >= timeout) {
            return false;
        }
        std::this_thread::yield();
    }
}

template<typename T>
bool RingQueue<T>::tryRead(std::span<T> data)
    noexcept
{
    static_assert(std::is_move_assignable_v<T>,
                  "T must be move assignable");
    if (data.size() > capacity()) [[unlikely]] {
        return false;
    }
    if (data.empty()) [[unlikely]] {
        return true;
    }

    const auto consume = [&](std::span<std::byte> first,
                             std::span<std::byte> second)
        noexcept {
        const auto firstCount = first.size() / sizeof(T);
        const auto secondCount = second.size() / sizeof(T);

        auto* firstObjects = reinterpret_cast<T*>(first.data());
        auto* secondObjects = reinterpret_cast<T*>(second.data());

        for (std::size_t i = 0; i < firstCount; ++i) {
            data[i] = std::move(firstObjects[i]);
        }
        for (std::size_t i = 0; i < secondCount; ++i) {
            data[firstCount + i] = std::move(secondObjects[i]);
        }

        for (std::size_t i = 0; i < firstCount; ++i) {
            std::destroy_at(firstObjects + i);
        }
        for (std::size_t i = 0; i < secondCount; ++i) {
            std::destroy_at(secondObjects + i);
        }
    };

    return Base::tryRead(static_cast<SizeType>(data.size()) * sizeof(T),
                         consume);
}

template<typename T>
void RingQueue<T>::read(std::span<T> data) noexcept
{
    assert(data.size() <= capacity());
    while (!tryRead(data)) {
        std::this_thread::yield();
    }
}

template<typename T>
template<typename Rep, typename Period>
bool RingQueue<T>::read(std::span<T> data,
                        std::chrono::duration<Rep, Period> timeout) noexcept
{
    assert(data.size() <= capacity());

    const auto start = std::chrono::steady_clock::now();
    for (;;) {
        if (tryRead(data)) {
            return true;
        }
        if (std::chrono::steady_clock::now() - start >= timeout) {
            return false;
        }
        std::this_thread::yield();
    }
}

template<typename T>
template<typename Func>
bool RingQueue<T>::tryRead(SizeType n, Func&& func)
    noexcept
{
    static_assert(std::is_trivially_destructible_v<T>,
                  "raw byte reads require trivially destructible T");
    if (n % sizeof(T) != 0) [[unlikely]] {
        return false;
    }
    return Base::tryRead(n, std::forward<Func>(func));
}

template<typename T>
RingQueue<T>::SizeType RingQueue<T>::size() const noexcept
{
    return Base::size() / sizeof(T);
}

template<typename T>
RingQueue<T>::SizeType RingQueue<T>::capacity() const noexcept
{
    return Base::capacity() / sizeof(T);
}

template<typename T>
RingQueue<T>::SizeType RingQueue<T>::availableSpace() const noexcept
{
    return Base::availableSpace() / sizeof(T);
}

template<typename T>
bool RingQueue<T>::isEmpty() const noexcept
{
    return Base::isEmpty();
}

template<typename T>
bool RingQueue<T>::isFull() const noexcept
{
    return size() == capacity();
}

} // namespace atom::containers::lock_free::spsc

#endif // ATOMC_CONTAINERS_LOCK_FREE_SPSC_RING_QUEUE_H
