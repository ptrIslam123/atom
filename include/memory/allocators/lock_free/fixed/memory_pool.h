#ifndef ATOM_LOCK_FREE_STATIC_MEMORY_POOL_H
#define ATOM_LOCK_FREE_STATIC_MEMORY_POOL_H

#include "include/containers/fixed/ring_queue.h"
#include "include/utils/assertion.h"

#include <type_traits>
#include <atomic>
#include <algorithm>
#include <optional>
#include <array>
#include <stdexcept>
#include <new>
#include <ostream>

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cassert>

namespace atom::memory::allocator::lock_free::fixed {

/**
 * @brief A thread-safe, lock-free static memory pool allocator.
 *
 * @tparam Id Unique identifier for the memory pool (used for debugging).
 * @tparam BlockSize Size of each memory block (must be a multiple of platform alignment).
 * @tparam Capacity Total number of blocks in the pool (must be a multiple of platform alignment).
 * @tparam CacheSize Per-thread cache size (must be <= Capacity / 2).
 *
 * Allocation complexity:
 * - **O(1)** if a free block is found in the thread-local cache.
 * - **O(Capacity)** in the worst case (linear search through the global pool).
 *
 * Deallocation complexity:
 * - **O(1)** if the thread-local cache has free slots.
 * - **O(1)** (atomic operation) when returning to the global pool.
 *
 * @note The allocator provides amortized O(1) performance when:
 * - CacheSize is properly sized for the workload.
 * - The global pool is not exhausted.
 */
template<std::size_t Id, std::size_t BlockSize, std::size_t Capacity, std::size_t CacheSize>
class MemoryPool {
public:
    static_assert(BlockSize > 0 && BlockSize % alignof(std::max_align_t) == 0,
                  "BlockSize must be multiple of platform alignment");
    static_assert(Capacity % alignof(std::max_align_t) == 0,
                  "Capacity must be multiple of platform alignment");
    static_assert(CacheSize <= static_cast<std::size_t>(Capacity / 2),
                     "CacheSize cannot be greater than Capacity");

    /// Unique pool identifier (debugging aid)
    static constexpr std::size_t ID = Id;
    /// Size of individual memory blocks
    static constexpr std::size_t BLOCK_SIZE = BlockSize;
    /// Total capacity of the pool (number of blocks)
    static constexpr std::size_t CAPACITY = Capacity;
    /// Per-thread cache size (number of blocks)
    static constexpr std::size_t CACHE_SIZE = CacheSize;

    explicit MemoryPool() = default;
    MemoryPool(const MemoryPool& ) = delete;
    MemoryPool(MemoryPool&& ) = delete;
    MemoryPool& operator=(const MemoryPool& ) = delete;
    MemoryPool& operator=(MemoryPool&& ) = delete;

    /**
     * @brief Allocates a memory block.
     * @return Pointer to the allocated memory.
     * @throws std::runtime_error if no blocks are available.
     *
     *  * Allocation complexity:
     * - **O(1)** if a free block is found in the thread-local cache.
     * - **O(Capacity)** in the worst case (linear search through the global pool).
     */
    std::byte* allocate();

    /**
     * @brief Deallocates a memory block.
     * @param ptr Pointer to the memory to deallocate.
     *
     *  * Deallocation complexity:
     * - **O(1)** if the thread-local cache has free slots.
     * - **O(1)** (atomic operation) when returning to the global pool.
     */
    void deallocate(std::byte* ptr);

    /**
     * @brief Dump pool statistics
     */
    void dumpStats(std::ostream& os);

private:
    struct MemoryBlock {
        enum class Status : std::uint8_t {
            Acquired = 0,
            Released
        };

        [[nodiscard]] Status getStatus() const noexcept;
        [[nodiscard]] bool tryAcquire() noexcept;
        [[nodiscard]] bool release() noexcept;

#if !defined(NDEBUG)
        std::size_t canaryStart{ID};
#endif
        std::atomic<Status> status{Status::Released};
        std::array<std::byte, BLOCK_SIZE> rawStorage;
#if !defined(NDEBUG)
        std::size_t canaryEnd{ID};
#endif
    };

    using CachedMemoryBlockStorage = containers::fixed::RingQueue<MemoryBlock*, CACHE_SIZE>;

    constexpr bool checkBlock(const MemoryBlock* block) const noexcept;
    CachedMemoryBlockStorage& getThreadLocalCachedMemoryBlockStorage() noexcept;

