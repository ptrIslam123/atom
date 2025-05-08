#ifndef REF_COUNTER_H
#define REF_COUNTER_H

#include "include/utils/assertion.h"

#include <type_traits>
#include <utility>
#include <atomic>

namespace atom::utils::owner {

#ifndef NDEBUG

template<typename T, typename RC> class Reference;
template<typename T, typename RC> class Reference<const T, RC>;

/**
 * @class Object
 *
 * @brief The `Object` class manages the lifetime of an object and provides mechanisms for borrowing immutable or mutable references.
 * It uses link counting to track active borrowings to spot potential dangling link cases.
 *
 * @tparam T The type of the managed object. Must not be volatile.
 * @tparam RC The type used for reference counting (default is `std::uint32_t`).
 *
 * @warning This class does not provide thread-safe access/sharing of data,
 * this class provides a thread-safe mechanism for detecting potentially dangling references.
 */
template<typename T, typename RC = std::uint32_t>
class Object final {
public:
    static_assert(!std::is_volatile_v<T>);
    using RefCountType = RC; //! Type used for reference counting.
    using ValueType = std::remove_const_t<T>; //! Type of the managed object without const qualifier.
    using ReferenceType = ValueType&; //! Reference type to the managed object.
    using ConstReferenceType = const ValueType&; //! Const reference type to the managed object.
    using PointerType = ValueType*; //! Pointer type to the managed object.
    using ConstPointerType = const ValueType*; //! Const pointer type to the managed object.

    /**
     * @brief Constructs the `Object` with the provided arguments to initialize the managed object.
     *
     * This constructor forwards the given arguments to the constructor of the managed object.
     *
     * @tparam Arg Variadic template parameter pack for constructor arguments.
     * @param arg Arguments to forward to the constructor of the managed object.
     */
    template<typename... Arg>
    Object(Arg&&... arg);

    /**
     * @brief Deleted copy constructor.
     *
     * Copying an `Object` is not allowed to ensure single ownership semantics.
     * @details Why can't this class be moved or copied? There is no reason to leave the ability to copy/move the owner of the object
     * due to the fact that all references to the owner of the object are immediately invalidated.
     */
    Object(const Object& other) = delete;
    Object(Object&& other) = delete;
    Object& operator=(const Object& other) = delete;
    Object& operator=(Object&& other) = delete;

    /**
     * @brief Destructor.
     *
     * Destroys the `Object` and the managed object if there are no active borrows.
     * @warning The destructor may panic if there are still references to the owner's data.
     */
    ~Object();

    /**
     * @brief Borrows an immutable reference to the managed object.
     * @return A `Reference<const ValueType, RC>` object representing the immutable borrow.
     */
    Reference<const ValueType, RC> borrowImmutable() const noexcept;

    /**
     * @brief Borrows a mutable reference to the managed object.
     * @return A `Reference<ValueType, RC>` object representing the mutable borrow.
     */
    Reference<ValueType, RC> borrowMutable() noexcept;

    /**
     * @brief Provides mutable access to the wrapped object
     * @tparam Func Callable type
     * @param f Callable that will receive T&
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
     * @brief Sets a new value (copy version)
     * @param newValue Value to copy from
     * @warning This method is available if T is trivial type
     */
    void setValue(T newValue);

    /**
     * @brief Gets a copy of the current value
     * @warning This method is available if T is trivial type
     * @return Copy of the wrapped value
     */
    T getValue() const;

private:
    friend Reference<const T, RC>;
    friend Reference<T, RC>;

    bool incrementRefCount() const noexcept;
    bool decrementRefCount() const noexcept;

