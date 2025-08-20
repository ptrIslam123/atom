#ifndef ATOM_STATIC_ARRAY_H
#define ATOM_STATIC_ARRAY_H

#include "include/compiler/compiler_attr.h"
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
    using ReferenceType = ElementType&;
    using ConstReferenceType = const ElementType&;
    using PointerType = ElementType*;
    using ConstPointerType = const ElementType*;
    using SizeType = std::size_t;
    using CapacityType = std::size_t;
    using VersionTagType = std::uint64_t;

    class Iterator final {
    public:
        using difference_type = std::ptrdiff_t;

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
        ConstPointerType operator->() const noexcept { return m_ptr; }
        PointerType operator->() noexcept { return m_ptr; }

        Iterator& operator++() noexcept { ++m_ptr; return *this; }
        Iterator operator++(int) noexcept { Iterator tmp = *this; ++m_ptr; return tmp; }
        Iterator& operator--() noexcept { --m_ptr; return *this; }
        Iterator operator--(int) noexcept { Iterator tmp = *this; --m_ptr; return tmp; }

        Iterator operator+(int n) const noexcept { return Iterator{m_ptr + n, m_array, m_version}; }
        Iterator operator-(int n) const noexcept { return Iterator{m_ptr - n, m_array, m_version}; }
        Iterator operator+=(int n) noexcept { m_ptr += n; return *this; }
        Iterator operator-=(int n) noexcept { m_ptr -= n; return *this; }
        difference_type operator-(const Iterator& other) const noexcept { return m_ptr - other.m_ptr; }

        bool operator==(const Iterator& other) const noexcept { return m_ptr == other.m_ptr; }
        bool operator!=(const Iterator& other) const noexcept { return m_ptr != other.m_ptr; }
        bool operator<(const Iterator& other) const noexcept { return m_ptr < other.m_ptr; }
        bool operator>(const Iterator& other) const noexcept { return m_ptr > other.m_ptr; }
        bool operator<=(const Iterator& other) const noexcept { return m_ptr <= other.m_ptr; }
        bool operator>=(const Iterator& other) const noexcept { return m_ptr >= other.m_ptr; }

        bool isExpired() const noexcept { return m_version != m_array->m_version; }
        bool isOutOfRange() const noexcept { return m_ptr >= m_array->endIter().m_ptr; }

    private:
        friend StaticArray;

        Iterator(PointerType ptr, const StaticArray* array, VersionTagType version):
        m_ptr(ptr),
        m_array(const_cast<StaticArray*>(array)),
        m_version(version)
        {}

        void copy(const Iterator& other) noexcept {
            std::memcpy(this, &other, sizeof(other));
        }

        PointerType m_ptr{nullptr};
        StaticArray* m_array{nullptr};
        VersionTagType m_version{0};
    };

    class ConstIterator final {
    public:
        using difference_type = std::ptrdiff_t;
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
        ConstPointerType operator->() const { return m_ptr; }
        ConstIterator& operator++() noexcept { ++m_ptr; return *this; }
        ConstIterator operator++(int) noexcept { ConstIterator tmp = *this; ++m_ptr; return tmp; }
        ConstIterator& operator--() noexcept { --m_ptr; return *this; }
        ConstIterator operator--(int) noexcept { ConstIterator tmp = *this; --m_ptr; return tmp; }

        ConstIterator operator+(int n) const noexcept { return ConstIterator{m_ptr + n, m_array, m_version}; }
        ConstIterator operator-(int n) const noexcept { return ConstIterator{m_ptr - n, m_array, m_version}; }
        ConstIterator& operator+=(int n) noexcept { m_ptr += n; return *this; }
        ConstIterator& operator-=(int n) noexcept { m_ptr -= n; return *this; }
        difference_type operator-(const Iterator& other) const noexcept { return m_ptr - other.m_ptr; }

        bool operator==(const ConstIterator& other) const noexcept { return m_ptr == other.m_ptr; }
        bool operator!=(const ConstIterator& other) const noexcept { return m_ptr != other.m_ptr; }
        bool operator<(const ConstIterator& other) const noexcept { return m_ptr < other.m_ptr; }
        bool operator>(const ConstIterator& other) const noexcept { return m_ptr > other.m_ptr; }
        bool operator<=(const ConstIterator& other) const noexcept { return m_ptr <= other.m_ptr; }
        bool operator>=(const ConstIterator& other) const noexcept { return m_ptr >= other.m_ptr; }

        bool isExpired() const noexcept { return m_version != m_array->m_version; }
        bool isOutOfRange() const noexcept { return m_ptr >= m_array->endIter().m_ptr; }

    private:
        friend StaticArray;

        ConstIterator(PointerType ptr, const StaticArray* array, VersionTagType version):
        m_ptr(ptr),
        m_array(const_cast<StaticArray*>(array)),
        m_version(version)
        {}

        void copy(const ConstIterator& other) noexcept {
            std::memcpy(this, &other, sizeof(other));
        }

        PointerType m_ptr{nullptr};
        StaticArray* m_array{nullptr};
        VersionTagType m_version{0};
    };

    class ReverseIterator final {
    public:
        using difference_type = std::ptrdiff_t;

        ReverseIterator(const ReverseIterator& other) noexcept { operator=(other); }
        ReverseIterator(ReverseIterator&& other) noexcept { operator=(std::move(other)); }
        ReverseIterator& operator=(const ReverseIterator& other) noexcept { copy(other); return *this; }
        ReverseIterator& operator=(ReverseIterator&& other) noexcept { copy(other); return *this; }

        ReferenceType operator*() {
            ASSERTION(!isExpired() && !isOutOfRange(), std::runtime_error, "Dereferencing invalid reverse iterator")
            return *m_ptr;
        }
        ConstReferenceType operator*() const {
            ASSERTION(!isExpired() && !isOutOfRange(), std::runtime_error, "Dereferencing invalid reverse iterator")
            return *m_ptr;
        }
        PointerType operator->() noexcept { return m_ptr; }
        ConstPointerType operator->() const noexcept { return m_ptr; }

        ReverseIterator& operator++() noexcept { --m_ptr; return *this; }
        ReverseIterator operator++(int) noexcept { ReverseIterator tmp = *this; --m_ptr; return tmp; }
        ReverseIterator& operator--() noexcept { ++m_ptr; return *this; }
        ReverseIterator operator--(int) noexcept { ReverseIterator tmp = *this; ++m_ptr; return tmp; }

        ReverseIterator operator+(int n) const noexcept { return ReverseIterator{m_ptr - n, m_array, m_version}; }
        ReverseIterator operator-(int n) const noexcept { return ReverseIterator{m_ptr + n, m_array, m_version}; }
        ReverseIterator& operator+=(int n) noexcept { m_ptr -= n; return *this; }
        ReverseIterator& operator-=(int n) noexcept { m_ptr += n; return *this; }
        difference_type operator-(const ReverseIterator& other) const noexcept { return other.m_ptr - m_ptr; }

        bool operator==(const ReverseIterator& other) const noexcept { return m_ptr == other.m_ptr; }
        bool operator!=(const ReverseIterator& other) const noexcept { return m_ptr != other.m_ptr; }
        bool operator<(const ReverseIterator& other) const noexcept { return m_ptr > other.m_ptr; }
        bool operator>(const ReverseIterator& other) const noexcept { return m_ptr < other.m_ptr; }
        bool operator<=(const ReverseIterator& other) const noexcept { return m_ptr >= other.m_ptr; }
        bool operator>=(const ReverseIterator& other) const noexcept { return m_ptr <= other.m_ptr; }

        bool isExpired() const noexcept { return m_version != m_array->m_version; }
        bool isOutOfRange() const noexcept { return m_ptr < m_array->firstIter().m_ptr; }

    private:
        friend StaticArray;

        ReverseIterator(PointerType ptr, const StaticArray* array, VersionTagType version) :
        m_ptr(ptr),
        m_array(const_cast<StaticArray*>(array)),
        m_version(version)
        {}

        void copy(const ReverseIterator& other) noexcept {
            std::memcpy(this, &other, sizeof(other));
        }

        PointerType m_ptr{nullptr};
        StaticArray* m_array{nullptr};
        VersionTagType m_version{0};
    };

    class ReverseConstIterator final {
    public:
        using difference_type = std::ptrdiff_t;

        ReverseConstIterator(const ReverseConstIterator& other) noexcept { operator=(other); }
        ReverseConstIterator(ReverseConstIterator&& other) noexcept { operator=(std::move(other)); }
        ReverseConstIterator& operator=(const ReverseConstIterator& other) noexcept { copy(other); return *this; }
        ReverseConstIterator& operator=(ReverseConstIterator&& other) noexcept { copy(other); return *this; }

        ConstReferenceType operator*() const {
            ASSERTION(!isExpired() && !isOutOfRange(), std::runtime_error, "Dereferencing invalid reverse const iterator")
            return *m_ptr;
        }
        ConstPointerType operator->() const noexcept { return m_ptr; }

        ReverseConstIterator& operator++() noexcept { --m_ptr; return *this; }
        ReverseConstIterator operator++(int) noexcept { ReverseConstIterator tmp = *this; --m_ptr; return tmp; }
        ReverseConstIterator& operator--() noexcept { ++m_ptr; return *this; }
        ReverseConstIterator operator--(int) noexcept { ReverseConstIterator tmp = *this; ++m_ptr; return tmp; }

        ReverseConstIterator operator+(int n) const noexcept { return ReverseConstIterator{m_ptr - n, m_array, m_version}; }
        ReverseConstIterator operator-(int n) const noexcept { return ReverseConstIterator{m_ptr + n, m_array, m_version}; }
        ReverseConstIterator& operator+=(int n) noexcept { m_ptr -= n; return *this; }
        ReverseConstIterator& operator-=(int n) noexcept { m_ptr += n; return *this; }
        difference_type operator-(const ReverseConstIterator& other) const noexcept { return other.m_ptr - m_ptr; }

        bool operator==(const ReverseConstIterator& other) const noexcept { return m_ptr == other.m_ptr; }
        bool operator!=(const ReverseConstIterator& other) const noexcept { return m_ptr != other.m_ptr; }
        bool operator<(const ReverseConstIterator& other) const noexcept { return m_ptr > other.m_ptr; }
        bool operator>(const ReverseConstIterator& other) const noexcept { return m_ptr < other.m_ptr; }
        bool operator<=(const ReverseConstIterator& other) const noexcept { return m_ptr >= other.m_ptr; }
        bool operator>=(const ReverseConstIterator& other) const noexcept { return m_ptr <= other.m_ptr; }

        bool isExpired() const noexcept { return m_version != m_array->m_version; }
        bool isOutOfRange() const noexcept { return m_ptr < m_array->beginIter().m_ptr; }

    private:
        friend StaticArray;

        ReverseConstIterator(PointerType ptr, const StaticArray* array, VersionTagType version) :
        m_ptr(ptr),
        m_array(const_cast<StaticArray*>(array)),
        m_version(version)
        {}

        void copy(const ReverseConstIterator& other) noexcept {
            std::memcpy(this, &other, sizeof(other));
        }

        PointerType m_ptr{nullptr};
        StaticArray* m_array{nullptr};
        VersionTagType m_version{0};
    };

    explicit StaticArray() = default;
    StaticArray(const T& data, SizeType count) {
        insert(firstConstIter(), data, count);
    }
    StaticArray(std::initializer_list<const T> data) {
        pushBack(data);
    }
    StaticArray(std::span<const T> data) {
        pushBack(data);
    }
    ~StaticArray() { clear(); }

    void pushBack(std::initializer_list<const T> data) {
        pushBack(std::span<const T>{data});
    }

    void pushBack(std::span<const T> data) {
        ASSERTION(size() + data.size() <= capacity(), std::runtime_error, "used up all the memory")
        for (auto i = 0; i < data.size(); ++i) {
            (void)insertBackElement(data[i]);
        }
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
    Iterator insert(ConstIterator pos, std::initializer_list<const T> data) {
        return insert(pos, std::span<const T>{data});
    }
    Iterator insert(ConstIterator pos, std::span<const T> data) {
        ASSERTION(!pos.isExpired(), std::runtime_error, "Using expired iterator for emplace")
        ASSERTION(size() + data.size() <= capacity(), std::runtime_error, "used up all the memory")
        auto start = iterToIndex(pos);
        for (auto i = 0; i < data.size(); ++i) {
            (void)insertElement(start + i, data[i]);
        }
        return indexToIter(start);
    }

    void popBack() {
        ASSERTION(!isEmpty(), std::runtime_error, "Out of memory")
        destruct(indexToPtr(size() - 1));
        --m_size;
    }
    Iterator erase(ConstIterator pos) {
        return erase(pos, pos + 1);
    }
    Iterator erase(ConstIterator first, ConstIterator last) {
        ASSERTION(!first.isExpired() && !last.isExpired(), std::runtime_error, "Using expired iterators to erase")
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
        if (!isEmpty()) {
            destructElements(0, size());
        }
    }

    ConstReferenceType atUnsafe(SizeType index) const noexcept { return *(data() + index); }
    ReferenceType atUnsafe(SizeType index) noexcept { return *(data() + index); }
    ConstReferenceType operator[](SizeType index) const {
        ASSERTION(index < size(), std::runtime_error, "Out of range")
        return atUnsafe(index);
    }
    ReferenceType operator[](SizeType index) {
        ASSERTION(index < size(), std::runtime_error, "Out of range")
        return atUnsafe(index);
    }

    ConstReferenceType firstElement() const {
        ASSERTION(!isEmpty(), std::runtime_error, "static array is empty")
        return *firstConstIter();
    }
    ReferenceType firstElement() {
        ASSERTION(!isEmpty(), std::runtime_error, "static array is empty")
        return *firstIter();
    }
    ConstReferenceType lastElement() const {
        ASSERTION(!isEmpty(), std::runtime_error, "static array is empty")
        return *lastConstIter();
    }
    ReferenceType lastElement() {
        ASSERTION(!isEmpty(), std::runtime_error, "static array is empty")
        return *lastIter();
    }

    Iterator firstIter() { return Iterator{data(), this, m_version}; }
    Iterator lastIter() {
        if LIKELY_EXPR(size() > 1) {
            return Iterator{data() + size() - 1, this, m_version};
        } else {
            return firstIter();
        }
    }
    Iterator endIter() const noexcept { return Iterator{const_cast<PointerType>(data()) + size(), this, m_version}; }
    ConstIterator firstConstIter() const noexcept { return ConstIterator{const_cast<PointerType>(data()), this, m_version}; }
    ConstIterator lastConstIter() const noexcept {
        if LIKELY_EXPR(size() > 1) {
            return ConstIterator{const_cast<PointerType>(data()) + size() - 1, this, m_version};
        } else {
            return firstConstIter();
        }
    }
    ConstIterator endConstIter() const noexcept { return ConstIterator{const_cast<PointerType>(data()) + size(), this, m_version}; }

    ReverseIterator firstReverseIter() const noexcept {
        if LIKELY_EXPR(size() > 0) {
            return ReverseIterator{const_cast<PointerType>(data() + size() - 1), this, m_version};
        } else {
            return endReverseIter();
        }
    }
    ReverseIterator lastReverseIter() const noexcept { return ReverseIterator{const_cast<PointerType>(data()), this, m_version};  }
    ReverseIterator endReverseIter() const noexcept { return ReverseIterator{const_cast<PointerType>(data()), this, m_version}; }

    ReverseConstIterator firstReverseConstIter() const noexcept {
        if LIKELY_EXPR(size() > 0) {
            return ReverseConstIterator{const_cast<PointerType>(data() + size() - 1), this, m_version};
        } else {
            return endReverseConstIter();
        }
    }
    ReverseConstIterator lastReverseConstIter() const noexcept { return ReverseConstIterator{const_cast<PointerType>(data()), this, m_version};  }
    ReverseConstIterator endReverseConstIter() const noexcept { return ReverseConstIterator{const_cast<PointerType>(data()), this, m_version}; }

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
        return data() + index;
    }

    Iterator indexToIter(SizeType index) {
        return Iterator{data() + index, this, m_version};
    }

    template<typename R>
    Iterator insertBackElement(R&& data) {
        const auto index = size();
        assert(index < capacity());
        construct(indexToPtr(index), std::forward<R>(data));
        ++m_size;
        updateVersion();
        return indexToIter(index);
    }

    template<typename R>
    Iterator insertElement(SizeType index, R&& data) {
        assert(index < capacity());
        shiftElements(index, size(), 1);
        construct(indexToPtr(index), std::forward<R>(data));
        ++m_size;
        updateVersion();
        return indexToIter(index);
    }

    template<typename ... Arg>
    Iterator emplaceBackElement(Arg&& ... arg) {
        const auto index = size();
        assert(index < capacity());
        construct(indexToPtr(index), std::forward<Arg>(arg) ... );
        ++m_size;
        updateVersion();
        return lastIter();
    }

    template<typename ... Arg>
    Iterator emplaceElement(SizeType index, Arg&& ... arg) {
        assert(index < capacity());
        shiftElements(index, size(), 1);
        construct(indexToPtr(index), std::forward<Arg>(arg) ... );
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
                        construct(start + dstIdx, start[srcIdx]);
                        destruct(start + srcIdx);
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
                        construct(start + dstIdx, start[srcIdx]);
                        destruct(start + srcIdx);
                    } else {
                        static_assert(std::is_move_constructible_v<T>,
                            "Type T must be movable or copyable");
                    }
                }
            }
        }
    }

    void updateVersion() {
        ++m_version;
    }

    VersionTagType m_version{0};
    SizeType m_size{0};
    std::byte m_data[N * sizeof(T)]{};
};

} //! namespace atom::containers

#endif //! ATOM_STATIC_ARRAY_H
