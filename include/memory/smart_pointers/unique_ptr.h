#ifndef ATOM_MEMORY_UNIQUE_PTR_H
#define ATOM_MEMORY_UNIQUE_PTR_H

#include "include/memory/allocators/default_allocator.h"
#include "include/utils/owner.h"
#include "include/utils/assertion.h"

#include <stdexcept>
#include <type_traits>

namespace atom::memory {

template<typename T, typename A>
class UniquePtr;

/**
 * @brief Performs a dynamic_cast between two UniquePtr types.
 *
 * @tparam D Destination type (must be a polymorphic type)
 * @tparam S Source type (must be a polymorphic type)
 * @tparam _A Allocator type (default: allocator::DefaultAllocator)
 *
 * @param other R-value reference to the source UniquePtr
 *
 * @return UniquePtr<D, _A>
 *   - If the cast succeeds, returns a new UniquePtr owning the converted object
 *   - If the cast fails, returns an empty UniquePtr
 *
 * @note Requires RTTI (Runtime Type Information) to be enabled
 * @warning Only works with polymorphic types (classes with virtual functions)
 *
 * Example:
 * @code
 * UniquePtr<Base> base = ...;
 * auto derived = DynamicCast<Derived>(std::move(base));
 * @endcode
 */
template<typename D, typename S, typename _A = allocator::DefaultAllocator>
UniquePtr<D, _A> DynamicCast(UniquePtr<S, _A>&& other);

/**
 * @brief Performs a static_cast between two UniquePtr types.
 *
 * @tparam D Destination type
 * @tparam S Source type
 * @tparam _A Allocator type (default: allocator::DefaultAllocator)
 *
 * @param other R-value reference to the source UniquePtr
 *
 * @return UniquePtr<D, _A>
 *   - New UniquePtr with statically converted pointer
 *   - Never returns nullptr (unlike DynamicCast)
 *
 * @warning No runtime type checking is performed.
 *          Undefined behavior if types are incompatible.
 *
 * Example:
 * @code
 * UniquePtr<Derived> derived = ...;
 * auto base = StaticCast<Base>(std::move(derived)); // Safe upcast
 * @endcode
 */
template<typename D, typename S, typename _A = allocator::DefaultAllocator>
UniquePtr<D, _A> StaticCast(UniquePtr<S, _A>&& other);

/**
 * @brief Performs a reinterpret_cast between two UniquePtr types.
 *
 * @tparam D Destination type
 * @tparam S Source type
 * @tparam _A Allocator type (default: allocator::DefaultAllocator)
 *
 * @param other R-value reference to the source UniquePtr
 *
 * @return UniquePtr<D, _A>
 *   - New UniquePtr with reinterpreted pointer value
 *
 * @warning Extremely unsafe! Only use when absolutely necessary.
 *          No type safety guarantees.
 *
 * Example:
 * @code
 * UniquePtr<int> intPtr = ...;
 * auto charPtr = ReinterpretCast<char>(std::move(intPtr));
 * @endcode
 */
template<typename D, typename S, typename _A = allocator::DefaultAllocator>
UniquePtr<D, _A> ReinterpretCast(UniquePtr<S, _A>&& other);

/**
 * @brief Performs a const_cast between two UniquePtr types.
 *
 * @tparam D Destination type (with different const-qualification)
 * @tparam S Source type
 * @tparam _A Allocator type (default: allocator::DefaultAllocator)
 *
 * @param other R-value reference to the source UniquePtr
 *
 * @return UniquePtr<D, _A>
 *   - New UniquePtr with modified const-qualification
 *
 * @warning Undefined behavior if casting away const from an originally const object
 *
 * Example:
 * @code
 * UniquePtr<const int> constPtr = ...;
 * auto mutablePtr = ConstCast<int>(std::move(constPtr));
 * @endcode
 */
template<typename D, typename S, typename _A = allocator::DefaultAllocator>
UniquePtr<D, _A> ConstCast(UniquePtr<S, _A>&& other);

/**
 * @class UniquePtr
 * @brief A smart pointer that manages the lifetime of a dynamically allocated object.
 *
 * The `UniquePtr` class ensures exclusive ownership of an object, managing its allocation and deallocation.
 * It uses a custom allocator to allocate and free memory. Copying is not allowed, but moving is supported.
 *
 * @tparam T The type of the managed object. Must not be volatile.
 * @tparam A The allocator type used for memory management (default is `allocator::DefaultAllocator`).
 */
template<typename T, typename A = allocator::DefaultAllocator>
class UniquePtr final {
public:
    static_assert(!std::is_volatile_v<T>, "T must not be volatile");

