#ifndef ATOM_STATIC_QUEUE_H
#define ATOM_STATIC_QUEUE_H

#include "include/sfinae/has_class_method.h"
#include "include/utils/compiler_attr.h"
#include "include/containers/fixed/array.h"

#include <initializer_list>
#include <cstddef>

namespace atom::containers::fixed {

namespace __fixed_queue_impl_details {

DECLARE_HAS_CLASS_METHOD(emplaceBack)
DECLARE_HAS_CLASS_METHOD(pushBack)
DECLARE_HAS_CLASS_METHOD(popFront)
DECLARE_HAS_CLASS_METHOD(clear)
DECLARE_HAS_CLASS_METHOD(size)
DECLARE_HAS_CLASS_METHOD(capacity)
DECLARE_HAS_CLASS_METHOD(isEmpty)
DECLARE_HAS_CLASS_METHOD(isFull)
DECLARE_HAS_CLASS_METHOD(frontElement)
DECLARE_HAS_CLASS_METHOD(backElement)
DECLARE_HAS_CLASS_METHOD(data)

} //! namespace __fixed_queue_impl_details

template<typename T, std::size_t N, typename C = Array<T, N>>
class Queue final {
public:
    using ElementType = T;
    using ReferenceType = ElementType&;
    using ConstReferenceType = const ElementType&;
    using PointerType = ElementType*;
    using ConstPointerType = const ElementType*;
    using SizeType = std::size_t;
    using ContainerType = C;

    ConstReferenceType front() const && = delete;
    ReferenceType front() && = delete;
    ConstReferenceType back() const && = delete;
    ReferenceType back() && = delete;

    FORCE_INLINE explicit constexpr Queue() = default;
    FORCE_INLINE explicit Queue(const T& value, SizeType count): m_container(value, count) {}
    FORCE_INLINE explicit Queue(std::initializer_list<const T> data): m_container(data) {}
    FORCE_INLINE Queue(const Queue& other): m_container(other.m_container) {}
    FORCE_INLINE Queue& operator=(const Queue& other) {
        clear();
        m_container = other.m_container;
        return *this;
    }
    FORCE_INLINE Queue(Queue&& other): m_container(std::move(other.m_container)) {}
    FORCE_INLINE Queue& operator=(Queue&& other) {
        clear();
        m_container = std::move(other.m_container);
        return *this;
    }
    FORCE_INLINE ~Queue() { clear(); }

    template<typename ... Arg>
    FORCE_INLINE void emplace(Arg&& ... arg) {
        m_container.emplaceBack(std::forward<Arg>(arg) ... );
    }

    FORCE_INLINE void push(const T& value) {
        using namespace __fixed_queue_impl_details;
        static_assert(HAS_CLASS_METHOD_pushBack<ContainerType, const T&>::value,
                      "fixed::Queue::ContainerType must have the 'pushBack' method");
        m_container.pushBack(value);
    }

    FORCE_INLINE void push(T&& value) {
        using namespace __fixed_queue_impl_details;
        static_assert(HAS_CLASS_METHOD_pushBack<ContainerType, T&&>::value,
                      "fixed::Queue::ContainerType must have the 'pushBack' method");
        m_container.pushBack(std::move(value));
    }

    FORCE_INLINE void pop() {
        using namespace __fixed_queue_impl_details;
        static_assert(HAS_CLASS_METHOD_popFront<ContainerType>::value,
                      "fixed::Queue::ContainerType must have the 'popFront' method");
        m_container.popFront();
    }

    FORCE_INLINE void clear() {
        using namespace __fixed_queue_impl_details;
        static_assert(HAS_CLASS_METHOD_clear<ContainerType>::value,
                      "fixed::Queue::ContainerType must have the 'clear' method");
        m_container.clear();
    }

    NODISCARD FORCE_INLINE constexpr ConstReferenceType front() const & {
        using namespace __fixed_queue_impl_details;
        static_assert(HAS_CLASS_METHOD_frontElement<ContainerType>::value,
                      "fixed::Queue::ContainerType must have the 'frontElement' method");
        return m_container.frontElement();
    }

    NODISCARD FORCE_INLINE constexpr ReferenceType front() & {
        using namespace __fixed_queue_impl_details;
        static_assert(HAS_CLASS_METHOD_frontElement<ContainerType>::value,
                      "fixed::Queue::ContainerType must have the 'frontElement' method");
        return m_container.frontElement();
    }

    NODISCARD FORCE_INLINE constexpr ConstReferenceType back() const & {
        using namespace __fixed_queue_impl_details;
        static_assert(HAS_CLASS_METHOD_backElement<ContainerType>::value,
                      "fixed::Queue::ContainerType must have the 'backElement' method");
        return m_container.backElement();
    }

    NODISCARD FORCE_INLINE constexpr ReferenceType back() & {
        using namespace __fixed_queue_impl_details;
        static_assert(HAS_CLASS_METHOD_backElement<ContainerType>::value,
                      "fixed::Queue::ContainerType must have the 'backElement' method");
        return m_container.backElement();
    }

    FORCE_INLINE constexpr SizeType size() const noexcept {
        using namespace __fixed_queue_impl_details;
        static_assert(HAS_CLASS_METHOD_size<ContainerType>::value,
                      "fixed::Queue::ContainerType must have the 'size' method");
        return m_container.size();
    }

    FORCE_INLINE constexpr SizeType capacity() const noexcept {
        using namespace __fixed_queue_impl_details;
        static_assert(HAS_CLASS_METHOD_capacity<ContainerType>::value,
                      "fixed::Queue::ContainerType must have the 'capacity' method");
        return m_container.capacity();
    }

    FORCE_INLINE constexpr bool isEmpty() const noexcept {
        using namespace __fixed_queue_impl_details;
        static_assert(HAS_CLASS_METHOD_isEmpty<ContainerType>::value,
                      "fixed::Queue::ContainerType must have the 'isEmpty' method");
        return m_container.isEmpty();
    }

    FORCE_INLINE constexpr bool isFull() const noexcept {
        using namespace __fixed_queue_impl_details;
        static_assert(HAS_CLASS_METHOD_isFull<ContainerType>::value,
                      "fixed::Queue::ContainerType must have the 'isFull' method");
        return m_container.isFull();
    }

    FORCE_INLINE PointerType data() noexcept {
        using namespace __fixed_queue_impl_details;
        static_assert(HAS_CLASS_METHOD_data<ContainerType>::value,
                      "fixed::Queue::ContainerType must have the 'data' method");
        return m_container.data();
    }

    FORCE_INLINE ConstPointerType data() const noexcept {
        using namespace __fixed_queue_impl_details;
        static_assert(HAS_CLASS_METHOD_data<ContainerType>::value,
                      "fixed::Queue::ContainerType must have the 'data' method");
        return m_container.data();
    }

private:
    ContainerType m_container{};
};

} //! namespace atom::containers::fixed

#endif //! ATOM_STATIC_QUEUE_H