    T m_data;
    mutable std::atomic<RefCountType> m_refCount{1};
};

/**
 * @class Reference
 * @brief A class representing a reference to an object managed by an `Object`.
 *
 * The `Reference` class provides access to an object managed by an `Object` and ensures proper reference counting.
 * It tracks whether the reference is valid and prevents accessing the object after it has been invalidated.
 *
 * @tparam T The type of the managed object. Must not be volatile.
 * @tparam RC The type used for reference counting (default is `std::uint32_t`).
 */
template<typename T, typename RC = std::uint32_t>
class Reference final {
public:
    using ObjectType = Object<T, RC>; //! Type of the `Object` managing the referenced object.
    using RefCountType = RC; //! Type used for reference counting.
    using ValueType = std::remove_const_t<T>; //! Type of the managed object without const qualifier.
    using ReferenceType = ValueType&; //! Reference type to the managed object.
    using ConstReferenceType = const ValueType&; //! Const reference type to the managed object.
    using PointerType = ValueType*; //! Pointer type to the managed object.
    using ConstPointerType = const ValueType*; //! Const pointer type to the managed object.

    /**
     * @brief Deleted copy constructor for immutable references.
     * Copying a `Reference<const ValueType, RC>` is not allowed to ensure proper ownership semantics.
     */
    Reference(const Reference<const ValueType, RC>& other) = delete;
    Reference(Reference<const ValueType, RC>&& other) = delete;
    Reference& operator=(const Reference<const ValueType, RC>& other) = delete;
    Reference& operator=(Reference<const ValueType, RC>&& other) = delete;

    Reference();
    ~Reference() noexcept;
    Reference(const Reference<ValueType, RC>& other) noexcept;
    Reference(Reference<ValueType, RC>&& other) noexcept;
    Reference& operator=(const Reference<ValueType, RC>& other) noexcept;
    Reference& operator=(Reference<ValueType, RC>&& other) noexcept;

    /**
     * @brief Provides mutable access to the wrapped object
     * @tparam Func Callable type
     * @param f Callable that will receive T&
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
     * @brief Sets a new value (copy version)
     * @param newValue Value to copy from
     * @warning This method is available if T is trivial type
     */
    void setValue(T newValue);

    /**
     * @brief Gets a copy of the current value
     * @warning This method is available if T is trivial type
     * @return Copy of the wrapped value
     */
    T getValue() const;

    /**
     * @brief Invalidates this reference.
     */
    void invalidate() noexcept;

    /**
     * @brief Checks whether this reference is valid.
     *
     * A reference is valid if it is associated with an active `Object` and the managed object has not been destroyed.
     *
     * @return `true` if the reference is valid, `false` otherwise.
     */
    bool isValid() const noexcept;

private:
    friend class Object<ValueType, RC>;
    friend class Object<const ValueType, RC>;
    friend class Reference<const ValueType, RC>;

    explicit Reference(ObjectType* owner);
    void copy(const Reference<ValueType, RC>& other) noexcept;
    void swap(Reference<ValueType, RC>& other) noexcept;
    ConstPointerType get() const;
    PointerType get();

    ObjectType* m_owner{nullptr};
};

template<typename T, typename RC>
class Reference<const T, RC> final {
public:
    using ObjectType = Object<T, RC>;
    using RefCountType = RC;
    using ValueType = std::remove_const_t<T>;
    using ReferenceType = ValueType&;
    using ConstReferenceType = const ValueType&;
    using PointerType = ValueType*;
    using ConstPointerType = const ValueType*;

    Reference();
    ~Reference() noexcept;
    Reference(const Reference<ValueType, RC>& other) noexcept;
    Reference(Reference<ValueType, RC>&& other) noexcept;
    Reference& operator=(const Reference<ValueType, RC>& other) noexcept;
    Reference& operator=(Reference<ValueType, RC>&& other) noexcept;
    Reference(const Reference<const ValueType, RC>& other) noexcept;
    Reference(Reference<const ValueType, RC>&& other) noexcept;
    Reference& operator=(const Reference<const ValueType, RC>& other) noexcept;
    Reference& operator=(Reference<const ValueType, RC>&& other) noexcept;

    template<typename Func>
    void accessImmutable(Func&& f) const;

    T getValue() const;

