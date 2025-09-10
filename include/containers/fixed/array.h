#ifndef ATOM_STATIC_ARRAY_H
#define ATOM_STATIC_ARRAY_H

#include "include/iterator/iterator_traits.h"
#include "include/iterator/iterator.h"
#include "include/utils/compiler_attr.h"
#include "include/utils/assertion.h"
#include "include/memory/memory.h"

#include <algorithm>
#include <type_traits>
#include <exception>
#include <initializer_list>

#include <cstring>
#include <cstddef>
#include <cstdint>
#include <cassert>

namespace atom::containers::fixed {

class ArrayException final : public std::exception {
public:
    explicit ArrayException(std::string_view msg): m_msg(msg) {}
    virtual const char* what() const noexcept { return m_msg.data(); }
private:
    std::string m_msg;
};

template<typename T, std::size_t N>
class Array final {
public:
    using ElementType = T;
    using ReferenceType = ElementType&;
    using ConstReferenceType = const ElementType&;
    using PointerType = ElementType*;
    using ConstPointerType = const ElementType*;
    using SizeType = std::size_t;
    using VersionTagType = std::uint64_t;

    class Iterator final {
    public:
        Iterator(const Iterator& other) noexcept { operator=(other); }
        Iterator(Iterator&& other) noexcept { operator=(std::move(other)); }
        Iterator& operator=(const Iterator& other) noexcept { copy(other); return *this; }
        Iterator& operator=(Iterator&& other) noexcept { copy(other); return *this; }

        ConstReferenceType operator*() const {
            ASSERTION(!isExpired() && !isOutOfRange(), std::runtime_error, "Dereferencing invalid iterator")
            return *m_ptr;
        }
        ReferenceType operator*() {
            ASSERTION(!isExpired() && !isOutOfRange(), std::runtime_error, "Dereferencing invalid iterator")
            return *m_ptr;
        }
        ConstPointerType operator->() const {
            ASSERTION(!isExpired() && !isOutOfRange(), std::runtime_error, "Dereferencing invalid iterator")
            return m_ptr;
        }
        PointerType operator->() {
            ASSERTION(!isExpired() && !isOutOfRange(), std::runtime_error, "Dereferencing invalid iterator")
            return m_ptr;
        }

        FORCE_INLINE Iterator& operator++() noexcept { ++m_ptr; return *this; }
        FORCE_INLINE Iterator operator++(int) noexcept { Iterator tmp = *this; ++m_ptr; return tmp; }
        FORCE_INLINE Iterator& operator--() noexcept { --m_ptr; return *this; }
        FORCE_INLINE Iterator operator--(int) noexcept { Iterator tmp = *this; --m_ptr; return tmp; }

        FORCE_INLINE Iterator operator+(std::ptrdiff_t n) const noexcept { return Iterator{m_ptr + n, m_array, m_version}; }
        FORCE_INLINE Iterator operator-(std::ptrdiff_t n) const noexcept { return Iterator{m_ptr - n, m_array, m_version}; }
        FORCE_INLINE Iterator& operator+=(std::ptrdiff_t n) noexcept { m_ptr += n; return *this; }
        FORCE_INLINE Iterator& operator-=(std::ptrdiff_t n) noexcept { m_ptr -= n; return *this; }
        FORCE_INLINE std::ptrdiff_t operator-(const Iterator& other) const noexcept { return m_ptr - other.m_ptr; }
        FORCE_INLINE ConstReferenceType operator[](std::ptrdiff_t n) const { return (operator+(n)).operator*(); }
        FORCE_INLINE ReferenceType operator[](std::ptrdiff_t n) { return (operator+(n)).operator*(); }
        FORCE_INLINE bool operator==(const Iterator& other) const noexcept { return m_ptr == other.m_ptr; }
        FORCE_INLINE bool operator!=(const Iterator& other) const noexcept { return m_ptr != other.m_ptr; }
        FORCE_INLINE bool operator<(const Iterator& other) const noexcept { return m_ptr < other.m_ptr; }
        FORCE_INLINE bool operator>(const Iterator& other) const noexcept { return m_ptr > other.m_ptr; }
        FORCE_INLINE bool operator<=(const Iterator& other) const noexcept { return m_ptr <= other.m_ptr; }
        FORCE_INLINE bool operator>=(const Iterator& other) const noexcept { return m_ptr >= other.m_ptr; }

