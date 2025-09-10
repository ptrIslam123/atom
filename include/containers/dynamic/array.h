#ifndef ATOM_DYNAMIC_ARRAY_H
#define ATOM_DYNAMIC_ARRAY_H

#include "include/memory/allocators/default_allocator.h"
#include "include/utils/owner.h"
#include "include/utils/assertion.h"
#include "include/types/pre-defined_types.h"
#include "include/sfinae/has_class_inner_type.h"
#include "include/sfinae/has_class_method.h"

#include <exception>
#include <type_traits>
#include <limits>
#include <initializer_list>
#include <span>

#include <cstring>
#include <cassert>

namespace atom::containers {

namespace __dynamic_array_impl_details {

DECLARE_HAS_CLASS_INNER_TYPE(AllocatorType)
DECLARE_HAS_CLASS_INNER_TYPE(ElementType)
DECLARE_HAS_CLASS_INNER_TYPE(SizeType)

template<typename A>
class MemoryBuffer final {
public:
    static_assert(HAS_CLASS_INNERT_TYPE_AllocatorType<A>::value, "A must have an inner type AllocatorType");
    static_assert(HAS_CLASS_INNERT_TYPE_ElementType<A>::value, "A must have an inner type ElementType");
    static_assert(HAS_CLASS_INNERT_TYPE_SizeType<A>::value, "A must have an inner type SizeType");

    using ArrayType = A;
    using SizeType = typename ArrayType::SizeType;
    using ElementType = typename ArrayType::ElementType;
    using AllocatorType = typename ArrayType::AllocatorType;

    MemoryBuffer(const MemoryBuffer& ) = delete;
    MemoryBuffer(MemoryBuffer&& other) noexcept = delete;
    MemoryBuffer& operator=(const MemoryBuffer& ) = delete;
    MemoryBuffer& operator=(MemoryBuffer&& other) noexcept = delete;

    MemoryBuffer(SizeType size):
    m_data(nullptr),
    m_allocator() {
        m_data = allocate(size * sizeof(ElementType));
    }

    MemoryBuffer(SizeType size, utils::bc::Reference<AllocatorType> allocator):
    m_data(nullptr),
    m_allocator(allocator) {
        m_data = allocate(size * sizeof(ElementType));
    }

    ~MemoryBuffer() {
        deallocate(m_data);
    }

    std::byte* data() noexcept { return m_data; }
    const std::byte* data() const noexcept { return m_data; }
    utils::bc::Reference<AllocatorType> getAllocator() const noexcept { return m_allocator; }
    void swap(MemoryBuffer& other) noexcept {
        std::swap(m_data, other.m_data);
        std::swap(m_allocator, other.m_allocator);
    }

private:
    std::byte* allocate(const SizeType size) {
        if (size == 0) [[unlikely]] {
            return nullptr;
        }

        std::byte* memory = nullptr;
        if constexpr (std::is_same_v<AllocatorType, atom::memory::allocator::DefaultAllocator>) {
            memory = atom::memory::allocator::DefaultAllocator{}.allocate(size);
        } else {
            m_allocator.accessMutable([&memory, size](AllocatorType& allocator) {
                memory = allocator.allocate(size);
            });
        }
        return memory;
    }
    void deallocate(std::byte *const memory) {
        if (!memory) [[unlikely]] {
            return;
        }
        if constexpr (std::is_same_v<AllocatorType, atom::memory::allocator::DefaultAllocator>) {
            //atom::memory::allocator::DefaultAllocator{}.deallocate(memory);
        } else {
            m_allocator.accessMutable([this, memory](AllocatorType& allocator) {
                allocator.deallocate(memory);
            });
        }
        m_data = nullptr;
    }

