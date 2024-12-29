#ifndef REF_COUNTER_H
#define REF_COUNTER_H

#include "include/utils/assertion.h"
#include "include/utils/tsan_object.h"

#include <optional>
#include <cstring>
#include <cstdint>

namespace atom::utils {

#ifndef NDEBUG

template<typename T, typename C>
class BasicRef;

template<typename T, typename C>
class BasicMutableRef;

template<typename T, typename C>
class BasicMutableOwner;

template<typename T, typename C>
class BasicOwner;

template<typename T, typename C>
class BasicOwner {
public:
    using RefCountType = C;
    using ValueType = std::remove_cv_t<T>;
    using RefType = BasicRef<T, C>;

    BasicOwner(const BasicOwner& other) = delete;
    BasicOwner(BasicOwner&& other) noexcept = delete;
    BasicOwner& operator=(const BasicOwner& other) = delete;
    BasicOwner& operator=(BasicOwner&& other) noexcept = delete;

    template<typename ... Arg>
    BasicOwner(Arg&& ... arg);
    virtual ~BasicOwner();

    RefType getRef() const;
    RefType getRef();

protected:
    void incrementRefCount();
    void decrementRefCount();

    std::atomic<RefCountType> m_refCount;
    ValueType m_value;

private:
    friend BasicRef<ValueType, RefCountType>;
    friend BasicMutableRef<ValueType, RefCountType>;
};

template<typename T, typename C>
class BasicMutableOwner final : public BasicOwner<T, C> {
public:
    using RefCountType = C;
    using ValueType = std::remove_cv_t<T>;
    using MutableRefType = BasicMutableRef<T, C>;

    BasicMutableOwner(const BasicMutableOwner& other) = delete;
    BasicMutableOwner(BasicMutableOwner&& other) noexcept = delete;
    BasicMutableOwner& operator=(const BasicMutableOwner& other) = delete;
    BasicMutableOwner& operator=(BasicMutableOwner&& other) noexcept = delete;

    template<typename ... Args>
    BasicMutableOwner(Args&& ... args);
    ~BasicMutableOwner() = default;

    MutableRefType getMutableRef();

private:
    friend BasicMutableRef<ValueType, RefCountType>;
    friend BasicMutableRef<ValueType, RefCountType>;
};

template<typename T, typename C>
class BasicRef {
public:
    using OwnerType = BasicOwner<T, C>;
    BasicRef(const BasicRef& other);
    BasicRef(BasicRef&& other) noexcept;
    virtual ~BasicRef();

    BasicRef& operator=(const BasicRef& other) = delete;
    BasicRef& operator=(BasicRef&& other) noexcept = delete;

    template<typename F>
    void accessImmutable(F func);
    void invalidate();

    bool isValid() const;
    bool isValid();

protected:
    friend BasicOwner<T, C>;
    friend BasicMutableRef<T, C>;

private:
    explicit BasicRef(BasicOwner<T, C>* owner);
    
    tsan::MutableObject<BasicOwner<T, C>*> m_ref;
};

template<typename T, typename C>
class BasicMutableRef final : public BasicRef<T, C> {
public:
    BasicMutableRef(const BasicMutableRef& other);
    BasicMutableRef(BasicMutableRef&& other) noexcept;
    ~BasicMutableRef() = default;

    BasicMutableRef& operator=(const BasicMutableRef& other) = delete;
    BasicMutableRef& operator=(BasicMutableRef&& other) noexcept = delete;

    template<typename F>
    void accessMutable(F func);

private:
    friend BasicMutableOwner<T, C>;