        FORCE_INLINE bool isExpired() const noexcept { return m_version != m_array->m_version; }
        FORCE_INLINE bool isOutOfRange() const noexcept { return m_ptr >= m_array->endIter().m_ptr; }

    private:
        friend Array;

        Iterator(PointerType ptr, const Array* array, VersionTagType version):
        m_ptr(ptr),
        m_array(const_cast<Array*>(array)),
        m_version(version)
        {}

        FORCE_INLINE void copy(const Iterator& other) noexcept {
            std::memcpy(this, &other, sizeof(other));
        }

        PointerType m_ptr{nullptr};
        Array* m_array{nullptr};
        VersionTagType m_version{0};
    };

    class ConstIterator final {
    public:
        ConstIterator(const Iterator it):
        m_ptr(it.m_ptr),
        m_array(it.m_array),
        m_version(it.m_version)
        {}

        ConstIterator(const ConstIterator& other) noexcept { operator=(other); }
        ConstIterator(ConstIterator&& other) noexcept { operator=(std::move(other)); }
        ConstIterator& operator=(const ConstIterator& other) noexcept { copy(other); return *this; }
        ConstIterator& operator=(ConstIterator&& other) noexcept { copy(other); return *this; }

        ConstReferenceType operator*() const {
            ASSERTION(!isExpired() && !isOutOfRange(), std::runtime_error, "Dereferencing invalid const iterator")
            return *m_ptr;
        }
        ConstPointerType operator->() const {
            ASSERTION(!isExpired() && !isOutOfRange(), std::runtime_error, "Dereferencing invalid const iterator")
            return m_ptr;
        }
        FORCE_INLINE ConstIterator& operator++() noexcept { ++m_ptr; return *this; }
        FORCE_INLINE ConstIterator operator++(int) noexcept { ConstIterator tmp = *this; ++m_ptr; return tmp; }
        FORCE_INLINE ConstIterator& operator--() noexcept { --m_ptr; return *this; }
        FORCE_INLINE ConstIterator operator--(int) noexcept { ConstIterator tmp = *this; --m_ptr; return tmp; }

        FORCE_INLINE ConstIterator operator+(std::ptrdiff_t n) const noexcept { return ConstIterator{m_ptr + n, m_array, m_version}; }
        FORCE_INLINE ConstIterator operator-(std::ptrdiff_t n) const noexcept { return ConstIterator{m_ptr - n, m_array, m_version}; }
        FORCE_INLINE ConstIterator& operator+=(std::ptrdiff_t n) noexcept { m_ptr += n; return *this; }
        FORCE_INLINE ConstIterator& operator-=(std::ptrdiff_t n) noexcept { m_ptr -= n; return *this; }
        FORCE_INLINE std::ptrdiff_t operator-(const Iterator& other) const noexcept { return m_ptr - other.m_ptr; }
        FORCE_INLINE ConstReferenceType operator[](std::ptrdiff_t n) const { return (operator+(n)).operator*(); }
        FORCE_INLINE ReferenceType operator[](std::ptrdiff_t n) { return (operator+(n)).operator*(); }

        FORCE_INLINE bool operator==(const ConstIterator& other) const noexcept { return m_ptr == other.m_ptr; }
        FORCE_INLINE bool operator!=(const ConstIterator& other) const noexcept { return m_ptr != other.m_ptr; }
        FORCE_INLINE bool operator<(const ConstIterator& other) const noexcept { return m_ptr < other.m_ptr; }
        FORCE_INLINE bool operator>(const ConstIterator& other) const noexcept { return m_ptr > other.m_ptr; }
        FORCE_INLINE bool operator<=(const ConstIterator& other) const noexcept { return m_ptr <= other.m_ptr; }
        FORCE_INLINE bool operator>=(const ConstIterator& other) const noexcept { return m_ptr >= other.m_ptr; }

