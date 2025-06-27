#ifndef REF_COUNTER_H
#define REF_COUNTER_H

#include "include/utils/assertion.h"

#include <type_traits>
#include <utility>
#include <atomic>

namespace atom::utils {

#ifndef NDEBUG

template<typename T, typename RC> class Reference;
template<typename T, typename RC> class Reference<const T, RC>;

/**
 * @class Owner
 *
 * @brief The `Owner` class manages the lifetime of an object and provides mechanisms for borrowing immutable or mutable references.
 * It uses link counting to track active borrowings to spot potential dangling link cases.
 *
 * @tparam T The type of the managed object. Must not be volatile.
 * @tparam RC The type used for reference counting (default is `std::uint32_t`).
 *
 * @warning This class does not provide thread-safe access/sharing of data,
 * this class provides a thread-safe mechanism for detecting potentially dangling references.
 */
template<typename T, typename RC = std::uint32_t>
class Owner final {
public:
    static_assert(!std::is_volatile_v<T>);
    using RefCountType = RC; //! Type used for reference counting.
    using ValueType = std::remove_const_t<T>; //! Type of the managed object without const qualifier.
    using ReferenceType = ValueType&; //! Reference type to the managed object.
    using ConstReferenceType = const ValueType&; //! Const reference type to the managed object.
    using PointerType = ValueType*; //! Pointer type to the managed object.
    using ConstPointerType = const ValueType*; //! Const pointer type to the managed object.

    /**
     * @brief Constructs the `Owner` with the provided arguments to initialize the managed object.
     *
     * This constructor forwards the given arguments to the constructor of the managed object.
     *
     * @tparam Arg Variadic template parameter pack for constructor arguments.
     * @param arg Arguments to forward to the constructor of the managed object.
     */
    template<typename... Arg>
    Owner(Arg&&... arg);

    /**
     * @brief Deleted copy constructor.
     *
     * Copying an `Owner` is not allowed to ensure single ownership semantics.
     * @details Why can't this class be moved or copied? There is no reason to leave the ability to copy/move the owner of the object
     * due to the fact that all references to the owner of the object are immediately invalidated.
     */
    Owner(const Owner& other) = delete;
    Owner(Owner&& other) = delete;
    Owner& operator=(const Owner& other) = delete;
    Owner& operator=(Owner&& other) = delete;

    /**
     * @brief Destructor.
     *
     * Destroys the `Owner` and the managed object if there are no active borrows.
     * @warning The destructor may panic if there are still references to the owner's data.
     */
    ~Owner();

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
 * @brief A class representing a reference to an object managed by an `Owner`.
 *
 * The `Reference` class provides access to an object managed by an `Owner` and ensures proper reference counting.
 * It tracks whether the reference is valid and prevents accessing the object after it has been invalidated.
 *
 * @tparam T The type of the managed object. Must not be volatile.
 * @tparam RC The type used for reference counting (default is `std::uint32_t`).
 */
template<typename T, typename RC = std::uint32_t>
class Reference final {
public:
    using OwnerType = Owner<T, RC>; //! Type of the `Owner` managing the referenced object.
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
     * A reference is valid if it is associated with an active `Owner` and the managed object has not been destroyed.
     *
     * @return `true` if the reference is valid, `false` otherwise.
     */
    bool isValid() const noexcept;

private:
    friend class Owner<ValueType, RC>;
    friend class Owner<const ValueType, RC>;
    friend class Reference<const ValueType, RC>;

    explicit Reference(OwnerType* owner);
    void copy(const Reference<ValueType, RC>& other) noexcept;
    void swap(Reference<ValueType, RC>& other) noexcept;
    ConstPointerType get() const;
    PointerType get();

    OwnerType* m_owner{nullptr};
};

template<typename T, typename RC>
class Reference<const T, RC> final {
public:
    using OwnerType = Owner<T, RC>;
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
    friend class Owner<ValueType, RC>;
    friend class Owner<const ValueType, RC>;
    friend class Reference<ValueType, RC>;

    explicit Reference(const OwnerType* owner);
    void copy(const Reference<ValueType, RC>& other) noexcept;
    void copy(const Reference<const ValueType, RC>& other) noexcept;
    template<typename C> void swap(Reference<C, RC>& other) noexcept;
    ConstPointerType get() const;