    std::array<MemoryBlock, CAPACITY> m_pool;
#if defined(STAT_MODE)
    std::atomic<std::uint64_t> m_allocateCacheMisses{0};
    std::atomic<std::uint64_t> m_allocateCacheHints{0};
    std::atomic<std::uint64_t> m_deallocateCacheMisses{0};
    std::atomic<std::uint64_t> m_deallocateCacheHints{0};
#endif
};

template<std::size_t Id, std::size_t BlockSize, std::size_t Capacity, std::size_t CacheSize>
std::byte* MemoryPool<Id, BlockSize, Capacity, CacheSize>::allocate() {
    auto& localCache = getThreadLocalCachedMemoryBlockStorage();
    if (!localCache.isEmpty()) {
        auto memBlock = localCache.back();
        localCache.dequeue();
#if defined(STAT_MODE)
        (void)m_allocateCacheHints.fetch_add(1);
#endif
        return memBlock->rawStorage.data();
    }

    for (auto& memBlock : m_pool) {
        if (memBlock.tryAcquire()) {
#if defined(STAT_MODE)
            (void)m_allocateCacheMisses.fetch_add(1);
#endif
            return memBlock.rawStorage.data();
        }
    }
    ASSERTION(false, std::runtime_error, "Could not allocate a new memory block with size=" + std::to_string(BlockSize))
}

template<std::size_t Id, std::size_t BlockSize, std::size_t Capacity, std::size_t CacheSize>
void MemoryPool<Id, BlockSize, Capacity, CacheSize>::deallocate(std::byte* ptr) {
    if (!ptr) [[unlikely]] {
        return;
    }

    MemoryBlock* block = reinterpret_cast<MemoryBlock*>(ptr);
    ASSERTION(block, std::runtime_error, "Broken memory pointer")
#if !defined(NDEBUG)
    //ASSERTION(checkBlock(block), std::runtime_error, "Broken memory block")
#endif
    auto& localCache = getThreadLocalCachedMemoryBlockStorage();
    if (!localCache.isFull()) {
        localCache.enqueue(block);
#if defined(STAT_MODE)
        (void)m_deallocateCacheHints.fetch_add(1);
#endif
    } else {
        ASSERTION(block->release(), std::runtime_error, "Invalid memory block")
#if defined(STAT_MODE)
        (void)m_deallocateCacheMisses.fetch_add(1);
#endif
    }
}

template<std::size_t Id, std::size_t BlockSize, std::size_t Capacity, std::size_t CacheSize>
typename MemoryPool<Id, BlockSize, Capacity, CacheSize>::MemoryBlock::Status
MemoryPool<Id, BlockSize, Capacity, CacheSize>::MemoryBlock::getStatus() const noexcept {
    return status.load();
}

template<std::size_t Id, std::size_t BlockSize, std::size_t Capacity, std::size_t CacheSize>
bool MemoryPool<Id, BlockSize, Capacity, CacheSize>::MemoryBlock::tryAcquire() noexcept {
    auto expect = Status::Released;
    return status.compare_exchange_strong(expect, Status::Acquired);
}

template<std::size_t Id, std::size_t BlockSize, std::size_t Capacity, std::size_t CacheSize>
bool MemoryPool<Id, BlockSize, Capacity, CacheSize>::MemoryBlock::release() noexcept {
    auto expect = Status::Acquired;
    return status.compare_exchange_strong(expect, Status::Released);
}

template<std::size_t Id, std::size_t BlockSize, std::size_t Capacity, std::size_t CacheSize>
constexpr bool MemoryPool<Id, BlockSize, Capacity, CacheSize>::checkBlock(const MemoryBlock* block) const noexcept {
#if !defined(NDEBUG)
    return (block->canaryStart == ID && block->canaryEnd == ID);
#else
    return true;
#endif
}

template<std::size_t Id, std::size_t BlockSize, std::size_t Capacity, std::size_t CacheSize>
typename MemoryPool<Id, BlockSize, Capacity, CacheSize>::CachedMemoryBlockStorage&
MemoryPool<Id, BlockSize, Capacity, CacheSize>::getThreadLocalCachedMemoryBlockStorage() noexcept {
    static thread_local CachedMemoryBlockStorage cache;
    return cache;
}

template<std::size_t Id, std::size_t BlockSize, std::size_t Capacity, std::size_t CacheSize>
void MemoryPool<Id, BlockSize, Capacity, CacheSize>::dumpStats(std::ostream& os) {
#if defined(STAT_MODE)
    const auto allocateCacheHints = m_allocateCacheHints.load();
    const auto allocateCacheMisses = m_allocateCacheMisses.load();
    const auto allocateCacheMissesInPercent = static_cast<double>(allocateCacheMisses) * 100 / (allocateCacheHints + allocateCacheMisses);
    const auto deallocateCacheHints = m_deallocateCacheHints.load();
    const auto deallocateCacheMisses = m_deallocateCacheMisses.load();
    const auto deallocateCacheMissesInPercent = static_cast<double>(deallocateCacheMisses) * 100 / (deallocateCacheHints + deallocateCacheMisses);

    os << "StaticMemoryPool<Id=" << Id << ", BlockSize=" << BlockSize << ", Capacity=" << Capacity << ", CacheSize=" << CacheSize << ">{\n";
    os << "\t" << "allocate cache hints=" << allocateCacheHints << ",";
    os << "  " << "allocate cache misses=" << allocateCacheMisses << ",";
    os << "  " << "allocate cache misses/hints=" << allocateCacheMissesInPercent << "%,\n";
    os << "\t" << "deallocate cache hints=" << deallocateCacheHints << ",";
    os << "  " << "deallocate cache misses=" << deallocateCacheMisses <<",";
    os << "  " << "deallocate cache misses/hints=" << deallocateCacheMissesInPercent << "%,\n";
    os << "}\n";
#endif
}

} //! namespace atom::memory::allocator::lock_free::fixed

#endif //! ATOM_LOCK_FREE_STATIC_MEMORY_POOL_H
