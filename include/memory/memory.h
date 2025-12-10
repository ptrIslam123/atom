#ifndef ATOM_MEMORY_H
#define ATOM_MEMORY_H

#include "include/utils/compiler_attr.h"

#include <utility>
#include <cassert>

namespace atom::memory {

template<typename T, typename ... Arg>
constexpr FORCE_INLINE void Construct(T* ptr, Arg&&... arg) IS_NOEXCEPT_CONSTRUCIBLE(T);

template<typename T>
constexpr FORCE_INLINE void Destruct(T* ptr) IS_NOEXCEPT_DESTRUCTIBLE(T);


template<typename Iter>
void Copy();

//////////////////// Impl ////////////////////

template<typename T, typename ... Arg>
constexpr FORCE_INLINE void Construct(T* ptr, Arg&&... arg) IS_NOEXCEPT_CONSTRUCIBLE(T) {
    assert(ptr);
    new(ptr) T{std::forward<Arg>(arg) ... };
}

template<typename T>
constexpr FORCE_INLINE void Destruct(T* ptr) IS_NOEXCEPT_DESTRUCTIBLE(T) {
    assert(ptr);
    ptr->~T();
}

} //! namespace atom::memory

#endif //! ATOM_MEMORY_H
