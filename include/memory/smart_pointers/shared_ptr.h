#ifndef ATOM_MEMORY_SHARED_PTR_H
#define ATOM_MEMORY_SHARED_PTR_H

#include "include/memory/allocators/default_allocator.h"
#include "include/utils/owner.h"
#include "include/utils/tsan_object.h"
#include "include/utils/assertion.h"

#include <stdexcept>
#include <atomic>
#include <cstdint>

namespace atom::memory {

template<typename T, typename C>
class SharedPtr;

template<typename T, typename C>
class WeakPtr;

/**
 * @brief Performs dynamic_cast conversion between SharedPtr types
 *
 * @tparam D Destination type (must be polymorphic and related to S)
 * @tparam S Source type
 * @tparam _C Control block type
 * @param srcSharedPtr Source SharedPtr to convert
 * @return SharedPtr<D, _C> containing converted pointer if successful,
 *         empty SharedPtr otherwise
 * @note Uses dynamic_cast internally, so D and S must be polymorphic types
 */
template<typename D, typename S, typename _C>
SharedPtr<D, _C> DynamicCast(SharedPtr<S, _C>&& srcSharedPtr);

/**
 * @brief Performs dynamic_cast conversion between SharedPtr types (const version)
 *
 * @tparam D Destination type (must be polymorphic and related to S)
 * @tparam S Source type
 * @tparam _C Control block type
 * @param srcSharedPtr Source SharedPtr to convert
 * @return SharedPtr<D, _C> containing converted pointer if successful,
 *         empty SharedPtr otherwise
 * @note Creates a new reference count for the converted pointer
 */
template<typename D, typename S, typename _C>
SharedPtr<D, _C> DynamicCast(const SharedPtr<S, _C>& srcSharedPtr);

/**
 * @brief Performs static_cast conversion between SharedPtr types
 *
 * @tparam D Destination type (must be convertible from S via static_cast)
 * @tparam S Source type
 * @tparam _C Control block type
 * @param srcSharedPtr Source SharedPtr to convert
 * @return SharedPtr<D, _C> containing converted pointer
 * @note Uses static_cast internally, no runtime type checking performed
 */
template<typename D, typename S, typename _C>
SharedPtr<D, _C> StaticCast(SharedPtr<S, _C>&& srcSharedPtr);

/**
 * @brief Performs static_cast conversion between SharedPtr types (const version)
 *
 * @tparam D Destination type (must be convertible from S via static_cast)
 * @tparam S Source type
 * @tparam _C Control block type
 * @param srcSharedPtr Source SharedPtr to convert
 * @return SharedPtr<D, _C> containing converted pointer
 * @note Creates a new reference count for the converted pointer
 */
template<typename D, typename S, typename _C>
SharedPtr<D, _C> StaticCast(const SharedPtr<S, _C>& srcSharedPtr);

/**
 * @brief Performs reinterpret_cast conversion between SharedPtr types
 *
 * @tparam D Destination type
 * @tparam S Source type
 * @tparam _C Control block type
 * @param srcSharedPtr Source SharedPtr to convert
 * @return SharedPtr<D, _C> containing converted pointer
 * @warning Uses reinterpret_cast internally - potentially unsafe operation
 * @note The behavior is implementation-defined and may be dangerous
 */
template<typename D, typename S, typename _C>
SharedPtr<D, _C> ReinterpretCast(SharedPtr<S, _C>&& srcSharedPtr);

/**
 * @brief Performs reinterpret_cast conversion between SharedPtr types (const version)
 *
 * @tparam D Destination type
 * @tparam S Source type
 * @tparam _C Control block type
 * @param srcSharedPtr Source SharedPtr to convert
 * @return SharedPtr<D, _C> containing converted pointer
 * @warning Uses reinterpret_cast internally - potentially unsafe operation
 * @note Creates a new reference count for the converted pointer
 */
template<typename D, typename S, typename _C>
SharedPtr<D, _C> ReinterpretCast(const SharedPtr<S, _C>& srcSharedPtr);

/**
 * @brief Performs const_cast conversion between SharedPtr types
 *
 * @tparam D Destination type (must differ only in const-ness from S)
 * @tparam S Source type
 * @tparam _C Control block type
 * @param srcSharedPtr Source SharedPtr to convert
 * @return SharedPtr<D, _C> with const-ness modified
 * @note Only modifies const qualification of the managed object
 */
template<typename D, typename S, typename _C>
SharedPtr<D, _C> ConstCast(SharedPtr<S, _C>&& srcSharedPtr);

/**
 * @brief Performs const_cast conversion between SharedPtr types (const version)
 *
 * @tparam D Destination type (must differ only in const-ness from S)
 * @tparam S Source type
 * @tparam _C Control block type
 * @param srcSharedPtr Source SharedPtr to convert
 * @return SharedPtr<D, _C> with const-ness modified
 * @note Creates a new reference count for the converted pointer
 */
template<typename D, typename S, typename _C>
SharedPtr<D, _C> ConstCast(const SharedPtr<S, _C>& srcSharedPtr);

/**
 * @brief A shared pointer implementation with customizable control block.
 *
 * @tparam T The type of the managed object. Must not be volatile.
 * @tparam C The control block type that manages reference counting and allocation.
 *
 * @warning This is important in case you want to use a Shared/WeakPtr with your custom control block
 * The control block must satisfy the following requirements:
 *
 * ### 1. Type Definitions
 * The control block must provide these public type aliases:
 * @code
 * using RefCountType = // integer type for reference counting
 * using AllocatorType = // allocator type used for memory management
 * @endcode
 *
 * ### 2. Static Methods
 * Must implement static methods for allocating control blocks with in-place construction:
 * @code
 * template<typename T, typename... Args>
 * static ControlBlock* Allocate(Args&&... args);
 *
 * template<typename T, typename... Args>
 * static ControlBlock* Allocate(utils::Reference<AllocatorType> alloc, Args&&... args);
 * @endcode
 * - Should handle both default and custom allocators.
 * - Must construct both control block and user object in a single allocation.
 * - Must properly clean up if construction fails.
 *
 * ### 3. Methods
 * Must provide these thread-compatible operations:
 * @code
 * // Strong operations
 * bool tryAcquireStrongly();
 * void releaseStrongly();
 * RefCountType strongRefCount() const noexcept;
 *
 * // Weak operations
 * bool tryAcquireWeakly();
 * void releaseWeakly();
 * RefCountType weakRefCount() const noexcept;
 * @endcode
 *
 * Must provide access to user data:
 * @code
 * std::byte* getUserData();  // For mutable access
 * const std::byte* getUserData() const;  // For const access
 *
 * ### Example Minimal Interface
 * @code
 * class MinimalControlBlock {
 * public:
 *     using RefCountType = uint32_t;
 *     using AllocatorType = // allocator type
 *
 *     // Allocation
 *     template<typename T, typename... Args>
 *     static MinimalControlBlock* Allocate(Args&&... args);
 *
 *     template<typename T, typename... Args>
 *     static MinimalControlBlock* Allocate(utils::Reference<AllocatorType> alloc, Args&&... args);
 *
 *     // Reference counting
 *     bool tryAcquireStrongly() noexcept;
 *     bool tryAcquireWeakly() noexcept;
 *
 *     templte<typename T>
 *     void releaseStrongly();
 *     template<typename T>
 *     void releaseWeakly();
 *
 *     RefCountType strongRefCount() const noexcept;
 *     RefCountType weakRefCount() const noexcept;
 *
 *     // Memory access
 *     std::byte* getUserData();
 *     const std::byte* getUserData() const;
 *
 * private:
 *     // ...
 * };
 * @endcode
 *
 * This interface provides the necessary functionality while allowing different implementations
 * (atomic/non-atomic, different allocation strategies, etc.) to be used as template parameters
 * for smart pointers like `SharedPtr` and `WeakPtr`.
 */
template<typename T, typename C>
class SharedPtr final {
public:
    static_assert(!std::is_volatile_v<T>);
    /// @brief Value type without const qualification
    using ValueType = std::remove_const_t<T>;

