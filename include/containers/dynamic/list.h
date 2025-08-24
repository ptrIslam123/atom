#ifndef ATOM_CONTAINERS_DYNAMIC_LIST_H
#define ATOM_CONTAINERS_DYNAMIC_LIST_H

#include "include/memory/allocators/default_allocator.h"
#include "include/utils/assertion.h"
#include "include/utils/scoped_lock.h"
#include "include/utils/compiler_attr.h"

#include <initializer_list>
#include <stdexcept>
#include <cstddef>
#include <cassert>

namespace atom::containers::dynamic {

template<typename T, typename A = memory::allocator::DefaultAllocator>
class List final : private A {
    struct Node;
public:
    using ElementType = T;
    using ReferenceType = ElementType&;
    using ConstReferenceType = const ElementType&;
    using PointerType = ElementType*;
    using ConstPointerType = const ElementType*;
    using SizeType = std::size_t;
    using AllocatorType = A;

    class Iterator final {
    public:
        Iterator(const Iterator& other) noexcept = default;
        Iterator(Iterator&& other) noexcept = default;
        Iterator& operator=(const Iterator& other) noexcept = default;
        Iterator& operator=(Iterator&& other) noexcept = default;

        ConstReferenceType operator*() const {
            ASSERTION(m_node, std::runtime_error, "Try to deref null iterator")
            return m_node->value;
        }

        ConstPointerType operator->() const {
            ASSERTION(m_node, std::runtime_error, "Try to deref null iterator")
            return &m_node->value;
        }

        ReferenceType operator*() {
            ASSERTION(m_node, std::runtime_error, "Try to deref null iterator")
            return m_node->value;
        }

        PointerType operator->() {
            ASSERTION(m_node, std::runtime_error, "Try to deref null iterator")
            return &m_node->value;
        }

        Iterator& operator++() {
            ASSERTION(m_node, std::runtime_error, "Try to deref null iterator")
            m_node = m_node->next;
            return *this;
        }

        Iterator operator++(int) {
            ASSERTION(m_node, std::runtime_error, "Try to deref null iterator")
            Iterator temp = *this;
            m_node = m_node->next;
            return temp;
        }

        Iterator& operator--() {
            ASSERTION(m_node, std::runtime_error, "Try to deref null iterator")
            m_node = m_node->prev;
            return *this;
        }

        Iterator operator--(int) {
            ASSERTION(m_node, std::runtime_error, "Try to deref null iterator")
            Iterator temp = *this;
            m_node = m_node->prev;
            return temp;
        }

        bool operator==(const Iterator& other) const noexcept {
            return m_node == other.m_node;
        }

        bool operator!=(const Iterator& other) const noexcept {
            return m_node != other.m_node;
        }

    private:
        friend List;
        Iterator(Node* node):
        m_node(node) {}
        Node* m_node{nullptr};
    };

    class ConstIterator final {
    public:
        ConstIterator(const Iterator& it): m_node(it.m_node) {}
        ConstIterator(const ConstIterator& other) noexcept = default;
        ConstIterator(ConstIterator&& other) noexcept = default;
        ConstIterator& operator=(const ConstIterator& other) noexcept = default;
        ConstIterator& operator=(ConstIterator&& other) noexcept = default;

        ConstReferenceType operator*() const {
            ASSERTION(m_node, std::runtime_error, "Try to deref null iterator")
            return m_node->value;
        }

        ConstPointerType operator->() const  {
            ASSERTION(m_node, std::runtime_error, "Try to deref null iterator")
            return &m_node->value;
        }

        ConstIterator& operator++() {
            ASSERTION(m_node, std::runtime_error, "Try to deref null iterator")
            m_node = m_node->next;
            return *this;
        }

        ConstIterator operator++(int) {
            ASSERTION(m_node, std::runtime_error, "Try to deref null iterator")
            ConstIterator temp = *this;
            m_node = m_node->next;
            return temp;
        }

        ConstIterator& operator--() {
            ASSERTION(m_node, std::runtime_error, "Try to deref null iterator")
            m_node = m_node->prev;
            return *this;
        }

        ConstIterator operator--(int) {
            ASSERTION(m_node, std::runtime_error, "Try to deref null iterator")
            ConstIterator temp = *this;
            m_node = m_node->prev;
            return temp;
        }

        bool operator==(const ConstIterator& other) const noexcept {
            return m_node == other.m_node;
        }

        bool operator!=(const ConstIterator& other) const noexcept {
            return m_node != other.m_node;
        }

    private:
        friend List;
        ConstIterator(Node* node):
        m_node(node) {}
        Node* m_node{nullptr};
    };

    constexpr explicit List() = default;
    List(std::initializer_list<const T> data) { pushBack(data); }
    List(const List& other) { (void)operator=(other); }
    List(List&& other) noexcept { (void)operator=(std::move(other)); }
    List& operator=(const List& other) {
        clear();
        copy(other);
        return *this;
    }
    List& operator=(List&& other) noexcept {
        clear();
        swap(other);
        return *this;
    }
    ~List() { clear(); }

