#ifndef ATOM_DYNAMIC_ARRAY_H
#define ATOM_DYNAMIC_ARRAY_H

#include "include/memory/allocators/default_allocator.h"
#include "include/utils/owner.h"
#include "include/utils/assertion.h"
#include "include/types/pre-defined_types.h"

#include <type_traits>
#include <limits>
#include <initializer_list>

#include <cstring>
#include <cassert>

namespace atom::containers {

namespace __dynamic_array_impl_details {

template<typename A>
class MemoryBuffer final {
public:
    using AllocatorType = A;

    MemoryBuffer(const MemoryBuffer& ) = delete;
    MemoryBuffer(MemoryBuffer&& other) noexcept = delete;
    MemoryBuffer& operator=(const MemoryBuffer& ) = delete;
    MemoryBuffer& operator=(MemoryBuffer&& other) noexcept = delete;

    MemoryBuffer(std::size_t size):
    m_data(nullptr),
    m_allocator() {
        static_assert (std::is_same_v<AllocatorType, atom::memory::allocator::DefaultAllocator>);
        m_data = atom::memory::allocator::DefaultAllocator{}.allocate(types::Size{size});
    }

    MemoryBuffer(std::size_t size, utils::owner::Reference<A> allocator):
    m_data(nullptr),
    m_allocator(allocator) {
        m_allocator.accessMutable([this, &size](AllocatorType& allocator) {
            m_data = allocator.allocate(types::Size{size});
        });
    }

    ~MemoryBuffer() {
        if (m_data) {
            if constexpr (std::is_same_v<AllocatorType, atom::memory::allocator::DefaultAllocator>) {
                atom::memory::allocator::DefaultAllocator{}.deallocate(m_data);
            } else {
                m_allocator.accessMutable([this](AllocatorType& allocator) {
                    allocator.deallocate(m_data);
                });
            }
            m_data = nullptr;
        }
    }

