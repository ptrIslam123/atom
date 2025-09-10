#ifndef ATOM_CONTAINERS_DYNAMIC_LIST_H
#define ATOM_CONTAINERS_DYNAMIC_LIST_H

#include "include/containers/dynamic/circular_list.h"
#include "include/memory/allocators/default_allocator.h"

namespace atom::containers::dynamic {

template<typename T, typename A = memory::allocator::DefaultAllocator>
using List = CircularList<T, A>;

} //! namespace atom::containers::dynamic

#endif //! ATOM_CONTAINERS_DYNAMIC_LIST_H