    /// @brief Reference type to the managed object
    using ReferenceType = ValueType&;

    /// @brief Const reference type to the managed object
    using ConstReferenceType = const ValueType&;

    /// @brief Pointer type to the managed object
    using PointerType = ValueType*;

    /// @brief Const pointer type to the managed object
    using ConstPointerType = const ValueType*;

    /// @brief Type of the control block
    using ControlBlockType = C;

    /// @brief Type used for reference counting
    using RefCountType = typename ControlBlockType::RefCountType;

    /// @brief Allocator type used by the control block
    using AllocatorType = typename ControlBlockType::AllocatorType;

    /**
     * @brief Creates a new SharedPtr with a managed object constructed in-place.
     * @details Use default allocator.
     *
     * @tparam Arg Argument types for object construction
     * @param arg Arguments for object construction
     * @return SharedPtr owning the newly created object
     */
    template<typename... Arg>
    static SharedPtr<T, C> Make(Arg&&... arg);

    /**
     * @brief Creates a new SharedPtr with custom allocator and managed object constructed in-place.
     *
     * @tparam Arg Argument types for object construction
     * @param allocator Custom allocator to use
     * @param arg Arguments for object construction
     * @return SharedPtr owning the newly created object
     */
    template<typename... Arg>
    static SharedPtr<T, C> Make(utils::owner::Reference<AllocatorType> allocator, Arg&&... arg);

    /**
     * @brief Constructs an empty SharedPtr
     *
     * @param ptr nullptr to indicate empty state
     */
    explicit SharedPtr(std::nullptr_t ptr = nullptr) noexcept;

    /**
     * @brief Copy constructors
     *
     * @param other SharedPtr to copy from
     */
    SharedPtr(const SharedPtr& other);
    SharedPtr(SharedPtr&& other) noexcept;

    /**
     * @brief Constructs from SharedPtr of a compatible type
     *
     * @tparam D Compatible type (must be base or derived of T)
     * @param other SharedPtr to convert from
     */
    template<typename D, typename = std::enable_if_t<std::is_base_of_v<D, T> || std::is_base_of_v<T, D>>>
    SharedPtr(const SharedPtr<D, C>& other);
    template<typename D, typename = std::enable_if_t<std::is_base_of_v<D, T> || std::is_base_of_v<T, D>>>
    SharedPtr(SharedPtr<D, C>&& other) noexcept;

    /**
     * @brief Constructs from WeakPtr if object still exists
     *
     * @param weakPtr WeakPtr to attempt conversion from
     * @throws std::bad_weak_ptr if the object no longer exists
     */
    SharedPtr(const WeakPtr<T, C>& weakPtr);

    /**
     * @brief Move assignment operators
     *
     * @param other SharedPtr to move from
     * @return Reference to this SharedPtr
     */
    SharedPtr& operator=(SharedPtr&& other) noexcept;
    SharedPtr& operator=(const SharedPtr& other);
    SharedPtr& operator=(std::nullptr_t ptr);

