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

namespace __details {

template<typename T, typename C>
class ImplMutableObject {
public:
    using ValueType = std::remove_cv_t<T>;
    using TimeStampType = C;
    using MutableValueRefType = ValueType&;
    using ImmutableValueRefType = const ValueType&;

    static constexpr auto TIMESTAMP_LIMIT = std::numeric_limits<TimeStampType>::max();
    static constexpr auto INVALID_TIMESTAMP = TimeStampType{-1};

    template<typename ... Arg>
    ImplMutableObject(Arg&& ... arg);
    ImplMutableObject(const ImplMutableObject& other);
    ImplMutableObject& operator=(const ImplMutableObject& other);
    ImplMutableObject(ImplMutableObject&& other) noexcept;
    ImplMutableObject& operator=(ImplMutableObject&& other) noexcept;
    ~ImplMutableObject() = default;

    template<typename Func>
    void accessMutable(Func f);

    template<typename Func>
    void accessImmutable(Func f) const;

    void setValue(const T& newValue);
    void setValue(T&& newValue);

    T getValue();
    T getValue() const;

private:
    TimeStampType genTimestamp(std::atomic<TimeStampType>& counter) const;
    TimeStampType getCurrentTimestamp(const std::atomic<TimeStampType>& counter) const;

    mutable std::atomic<TimeStampType> m_writers;
    mutable std::atomic<TimeStampType> m_readers;
    T m_value;
};


template<typename T, typename C>
template<typename ... Args>
ImplMutableObject<T, C>::ImplMutableObject(Args&& ... args):
m_writers(0),
m_readers(0),
m_value(std::forward<Args>(args) ...)
{}

template<typename T, typename C>
ImplMutableObject<T, C>::ImplMutableObject(const ImplMutableObject& other):
m_writers(0),
m_readers(0),
m_value(other.getValue())
{}

template<typename T, typename C>
ImplMutableObject<T, C>& ImplMutableObject<T, C>::operator=(const ImplMutableObject& other)
{
    setValue(other.getValue());
    return *this;
}

template<typename T, typename C>
ImplMutableObject<T, C>::ImplMutableObject(ImplMutableObject&& other) noexcept:
m_writers(0),
m_readers(0),
m_value(other.getValue()) {}

template<typename T, typename C>
ImplMutableObject<T, C>& ImplMutableObject<T, C>::operator=(ImplMutableObject&& other) noexcept {
    setValue(other.getValue());
    return *this;
}

template<typename T, typename C>
template<typename Func>
void ImplMutableObject<T, C>::accessImmutable(Func f) const
{
    static_assert(std::is_invocable_v<Func, ImmutableValueRefType>, "Func must accept const T&");
    const auto oldW = genTimestamp(m_writers);

    f(m_value);

    const auto currentW = getCurrentTimestamp(m_writers);

    PANIC(currentW == INVALID_TIMESTAMP);
    PANIC(oldW != currentW);
}

template<typename T, typename C>
template<typename Func>
void ImplMutableObject<T, C>::accessMutable(Func f)
{
    static_assert(std::is_invocable_v<Func, MutableValueRefType>, "Func must accept T&");
    const auto oldW = genTimestamp(m_writers);
    const auto oldR = genTimestamp(m_readers);

    f(m_value);

    const auto currentW = getCurrentTimestamp(m_writers);
    const auto currentR = getCurrentTimestamp(m_readers);

    PANIC(currentW == INVALID_TIMESTAMP);
    PANIC(oldW != currentW && oldR == currentR);
}

template<typename T, typename C>
void ImplMutableObject<T, C>::setValue(const T& newValue)
{
    accessMutable([&newValue](T& value) {
        value = newValue;
    });
}

template<typename T, typename C>
void ImplMutableObject<T, C>::setValue(T&& newValue)
{
    accessMutable([newValue = std::move(newValue)](T&& value) {
        value = std::move(newValue);
    });
}

template<typename T, typename C>
T ImplMutableObject<T, C>::getValue()
{
    T copy;
    accessImmutable([&copy](const T& value) {
        copy = value;
    });
    return copy;
}

template<typename T, typename C>
T ImplMutableObject<T, C>::getValue() const
{
    T copy;
    accessImmutable([&copy](const T& value) {
        copy = value;
    });
    return copy;
}

template<typename T, typename C>
typename ImplMutableObject<T, C>::TimeStampType
ImplMutableObject<T, C>::genTimestamp(std::atomic<TimeStampType>& counter) const
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
typename ImplMutableObject<T, C>::TimeStampType
ImplMutableObject<T, C>::getCurrentTimestamp(const std::atomic<TimeStampType>& counter) const
{
    return counter.load();
}

} //! namespace __details

