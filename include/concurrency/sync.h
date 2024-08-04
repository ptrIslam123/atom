#ifndef CONCURRENCY_SYNC_H
#define CONCURRENCY_SYNC_H

#include <type_traits>
#include <utility>
#include <atomic>
#include <limits>
#include <functional>
#include <variant>
#include <cstring>
#include <cstdint>
#include <cassert>

#include "include/utils/assertion.h"

namespace atom::concurrency {

namespace __details {

// Helper type trait to detect if a type can be called with const T& or const T*&
template<typename, typename, typename = std::void_t<>>
struct IsCallableWithConstRef : std::false_type {};

template<typename T, typename Func>
struct IsCallableWithConstRef<T, Func, std::void_t<decltype(std::declval<Func>()(std::declval<const T&>()))>> : std::true_type {};

template<typename T, typename Func>
struct IsCallableWithConstRef<T*, Func, std::void_t<decltype(std::declval<Func>()(std::declval<const T*&>()))>> : std::true_type {};

} //! namespace __details

template<typename T, typename C>
class MutableSync;

template<typename T>
class Sync;

struct DefaultSyncConfig final {
    using TimeStampeType = std::int16_t;

    static constexpr auto TIME_STAMPE_LIMIT = std::numeric_limits<TimeStampeType>::max();
    static constexpr auto INVALID_TIME_STAMPE = TimeStampeType{-1};
};

template<typename T>
struct SyncTraits;


template<typename T>
struct SyncTraits<Sync<T>> final {
    using SyncType = Sync<T>;
    using ValueType = T;
//    using ValueType = std::enable_if_t<
//                        !std::is_pointer_v<std::remove_cv_t<T>> &&
//                        !std::is_reference_v<std::remove_cv_t<T>>, T>;
    //static constexpr auto SYNC_TYPE_SIZE = sizeof(Sync<T>);
};

template<typename T, typename C>
struct SyncTraits<MutableSync<T, C>> final {
    using ConfigType = C;
    using TimeStampeType = typename ConfigType::TimeStampeType;

    using SyncType = MutableSync<T, C>;
    using ValueType = T;
//    using ValueType = std::enable_if_t<
//                        !std::is_pointer_v<std::remove_cv_t<T>> &&
//                        !std::is_reference_v<std::remove_cv_t<T>>, T>;

    static constexpr auto TIME_STAMPE_LIMIT = ConfigType::TIME_STAMPE_LIMIT;
    static constexpr auto INVALID_TIME_STAMPE = ConfigType::INVALID_TIME_STAMPE;
    //static constexpr auto SYNC_TYPE_SIZE = sizeof(MutableSync<T, C>);
};


template<typename T, typename C = DefaultSyncConfig>
class MutableSync final {
public:
    using TraitsType = SyncTraits<MutableSync<T>>;
    using ValueType = typename TraitsType::ValueType;
    using MutableValueRefType = ValueType&;
    using ImmutableValueRefType = const ValueType&;
    using ConfigType = typename TraitsType::ConfigType;
    using TimeStampeType = typename TraitsType::TimeStampeType;

    static constexpr auto TIME_STAMPE_LIMIT = TraitsType::TIME_STAMPE_LIMIT;
    static constexpr auto INVALID_TIME_STAMPE = TraitsType::INVALID_TIME_STAMPE;

    template<typename ... Args>
    MutableSync(Args&& ... args);
    ~MutableSync() = default;

    template<typename Func>
    void accessMutable(Func f);

    template<typename Func>
    void accessImmutable(Func f);

    void setValue(const T& newValue);
    void setValue(T&& newValue);

    T getValue();
    T getValue() const;

private:
    TimeStampeType genTimestamp(std::atomic<TimeStampeType>& counter) const;
    TimeStampeType getCurrentTimestamp(const std::atomic<TimeStampeType>& counter) const;

    mutable std::atomic<TimeStampeType> m_writers;
    mutable std::atomic<TimeStampeType> m_readers;
    std::atomic<std::uint64_t> m_refCount;
    T m_value;
};

template<typename T>
class Sync final {
public:
    using ValueType = T;
    using ImmutableValueRefType = const ValueType&;

