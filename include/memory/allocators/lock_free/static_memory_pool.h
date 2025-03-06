#ifndef ATOM_LOCK_FREE_STATIC_MEMORY_POOL_H
#define ATOM_LOCK_FREE_STATIC_MEMORY_POOL_H

#include "include/containers/static/ring_queue.h"

#include <type_traits>
#include <atomic>
#include <array>
#include <cstdint>
#include <cstddef>
#include <cstring>

namespace atom::memory::allocator::lock_free {

template<std::size_t BlockSize, std::size_t Capacity>
class StaticMemoryPool final {
private:
    struct ControlBlock {
        std::uint32_t strongRefCount{0};
        std::uint32_t weakRefCount{0};
    };

    struct MemoryBlock {
        alignas(alignof(std::max_align_t)) std::byte memory[BlockSize];
        std::atomic<ControlBlock> control;
    };

    std::array<MemoryBlock, Capacity> m_blocks{};

public:
    struct Descriptor {
        Descriptor(const Descriptor& other) noexcept;
        Descriptor(Descriptor&& other) noexcept;
        Descriptor& operator=(const Descriptor& other) noexcept;
        Descriptor& operator=(Descriptor&& other) noexcept;

        template<typename T>
        T& interpretAs() noexcept;

        template<typename T>
        const T& interpretAs() const noexcept;

        bool operator==(const Descriptor& other) const noexcept;
        bool operator==(std::nullptr_t /*ptr*/) const noexcept;

        explicit Descriptor(std::atomic<ControlBlock>* ptr);
        std::atomic<ControlBlock>* m_ptr{nullptr};
    };

    StaticMemoryPool(const StaticMemoryPool& ) = delete;
    StaticMemoryPool(StaticMemoryPool&& ) noexcept = delete;
    StaticMemoryPool& operator=(const StaticMemoryPool& ) = delete;
    StaticMemoryPool& operator=(StaticMemoryPool&& ) noexcept = delete;
    explicit StaticMemoryPool() = default;
    ~StaticMemoryPool() = default;

    template<typename T>
    class WeakPtr;

    template<typename T>
    class SharedPtr;

    template<typename T>
    class SharedPtr final {
    public:
        explicit SharedPtr(Descriptor&& descriptor, StaticMemoryPool& allocator): m_descriptor(descriptor) {}
        SharedPtr(const SharedPtr& other) {
            (void)this->operator=(other);
        }
        SharedPtr(SharedPtr&& other) {
            (void)this->operator=(std::move(other));
        }
        template<typename D, typename = std::enable_if_t<!std::is_same_v<T, D> && std::is_base_of_v<T, D>, D>>
        SharedPtr(const SharedPtr<D>& other);
        template<typename D, typename = std::enable_if_t<!std::is_same_v<T, D> && std::is_base_of_v<T, D>, D>>
        SharedPtr(SharedPtr<D>&& other);
        SharedPtr(std::nullptr_t ptr = nullptr) {
            (void)this->operator=(ptr);
        }
        SharedPtr& operator=(const SharedPtr& other) {
            clear();
            copy(other);
            return *this;
        }
        SharedPtr& operator=(SharedPtr&& other) {
            clear();
            swap(other);
            return *this;
        }
        template<typename D, typename = std::enable_if_t<!std::is_same_v<T, D> && std::is_base_of_v<T, D>, D>>
        SharedPtr& operator=(const SharedPtr<D>& other);
        template<typename D, typename = std::enable_if_t<!std::is_same_v<T, D> && std::is_base_of_v<T, D>, D>>
        SharedPtr& operator=(SharedPtr<D>&& other);
        SharedPtr& operator=(std::nullptr_t /*ptr*/) {
            clear();
            return *this;
        }
        ~SharedPtr() {
            clear();
        }

        void clear() {
            if (m_descriptor != nullptr) {
                for (;;) {
                    ControlBlock current = m_descriptor.m_ptr->load();
                    ControlBlock desired{current};
                    --desired.strongRefCount;
                    if (m_descriptor.m_ptr->compare_exchange_strong(current, desired)) {
                        if (desired.strongRefCount == 0 && desired.weakRefCount == 0) {

                        }
                        return;
                    }
                }

            }
            m_descriptor = nullptr;
        }

    private:
        friend StaticMemoryPool;

        void swap(SharedPtr& other) {

        }
        void copy(const SharedPtr& other) {

        }

        Descriptor m_descriptor;
        StaticMemoryPool& allocator;
    };

    template<typename T>
    class WeakPtr final {/*TODO*/};

