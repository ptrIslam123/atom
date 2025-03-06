#ifndef ATOM_CONTAINERS_LOCK_FREE_QUEUE_H
#define ATOM_CONTAINERS_LOCK_FREE_QUEUE_H

#include "include/memory/allocators/lock_free/static_memory_pool.h"

#include <type_traits>
#include <atomic>

namespace atom::containers::lock_free {

template<typename T, std::size_t N>
class Queue final {
public:
    using ValueType = std::remove_cv_t<T>;
    using Reference = ValueType&;
    using ConstReference = const ValueType&;
    using AllocatorType = memory::allocator::lock_free::StaticMemoryPool<sizeof(ValueType), N>;

    ConstReference front() const;
    Reference front();

    ConstReference back() const;
    Reference back();

    template<typename ... Arg>
    void emplace(Arg&& ... arg);
    void enqueue(const T& value);
    void enqueue(T&& value);

    bool isEmpty() const noexcept { return size() == 0; }
    std::size_t size() const noexcept { return m_size.load(); }
    constexpr std::size_t capacity() const noexcept { return N; }

private:
    AllocatorType m_allocator;
    std::atomic<std::size_t> m_size{0};
};

} //! namespace atom::containers::lock_free

#endif //! ATOM_CONTAINERS_LOCK_FREE_QUEUE_H