        FORCE_INLINE bool isExpired() const noexcept { return m_version != m_array->m_version; }
        FORCE_INLINE bool isOutOfRange() const noexcept { return m_ptr >= m_array->endIter().m_ptr; }

    private:
        friend Array;

        ConstIterator(PointerType ptr, const Array* array, VersionTagType version):
        m_ptr(ptr),
        m_array(const_cast<Array*>(array)),
        m_version(version)
        {}

        FORCE_INLINE void copy(const ConstIterator& other) noexcept {
            std::memcpy(this, &other, sizeof(other));
        }

        PointerType m_ptr{nullptr};
        Array* m_array{nullptr};
        VersionTagType m_version{0};
    };

    class ReverseIterator final {
    public:
        ReverseIterator(const ReverseIterator& other) noexcept { operator=(other); }
        ReverseIterator(ReverseIterator&& other) noexcept { operator=(std::move(other)); }
        ReverseIterator& operator=(const ReverseIterator& other) noexcept { copy(other); return *this; }
        ReverseIterator& operator=(ReverseIterator&& other) noexcept { copy(other); return *this; }

        Iterator reverse() const noexcept {
            auto start = m_array->data();
            auto end = start + m_array->size();
            auto ptr = end - m_ptr + start;
            return Iterator{ptr, m_array, m_version};
        }

        ReferenceType operator*() {
            ASSERTION(!isExpired() && !isOutOfRange(), std::runtime_error, "Dereferencing invalid reverse iterator")
            return *m_ptr;
        }
        ConstReferenceType operator*() const {
            ASSERTION(!isExpired() && !isOutOfRange(), std::runtime_error, "Dereferencing invalid reverse iterator")
            return *m_ptr;
        }
        PointerType operator->() {
            ASSERTION(!isExpired() && !isOutOfRange(), std::runtime_error, "Dereferencing invalid reverse iterator")
            return m_ptr;
        }
        ConstPointerType operator->() const {
            ASSERTION(!isExpired() && !isOutOfRange(), std::runtime_error, "Dereferencing invalid reverse iterator")
            return m_ptr;
        }

        FORCE_INLINE ReverseIterator& operator++() noexcept { --m_ptr; return *this; }
        FORCE_INLINE ReverseIterator operator++(int) noexcept { ReverseIterator tmp = *this; --m_ptr; return tmp; }
        FORCE_INLINE ReverseIterator& operator--() noexcept { ++m_ptr; return *this; }
        FORCE_INLINE ReverseIterator operator--(int) noexcept { ReverseIterator tmp = *this; ++m_ptr; return tmp; }

        FORCE_INLINE ReverseIterator operator+(std::ptrdiff_t n) const noexcept { return ReverseIterator{m_ptr - n, m_array, m_version}; }
        FORCE_INLINE ReverseIterator operator-(std::ptrdiff_t n) const noexcept { return ReverseIterator{m_ptr + n, m_array, m_version}; }
        FORCE_INLINE ReverseIterator& operator+=(std::ptrdiff_t n) noexcept { m_ptr -= n; return *this; }
        FORCE_INLINE ReverseIterator& operator-=(std::ptrdiff_t n) noexcept { m_ptr += n; return *this; }
        FORCE_INLINE std::ptrdiff_t operator-(const ReverseIterator& other) const noexcept { return other.m_ptr - m_ptr; }
        FORCE_INLINE ConstReferenceType operator[](std::ptrdiff_t n) const { return (operator+(n)).operator*(); }
        FORCE_INLINE ReferenceType operator[](std::ptrdiff_t n) { return (operator+(n)).operator*(); }

        FORCE_INLINE bool operator==(const ReverseIterator& other) const noexcept { return m_ptr == other.m_ptr; }
        FORCE_INLINE bool operator!=(const ReverseIterator& other) const noexcept { return m_ptr != other.m_ptr; }
        FORCE_INLINE bool operator<(const ReverseIterator& other) const noexcept { return m_ptr > other.m_ptr; }
        FORCE_INLINE bool operator>(const ReverseIterator& other) const noexcept { return m_ptr < other.m_ptr; }
        FORCE_INLINE bool operator<=(const ReverseIterator& other) const noexcept { return m_ptr >= other.m_ptr; }
        FORCE_INLINE bool operator>=(const ReverseIterator& other) const noexcept { return m_ptr <= other.m_ptr; }