    std::byte* m_data{nullptr};
    utils::bc::Reference<AllocatorType> m_allocator;
};

} //! namespace __dynamic_array_impl_details


/**
 * @struct DefaultDynamicArrayGrowthPolicyType
 * @brief Default growth policy for DynamicArray that doubles capacity
 *
 * @tparam A Allocator type (unused in default policy but castom impl can request some extra info from AllocatorType(A))
 *
 * @details This policy implements the standard exponential growth strategy:
 * - If current capacity is 0, grows to 1
 * - Otherwise doubles the current capacity
 *
 * @note The allocator parameter is unused but kept for interface compatibility
 * with custom policies that might need allocator awareness.
 */
template<typename A>
struct DefaultDynamicArrayGrowthPolicyType {
    void operator()(std::size_t& capacity, utils::bc::Reference<A> allocator) const noexcept;
};

/**
 * @class DynamicArray
 * @brief A dynamic array container with custom allocator support and growth policies
 *
 * @tparam T Type of elements stored in the array
 * @tparam A Allocator type (default: memory::allocator::DefaultAllocator)
 * @tparam G Growth policy type (default: DefaultDynamicArrayGrowthPolicyType<A>)
 *
 * @details This class provides a dynamic array implementation similar to std::vector but with:
 * - Custom allocator support
 * - Configurable growth policy
 * - Versioned iterators for safety
 * - Comprehensive iterator support (forward, reverse, const)
 */
template<
    typename T,
    typename A = memory::allocator::DefaultAllocator,
    typename G = DefaultDynamicArrayGrowthPolicyType<A>
> class DynamicArray final {
public:
    /// Type for size-related operations
    using SizeType = std::size_t;

    /// Type for capacity-related operations
    using CapacityType = std::size_t;

    /// Type for indexing operations
    using IndexType = std::size_t;

    /// Type of elements stored
    using ElementType = T;

    /// Reference to element type
    using ReferenceType = ElementType&;

    /// Const reference to element type
    using ConstReferenceType = const ElementType&;

    /// Pointer to element type
    using PointerType = ElementType*;

    /// Const pointer to element type
    using ConstPointerType = const ElementType*;

    /// Allocator type
    using AllocatorType = A;

    /// Growth policy type
    using GrowthPolicyType = G;

    /// Type for tracking allocation versions
    using AllocationVersionType = std::uint64_t;

    class Iterator {
    public:
        using DifferenceType = std::ptrdiff_t;

        Iterator(const Iterator& other) noexcept = default;
        Iterator(Iterator&& other) noexcept = default;
        Iterator& operator=(const Iterator& other) noexcept = default;
        Iterator& operator=(Iterator&& other) noexcept = default;
        ~Iterator() = default;

        bool isValid() const noexcept {
            const auto index = static_cast<IndexType>(m_ptr - reinterpret_cast<ConstPointerType>(const_cast<std::byte*>(m_pArray->m_storage.data())));
            return index < m_pArray->size();
        }

        bool isExpired() const noexcept {
            return (m_version == INVALID_ALLOCATION_VERSION || m_version != m_pArray->m_version);
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
            tmp += offset;
            return tmp;
        }

        Iterator operator-(std::size_t offset) const {
            Iterator tmp{*this};
            tmp -= offset;
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

        DifferenceType operator-(const Iterator& other) const {
            return m_ptr - other.m_ptr;
        }

        bool operator==(const Iterator& other) const noexcept {
            return m_ptr == other.m_ptr;
        }

        bool operator!=(const Iterator& other) const noexcept {
            return !(*this == other);
        }

        bool operator<(const Iterator& other) const noexcept {
            return m_ptr < other.m_ptr;
        }

        bool operator<=(const Iterator& other) const noexcept {
            return (*this < other) || (*this == other);
        }

        bool operator>(const Iterator& other) const noexcept {
            return !(*this <= other);
        }

        bool operator>=(const Iterator& other) const noexcept {
            return !(*this < other);
        }

        ReferenceType operator*() {
            return *get();
        }

        PointerType operator->() {
            return get();
        }

        ReferenceType operator[](std::size_t offset) {
            return *(get() + offset);
        }

        ConstReferenceType operator*() const {
            return *get();
        }

        ConstPointerType operator->() const {
            return get();
        }

        ConstReferenceType operator[](std::size_t offset) const {
            return *(get() + offset);
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

        ConstPointerType get() const {
            ASSERTION(!isExpired() && isValid(), std::runtime_error, "Attempt to deref invalid iterator")
            return m_ptr;
        }

        PointerType m_ptr{nullptr};
        DynamicArray* m_pArray{nullptr};
        std::uint64_t m_version{0};
    };

    class ConstIterator {
    public:
        using DifferenceType = typename Iterator::DifferenceType;

        ConstIterator(Iterator it): m_it(it) {}

        bool isValid() const noexcept {
            return m_it.isValid();
        }

        bool isExpired() const noexcept {
            return m_it.isExpired();
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

        DifferenceType operator-(const Iterator& other) const {
            return m_it.operator-(other);
        }

        bool operator==(const ConstIterator& other) const noexcept {
            return m_it.operator==(other.m_it);
        }

        bool operator!=(const ConstIterator& other) const noexcept {
            return !(*this == other);
        }

        bool operator<(const ConstIterator& other) const noexcept {
            return m_it.operator<(other.m_it);
        }

        bool operator<=(const ConstIterator& other) const noexcept {
            return (*this < other) || (*this == other);
        }

        bool operator>(const ConstIterator& other) const noexcept {
            return !(*this <= other);
        }

        bool operator>=(const ConstIterator& other) const noexcept {
            return !(*this < other);
        }

        ConstReferenceType operator*() const {
            return m_it.operator*();
        }

        ConstPointerType operator->() const {
            return m_it.operator->();
        }

        ConstReferenceType operator[](std::size_t offset) {
            return m_it.operator[](offset);
        }

    private:
        friend DynamicArray;
        Iterator m_it;
    };

    class ReverseIterator {
    public:
        using DifferenceType = typename Iterator::DifferenceType;

        ReverseIterator() = default;

        explicit ReverseIterator(Iterator it) : m_current(it) {}

        explicit operator Iterator() const { return m_current; }

        Iterator base() const { return m_current; }

        bool isValid() const noexcept { return m_current.isValid(); }
        bool isExpired() const noexcept { return m_current.isExpired(); }

        ReverseIterator& operator+=(std::size_t offset) {
            m_current -= offset;
            return *this;
        }

        ReverseIterator& operator-=(std::size_t offset) {
            m_current += offset;
            return *this;
        }

        ReverseIterator operator+(std::size_t offset) const {
            return ReverseIterator(m_current - offset);
        }

        ReverseIterator operator-(std::size_t offset) const {
            return ReverseIterator(m_current + offset);
        }

        ReverseIterator& operator++() {
            --m_current;
            return *this;
        }

        ReverseIterator operator++(int) {
            ReverseIterator tmp{*this};
            --m_current;
            return tmp;
        }

        ReverseIterator& operator--() {
            ++m_current;
            return *this;
        }

        ReverseIterator operator--(int) {
            ReverseIterator tmp{*this};
            ++m_current;
            return tmp;
        }

        DifferenceType operator-(const ReverseIterator& other) const {
            return other.m_current - m_current;
        }

        bool operator==(const ReverseIterator& other) const noexcept {
            return m_current == other.m_current;
        }

        bool operator!=(const ReverseIterator& other) const noexcept {
            return !(*this == other);
        }

        bool operator<(const ReverseIterator& other) const noexcept {
            return m_current > other.m_current;
        }

        bool operator<=(const ReverseIterator& other) const noexcept {
            return m_current >= other.m_current;
        }

        bool operator>(const ReverseIterator& other) const noexcept {
            return m_current < other.m_current;
        }

        bool operator>=(const ReverseIterator& other) const noexcept {
            return m_current <= other.m_current;
        }

        ReferenceType operator*() {
            return m_current.operator*();
        }

        PointerType operator->() {
            return m_current.operator->();
        }

        ReferenceType operator[](std::size_t offset) {
            return *(*this + offset);
        }

    private:
        Iterator m_current;
    };

    class ConstReverseIterator {
    public:
        using DifferenceType = typename Iterator::DifferenceType;

        ConstReverseIterator() = default;

        explicit ConstReverseIterator(ConstIterator it) : m_current(it) {}
        explicit ConstReverseIterator(ReverseIterator it) : m_current(it.base()) {}

        explicit operator ConstIterator() const { return m_current; }

        ConstIterator base() const { return m_current; }

        bool isValid() const noexcept { return m_current.isValid(); }
        bool isExpired() const noexcept { return m_current.isExpired(); }

        ConstReverseIterator& operator+=(std::size_t offset) {
            m_current -= offset;
            return *this;
        }

        ConstReverseIterator& operator-=(std::size_t offset) {
            m_current += offset;
            return *this;
        }

        ConstReverseIterator operator+(std::size_t offset) const {
            return ConstReverseIterator(m_current - offset);
        }

        ConstReverseIterator operator-(std::size_t offset) const {
            return ConstReverseIterator(m_current + offset);
        }

        ConstReverseIterator& operator++() {
            --m_current;
            return *this;
        }

        ConstReverseIterator operator++(int) {
            ConstReverseIterator tmp = *this;
            --m_current;
            return tmp;
        }

        ConstReverseIterator& operator--() {
            ++m_current;
            return *this;
        }

        ConstReverseIterator operator--(int) {
            ConstReverseIterator tmp = *this;
            ++m_current;
            return tmp;
        }

        DifferenceType operator-(const ConstReverseIterator& other) const {
            return other.m_current - m_current;
        }

        bool operator==(const ConstReverseIterator& other) const noexcept {
            return m_current == other.m_current;
        }

        bool operator!=(const ConstReverseIterator& other) const noexcept {
            return !(*this == other);
        }

        bool operator<(const ConstReverseIterator& other) const noexcept {
            return m_current > other.m_current;
        }

        bool operator<=(const ConstReverseIterator& other) const noexcept {
            return m_current >= other.m_current;
        }

        bool operator>(const ConstReverseIterator& other) const noexcept {
            return m_current < other.m_current;
        }

        bool operator>=(const ConstReverseIterator& other) const noexcept {
            return m_current <= other.m_current;
        }

        ConstReferenceType operator*() const {
            return m_current.operator*();
        }

        ConstPointerType operator->() const {
            return m_current.operator->();
        }

        ConstReferenceType operator[](std::size_t offset) const {
            return *(*this + offset);
        }

    private:
        ConstIterator m_current;
    };

    /**
     * @brief Default constructor
     * @param allocator Optional allocator reference
     */
    explicit DynamicArray(utils::bc::Reference<AllocatorType> allocator = {});

    /**
     * @brief Construct with initial capacity
     * @param capacity Initial capacity to reserve
     * @param allocator Optional allocator reference
     * @throw std::runtime_error
     */
    explicit DynamicArray(
        CapacityType capacity,
        utils::bc::Reference<AllocatorType> allocator = {}
    );

    /**
     * @brief Construct with initial size
     * @param size Initial number of elements
     * @param defaultData Value to initialize elements with
     * @param allocator Optional allocator reference
     */
    explicit DynamicArray(
        SizeType size,
        const T& defaultData = T{},
        utils::bc::Reference<AllocatorType> allocator = {}
    );

    /**
     * @brief Construct from initializer list
     * @param data Initializer list of elements
     * @param allocator Optional allocator reference
     */
    explicit DynamicArray(
        std::initializer_list<T> data,
        utils::bc::Reference<AllocatorType> allocator = {}
    );

    /**
     * @brief Construct from span
     * @param data Span of elements to copy
     * @param allocator Optional allocator reference
     */
    explicit DynamicArray(
        std::span<const T> data,
        utils::bc::Reference<AllocatorType> allocator = {}
    );

    DynamicArray(const DynamicArray& other);
    DynamicArray(DynamicArray&& other);
    DynamicArray& operator=(const DynamicArray& other);
    DynamicArray& operator=(DynamicArray&& other);
    ~DynamicArray();

    /**
     * @brief Reserve storage capacity
     * @param newCapacity Minimum capacity to reserve
     *
     * If newCapacity > current capacity, reallocates storage.
     * Otherwise does nothing.
     */
    void reserve(SizeType newCapacity);

    /**
     * @brief Resize the array
     * @param size New size of array
     *
     * If size > current size, new elements are default-constructed.
     * If size < current size, elements are destroyed from the end.
     */
    void resize(SizeType size);

    /**
     * @brief Reduce capacity to fit current size
     */
    void shrinkToFit();

    /**
     * @brief Add element to end (copy)
     * @param data Element to add
     */
    void pushBack(const T& data);

    /**
     * @brief Add element to end (move)
     * @param data Element to add
     */
    void pushBack(T&& data);

    /**
     * @brief Add range of elements
     * @param data Span of elements to add
     */
    void pushBack(std::span<const T> data);

    /**
    * @brief Construct element in-place at end
    * @tparam Arg Argument types
    * @param arg Arguments to forward to element constructor
    */
    template<typename ... Arg>
    void emplaceBack(Arg&& ... arg);

    /**
     * @brief Construct element in-place at position
     * @tparam Arg Argument types
     * @param pos Iterator to insertion position
     * @param arg Arguments to forward to element constructor
     * @return Iterator to inserted element
     */
    template<typename ... Arg>
    Iterator emplace(ConstIterator pos, Arg&& ... arg);

    /**
     * @brief Insert element at position (copy)
     * @param pos Iterator to insertion position
     * @param data Element to insert
     * @return Iterator to inserted element
     */
    Iterator insert(ConstIterator pos, const T& data);

    /**
     * @brief Insert element at position (move)
     * @param pos Iterator to insertion position
     * @param data Element to insert
     * @return Iterator to inserted element
     */
    Iterator insert(ConstIterator pos, T&& data);

    /**
     * @brief Insert multiple copies of element
     * @param pos Iterator to insertion position
     * @param count Number of copies to insert
     * @param data Element to insert
     * @return Iterator to first inserted element
     */
    Iterator insert(ConstIterator pos, SizeType count, const T& data);

    /**
     * @brief Insert elements from initializer list
     * @param pos Iterator to insertion position
     * @param data Elements to insert
     * @return Iterator to first inserted element
     */
    Iterator insert(ConstIterator pos, std::initializer_list<T> data);

    /**
     * @brief Insert elements from span
     * @param pos Iterator to insertion position
     * @param data Elements to insert
     * @return Iterator to first inserted element
     */
    Iterator insert(ConstIterator pos, std::span<const T> data);

    /**
     * @brief Remove last element
     */
    void popBack();

    /**
     * @brief Remove element at position
     * @param pos Iterator to element to remove
     * @return Iterator following last removed element
     */
    Iterator erase(ConstIterator pos);

    /**
     * @brief Remove range of elements
     * @param first Iterator to first element to remove
     * @param last Iterator to one past last element to remove
     * @return Iterator following last removed element
     */
    Iterator erase(ConstIterator first, ConstIterator last);

    /**
     * @brief Remove all elements
     */
    void clear();

    /**
     * @brief Get the allocator
     * @return bc::Reference to allocator
     */
    utils::bc::Reference<AllocatorType> getAllocator() const noexcept;

    /**
     * @brief Access element with bounds checking
     * @param index Element index
     * @return Reference to element
     * @throws std::runtime_error if index out of bounds
     */
    ReferenceType operator[](IndexType index);

    /**
     * @brief Access element with bounds checking (const)
     * @param index Element index
     * @return Const reference to element
     * @throws std::runtime_error if index out of bounds
     */
    ConstReferenceType operator[](IndexType index) const;

    /**
     * @brief Access element without bounds checking
     * @param index Element index
     * @return Reference to element
     * @warning No bounds checking performed
     */
    ReferenceType atUnsafe(IndexType index) noexcept;

    /**
     * @brief Access element without bounds checking (const)
     * @param index Element index
     * @return Const reference to element
     * @warning No bounds checking performed
     */
    ConstReferenceType atUnsafe(IndexType index) const noexcept;

    /**
     * @brief Get current size
     * @return Number of elements
     */
    SizeType size() const noexcept;

    /**
     * @brief Get current capacity
     * @return Current storage capacity
     */
    SizeType capacity() const noexcept;

    /**
     * @brief Check if empty
     * @return true if empty, false otherwise
     */
    bool isEmpty() const noexcept;

    /**
     * @brief Get first element (const)
     * @return Const reference to first element
     */
    ConstReferenceType firstElement() const;

    /**
     * @brief Get last element (const)
     * @return Const reference to last element
     */
    ConstReferenceType lastElement() const;

    /**
     * @brief Get first element
     * @return Reference to first element
     */
    ReferenceType firstElement();

    /**
     * @brief Get last element
     * @return Reference to last element
     */
    ReferenceType lastElement();

    /**
     * @brief Get const iterator to beginning
     * @return Const iterator to first element
     */
    ConstIterator firstConstIter() const noexcept;

    /**
     * @brief Get const iterator to last
     * @return Const iterator to last element
     */
    ConstIterator lastConstIter() const noexcept;

    /**
     * @brief Get const iterator to end
     * @return Const iterator to one past last element
     */
    ConstIterator endConstIter() const noexcept;

    /**
     * @brief Get iterator to beginning
     * @return Iterator to first element
     */
    Iterator firstIter() noexcept;

    /**
     * @brief Get iterator to last
     * @return Iterator to last element
     */
    Iterator lastIter() noexcept;

    /**
     * @brief Get iterator to end
     * @return Iterator to one past last element
     */
    Iterator endIter() noexcept;

    /**
     * @brief Get const reverse iterator to beginning
     * @return Const reverse iterator to last element
     */
    ConstReverseIterator firstConstReserveIter() const noexcept;

    /**
     * @brief Get const reverse iterator to last
     * @return Const reverse iterator to first element
     */
    ConstReverseIterator lastConstReserveIter() const noexcept;

    /**
     * @brief Get const reverse iterator to end
     * @return Const reverse iterator to one before first element
     */
    ConstReverseIterator endConstReserveIter() const noexcept;

    /**
     * @brief Get reverse iterator to beginning
     * @return Reverse iterator to last element
     */
    ReverseIterator firstReverseIter() noexcept;

    /**
     * @brief Get reverse iterator to last
     * @return Reverse iterator to first element
     */
    ReverseIterator lastReverseIter() noexcept;

    /**
     * @brief Get reverse iterator to end
     * @return Reverse iterator to one before first element
     */
    ReverseIterator endReverseIter() noexcept;

private:
    static constexpr AllocationVersionType INVALID_ALLOCATION_VERSION = 0;
    static constexpr AllocationVersionType FIRST_VALID_ALLOCATION_VERSION = INVALID_ALLOCATION_VERSION + 1;
    static constexpr AllocationVersionType VERSION_LIMIT = std::numeric_limits<AllocationVersionType>::max();

    using BufferType = __dynamic_array_impl_details::MemoryBuffer<DynamicArray>;

    template<typename ... Arg>
    void construct(PointerType ptr, Arg&& ... arg);
    void destruct(PointerType ptr);
    void destructElements(SizeType firstIndex, SizeType lastIndex);

    SizeType iteratorToIndex(ConstIterator it);
    PointerType indexToPtr(SizeType index);
    Iterator indexToIter(SizeType index);

    void shiftElements(SizeType firstIndex, SizeType lastIndex, int offset);
    template<typename ... Arg>
    Iterator emplaceElement(SizeType index, Arg&& ... arg);

    void copy(const DynamicArray& other);
    void swap(DynamicArray& other) noexcept;

    void increaseArray(SizeType newSize);
    void decreaseArray(SizeType newSize);

    inline void updateVersion() noexcept;

    SizeType m_size{0};
    CapacityType m_capacity{0};
    BufferType m_storage{};
    AllocationVersionType m_version{INVALID_ALLOCATION_VERSION};
};

template<typename T, typename A, typename G>
DynamicArray<T, A, G>::DynamicArray(utils::bc::Reference<AllocatorType> allocator):
m_storage(0, allocator),
m_size(0),
m_capacity(0),
m_version(INVALID_ALLOCATION_VERSION) {}

template<typename T, typename A, typename G>
DynamicArray<T, A, G>::DynamicArray(
    const CapacityType capacity,
    utils::bc::Reference<AllocatorType> allocator
):
m_size(0),
m_capacity(capacity),
m_storage(capacity, allocator),
m_version(INVALID_ALLOCATION_VERSION) {}


template<typename T, typename A, typename G>
DynamicArray<T, A, G>::DynamicArray(
    const SizeType size,
    const T& defaultData,
    utils::bc::Reference<AllocatorType> allocator
):
m_size(0),
m_capacity(m_size),
m_storage(size, allocator),
m_version(m_size > 0 ? FIRST_VALID_ALLOCATION_VERSION : INVALID_ALLOCATION_VERSION) {
    (void)insert(firstConstIter(), size, defaultData);
}

template<typename T, typename A, typename G>
DynamicArray<T, A, G>::DynamicArray(
    std::initializer_list<T> data,
    utils::bc::Reference<AllocatorType> allocator
):
m_size(0),
m_capacity(m_size),
m_storage(data.size(), allocator),
m_version(m_size > 0 ? FIRST_VALID_ALLOCATION_VERSION : INVALID_ALLOCATION_VERSION) {
    (void)insert(firstConstIter(), data);
}


template<typename T, typename A, typename G>
DynamicArray<T, A, G>::DynamicArray(
    std::span<const T> data,
    utils::bc::Reference<AllocatorType> allocator
):
m_size(0),
m_capacity(m_size),
m_storage(data.size(), allocator),
m_version(m_size > 0 ? FIRST_VALID_ALLOCATION_VERSION : INVALID_ALLOCATION_VERSION) {
    (void)insert(firstConstIter(), data);
}

template<typename T, typename A, typename G>
DynamicArray<T, A, G>::DynamicArray(const DynamicArray& other):
m_size(0),
m_capacity(0),
m_storage(0, utils::bc::Reference<AllocatorType>{}),
m_version(INVALID_ALLOCATION_VERSION) {
    (void)operator=(other);
}

template<typename T, typename A, typename G>
DynamicArray<T, A, G>::DynamicArray(DynamicArray&& other):
m_size(0),
m_capacity(0),
m_storage(0, utils::bc::Reference<AllocatorType>{}),
m_version(INVALID_ALLOCATION_VERSION) {
    (void)operator=(std::move(other));
}

template<typename T, typename A, typename G>
DynamicArray<T, A, G>& DynamicArray<T, A, G>::operator=(const DynamicArray& other) {
    clear();
    copy(other);
    return *this;
}

template<typename T, typename A, typename G>
DynamicArray<T, A, G>& DynamicArray<T, A, G>::operator=(DynamicArray&& other) {
    clear();
    swap(other);
    return *this;
}

template<typename T, typename A, typename G>
DynamicArray<T, A, G>::~DynamicArray() { clear(); }

template<typename T, typename A, typename G>
void DynamicArray<T, A, G>::reserve(const SizeType newCapacity) {
    if (newCapacity < capacity()) {
        return;
    }

    DynamicArray newArray{newCapacity, getAllocator()};
    if constexpr (std::is_trivially_copyable_v<T>) {
        std::memcpy(newArray.m_storage.data(), m_storage.data(), m_size * sizeof(T));
        newArray.m_size = size();
    } else {
        for (std::size_t i = 0; i < m_size; ++i) {
            newArray.pushBack(std::move_if_noexcept(atUnsafe(IndexType{i})));
        }
    }
    // --------------------- Kalb Line --------------------- //
    swap(newArray);
    updateVersion();
}

template<typename T, typename A, typename G>
void DynamicArray<T, A, G>::resize(const SizeType size) {
    const auto newSize = size;
    if (newSize > m_size) {
        increaseArray(newSize);
    } else if (newSize < m_size) {
        decreaseArray(newSize);
    } else {}
}

template<typename T, typename A, typename G>
void DynamicArray<T, A, G>::shrinkToFit() {
    DynamicArray copy{*this};
    swap(copy);
}

template<typename T, typename A, typename G>
inline void DynamicArray<T, A, G>::pushBack(const T& data) { (void)emplaceBack(data); }

template<typename T, typename A, typename G>
inline void DynamicArray<T, A, G>::pushBack(T&& data) { (void)emplaceBack(std::move(data)); }

template<typename T, typename A, typename G>
inline void DynamicArray<T, A, G>::pushBack(std::span<const T> data) {
    reserve(size() + data.size());
    for (const auto& value : data) {
        pushBack(value);
    }
}
template<typename T, typename A, typename G>
template<typename ... Arg>
inline void DynamicArray<T, A, G>::emplaceBack(Arg&& ... arg) { (void)emplace(endConstIter(), std::forward<Arg>(arg) ... ); }

template<typename T, typename A, typename G>
template<typename ... Arg>
typename DynamicArray<T, A, G>::Iterator DynamicArray<T, A, G>::emplace(ConstIterator pos, Arg&& ... arg) {
    ASSERTION(pos <= endConstIter(), std::runtime_error, "Out of range")
    const auto index = iteratorToIndex(pos);
    if (m_capacity == m_size) [[unlikely]] {
        auto newCapacity = m_capacity;
        GrowthPolicyType{}(newCapacity, getAllocator());
        reserve(newCapacity);
    }
    return emplaceElement(index, std::forward<Arg>(arg) ... );
}

template<typename T, typename A, typename G>
inline typename DynamicArray<T, A, G>::Iterator DynamicArray<T, A, G>::insert(ConstIterator pos, const T& data) { return insert(pos, SizeType{1}, data); }

template<typename T, typename A, typename G>
inline typename DynamicArray<T, A, G>::Iterator DynamicArray<T, A, G>::insert(ConstIterator pos, T&& data) { return insert(pos, SizeType{1}, std::move(data)); }

template<typename T, typename A, typename G>
typename DynamicArray<T, A, G>::Iterator DynamicArray<T, A, G>::insert(ConstIterator pos, const SizeType count, const T& data) {
    if (count == 0) [[unlikely]] {
        return endIter();
    }
    const auto index = iteratorToIndex(pos);
    reserve(size() + count);
    for (SizeType i = 0; i < count; ++i) {
        (void)emplaceElement(index + i, data);
    }
    return indexToIter(index);
}

template<typename T, typename A, typename G>
typename DynamicArray<T, A, G>::Iterator DynamicArray<T, A, G>::insert(ConstIterator pos, std::initializer_list<T> data) {
    const auto count = static_cast<SizeType>(data.size());
    if (count == 0) [[unlikely]] {
        return endIter();
    }
    const auto index = iteratorToIndex(pos);
    reserve(size() + count);
    for (SizeType i = 0; i < count; ++i) {
        (void)emplaceElement(index + i, *(data.begin() + i));
    }
    return indexToIter(index);
}

template<typename T, typename A, typename G>
typename DynamicArray<T, A, G>::Iterator DynamicArray<T, A, G>::insert(ConstIterator pos, std::span<const T> data) {
    const auto count = static_cast<SizeType>(data.size());
    if (count == 0) [[unlikely]] {
        return endIter();
    }
    const auto index = iteratorToIndex(pos);
    reserve(size() + count);
    for (SizeType i = 0; i < count; ++i) {
        (void)emplaceElement(index + i, data[i]);
    }
    return indexToIter(index);
}


template<typename T, typename A, typename G>
inline void DynamicArray<T, A, G>::popBack() { (void)erase(lastConstIter()); }

template<typename T, typename A, typename G>
inline typename DynamicArray<T, A, G>::Iterator DynamicArray<T, A, G>::erase(ConstIterator pos) { return erase(pos, pos + 1); }

template<typename T, typename A, typename G>
typename DynamicArray<T, A, G>::Iterator DynamicArray<T, A, G>::erase(ConstIterator first, ConstIterator last) {
    ASSERTION(first.isValid() && !first.isExpired() || first == endConstIter(), std::runtime_error, "First Iter invalid/out of range")
    ASSERTION(last.isValid() && !last.isExpired() || last == endConstIter(), std::runtime_error, "Last Iter invalid/out of range")
    if (first < last) {
        const auto firstIndex = iteratorToIndex(first);
        const auto lastIndex = iteratorToIndex(last);
        const auto numErased = lastIndex - firstIndex;

        destructElements(firstIndex, lastIndex);
        shiftElements(lastIndex, size(), -numErased);
        m_size -= numErased;
        updateVersion();
        return indexToIter(firstIndex);
    } else if (first == last) {
        return last.m_it;
    } else {
        return endIter();
    }
}

template<typename T, typename A, typename G>
inline void DynamicArray<T, A, G>::clear() {
    //! if (!isEmpty()) TODO
    destructElements(
        iteratorToIndex(firstConstIter()),
        iteratorToIndex(endConstIter())
    );
    m_size = 0;
}


template<typename T, typename A, typename G>
inline utils::bc::Reference<typename DynamicArray<T, A, G>::AllocatorType>
DynamicArray<T, A, G>::getAllocator() const noexcept { return m_storage.getAllocator(); }

template<typename T, typename A, typename G>
inline typename DynamicArray<T, A, G>::ReferenceType DynamicArray<T, A, G>::operator[](const IndexType index) {
    ASSERTION(index < size(), std::runtime_error, "Out of range")
    return atUnsafe(index);
}

template<typename T, typename A, typename G>
inline typename DynamicArray<T, A, G>::ConstReferenceType DynamicArray<T, A, G>::operator[](const IndexType index) const {
    ASSERTION(index < size(), std::runtime_error, "Out of range")
    return atUnsafe(index);
}

template<typename T, typename A, typename G>
inline typename DynamicArray<T, A, G>::ReferenceType DynamicArray<T, A, G>::atUnsafe(const IndexType index) noexcept {
    return *(reinterpret_cast<PointerType>(m_storage.data()) + index);
}

template<typename T, typename A, typename G>
inline typename DynamicArray<T, A, G>::ConstReferenceType DynamicArray<T, A, G>::atUnsafe(const IndexType index) const noexcept {
    return *(reinterpret_cast<ConstPointerType>(m_storage.data()) + index);
}

template<typename T, typename A, typename G>
inline typename DynamicArray<T, A, G>::SizeType DynamicArray<T, A, G>::size() const noexcept {
    return SizeType{m_size};
}

template<typename T, typename A, typename G>
inline typename DynamicArray<T, A, G>::SizeType DynamicArray<T, A, G>::capacity() const noexcept {
    return SizeType{m_capacity};
}

template<typename T, typename A, typename G>
inline bool DynamicArray<T, A, G>::isEmpty() const noexcept {
    return m_size == 0;
}

template<typename T, typename A, typename G>
typename DynamicArray<T, A, G>::ConstReferenceType DynamicArray<T, A, G>::firstElement() const {
    return *firstConstIter();
}

template<typename T, typename A, typename G>
typename DynamicArray<T, A, G>::ConstReferenceType DynamicArray<T, A, G>::lastElement() const {
    return *lastConstIter();
}

template<typename T, typename A, typename G>
typename DynamicArray<T, A, G>::ReferenceType DynamicArray<T, A, G>::firstElement() {
    return *firstIter();
}

template<typename T, typename A, typename G>
typename DynamicArray<T, A, G>::ReferenceType DynamicArray<T, A, G>::lastElement() {
    return *lastIter();
}

template<typename T, typename A, typename G>
typename DynamicArray<T, A, G>::ConstIterator DynamicArray<T, A, G>::firstConstIter() const noexcept {
    // `const_cast` in this context is safe because noone doesn`t mutate `this`!
    return ConstIterator{const_cast<DynamicArray<T, A, G>*>(this)->firstIter()};
}

template<typename T, typename A, typename G>
typename DynamicArray<T, A, G>::ConstIterator DynamicArray<T, A, G>::lastConstIter() const noexcept {
    // `const_cast` in this context is safe because noone doesn`t mutate `this`!
    return ConstIterator{const_cast<DynamicArray<T, A, G>*>(this)->lastIter()};
}

template<typename T, typename A, typename G>
typename DynamicArray<T, A, G>::ConstIterator DynamicArray<T, A, G>::endConstIter() const noexcept {
    // `const_cast` in this context is safe because noone doesn`t mutate `this`!
    return ConstIterator{const_cast<DynamicArray<T, A, G>*>(this)->endIter()};
}

template<typename T, typename A, typename G>
typename DynamicArray<T, A, G>::Iterator DynamicArray<T, A, G>::firstIter() noexcept {
    return Iterator{reinterpret_cast<ElementType*>(m_storage.data()), this, m_version};
}

template<typename T, typename A, typename G>
typename DynamicArray<T, A, G>::Iterator DynamicArray<T, A, G>::lastIter() noexcept {
    if (size() > 1) [[unlikely]] {
        return Iterator{reinterpret_cast<ElementType*>(m_storage.data()) + size() - 1, this, m_version};
    } else {
        return firstIter();
    }
}

template<typename T, typename A, typename G>
typename DynamicArray<T, A, G>::Iterator DynamicArray<T, A, G>::endIter() noexcept {
    return Iterator{reinterpret_cast<ElementType*>(m_storage.data()) + size(), this, m_version};
}

template<typename T, typename A, typename G>
typename DynamicArray<T, A, G>::ConstReverseIterator DynamicArray<T, A, G>::firstConstReserveIter() const noexcept {
    // `const_cast` in this context is safe because noone doesn`t mutate `this`!
    return ConstReverseIterator{const_cast<DynamicArray<T, A, G>*>(this)->firstIter()};
}

template<typename T, typename A, typename G>
typename DynamicArray<T, A, G>::ConstReverseIterator DynamicArray<T, A, G>::lastConstReserveIter() const noexcept {
    // `const_cast` in this context is safe because noone doesn`t mutate `this`!
    return ConstReverseIterator{const_cast<DynamicArray<T, A, G>*>(this)->lastIter()};
}

template<typename T, typename A, typename G>
typename DynamicArray<T, A, G>::ConstReverseIterator DynamicArray<T, A, G>::endConstReserveIter() const noexcept {
    // `const_cast` in this context is safe because noone doesn`t mutate `this`!
    return ConstReverseIterator{const_cast<DynamicArray<T, A, G>*>(this)->endIter()};
}

template<typename T, typename A, typename G>
typename DynamicArray<T, A, G>::ReverseIterator DynamicArray<T, A, G>::firstReverseIter() noexcept {
    return ReverseIterator{lastIter()};
}

template<typename T, typename A, typename G>
typename DynamicArray<T, A, G>::ReverseIterator DynamicArray<T, A, G>::lastReverseIter() noexcept {
    return ReverseIterator{firstIter()};
}

template<typename T, typename A, typename G>
typename DynamicArray<T, A, G>::ReverseIterator DynamicArray<T, A, G>::endReverseIter() noexcept {
    return ReverseIterator{endIter()};
}

template<typename T, typename A, typename G>
template<typename ... Arg>
inline void DynamicArray<T, A, G>::construct(PointerType ptr, Arg&& ... arg) {
    assert(ptr);
    if constexpr (std::is_same_v<AllocatorType, memory::allocator::DefaultAllocator>) {
        memory::allocator::DefaultAllocator{}.construct(ptr, std::forward<Arg>(arg) ... );
    } else {
        getAllocator().accessMutable([&](AllocatorType& allocator) {
            allocator.construct(ptr, std::forward<Arg>(arg) ... );
        });
    }
}

template<typename T, typename A, typename G>
inline void DynamicArray<T, A, G>::destruct(PointerType ptr) {
    assert(ptr);
    if constexpr (std::is_same_v<AllocatorType, memory::allocator::DefaultAllocator>) {
        memory::allocator::DefaultAllocator{}.destruct(ptr);
    } else {
        getAllocator().accessMutable([&](AllocatorType& allocator) {
            allocator.destruct(ptr);
        });
    }
}

template<typename T, typename A, typename G>
inline void DynamicArray<T, A, G>::destructElements(SizeType firstIndex, SizeType lastIndex) {
    if constexpr (!std::is_trivial_v<T>) {
        PointerType ptr = firstIter().m_ptr;
        for (auto it = firstIndex; it < lastIndex; ++it) {
            destruct(ptr + it);
        }
    }
}

template<typename T, typename A, typename G>
inline typename DynamicArray<T, A, G>::SizeType DynamicArray<T, A, G>::iteratorToIndex(ConstIterator it) {
    ASSERTION(it >= firstConstIter() && it <= endConstIter(), std::runtime_error, "Out of range")
    return (static_cast<SizeType>(it.m_it.m_ptr - firstConstIter().m_it.m_ptr));
}

template<typename T, typename A, typename G>
inline typename DynamicArray<T, A, G>::PointerType DynamicArray<T, A, G>::indexToPtr(const SizeType index) {
    ASSERTION(index < capacity(), std::runtime_error, "Out of range")
    return (reinterpret_cast<PointerType>(m_storage.data()) + index);
}

template<typename T, typename A, typename G>
inline typename DynamicArray<T, A, G>::Iterator DynamicArray<T, A, G>::indexToIter(const SizeType index) {
    ASSERTION(index < capacity(), std::runtime_error, "Out of range")
    return firstIter() + index;
}

template<typename T, typename A, typename G>
void DynamicArray<T, A, G>::shiftElements(const SizeType firstIndex, const SizeType lastIndex, const int offset) {
    assert(firstIndex <= lastIndex && offset != 0);
    assert(offset >= 0 || firstIndex >= -offset);
    assert(firstIndex + (offset > 0 ? offset : 0) <= m_capacity);

    if constexpr (std::is_trivial_v<T>) {
        PointerType src = (firstIter() + firstIndex).m_ptr;
        PointerType dst = src + offset;
        std::size_t count = lastIndex - firstIndex;
        std::memmove(dst, src, sizeof(T) * count);
    } else {
        PointerType first = (firstIter() + firstIndex).m_ptr;
        PointerType last = (firstIter() + lastIndex - 1).m_ptr;
        if (offset > 0) {
            for (PointerType it = last - 1; it >= first; --it) {
                construct(it + offset, std::move_if_noexcept(*it));
                destruct(it);
            }
        } else {
            for (PointerType it = first; it < last; ++it) {
                construct(it + offset, std::move_if_noexcept(*it));
                destruct(it);
            }
        }
    }
}

template<typename T, typename A, typename G>
template<typename ... Arg>
typename DynamicArray<T, A, G>::Iterator DynamicArray<T, A, G>::emplaceElement(const SizeType index, Arg&& ... arg) {
    assert(index < capacity());
    shiftElements(index, m_size, 1);
    construct(indexToPtr(index), std::forward<Arg>(arg) ... );
    ++m_size;
    updateVersion();
    return lastIter();
}

template<typename T, typename A, typename G>
void DynamicArray<T, A, G>::copy(const DynamicArray& other) {
    reserve(other.size());
    if constexpr (std::is_trivial_v<T>) {
        std::memcpy(m_storage.data(), other.m_storage.data(), other.m_size * sizeof(T));
        m_size = other.m_size;
    } else {
        for (std::size_t i = 0; i < other.m_size; ++i) {
            pushBack(other.atUnsafe(i));
        }
    }
    updateVersion();
}

template<typename T, typename A, typename G>
void DynamicArray<T, A, G>::swap(DynamicArray& other) noexcept {
    m_storage.swap(other.m_storage);
    std::swap(m_size, other.m_size);
    std::swap(m_capacity, other.m_capacity);
    updateVersion();
    other.updateVersion();
}

template<typename T, typename A, typename G>
void DynamicArray<T, A, G>::increaseArray(const SizeType newSize) {
    if (size() == 0 || (std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T>)) {
        reserve(newSize);
        PointerType ptr = firstIter().m_ptr;
        for (SizeType i = size(); i < newSize; ++i) {
            construct(ptr + i, T{});
        }
        m_size = newSize;
    } else {
        DynamicArray newArray{newSize, getAllocator()};
        newArray = *this;
        do {
            newArray.pushBack(T{});
        } while (newArray.size() < newSize);
        // --------------------- Kalb Line --------------------- //
        swap(newArray);
    }
    updateVersion();
}

template<typename T, typename A, typename G>
void DynamicArray<T, A, G>::decreaseArray(const SizeType newSize) {
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

template<typename T, typename A, typename G>
inline void DynamicArray<T, A, G>::updateVersion() noexcept {
    if (m_version < VERSION_LIMIT) {
        ++m_version;
    } else {
        m_version = FIRST_VALID_ALLOCATION_VERSION;
    }
}

template<typename A>
void DefaultDynamicArrayGrowthPolicyType<A>::operator()(
        std::size_t& capacity,
        utils::bc::Reference<A> /*allocator*/
) const noexcept {
    if (capacity == 0) [[unlikely]] {
        capacity = 1;
        return;
    }
    capacity *= 2;
}

} //! namespace atom::containers

#endif //! ATOM_DYNAMIC_ARRAY_H