    /**
     * @brief Assigns from SharedPtr of compatible type
     *
     * @tparam D Compatible type (must be base or derived of T)
     * @param other SharedPtr to copy from
     * @return Reference to this SharedPtr
     */
    template<typename D, typename = std::enable_if_t<std::is_base_of_v<D, T> || std::is_base_of_v<T, D>>>
    SharedPtr& operator=(const SharedPtr<D, C>& other) noexcept;
    template<typename D, typename = std::enable_if_t<std::is_base_of_v<D, T> || std::is_base_of_v<T, D>>>
    SharedPtr& operator=(SharedPtr<D, C>&& other) noexcept;

    /**
     * @brief Assignment from WeakPtr
     *
     * @param weakPtr WeakPtr to attempt conversion from
     * @return Reference to this SharedPtr
     * @throws std::bad_weak_ptr if the object no longer exists
     */
    SharedPtr& operator=(const WeakPtr<T, C>& weakPtr);

    /**
     * @brief Destructor - decrements reference count and cleans up if last reference
     */
    ~SharedPtr();

    /**
     * @brief Checks if this SharedPtr is empty
     *
     * @param ptr nullptr to compare against
     * @return true if empty, false otherwise
     */
    constexpr bool operator==(std::nullptr_t ptr) const noexcept;

    /**
     * @brief Compares two SharedPtrs for equality
     *
     * @param other SharedPtr to compare with
     * @return true if both manage the same object, false otherwise
     */
    constexpr bool operator==(const SharedPtr& other) const noexcept;

    /**
     * @brief Releases ownership of the managed object
     */
    void clear();

    /**
     * @brief Dereference operator (const version)
     *
     * @return Const reference to the managed object
     * @throws std::runtime_error if pointer is null
     */
    ConstReferenceType operator*() const;
    ReferenceType operator*();

    /**
     * @brief Member access operator
     *
     * @return Pointer to the managed object
     * @throws std::runtime_error if pointer is null
     */
    ConstPointerType operator->() const;
    PointerType operator->();

    /**
     * @brief Gets the current strong reference count
     *
     * @return Number of strong references
     */
    RefCountType strongRefCount() const noexcept;

    /**
     * @brief Gets the current weak reference count
     *
     * @return Number of weak references
     */
    RefCountType weakRefCount() const noexcept;

private:
    friend WeakPtr<T, C>;

    template<typename D, typename S, typename _C>
    friend SharedPtr<D, _C> DynamicCast(SharedPtr<S, _C>&& srcSharedPtr);

    template<typename D, typename S, typename _C>
    friend SharedPtr<D, _C> StaticCast(SharedPtr<S, _C>&& srcSharedPtr);

    template<typename D, typename S, typename _C>
    friend SharedPtr<D, _C> ReinterpretCast(SharedPtr<S, _C>&& srcSharedPtr);

    template<typename D, typename S, typename _C>
    friend SharedPtr<D, _C> ConstCast(SharedPtr<S, _C>&& srcSharedPtr);

    constexpr explicit SharedPtr(ControlBlockType* controlBlock, PointerType ptr = nullptr) noexcept;

    void copy(const SharedPtr& other);
    void swap(SharedPtr& other) noexcept;

    ConstPointerType get() const;
    PointerType get();

    PointerType m_ptr{nullptr};
    ControlBlockType* m_controlBlock{nullptr};
};

/**
 * @brief A non-owning weak reference to an object managed by SharedPtr.
 *
 * @tparam T The type of the managed object.
 * @tparam C The control block type used by the associated SharedPtr.
 *
 * WeakPtr allows safe observation of an object owned by SharedPtr without
 * extending its lifetime. Can be converted to SharedPtr if the object still exists.
 */
template<typename T, typename C>
class WeakPtr final {
public:
    /// @brief Type of the managed object
    using ValueType = T;

    /// @brief Type of the control block
    using ControlBlockType = C;

    /// @brief Type used for reference counting
    using RefCountType = typename ControlBlockType::RefCountType;

    /// @brief Allocator type used by the control block
    using AllocatorType = typename ControlBlockType::AllocatorType;

    /**
     * @brief Constructs a WeakPtr that shares ownership with another WeakPtr
     *
     * @param other WeakPtr to copy from
     */
    WeakPtr(const WeakPtr& other);

    /**
     * @brief Constructs a WeakPtr by transferring ownership from another WeakPtr
     *
     * @param other WeakPtr to move from
     */
    WeakPtr(WeakPtr&& other) noexcept;

    /**
     * @brief Constructs a WeakPtr from a SharedPtr
     *
     * @param sharedPtr SharedPtr to create weak reference from
     */
    WeakPtr(const SharedPtr<T, C>& sharedPtr);

    /**
     * @brief Copy assignment operator
     *
     * @param other WeakPtr to copy from
     * @return Reference to this WeakPtr
     */
    WeakPtr& operator=(const WeakPtr& other);

    /**
     * @brief Move assignment operator
     *
     * @param other WeakPtr to move from
     * @return Reference to this WeakPtr
     */
    WeakPtr& operator=(WeakPtr&& other) noexcept;

    /**
     * @brief Assignment from SharedPtr
     *
     * @param sharedPtr SharedPtr to create weak reference from
     * @return Reference to this WeakPtr
     */
    WeakPtr& operator=(const SharedPtr<T, C>& sharedPtr);