        FORCE_INLINE bool isExpired() const noexcept { return m_version != m_array->m_version; }
        FORCE_INLINE bool isOutOfRange() const noexcept { return m_ptr < m_array->firstIter().m_ptr; }

    private:
        friend Array;

        ReverseIterator(PointerType ptr, const Array* array, VersionTagType version) :
        m_ptr(ptr),
        m_array(const_cast<Array*>(array)),
        m_version(version)
        {}

        FORCE_INLINE void copy(const ReverseIterator& other) noexcept {
            std::memcpy(this, &other, sizeof(other));
        }

        PointerType m_ptr{nullptr};
        Array* m_array{nullptr};
        VersionTagType m_version{0};
    };

    class ReverseConstIterator final {
    public:
        ReverseConstIterator(const ReverseConstIterator& other) noexcept { operator=(other); }
        ReverseConstIterator(ReverseConstIterator&& other) noexcept { operator=(std::move(other)); }
        ReverseConstIterator& operator=(const ReverseConstIterator& other) noexcept { copy(other); return *this; }
        ReverseConstIterator& operator=(ReverseConstIterator&& other) noexcept { copy(other); return *this; }

        ConstIterator reverse() const noexcept {
            auto start = m_array->firstConstIter().m_ptr;
            auto end = m_array->lastConstIter().m_ptr;
            auto ptr = end - m_ptr + start;
            return ConstIterator{ptr, m_array, m_version};
        }

        ConstReferenceType operator*() const {
            ASSERTION(!isExpired() && !isOutOfRange(), std::runtime_error, "Dereferencing invalid reverse const iterator")
            return *m_ptr;
        }
        ConstPointerType operator->() const {
            ASSERTION(!isExpired() && !isOutOfRange(), std::runtime_error, "Dereferencing invalid reverse const iterator")
            return m_ptr;
        }

        FORCE_INLINE ReverseConstIterator& operator++() noexcept { --m_ptr; return *this; }
        FORCE_INLINE ReverseConstIterator operator++(int) noexcept { ReverseConstIterator tmp = *this; --m_ptr; return tmp; }
        FORCE_INLINE ReverseConstIterator& operator--() noexcept { ++m_ptr; return *this; }
        FORCE_INLINE ReverseConstIterator operator--(int) noexcept { ReverseConstIterator tmp = *this; ++m_ptr; return tmp; }

        FORCE_INLINE ReverseConstIterator operator+(std::ptrdiff_t n) const noexcept { return ReverseConstIterator{m_ptr - n, m_array, m_version}; }
        FORCE_INLINE ReverseConstIterator operator-(std::ptrdiff_t n) const noexcept { return ReverseConstIterator{m_ptr + n, m_array, m_version}; }
        FORCE_INLINE ReverseConstIterator& operator+=(std::ptrdiff_t n) noexcept { m_ptr -= n; return *this; }
        FORCE_INLINE ReverseConstIterator& operator-=(std::ptrdiff_t n) noexcept { m_ptr += n; return *this; }
        FORCE_INLINE std::ptrdiff_t operator-(const ReverseConstIterator& other) const noexcept { return other.m_ptr - m_ptr; }
        FORCE_INLINE ConstReferenceType operator[](std::ptrdiff_t n) const { return (operator+(n)).operator*(); }
        FORCE_INLINE ReferenceType operator[](std::ptrdiff_t n) { return (operator+(n)).operator*(); }

        FORCE_INLINE bool operator==(const ReverseConstIterator& other) const noexcept { return m_ptr == other.m_ptr; }
        FORCE_INLINE bool operator!=(const ReverseConstIterator& other) const noexcept { return m_ptr != other.m_ptr; }
        FORCE_INLINE bool operator<(const ReverseConstIterator& other) const noexcept { return m_ptr > other.m_ptr; }
        FORCE_INLINE bool operator>(const ReverseConstIterator& other) const noexcept { return m_ptr < other.m_ptr; }
        FORCE_INLINE bool operator<=(const ReverseConstIterator& other) const noexcept { return m_ptr >= other.m_ptr; }
        FORCE_INLINE bool operator>=(const ReverseConstIterator& other) const noexcept { return m_ptr <= other.m_ptr; }

