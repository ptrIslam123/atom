#ifndef ATOM_STATIC_ARRAY_H
#define ATOM_STATIC_ARRAY_H

#include "include/utils/assertion.h"

#include <algorithm>
#include <type_traits>
#include <exception>
#include <span>
#include <initializer_list>

#include <cstring>
#include <cstddef>
#include <cstdint>
#include <cassert>

namespace atom::containers {

template<typename T, std::size_t N>
class StaticArray final {
public:
    using ElementType = T;
    using RefereceType = ElementType&;
    using ConstReferenceType = const ElementType&;
    using PointerType = ElementType*;
    using ConstPointerType = const ElementType*;
    using SizeType = std::size_t;
    using CapacityType = std::size_t;

    class Iterator {
    public:
        using difference_type = std::ptrdiff_t;
        ConstReferenceType operator*() const {
            ASSERTION(m_ptr != nullptr, std::runtime_error, "Dereferencing null iterator");
            return *m_ptr;
        }
        RefereceType operator*() {
            ASSERTION(m_ptr != nullptr, std::runtime_error, "Dereferencing null iterator");
            return *m_ptr;
        }
        ConstPointerType operator->() const { return m_ptr; }
        PointerType operator->() { return m_ptr; }

        Iterator& operator++() { ++m_ptr; return *this; }
        Iterator operator++(int) { Iterator tmp = *this; ++m_ptr; return tmp; }
        Iterator& operator--() { --m_ptr; return *this; }
        Iterator operator--(int) { Iterator tmp = *this; --m_ptr; return tmp; }

        Iterator operator+(difference_type n) const { return Iterator(m_ptr + n); }
        Iterator operator-(difference_type n) const { return Iterator(m_ptr - n); }
        difference_type operator-(const Iterator& other) const { return m_ptr - other.m_ptr; }

        bool operator==(const Iterator& other) const { return m_ptr == other.m_ptr; }
        bool operator!=(const Iterator& other) const { return m_ptr != other.m_ptr; }
        bool operator<(const Iterator& other) const { return m_ptr < other.m_ptr; }
        bool operator>(const Iterator& other) const { return m_ptr > other.m_ptr; }
        bool operator<=(const Iterator& other) const { return m_ptr <= other.m_ptr; }
        bool operator>=(const Iterator& other) const { return m_ptr >= other.m_ptr; }

    private:
        friend StaticArray;

        explicit Iterator(PointerType ptr): m_ptr(ptr) {}

        PointerType m_ptr;
    };
    class ConstIterator {
    public:
        using difference_type = std::ptrdiff_t;
        ConstReferenceType operator*() const {
            ASSERTION(m_ptr != nullptr, std::runtime_error, "Dereferencing null iterator");
            return *m_ptr;
        }
        ConstPointerType operator->() const { return m_ptr; }
        ConstIterator& operator++() { ++m_ptr; return *this; }
        ConstIterator operator++(int) { ConstIterator tmp = *this; ++m_ptr; return tmp; }
        ConstIterator& operator--() { --m_ptr; return *this; }
        ConstIterator operator--(int) { ConstIterator tmp = *this; --m_ptr; return tmp; }

        ConstIterator operator+(difference_type n) const { return ConstIterator(m_ptr + n); }
        ConstIterator operator-(difference_type n) const { return ConstIterator(m_ptr - n); }
        difference_type operator-(const Iterator& other) const { return m_ptr - other.m_ptr; }

        bool operator==(const ConstIterator& other) const { return m_ptr == other.m_ptr; }
        bool operator!=(const ConstIterator& other) const { return m_ptr != other.m_ptr; }
        bool operator<(const ConstIterator& other) const { return m_ptr < other.m_ptr; }
        bool operator>(const ConstIterator& other) const { return m_ptr > other.m_ptr; }
        bool operator<=(const ConstIterator& other) const { return m_ptr <= other.m_ptr; }
        bool operator>=(const ConstIterator& other) const { return m_ptr >= other.m_ptr; }

    private:
        friend StaticArray;

        explicit ConstIterator(PointerType ptr): m_ptr(ptr) {}

        PointerType m_ptr;
    };

    explicit StaticArray() = default;