    /**
     * @brief Destructor - releases the weak reference
     */
    ~WeakPtr();

    /**
     * @brief Releases the weak reference
     */
    void clear();

    /**
     * @brief Attempts to create a SharedPtr from this WeakPtr
     *
     * @return SharedPtr managing the object if it still exists,
     *         empty SharedPtr otherwise
     * @note This operation is thread-safe
     */
    SharedPtr<T, C> tryLock() const noexcept;

    /**
     * @brief Checks if the managed object has been deleted
     *
     * @return true if no SharedPtrs own the object, false otherwise
     * @note Equivalent to strongRefCount() == 0
     */
    bool isExpired() const noexcept;

    /**
     * @brief Gets the current strong reference count
     *
     * @return Number of SharedPtr instances owning the object
     */
    RefCountType strongRefCount() const noexcept;

    /**
     * @brief Gets the current weak reference count
     *
     * @return Number of WeakPtr instances observing the object
     */
    RefCountType weakRefCount() const noexcept;

private:
    void copy(const WeakPtr& other);
    void copy(const SharedPtr<T, C>& sharedPtr);
    void swap(WeakPtr& other) noexcept;

    ControlBlockType* m_controlBlock{nullptr};
};

// Implementation of SharedPtr methods
template<typename T, typename C>
template<typename... Arg>
SharedPtr<T, C> SharedPtr<T, C>::Make(Arg&&... arg) {
    return SharedPtr{C::template Allocate<T, Arg...>(std::forward<Arg>(arg)...)};
}

template<typename T, typename C>
template<typename... Arg>
SharedPtr<T, C> SharedPtr<T, C>::Make(utils::owner::Reference<AllocatorType> allocator, Arg&&... arg) {
    return SharedPtr{C::template Allocate<T, Arg...>(allocator, std::forward<Arg>(arg)...)};
}

template<typename T, typename C>
SharedPtr<T, C>::SharedPtr(std::nullptr_t /*ptr*/) noexcept:
m_controlBlock(nullptr)
{}

template<typename T, typename C>
SharedPtr<T, C>::SharedPtr(const SharedPtr& other) {
    (void)this->operator=(other);
}

template<typename T, typename C>
SharedPtr<T, C>::SharedPtr(SharedPtr&& other) noexcept {
    (void)this->operator=(std::move(other));
}

template<typename T, typename C>
SharedPtr<T, C>::SharedPtr(const WeakPtr<T, C>& weakPtr) {
    (void)this->operator=(weakPtr);
}

template<typename T, typename C>
template<typename D, typename>
SharedPtr<T, C>::SharedPtr(const SharedPtr<D, C>& other) {
    (void)this->operator=(other);
}

template<typename T, typename C>
template<typename D, typename>
SharedPtr<T, C>::SharedPtr(SharedPtr<D, C>&& other) noexcept {
    (void)this->operator=(std::move(other));
}

template<typename T, typename C>
SharedPtr<T, C>::~SharedPtr() {
    clear();
}

template<typename T, typename C>
SharedPtr<T, C>& SharedPtr<T, C>::operator=(SharedPtr&& other) noexcept {
    clear();
    swap(other);
    return *this;
}

template<typename T, typename C>
SharedPtr<T, C>& SharedPtr<T, C>::operator=(const SharedPtr& other) {
    if (*this != other) {
        clear();
        copy(other);
    }
    return *this;
}

template<typename T, typename C>
SharedPtr<T, C>& SharedPtr<T, C>::operator=(const WeakPtr<T, C>& weakPtr) {
    return this->operator=(weakPtr.tryLock());
}

template<typename T, typename C>
SharedPtr<T, C>& SharedPtr<T, C>::operator=(std::nullptr_t /*ptr*/) {
    clear();
    return *this;
}

template<typename T, typename C>
template<typename D, typename>
SharedPtr<T, C>& SharedPtr<T, C>::operator=(const SharedPtr<D, C>& other) noexcept {
    clear();
    auto tmp{DynamicCast<T>(other)};
    swap(tmp);
    return *this;
}

template<typename T, typename C>
template<typename D, typename>
SharedPtr<T, C>& SharedPtr<T, C>::operator=(SharedPtr<D, C>&& other) noexcept {
    clear();
    auto tmp{DynamicCast<T>(std::move(other))};
    swap(tmp);
    return *this;
}

template<typename T, typename C>
constexpr bool SharedPtr<T, C>::operator==(std::nullptr_t /*ptr*/) const noexcept {
    return m_controlBlock == nullptr;
}

template<typename T, typename C>
constexpr bool SharedPtr<T, C>::operator==(const SharedPtr& other) const noexcept {
    return m_ptr == other.m_ptr;
}

template<typename T, typename C>
void SharedPtr<T, C>::clear() {
    if (m_controlBlock) {
        m_controlBlock->template releaseStrongly<T>();
        m_controlBlock = nullptr;
        m_ptr = nullptr;
    }
}

template<typename T, typename C>
typename SharedPtr<T, C>::ConstReferenceType SharedPtr<T, C>::operator*() const {
    return *get();
}

template<typename T, typename C>
typename SharedPtr<T, C>::ReferenceType SharedPtr<T, C>::operator*() {
    return *get();
}

template<typename T, typename C>
typename SharedPtr<T, C>::ConstPointerType SharedPtr<T, C>::operator->() const {
    return get();
}

template<typename T, typename C>
typename SharedPtr<T, C>::PointerType SharedPtr<T, C>::operator->() {
    return get();
}

template<typename T, typename C>
typename SharedPtr<T, C>::RefCountType SharedPtr<T, C>::strongRefCount() const noexcept {
    return (m_controlBlock ? m_controlBlock->strongRefCount() : 0);
}

template<typename T, typename C>
typename SharedPtr<T, C>::RefCountType SharedPtr<T, C>::weakRefCount() const noexcept {
    return (m_controlBlock ? m_controlBlock->weakRefCount() : 0);
}

template<typename T, typename C>
constexpr SharedPtr<T, C>::SharedPtr(ControlBlockType* controlBlock, PointerType ptr) noexcept:
m_ptr(ptr ? ptr : reinterpret_cast<PointerType>(controlBlock->getUserData())),
m_controlBlock(controlBlock)
{}

template<typename T, typename C>
void SharedPtr<T, C>::copy(const SharedPtr& other) {
    if (other.m_controlBlock && other.m_controlBlock->tryAcquireStrongly()) {
        m_controlBlock = other.m_controlBlock;
        m_ptr = other.m_ptr;
    }
}

template<typename T, typename C>
void SharedPtr<T, C>::swap(SharedPtr& other) noexcept {
    std::swap(m_controlBlock, other.m_controlBlock);
    std::swap(m_ptr, other.m_ptr);
}

template<typename T, typename C>
typename SharedPtr<T, C>::ConstPointerType SharedPtr<T, C>::get() const {
    ASSERTION(m_ptr, std::runtime_error, "Attempt to deref null shared ptr")
    return m_ptr;
}

template<typename T, typename C>
typename SharedPtr<T, C>::PointerType SharedPtr<T, C>::get() {
    ASSERTION(m_ptr, std::runtime_error, "Attempt to deref null shared ptr")
    return m_ptr;
}

// Implementation of WeakPtr methods
template<typename T, typename C>
WeakPtr<T, C>::WeakPtr(const WeakPtr& other) {
    (void)this->operator=(other);
}

template<typename T, typename C>
WeakPtr<T, C>::WeakPtr(WeakPtr&& other) noexcept {
    (void)this->operator=(std::move(other));
}

template<typename T, typename C>
WeakPtr<T, C>::WeakPtr(const SharedPtr<T, C>& sharedPtr) {
    (void)this->operator=(sharedPtr);
}

template<typename T, typename C>
WeakPtr<T, C>& WeakPtr<T, C>::operator=(const WeakPtr& other) {
    clear();
    copy(other);
    return *this;
}

template<typename T, typename C>
WeakPtr<T, C>& WeakPtr<T, C>::operator=(WeakPtr&& other) noexcept {
    clear();
    swap(other);
    return *this;
}

template<typename T, typename C>
WeakPtr<T, C>& WeakPtr<T, C>::operator=(const SharedPtr<T, C>& sharedPtr) {
    clear();
    copy(sharedPtr);
    return *this;
}

template<typename T, typename C>
WeakPtr<T, C>::~WeakPtr() {
    clear();
}

template<typename T, typename C>
void WeakPtr<T, C>::clear() {
    if (m_controlBlock) {
        m_controlBlock->template releaseWeakly<T>();
        m_controlBlock = nullptr;
    }
}

template<typename T, typename C>
SharedPtr<T, C> WeakPtr<T, C>::tryLock() const noexcept {
    if (m_controlBlock != nullptr && m_controlBlock->tryAcquireStrongly()) {
        return SharedPtr<T, C>{m_controlBlock};
    }
    return SharedPtr<T, C>{nullptr};
}

template<typename T, typename C>
bool WeakPtr<T, C>::isExpired() const noexcept {
    return strongRefCount() == 0;
}

template<typename T, typename C>
typename WeakPtr<T, C>::RefCountType WeakPtr<T, C>::strongRefCount() const noexcept {
    return (m_controlBlock ? m_controlBlock->strongRefCount() : 0);
}

template<typename T, typename C>
typename WeakPtr<T, C>::RefCountType WeakPtr<T, C>::weakRefCount() const noexcept {
    return (m_controlBlock ? m_controlBlock->weakRefCount() : 0);
}

template<typename T, typename C>
void WeakPtr<T, C>::copy(const WeakPtr& other) {
    if (other.m_controlBlock && other.m_controlBlock->tryAcquireWeakly()) {
        m_controlBlock = other.m_controlBlock;
    }
}

template<typename T, typename C>
void WeakPtr<T, C>::copy(const SharedPtr<T, C>& sharedPtr) {
    if (sharedPtr.m_controlBlock && sharedPtr.m_controlBlock->tryAcquireWeakly()) {
        m_controlBlock = sharedPtr.m_controlBlock;
    }
}

template<typename T, typename C>
void WeakPtr<T, C>::swap(WeakPtr& other) noexcept {
    std::swap(m_controlBlock, other.m_controlBlock);
}

// Implementation of Pointer casts
template<typename D, typename S, typename _C>
SharedPtr<D, _C> DynamicCast(const SharedPtr<S, _C>& srcSharedPtr) {
    return DynamicCast<D, S, _C>(SharedPtr<S, _C>{srcSharedPtr});
}

template<typename D, typename S, typename _C>
SharedPtr<D, _C> StaticCast(const SharedPtr<S, _C>& srcSharedPtr) {
    return StaticCast<D, S, _C>(SharedPtr<S, _C>{srcSharedPtr});
}

template<typename D, typename S, typename _C>
SharedPtr<D, _C> ReinterpretCast(const SharedPtr<S, _C>& srcSharedPtr) {
    return ReinterpretCast<D, S, _C>(SharedPtr<S, _C>{srcSharedPtr});
}

template<typename D, typename S, typename _C>
SharedPtr<D, _C> ConstCast(const SharedPtr<S, _C>& srcSharedPtr) {
    return ConstCast<D, S, _C>(SharedPtr<S, _C>{srcSharedPtr});
}

template<typename D, typename S, typename _C>
SharedPtr<D, _C> DynamicCast(SharedPtr<S, _C>&& srcSharedPtr) {
    static_assert(std::is_base_of_v<D, S> || std::is_base_of_v<S, D> && "DynamicCast requires polymorphic types");
    SharedPtr<D, _C> dstSharedPtr;
    if (srcSharedPtr != nullptr) {
        if (D* ptr = dynamic_cast<D*>(srcSharedPtr.m_ptr)) {
            dstSharedPtr = SharedPtr<D, _C>{srcSharedPtr.m_controlBlock, ptr};
            srcSharedPtr.m_controlBlock = nullptr;
            srcSharedPtr.m_ptr = nullptr;
        }
    }
    return dstSharedPtr;
}

template<typename D, typename S, typename _C>
SharedPtr<D, _C> StaticCast(SharedPtr<S, _C>&& srcSharedPtr) {
    SharedPtr<D, _C> dstSharedPtr;
    if (srcSharedPtr != nullptr) {
        if (D* ptr = static_cast<D*>(srcSharedPtr.m_ptr)) {
            dstSharedPtr = SharedPtr<D, _C>{srcSharedPtr.m_controlBlock, ptr};
            srcSharedPtr.m_controlBlock = nullptr;
            srcSharedPtr.m_ptr = nullptr;
        }
    }
    return dstSharedPtr;
}

template<typename D, typename S, typename _C>
SharedPtr<D, _C> ReinterpretCast(SharedPtr<S, _C>&& srcSharedPtr) {
    SharedPtr<D, _C> dstSharedPtr;
    if (srcSharedPtr != nullptr) {
        if (D* ptr = reinterpret_cast<D*>(srcSharedPtr.m_ptr)) {
            dstSharedPtr = SharedPtr<D, _C>{srcSharedPtr.m_controlBlock, ptr};
            srcSharedPtr.m_controlBlock = nullptr;
            srcSharedPtr.m_ptr = nullptr;
        }
    }
    return dstSharedPtr;
}

template<typename D, typename S, typename _C>
SharedPtr<D, _C> ConstCast(SharedPtr<S, _C>&& srcSharedPtr) {
    SharedPtr<D, _C> dstSharedPtr;
    if (srcSharedPtr != nullptr) {
        if (D* ptr = const_cast<D*>(srcSharedPtr.m_ptr)) {
            dstSharedPtr = SharedPtr<D, _C>{srcSharedPtr.m_controlBlock, ptr};
            srcSharedPtr.m_controlBlock = nullptr;
            srcSharedPtr.m_ptr = nullptr;
        }
    }
    return dstSharedPtr;
}


namespace __shared_ptr_impl_details {

template<typename A, typename RC>
class NonAtomicControlBlock final {
public:
    using AllocatorType = A;
    using RefCountType = RC;