        FORCE_INLINE bool isExpired() const noexcept { return m_version != m_array->m_version; }
        FORCE_INLINE bool isOutOfRange() const noexcept { return m_ptr < m_array->firstConstIter().m_ptr; }

    private:
        friend Array;

        ReverseConstIterator(PointerType ptr, const Array* array, VersionTagType version) :
        m_ptr(ptr),
        m_array(const_cast<Array*>(array)),
        m_version(version)
        {}

        FORCE_INLINE void copy(const ReverseConstIterator& other) noexcept {
            std::memcpy(this, &other, sizeof(other));
        }

        PointerType m_ptr{nullptr};
        Array* m_array{nullptr};
        VersionTagType m_version{0};
    };

    FORCE_INLINE constexpr explicit Array() = default;
    FORCE_INLINE Array(const Array& other) { copy(other); }
    FORCE_INLINE Array(Array&& other) { swap(other); }
    FORCE_INLINE Array& operator=(const Array& other) {
        clear();
        copy(other);
        return *this;
    }
    FORCE_INLINE Array& operator=(Array&& other) {
        clear();
        swap(other);
        return *this;
    }

    template<typename InputIterator>
    Array(InputIterator first, InputIterator last) { pushBack(first, last); }
    Array(const T& data, SizeType count) {
        (void)insert(firstConstIter(), data, count);
    }
    Array(std::initializer_list<const T> data) {
        pushBack(data.begin(), data.end());
    }
    ~Array() { clear(); }

    FORCE_INLINE void pushBack(std::initializer_list<const T> data) {
        pushBack(data.begin(), data.end());
    }

    template<typename InputIterator>
    void pushBack(InputIterator first, InputIterator last) {
        (void)insert(endConstIter(), first, last);
    }

    void pushBack(const T& data) {
        ASSERTION(size() + 1 <= capacity(), std::runtime_error, "used up all the memory")
        (void)insertBackElement(data);
    }

    void pushBack(T&& data) {
        ASSERTION(size() + 1 <= capacity(), std::runtime_error, "used up all the memory")
        (void)insertBackElement(std::move(data));
    }

    template<typename ... Arg>
    void emplaceBack(Arg&& ... arg) {
        ASSERTION(size() + 1 <= capacity(), std::runtime_error, "used up all the memory")
        (void)emplaceBackElement(std::forward<Arg>(arg) ... );
    }

    template<typename ... Arg>
    Iterator emplace(ConstIterator pos, Arg&& ... arg) {
        ASSERTION(!pos.isExpired(), std::runtime_error, "Using expired iterator for emplace")
        ASSERTION(size() + 1 <= capacity(), std::runtime_error, "used up all the memory")
        return emplaceElement(iterToIndex(pos), std::forward<Arg>(arg) ... );
    }

    template<typename InputIterator>
    Iterator insert(ConstIterator pos, InputIterator first, InputIterator last) {
        static_assert(iter::isInputIterator<InputIterator>);
        ASSERTION(!pos.isExpired(), std::runtime_error, "Using expired iterator for emplace")
        ASSERTION(size() + iter::Distance(first, last) <= capacity(), std::runtime_error, "used up all the memory")
        auto start = iterToIndex(pos);
        auto i = 0;
        for (auto it = first; it != last; ++it, ++i) {
            (void)insertElement(start + i, *it);
        }
        return indexToIter(start);
    }