    using ValueType = T; ///< Type of the managed object.
    using PointerType = ValueType*; ///< Pointer type to the managed object.
    using ConstPointerType = ValueType const*; ///< Const pointer type to the managed object.
    using ReferenceType = ValueType&; ///< Reference type to the managed object.
    using ConstReferenceType = ValueType const&; ///< Const reference type to the managed object.
    using AllocatorType = A; ///< Type of the allocator used for memory management.
    using RefAllocatorType = utils::Reference<AllocatorType>; ///< Safe reference type to the allocator.

    /**
     * @brief Creates a new `UniquePtr` instance with the given arguments.
     *
     * Allocates and constructs the managed object using the provided allocator.
     *
     * @tparam Arg Variadic template parameter pack for constructor arguments.
     * @param allocator The allocator used to manage memory.
     * @param arg Arguments to forward to the constructor of the managed object.
     * @return A `UniquePtr` instance managing the newly created object.
     */
    template<typename ... Arg>
    static UniquePtr Make(utils::Reference<AllocatorType> allocator, Arg&& ... arg);

    /**
     * @brief Creates a new `UniquePtr` instance with the given arguments using the default allocator.
     *
     * Allocates and constructs the managed object using the default allocator.
     *
     * @tparam Arg Variadic template parameter pack for constructor arguments.
     * @param arg Arguments to forward to the constructor of the managed object.
     * @return A `UniquePtr` instance managing the newly created object.
     */
    template<typename ... Arg>
    static UniquePtr Make(Arg&& ... arg);

    /**
     * @brief Deleted copy constructor/assignment operator.
     * Copying a `UniquePtr` is not allowed to ensure exclusive ownership semantics.
     */
    UniquePtr(const UniquePtr& ) = delete;
    UniquePtr& operator=(const UniquePtr& ) = delete;
    ConstReferenceType operator*() && = delete;
    ConstPointerType operator->() && = delete;

    explicit UniquePtr(std::nullptr_t ptr = nullptr) noexcept;
    UniquePtr(UniquePtr&& other) noexcept;
    UniquePtr& operator=(UniquePtr&& other) noexcept;

    template<typename D, typename = std::enable_if_t<std::is_base_of_v<T, D>>>
    UniquePtr(UniquePtr<D, A>&& other) noexcept;

    template<typename D, typename = std::enable_if_t<std::is_base_of_v<T, D>>>
    UniquePtr& operator=(UniquePtr<D, A>&& other) noexcept;

    /**
     * @brief Destructor.
     *
     * Destroys the managed object and deallocates its memory using the associated allocator.
     */
    ~UniquePtr();

    /**
     * @brief Dereferences the managed object (mutable version).
     * @return A reference to the managed object.
     * @throws std::runtime_error If the `UniquePtr` is null.
     */
    ReferenceType operator*() &;
    ConstReferenceType operator*() const &;

    PointerType operator->() &;
    ConstPointerType operator->() const &;

    bool operator==(std::nullptr_t ptr) const noexcept;
    bool operator!=(std::nullptr_t ptr) const noexcept;

    /**
     * @brief Releases ownership of the managed object.
     *
     * Returns the pointer to the managed object and transfers ownership to the caller.
     * The `UniquePtr` is set to null after this operation.
     *
     * @return A pointer to the managed object, or `nullptr` if the `UniquePtr` was null.
     */
    [[nodiscard]] T* release() noexcept;

    /**
     * @brief Destroys the managed object and resets the `UniquePtr` to null.
     */
    void clear();

    /**
     * @brief getAllocator
     * @return A safe reference to the allocator.
     */
    utils::Reference<AllocatorType> getAllocator();

private:
    template<typename, typename>
    friend class UniquePtr;

