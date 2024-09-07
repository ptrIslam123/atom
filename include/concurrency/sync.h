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

#ifdef NDEBUG
    
template<typename T, typename C>
class MutableSync;

template<typename T>
class Sync;

struct DefaultSyncConfig final {
    using TimeStampType = std::int16_t;

    static constexpr auto TIME_STAMP_LIMIT = std::numeric_limits<TimeStampType>::max();
    static constexpr auto INVALID_TIME_STAMP = TimeStampType{-1};
};

template<typename T>
struct SyncTraits;

template<typename T>
struct SyncTraits<Sync<T>> final {
    using SyncType = Sync<T>;
    using ValueType = T;
};

template<typename T, typename C>
struct SyncTraits<MutableSync<T, C>> final {
    using ConfigType = C;
    using TimeStampType = typename ConfigType::TimeStampType;

    using SyncType = MutableSync<T, C>;
    using ValueType = T;

    static constexpr auto TIME_STAMP_LIMIT = ConfigType::TIME_STAMP_LIMIT;
    static constexpr auto INVALID_TIME_STAMP = ConfigType::INVALID_TIME_STAMP;
};


template<typename T, typename C = DefaultSyncConfig>
class MutableSync final {
public:
    using TraitsType = SyncTraits<MutableSync<T>>;
    using ValueType = typename TraitsType::ValueType;
    using MutableValueRefType = ValueType&;
    using ImmutableValueRefType = const ValueType&;
    using ConfigType = typename TraitsType::ConfigType;
    using TimeStampType = typename TraitsType::TimeStampType;

    static constexpr auto TIME_STAMP_LIMIT = TraitsType::TIME_STAMP_LIMIT;
    static constexpr auto INVALID_TIME_STAMP = TraitsType::INVALID_TIME_STAMP;

    template<typename ... Args>
    MutableSync(Args&& ... args): m_value(std::forward<Args>(args) ...) {}
    ~MutableSync() = default;

    template<typename Func>
    inline void accessMutable(Func f) { f(m_value); }

    template<typename Func>
    inline void accessImmutable(Func f) { f(m_value); }

    inline void setValue(const T& newValue) { m_value = newValue; }
    inline void setValue(T&& newValue) { m_value = std::move(newValue); }

    inline T getValue() { return m_value; }
    inline T getValue() const { return m_value; }

private:
    T m_value;
};

template<typename T>
class Sync final {
public:
    using ValueType = T;
    using ImmutableValueRefType = const ValueType&;

    template<typename ... Args>
    Sync(Args&& ... args): m_value(std::forward<Args>(args) ...) {}
    Sync(Sync&& other) noexcept = default;
    Sync& operator=(Sync&& other) noexcept = default;
    Sync(const Sync& other) = default;
    Sync& operator=(const Sync& other) = default;
    ~Sync() = default;

    template<typename Func>
    inline void accessImmutable(Func f) { f(m_value); }

    template<typename Func>
    inline void accessImmutable(Func f) const { f(m_value); }

    inline T getValue() { return m_value; }
    inline T getValue() const { m_value; }

private:
    T m_value;
};

#else

template<typename T, typename C>
class MutableSync;

template<typename T>
class Sync;

struct DefaultSyncConfig final {
    using TimeStampType = std::int16_t;

    static constexpr auto TIME_STAMP_LIMIT = std::numeric_limits<TimeStampType>::max();
    static constexpr auto INVALID_TIME_STAMP = TimeStampType{-1};
};

template<typename T>
struct SyncTraits;


template<typename T>
struct SyncTraits<Sync<T>> final {
    using SyncType = Sync<T>;
    using ValueType = T;
};

template<typename T, typename C>
struct SyncTraits<MutableSync<T, C>> final {
    using ConfigType = C;
    using TimeStampType = typename ConfigType::TimeStampType;

    using SyncType = MutableSync<T, C>;
    using ValueType = T;

    static constexpr auto TIME_STAMP_LIMIT = ConfigType::TIME_STAMP_LIMIT;
    static constexpr auto INVALID_TIME_STAMP = ConfigType::INVALID_TIME_STAMP;
};


template<typename T, typename C = DefaultSyncConfig>
class MutableSync final {
public:
    using TraitsType = SyncTraits<MutableSync<T>>;
    using ValueType = typename TraitsType::ValueType;
    using MutableValueRefType = ValueType&;
    using ImmutableValueRefType = const ValueType&;
    using ConfigType = typename TraitsType::ConfigType;
    using TimeStampType = typename TraitsType::TimeStampType;

    static constexpr auto TIME_STAMP_LIMIT = TraitsType::TIME_STAMP_LIMIT;
    static constexpr auto INVALID_TIME_STAMP = TraitsType::INVALID_TIME_STAMP;

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
    TimeStampType genTimestamp(std::atomic<TimeStampType>& counter) const;
    TimeStampType getCurrentTimestamp(const std::atomic<TimeStampType>& counter) const;

    mutable std::atomic<TimeStampType> m_writers;
    mutable std::atomic<TimeStampType> m_readers;
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

    PANIC(currentW == INVALID_TIME_STAMP);
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

    PANIC(currentW == INVALID_TIME_STAMP);
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
typename MutableSync<T, C>::TimeStampType
MutableSync<T, C>::genTimestamp(std::atomic<TimeStampType>& counter) const
{
    TimeStampType timestamp = 0;
    while (true) {
        timestamp = counter.fetch_add(1) + 1;
        if (timestamp < TIME_STAMP_LIMIT) {
            break;
        }

        counter.store(0);
    }

    return timestamp;
}

template<typename T, typename C>
typename MutableSync<T, C>::TimeStampType
MutableSync<T, C>::getCurrentTimestamp(const std::atomic<TimeStampType>& counter) const
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

#endif

} //! namespace atom::concurrency::mem

#endif //! CONCURRENCY_SYNC_H