    void invalidate() noexcept;
    bool isValid() const noexcept;

private:
    friend class Object<ValueType, RC>;
    friend class Object<const ValueType, RC>;
    friend class Reference<ValueType, RC>;

    explicit Reference(const ObjectType* owner);
    void copy(const Reference<ValueType, RC>& other) noexcept;
    void copy(const Reference<const ValueType, RC>& other) noexcept;
    template<typename C> void swap(Reference<C, RC>& other) noexcept;
    ConstPointerType get() const;

    const ObjectType* m_owner{nullptr};
};

template<typename T, typename RC>
template<typename... Arg>
Object<T, RC>::Object(Arg&&... arg):
m_data(std::forward<Arg>(arg)...) {}

template<typename T, typename RC>
Object<T, RC>::~Object() {
    PANIC(m_refCount.load() > 1)
}

template<typename T, typename RC>
template<typename Func>
void Object<T, RC>::accessMutable(Func&& f) {
    static_assert((!std::is_const_v<T> && std::is_invocable_v<Func, ValueType&>) &&
                  "T must be mutable and Func must accept T&");
    f(m_data);
}

template<typename T, typename RC>
template<typename Func>
void Object<T, RC>::accessImmutable(Func&& f) const {
    static_assert((!std::is_const_v<T> && std::is_invocable_v<Func, const ValueType&>) &&
                  "T must be mutable and Func must accept T&");
    f(m_data);
}

template<typename T, typename RC>
void Object<T, RC>::setValue(T newValue) {
    static_assert(std::is_trivial_v<T> && !std::is_const_v<T>);
    m_data = newValue;
}

template<typename T, typename RC>
T Object<T, RC>::getValue() const {
    static_assert(std::is_trivial_v<T>);
    return m_data;
}

template<typename T, typename RC>
Reference<const typename Object<T, RC>::ValueType, RC> Object<T, RC>::borrowImmutable() const noexcept {
    using ReferenceType = Reference<const ValueType, RC>;
    if (incrementRefCount()) {
        return ReferenceType{this};
    } else {
        return ReferenceType{nullptr};
    }
}

template<typename T, typename RC>
Reference<typename Object<T, RC>::ValueType, RC> Object<T, RC>::borrowMutable() noexcept {
    static_assert(!std::is_const_v<T> && "T must be mutable");
    using ReferenceType = Reference<ValueType, RC>;
    if (incrementRefCount()) {
        return ReferenceType{this};
    } else {
        return ReferenceType{nullptr};
    }
}

template<typename T, typename RC>
bool Object<T, RC>::incrementRefCount() const noexcept {
    // Important: The ABA problem is not a problem here because:
    // 1. We are only interested in the current value of the counter, not its history of changes
    // 2. Even if A->B->A occurred between load and CAS, the final value will be correct
    // 3. The main thing is the atomicity of the counter change operation
    auto current = m_refCount.load();
    while(current > 0) {
        if (m_refCount.compare_exchange_strong(current, current + 1)) {
            return true;
        }
    }
    return false;
}

template<typename T, typename RC>
bool Object<T, RC>::decrementRefCount() const noexcept {
    // Important: The ABA problem is not a problem here because:
    // 1. We are only interested in the current value of the counter, not its history of changes
    // 2. Even if A->B->A occurred between load and CAS, the final value will be correct
    // 3. The main thing is the atomicity of the counter change operation
    auto current = m_refCount.load();
    while (current > 0) {
        if (m_refCount.compare_exchange_strong(current, current - 1)) {
            return true;
        }
    }
    return false;
}

template<typename T, typename RC>
Reference<T, RC>::Reference(): m_owner(nullptr) {}

template<typename T, typename RC>
Reference<T, RC>::~Reference() noexcept { invalidate(); }

template<typename T, typename RC>
Reference<T, RC>::Reference(const Reference<ValueType, RC>& other) noexcept {
    (void)this->operator=(other);
}

template<typename T, typename RC>
Reference<T, RC>::Reference(Reference<ValueType, RC>&& other) noexcept {
    (void)this->operator=(std::move(other));
}

template<typename T, typename RC>
Reference<T, RC>& Reference<T, RC>::operator=(const Reference<ValueType, RC>& other) noexcept {
    invalidate();
    copy(other);
    return *this;
}

template<typename T, typename RC>
Reference<T, RC>& Reference<T, RC>::operator=(Reference<ValueType, RC>&& other) noexcept {
    invalidate();
    swap(other);
    return *this;
}

template<typename T, typename RC>
void Reference<T, RC>::setValue(T newValue) {
    m_owner->setValue(newValue);
}

template<typename T, typename RC>
T Reference<T, RC>::getValue() const {
    return m_owner->getValue();
}

template<typename T, typename RC>
template<typename Func>
void Reference<T, RC>::accessMutable(Func&& f) {
    ASSERTION(isValid(), std::runtime_error, "Attempt dered invalid reference")
    m_owner->accessMutable(std::forward<Func>(f));
}

template<typename T, typename RC>
template<typename Func>
void Reference<T, RC>::accessImmutable(Func&& f) const {
    ASSERTION(isValid(), std::runtime_error, "Attempt dered invalid reference")
    m_owner->accessImmutable(std::forward<Func>(f));
}

template<typename T, typename RC>
void Reference<T, RC>::invalidate() noexcept {
    if (m_owner) {
        m_owner->decrementRefCount();
        m_owner = nullptr;
    }
}

template<typename T, typename RC>
bool Reference<T, RC>::isValid() const noexcept { return m_owner != nullptr; }

template<typename T, typename RC>
Reference<T, RC>::Reference(ObjectType* owner):
m_owner(owner)
{}

template<typename T, typename RC>
void Reference<T, RC>::copy(const Reference<ValueType, RC>& other) noexcept {
    if (other.isValid() && other.m_owner->incrementRefCount()) {
        m_owner = other.m_owner;
    } else {
        m_owner = nullptr;
    }
}

template<typename T, typename RC>
void Reference<T, RC>::swap(Reference<ValueType, RC>& other) noexcept {
    std::swap(m_owner, other.m_owner);
}

template<typename T, typename RC>
typename Reference<T, RC>::ConstPointerType Reference<T, RC>::get() const {
    ASSERTION(isValid(), std::runtime_error, "Attempt to deref invalida reference")
    return &m_owner->m_data;
}

template<typename T, typename RC>
typename Reference<T, RC>::PointerType Reference<T, RC>::get() {
    ASSERTION(isValid(), std::runtime_error, "Attempt to deref invalida reference")
    return &m_owner->m_data;
}

template<typename T, typename RC>
Reference<const T, RC>::Reference(): m_owner(nullptr) {}

template<typename T, typename RC>
Reference<const T, RC>::~Reference() noexcept { invalidate(); }

template<typename T, typename RC>
Reference<const T, RC>::Reference(const Reference<ValueType, RC>& other) noexcept {
    (void)this->operator=(other);
}

template<typename T, typename RC>
Reference<const T, RC>::Reference(Reference<ValueType, RC>&& other) noexcept {
    (void)this->operator=(std::move(other));
}

template<typename T, typename RC>
Reference<const T, RC>& Reference<const T, RC>::operator=(const Reference<ValueType, RC>& other) noexcept {
    invalidate();
    copy(other);
    return *this;
}

template<typename T, typename RC>
Reference<const T, RC>& Reference<const T, RC>::operator=(Reference<ValueType, RC>&& other) noexcept {
    invalidate();
    swap(other);
    return *this;
}

template<typename T, typename RC>
Reference<const T, RC>::Reference(const Reference<const ValueType, RC>& other) noexcept {
    (void)this->operator=(other);
}

template<typename T, typename RC>
Reference<const T, RC>::Reference(Reference<const ValueType, RC>&& other) noexcept {
    (void)this->operator=(std::move(other));
}

template<typename T, typename RC>
Reference<const T, RC>& Reference<const T, RC>::operator=(const Reference<const ValueType, RC>& other) noexcept {
    invalidate();
    copy(other);
    return *this;
}

template<typename T, typename RC>
Reference<const T, RC>& Reference<const T, RC>::operator=(Reference<const ValueType, RC>&& other) noexcept {
    invalidate();
    swap(other);
    return *this;
}

template<typename T, typename RC>
template<typename Func>
void Reference<const T, RC>::accessImmutable(Func&& f) const {
    ASSERTION(isValid(), std::runtime_error, "Attempt dered invalid reference")
    m_owner->accessImmutable(std::forward<Func>(f));
}

template<typename T, typename RC>
void Reference<const T, RC>::invalidate() noexcept {
    if (m_owner) {
        m_owner->decrementRefCount();
        m_owner = nullptr;
    }
}

template<typename T, typename RC>
T Reference<const T, RC>::getValue() const {
    return m_owner->getValue();
}

template<typename T, typename RC>
bool Reference<const T, RC>::isValid() const noexcept { return m_owner != nullptr; }

template<typename T, typename RC>
Reference<const T, RC>::Reference(const ObjectType* owner) : m_owner(owner) {}

template<typename T, typename RC>
void Reference<const T, RC>::copy(const Reference<ValueType, RC>& other) noexcept {
    if (other.m_owner->incrementRefCount()) {
        m_owner = other.m_owner;
    } else {
        m_owner = nullptr;
    }
}

template<typename T, typename RC>
void Reference<const T, RC>::copy(const Reference<const ValueType, RC>& other) noexcept {
    if (other.m_owner->incrementRefCount()) {
        m_owner = other.m_owner;
    } else {
        m_owner = nullptr;
    }
}

template<typename T, typename RC>
template<typename C>
void Reference<const T, RC>::swap(Reference<C, RC>& other) noexcept {
    std::swap(m_owner, other.m_owner);
}

template<typename T, typename RC>
typename Reference<const T, RC>::ConstPointerType Reference<const T, RC>::get() const {
    ASSERTION(isValid(), std::runtime_error, "Attempt to deref invalida reference")
    return &m_owner->m_data;
}

#else

template<typename T, typename RC>
class Reference;

template<typename T, typename RC = std::uint32_t>
class Object final {
public:
    static_assert(!std::is_volatile_v<T>);
    using RefCountType = RC;
    using ValueType = std::remove_const_t<T>;
    using ReferenceType = ValueType&;
    using ConstReferenceType = const ValueType&;
    using PointerType = ValueType*;
    using ConstPointerType = const ValueType*;