    template<typename T, typename ... Arg>
    static NonAtomicControlBlock* Allocate(Arg&& ... arg) {
        static_assert(std::is_same_v<AllocatorType, allocator::DefaultAllocator> &&
                      "AllocatorType must be DefaultAllocator in this case");
        constexpr auto TOTAL_SIZE = sizeof(NonAtomicControlBlock) + sizeof(T);
        AllocatorType allocator;
        std::byte* controlBlockStart = nullptr;
        NonAtomicControlBlock* controlBlock = nullptr;
        T* userData = nullptr;
        try {
            controlBlockStart = allocator.allocate(types::Size{TOTAL_SIZE});
            controlBlock = reinterpret_cast<NonAtomicControlBlock*>(controlBlockStart);
            userData = reinterpret_cast<T*>(controlBlockStart + sizeof(NonAtomicControlBlock));

            allocator.construct(userData, std::forward<Arg>(arg) ... );
            allocator.construct(controlBlock);
        } catch (...) {
            if (userData) {
                allocator.destruct(userData);
            }
            if (controlBlock) {
                allocator.destruct(controlBlock);
            }
            if (controlBlockStart) {
                allocator.deallocate(reinterpret_cast<std::byte*>(controlBlock));
            }
            throw;
        }

        controlBlock->m_strongRefCount = 1;
        return controlBlock;
    }