    template<typename ... Arg>
    FORCE_INLINE void emplaceBack(Arg&& ... arg) {
        (void)emplace(endConstIter(), std::forward<Arg>(arg) ... );
    }
    FORCE_INLINE void pushBack(const T& value) { (void)insert(endConstIter(), value); }
    FORCE_INLINE void pushBack(T&& value) { (void)insert(endConstIter(), std::move(value)); }
    void pushBack(std::initializer_list<const T> data) { (void)insert(endConstIter(), data); }
    template<typename ... Arg>
    FORCE_INLINE void emplaceFront(Arg&& ... arg) {
        (void)emplace(firstConstIter(), std::forward<Arg>(arg) ... );
    }
    FORCE_INLINE void pushFront(const T& value) { (void)insert(firstConstIter(), value); }
    FORCE_INLINE void pushFront(T&& value) { (void)insert(firstConstIter(), std::move(value)); }

    template<typename ... Arg>
    Iterator emplace(ConstIterator pos, Arg&& ... arg) {}
    FORCE_INLINE Iterator insert(ConstIterator pos, const T& value) {
        return insertNode(iterToNode(pos), allocateNode(value));
    }
    FORCE_INLINE Iterator insert(ConstIterator pos, T&& value) {
        return insertNode(iterToNode(pos), allocateNode(std::move(value)));
    }
    FORCE_INLINE Iterator insert(ConstIterator pos, const T& value, SizeType count) {
        return insertNodes(iterToNode(pos), value, count);
    }
    FORCE_INLINE Iterator insert(ConstIterator pos, const std::initializer_list<const T>& data) {
        return insertNodes(iterToNode(pos), data);
    }

    FORCE_INLINE void popBack() { erase(lastConstIter()); }
    FORCE_INLINE void popFront() { erase(firstConstIter()); }

    Iterator erase(ConstIterator pos) {}
    Iterator erase(ConstIterator first, ConstIterator last) {}
    FORCE_INLINE void clear() { erase(firstConstIter(), endConstIter()); }

    Iterator firstIter() { return Iterator{m_head}; }
    Iterator lastIter() { return Iterator{m_tail}; }
    Iterator endIter() { return Iterator{nullptr}; }

    ConstIterator firstConstIter() const { return ConstIterator{m_head}; }
    ConstIterator lastConstIter() const { return ConstIterator{m_tail}; }
    ConstIterator endConstIter() const { return ConstIterator{nullptr}; }

    FORCE_INLINE ReferenceType firstElement() noexcept { return *firstIter(); }
    FORCE_INLINE ReferenceType lastElement() noexcept { return *lastIter(); }

    FORCE_INLINE ConstReferenceType firstElement() const noexcept { return *firstConstIter(); }
    FORCE_INLINE ConstReferenceType lastElement() const noexcept { return *lastConstIter(); }

    FORCE_INLINE SizeType size() const noexcept { return m_size; }
    FORCE_INLINE bool isEmpty() const noexcept { return size() == 0; }

private:
    struct Node {
        Node* prev{nullptr};
        Node* next{nullptr};
        T value{};

        template<typename ... Arg>
        Node(Arg&& ... arg):
        prev(nullptr),
        next(nullptr),
        value(std::forward<Arg>(arg) ... )
        {}
    };

    FORCE_INLINE Node* iterToNode(ConstIterator it) noexcept {
        // check it!
        return it.m_node;
    }

    Iterator insertNodes(Node* node, const T& data, SizeType count) {
        assert(false);
    }

    Iterator insertNodes(Node* node, const std::initializer_list<const T>& data) {
        assert(false);
    }

    Iterator insertNode(Node* node, Node* newNode) {
        if UNLIKELY_EXPR(isEmpty()) {
            // Пустой список
            m_head = m_tail = newNode;
            newNode->prev = nullptr;
            newNode->next = nullptr;
        } else if (node == m_head) {
            // Вставка перед головой
            newNode->prev = nullptr;
            newNode->next = m_head;
            m_head->prev = newNode;
            m_head = newNode;
        } else if (node == nullptr) {
            // Вставка в конец (после хвоста)
            newNode->prev = m_tail;
            newNode->next = nullptr;
            m_tail->next = newNode;
            m_tail = newNode;
        } else {
            // Вставка в середину
            newNode->prev = node->prev;
            newNode->next = node;
            node->prev->next = newNode;
            node->prev = newNode;
        }
        ++m_size;
        return Iterator(newNode);
    }

    template<typename ... Arg>
    Node* allocateNode(Arg&& ... arg) {
        auto node = reinterpret_cast<Node*>(AllocatorType::allocate(sizeof(Node)));
        assert(node);
        utils::ScopedLock guard{[this, node] {
            AllocatorType::deallocate(reinterpret_cast<std::byte*>(node));
        }};
        new(node) Node{std::forward<Arg>(arg) ... };
        guard.cancel();
        return node;
    }

    void deallocateNode(Node*& node) {
        utils::ScopedLock guard{[this, node] {
            AllocatorType::deallocate(reinterpret_cast<std::byte*>(node));
        }};
        node->value.~T();
        AllocatorType::deallocate(reinterpret_cast<std::byte*>(node));
        node = nullptr;
        guard.cancel();
    }

    void copy(const List& other) {}
    void swap(List&& other) noexcept {}

    SizeType m_size{0};
    Node* m_head{nullptr};
    Node* m_tail{nullptr};
};

} //! namespace atom::containers::dynamic

#endif //! ATOM_CONTAINERS_DYNAMIC_LIST_H