    explicit BasicMutableRef(BasicOwner<T, C>* owner);
};

template<typename T, typename C>
template<typename ... Arg>
BasicOwner<T, C>::BasicOwner(Arg&& ... arg):
m_refCount(0),
m_value(std::forward<Arg>(arg) ...)
{}

template<typename T, typename C>
BasicOwner<T, C>::~BasicOwner() {
    // We can safely destroy the owner if the reference count is zero
    PANIC(m_refCount.load() > 0)
}

template<typename T, typename C>
BasicOwner<T, C>::RefType BasicOwner<T, C>::getRef() {
    incrementRefCount();
    return RefType{ this };
}

template<typename T, typename C>
void BasicOwner<T, C>::incrementRefCount() {
    (void)m_refCount.fetch_add(1);
}

template<typename T, typename C>
void BasicOwner<T, C>::decrementRefCount() {
    (void)m_refCount.fetch_sub(1);
}

/////

template<typename T, typename C>
template<typename ... Arg>
BasicMutableOwner<T, C>::BasicMutableOwner(Arg&& ... arg):
BasicOwner<T, C>(std::forward<Arg>(arg) ...) {}

template<typename T, typename C>
BasicMutableOwner<T, C>::MutableRefType BasicMutableOwner<T, C>::getMutableRef() {
    BasicOwner<T, C>::incrementRefCount();
    return MutableRefType{ this };
}


////// BasicRef

template<typename T, typename C>
BasicRef<T, C>::BasicRef(BasicOwner<T, C>* owner):
m_ref(owner) {}

template<typename T, typename C>
template<typename F>
void BasicRef<T, C>::accessImmutable(F func) {
    m_ref.accessImmutable([func](BasicOwner<T, C> *const& owner) {
        func(owner->m_value);
    });
}

template<typename T, typename C>
BasicRef<T, C>::BasicRef(const BasicRef& other): m_ref(other.m_ref.getValue()) {}

template<typename T, typename C>
BasicRef<T, C>::~BasicRef() { invalidate(); }

template<typename T, typename C>
void BasicRef<T, C>::invalidate() {
    BasicOwner<T, C>* ptr = nullptr;
    m_ref.accessMutable([ptr](BasicOwner<T, C>*& owner) mutable {
        ptr = owner;
        owner = nullptr;
    });

    if (ptr) {
        ptr->decrementRefCount();
    }
}

/////

template<typename T, typename C>
BasicMutableRef<T, C>::BasicMutableRef(BasicOwner<T, C>* owner):
BasicRef<T, C>(owner) {}

template<typename T, typename C>
BasicMutableRef<T, C>::BasicMutableRef(const BasicMutableRef& other):
BasicRef<T, C>(static_cast<const BasicMutableRef&>(other)) {}

template<typename T, typename C>
BasicMutableRef<T, C>::BasicMutableRef(BasicMutableRef&& other) noexcept:
BasicRef<T, C>(static_cast<BasicMutableRef&&>(other)) {}

template<typename T, typename C>
template<typename F>
void BasicMutableRef<T, C>::accessMutable(F func) {
    dynamic_cast<BasicMutableOwner<T, C>*>(BasicRef<T, C>::m_ref.getValue());
    
    // BasicRef<T, C>::m_ref.accessMutable([func](BasicOwner<T, C>*& owner) {
    //     func(owner->m_value);
    // });
}

template<typename T>
using Owner = BasicOwner<T, std::int32_t>;

template<typename T>
using MutableOwner = BasicMutableOwner<T, std::int32_t>;

template<typename T>
using Ref = BasicRef<T, std::int32_t>;

template<typename T>
using MutableRef = BasicMutableRef<T, std::int32_t>;


#else

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
    BasicOwner(Args&& ... args): m_value(std::forward<Args>(args) ...) {}
    BasicOwner(BasicOwner<T, C>&& other) noexcept { m_value = std::move(other.m_value); }
    ~BasicOwner() = default;

    BasicRef<T, C> getMutableRef() { return BasicRef<T, C>{m_value}; }

private:
    friend BasicRef<T, C>;

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
    using MutableValueRefType = ValueType&;
    using ImmutableValueRefType = const ValueType&;

    BasicRef& operator=(const BasicRef& ) = delete;
    BasicRef(BasicRef&& ) noexcept = delete;
    BasicRef& operator=(BasicRef&& ) noexcept = delete;

    BasicRef(const BasicRef& other) { m_rvalue = other.m_rvalue; }
    ~BasicRef() = default;

    template<typename Func>
    inline void accessMutable(Func f) { f(m_rvalue); }

    template<typename Func>
    inline void accessImmutable(Func f) { f(m_rvalue); }

    template<typename Func>
    inline void accessImmutable(Func f) const { f(m_rvalue); }

    inline void invalidate() {}

private:
    friend BasicRef<T, C> BasicOwner<T, C>::getMutableRef();

    explicit BasicRef(T& rvalue): m_rvalue(rvalue) {}
    T& m_rvalue;
};

template<typename T>
using Ref = BasicRef<T, DefaultOwnerConfig>;

template<typename T>
using Owner = BasicOwner<T, DefaultOwnerConfig>;

#endif //! NDEBUG

} //! namespace atom::utils

#endif //! REF_COUNTER_H
