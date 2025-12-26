#ifndef ATOM_LOCK_FREE_STATIC_ALLOCATOR_H
#define ATOM_LOCK_FREE_STATIC_ALLOCATOR_H

#include "include/memory/allocators/lock_free/fixed/memory_pool.h"
#include "include/utils/assertion.h"

#include <ostream>
#include <stdexcept>
#include <cassert>

#define CALC_STATIC_ALLOCATION_STAT(size, n) \
                                    const auto overhead = size - n; \
                                    assert(overhead >= 0);  \
                                    (void)m_totalMemoryOverhead.fetch_add(overhead);

#define CALC_STATIC_DEALLOCATION_STAT(size, n) \
                                    const auto overhead = size - n; \
                                    assert(overhead >= 0);  \
                                    (void)m_totalMemoryOverhead.fetch_sub(overhead);

namespace atom::memory::allocator::lock_free::fixed {

template<typename T, std::size_t ... >
class StaticAllocator;


/**
 * @brief A static memory allocator for fixed-size objects
 *
 * @tparam T Type of objects to allocate
 * @tparam N Maximum number of objects that can be allocated
 *
 * This allocator uses a lock-free memory pool for efficient allocations.
 * It supports objects of type T and its derived classes.
 */
template<typename T, std::size_t N>
class StaticAllocator<T, N> {
public:
    /**
     * @brief Allocates memory for an object
     *
     * @param size Number of bytes to allocate (must be <= sizeof(T))
     * @return std::byte* Pointer to allocated memory
     *
     * @note The actual allocation size is fixed to sizeof(T)
     */
    std::byte* allocate(std::size_t size);

    /**
     * @brief Deallocates previously allocated memory
     *
     * @param start Pointer to memory to deallocate
     */
    void deallocate(std::byte* start);

    /**
     * @brief Constructs an object in allocated memory
     *
     * @tparam U Type of object to construct (must be T or derived from T)
     * @tparam Arg Types of constructor arguments
     * @param object Pointer to memory where to construct the object
     * @param arg Arguments to forward to constructor
     *
     * @throws std::runtime_error if object pointer is null
     */
    template<typename U, typename... Arg>
    void construct(U* object, Arg&&... arg);