    const OwnerType* m_owner{nullptr};
};

template<typename T, typename RC>
template<typename... Arg>
Owner<T, RC>::Owner(Arg&&... arg):
m_data(std::forward<Arg>(arg)...) {}

template<typename T, typename RC>
Owner<T, RC>::~Owner() {
    PANIC(m_refCount.load() > 1)
}

template<typename T, typename RC>
template<typename Func>
void Owner<T, RC>::accessMutable(Func&& f) {
    static_assert((!std::is_const_v<T> && std::is_invocable_v<Func, ValueType&>) &&
                  "T must be mutable and Func must accept T&");
    f(m_data);
}

template<typename T, typename RC>
template<typename Func>
void Owner<T, RC>::accessImmutable(Func&& f) const {
    static_assert((!std::is_const_v<T> && std::is_invocable_v<Func, const ValueType&>) &&
                  "T must be mutable and Func must accept T&");
    f(m_data);
}

template<typename T, typename RC>
void Owner<T, RC>::setValue(T newValue) {
    static_assert(std::is_trivial_v<T> && !std::is_const_v<T>);
    m_data = newValue;
}

template<typename T, typename RC>
T Owner<T, RC>::getValue() const {
    static_assert(std::is_trivial_v<T>);
    return m_data;
}

template<typename T, typename RC>
Reference<const typename Owner<T, RC>::ValueType, RC> Owner<T, RC>::borrowImmutable() const noexcept {
    using ReferenceType = Reference<const ValueType, RC>;
    if (incrementRefCount()) {
        return ReferenceType{this};
    } else {
        return ReferenceType{nullptr};
    }
}

template<typename T, typename RC>
Reference<typename Owner<T, RC>::ValueType, RC> Owner<T, RC>::borrowMutable() noexcept {
    static_assert(!std::is_const_v<T> && "T must be mutable");
    using ReferenceType = Reference<ValueType, RC>;
    if (incrementRefCount()) {
        return ReferenceType{this};
    } else {
        return ReferenceType{nullptr};
    }
}

template<typename T, typename RC>
bool Owner<T, RC>::incrementRefCount() const noexcept {
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
bool Owner<T, RC>::decrementRefCount() const noexcept {
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
Reference<T, RC>::Reference(OwnerType* owner):
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
Reference<const T, RC>::Reference(const OwnerType* owner) : m_owner(owner) {}

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

template<typename T, typename RC> class Reference;
template<typename T, typename RC> class Reference<const T, RC>;

/**
 * @class Owner
 *
 * @brief The `Owner` class manages the lifetime of an object and provides mechanisms for borrowing immutable or mutable references.
 * It uses link counting to track active borrowings to spot potential dangling link cases.
 *
 * @tparam T The type of the managed object. Must not be volatile.
 * @tparam RC The type used for reference counting (default is `std::uint32_t`).
 *
 * @warning This class does not provide thread-safe access/sharing of data,
 * this class provides a thread-safe mechanism for detecting potentially dangling references.
 */
template<typename T, typename RC = std::uint32_t>
class Owner final {
public:
    static_assert(!std::is_volatile_v<T>);
    using RefCountType = RC; //! Type used for reference counting.
    using ValueType = std::remove_const_t<T>; //! Type of the managed object without const qualifier.
    using ReferenceType = ValueType&; //! Reference type to the managed object.
    using ConstReferenceType = const ValueType&; //! Const reference type to the managed object.
    using PointerType = ValueType*; //! Pointer type to the managed object.
    using ConstPointerType = const ValueType*; //! Const pointer type to the managed object.

    /**
     * @brief Constructs the `Owner` with the provided arguments to initialize the managed object.
     *
     * This constructor forwards the given arguments to the constructor of the managed object.
     *
     * @tparam Arg Variadic template parameter pack for constructor arguments.
     * @param arg Arguments to forward to the constructor of the managed object.
     */
    template<typename... Arg>
    Owner(Arg&&... arg): m_data(std::forward<Arg>(arg) ... ) {}

    /**
     * @brief Deleted copy constructor.
     *
     * Copying an `Owner` is not allowed to ensure single ownership semantics.
     * @details Why can't this class be moved or copied? There is no reason to leave the ability to copy/move the owner of the object
     * due to the fact that all references to the owner of the object are immediately invalidated.
     */
    Owner(const Owner& other) = delete;
    Owner(Owner&& other) = delete;
    Owner& operator=(const Owner& other) = delete;
    Owner& operator=(Owner&& other) = delete;

    /**
     * @brief Destructor.
     *
     * Destroys the `Owner` and the managed object if there are no active borrows.
     * @warning The destructor may panic if there are still references to the owner's data.
     */
    ~Owner() = default;

    /**
     * @brief Borrows an immutable reference to the managed object.
     * @return A `Reference<const ValueType, RC>` object representing the immutable borrow.
     */
    Reference<const ValueType, RC> borrowImmutable() const noexcept {
        return Reference<const ValueType, RC>{&m_data};
    }

    /**
     * @brief Borrows a mutable reference to the managed object.
     * @return A `Reference<ValueType, RC>` object representing the mutable borrow.
     */
    Reference<ValueType, RC> borrowMutable() noexcept {
        return Reference<ValueType, RC>{&m_data};
    }

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
    void accessMutable(Func&& f) {
        f(m_data);
    }

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
    void accessImmutable(Func&& f) const {
        f(m_data);
    }

    /**
     * @brief Sets a new value (copy version)
     * @param newValue Value to copy from
     * @warning This method is available if T is trivial type
     */
    void setValue(T newValue) {
        m_data = newValue;
    }

    /**
     * @brief Gets a copy of the current value
     * @warning This method is available if T is trivial type
     * @return Copy of the wrapped value
     */
    T getValue() const { return m_data; }

private:
    T m_data;
};

/**
 * @class Reference
 * @brief A class representing a reference to an object managed by an `Owner`.
 *
 * The `Reference` class provides access to an object managed by an `Owner` and ensures proper reference counting.
 * It tracks whether the reference is valid and prevents accessing the object after it has been invalidated.
 *
 * @tparam T The type of the managed object. Must not be volatile.
 * @tparam RC The type used for reference counting (default is `std::uint32_t`).
 */
template<typename T, typename RC = std::uint32_t>
class Reference final {
public:
    using OwnerType = Owner<T, RC>; //! Type of the `Owner` managing the referenced object.
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

    Reference() = default;
    ~Reference() noexcept = default;
    Reference(const Reference<ValueType, RC>& other) noexcept: m_data(other.m_data) {}
    Reference(Reference<ValueType, RC>&& other) noexcept: m_data(other.m_data) {}
    Reference& operator=(const Reference<ValueType, RC>& other) noexcept{
        m_data = other.m_data;
        return *this;
    }
    Reference& operator=(Reference<ValueType, RC>&& other) noexcept{
        m_data = other.m_data;
        return *this;
    }
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
    void accessMutable(Func&& f) {
        f(*m_data);
    }

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
    void accessImmutable(Func&& f) const {
        f(*m_data);
    }

    /**
     * @brief Sets a new value (copy version)
     * @param newValue Value to copy from
     * @warning This method is available if T is trivial type
     */
    void setValue(T newValue) {
        *m_data = newValue;
    }

    /**
     * @brief Gets a copy of the current value
     * @warning This method is available if T is trivial type
     * @return Copy of the wrapped value
     */
    T getValue() const { return *m_data; }

    /**
     * @brief Invalidates this reference.
     */
    constexpr void invalidate() noexcept {}

    /**
     * @brief Checks whether this reference is valid.
     *
     * A reference is valid if it is associated with an active `Owner` and the managed object has not been destroyed.
     *
     * @return `true` if the reference is valid, `false` otherwise.
     */
    constexpr bool isValid() const noexcept { return true; }

private:
    friend class Owner<ValueType, RC>;
    friend class Owner<const ValueType, RC>;
    friend class Reference<const ValueType, RC>;

    explicit Reference(T* data): m_data(data) {}

    T* m_data{nullptr};
};

template<typename T, typename RC>
class Reference<const T, RC> final {
public:
    using OwnerType = Owner<T, RC>;
    using RefCountType = RC;
    using ValueType = std::remove_const_t<T>;
    using ReferenceType = ValueType&;
    using ConstReferenceType = const ValueType&;
    using PointerType = ValueType*;
    using ConstPointerType = const ValueType*;

    Reference() = default;
    ~Reference() noexcept = default;
    Reference(const Reference<ValueType, RC>& other) noexcept: m_data(other.m_data) {}
    Reference(Reference<ValueType, RC>&& other) noexcept: m_data(other.m_data) {}
    Reference(const Reference<const ValueType, RC>& other) noexcept: m_data(other.m_data) {}
    Reference(Reference<const ValueType, RC>&& other) noexcept: m_data(other.m_data) {}

    Reference& operator=(const Reference<ValueType, RC>& other) noexcept {
        m_data = other.m_data;
        return *this;
    }
    Reference& operator=(Reference<ValueType, RC>&& other) noexcept{
        m_data = other.m_data;
        return *this;
    }
    Reference& operator=(const Reference<const ValueType, RC>& other) noexcept{
        m_data = other.m_data;
        return *this;
    }
    Reference& operator=(Reference<const ValueType, RC>&& other) noexcept{
        m_data = other.m_data;
        return *this;
    }

    template<typename Func>
    void accessImmutable(Func&& f) const {
        f(*m_data);
    }

    T getValue() const {
        return *m_data;
    }

    void invalidate() noexcept;
    bool isValid() const noexcept;

private:
    friend class Owner<ValueType, RC>;
    friend class Owner<const ValueType, RC>;
    friend class Reference<ValueType, RC>;

    explicit Reference(const T* data): m_data(const_cast<T*>(data)) {}

    T* m_data{nullptr};
};

#endif //! NDEBUG

} //! namespace atom::utils

#endif //! REF_COUNTER_H