    template<typename D, typename S, typename _A>
    friend UniquePtr<D, _A> DynamicCast(UniquePtr<S, _A>&& other);

    template<typename D, typename S, typename _A>
    friend UniquePtr<D, _A> StaticCast(UniquePtr<S, _A>&& other);

    template<typename D, typename S, typename _A>
    friend UniquePtr<D, _A> ReinterpretCast(UniquePtr<S, _A>&& other);

    template<typename D, typename S, typename _A>
    friend UniquePtr<D, _A> ConstCast(UniquePtr<S, _A>&& other);

    using AllocatorRefType = utils::Reference<AllocatorType>;
    explicit UniquePtr(PointerType ptr, AllocatorRefType allocator = AllocatorRefType{});

    void swap(UniquePtr& other) noexcept;
    template<typename D>
    void swap(UniquePtr<D, A>& other) noexcept;

    PointerType get();
    ConstPointerType get() const;

    PointerType m_ptr{nullptr};
    RefAllocatorType m_allocator;
};

template<typename T, typename A>
class UniquePtr<const T, A> final {
public:
    static_assert(!std::is_volatile_v<T>);

    using ValueType = std::remove_const_t<T>;
    using ConstPointerType = ValueType const*;
    using ConstReferenceType = ValueType const&;
    using AllocatorType = A;
    using RefAllocatorType = utils::Reference<AllocatorType>;

    template<typename ... Arg>
    static UniquePtr Make(utils::Owner<AllocatorType>& allocator, Arg&& ... arg);

    template<typename ... Arg>
    static UniquePtr Make(Arg&& ... arg);

    explicit UniquePtr(std::nullptr_t ptr = nullptr) noexcept;
    UniquePtr(UniquePtr<ValueType, A>&& other) noexcept;
    UniquePtr& operator=(UniquePtr<ValueType, A>&& other) noexcept;
    UniquePtr(UniquePtr<const ValueType, A>&& other) noexcept;
    UniquePtr& operator=(UniquePtr<const ValueType, A>&& other) noexcept;

    ConstReferenceType operator*() const &;
    ConstPointerType operator->() const &;

    bool operator==(std::nullptr_t ptr) const noexcept;
    bool operator!=(std::nullptr_t ptr) const noexcept;

    [[nodiscard]] T* release() noexcept;
    void clear();

    RefAllocatorType getAllocator();

private:
    void swap(UniquePtr<ValueType, A>& other) noexcept;
    void swap(UniquePtr<const ValueType, A>& other) noexcept;

