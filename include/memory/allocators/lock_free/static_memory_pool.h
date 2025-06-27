#ifndef ATOM_LOCK_FREE_STATIC_MEMORY_POOL_H
#define ATOM_LOCK_FREE_STATIC_MEMORY_POOL_H

#include "include/containers/static/ring_queue.h"
#include "include/utils/assertion.h"

#include <type_traits>
#include <atomic>
#include <algorithm>
#include <optional>
#include <array>
#include <stdexcept>
#include <new>

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cassert>

namespace atom::memory::allocator::lock_free {

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
class StaticMemoryPool final {
public:
    static_assert(BlockSize > 0 && BlockSize % alignof(std::max_align_t) == 0,
                  "BlockSize must be multiple of platform alignment");
    static_assert(Capacity > 0 && Capacity % alignof(std::max_align_t) == 0,
                  "Capacity must be multiple of platform alignment");
    static_assert(CacheSize > 0 && CacheSize <= static_cast<std::size_t>(Capacity / 2),
                     "CacheSize cannot be greater than Capacity");

    /// Unique pool identifier (debugging aid)
    static constexpr std::size_t ID = Id;
    /// Size of individual memory blocks
    static constexpr std::size_t BLOCK_SIZE = BlockSize;
    /// Total capacity of the pool (number of blocks)
    static constexpr std::size_t CAPACITY = Capacity;
    /// Per-thread cache size (number of blocks)
    static constexpr std::size_t CACHE_SIZE = CacheSize;

    explicit StaticMemoryPool() = default;
    StaticMemoryPool(const StaticMemoryPool& ) = delete;
    StaticMemoryPool(StaticMemoryPool&& ) = delete;
    StaticMemoryPool& operator=(const StaticMemoryPool& ) = delete;
    StaticMemoryPool& operator=(StaticMemoryPool&& ) = delete;

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
        std::array<std::byte, BLOCK_SIZE> rawStorage;
        alignas(std::max_align_t) std::atomic<Status> status{Status::Released};
#if !defined(NDEBUG)
        std::size_t canaryEnd{ID};
#endif
    };

    using CachedMemoryBlockStorage = containers::StaticRingQueue<MemoryBlock*, CACHE_SIZE>;

    constexpr bool checkBlock(const MemoryBlock* block) const noexcept;
    CachedMemoryBlockStorage& getThreadLocalCachedMemoryBlockStorage() noexcept;

    alignas(std::max_align_t) std::array<MemoryBlock, CAPACITY> m_pool;
};

template<std::size_t Id, std::size_t BlockSize, std::size_t Capacity, std::size_t CacheSize>
std::byte* StaticMemoryPool<Id, BlockSize, Capacity, CacheSize>::allocate() {
    auto& localCache = getThreadLocalCachedMemoryBlockStorage();
    if (!localCache.isEmpty()) {
        auto memBlock = localCache.back();
        localCache.dequeue();
        return memBlock->rawStorage.data();
    }

    for (auto& memBlock : m_pool) {
        if (memBlock.tryAcquire()) {
            return memBlock.rawStorage.data();
        }
    }
    ASSERTION(false, std::runtime_error, "Could not allocate a new mempry block")
}

template<std::size_t Id, std::size_t BlockSize, std::size_t Capacity, std::size_t CacheSize>
void StaticMemoryPool<Id, BlockSize, Capacity, CacheSize>::deallocate(std::byte* ptr) {
    if (!ptr) [[unlikely]] {
        return;
    }

    MemoryBlock* block = reinterpret_cast<MemoryBlock*>(ptr);
    ASSERTION(block, std::runtime_error, "Broken memory pointer")
#if !defined(NDEBUG)
    ASSERTION(checkBlock(block), std::runtime_error, "Broken memory block")
#endif
    auto& localCache = getThreadLocalCachedMemoryBlockStorage();
    if (!localCache.isFull()) {
        localCache.enqueue(block);
    } else {
        ASSERTION(block->release(), std::runtime_error, "Invalid memory block")
    }
}

template<std::size_t Id, std::size_t BlockSize, std::size_t Capacity, std::size_t CacheSize>
typename StaticMemoryPool<Id, BlockSize, Capacity, CacheSize>::MemoryBlock::Status
StaticMemoryPool<Id, BlockSize, Capacity, CacheSize>::MemoryBlock::getStatus() const noexcept {
    return status.load();
}

template<std::size_t Id, std::size_t BlockSize, std::size_t Capacity, std::size_t CacheSize>
bool StaticMemoryPool<Id, BlockSize, Capacity, CacheSize>::MemoryBlock::tryAcquire() noexcept {
    auto expect = Status::Released;
    return status.compare_exchange_strong(expect, Status::Acquired);
}

template<std::size_t Id, std::size_t BlockSize, std::size_t Capacity, std::size_t CacheSize>
bool StaticMemoryPool<Id, BlockSize, Capacity, CacheSize>::MemoryBlock::release() noexcept {
    auto expect = Status::Acquired;
    return status.compare_exchange_strong(expect, Status::Released);
}

template<std::size_t Id, std::size_t BlockSize, std::size_t Capacity, std::size_t CacheSize>
constexpr bool StaticMemoryPool<Id, BlockSize, Capacity, CacheSize>::checkBlock(const MemoryBlock* block) const noexcept {
#if !defined(NDEBUG)
    return (block->canaryStart == ID && block->canaryEnd == ID);
#else
    return true;
#endif
}

template<std::size_t Id, std::size_t BlockSize, std::size_t Capacity, std::size_t CacheSize>
typename StaticMemoryPool<Id, BlockSize, Capacity, CacheSize>::CachedMemoryBlockStorage&
StaticMemoryPool<Id, BlockSize, Capacity, CacheSize>::getThreadLocalCachedMemoryBlockStorage() noexcept {
    static thread_local CachedMemoryBlockStorage cache;
    return cache;
}

} //! namespace atom::memory::allocator::lock_free

#endif //! ATOM_LOCK_FREE_STATIC_MEMORY_POOL_H
