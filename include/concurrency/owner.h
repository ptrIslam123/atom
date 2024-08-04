#ifndef REF_COUNTER_H
#define REF_COUNTER_H

#include <cstdint>
#include <cstring>

#include "include/concurrency/sync.h"
#include "include/utils/assertion.h"

namespace atom::concurrency {

template<typename T, typename C>
class BasicRef;

struct DefaultOwnerConfig final {
    using RefCountType = std::int32_t;
    static constexpr auto INVALID_REF_COUNT = RefCountType{-1};
};

template<typename T, typename C = DefaultOwnerConfig>
class BasicOwner final {
public:
    using ConfigType = C;
    using ValueType = T;
    using RefCountType = typename ConfigType::RefCountType;

    BasicOwner(const BasicOwner& ) = delete;
    BasicOwner& operator=(const BasicOwner& ) = delete;
    BasicOwner& operator=(BasicOwner&& ) noexcept = delete;

    template<typename ... Args>
    BasicOwner(Args&& ... args);
    BasicOwner(BasicOwner<T, C>&& other) noexcept;
    ~BasicOwner();

    BasicRef<T, C> getMutableRef();

private:
    friend BasicRef<T, C>;

    static constexpr auto INVALID_REF_COUNT = ConfigType::INVALID_REF_COUNT;

    void decrementRefCount();
    void incrementRefCount();

    MutableSync<RefCountType> m_refCount;
    T m_value;
};

template<typename T, typename C>
class BasicRef final {
public:
    using ConfigType = C;
    using ValueType = T;
    using RefCountType = typename ConfigType::RefCountType;
    using BasicOwnerType = BasicOwner<T, C>;
    using BasicOwnerPtrType = BasicOwnerType*;
    using MutableSyncBasicOwnerPtrType = MutableSync<BasicOwnerPtrType>;
    using MutableValueRefType = ValueType&; //typename MutableSyncBasicOwnerPtrType::MutableValueRefType;
    using ImmutableValueRefType = const ValueType&; //typename MutableSyncBasicOwnerPtrType::ImmutableValueRefType;

    BasicRef& operator=(const BasicRef& ) = delete;
    BasicRef(BasicRef&& ) noexcept = delete;
    BasicRef& operator=(BasicRef&& ) noexcept = delete;

    BasicRef(const BasicRef& other);
    ~BasicRef();

    template<typename Func>
    void accessMutable(Func f);

    template<typename Func>
    void accessImmutable(Func f);

    template<typename Func>
    void accessImmutable(Func f) const;

    void invalidate();

private:
    friend BasicRef<T, C> BasicOwner<T, C>::getMutableRef();

    static constexpr auto INVALID_REF_COUNT = ConfigType::INVALID_REF_COUNT;

    explicit BasicRef(BasicOwner<T, C>* rOwner);
    MutableSync<BasicOwner<T, C>*> m_rOwner;
};

template<typename T>
using Ref = BasicRef<T, DefaultOwnerConfig>;

template<typename T>
using Owner = BasicOwner<T, DefaultOwnerConfig>;

template<typename T, typename C>
template<typename ... Args>
BasicOwner<T, C>::BasicOwner(Args&& ... args):
m_refCount(0),
m_value(std::forward<Args>(args) ...)
{}

template<typename T, typename C>
BasicOwner<T, C>::~BasicOwner() {
    PANIC(m_refCount.getValue() > 0);
}

template<typename T, typename C>
BasicOwner<T, C>::BasicOwner(BasicOwner<T, C>&& other) noexcept:
m_refCount(0),
m_value() {
    other.m_refCount.accessMutable([&](RefCountType& otherRefCount) mutable {
        PANIC(otherRefCount != 0);
        m_value = std::move(other.m_value);
        otherRefCount = INVALID_REF_COUNT;
    });
}

template<typename T, typename C>
BasicRef<T, C> BasicOwner<T, C>::getMutableRef()
{
    incrementRefCount();
    return BasicRef<T, C>{ this };
}

template<typename T, typename C>
void BasicOwner<T, C>::decrementRefCount()
{
    m_refCount.accessMutable([](RefCountType& refCount) { --refCount; });
}

template<typename T, typename C>
void BasicOwner<T, C>::incrementRefCount()
{
    m_refCount.accessMutable([](RefCountType& refCount) { ++refCount; });
}

//////////////////

template<typename T, typename C>
BasicRef<T, C>::BasicRef(BasicOwner<T, C>* const rOwner):
m_rOwner(rOwner)
{}

template<typename T, typename C>
BasicRef<T, C>::~BasicRef() {
    invalidate();
}

template<typename T, typename C>
void BasicRef<T, C>::invalidate() {
    m_rOwner.accessMutable([](BasicOwner<T, C>*& rOwner) {
        rOwner->decrementRefCount();
        rOwner = nullptr;
    });
}

template<typename T, typename C>
BasicRef<T, C>::BasicRef(const BasicRef& other) {
    std::memcpy(this, &other, sizeof(other));
}

template<typename T, typename C>
template<typename Func>
void BasicRef<T, C>::accessImmutable(Func f) const
{
    static_assert(__details::IsCallableWithConstRef<T, Func>::value, "Func must accept const T&");
    m_rOwner.accessImmutable([&f](const BasicOwner<T, C>*& rOwner) {
        f(rOwner->m_value);
    });
}

template<typename T, typename C>
template<typename Func>
void BasicRef<T, C>::accessImmutable(Func f)
{
    static_assert(__details::IsCallableWithConstRef<T, Func>::value, "Func must accept const T&");
    static_assert(__details::IsCallableWithConstRef<BasicOwner<T, C>*, decltype([](const BasicOwner<T, C>*& rOwner) {})>::value,
            "Func must accept const T&");
    using MutableSyncBasicOwnerType = MutableSync<BasicOwner<T, C>*>;

    m_rOwner.accessImmutable([&f](const typename MutableSyncBasicOwnerType::ValueType& rOwner) {

    });

//    m_rOwner.accessImmutable([&f](const BasicOwner<T, C>*& rOwner) {

//    });
}

template<typename T, typename C>
template<typename Func>
void BasicRef<T, C>::accessMutable(Func f)
{
    m_rOwner.accessMutable([&f](BasicOwner<T, C>*& rOwner) {
        f(rOwner->m_value);
    });
}

} //! namespace atom::concurrency

#endif //! REF_COUNTER_H