    Descriptor allocate();
    void deallocate(Descriptor&& descriptor);
    constexpr std::size_t capacity() const noexcept;

private:
    using FreeDescriptosQueue = containers::StaticRingQueue<Descriptor, Capacity / 2>;
    static FreeDescriptosQueue& getFreeDescriptorsQueue();
};

template<std::size_t BlockSize, std::size_t Capacity>
constexpr std::size_t StaticMemoryPool<BlockSize, Capacity>::capacity() const noexcept {
    return Capacity;
}

template<std::size_t BlockSize, std::size_t Capacity>
typename StaticMemoryPool<BlockSize, Capacity>::Descriptor
StaticMemoryPool<BlockSize, Capacity>::allocate() {
    auto& freeDescriptors = getFreeDescriptorsQueue();
    if (!freeDescriptors.isEmpty()) {
        auto descriptor = freeDescriptors.back();
        freeDescriptors.dequeue();
        return descriptor;
    } else {
        for (MemoryBlock& block : m_blocks) {
            ControlBlock current = block.control.load();
            if (current.strongRefCount == 0 && current.weakRefCount == 0) {
                ControlBlock desired{current};
                ++desired.strongRefCount;
                if (block.control.compare_exchange_strong(current, desired)) {
                    return Descriptor{&block.control};
                }
            }
        }
    }
    return Descriptor{nullptr};
}

template<std::size_t BlockSize, std::size_t Capacity>
void StaticMemoryPool<BlockSize, Capacity>::deallocate(Descriptor&& descriptor) {
    auto& freeDescriptors = getFreeDescriptorsQueue();
    if (!freeDescriptors.isFull()) {
        freeDescriptors.enqueue(std::move(descriptor));
    } else {
        descriptor.m_ptr->store(ControlBlock{});
    }
}

template<std::size_t BlockSize, std::size_t Capacity>
typename StaticMemoryPool<BlockSize, Capacity>::FreeDescriptosQueue&
StaticMemoryPool<BlockSize, Capacity>::getFreeDescriptorsQueue() {
    thread_local FreeDescriptosQueue queue;
    return queue;
}

template<std::size_t BlockSize, std::size_t Capacity>
StaticMemoryPool<BlockSize, Capacity>::Descriptor::Descriptor(std::atomic<ControlBlock>* ptr):
m_ptr(ptr) {}

template<std::size_t BlockSize, std::size_t Capacity>
StaticMemoryPool<BlockSize, Capacity>::Descriptor::Descriptor(const Descriptor& other) noexcept {
    (void)this->operator=(other);
}

template<std::size_t BlockSize, std::size_t Capacity>
StaticMemoryPool<BlockSize, Capacity>::Descriptor::Descriptor(Descriptor&& other) noexcept {
    (void)this->operator=(other);
}

template<std::size_t BlockSize, std::size_t Capacity>
typename StaticMemoryPool<BlockSize, Capacity>::Descriptor&
StaticMemoryPool<BlockSize, Capacity>::Descriptor::operator=(const Descriptor& other) noexcept {
    std::memcpy(this, &other, sizeof(other));
    return *this;
}

template<std::size_t BlockSize, std::size_t Capacity>
template<typename T>
T& StaticMemoryPool<BlockSize, Capacity>::Descriptor::interpretAs() noexcept {
    return *reinterpret_cast<T*>(reinterpret_cast<std::byte*>(m_ptr) - BlockSize);
}

template<std::size_t BlockSize, std::size_t Capacity>
template<typename T>
const T& StaticMemoryPool<BlockSize, Capacity>::Descriptor::interpretAs() const noexcept {
    return *reinterpret_cast<const T*>(reinterpret_cast<std::byte*>(m_ptr) - BlockSize);
}

template<std::size_t BlockSize, std::size_t Capacity>
typename StaticMemoryPool<BlockSize, Capacity>::Descriptor&
StaticMemoryPool<BlockSize, Capacity>::Descriptor::operator=(Descriptor&& other) noexcept {
    return this->operator=(other);
}

template<std::size_t BlockSize, std::size_t Capacity>
bool StaticMemoryPool<BlockSize, Capacity>::Descriptor::operator==(const Descriptor& other) const noexcept {
    return m_ptr == other.m_ptr;
}

template<std::size_t BlockSize, std::size_t Capacity>
bool StaticMemoryPool<BlockSize, Capacity>::Descriptor::operator==(std::nullptr_t /*ptr*/) const noexcept {
    return m_ptr == nullptr;
}

} //! namespace atom::memory::allocator::lock_free

#endif //! ATOM_LOCK_FREE_STATIC_MEMORY_POOL_H
