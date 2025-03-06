#ifndef TSAN_OBJECT_H
#define TSAN_OBJECT_H

#include <type_traits>
#include <utility>
#include <atomic>
#include <limits>
#include <cstring>
#include <cstdint>
#include <cassert>

#include "include/utils/assertion.h"

namespace atom::utils::tsan {

#ifndef NDEBUG

/**
 * @brief A runtime consistency checker for object access patterns.
 *
 * @tparam T Type of the wrapped value (must not be volatile)
 * @tparam C Type used for timestamp counters (default: uint32_t)
 *
 * This class wraps an object and detects concurrent access violations during runtime.
 * It does NOT provide thread safety, but rather verifies that no other thread
 * interferes with the object while it's being accessed (reads or writes).
 *
 * The mechanism uses timestamp counters to track operations:
 * - m_writers: Monitors write operations
 * - m_readers: Monitors read operations
 *
 * When inconsistent access is detected (another thread interfering during an operation),
 * the class will trigger a panic (crash) to surface the violation.
 *
 * Key characteristics:
 * - NOT a thread-synchronization primitive
 * - Pure runtime checking with no prevention
 * - accessMutable/accessImmutable verify exclusive access during operations
 * - Automatic timestamp generation for operation sequencing
 * - Panics on detected consistency violations
 * - Supports both mutable and immutable access patterns
 *
 * @warning Violations result in program termination via panic.
 */
template<typename T, typename C = std::uint32_t>
class Object {
public:
    static_assert(!std::is_volatile_v<T>, "Volatile types are not supported");

    //! The non-const, non-volatile version of the wrapped type
    using ValueType = std::remove_const_t<T>;

    //! Type used for timestamp counters
    using TimeStampType = C;

    //! Reference type for mutable access
    using MutableRefType = ValueType&;

    //! Reference type for immutable access
    using ImmutableRefType = const ValueType&;

    //! Maximum value before timestamp counter wraps around
    static constexpr auto TIMESTAMP_LIMIT = std::numeric_limits<TimeStampType>::max();

    /**
     * @brief Constructs the object with given arguments
     * @tparam Arg Argument types
     * @param arg Arguments forwarded to T's constructor
     */
    template<typename ... Arg>
    Object(Arg&& ... arg);

    /**
     * @brief Copy constructor
     * @param other Object to copy from
     * @note Creates new timestamp counters but copies the value
     */
    Object(const Object& other);

    /**
     * @brief Copy assignment
     * @param other Object to copy from
     * @return Reference to this object
     */
    Object& operator=(const Object& other);

    /**
     * @brief Move constructor
     * @param other Object to move from
     * @note Creates new timestamp counters but copies the value (strong exception guarantee)
     */
    Object(Object&& other) noexcept;

    /**
     * @brief Move assignment
     * @param other Object to move from
     * @return Reference to this object
     */
    Object& operator=(Object&& other) noexcept;

    ~Object() = default;

    /**
     * @brief Provides mutable access to the wrapped object
     * @tparam Func Callable type
     * @param f Callable that will receive T&
     * @warning May panic if concurrent access is detected
     *
     * Usage:
     * @code
     * obj.accessMutable([](auto& value) {
     *     value.modify();
     * });
     * @endcode
     */
    template<typename Func>
    void accessMutable(Func&& f);

    /**
     * @brief Provides immutable access to the wrapped object
     * @tparam Func Callable type
     * @param f Callable that will receive const T&
     * @warning May panic if concurrent write is detected
     *
     * Usage:
     * @code
     * obj.accessImmutable([](const auto& value) {
     *     value.inspect();
     * });
     * @endcode
     */
    template<typename Func>
    void accessImmutable(Func&& f) const;

    /**
     * @brief Sets a new value (move version)
     * @param newValue Value to move from
     * @warning This method is available if T is trivial type
     */
    void setValue(ValueType newValue);

    /**
     * @brief Gets a copy of the current value
     * @warning This method is available if T is trivial type
     * @return Copy of the wrapped value
     */
    T getValue() const;

private:
    /**
     * @brief Generates a new timestamp from the counter
     * @param counter Counter to use (m_writers or m_readers)
     * @return New timestamp value
     * @note Handles counter wrap-around when reaching TIMESTAMP_LIMIT
     */
    TimeStampType genTimestamp(std::atomic<TimeStampType>& counter) const;