    std::byte* data() noexcept { return m_data; }
    const std::byte* data() const noexcept { return m_data; }
    utils::owner::Reference<A> getAllocator() const noexcept { return m_allocator; }

private:
    std::byte* m_data{nullptr};
    utils::owner::Reference<A> m_allocator;
};

} //! namespace __dynamic_array_impl_details

template<typename A>
struct DefaultDynamicArrayGrowthPolicyType {
    void operator()(std::size_t& capacity, utils::owner::Reference<A> /*allocator*/) const noexcept {
        capacity *= 2;
    }
};

template<
    typename T,
    typename A = memory::allocator::DefaultAllocator,
    typename G = DefaultDynamicArrayGrowthPolicyType<A>
> class DynamicArray final {
public:
    using SizeType = std::size_t;
    using CapacityType = std::size_t;
    using IndexType = std::size_t;
    using ElementType = T;
    using ReferenceType = ElementType&;
    using ConstReferenceType = const ElementType&;
    using PointerType = ElementType*;
    using ConstPointerType = const ElementType*;
    using AllocatorType = A;
    using GrowthPolicyType = G;
    using AllocationVersionType = std::uint64_t;

    class Iterator {
    public:
        Iterator(const Iterator& other) noexcept = default;
        Iterator(Iterator&& other) noexcept = default;
        Iterator& operator=(const Iterator& other) noexcept = default;
        Iterator& operator=(Iterator&& other) noexcept = default;
        ~Iterator() = default;

        constexpr bool isValid() const noexcept {
            return m_ptr <= static_cast<PointerType>(m_pArray->lastIter());
        }

        constexpr bool isExpired() const noexcept {
            return m_version != m_pArray->m_version;
        }

        explicit constexpr operator PointerType() const noexcept {
            return m_ptr;
        }

        explicit constexpr operator ConstPointerType() const noexcept {
            return m_ptr;
        }

        explicit constexpr operator IndexType() const {
            return IndexType{0};
        }

        Iterator& operator+=(std::size_t offset) {
            m_ptr += offset;
            return *this;
        }

        Iterator& operator-=(std::size_t offset) {
            m_ptr -= offset;
            return *this;
        }

        Iterator operator+(std::size_t offset) const {
            Iterator tmp{*this};
            tmp += 1;
            return tmp;
        }

        Iterator operator-(std::size_t offset) const {
            Iterator tmp{*this};
            tmp -= 1;
            return tmp;
        }

        Iterator& operator++() {
            operator+=(1);
            return *this;
        }
        Iterator operator++(int) {
            Iterator tmp{*this};
            operator+=(1);
            return tmp;
        }
        Iterator& operator--() {
            operator-=(1);
            return *this;
        }
        Iterator operator--(int) {
            Iterator tmp{*this};
            operator-=(1);
            return tmp;
        }

    private:
        friend DynamicArray;
        Iterator(PointerType ptr, DynamicArray* pArray, std::uint64_t version):
        m_ptr(ptr),
        m_pArray(pArray),
        m_version(version)
        {}

        PointerType get() {
            ASSERTION(!isExpired() && isValid(), std::runtime_error, "Attempt to deref invalid iterator")
            return m_ptr;
        }

        PointerType m_ptr{nullptr};
        DynamicArray* m_pArray{nullptr};
        std::uint64_t m_version{0};
    };

    class ConstIterator {
    public:
        explicit ConstIterator(Iterator it): m_it(it) {}

        constexpr bool isValid() const noexcept {
            return m_it.isValid();
        }

        constexpr bool isExpired() const noexcept {
            return m_it.isExpired();
        }

        explicit constexpr operator IndexType() const {
            return IndexType{0};
        }

        ConstIterator& operator+=(std::size_t offset) {
            m_it.operator+=(offset);
            return *this;
        }

        ConstIterator& operator-=(std::size_t offset) {
            m_it.operator-=(offset);
            return *this;
        }

        ConstIterator operator+(std::size_t offset) const {
            return ConstIterator{m_it.operator+(offset)};
        }

        ConstIterator operator-(std::size_t offset) const {
            return ConstIterator{m_it.operator-(offset)};
        }

        ConstIterator& operator++() {
            operator+=(1);
            return *this;
        }
        ConstIterator operator++(int) {
            ConstIterator tmp{*this};
            operator+=(1);
            return tmp;
        }
        ConstIterator& operator--() {
            operator-=(1);
            return *this;
        }
        ConstIterator operator--(int) {
            ConstIterator tmp{*this};
            operator-=(1);
            return tmp;
        }

    private:
        Iterator m_it;
    };

    class ReverseIterator {
    public:
        constexpr bool isValid() const noexcept {
            return m_it.isValid();
        }

        constexpr bool isExpired() const noexcept {
            return m_it.isExpired();
        }
    private:
        ReverseIterator(Iterator it): m_it(it) {}

        Iterator m_it;
    };

    class ConstReverseIterator {
    public:
        constexpr bool isValid() const noexcept {
            return m_it.isValid();
        }

        constexpr bool isExpired() const noexcept {
            return m_it.isExpired();
        }
    private:
        ConstReverseIterator(Iterator it): m_it(it) {}

        Iterator m_it;
    };

    explicit DynamicArray(utils::owner::Reference<AllocatorType> allocator = {}):
    m_storage(0, allocator),
    m_size(0),
    m_capacity(0) {}

    explicit DynamicArray(
        CapacityType capacity,
        utils::owner::Reference<AllocatorType> allocator = {}
    ):
    m_storage(capacity, allocator),
    m_size(0),
    m_capacity(capacity) {}

    explicit DynamicArray(
        SizeType size,
        const T& defaultData = T{},
        utils::owner::Reference<AllocatorType> allocator = {}
    ):
    m_storage(size, allocator),
    m_size(size),
    m_capacity(m_size) {
        (void)insert(firstConstIter(), size, defaultData);
    }

    explicit DynamicArray(
        std::initializer_list<T> data,
        utils::owner::Reference<AllocatorType> allocator = {}
    ):
    m_storage(data.size(), allocator),
    m_size(data.size()),
    m_capacity(m_size) {
        (void)insert(firstConstIter(), data);
    }

    DynamicArray(const DynamicArray& other) {
        (void)operator=(other);
    }

    DynamicArray(DynamicArray&& other) {
        (void)operator=(std::move(other));
    }

    DynamicArray& operator=(const DynamicArray& other) {
        clear();
        copy(other);
        return *this;
    }
    DynamicArray& operator=(DynamicArray&& other) {
        clear();
        swap(other);
        return *this;
    }

    ~DynamicArray() { clear(); }

    void reserve(SizeType size) {
        if (size < capacity()) {
            return;
        }

        DynamicArray newArray{size, getAllocator()};
        if constexpr (std::is_trivially_copyable_v<T>) {
            std::memcpy(newArray.m_storage.data(), m_storage.data(), m_size * sizeof(T));
        } else {
            for (std::size_t i = 0; i < m_size; ++i) {
                newArray.pushBack(std::move_if_noexcept(atUnsafe(IndexType{i})));
            }
        }
        // --------------------- Kalb Line --------------------- //
        swap(newArray);
        updateVersion();
    }

    void resize(SizeType size) {
        const auto newSize = size;
        if (newSize > m_size) {
            increaseArray(newSize);
        } else if (newSize < m_size) {
            decreaseArray(newSize);
        } else {}
    }

    void shrinkToFit() {
        DynamicArray copy{*this};
        swap(copy);
    }

    void pushBack(const T& data) {
        (void)emplaceBack(data);
    }

    void pushBack(T&& data) {
        (void)emplaceBack(std::move(data));
    }

    template<typename ... Arg>
    void emplaceBack(Arg&& ... arg) {
        (void)emplace(lastConstIter() + 1, std::forward<Arg>(arg) ... );
    }

    // Разместить новый элемент перед указаным итератором
    template<typename ... Arg>
    Iterator emplace(ConstIterator pos, Arg&& ... arg) {
        ASSERTION(pos <= lastConstIter() + 1 && !pos.isExpired(), std::runtime_error, "Invalid/Expired iterator")
        if (m_capacity == m_size) {
            auto newCapacity = m_capacity;
            GrowthPolicyType{}(newCapacity, getAllocator());
            reserve(newCapacity);
        }

        shiftElements(static_cast<std::size_t>(pos), m_size, 1);
        construct(&*pos, std::forward<Arg>(arg) ... );
        ++m_size;
        updateVersion();
        return Iterator{pos};
    }

    void popBack() {
        (void)erase(lastConstIter());
    }

    void clear() {
        destructElements(
            static_cast<IndexType>(firstConstIter()),
            static_cast<IndexType>(lastConstIter() + 1)
        );
        m_size = 0;
    }

    // Вставить новый элемент перед указаным итератором
    Iterator insert(Iterator pos, const T& data) {
        return insert(pos, SizeType{1}, data);
    }

    Iterator insert(Iterator pos, T&& data) {
        return insert(pos, SizeType{1}, std::move(data));
    }

    Iterator insert(Iterator pos, SizeType count, const T& data) {
        const auto n = count;
        ASSERTION(n > 0, std::runtime_error, "`count` must be > 0")
        reserve(size() + n);
        const auto result = emplace(pos, data);
        for (std::size_t i = 1; i < n; ++i) {
            emplace(pos + i, data);
        }
        return result;
    }

    Iterator insert(Iterator pos, std::initializer_list<T> data) {
        reserve(size() + data.size());
        const auto n = data.size();
        const auto result = emplace(pos, data[0]);
        for (std::size_t i = 1; i < n; ++i) {
            emplace(pos + i, std::move(data[i]));
        }
        return result;
    }

    Iterator erase(ConstIterator pos) {
        return erase(pos, pos + 1);
    }

    Iterator erase(ConstIterator first, ConstIterator last) {
        if (first != last) {
            destructElements(
                static_cast<std::size_t>(first),
                static_cast<std::size_t>(last)
            );
            shiftElements(static_cast<std::size_t>(last), m_size, -(last - first));
            updateVersion();
        }
    }

    utils::owner::Reference<AllocatorType> getAllocator() const noexcept {
        return m_storage.getAllocator();
    }

    ReferenceType operator[](IndexType index) {
        ASSERTION(index < size(), std::runtime_error, "Out of range")
        return atUnsafe(index);
    }

    ConstReferenceType operator[](IndexType index) const {
        ASSERTION(index < size(), std::runtime_error, "Out of range")
        return atUnsafe(index);
    }

    ReferenceType atUnsafe(IndexType index) {
        return *reinterpret_cast<PointerType>(m_storage.data()) + index;
    }

    ConstReferenceType atUnsafe(IndexType index) const {
        return *reinterpret_cast<ConstPointerType>(m_storage.data()) + index;
    }

    SizeType size() const noexcept { return SizeType{m_size}; }
    SizeType capacity() const noexcept { return SizeType{m_capacity}; }
    bool isEmpty() const noexcept { return m_size == 0; }
    bool isValidIter(ConstIterator it) const noexcept {
        return it.isValid();
    }
    bool isValidIter(ConstReverseIterator it) const noexcept {
        return it.isValid();
    }

    ConstReferenceType firstElement() const { return *firstConstIter(); }
    ConstReferenceType lastElement() const { return *lastConstIter(); }

    ReferenceType firstElement() { return *firstIter(); }
    ReferenceType lastElement() { return *lastIter(); }

    ConstIterator firstConstIter() const noexcept {
        // `const_cast` in this context is safe because noone doesn`t mutate `this`!
        return ConstIterator{const_cast<DynamicArray<T, A, G>*>(this)->firstIter()};
    }
    ConstIterator lastConstIter() const noexcept {
        // `const_cast` in this context is safe because noone doesn`t mutate `this`!
        return ConstIterator{const_cast<DynamicArray<T, A, G>*>(this)->lastIter()};
    }

    Iterator firstIter() noexcept {
        if (!isEmpty()) {
            return Iterator{reinterpret_cast<ElementType*>(m_storage.data()), this, m_version};
        } else {
            return Iterator{nullptr, this, INVALID_ALLOCATION_VERSION};
        }
    }
    Iterator lastIter() noexcept {
        if (!isEmpty()) {
            return Iterator{reinterpret_cast<ElementType*>(m_storage.data() + size() - 1), this, m_version};
        } else {
            return Iterator{nullptr, this, INVALID_ALLOCATION_VERSION};
        }
    }

    ConstReverseIterator firstConstReserveIter() const noexcept {
        // `const_cast` in this context is safe because noone doesn`t mutate `this`!
        return ConstReverseIterator{const_cast<DynamicArray<T, A, G>*>(this)->firstIter()};
    }
    ConstReverseIterator lastConstReserveIter() const noexcept {
        // `const_cast` in this context is safe because noone doesn`t mutate `this`!
        return ConstReverseIterator{const_cast<DynamicArray<T, A, G>*>(this)->lastIter()};
    }

    ReverseIterator firstReverseIter() noexcept {
        return ReverseIterator{firstIter()};
    }
    ReverseIterator lastReverseIter() noexcept {
        return ReverseIterator{lastIter()};
    }

private:
    static constexpr AllocationVersionType INVALID_ALLOCATION_VERSION = 0;
    static constexpr AllocationVersionType FIRST_VALID_ALLOCATION_VERSION = INVALID_ALLOCATION_VERSION + 1;
    static constexpr AllocationVersionType VERSION_LIMIT = std::numeric_limits<AllocationVersionType>::max();

    using BufferType = __dynamic_array_impl_details::MemoryBuffer<AllocatorType>;

    template<typename ... Arg>
    void construct(PointerType ptr, Arg&& ... arg) {
        if constexpr (std::is_same_v<AllocatorType, memory::allocator::DefaultAllocator>) {
            memory::allocator::DefaultAllocator{}.construct(ptr, std::forward<Arg>(arg) ... );
        } else {
            getAllocator().accessMutable([&](AllocatorType& allocator) {
                allocator.construct(ptr, std::forward<Arg>(arg) ... );
            });
        }
    }

    void destruct(PointerType ptr) {
        if constexpr (std::is_same_v<AllocatorType, memory::allocator::DefaultAllocator>) {
            memory::allocator::DefaultAllocator{}.destruct(ptr);
        } else {
            getAllocator().accessMutable([&](AllocatorType& allocator) {
                allocator.destruct(ptr);
            });
        }
    }

    void copy(const DynamicArray& other) {
        reserve(other.size());
        if constexpr (std::is_trivial_v<T>) {
            std::memcpy(m_storage.data(), other.m_storage.data(), other.m_size);
            m_size = other.m_size;
        } else {
            for (std::size_t i = 0; i < other.m_size; ++i) {
                pushBack(other.atUnsafe(IndexType{i}));
            }
        }
        updateVersion();
    }

    void swap(DynamicArray& other) noexcept {
        std::swap(m_storage, other.m_storage);
        std::swap(m_size, other.m_size);
        std::swap(m_capacity, other.m_capacity);
        updateVersion();
        other.updateVersion();
    }

    void increaseArray(std::size_t newSize) {
        if (m_size == 0 || (std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T>)) {
            reserve(SizeType{newSize});
            for (std::size_t i = m_size; i < newSize; ++i) {
                construct(m_storage.data() + i, T{});
            }
            m_size = newSize;
        } else {
            DynamicArray newArray{newSize, getAllocator()};
            newArray = *this;
            do {
                newArray.pushBack(T{});
            } while (newArray.size().load() < newSize);
            // --------------------- Kalb Line --------------------- //
            swap(newArray);
        }
        updateVersion();
    }

    void decreaseArray(std::size_t newSize) {
        /*
         * When decreasing the size, we only call destructors for "extra" elements
         * (from newSize to m_size) and update m_size.
         * No memory operations or data movement occur - only destruction of objects.
         * According to the C++ standard, destructors should not throw exceptions
         * (unless it is a special case with noexcept(false), which is extremely rare and bad).
         * If the destructor of T does throw an exception, it is UB (undefined behavior),
         * and the container is not required to handle it.
         */
        destructElements(newSize, m_size);
        m_size = newSize;
        updateVersion();
    }

    void shiftElements(std::size_t firstIndex, std::size_t lastIndex, int offset) {
        assert(firstIndex <= lastIndex && offset != 0);
        assert(offset >= 0 || firstIndex >= static_cast<std::size_t>(-offset));
        assert(firstIndex + (offset > 0 ? offset : 0) <= m_capacity);

        if constexpr (std::is_trivial_v<T>) {
            PointerType src = m_storage.data() + firstIndex;
            PointerType dst = src + offset;
            std::size_t count = lastIndex - firstIndex;
            std::memmove(dst, src, sizeof(T) * count);
        } else {
            PointerType first = m_storage.data() + firstIndex;
            PointerType last = m_storage.data() + lastIndex - 1;
            if (offset > 0) {
                // Сдвиг вправо (от конца к началу)
                for (PointerType it = last - 1; it >= first; --it) {
                    construct(it + offset, std::move_if_noexcept(*it));
                    destruct(it);
                }
            } else {
                // Сдвиг влево (от начала к концу)
                for (PointerType it = first; it < last; ++it) {
                    construct(it + offset, std::move_if_noexcept(*it));
                    destruct(it);
                }
            }
        }
    }

    void destructElements(std::size_t firstIndex, std::size_t lastIndex) {
        if constexpr (!std::is_trivial_v<T>) {
            for (auto it = firstIndex; it < lastIndex; ++it) {
                destruct(m_storage.data() + it);
            }
        }
    }

    void updateVersion() noexcept {
        m_version = (m_version < VERSION_LIMIT ?
                                   m_version + 1 :
                                   FIRST_VALID_ALLOCATION_VERSION
        );
    }

    BufferType m_storage;
    std::size_t m_size{0};
    std::size_t m_capacity{0};
    AllocationVersionType m_version{FIRST_VALID_ALLOCATION_VERSION};
};

} //! namespace atom::containers

#endif //! ATOM_DYNAMIC_ARRAY_H