    void pushBack(std::span<const T> data) {
        ASSERTION(size() + data.size() <= capacity(), std::runtime_error, "used up all the memory")
        for (auto i = 0, j = size(); i < data.size(); ++i, ++j) {
            (void)insertElement(j, data[i]);
        }
    }

    void pushBack(const T& data) {
        ASSERTION(size() + 1 <= capacity(), std::runtime_error, "used up all the memory")
        (void)insertElement(size(), data);
    }

    void pushBack(T&& data) {
        ASSERTION(size() + 1 <= capacity(), std::runtime_error, "used up all the memory")
        (void)insertElement(size(), std::move(data));
    }

    template<typename ... Arg>
    void emplaceBack(Arg&& ... arg) {
        ASSERTION(size() + 1 <= capacity(), std::runtime_error, "used up all the memory")
        (void)emplaceElement(size(), std::forward<Arg>(arg) ... );
    }

    template<typename ... Arg>
    void emplace(ConstIterator pos, Arg&& ... arg) {
        ASSERTION(size() + 1 <= capacity(), std::runtime_error, "used up all the memory")
        (void)emplaceElement(iterToIndex(pos), std::forward<Arg>(arg) ... );
    }

    Iterator insert(ConstIterator pos, const T& data) {
        ASSERTION(size() + 1 <= capacity(), std::runtime_error, "used up all the memory")
        return insertElement(iterToIndex(pos), data);
    }
    Iterator insert(ConstIterator pos, T&& data) {
        ASSERTION(size() + 1 <= capacity(), std::runtime_error, "used up all the memory")
        return insertElement(iterToIndex(pos), std::move(data));
    }
    Iterator insert(ConstIterator pos, SizeType count, const T& data) {
        ASSERTION(size() + count <= capacity(), std::runtime_error, "used up all the memory")
        auto ptr = iterToIndex(pos);
        for (auto i = 0; i < count; ++i) {
            (void)insertElement(ptr + i, data);
        }
        return endIter(); //! TODO
    }
    Iterator insert(ConstIterator pos, std::initializer_list<T> data) {
        ASSERTION(size() + data.size() <= capacity(), std::runtime_error, "used up all the memory")
        auto ptr = iterToIndex(pos);
        for (auto i = 0; i < data.size(); ++i) {
            (void)insertElement(ptr + i, std::move(*(data.begin() + i)));
        }
        return endIter(); //! TODO
    }
    Iterator insert(ConstIterator pos, std::span<const T> data) {
        ASSERTION(size() + data.size() <= capacity(), std::runtime_error, "used up all the memory")
        auto ptr = iterToIndex(pos);
        for (auto i = 0; i < data.size(); ++i) {
            (void)insertElement(ptr + i, data[i]);
        }
        return endIter(); //! TODO
    }

    void popBack() {
        ASSERTION(!isEmpty(), std::runtime_error, "Out of memory")
        destruct(indexToPtr(size() - 1));
    }
    Iterator erase(ConstIterator pos) {
        return erase(pos, pos + 1);
    }
    Iterator erase(ConstIterator first, ConstIterator last) {
        if (first < last) {
            const auto firstIndex = iterToIndex(first);
            const auto lastIndex = iterToIndex(last);
            const auto numErased = lastIndex - firstIndex;

            destructElements(firstIndex, lastIndex);
            shiftElements(lastIndex, size(), -numErased);
            return indexToIter(firstIndex);
        } else if (first == last) {
            return Iterator{last.m_ptr};
        } else {
            return endIter();
        }
    }
    void clear() noexcept(std::is_nothrow_destructible_v<T>) {
        if (!isEmpty()) {
            destructElements(0, size());
        }
    }

    ConstReferenceType atUnsafe(SizeType index) const noexcept { return *(data() + index); }
    RefereceType atUnsafe(SizeType index) noexcept { return *(data() + index); }
    ConstReferenceType operator[](SizeType index) const {
        ASSERTION(index < size(), std::runtime_error, "Out of range")
        return atUnsafe(index);
    }
    RefereceType operator[](SizeType index) {
        ASSERTION(index < size(), std::runtime_error, "Out of range")
        return atUnsafe(index);
    }