    template<typename ... Arg>
    Object(Arg&& ... arg): m_data(std::forward<Arg>(arg) ... ) {}
    Object(const Object& other) = delete;
    Object(Object&& other) = delete;
    Object& operator=(const Object& other) = delete;
    Object& operator=(Object&& other) = delete;
    ~Object() = default;

    ConstReferenceType operator*() const noexcept { return m_data; }
    ReferenceType operator*() noexcept { return m_data; }

    ConstPointerType operator->() const noexcept { return &m_data; }
    PointerType operator->() noexcept { return &m_data; }

    Reference<const ValueType, RC> borrowImmutable() const noexcept {
        return Reference<const ValueType, RC>{&m_data};
    }
    Reference<ValueType, RC> borrowMutable() noexcept {
        return Reference<ValueType, RC>{&m_data};
    }

private:
    friend Reference<const T, RC>;
    friend Reference<T, RC>;

    T m_data;
};

template<typename T, typename RC = std::uint32_t>
class Reference final {
public:
    using ObjectType = Object<T, RC>;
    using RefCountType = RC;
    using ValueType = std::remove_const_t<T>;
    using ReferenceType = ValueType&;
    using ConstReferenceType = const ValueType&;
    using PointerType = ValueType*;
    using ConstPointerType = const ValueType*;

