#ifndef ATOM_STATIC_CONTAINER_UNORDERED_SET_H
#define ATOM_STATIC_CONTAINER_UNORDERED_SET_H

#include "include/utils/compiler_attr.h"
#include "include/utils/assertion.h"

#include <stdexcept>
#include <utility>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cassert>

namespace atom::containers::fixed {

// template<typename K, std::size_t N, typename H, typename E>
// class OpenAddressingSet final {
// public:
//     using KeyType = K;
//     using HasherType = H;
//     using KeyEqualType = E;
//     using ReferenceType = KeyType&;
//     using ConstReferenceType = const KeyType&;
//     using PointerType = KeyType*;
//     using ConstPointerType = const KeyType*;
//     using SizeType = std::size_t;
//     using CapacityType = std::size_t;
//     using VersionTagType = std::uint64_t;

//     class Iterator final {
//     public:
//         using iterator_category = std::forward_iterator_tag;
//         using value_type        = KeyType;
//         using difference_type   = std::ptrdiff_t;
//         using pointer           = PointerType;
//         using reference         = ReferenceType;

//         // Конструктор по умолчанию — создаёт end-итератор
//         Iterator() noexcept : m_set(nullptr), m_index(0) {}

//         // Преинкремент
//         Iterator& operator++() noexcept {
//             assert(m_set != nullptr);
//             if (m_index >= m_set->capacity()) {
//                 return *this; // уже end
//             }

//             do {
//                 ++m_index;
//             } while (m_index < m_set->capacity() &&
//                      m_set->m_statuses[m_index] != Status::Occupied);

//             if (m_index >= m_set->capacity()) {
//                 m_index = m_set->capacity(); // нормализуем в "end"
//             }

//             return *this;
//         }

//         // Постинкремент
//         Iterator operator++(int) noexcept {
//             Iterator tmp = *this;
//             ++(*this);
//             return tmp;
//         }

//         // Разыменование
//         reference operator*() const noexcept {
//             assert(m_set != nullptr);
//             assert(m_index < m_set->capacity());
//             assert(m_set->m_statuses[m_index] == Status::Occupied);
//             return *m_set->slot(m_index);
//         }

//         pointer operator->() const noexcept {
//             return &operator*();
//         }

//         // Сравнение
//         bool operator==(const Iterator& other) const noexcept {
//             return m_set == other.m_set && m_index == other.m_index;
//         }

//         bool operator!=(const Iterator& other) const noexcept {
//             return !(*this == other);
//         }

//     private:
//         // Дружественные классы могут создавать итераторы
//         friend class StaticOpenAddressingSet;

//         // Приватный конструктор: нужен для begin() и end()
//         Iterator(const StaticOpenAddressingSet* set, SizeType index) noexcept
//             : m_set(const_cast<StaticOpenAddressingSet*>(set)), m_index(index) {}

//         StaticOpenAddressingSet* m_set;
//         SizeType m_index;
//     };

//     // ConstIterator — почти как Iterator, но возвращает const ссылки
//     class ConstIterator final {
//     public:
//         using iterator_category = std::forward_iterator_tag;
//         using value_type        = KeyType;
//         using difference_type   = std::ptrdiff_t;
//         using pointer           = ConstPointerType;
//         using reference         = ConstReferenceType;

//         ConstIterator() noexcept : m_set(nullptr), m_index(0) {}

//         // Конструктор из non-const Iterator
//         ConstIterator(Iterator it) noexcept : m_set(it.m_set), m_index(it.m_index) {}

//         ConstIterator& operator++() noexcept {
//             assert(m_set != nullptr);
//             if (m_index >= m_set->capacity()) {
//                 return *this;
//             }

//             do {
//                 ++m_index;
//             } while (m_index < m_set->capacity() &&
//                      m_set->m_statuses[m_index] != Status::Occupied);

//             if (m_index >= m_set->capacity()) {
//                 m_index = m_set->capacity();
//             }

//             return *this;
//         }

//         ConstIterator operator++(int) noexcept {
//             ConstIterator tmp = *this;
//             ++(*this);
//             return tmp;
//         }

//         reference operator*() const noexcept {
//             assert(m_set != nullptr);
//             assert(m_index < m_set->capacity());
//             assert(m_set->m_statuses[m_index] == Status::Occupied);
//             return *m_set->slot(m_index);
//         }

//         pointer operator->() const noexcept {
//             return &operator*();
//         }

//         bool operator==(const ConstIterator& other) const noexcept {
//             return m_set == other.m_set && m_index == other.m_index;
//         }

//         bool operator!=(const ConstIterator& other) const noexcept {
//             return !(*this == other);
//         }

//     private:
//         friend class StaticOpenAddressingSet;

//         ConstIterator(const StaticOpenAddressingSet* set, SizeType index) noexcept
//             : m_set(set), m_index(index) {}

//         const StaticOpenAddressingSet* m_set;
//         SizeType m_index;
//     };

//     class ConstIterator final {
//     public:

//     private:

//     };

//     ~OpenAddressingSet() { clear(); }

//     std::pair<Iterator, bool> insert(const K& key) {
//         ASSERTION(!isFull(), std::runtime_error, "Set is full")
//         const auto index = HasherType{}(key) % capacity();
//         auto i = index;
//         do {
//             assert(i < capacity());
//             Status& status = m_statuses[i];
//             if (status == Status::Occupied) {
//                 if (KeyEqualType{}(*slot(i), key)) {
//                     return std::make_pair(indexToIter(i), false);
//                 } else {} // hash collision
//             } else if (status == Status::Free || status == Status::Deleted) { // hash hint
//                 constructSlot(slot(i), key);
//                 status = Status::Occupied;
//                 ++m_size;
//                 return std::make_pair(indexToIter(i), true);
//             }
//             i = (i + 1) % capacity();
//         } while (i != index);

//         assert(false && "This case must be unreachable");
//         ASSERTION(false, std::runtime_error, "Set is Full")
//     }

//     Iterator find(const K& key) const noexcept {
//         if (isEmpty()) {
//             return endIter();
//         }

//         const auto index = HasherType{}(key) % capacity();
//         auto i = index;
//         do {
//             assert(i < capacity());
//             const Status& status = m_statuses[i];
//             if (status == Status::Free) {
//                 return endIter();
//             } else if (status == Status::Occupied) {
//                 if (KeyEqualType{}(*slot(i), key)) {
//                     return indexToIter(i);
//                 }
//             } else if (status == Status::Deleted) {}
//             i = (i + 1) % capacity();
//         } while (i != index);
//         return endIter();
//     }

//     Iterator erase(ConstIterator pos) {
//         const auto index = iterToIndex(pos);
//         assert(index < capacity());
//         destructSlot(slot(index));
//         m_statuses[index] = Status::Deleted;
//         --m_size;

//         for (auto i = index + 1; i < capacity(); ++i) {
//             if (m_statuses[i] == Status::Occupied) {
//                 return indexToIter(i);
//             }
//         }
//         return endIter();
//     }

//     void clear() {
//         destructSlots(0, size(), Status::Free);
//         m_size = 0;
//     }

//     Iterator endIter() {
//         return Iterator{};
//     }

//     constexpr SizeType capacity() const noexcept {
//         return N;
//     }

//     constexpr SizeType size() const noexcept {
//         return m_size;
//     }

//     constexpr bool isEmpty() const noexcept {
//         return size() == 0;
//     }

//     constexpr bool isFull() const noexcept {
//         return size() >= capacity();
//     }

// private:
//     enum class Status : std::uint8_t {
//         Free = 0,
//         Occupied,
//         Deleted,
//     };

//     Iterator indexToIter(SizeType index) noexcept {
//         return Iterator{}; //! TODO
//     }

//     SizeType iterToIndex(ConstIterator it) noexcept {
//         return SizeType{}; //! TODO
//     }

//     PointerType slot(SizeType index) noexcept {
//         return reinterpret_cast<PointerType>(m_slots.data() + index * sizeof(K));
//     }

//     ConstPointerType slot(SizeType index) const noexcept {
//         return reinterpret_cast<ConstPointerType>(m_slots.data() + index * sizeof(K));
//     }

//     template<typename ... Arg>
//     void constructSlot(void* slot, Arg&& ... arg) {
//         assert(slot);
//         new(slot) KeyType{std::forward<Arg>(arg) ... };
//     }

//     void destructSlot(void* slot) {
//         assert(slot);
//         reinterpret_cast<KeyType*>(slot)->~KeyType();
//     }

//     void destructSlots(SizeType begin, SizeType end, Status status) {
//         assert(begin <= end);
//         for (auto i = begin; i != end; ++i) {
//             destructSlot(m_slots.data() + i * sizeof(KeyType));
//             m_statuses[i] = status;
//         }
//     }

//     SizeType m_size{0};
//     std::array<Status, N> m_statuses{Status::Free};
//     alignas(alignof(K)) std::array<std::byte, sizeof(K) * N> m_slots{};
// };

} //! namespace atom::containers::fixed

#endif //! ATOM_STATIC_CONTAINER_UNORDERED_SET_H