    /**
     * @brief Gets the current timestamp value
     * @param counter Counter to check
     * @return Current timestamp value
     */
    TimeStampType getCurrentTimestamp(const std::atomic<TimeStampType>& counter) const;

    mutable std::atomic<TimeStampType> m_writers; //! Counter for write operations
    mutable std::atomic<TimeStampType> m_readers; //! Counter for read operations
    T m_value; //! The wrapped value
};


template<typename T, typename C>
template<typename ... Args>
Object<T, C>::Object(Args&& ... args):
m_writers(0),
m_readers(0),
m_value(std::forward<Args>(args) ...)
{}

template<typename T, typename C>
Object<T, C>::Object(const Object& other):
m_writers(0),
m_readers(0),
m_value(other.getValue())
{}

template<typename T, typename C>
Object<T, C>::Object(Object&& other) noexcept:
m_writers(0),
m_readers(0),
m_value(other.getValue())
{}

template<typename T, typename C>
Object<T, C>& Object<T, C>::operator=(const Object& other)
{
    setValue(other.getValue());
    return *this;
}

template<typename T, typename C>
Object<T, C>& Object<T, C>::operator=(Object&& other) noexcept {
    setValue(other.getValue());
    return *this;
}

template<typename T, typename C>
template<typename Func>
void Object<T, C>::accessImmutable(Func&& f) const
{
    static_assert(std::is_invocable_v<Func, const ValueType&> && "Func must accept const T&");
    const auto oldW = genTimestamp(m_writers);

    f(m_value);

    const auto currentW = getCurrentTimestamp(m_writers);

    PANIC(oldW != currentW);
}

template<typename T, typename C>
template<typename Func>
void Object<T, C>::accessMutable(Func&& f)
{
    static_assert((!std::is_const_v<T> && std::is_invocable_v<Func, ValueType&>) &&
                  "T must be mutable and Func must accept T&");
    const auto oldW = genTimestamp(m_writers);
    const auto oldR = genTimestamp(m_readers);

    f(m_value);

    const auto currentW = getCurrentTimestamp(m_writers);
    const auto currentR = getCurrentTimestamp(m_readers);

    PANIC(oldW != currentW || oldR != currentR);
}

template<typename T, typename C>
void Object<T, C>::setValue(ValueType newValue)
{
    static_assert(std::is_trivial_v<T> && !std::is_const_v<T>);
    accessMutable([newValue](T& value) {
        value = newValue;
    });
}

template<typename T, typename C>
T Object<T, C>::getValue() const
{
    static_assert(std::is_trivial_v<T>);
    T copy;
    accessImmutable([&copy](const T& value) {
        copy = value;
    });
    return copy;
}

template<typename T, typename C>
typename Object<T, C>::TimeStampType
Object<T, C>::genTimestamp(std::atomic<TimeStampType>& counter) const
{
    TimeStampType timestamp = 0;
    while (true) {
        timestamp = counter.fetch_add(1) + 1;
        if (timestamp < TIMESTAMP_LIMIT) {
            break;
        }

        counter.store(0);
    }

    return timestamp;
}

template<typename T, typename C>
typename Object<T, C>::TimeStampType
Object<T, C>::getCurrentTimestamp(const std::atomic<TimeStampType>& counter) const
{
    return counter.load();
}

#else

template<typename T, typename C = std::int32_t>
class Object {
public:
    static_assert(!std::is_volatile_v<T>);
    using ValueType = std::remove_const_t<T>;
    using TimeStampType = C;
    using MutableRefType = ValueType&;
    using ImmutableRefType = const ValueType&;

    static constexpr auto TIMESTAMP_LIMIT = std::numeric_limits<TimeStampType>::max();

    template<typename ... Arg>
    Object(Arg&& ... arg): m_value(std::forward<Arg>(arg) ...) {}
    Object(const Object& other) = default;
    Object& operator=(const Object& other) = default;
    Object(Object&& other) noexcept = default;
    Object& operator=(Object&& other) noexcept = default;
    ~Object() = default;

    template<typename Func>
    void accessMutable(Func f) { f(m_value); }

    template<typename Func>
    void accessImmutable(Func f) const { f(m_value); }

    void setValue(const ValueType& newValue) { m_value = newValue; }
    void setValue(ValueType&& newValue) { m_value = std::move(newValue); }
    ValueType getValue() const { return m_value; }

private:
    T m_value;
};

#endif //! NDEBUG

} //! namespace atom::utils::tsan

#endif //! TSAN_OBJECT_H
