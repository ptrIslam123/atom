#ifndef ATOM_ALLOCATOR_TRAITS_H
#define ATOM_ALLOCATOR_TRAITS_H

#include "include/memory/memory.h"

#include <type_traits>

namespace atom::memory::allocator {

template<typename Alloc, typename T>
class AllocatorTraits : private Alloc {
public:
    using AllocatorType = Alloc;
    using ElementType = std::remove_cvref_t<T>;
    using PointerType = ElementType*;
    using ConstPointerType = const ElementType*;

    template<typename ... Arg>
    PointerType allocate(Arg&& ... arg) {}

    void deallocate(PointerType ptr) {}
};

} //! namespace atom::memory::allocator

#endif //! ATOM_ALLOCATOR_TRAITS_H
