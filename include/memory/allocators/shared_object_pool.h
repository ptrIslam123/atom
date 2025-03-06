#ifndef ATOM_MEM_POOL_H
#define ATOM_MEM_POOL_H

#include "include/concurrency/synchronizer/fictitious_mutex.h"
#include "include/concurrency/synchronizer/synchronized.h"
#include "include/memory/allocators/default_allocator.h"
#include "include/utils/assertion.h"

#include <exception>
#include <type_traits>
#include <shared_mutex>
#include <unordered_map>

namespace atom::memory::allocator {

class BadSharedObjectPool final : public std::exception {
public:
    explicit BadSharedObjectPool(std::string_view msg);
    virtual const char* what() const noexcept;

private:
    std::string m_msg;
};

/**
 * @brief A thread-safe shared memory pool for managing reusable, immutable objects of type T.
 *
 * This class is designed to minimize memory allocations by reusing existing objects when possible.
 * It is particularly useful in scenarios where the number of unique object types (categories) is
 * significantly smaller than the total number of objects to be managed. For example, if there are
 * 1000 objects but only 10 unique types, this pool will store only 10 instances and reuse them.
 *
 * @note Objects stored in this pool are immutable. They cannot be modified after creation.
 *       The `Pointer` class provides read-only access to the objects.
 *
 * @tparam T The type of objects to be stored in the pool. Must be move-constructible or copy-constructible.
 * @tparam H The hash function type used to identify objects. Defaults to `std::hash<T>`.
 * @tparam E The equality comparator type used to compare objects. Defaults to `std::equal_to<T>`.
 * @tparam M The mutex type used for thread synchronization. Defaults to `std::shared_mutex`.
 * @tparam A The allocator type used for memory management. Defaults to `DefaultAllocator`.
 */
template<
    typename T,
    typename H = std::hash<T>,
    typename E = std::equal_to<T>,
    typename M = std::shared_mutex,
    typename A = DefaultAllocator
> class SharedObjectPool final {
public:
    using ValueType =  std::remove_cv_t<T>;
    using HashType = H;
    using EqType = E;
    using SharedMutexType = M;
    using AllocatorType = A;

    /**
     * @brief A read-only pointer to an object stored in the SharedMemoryPool.
     *
     * This class provides safe, read-only access to objects managed by the SharedMemoryPool.
     * It does not allow modification of the underlying object.
     */
    class Pointer final {
    public:
        using IdType = std::size_t; ///< The hash identifier of the object.

        Pointer(const Pointer& ) noexcept = default;
        Pointer& operator=(const Pointer& ) noexcept = default;

        Pointer(Pointer&& other) noexcept;
        Pointer& operator=(Pointer&& other) noexcept;
        Pointer(std::nullptr_t ptr) noexcept;
        Pointer& operator=(std::nullptr_t ptr) noexcept;

        /**
         * @brief Dereferences the pointer to access the underlying object.
         * @return A const reference to the object.
         */
        const T& operator*() const;

        /**
         * @brief Accesses the underlying object via pointer semantics.
         * @return A const pointer to the object.
         */
        const T* operator->() const;

        /**
         * @brief Accesses the underlying object.
         * @return A const pointer to the object.
         */
        const T* get() const;

        bool operator==(const Pointer& other) const;
        bool operator==(std::nullptr_t ptr) const;

        void swap(Pointer& ptr);

    private:
        friend SharedObjectPool;
        explicit Pointer(IdType id, SharedObjectPool* memPool);

        IdType m_id;
        SharedObjectPool* m_memPool;
    };

    explicit SharedObjectPool() = default;

    /**
     * @brief Allocates an object in the pool or reuses an existing one.
     *
     * This method creates a new object of type T using the provided arguments. If an object with the same
     * value already exists in the pool, it is reused instead of creating a new one. This minimizes memory
     * allocations and improves performance.
     *
     * @tparam Arg The types of arguments passed to the constructor of T.
     * @param arg The arguments passed to the constructor of T.
     * @return A Pointer to the allocated or reused object.
     *
     * @note If the object already exists in the pool, no new memory is allocated.
     */
    template<typename D, typename ... Arg>
    Pointer allocate(Arg&& ... arg);

    /**
     * @brief Deallocates an object from the pool.
     *
     * This method removes the object associated with the given Pointer from the pool and releases its memory.
     * If the object is not found in the pool, an exception is thrown.
     *
     * @param ptr A Pointer to the object to be deallocated.
     * @throws std::runtime_error If the Pointer is invalid or the object is not found in the pool.
     */
    void deallocate(Pointer&& ptr);

    /**
     * @brief Reserves space in the pool for a specified number of objects.
     *
     * This method pre-allocates memory to store the specified number of objects, improving performance
     * when adding multiple objects to the pool.
     *
     * @param size The number of objects to reserve space for.
     */
    void reserve(std::size_t size);

    T* release(Pointer&& ptr);

    /**
     * @brief Clear all allocated objects from pool.
     */
    void clear();

    /**
     * @brief return pool allocator.
     */
    AllocatorType& getAllocator();

private:
    friend Pointer;

    using IdType = Pointer::IdType;
    using PoolType = std::unordered_map<IdType, T*>;

    const T* deref(const Pointer& ptr);

    AllocatorType m_allocator;
    atom::concurency::synchronizer::Synchronized<PoolType, SharedMutexType> m_pool;
};

template<typename T, typename H = std::hash<T>, typename E = std::equal_to<T>, typename A = DefaultAllocator>
using NonThreadSafeSharedObjectPool = SharedObjectPool<T, H, E, concurrentce::synchronizer::SharedFictitiousMutex, A>;

template<typename T, typename H = std::hash<T>, typename E = std::equal_to<T>, typename A = DefaultAllocator>
using ThreadSafeSharedObjectPool = SharedObjectPool<T, H, E, std::shared_mutex, A>;


template<typename T, typename H, typename E, typename M, typename A>
void SharedObjectPool<T, H, E, M, A>::reserve(const std::size_t size) {
    auto pool = m_pool.genericLock();
    pool->reserve(size);
}

template<typename T, typename H, typename E, typename M, typename A>
T* SharedObjectPool<T, H, E, M, A>::release(Pointer&& ptr) {
    ASSERTION(ptr != nullptr, BadSharedObjectPool, "Attempt to deallocate nullptr pointer")

    auto pool = m_pool.genericLock();
    auto it = pool->find(ptr.m_id);
    if (it != pool->cend()) {
        T* ptrObject = it->second;
        pool->erase(it);
        return ptrObject;
    } else {
        ASSERTION(false, BadSharedObjectPool, "Invalid shared object pool pointer: could not find object with pointer into pool")
    }
}

template<typename T, typename H, typename E, typename M, typename A>
void SharedObjectPool<T, H, E, M, A>::clear() {
    auto pool = m_pool.genericLock();
    for (const auto& [key, ptrObject] : *pool) {
        m_allocator.destruct(ptrObject);
        m_allocator.deallocate(reinterpret_cast<std::byte*>(const_cast<T*>(ptrObject)));
    }
}

template<typename T, typename H, typename E, typename M, typename A>
template<typename D, typename ... Arg>
SharedObjectPool<T, H, E, M, A>::Pointer SharedObjectPool<T, H, E, M, A>::allocate(Arg&& ... arg) {
    static_assert(std::is_same_v<T, D> || std::is_base_of_v<T, D> && "D must be T or derived type");
    D object{std::forward<Arg>(arg) ...};
    const auto hash = HashType{}.operator()(object);
    auto pool = m_pool.genericLock();
    auto it = pool->find(hash);
    if (it != pool->cend() && EqType{}.operator()(*it->second, object)) {
        return Pointer{it->first, this};
    } else {
        auto ptrObject = reinterpret_cast<D*>(m_allocator.allocate(sizeof(D)));
        // construct object
        {
            if constexpr (std::is_nothrow_move_constructible_v<D>) {
                m_allocator.template construct<D>(ptrObject, std::move(object));
            } else if constexpr (std::is_move_constructible_v<D>) {
                try {
                    m_allocator.template construct<D>(ptrObject, std::move(object));
                } catch (const std::exception& e) {
                    ASSERTION(false, BadSharedObjectPool, "Throw move constructor with erorr: " + std::string{e.what()})
                } catch (...) {
                    ASSERTION(false, BadSharedObjectPool, "Throw move constructor")
                }
            } else if constexpr (std::is_nothrow_copy_constructible_v<D>) {
                m_allocator.template construct<D>(ptrObject, object);
            } else if constexpr (std::is_copy_constructible_v<D>) {
                try {
                    m_allocator.template construct<D>(ptrObject, object);
                } catch (const std::exception& e) {
                    ASSERTION(false, BadSharedObjectPool, "Throw copy constructor with erorr: " + std::string{e.what()})
                } catch (...) {
                    ASSERTION(false, BadSharedObjectPool, "Throw copy constructor")
                }
            } else {
                try {
                    m_allocator.template construct<D>(ptrObject, std::forward<Arg>(arg) ...);
                } catch (const std::exception& e) {
                    ASSERTION(false, BadSharedObjectPool, "Throw constructor with erorr: " + std::string{e.what()})
                } catch (...) {
                    ASSERTION(false, BadSharedObjectPool, "Throw constructor")
                }
            }
        }
        pool->insert({hash, ptrObject});
        return Pointer{hash, this};
    }
}

template<typename T, typename H, typename E, typename M, typename A>
void SharedObjectPool<T, H, E, M, A>::deallocate(Pointer&& ptr) {
    ASSERTION(ptr != nullptr, BadSharedObjectPool, "Attempt to deallocate nullptr pointer")

    T const* object = nullptr;
    {
        auto pool = m_pool.genericLock();
        auto it = pool->find(ptr.m_id);
        if (it != pool->cend()) {
            object = it->second;
            pool->erase(it);
        } else {
            ASSERTION(false, BadSharedObjectPool, "Invalid shared object pool pointer: could not find object with pointer into pool")
        }
    }

    assert(object);
    m_allocator.destruct(object);
    m_allocator.deallocate(reinterpret_cast<std::byte*>(const_cast<T*>(object)));
}

template<typename T, typename H, typename E, typename M, typename A>
const T* SharedObjectPool<T, H, E, M, A>::deref(const Pointer& ptr) {
    ASSERTION(ptr != nullptr, BadSharedObjectPool, "Attempt to deref nullptr pointer")

    auto pool = m_pool.sharedLock();
    auto it = pool->find(ptr.m_id);
    if (it != pool->cend()) {
        return it->second;
    } else {
        ASSERTION(false, BadSharedObjectPool, "Invalid shared object pool pointer: could not find object with pointer into pool")
    }
}

template<typename T, typename H, typename E, typename M, typename A>
const T& SharedObjectPool<T, H, E, M, A>::Pointer::operator*() const {
    return *get();
}

template<typename T, typename H, typename E, typename M, typename A>
const T* SharedObjectPool<T, H, E, M, A>::Pointer::operator->() const {
    return get();
}

template<typename T, typename H, typename E, typename M, typename A>
const T* SharedObjectPool<T, H, E, M, A>::Pointer::get() const {
    return m_memPool->deref(*this);
}

template<typename T, typename H, typename E, typename M, typename A>
SharedObjectPool<T, H, E, M, A>::Pointer::Pointer(Pointer&& other) noexcept {
    this->operator=(std::move(other));
}

template<typename T, typename H, typename E, typename M, typename A>
SharedObjectPool<T, H, E, M, A>::Pointer& SharedObjectPool<T, H, E, M, A>::Pointer::operator=(Pointer&& other) noexcept {
    m_id = 0;
    m_memPool = nullptr;
    swap(other);
    return *this;
}

template<typename T, typename H, typename E, typename M, typename A>
SharedObjectPool<T, H, E, M, A>::Pointer::Pointer(std::nullptr_t ptr) noexcept {
    this->operator=(ptr);
}

template<typename T, typename H, typename E, typename M, typename A>
SharedObjectPool<T, H, E, M, A>::Pointer& SharedObjectPool<T, H, E, M, A>::Pointer::operator=(std::nullptr_t ptr) noexcept {
    m_id = 0;
    m_memPool = nullptr;
    return *this;
}

template<typename T, typename H, typename E, typename M, typename A>
bool SharedObjectPool<T, H, E, M, A>::Pointer::operator==(const Pointer& other) const {
    return (m_id == other.m_id) && (m_memPool == other.m_memPool);
}

template<typename T, typename H, typename E, typename M, typename A>
bool SharedObjectPool<T, H, E, M, A>::Pointer::operator==(std::nullptr_t ptr) const {
    return (m_id == 0 && m_memPool == nullptr);
}

template<typename T, typename H, typename E, typename M, typename A>
void SharedObjectPool<T, H, E, M, A>::Pointer::swap(Pointer& ptr) {
    std::swap(m_id, ptr.m_id);
    std::swap(m_memPool, ptr.m_memPool);
}

template<typename T, typename H, typename E, typename M, typename A>
SharedObjectPool<T, H, E, M, A>::Pointer::Pointer(IdType id, SharedObjectPool* memPool):
m_id(id), m_memPool(memPool) {}


inline BadSharedObjectPool::BadSharedObjectPool(const std::string_view msg):
m_msg("BadSharedObjectPool: " + std::string{msg}) {}

inline const char* BadSharedObjectPool::what() const noexcept {
    return m_msg.data();
}

}

//! namespace atom::memory::allocator

#endif //! ATOM_SHARED_MEM_POOL_H