    /**
     * @brief Destroys a constructed object
     *
     * @tparam U Type of object to destroy (must be T or derived from T)
     * @param object Pointer to object to destroy
     */
    template<typename U>
    void destruct(U* object);

private:
    using LockFreeMemoryPool = MemoryPool<0, sizeof(T), N, N / 4>;
    LockFreeMemoryPool m_pool;
};


/**
 * @brief A static memory allocator for variable-sized blocks
 *
 * @tparam N64 Number of 64-byte blocks
 * @tparam N128 Number of 128-byte blocks
 * @tparam N512 Number of 512-byte blocks
 * @tparam N1024 Number of 1024-byte (1KB) blocks
 * @tparam N4096 Number of 4096-byte (4KB) blocks
 * @tparam N8192 Number of 8192-byte (8KB) blocks
 * @tparam N16384 Number of 16384-byte (16KB) blocks
 * @tparam N32768 Number of 32768-byte (32KB) blocks
 * @tparam N65536 Number of 65536-byte (64KB) blocks
 *
 * This allocator uses multiple lock-free memory pools for different block sizes.
 * It automatically selects the appropriate pool based on requested size.
 */
template<
        std::size_t N64,
        std::size_t N128,
        std::size_t N512,
        std::size_t N1024,      // 1KB
        std::size_t N4096,      // 4KB
        std::size_t N8192,      // 8KB
        std::size_t N16384,     // 16KB
        std::size_t N32768,     // 32KB
        std::size_t N65536      // 65KB
> class StaticAllocator<void*, N64, N128, N512, N1024, N4096, N8192, N16384, N32768, N65536> : private
        MemoryPool<1, 64, N64, N64 / 4>,
        MemoryPool<2, 128, N128, N128 / 4>,
        MemoryPool<3, 512, N512, N512 / 4>,
        MemoryPool<4, 1024, N1024, N1024 / 4>,
        MemoryPool<5, 4096, N4096, N4096 / 4>,
        MemoryPool<6, 8192, N8192, N8192 / 4>,
        MemoryPool<7, 16384, N16384, N16384 / 4>,
        MemoryPool<8, 32768, N32768, N32768 / 4>,
        MemoryPool<9, 65536, N65536, N65536 / 4>
{
public:
    template<std::size_t Id, std::size_t BlockSize, std::size_t Capacity>
    using LockFreeMemoryPool = ::atom::memory::allocator::lock_free::fixed::MemoryPool<
        Id, BlockSize, Capacity, Capacity / 4
    >;
    using PoolForMemReq64 = LockFreeMemoryPool<1, 64, N64>;
    using PoolForMemReq128 = LockFreeMemoryPool<2, 128, N128>;
    using PoolForMemReq512 = LockFreeMemoryPool<3, 512, N512>;
    using PoolForMemReq1024 = LockFreeMemoryPool<4, 1024, N1024>;
    using PoolForMemReq4096 = LockFreeMemoryPool<5, 4096, N4096>;
    using PoolForMemReq8192 = LockFreeMemoryPool<6, 8192, N8192>;
    using PoolForMemReq16384 = LockFreeMemoryPool<7, 16384, N16384>;
    using PoolForMemReq32768 = LockFreeMemoryPool<8, 32768, N32768>;
    using PoolForMemReq65536 = LockFreeMemoryPool<9, 65536, N65536>;

    /**
     * @brief Allocates memory block of requested size
     *
     * @param size Requested block size in bytes (must be <= 65536)
     * @return std::byte* Pointer to allocated memory
     * @throws std::runtime_error in case allocation failed
     */
    std::byte* allocate(std::size_t size);

    /**
     * @brief Deallocates previously allocated memory block
     *
     * @param ptr Pointer to memory block to deallocate
     * @param size Size of the memory block (must match allocation size)
     * @throws std::runtime_error in case deallocation failed
     */
    void deallocate(std::byte* ptr, std::size_t size);

    /**
     * @brief Constructs an object in allocated memory
     *
     * @tparam U Type of object to construct
     * @tparam Arg Types of constructor arguments
     * @param object Pointer to memory where to construct
     * @param arg Arguments to forward to constructor
     * @throws std::runtime_error if object pointer is null
     */
    template<typename U, typename ... Arg>
    void construct(U* object, Arg&& ... arg);

    /**
     * @brief Destroys a constructed object
     *
     * @tparam U Type of object to destroy
     * @param object Pointer to object to destroy
     */
    template<typename U>
    void destruct(U* object);

    void dumpStats(std::ostream& os) {
#if defined(STATIC_ALLOCATOR_STATISTIC_MODE)
        os << "StaticAllocator<void*, "
           << "N64=" << N64 << ", "
           << "N128=" << N128 << ", "
           <<  "N512=" << N512 << ", "
           << "N1024=" << N1024 << ", "
           << "N4096=" << N4096 << ", "
           << "N8192=" << N8192 << ", "
           << "N16384=" << N16384  << ", "
           << "N32768=" << N32768 << ","
           << "N65536=" << N65536 << "{ "
           << "total memory usage=" << m_totalMemoryUsage.load() << ", "
           << "total memory overhead=" << m_totalMemoryOverhead.load() << " "
           << "}\n";
#endif
    }

private:
#if defined(STATIC_ALLOCATOR_STATISTIC_MODE)
    std::atomic<std::size_t> m_totalMemoryUsage{0};
    std::atomic<std::size_t> m_totalMemoryOverhead{0};
#endif
};


template<typename T, std::size_t N>
std::byte* StaticAllocator<T, N>::allocate(std::size_t size) {
    return m_pool.allocate(size);
}

template<typename T, std::size_t N>
void StaticAllocator<T, N>::deallocate(std::byte* start) {
    m_pool.deallocate(start);
}

template<typename T, std::size_t N>
template<typename U, typename... Arg>
void StaticAllocator<T, N>::construct(U* object, Arg&&... arg) {
    static_assert(std::is_same_v<T, U> || std::is_base_of_v<T, U>,
                 "Type mismatch in construct");
    ASSERTION(object, std::runtime_error, "Object pointer is null")
    new(object) U(std::forward<Arg>(arg)...);
}

template<typename T, std::size_t N>
template<typename U>
void StaticAllocator<T, N>::destruct(U* object) {
    static_assert(std::is_same_v<T, U> || std::is_base_of_v<T, U>,
                 "Type mismatch in destroy");
    if (object) {
        object->~U();
    }
}

//////////////////////////////////////////////////////////////////////////////////////


template<std::size_t N64, std::size_t N128, std::size_t N512, std::size_t N1024,
         std::size_t N4096, std::size_t N8192, std::size_t N16384,
         std::size_t N32768, std::size_t N65536>
std::byte* StaticAllocator<void*, N64, N128, N512, N1024, N4096, N8192, N16384, N32768, N65536>::allocate(const std::size_t size) {
#if defined(STATIC_ALLOCATOR_STATISTIC_MODE)
            (void)m_totalMemoryUsage.fetch_add(size);
#endif
        ASSERTION(size <= 65536, std::runtime_error,
                  "Could not handle memory request with size=" + std::to_string(size))
        if constexpr (N64 > 0) {
            if (size <= 64) {
#if defined(STATIC_ALLOCATOR_STATISTIC_MODE)
                CALC_STATIC_ALLOCATION_STAT(size, 64)
#endif
                return static_cast<PoolForMemReq64*>(this)->allocate();
            }
        }
        if constexpr (N128 > 0) {
            if (size <= 128) {
#if defined(STATIC_ALLOCATOR_STATISTIC_MODE)
                CALC_STATIC_ALLOCATION_STAT(size, 128)
#endif
                return static_cast<PoolForMemReq128*>(this)->allocate();
            }
        }
        if constexpr (N512 > 0) {
            if (size <= 512) {
#if defined(STATIC_ALLOCATOR_STATISTIC_MODE)
                CALC_STATIC_ALLOCATION_STAT(size, 512)
#endif
                return static_cast<PoolForMemReq512*>(this)->allocate();
            }
        }
        if constexpr (N1024 > 0) {
            if (size <= 1024) {
#if defined(STATIC_ALLOCATOR_STATISTIC_MODE)
                CALC_STATIC_ALLOCATION_STAT(size, 1024)
#endif
                return static_cast<PoolForMemReq1024*>(this)->allocate();
            }
        }
        if constexpr (N4096 > 0) {
            if (size <= 4096) {
#if defined(STATIC_ALLOCATOR_STATISTIC_MODE)
                CALC_STATIC_ALLOCATION_STAT(size, 4096)
#endif
                return static_cast<PoolForMemReq4096*>(this)->allocate();
            }
        }
        if constexpr (N8192 > 0) {
            if (size <= 8192) {
#if defined(STATIC_ALLOCATOR_STATISTIC_MODE)
                CALC_STATIC_ALLOCATION_STAT(size, 8192)
#endif
                return static_cast<PoolForMemReq8192*>(this)->allocate();
            }
        }
        if constexpr (N16384 > 0) {
            if (size <= 16384) {
#if defined(STATIC_ALLOCATOR_STATISTIC_MODE)
                CALC_STATIC_ALLOCATION_STAT(size, 16384)
#endif
                return static_cast<PoolForMemReq16384*>(this)->allocate();
            }
        }
        if constexpr (N32768 > 0) {
            if (size <= 32768) {
#if defined(STATIC_ALLOCATOR_STATISTIC_MODE)
                CALC_STATIC_ALLOCATION_STAT(size, 32768)
#endif
                return static_cast<PoolForMemReq32768*>(this)->allocate();
            }
        }
        if constexpr (N65536 > 0) {
            if (size <= 65536) {
#if defined(STATIC_ALLOCATOR_STATISTIC_MODE)
                CALC_STATIC_ALLOCATION_STAT(size, 65536)
#endif
                return static_cast<PoolForMemReq65536*>(this)->allocate();
            }
        }
        assert(false);
        return nullptr;
}

template<std::size_t N64, std::size_t N128, std::size_t N512, std::size_t N1024,
         std::size_t N4096, std::size_t N8192, std::size_t N16384,
         std::size_t N32768, std::size_t N65536>
void StaticAllocator<void*, N64, N128, N512, N1024, N4096, N8192, N16384, N32768, N65536>::deallocate(std::byte *const ptr, const std::size_t size) {
#if defined(STATIC_ALLOCATOR_STATISTIC_MODE)
            (void)m_totalMemoryUsage.fetch_sub(size);
#endif
        if (!ptr) [[unlikely]] {
            return;
        }

        ASSERTION(size <= 65536, std::runtime_error, "Could not handle memory request with size=" + std::to_string(size))
        if constexpr (N64 > 0) {
            if (size <= 64) {
#if defined(STATIC_ALLOCATOR_STATISTIC_MODE)
            CALC_STATIC_DEALLOCATION_STAT(size, 64)
#endif
                return static_cast<PoolForMemReq64*>(this)->deallocate(ptr);
            }
        }
        if constexpr (N128 > 0) {
            if (size <= 128) {
#if defined(STATIC_ALLOCATOR_STATISTIC_MODE)
            CALC_STATIC_DEALLOCATION_STAT(size, 128)
#endif
                return static_cast<PoolForMemReq128*>(this)->deallocate(ptr);
            }
        }
        if constexpr (N512 > 0) {
            if (size <= 512) {
#if defined(STATIC_ALLOCATOR_STATISTIC_MODE)
            CALC_STATIC_DEALLOCATION_STAT(size, 512)
#endif
                return static_cast<PoolForMemReq512*>(this)->deallocate(ptr);
            }
        }
        if constexpr (N1024 > 0) {
            if (size <= 1024) {
#if defined(STATIC_ALLOCATOR_STATISTIC_MODE)
            CALC_STATIC_DEALLOCATION_STAT(size, 1024)
#endif
                return static_cast<PoolForMemReq1024*>(this)->deallocate(ptr);
            }
        }
        if constexpr (N4096 > 0) {
            if (size <= 4096) {
#if defined(STATIC_ALLOCATOR_STATISTIC_MODE)
            CALC_STATIC_DEALLOCATION_STAT(size, 4096)
#endif
                return static_cast<PoolForMemReq4096*>(this)->deallocate(ptr);
            }
        }
        if constexpr (N8192 > 0) {
            if (size <= 8192) {
#if defined(STATIC_ALLOCATOR_STATISTIC_MODE)
            CALC_STATIC_DEALLOCATION_STAT(size, 8192)
#endif
                return static_cast<PoolForMemReq8192*>(this)->deallocate(ptr);
            }
        }
        if constexpr (N16384 > 0) {
            if (size <= 16384) {
#if defined(STATIC_ALLOCATOR_STATISTIC_MODE)
            CALC_STATIC_DEALLOCATION_STAT(size, 16384)
#endif
                return static_cast<PoolForMemReq16384*>(this)->deallocate(ptr);
            }
        }
        if constexpr (N32768 > 0) {
            if (size <= 32768) {
#if defined(STATIC_ALLOCATOR_STATISTIC_MODE)
            CALC_STATIC_DEALLOCATION_STAT(size, 32768)
#endif
                return static_cast<PoolForMemReq32768*>(this)->deallocate(ptr);
            }
        }
        if constexpr (N65536 > 0) {
            if (size <= 65536) {
#if defined(STATIC_ALLOCATOR_STATISTIC_MODE)
            CALC_STATIC_DEALLOCATION_STAT(size, 65536)
#endif
                return static_cast<PoolForMemReq65536*>(this)->deallocate(ptr);
            }
        }
}

template<std::size_t N64, std::size_t N128, std::size_t N512, std::size_t N1024,
         std::size_t N4096, std::size_t N8192, std::size_t N16384,
         std::size_t N32768, std::size_t N65536>
template<typename U, typename... Arg>
void StaticAllocator<void*, N64, N128, N512, N1024, N4096, N8192, N16384, N32768, N65536>::construct(U* object, Arg&&... arg) {
    ASSERTION(object, std::runtime_error, "Object pointer is null")
    new(object) U(std::forward<Arg>(arg)...);
}

// Implementation of destruct
template<std::size_t N64, std::size_t N128, std::size_t N512, std::size_t N1024,
         std::size_t N4096, std::size_t N8192, std::size_t N16384,
         std::size_t N32768, std::size_t N65536>
template<typename U>
void StaticAllocator<void*, N64, N128, N512, N1024, N4096, N8192, N16384, N32768, N65536>::destruct(U* object) {
    if (object) {
        object->~U();
    }
}

#undef CALC_STATIC_ALLOCATION_STAT
#undef CALC_STATIC_DEALLOCATION_STAT

} //! namespace atom::memory::allocator::lock_free::fixed

#endif //! ATOM_LOCK_FREE_STATIC_ALLOCATOR_H