    template<typename T, typename ... Arg>
    static NonAtomicControlBlock* Allocate(utils::owner::Reference<AllocatorType> allocator, Arg&& ... arg) {
        constexpr auto TOTAL_SIZE = sizeof(NonAtomicControlBlock) + sizeof(T);
        std::byte* controlBlockStart = nullptr;
        NonAtomicControlBlock* controlBlock = nullptr;
        T* userData = nullptr;
        try {
            allocator.accessMutable([&controlBlock, &controlBlockStart, &userData, &arg ...](AllocatorType& _allocator) {
                std::byte* controlBlockStart = _allocator.allocate(types::Size{TOTAL_SIZE});
                controlBlock = reinterpret_cast<NonAtomicControlBlock*>(controlBlockStart);
                auto userData = reinterpret_cast<T*>(controlBlockStart + sizeof(NonAtomicControlBlock));

                _allocator.construct(userData, std::forward<Arg>(arg) ... );
                _allocator.construct(controlBlock);
            });
        } catch (...) {
            allocator.accessMutable([&controlBlock, &controlBlockStart, &userData](AllocatorType& _allocator) {
                if (userData) {
                    _allocator.destruct(userData);
                }
                if (controlBlock) {
                    _allocator.destruct(controlBlock);
                }
                if (controlBlockStart) {
                    _allocator.deallocate(reinterpret_cast<std::byte*>(controlBlock));
                }
            });
            throw;
        }

        controlBlock->m_strongRefCount = 1;
        controlBlock->m_allocator = allocator;
        return controlBlock;
    }

