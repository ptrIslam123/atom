#ifndef RTE_COUNTER_H
#define RTE_COUNTER_H

#include <type_traits>
#include <thread>
#include <atomic>
#include <limits>
#include <cstdint>

namespace rte
{

enum class CounterId;
enum class CounterPolicy;
template <CounterId, typename T, CounterPolicy, auto... Args> class Counter;

enum class CounterId {
    None = 0,
};

/**
 * @brief Enum class defining available counter policies
 */
enum class CounterPolicy {
    //! Use simple atomic fetch_add/load operations
    AtomicBased,
    ///! Local thread counter for each thread and periodic counter merging into a global atomic counter
    ThreadLocalBased,
};

/**
 * @brief High-performance counter using thread-local buffering
 * @tparam Id unique id for class instance
 * @tparam T Counter value type
 * @tparam FlushFrequency Frequency of flushing thread-local values to global counter
 */
template <CounterId Id, typename T, T FlushFrequency>
using HighPerfCounter = Counter<Id, T, CounterPolicy::ThreadLocalBased, FlushFrequency>;

/**
 * @brief Heavy atomic counter with strict consistency
 * @tparam Id unique id for class instance
 * @tparam Id unique id for class instance
 * @tparam T Counter value type
 */
template <CounterId Id, typename T>
using HeavyCounter = Counter<Id, T, CounterPolicy::AtomicBased>;

/**
 * @brief Atomic-based counter implementation
 * @tparam T Unsigned integral type for counter
 */
template <CounterId Id, typename T>
class Counter<Id, T, CounterPolicy::AtomicBased> final {
public:
    static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>,
                  "T must be an integral type");

    static constexpr auto POLICY_TYPE = CounterPolicy::AtomicBased;

    explicit Counter(T data = T());
    void increment() noexcept;
    constexpr void flush() noexcept;
    [[nodiscard]] T load() const noexcept;
    Counter& operator++() noexcept;

private:
    std::atomic<T> m_counter{ 0 };
};

/**
 * @brief Thread-local buffered counter implementation
 * @tparam Id unique id for class instance
 * @tparam T Unsigned integral type for counter
 * @tparam F Flush frequency (how often to update global counter)
 * @details
 * - Each thread keeps its own local counter (thread_local T local_counter).
 * - Local changes are periodically flushed to the global atomic counter (m_global_counter) when
 *      `FLUSH_FREQUENCY` is reached or forced (force_flush).
 * - The load() method returns the current value of the global counter.
 * @warning
 * Problem:
 * load() returns only m_global_counter, but does not take into account the current local_counter values ​​in threads.
 * This means that load() may undercount up to `FLUSH_FREQUENCY` - 1 operations in each thread.
 *  Example:
 *      If `FLUSH_FREQUENCY` = 100, and in 10 threads local_counter = 99, then load() will undercount 990 increments.
 *  Solution:
 *      reduce `FLUSH_FREQUENCY` or use forced reset before load(), but reducing `FLUSH_FREQUENCY` leads to performance loss.
 */
template <CounterId Id, typename T, T F>
class Counter<Id, T, CounterPolicy::ThreadLocalBased, F> final {
public:
    static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>,
                  "T must be an integral type");

    static constexpr auto FLUSH_FREQUENCY = F;
    static constexpr auto POLICY_TYPE = CounterPolicy::ThreadLocalBased;

    explicit Counter(T data = T());
    void increment() noexcept;
    void flush() noexcept;
    [[nodiscard]] T load() const noexcept;
    Counter& operator++() noexcept;

private:
    static T& get_thread_local_counter();

    std::atomic<T> m_global_counter{ 0 };
};

/* Impl details */
template <CounterId Id, typename T, T F>
Counter<Id, T, CounterPolicy::ThreadLocalBased, F>::Counter(T data): 
m_global_counter(data) {}

template <CounterId Id, typename T, T F>
void Counter<Id, T, CounterPolicy::ThreadLocalBased, F>::increment() noexcept {
    auto &local_counter = get_thread_local_counter();
    ++local_counter;
    if (local_counter >= FLUSH_FREQUENCY) {
        m_global_counter.fetch_add(local_counter, std::memory_order_relaxed);
        local_counter = 0;
    }
}

template <CounterId Id, typename T, T F>
void Counter<Id, T, CounterPolicy::ThreadLocalBased, F>::flush() noexcept {
    auto &local_counter = get_thread_local_counter();
    m_global_counter.fetch_add(local_counter, std::memory_order_relaxed);
    local_counter = 0;
}

template <CounterId Id, typename T, T F>
T Counter<Id, T, CounterPolicy::ThreadLocalBased, F>::load() const noexcept {
    return m_global_counter.load(std::memory_order_relaxed);
}

template <CounterId Id, typename T, T F>
Counter<Id, T, CounterPolicy::ThreadLocalBased, F> &
Counter<Id, T, CounterPolicy::ThreadLocalBased, F>::operator++() noexcept {
    increment();
    return *this;
}

template <CounterId Id, typename T, T F>
T& Counter<Id, T, CounterPolicy::ThreadLocalBased, F>::get_thread_local_counter() {
    static thread_local T local_counter{ 0 };
    return local_counter;
}

template <CounterId Id, typename T>
Counter<Id, T, CounterPolicy::AtomicBased>::Counter(T data): 
m_counter(data) {}

template <CounterId Id, typename T>
void Counter<Id, T, CounterPolicy::AtomicBased>::increment() noexcept {
    m_counter.fetch_add(1, std::memory_order_relaxed);
}

template <CounterId Id, typename T>
constexpr void Counter<Id, T, CounterPolicy::AtomicBased>::flush() noexcept {}

template <CounterId Id, typename T>
T Counter<Id, T, CounterPolicy::AtomicBased>::load() const noexcept {
    return m_counter.load(std::memory_order_relaxed);
}

template <CounterId Id, typename T>
Counter<Id, T, CounterPolicy::AtomicBased>&
Counter<Id, T, CounterPolicy::AtomicBased>::operator++() noexcept {
    increment();
    return *this;
}

} //! namespace rte

#endif //! RTE_COUNTER_H