    Reference(): m_ptr(nullptr) {}
    Reference(const Reference<const ValueType, RC>& other) = delete;
    Reference(Reference<const ValueType, RC>&& other) = delete;
    Reference& operator=(const Reference<const ValueType, RC>& other) = delete;
    Reference& operator=(Reference<const ValueType, RC>&& other) = delete;

    inline ConstReferenceType operator*() const noexcept { return *m_ptr; }
    inline ReferenceType operator*() noexcept { return *m_ptr; }

    ConstPointerType operator->() const noexcept { return m_ptr; }
    PointerType operator->() noexcept { return m_ptr; }

    inline ~Reference() = default;
    inline Reference(const Reference<ValueType, RC>& other) noexcept {
        (void)this->operator=(other);
    }
    inline Reference(Reference<ValueType, RC>&& other) noexcept {
        (void)this->operator=(std::move(other));
    }
    inline Reference& operator=(const Reference<ValueType, RC>& other) noexcept {
        m_ptr = other.m_ptr;
        return *this;
    }
    inline Reference& operator=(Reference<ValueType, RC>&& other) noexcept {
        m_ptr = other.m_ptr;
        return *this;
    }

    inline void invalidate() noexcept {
        m_ptr = nullptr;
    }
    inline bool isValid() const noexcept { return m_ptr != nullptr; };

private:
    friend class Object<ValueType, RC>;
    friend class Object<const ValueType, RC>;
    friend class Reference<const ValueType, RC>;

