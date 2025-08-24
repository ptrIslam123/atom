#ifndef ATOM_DEFAULT_ALLOCATOR_H
#define ATOM_DEFAULT_ALLOCATOR_H

#include "include/utils/assertion.h"

#include <stdexcept>
#include <new>
#include <utility>
#include <cstddef>
#include <cstdlib>
#include <cassert>

namespace atom::memory::allocator {

/**
 * @brief Default memory allocator for managing dynamic memory.
 *
 * This struct provides methods for allocating, reallocating, and deallocating memory,
 * as well as constructing and destructing objects in the allocated memory.
 */
struct DefaultAllocator {
    /**
     * @brief Allocates a block of memory of the specified size.
     *
     * @param size The number of bytes to allocate.
     * @return std::byte* A pointer to the allocated memory block.
     *                    Returns nullptr if allocation fails.
     * @throws std::bad_alloc If the operation fails.
     */
    std::byte* allocate(std::size_t size);

    /**
     * @brief Deallocates a previously allocated block of memory.
     *
     * @param start A pointer to the memory block to deallocate.
     *              If the pointer is nullptr, this function does nothing.
     *
     * @throws std::bad_alloc If the operation fails.
     *
     * @warning The behavior is undefined if the pointer was not allocated by this allocator.
     */
    void deallocate(std::byte* start);

    /**
     * @brief Constructs an object of type T in the allocated memory.
     *
     * @tparam T The type of the object to construct.
     * @tparam Arg The types of the arguments to forward to the constructor of T.
     *
     * @param object A pointer to the memory location where the object should be constructed.
     * @param arg The arguments to forward to the constructor of T.
     *
     * @throws std::bad_alloc If the operation fails.
     *
     * @details This method uses placement new to construct the object in the provided memory.
     */
    template<typename T, typename ... Arg>
    void construct(T* object, Arg&& ... arg);

    /**
     * @brief Destructs an object of type T.
     *
     * @tparam T The type of the object to destruct.
     *
     * @param object A pointer to the object to destruct.
     *
     * @details This method explicitly calls the destructor of the object.
     *          It does not deallocate the memory occupied by the object.
     */
    template<typename T>
    void destruct(T* object);
};

inline std::byte* DefaultAllocator::allocate(std::size_t size) {
    auto ptr = reinterpret_cast<std::byte*>(malloc(size));
    ASSERTION(ptr, std::runtime_error, "Could not allocate memory with size=" + std::to_string(size))
    return ptr;
}

inline void DefaultAllocator::deallocate(std::byte* start) {
    free(static_cast<void*>(start));
}

template<typename T, typename ... Arg>
inline void DefaultAllocator::construct(T* object, Arg&& ... arg) {
    ASSERTION(object, std::runtime_error, "Object pointer is null")
    new(object) T(std::forward<Arg>(arg) ...);
}

template<typename T>
inline void DefaultAllocator::destruct(T* object) {
    if (object) {
        object->~T();
    }
}

} //! namespace atom::memory::allocator

#endif //! ATOM_DEFAULT_ALLOCATOR_H