    UniquePtr<ValueType, A> m_ptr;
};

template<typename T, typename A>
template<typename ... Arg>
UniquePtr<T, A> UniquePtr<T, A>::Make(utils::Reference<AllocatorType> allocator, Arg&& ... arg) {
    PointerType ptr = nullptr;
    allocator.accessMutable([&ptr, &arg ...](AllocatorType& _allocator) mutable {
        ptr = reinterpret_cast<PointerType>(_allocator.allocate(sizeof(ValueType)));
        _allocator.construct(ptr, std::forward<Arg>(arg) ...);
    });
    return UniquePtr{ptr, allocator};
}

template<typename T, typename A>
template<typename ... Arg>
UniquePtr<T, A> UniquePtr<T, A>::Make(Arg&& ... arg) {
    static_assert(std::is_same_v<AllocatorType, allocator::DefaultAllocator> && "AllocatorType must be DefaultAllocator in this case");
    AllocatorType allocator;
    PointerType ptr = reinterpret_cast<PointerType>(allocator.allocate(sizeof(ValueType)));
    allocator.construct(ptr, std::forward<Arg>(arg) ...);
    return UniquePtr{ptr};
}

template<typename T, typename A>
UniquePtr<T, A>::UniquePtr(PointerType ptr, AllocatorRefType allocator):
m_ptr(ptr),
m_allocator(allocator)
{}

template<typename T, typename A>
template<typename D, typename>
UniquePtr<T, A>::UniquePtr(UniquePtr<D, A>&& other) noexcept {
    (void)this->operator=(std::move(other));
}

template<typename T, typename A>
template<typename D, typename>
UniquePtr<T, A>& UniquePtr<T, A>::operator=(UniquePtr<D, A>&& other) noexcept {
    clear();
    swap(other);
    return *this;
}

template<typename T, typename A>
UniquePtr<T, A>::UniquePtr(std::nullptr_t ptr) noexcept:
m_ptr(nullptr) {}

template<typename T, typename A>
UniquePtr<T, A>::~UniquePtr() { clear(); }

template<typename T, typename A>
UniquePtr<T, A>::UniquePtr(UniquePtr<T, A>&& other) noexcept : m_ptr(nullptr) {
    this->operator=(std::move(other));
}

template<typename T, typename A>
UniquePtr<T, A>& UniquePtr<T, A>::operator=(UniquePtr<T, A>&& other) noexcept {
    clear();
    swap(other);
    return *this;
}

template<typename T, typename A>
void UniquePtr<T, A>::clear() {
    if (m_ptr) {
        if constexpr (std::is_same_v<AllocatorType, allocator::DefaultAllocator>) {
            AllocatorType allocator;
            allocator.destruct(m_ptr);
            allocator.deallocate(reinterpret_cast<std::byte*>(m_ptr));
        } else {
            ASSERTION(m_allocator.isValid(), std::runtime_error, "")
            m_allocator.accessMutable([ptr = m_ptr](AllocatorType& _allocator) {
                _allocator.destruct(ptr);
                _allocator.deallocate(reinterpret_cast<std::byte*>(ptr));
            });
            m_allocator.invalidate();
        }
        m_ptr = nullptr;
    }
}

template<typename T, typename A>
UniquePtr<T, A>::RefAllocatorType UniquePtr<T, A>::getAllocator() {
    return m_allocator;
}

template<typename T, typename A>
UniquePtr<T, A>::PointerType UniquePtr<T, A>::get() {
    if (m_ptr) {
        return m_ptr;
    } else {
        ASSERTION(false, std::runtime_error, "Attempt to deref null pointer")
    }
    return nullptr;
}

template<typename T, typename A>
UniquePtr<T, A>::ConstPointerType UniquePtr<T, A>::get() const {
    if (m_ptr) {
        return m_ptr;
    } else {
        ASSERTION(false, std::runtime_error, "Attempt to deref null pointer")
    }
    return nullptr;
}

template<typename T, typename A>
UniquePtr<T, A>::ReferenceType UniquePtr<T, A>::operator*() & {
    return *get();
}

template<typename T, typename A>
UniquePtr<T, A>::ConstReferenceType UniquePtr<T, A>::operator*() const & {
    return *get();
}

template<typename T, typename A>
UniquePtr<T, A>::PointerType UniquePtr<T, A>::operator->() & {
    return get();
}

template<typename T, typename A>
UniquePtr<T, A>::ConstPointerType UniquePtr<T, A>::operator->() const & {
    return get();
}

template<typename T, typename A>
T* UniquePtr<T, A>::release() noexcept {
    auto cp = m_ptr;
    m_ptr = nullptr;
    return cp;
}

template<typename T, typename A>
bool UniquePtr<T, A>::operator==(const std::nullptr_t ptr) const noexcept {
    return m_ptr == ptr;
}

template<typename T, typename A>
bool UniquePtr<T, A>::operator!=(const std::nullptr_t ptr) const noexcept {
    return m_ptr != ptr;
}


template<typename T, typename A>
template<typename ... Arg>
UniquePtr<const T, A> UniquePtr<const T, A>::Make(utils::Owner<AllocatorType>& allocator, Arg&& ... arg) {
    return UniquePtr<const T, A>{UniquePtr<T, A>::Make(allocator, std::forward<Arg>(arg) ...)};
}

template<typename T, typename A>
template<typename ... Arg>
UniquePtr<const T, A> UniquePtr<const T, A>::Make(Arg&& ... arg) {
    return UniquePtr<const T, A>{UniquePtr<T, A>::Make(std::forward<Arg>(arg) ...)};
}

template<typename T, typename A>
UniquePtr<const T, A>::UniquePtr(std::nullptr_t ptr) noexcept:
m_ptr(ptr) {}

template<typename T, typename A>
UniquePtr<const T, A>::UniquePtr(UniquePtr<ValueType, A>&& other) noexcept:
m_ptr() {
    this->operator=(std::move(other));
}

template<typename T, typename A>
UniquePtr<const T, A>& UniquePtr<const T, A>::operator=(UniquePtr<ValueType, A>&& other) noexcept {
    clear();
    swap(other);
    return *this;
}

template<typename T, typename A>
UniquePtr<const T, A>::UniquePtr(UniquePtr<const ValueType, A>&& other) noexcept:
m_ptr(nullptr) {
    this->operator=(std::move(other));
}

template<typename T, typename A>
UniquePtr<const T, A>& UniquePtr<const T, A>::operator=(UniquePtr<const ValueType, A>&& other) noexcept {
    clear();
    swap(other);
    return *this;
}

template<typename T, typename A>
UniquePtr<const T, A>::ConstReferenceType UniquePtr<const T, A>::operator*() const & {
    return m_ptr.operator*();
}

template<typename T, typename A>
UniquePtr<const T, A>::ConstPointerType UniquePtr<const T, A>::operator->() const & {
    return m_ptr.operator->();
}

template<typename T, typename A>
bool UniquePtr<const T, A>::operator==(std::nullptr_t ptr) const noexcept {
    return m_ptr == ptr;
}

template<typename T, typename A>
bool UniquePtr<const T, A>::operator!=(std::nullptr_t ptr) const noexcept {
    return m_ptr != ptr;
}

template<typename T, typename A>
void UniquePtr<T, A>::swap(UniquePtr<ValueType, A>& other) noexcept {
    std::swap(m_ptr, other.m_ptr);
    std::swap(m_allocator, other.m_allocator);
}

template<typename T, typename A>
template<typename D>
void UniquePtr<T, A>::swap(UniquePtr<D, A>& other) noexcept {
    {
        auto otherPtr = dynamic_cast<PointerType>(other.m_ptr);
        other.m_ptr = dynamic_cast<decltype(other.m_ptr)>(m_ptr);
        m_ptr = otherPtr;
    }
    std::swap(m_allocator, other.m_allocator);
}

template<typename T, typename A>
void UniquePtr<const T, A>::swap(UniquePtr<ValueType, A>& other) noexcept {
    std::swap(m_ptr.m_ptr, other.m_ptr);
}

template<typename T, typename A>
void UniquePtr<const T, A>::swap(UniquePtr<const ValueType, A>& other) noexcept {
    std::swap(m_ptr.m_ptr, other.m_ptr);
}

template<typename T, typename A>
[[nodiscard]] T* UniquePtr<const T, A>::release() noexcept {
    return m_ptr.release();
}

template<typename T, typename A>
void UniquePtr<const T, A>::clear() {
    m_ptr.clear();
}

template<typename T, typename A>
UniquePtr<const T, A>::RefAllocatorType UniquePtr<const T, A>::getAllocator() {
    return m_ptr.getAllocator();
}

template<typename T, typename U, typename A>
UniquePtr<T, A> DynamicCast(UniquePtr<U, A>&& other) {
    if constexpr (std::is_base_of_v<T, U>) {
        return UniquePtr<T, A>{static_cast<T*>(other.release())};
    } else if constexpr (std::is_base_of_v<U, T>) {
        return UniquePtr<T, A>{dynamic_cast<T*>(other.release())};
    } else {
        return UniquePtr<T, A>{nullptr};
    }
}

template<typename T, typename U, typename A>
UniquePtr<T, A> StaticCast(UniquePtr<U, A>&& other) {
    return UniquePtr<T, A>{static_cast<T*>(other.release())};
}

template<typename T, typename U, typename A>
UniquePtr<T, A> ReinterpretCast(UniquePtr<U, A>&& other) {
    return UniquePtr<T, A>{reinterpret_cast<T*>(other.release())};
}

template<typename T, typename U, typename A>
UniquePtr<T, A> ConstCast(UniquePtr<U, A>&& other) {
    return UniquePtr<T, A>{const_cast<T*>(other.release())};
}

} //! namespace atom::memory

#endif //! ATOM_MEMORY_UNIQUE_PTR_H