    explicit Reference(T* ptr): m_ptr(ptr) {}

    T* m_ptr;
};

template<typename T, typename RC>
class Reference<const T, RC> final {
public:
    using ObjectType = Object<T, RC>;
    using RefCountType = RC;
    using ValueType = std::remove_const_t<T>;
    using ReferenceType = ValueType&;
    using ConstReferenceType = const ValueType&;
    using PointerType = ValueType*;
    using ConstPointerType = const ValueType*;

    inline ConstReferenceType operator*() const noexcept { return *m_ptr; }
    inline ConstPointerType operator->() const noexcept { return m_ptr; }

    inline ~Reference() = default;
    inline Reference(const Reference<ValueType, RC>& other) noexcept {
        (void)this->operator=(other);
    }
    inline Reference(Reference<ValueType, RC>&& other) noexcept {
        (void)this->operator=(std::move(other));
    }
    inline Reference& operator=(const Reference<ValueType, RC>& other) noexcept {
        m_ptr = other.m_ptr;
        return *this;
    }
    inline Reference& operator=(Reference<ValueType, RC>&& other) noexcept {
        m_ptr = other.m_ptr;
        return *this;
    }
    inline Reference(const Reference<const ValueType, RC>& other) noexcept {
        (void)this->operator=(other);
    }
    inline Reference(Reference<const ValueType, RC>&& other) noexcept {
        (void)this->operator=(std::move(other));
    }
    inline Reference& operator=(const Reference<const ValueType, RC>& other) noexcept {
        m_ptr = other.m_ptr;
        return *this;
    }
    inline Reference& operator=(Reference<const ValueType, RC>&& other) noexcept {
        m_ptr = other.m_ptr;
        return *this;
    }

    inline void invalidate() noexcept {
        m_ptr = nullptr;
    }
    inline bool isValid() const noexcept { return m_ptr != nullptr; };

private:
    friend class Object<ValueType, RC>;
    friend class Object<const ValueType, RC>;
    friend class Reference<ValueType, RC>;

    Reference(const T* ptr): m_ptr(ptr) {}

    const T* m_ptr{nullptr};
};
#endif //! NDEBUG

} //! namespace atom::utils::owner

namespace std {


template<typename T, typename RC>
inline void swap(atom::utils::owner::Reference<T, RC>& a, atom::utils::owner::Reference<T, RC>& b) noexcept {
    atom::utils::owner::Reference<T, RC> tmp{a};
    a = std::move(b);
    b = std::move(tmp);
}

} //! namespace std

#endif //! REF_COUNTER_H
