#ifndef ATOM_STATIC_SATCK_H
#define ATOM_STATIC_SATCK_H

#include "include/sfinae/has_class_method.h"
#include "include/utils/compiler_attr.h"
#include "include/containers/fixed/array.h"

#include <initializer_list>
#include <cstddef>

namespace atom::containers::fixed {

namespace __fixed_stack_impl_details {

DECLARE_HAS_CLASS_METHOD(emplaceBack)
DECLARE_HAS_CLASS_METHOD(pushBack)
DECLARE_HAS_CLASS_METHOD(popBack)
DECLARE_HAS_CLASS_METHOD(clear)
DECLARE_HAS_CLASS_METHOD(size)
DECLARE_HAS_CLASS_METHOD(capacity)
DECLARE_HAS_CLASS_METHOD(isEmpty)
DECLARE_HAS_CLASS_METHOD(isFull)
DECLARE_HAS_CLASS_METHOD(lastElement)
DECLARE_HAS_CLASS_METHOD(data)

} //! namespace __fixed_stack_impl_details

template<typename T, std::size_t N, typename C = Array<T, N>>
class Stack final {
public:
    using ElementType = T;
    using ReferenceType = ElementType&;
    using ConstReferenceType = const ElementType&;
    using PointerType = ElementType*;
    using ConstPointerType = const ElementType*;
    using SizeType = std::size_t;
    using ContainerType = C;

    ConstReferenceType top() const && = delete;
    ReferenceType top() && = delete;

    FORCE_INLINE explicit constexpr Stack() = default;
    FORCE_INLINE explicit Stack(const T& value, SizeType count): m_container(value, count) {}
    FORCE_INLINE explicit Stack(std::initializer_list<const T> data): m_container(data) {}
    FORCE_INLINE Stack(const Stack& other): m_container(other.m_container) {}
    FORCE_INLINE Stack& operator=(const Stack& other) {
        clear();
        m_container = other.m_container;
        return *this;
    }
    FORCE_INLINE Stack(Stack&& other): m_container(std::move(other.m_container)) {}
    FORCE_INLINE Stack& operator=(Stack&& other) {
        clear();
        m_container = std::move(other.m_container);
        return *this;
    }
    FORCE_INLINE ~Stack() { clear(); }

    template<typename ... Arg>
    FORCE_INLINE void emplace(Arg&& ... arg) {
        m_container.emplaceBack(std::forward<Arg>(arg) ... );
    }
    FORCE_INLINE void push(const T& value) {
        using namespace __fixed_stack_impl_details;
        static_assert(HAS_CLASS_METHOD_pushBack<ContainerType, const T&>::value,
                      "fixed::Stack::ContainerType must have the 'pushBack' method");
        m_container.pushBack(value);
    }
    FORCE_INLINE void push(T&& value) {
        using namespace __fixed_stack_impl_details;
        static_assert(HAS_CLASS_METHOD_pushBack<ContainerType, T&&>::value,
                      "fixed::Stack::ContainerType must have the 'pushBack' method");
        m_container.pushBack(std::move(value));
    }

    FORCE_INLINE void pop() {
        using namespace __fixed_stack_impl_details;
        static_assert(HAS_CLASS_METHOD_popBack<ContainerType>::value,
                      "fixed::Stack::ContainerType must have the 'popBack' method");
        m_container.popBack();
    }

    FORCE_INLINE void clear() {
        using namespace __fixed_stack_impl_details;
        static_assert(HAS_CLASS_METHOD_clear<ContainerType>::value,
                      "fixed::Stack::ContainerType must have the 'clear' method");
        m_container.clear();
    }

    NODISCARD FORCE_INLINE constexpr ConstReferenceType top() const & {
        using namespace __fixed_stack_impl_details;
        static_assert(HAS_CLASS_METHOD_lastElement<ContainerType>::value,
                      "fixed::Stack::ContainerType must have the 'lastElement' method");
        return m_container.lastElement();
    }
    NODISCARD FORCE_INLINE constexpr ReferenceType top() & {
        using namespace __fixed_stack_impl_details;
        static_assert(HAS_CLASS_METHOD_lastElement<ContainerType>::value,
                      "fixed::Stack::ContainerType must have the 'lastElement' method");
        return m_container.lastElement();
    }

    FORCE_INLINE constexpr SizeType size() const noexcept {
        using namespace __fixed_stack_impl_details;
        static_assert(HAS_CLASS_METHOD_size<ContainerType>::value,
                      "fixed::Stack::ContainerType must have the 'size' method");
        return m_container.size();
    }
    FORCE_INLINE constexpr SizeType capacity() const noexcept {
        using namespace __fixed_stack_impl_details;
        static_assert(HAS_CLASS_METHOD_capacity<ContainerType>::value,
                      "fixed::Stack::ContainerType must have the 'capacity' method");
        return m_container.capacity();
    }
    FORCE_INLINE constexpr bool isEmpty() const noexcept {
        using namespace __fixed_stack_impl_details;
        static_assert(HAS_CLASS_METHOD_isEmpty<ContainerType>::value,
                      "fixed::Stack::ContainerType must have the 'isEmpty' method");
        return m_container.isEmpty();
    }
    FORCE_INLINE constexpr bool isFull() const noexcept {
        using namespace __fixed_stack_impl_details;
        static_assert(HAS_CLASS_METHOD_isFull<ContainerType>::value,
                      "fixed::Stack::ContainerType must have the 'isFull' method");
        return m_container.isFull();
    }

    FORCE_INLINE PointerType data() noexcept {
        using namespace __fixed_stack_impl_details;
        static_assert(HAS_CLASS_METHOD_data<ContainerType>::value,
                      "fixed::Stack::ContainerType must have the 'data' method");
        return m_container.data();
    }
    FORCE_INLINE ConstPointerType data() const noexcept {
        using namespace __fixed_stack_impl_details;
        static_assert(HAS_CLASS_METHOD_data<ContainerType>::value,
                      "fixed::Stack::ContainerType must have the 'data' method");
        return m_container.data();
    }

private:
    ContainerType m_container{};
};

} //! namespace atom::containers::fixed

#endif //! ATOM_STATIC_SATCK_H