    Iterator insert(ConstIterator pos, const T& data) {
        ASSERTION(!pos.isExpired(), std::runtime_error, "Using expired iterator for emplace")
        ASSERTION(size() + 1 <= capacity(), std::runtime_error, "used up all the memory")
        return insertElement(iterToIndex(pos), data);
    }
    Iterator insert(ConstIterator pos, T&& data) {
        ASSERTION(!pos.isExpired(), std::runtime_error, "Using expired iterator for emplace")
        ASSERTION(size() + 1 <= capacity(), std::runtime_error, "used up all the memory")
        if constexpr (std::is_move_constructible_v<T>) {
            return insertElement(iterToIndex(pos), std::move(data));
        } else {
            return insertElement(iterToIndex(pos), data);
        }
    }
    Iterator insert(ConstIterator pos, const T& data, SizeType count) {
        ASSERTION(!pos.isExpired(), std::runtime_error, "Using expired iterator for emplace")
        ASSERTION(size() + count <= capacity(), std::runtime_error, "used up all the memory")
        auto start = iterToIndex(pos);
        for (auto i = 0; i < count; ++i) {
            (void)insertElement(start + i, data);
        }
        return indexToIter(start);
    }
    FORCE_INLINE Iterator insert(ConstIterator pos, std::initializer_list<const T> data) {
        return insert(pos, data.begin(), data.end());
    }

    void popBack() {
        ASSERTION(!isEmpty(), std::runtime_error, "Out of memory")
        memory::Destruct(indexToPtr(size() - 1));
        --m_size;
    }
    FORCE_INLINE Iterator erase(ConstIterator pos) {
        return erase(pos, pos + 1);
    }
    Iterator erase(ConstIterator first, ConstIterator last) {
        ASSERTION(!first.isExpired() && !last.isExpired(),
                  std::runtime_error, "Using expired iterators to erase")
        if LIKELY_EXPR(first < last) {
            const auto firstIndex = iterToIndex(first);
            const auto lastIndex = iterToIndex(last);
            ASSERTION(firstIndex < size() && lastIndex <= size(), std::runtime_error, "")
            const auto numErased = lastIndex - firstIndex;

            destructElements(firstIndex, lastIndex);
            shiftElements(lastIndex, size(), -numErased);
            updateVersion();

            if LIKELY_EXPR(!isEmpty()) {
                return indexToIter(firstIndex);
            } else {
                return endIter();
            }
        } else if (first == last) {
            return Iterator{first.m_ptr, first.m_array, first.m_version};
        } else {
            return endIter();
        }
    }
    void clear() noexcept(std::is_nothrow_destructible_v<T>) {
        if LIKELY_EXPR(!isEmpty()) {
            destructElements(0, size());
        }
    }

    FORCE_INLINE ConstReferenceType atUnsafe(SizeType index) const noexcept {
        return *(data() + index);
    }
    FORCE_INLINE ReferenceType atUnsafe(SizeType index) noexcept { return *(data() + index); }
    ConstReferenceType operator[](SizeType index) const {
        ASSERTION(index < size(), std::runtime_error, "Out of range")
        return atUnsafe(index);
    }
    ReferenceType operator[](SizeType index) {
        ASSERTION(index < size(), std::runtime_error, "Out of range")
        return atUnsafe(index);
    }

    ConstReferenceType firstElement() const {
        ASSERTION(!isEmpty(), std::runtime_error, "Static array is empty")
        return *firstConstIter();
    }
    ReferenceType firstElement() {
        ASSERTION(!isEmpty(), std::runtime_error, "Static array is empty")
        return *firstIter();
    }
    ConstReferenceType lastElement() const {
        ASSERTION(!isEmpty(), std::runtime_error, "Static array is empty")
        return *lastConstIter();
    }
    ReferenceType lastElement() {
        ASSERTION(!isEmpty(), std::runtime_error, "Static array is empty")
        return *lastIter();
    }

    Iterator firstIter() noexcept { return Iterator{data(), this, m_version}; }
    Iterator lastIter() noexcept {
        if LIKELY_EXPR(size() > 1) {
            return Iterator{data() + size() - 1, this, m_version};
        } else {
            return firstIter();
        }
    }
    Iterator endIter() noexcept { return Iterator{data() + size(), this, m_version}; }