    template<typename ... Args>
    Sync(Args&& ... args);
    Sync(Sync&& other) noexcept = default;
    Sync& operator=(Sync&& other) noexcept = default;
    Sync(const Sync& other) = default;
    Sync& operator=(const Sync& other) = default;
    ~Sync() = default;

    template<typename Func>
    void accessImmutable(Func f);

    template<typename Func>
    void accessImmutable(Func f) const;

    T getValue();
    T getValue() const;

private:
    T m_value;
};

/* start class MutableSync<T> */

template<typename T, typename C>
template<typename ... Args>
MutableSync<T, C>::MutableSync(Args&& ... args):
m_writers(0),
m_readers(0),
m_value(std::forward<Args>(args) ...)
{}

template<typename T, typename C>
template<typename Func>
void MutableSync<T, C>::accessImmutable(Func f)
{
    static_assert(__details::IsCallableWithConstRef<T, Func>::value, "Func must accept const T&");
    const auto oldW = genTimestamp(m_writers);

    f(m_value);

    const auto currentW = getCurrentTimestamp(m_writers);

    PANIC(currentW == INVALID_TIME_STAMPE);
    PANIC(oldW != currentW);
}

template<typename T, typename C>
template<typename Func>
void MutableSync<T, C>::accessMutable(Func f)
{
    const auto oldW = genTimestamp(m_writers);
    const auto oldR = genTimestamp(m_readers);

    f(m_value);

    const auto currentW = getCurrentTimestamp(m_writers);
    const auto currentR = getCurrentTimestamp(m_readers);

    PANIC(currentW == INVALID_TIME_STAMPE);
    PANIC(oldW != currentW && oldR == currentR);
}

template<typename T, typename C>
void MutableSync<T, C>::setValue(const T& newValue)
{
    accessMutable([&newValue](T& value) {
        value = newValue;
    });
}

template<typename T, typename C>
void MutableSync<T, C>::setValue(T&& newValue)
{
    accessMutable([newValue = std::move(newValue)](T&& value) {
        value = std::move(newValue);
    });
}

template<typename T, typename C>
T MutableSync<T, C>::getValue()
{
    T copy;
    accessImmutable([&copy](const T& value) {
        copy = value;
    });

    return copy;
}

template<typename T, typename C>
T MutableSync<T, C>::getValue() const
{
    T copy;
    accessImmutable([&copy](const T& value) {
        copy = value;
    });

    return copy;
}

template<typename T, typename C>
typename MutableSync<T, C>::TimeStampeType
MutableSync<T, C>::genTimestamp(std::atomic<TimeStampeType>& counter) const
{
    TimeStampeType timestamp = 0;
    while (true) {
        timestamp = counter.fetch_add(1) + 1;
        if (timestamp < TIME_STAMPE_LIMIT) {
            break;
        }

        counter.store(0);
    }

    return timestamp;
}

template<typename T, typename C>
typename MutableSync<T, C>::TimeStampeType
MutableSync<T, C>::getCurrentTimestamp(const std::atomic<TimeStampeType>& counter) const
{
    return counter.load();
}

/* end class MutableSync<T> */

/* start class Sync<T> */

template<typename T>
template<typename ... Args>
Sync<T>::Sync(Args&& ... args):
m_value(std::forward<Args>(args) ...)
{}

template<typename T>
template<typename Func>
void Sync<T>::accessImmutable(Func f) const
{
    static_assert(__details::IsCallableWithConstRef<T, Func>::value, "Func must accept const T&");
    f(m_value);
}

template<typename T>
template<typename Func>
void Sync<T>::accessImmutable(Func f)
{
    static_assert(__details::IsCallableWithConstRef<T, Func>::value, "Func must accept const T&");
    f(m_value);
}

template<typename T>
T Sync<T>::getValue()
{
    return m_value;
}

template<typename T>
T Sync<T>::getValue() const
{
    return m_value;
}

/* end class Sync<T> */

} // namespace atom::concurrency::mem

#endif //! CONCURRENCY_SYNC_H
