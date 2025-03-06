#ifndef ATOM_DEFAULT_ALLOCATOR_H
#define ATOM_DEFAULT_ALLOCATOR_H

#include <stdexcept>
#include <new>
#include <utility>
#include <cstddef>
#include <cstdlib>
#include <cassert>

namespace atom::memory::allocator {

struct DefaultAllocator final {
    std::byte* allocate(std::size_t size);
    void reallocate(std::byte*& start, std::size_t size);
    void deallocate(std::byte* start);

    template<typename T, typename ... Arg>
    void construct(T* object, Arg&& ... arg);

    template<typename T>
    void destruct(T* object);
};

inline std::byte* DefaultAllocator::allocate(std::size_t size) {
    auto ptr = reinterpret_cast<std::byte*>(malloc(size));
    if (ptr) {
        return ptr;
    } else {
        throw std::bad_alloc{};
    }
}

inline void DefaultAllocator::reallocate(std::byte*& start, std::size_t size) {
    start = static_cast<std::byte*>(realloc(static_cast<std::byte*>(start), size));
    if (!start) {
        throw std::bad_alloc{};
    }
}

inline void DefaultAllocator::deallocate(std::byte* start) {
    free(static_cast<void*>(start));
}

template<typename T, typename ... Arg>
inline void DefaultAllocator::construct(T* object, Arg&& ... arg) {
    if (object) {
        new(object) T(std::forward<Arg>(arg) ...);
    } else {
        throw std::invalid_argument("Object pointer is null");
    }

}

template<typename T>
inline void DefaultAllocator::destruct(T* object) {
    if (object) {
        object->~T();
    }
}

} //! namespace atom::memory::allocator

#endif //! ATOM_DEFAULT_ALLOCATOR_H