template<typename T, typename C>
class BasicObject {
public:
    using ValueType = std::remove_cv_t<T>;
    using TimeStampType = C;
    using ImmutableValueRefType = const ValueType&;

    template<typename ... Arg>
    BasicObject(Arg&& ... arg): m_object(std::forward<Arg>(arg) ...) {}
    BasicObject(const BasicObject& other): m_object(other.m_object.getValue()) {}
    BasicObject& operator=(const BasicObject& other) {
        m_object.setValue(other.m_object.getValue());
        return *this;
    }
    BasicObject(BasicObject&& other) noexcept: m_object(other.m_object.getValue()) {}
    BasicObject& operator=(BasicObject&& other) noexcept {
        m_object.setValue(other.m_object.getValue());
        return *this;
    }
    virtual ~BasicObject() = default;

    template<typename Func>
    void accessImmutable(Func f) const { m_object.accessImmutable(f); }

    template<typename Func>
    void accessImmutable(Func f) { m_object.accessImmutable(f); }

    T getValue() const { return m_object.getValue(); }
    T getValue() { return m_object.getValue(); }

protected:
   __details::ImplMutableObject<ValueType, TimeStampType> m_object;
};

template<typename T, typename C>
class BasicMutableObject final : public BasicObject<T, C> {
public:
    using ValueType = std::remove_cv_t<T>;
    using TimeStampType = C;
    using ImmutableValueRefType = const ValueType&;
    using MutableValueRefType = ValueType&;

    template<typename ... Arg>
    BasicMutableObject(Arg&& ... arg): BasicObject<T, C>(std::forward<Arg>(arg) ...) {}

    template<typename Func>
    void accessMutable(Func f) { BasicObject<T, C>::m_object.accessMutable(f); }

    void setValue(const T& newValue) { BasicObject<T, C>::m_object.setValue(newValue); }
    void setValue(T&& newValue) { BasicObject<T, C>::m_object.setValue(std::move(newValue)); }
};

#else

template<typename T, typename C>
class BasicObject {
public:
   using ValueType = std::remove_cv_t<T>;
   using TimeStampType = C;
   using ImmutableValueRefType = const ValueType&;

   template<typename ... Arg>
   BasicObject(Arg&& ... arg): m_value(std::forward<Arg>(arg) ...) {}

   template<typename Func>
   inline void accessImmutable(Func f) const { f(m_value); }

   template<typename Func>
   inline void accessImmutable(Func f) { f(m_value); }

   inline ValueType getValue() const { return m_value; }
   inline ValueType getValue() { return m_value; }

protected:
   ValueType m_value;
};

template<typename T, typename C>
class BasicMutableObject final : public BasicObject<T, C> {
public:
   using ValueType = std::remove_cv_t<T>;
   using TimeStampType = C;
   using ImmutableValueRefType = const ValueType&;

   template<typename ... Arg>
   BasicMutableObject(Arg&& ... arg): BasicObject<T, C>(std::forward<Arg>(arg) ...) {}

   template<typename Func>
   inline void accessMutable(Func f) { f(BasicObject<T, C>::m_value); }

   inline void setValue(const T& newValue) { BasicObject<T, C>::m_value = newValue; }
   inline void setValue(T&& newValue) { BasicObject<T, C>::m_value = std::move(newValue); }
};

#endif //! NDEBUG

template<typename T>
using MutableObject = BasicMutableObject<T, std::int16_t>;

template<typename T>
using Object = BasicObject<T, std::int16_t>;

} //! namespace atom::utils::tsan

#endif //! TSAN_OBJECT_H