    ConstReferenceType firstElement() const {
        ASSERTION(!isEmpty(), std::runtime_error, "static array is empty")
        return *firstConstIter();
    }
    RefereceType firstElement() {
        ASSERTION(!isEmpty(), std::runtime_error, "static array is empty")
        return *firstIter();
    }
    ConstReferenceType lastElement() const {
        ASSERTION(!isEmpty(), std::runtime_error, "static array is empty")
        return *lastConstIter();
    }
    RefereceType lastElement() {
        ASSERTION(!isEmpty(), std::runtime_error, "static array is empty")
        return *lastIter();
    }

    Iterator firstIter() { return Iterator{data()}; }
    Iterator lastIter() {
        if (size() > 1) [[likely]] {
            return Iterator{data() + size() - 1};
        } else {
            return firstIter();
        }
    }
    Iterator endIter() const { return Iterator{ const_cast<PointerType>(data()) + size()}; }
    ConstIterator firstConstIter() const { return ConstIterator{ const_cast<PointerType>(data())}; }
    ConstIterator lastConstIter() const {
        if (size() > 1) [[likely]] {
            return ConstIterator{const_cast<PointerType>(data()) + size() - 1};
        } else {
            return firstConstIter();
        }
    }
    ConstIterator endConstIter() const { return ConstIterator{ const_cast<PointerType>(data()) + size()}; }

    constexpr SizeType size() const noexcept {
        return m_size;
    }

    constexpr CapacityType capacity() const noexcept {
        return N;
    }

    constexpr bool isEmpty() const noexcept {
        return (size() == 0);
    }

    ConstPointerType data() const noexcept { return reinterpret_cast<ConstPointerType>(m_data); }
    PointerType data() noexcept { return reinterpret_cast<PointerType>(m_data); }

private:
    template<typename ... Arg>
    void construct(PointerType ptr, Arg&& ... arg) noexcept(std::is_nothrow_constructible_v<T>) {
        new(ptr) T{std::forward<Arg>(arg) ... };
    }

    void destruct(PointerType ptr) noexcept(std::is_nothrow_destructible_v<T>) {
        ptr->~T();
    }

    void destructElements(SizeType firstIndex, SizeType lastIndex) {
        assert(firstIndex <= lastIndex && firstIndex < size() && lastIndex <= size());
        const auto ptr = data();
        for (auto i = firstIndex; i < lastIndex; ++i) {
            destruct(ptr + i);
        }
        m_size -= lastIndex - firstIndex;
    }

    SizeType iterToIndex(ConstIterator it) {
        return static_cast<SizeType>(it.m_ptr - data());
    }

    PointerType indexToPtr(SizeType index) {
        return (data() + index);
    }

    Iterator indexToIter(SizeType index) {
        return (firstIter() + index);
    }

    template<typename R>
    Iterator insertElement(SizeType index, R&& data) {
        assert(index < capacity());
        shiftElements(index, size(), 1);
        construct(indexToPtr(index), std::forward<R>(data));
        ++m_size;
        return Iterator{indexToPtr(index)};
    }

    template<typename ... Arg>
    Iterator emplaceElement(SizeType index, Arg&& ... arg) {
        assert(index < capacity());
        shiftElements(index, size(), 1);
        construct(indexToPtr(index), std::forward<Arg>(arg) ... );
        ++m_size;
        return lastIter();
    }

    void shiftElements(SizeType firstIndex, SizeType count, int offset) {
        if (count == 0 || offset == 0) [[unlikely]] {
            return;
        }

        auto start = data();
        if constexpr (std::is_trivially_copyable_v<T>) {
            PointerType src = start + firstIndex;
            PointerType dst = src + offset;
            std::memmove(dst, src, count * sizeof(T));
        } else if constexpr (std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T>){
            assert(false);
        } else {
            if (offset > 0) {
                for (auto i = 0; i < count; ++i) {
                    auto src = start + firstIndex + i;
                    auto dst = src + offset;
                    construct(start + firstIndex + i + offset, );
                }
            } else {

            }
        }
    }

    SizeType m_size{0};
    std::byte m_data[N * sizeof(T)]{};
};

} //! namespace atom::containers

#endif //! ATOM_STATIC_ARRAY_H