    ConstIterator firstConstIter() const noexcept {
        return ConstIterator{const_cast<PointerType>(data()), this, m_version};
    }
    ConstIterator lastConstIter() const noexcept {
        if LIKELY_EXPR(size() > 1) {
            return ConstIterator{const_cast<PointerType>(data()) + size() - 1, this, m_version};
        } else {
            return firstConstIter();
        }
    }
    ConstIterator endConstIter() const noexcept {
        return ConstIterator{const_cast<PointerType>(data()) + size(), this, m_version};
    }

    ReverseIterator firstReverseIter() noexcept {
        if LIKELY_EXPR(size() > 0) {
            return ReverseIterator{data() + size() - 1, this, m_version};
        } else {
            return endReverseIter();
        }
    }
    ReverseIterator lastReverseIter() noexcept {
        if LIKELY_EXPR(size() > 0) {
            return ReverseIterator{data(), this, m_version};
        } else {
            return endReverseIter();
        }
    }
    ReverseIterator endReverseIter() noexcept {
        return ReverseIterator{data() - 1, this, m_version};
    }

    ReverseConstIterator firstReverseConstIter() const noexcept {
        if LIKELY_EXPR(size() > 0) {
            return ReverseConstIterator{const_cast<PointerType>(data()) + size() - 1, this, m_version};
        } else {
            return endReverseConstIter();
        }
    }
    ReverseConstIterator lastReverseConstIter() const noexcept {
        if LIKELY_EXPR(size() > 0) {
            return ReverseConstIterator{const_cast<PointerType>(data()), this, m_version};
        } else {
            return endReverseConstIter();
        }
    }
    ReverseConstIterator endReverseConstIter() const noexcept {
        return ReverseConstIterator{const_cast<PointerType>(data()) - 1, this, m_version};
    }

    FORCE_INLINE constexpr SizeType size() const noexcept {
        return m_size;
    }

    FORCE_INLINE constexpr SizeType capacity() const noexcept {
        return N;
    }

    FORCE_INLINE constexpr bool isEmpty() const noexcept {
        return (size() == 0);
    }

    FORCE_INLINE constexpr bool isFull() const noexcept {
        return size() >= capacity();
    }

    FORCE_INLINE ConstPointerType data() const noexcept {
        return reinterpret_cast<ConstPointerType>(m_data);
    }
    FORCE_INLINE PointerType data() noexcept {
        return reinterpret_cast<PointerType>(m_data);
    }

private:
    void copy(const Array& other) IS_NOEXCEPT_CONSTRUCIBLE(T) {
        assert(capacity() == other.capacity());
        const auto otherSize = other.size();
        PointerType thisSrc = data();
        ConstPointerType otherSrc = other.data();
        if constexpr (std::is_trivially_copyable_v<T> && std::is_default_constructible_v<T>) {
            std::memcpy(thisSrc, otherSrc, otherSize * sizeof(T));
        } else if constexpr (std::is_copy_constructible_v<T>) {
            for (auto i = 0; i < otherSize; ++i) {
                memory::Construct(thisSrc + i, *(otherSrc + i));
            }
        } else {
            static_assert(std::is_copy_constructible_v<T>,
                          "T must be copy constructible");
        }
        m_size = otherSize;
    }
    void swap(Array& other) IS_NOEXCEPT_CONSTR_AND_DESTR(T) {
        assert(capacity() == other.capacity());
        const auto otherSize = other.size();
        auto thisSrc = data();
        auto otherSrc = other.data();
        if constexpr (std::is_trivially_copyable_v<T> && std::is_default_constructible_v<T>) {
            std::memcpy(thisSrc, otherSrc, otherSize * sizeof(T));
        } else if constexpr (std::is_move_constructible_v<T>) {
            for (auto i = 0; i < otherSize; ++i) {
                memory::Construct(thisSrc + i, std::move(*(otherSrc + i)));
            }
        } else if constexpr (std::is_copy_constructible_v<T>) {
            for (auto i = 0; i < otherSize; ++i) {
                memory::Construct(thisSrc + i, *(otherSrc + i));
                memory::Destruct(otherSrc + i);
            }
        } else {
            static_assert(std::is_move_constructible_v<T> || std::is_copy_constructible_v<T>,
                          "T must be move or copy constructible");
        }
        m_size = otherSize;
        other.clear();
    }