    template<typename T>
    static void Deallocate(NonAtomicControlBlock* controlBlock) {
        if constexpr (std::is_same_v<AllocatorType, allocator::DefaultAllocator>) {
            AllocatorType allocator;
            allocator.destruct(controlBlock);
            allocator.deallocate(reinterpret_cast<std::byte*>(controlBlock));
        } else {
            utils::owner::Reference<AllocatorType> allocator{controlBlock->m_allocator};
            ASSERTION(allocator.isValid(), std::runtime_error, "Attempt to use invalid reference to castom allocator")
            allocator.accessMutable([&controlBlock](AllocatorType& _allocator) {
                _allocator.destruct(controlBlock);
                _allocator.deallocate(reinterpret_cast<std::byte*>(controlBlock));
            });
        }
    }

    bool tryAcquireStrongly() noexcept {
        if (m_strongRefCount == 0) {
            return false;
        } else {
            ++m_strongRefCount;
            return true;
        }
    }
    bool tryAcquireWeakly() noexcept {
        if (weakRefCount() > 0) {
            ++m_weakRefCount;
            return true;
        } else if (weakRefCount() == 0 && strongRefCount() > 0) {
            ++m_weakRefCount;
            return true;
        }
        return false;
    }

    template<typename T>
    void releaseStrongly() {
        --m_strongRefCount;
        if (m_strongRefCount == 0) {
            ++m_weakRefCount;
            clearUserData<T>();
            if (--m_weakRefCount == 0) {
                clearControlBlock<T>();
            }
        }
    }

    template<typename T>
    void releaseWeakly() {
        --m_weakRefCount;
        if (m_weakRefCount == 0 && m_strongRefCount == 0) {
            clearControlBlock<T>();
        }
    }

    RefCountType strongRefCount() const noexcept {
        return m_strongRefCount;
    }

    RefCountType weakRefCount() const noexcept {
        return m_weakRefCount;
    }

    const std::byte* getUserData() const { return (reinterpret_cast<std::byte*>(this) + sizeof(*this)); }
    std::byte* getUserData() { return (reinterpret_cast<std::byte*>(this) + sizeof(*this)); }

private:
    template<typename T>
    void clearUserData() {
        if constexpr (std::is_same_v<AllocatorType, allocator::DefaultAllocator>) {
            AllocatorType allocator;
            allocator.destruct(reinterpret_cast<T*>(getUserData()));
        } else {
            ASSERTION(m_allocator.isValid(), std::runtime_error, "Attempt to use invalid referecen to castom allocator")
            m_allocator.accessMutable([this](AllocatorType& _allocator) {
                _allocator.destruct(reinterpret_cast<T*>(getUserData()));
            });
        }
    }

    template<typename T>
    void clearControlBlock() {
        Deallocate<T>(this);
    }

    RefCountType m_strongRefCount{0};
    RefCountType m_weakRefCount{0};
    utils::owner::Reference<AllocatorType> m_allocator;
    // ... user data
};


template<typename A, typename RC>
class AtomicControlBlock final {
public:
    using AllocatorType = A;
    using RefCountType = RC;

    template<typename T, typename ... Arg>
    static AtomicControlBlock* Allocate(Arg&& ... arg) {
        static_assert(std::is_same_v<AllocatorType, allocator::DefaultAllocator> &&
                      "AllocatorType must be DefaultAllocator in this case");
        constexpr auto TOTAL_SIZE = sizeof(AtomicControlBlock) + sizeof(T);
        AllocatorType allocator;
        std::byte* controlBlockStart = nullptr;
        AtomicControlBlock* controlBlock = nullptr;
        T* userData = nullptr;
        try {
            controlBlockStart = allocator.allocate(TOTAL_SIZE);
            controlBlock = reinterpret_cast<AtomicControlBlock*>(controlBlockStart);
            userData = reinterpret_cast<T*>(controlBlockStart + sizeof(AtomicControlBlock));

            allocator.construct(userData, std::forward<Arg>(arg) ... );
            allocator.construct(controlBlock);
        } catch (...) {
            if (userData) {
                allocator.destruct(userData);
            }
            if (controlBlock) {
                allocator.destruct(controlBlock);
            }
            if (controlBlockStart) {
                allocator.deallocate(reinterpret_cast<std::byte*>(controlBlock));
            }
            throw;
        }

        controlBlock->m_strongRefCount = 1;
        return controlBlock;
    }

