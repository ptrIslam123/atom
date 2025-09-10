#ifndef ATOM_DYNAMIC_CONTAINER_UNORDERED_SET_H
#define ATOM_DYNAMIC_CONTAINER_UNORDERED_SET_H

#include "include/containers/dynamic/list.h"
#include "include/utils/compiler_attr.h"
#include "include/utils/assertion.h"

#include <functional>
#include <stdexcept>
#include <utility>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cassert>

namespace atom::containers::dynamic {

template<
        typename K,
        std::size_t N,
        typename H = std::hash<K>,
        typename E = std::equal_to<K>,
        typename A = memory::allocator::DefaultAllocator
> class UnorderedSet final {
public:
    using KeyType = K;
    using HasherType = H;
    using KeyEqualType = E;
    using ReferenceType = KeyType&;
    using ConstReferenceType = const KeyType&;
    using PointerType = KeyType*;
    using ConstPointerType = const KeyType*;
    using SizeType = std::size_t;
    using CapacityType = std::size_t;
    using VersionTagType = std::uint64_t;

    class Iterator final {};
    class ConstIterator final {};

    ~UnorderedSet() { clear(); }

    std::pair<ConstIterator, bool> insert(const K& key) {
        const auto index = HasherType{}(key) % size();
        Bucket& bucket = m_buckets[index];
        if (bucket.isEmpty()) {
            auto it = m_elements.insertBack(key);
            bucket.pushBack(it);
            return std::make_pair(, true);
        } else {
            return std::make_pair(, false);
        }
    }

    ConstIterator find(const K& key) const noexcept {

    }

    ConstIterator erase(ConstIterator pos) {

    }

    void clear() {}

    Iterator endIter() {
        return Iterator{};
    }

    constexpr SizeType size() const noexcept {
        return m_size;
    }

    constexpr bool isEmpty() const noexcept {
        return size() == 0;
    }

private:
    struct Bucket {
        List<Iterator> data;
    };
    SizeType m_size{0};
    Bucket* m_buckets;
    List<KeyType> m_elements;
};

} //! namespace atom::containers::dynamic

#endif //! ATOM_DYNAMIC_CONTAINER_UNORDERED_SET_H