    void destructElements(SizeType firstIndex, SizeType lastIndex) {
        assert(firstIndex <= lastIndex && firstIndex < size() && lastIndex <= size());
        const auto ptr = data();
        for (auto i = firstIndex; i < lastIndex; ++i) {
            memory::Destruct(ptr + i);
        }
        m_size -= lastIndex - firstIndex;
    }

    FORCE_INLINE SizeType iterToIndex(ConstIterator it) {
        return static_cast<SizeType>(it.m_ptr - data());
    }

    FORCE_INLINE PointerType indexToPtr(SizeType index) {
        return data() + index;
    }

    FORCE_INLINE Iterator indexToIter(SizeType index) {
        return Iterator{data() + index, this, m_version};
    }

    template<typename R>
    Iterator insertBackElement(R&& data) {
        const auto index = size();
        assert(index < capacity());
        memory::Construct(indexToPtr(index), std::forward<R>(data));
        ++m_size;
        updateVersion();
        return indexToIter(index);
    }

    template<typename R>
    Iterator insertElement(SizeType index, R&& data) {
        assert(index < capacity());
        shiftElements(index, size(), 1);
        memory::Construct(indexToPtr(index), std::forward<R>(data));
        ++m_size;
        updateVersion();
        return indexToIter(index);
    }

    template<typename ... Arg>
    Iterator emplaceBackElement(Arg&& ... arg) {
        const auto index = size();
        assert(index < capacity());
        memory::Construct(indexToPtr(index), std::forward<Arg>(arg) ... );
        ++m_size;
        updateVersion();
        return lastIter();
    }

    template<typename ... Arg>
    Iterator emplaceElement(SizeType index, Arg&& ... arg) {
        assert(index < capacity());
        shiftElements(index, size(), 1);
        memory::Construct(indexToPtr(index), std::forward<Arg>(arg) ... );
        ++m_size;
        updateVersion();
        return lastIter();
    }

    void shiftElements(SizeType firstIndex, SizeType count, int offset) {
        if UNLIKELY_EXPR(count == 0 || offset == 0) {
            return;
        }

        auto* start = data();
        if constexpr (std::is_trivially_copyable_v<T>) {
            std::memmove(start + firstIndex + offset,
                        start + firstIndex,
                        count * sizeof(T));
        } else {
            if (offset > 0) {
                for (auto i = count; i > 0; --i) {
                    const SizeType srcIdx = firstIndex + i - 1;
                    const SizeType dstIdx = srcIdx + offset;
                    assert(srcIdx < capacity());
                    assert(dstIdx < capacity());
                    if constexpr (std::is_nothrow_move_assignable_v<T>) {
                        start[dstIdx] = std::move(start[srcIdx]);
                    } else if constexpr (std::is_copy_constructible_v<T>) {
                        memory::Construct(start + dstIdx, start[srcIdx]);
                        memory::Destruct(start + srcIdx);
                    } else {
                        static_assert(std::is_move_constructible_v<T>,
                            "Type T must be movable or copyable");
                    }
                }
            } else {
                for (auto i = 0; i < count; ++i) {
                    const SizeType srcIdx = firstIndex + i;
                    const SizeType dstIdx = srcIdx + offset;
                    assert(srcIdx < capacity());
                    assert(dstIdx < capacity());
                    if constexpr (std::is_nothrow_move_assignable_v<T>) {
                        start[dstIdx] = std::move(start[srcIdx]);
                    } else if constexpr (std::is_copy_constructible_v<T>) {
                        memory::Construct(start + dstIdx, start[srcIdx]);
                        memory::Destruct(start + srcIdx);
                    } else {
                        static_assert(std::is_move_constructible_v<T>,
                            "Type T must be movable or copyable");
                    }
                }
            }
        }
    }

    FORCE_INLINE void updateVersion() {
        ++m_version;
    }

    VersionTagType m_version{0};
    SizeType m_size{0};
    std::byte m_data[N * sizeof(T)]{};
};

} //! namespace atom::containers::fixed

#endif //! ATOM_STATIC_ARRAY_H