    template<typename T, typename ... Arg>
    static AtomicControlBlock* Allocate(utils::owner::Reference<AllocatorType> allocator, Arg&& ... arg) {
        constexpr auto TOTAL_SIZE = sizeof(AtomicControlBlock) + sizeof(T);
        std::byte* controlBlockStart = nullptr;
        AtomicControlBlock* controlBlock = nullptr;
        T* userData = nullptr;
        try {
            allocator.accessMutable([&controlBlock, &controlBlockStart, &userData, &arg ...](AllocatorType& _allocator) {
                std::byte* controlBlockStart = _allocator.allocate(TOTAL_SIZE);
                controlBlock = reinterpret_cast<AtomicControlBlock*>(controlBlockStart);
                auto userData = reinterpret_cast<T*>(controlBlockStart + sizeof(AtomicControlBlock));

                _allocator.construct(userData, std::forward<Arg>(arg) ... );
                _allocator.construct(controlBlock);
            });
        } catch (...) {
            allocator.accessMutable([&controlBlock, &controlBlockStart, &userData](AllocatorType& _allocator) {
                if (userData) {
                    _allocator.destruct(userData);
                }
                if (controlBlock) {
                    _allocator.destruct(controlBlock);
                }
                if (controlBlockStart) {
                    _allocator.deallocate(reinterpret_cast<std::byte*>(controlBlock));
                }
            });
            throw;
        }

        controlBlock->m_strongRefCount = 1;
        controlBlock->m_allocator = allocator;
        return controlBlock;
    }

    template<typename T>
    static void Deallocate(AtomicControlBlock* controlBlock) {
        if constexpr (std::is_same_v<AllocatorType, allocator::DefaultAllocator>) {
            AllocatorType allocator;
            allocator.destruct(controlBlock);
            allocator.deallocate(reinterpret_cast<std::byte*>(controlBlock));
        } else {
            utils::owner::Reference<AllocatorType> allocator{controlBlock->m_allocator};
            ASSERTION(allocator.isValid(), std::runtime_error, "Attempt to use invalid reference to castom allocator")
            allocator.accessMutable([&controlBlock](AllocatorType& _allocator) {
                _allocator.destruct(controlBlock);
                _allocator.deallocate(reinterpret_cast<std::byte*>(controlBlock));
            });
        }
    }

    bool tryAcquireStrongly() noexcept {
        auto current = m_strongRefCount.load();
        while (current > 0) {
            if (m_strongRefCount.compare_exchange_weak(current, current + 1)) {
                return true;
            }
        }
        return false;
    }
    bool tryAcquireWeakly() noexcept {
        auto current = m_weakRefCount.load();
        for (;;) {
            if (current == 0 && m_strongRefCount.load() == 0) {
                break;
            }

            if (m_weakRefCount.compare_exchange_weak(current, current + 1)) {
                return true;
            }
        }
        return false;
    }

    template<typename T>
    void releaseStrongly() {
        const auto strongRefCount = m_strongRefCount.fetch_sub(1) - 1;
        if (strongRefCount == 0) {
            clearUserData<T>();

            if (m_weakRefCount.load() == 0) {
                clearControlBlock<T>();
            }
        }
    }

    template<typename T>
    void releaseWeakly() {
        const auto weakRefCount = m_weakRefCount.fetch_sub(1) - 1;
        if (weakRefCount == 0 && m_strongRefCount.load() == 0) {
            clearControlBlock<T>();
        }
    }

    RefCountType strongRefCount() const noexcept {
        return m_strongRefCount.load();
    }

    RefCountType weakRefCount() const noexcept {
        return m_weakRefCount.load();
    }

    const std::byte* getUserData() const { return (reinterpret_cast<std::byte*>(this) + sizeof(*this)); }
    std::byte* getUserData() { return (reinterpret_cast<std::byte*>(this) + sizeof(*this)); }

private:
    template<typename T>
    void clearUserData() {
        if constexpr (std::is_same_v<AllocatorType, allocator::DefaultAllocator>) {
            AllocatorType allocator;
            allocator.destruct(reinterpret_cast<T*>(getUserData()));
        } else {
            ASSERTION(m_allocator.isValid(), std::runtime_error, "Attempt to use invalid referecen to castom allocator")
            m_allocator.accessMutable([this](AllocatorType& _allocator) {
                _allocator.destruct(reinterpret_cast<T*>(getUserData()));
            });
        }
    }

    template<typename T>
    void clearControlBlock() {
        Deallocate<T>(this);
    }

    std::atomic<RefCountType> m_strongRefCount{0};
    std::atomic<RefCountType> m_weakRefCount{0};
    utils::owner::Reference<AllocatorType> m_allocator;
    // ... user data
};

} //! namespace __shared_ptr_impl_details

template<typename T, typename A = allocator::DefaultAllocator, typename RC = std::uint64_t>
using NonAtomicSharedPtr = SharedPtr<T, __shared_ptr_impl_details::NonAtomicControlBlock<A, RC>>;

template<typename T, typename A = allocator::DefaultAllocator, typename RC = std::uint64_t>
using NonAtomicWeakPtr = WeakPtr<T, __shared_ptr_impl_details::NonAtomicControlBlock<A, RC>>;

template<typename T, typename A = allocator::DefaultAllocator, typename RC = std::uint64_t>
using AtomicSharedPtr = SharedPtr<T, __shared_ptr_impl_details::AtomicControlBlock<A, RC>>;

template<typename T, typename A = allocator::DefaultAllocator, typename RC = std::uint64_t>
using AtomicWeakPtr = WeakPtr<T, __shared_ptr_impl_details::AtomicControlBlock<A, RC>>;

} //! namespace atom::memory

#endif //! ATOM_MEMORY_SHARED_PTR_H
