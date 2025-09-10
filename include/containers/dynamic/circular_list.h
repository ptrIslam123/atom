#ifndef ATOM_CONTAINERS_DYNAMIC_CIRCULAR_LIST_H
#define ATOM_CONTAINERS_DYNAMIC_CIRCULAR_LIST_H

#include "include/memory/memory.h"
#include "include/memory/allocators/default_allocator.h"

#include "include/iterator/iterator_traits.h"
#include "include/iterator/iterator.h"

#include "include/utils/assertion.h"
#include "include/utils/scoped_guard.h"
#include "include/utils/compiler_attr.h"

#include <cstddef>
#include <initializer_list>

namespace atom::containers::dynamic {

class ListException : public std::exception {
public:
    ListException(const std::string& msg): m_msg(msg) {}
    virtual const char* what() const noexcept { return m_msg.data(); }

private:
    std::string m_msg;
};

template <typename T, typename A = memory::allocator::DefaultAllocator>
class CircularList : private A {
    struct Node;
    struct DataNode;
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
            ASSERTION(m_node && m_node != m_end, ListException, "Try to deref null iterator")
            return getDataNode()->data;
        }

        ConstPointerType operator->() const {
            ASSERTION(m_node && m_node != m_end, ListException, "Try to deref null iterator")
            return &getDataNode()->data;
        }

        ReferenceType operator*() {
            ASSERTION(m_node && m_node != m_end, ListException, "Try to deref null iterator")
            return getDataNode()->data;
        }

        PointerType operator->() {
            ASSERTION(m_node && m_node != m_end, ListException, "Try to deref null iterator")
            return &getDataNode()->data;
        }

        Iterator& operator++() {
            ASSERTION(m_node, ListException, "Try to deref null iterator")
            m_node = m_node->next;
            return *this;
        }

        Iterator operator++(int) {
            ASSERTION(m_node, ListException, "Try to deref null iterator")
            Iterator temp = *this;
            m_node = m_node->next;
            return temp;
        }

        Iterator& operator--() {
            ASSERTION(m_node, ListException, "Try to deref null iterator")
            m_node = m_node->prev;
            return *this;
        }

        Iterator operator--(int) {
            ASSERTION(m_node, ListException, "Try to deref null iterator")
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
        friend CircularList;
        Iterator(Node* node, Node* end):
            m_node(node),
            m_end(end)
        {}
        const DataNode* getDataNode() const { return static_cast<const DataNode*>(m_node); }
        DataNode* getDataNode() { return static_cast<DataNode*>(m_node); }

        Node* m_node{nullptr};
        Node* m_end{nullptr};
    };

    class ConstIterator final {
    public:
        ConstIterator(const Iterator& it) noexcept: m_node(it.m_node), m_end(it.m_end) {}
        ConstIterator(const ConstIterator& other) noexcept = default;
        ConstIterator(ConstIterator&& other) noexcept = default;
        ConstIterator& operator=(const ConstIterator& other) noexcept = default;
        ConstIterator& operator=(ConstIterator&& other) noexcept = default;

        ConstReferenceType operator*() const {
            ASSERTION(m_node && m_node != m_end, ListException, "Try to deref null iterator")
            return getDataNode()->data;
        }

        ConstPointerType operator->() const  {
            ASSERTION(m_node && m_node != m_end, ListException, "Try to deref null iterator")
            return &getDataNode()->data;
        }

        ConstIterator& operator++() {
            ASSERTION(m_node, ListException, "Try to deref null iterator")
            m_node = m_node->next;
            return *this;
        }

        ConstIterator operator++(int) {
            ASSERTION(m_node, ListException, "Try to deref null iterator")
            ConstIterator temp = *this;
            m_node = m_node->next;
            return temp;
        }

        ConstIterator& operator--() {
            ASSERTION(m_node, ListException, "Try to deref null iterator")
            m_node = m_node->prev;
            return *this;
        }

        ConstIterator operator--(int) {
            ASSERTION(m_node, ListException, "Try to deref null iterator")
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
        friend CircularList;
        ConstIterator(const Node * node, const Node* end):
            m_node(const_cast<Node*>(node)),
            m_end(const_cast<Node*>(end))
        {}
        const DataNode* getDataNode() const { return static_cast<const DataNode*>(m_node); }
        DataNode* getDataNode() { return static_cast<DataNode*>(m_node); }

        Node* m_node{nullptr};
        Node* m_end{nullptr};
    };

    class ReverseIterator final {
    public:
        ReverseIterator(const ReverseIterator& other) noexcept = default;
        ReverseIterator(ReverseIterator&& other) noexcept = default;
        ReverseIterator& operator=(const ReverseIterator& other) noexcept = default;
        ReverseIterator& operator=(ReverseIterator&& other) noexcept = default;

        ReferenceType operator*() {
            ASSERTION(m_node && m_node != m_end, ListException, "Try to deref null iterator")
            return getDataNode()->data;
        }

        PointerType operator->() {
            ASSERTION(m_node && m_node != m_end, ListException, "Try to deref null iterator")
            return &getDataNode()->data;
        }

        ConstReferenceType operator*() const {
            ASSERTION(m_node && m_node != m_end, ListException, "Try to deref null iterator")
            return getDataNode()->data;
        }

        ConstPointerType operator->() const {
            ASSERTION(m_node && m_node != m_end, ListException, "Try to deref null iterator")
            return &getDataNode()->data;
        }

        ReverseIterator& operator++() {
            ASSERTION(m_node, ListException, "Try to deref null iterator")
            m_node = m_node->prev;
            return *this;
        }

        ReverseIterator operator++(int) {
            ASSERTION(m_node, ListException, "Try to deref null iterator")
            ReverseIterator temp = *this;
            m_node = m_node->prev;
            return temp;
        }

        ReverseIterator& operator--() {
            ASSERTION(m_node, ListException, "Try to deref null iterator")
            m_node = m_node->next;
            return *this;
        }

        ReverseIterator operator--(int) {
            ASSERTION(m_node, ListException, "Try to deref null iterator")
            ReverseIterator temp = *this;
            m_node = m_node->next;
            return temp;
        }

        bool operator==(const ReverseIterator& other) const noexcept {
            return m_node == other.m_node;
        }

        bool operator!=(const ReverseIterator& other) const noexcept {
            return m_node != other.m_node;
        }

    private:
        friend CircularList;
        ReverseIterator(Node* node, Node* end):
            m_node(node),
            m_end(end)
        {}
        DataNode* getDataNode() { return static_cast<DataNode*>(m_node); }

        Node* m_node{nullptr};
        Node* m_end{nullptr};
    };

    class ReverseConstIterator final {
    public:
        ReverseConstIterator(const ReverseConstIterator& other) noexcept = default;
        ReverseConstIterator(ReverseConstIterator&& other) noexcept = default;
        ReverseConstIterator& operator=(const ReverseConstIterator& other) noexcept = default;
        ReverseConstIterator& operator=(ReverseConstIterator&& other) noexcept = default;

        ConstReferenceType operator*() const {
            ASSERTION(m_node && m_node != m_end, ListException, "Try to deref null iterator")
            return getDataNode()->data;
        }

        ConstPointerType operator->() const {
            ASSERTION(m_node && m_node != m_end, ListException, "Try to deref null iterator")
            return &getDataNode()->data;
        }

        ReverseConstIterator& operator++() {
            ASSERTION(m_node, ListException, "Try to deref null iterator")
            m_node = m_node->prev;
            return *this;
        }

        ReverseConstIterator operator++(int) {
            ASSERTION(m_node, ListException, "Try to deref null iterator")
            ReverseConstIterator temp = *this;
            m_node = m_node->prev;
            return temp;
        }

        ReverseConstIterator& operator--() {
            ASSERTION(m_node, ListException, "Try to deref null iterator")
            m_node = m_node->next;
            return *this;
        }

        ReverseConstIterator operator--(int) {
            ASSERTION(m_node, ListException, "Try to deref null iterator")
            ReverseConstIterator temp = *this;
            m_node = m_node->next;
            return temp;
        }

        bool operator==(const ReverseConstIterator& other) const noexcept {
            return m_node == other.m_node;
        }

        bool operator!=(const ReverseConstIterator& other) const noexcept {
            return m_node != other.m_node;
        }

    private:
        friend CircularList;
        ReverseConstIterator(const Node* node, const Node* end):
            m_node(const_cast<Node*>(node)),
            m_end(const_cast<Node*>(end))
        {}
        const DataNode* getDataNode() const { return static_cast<const DataNode*>(m_node); }
        Node* m_node{nullptr};
        Node* m_end{nullptr};
    };

    constexpr explicit CircularList() { setUpCircularList(); }
    template<typename InputIterator>
    CircularList(InputIterator first, InputIterator last) {
        setUpCircularList();
        (void)insert(endConstIter(), first, last);
    }
    explicit CircularList(const T& value, SizeType count) {
        setUpCircularList();
        (void)insert(endConstIter(), value, count);
    }
    explicit CircularList(std::initializer_list<T> data) {
        setUpCircularList();
        (void)insert(endConstIter(), data.begin(), data.end());
    }
    CircularList(const CircularList& other) {
        setUpCircularList();
        (void)operator=(other);
    }
    CircularList(CircularList&& other) noexcept {
        setUpCircularList();
        (void)operator=(std::move(other));
    }
    CircularList& operator=(const CircularList& other) {
        setUpCircularList();
        clear();
        copy(other);
        return *this;
    }
    CircularList& operator=(CircularList&& other) noexcept {
        setUpCircularList();
        clear();
        swap(other);
        return *this;
    }
    ~CircularList() { clear(); }

    template<typename ... Arg>
    FORCE_INLINE void emplaceBack(Arg&& ... arg) {
        (void)emplace(endConstIter(), std::forward<Arg>(arg) ... );
    }
    FORCE_INLINE void pushBack(const T& value) { (void)insert(endConstIter(), value); }
    FORCE_INLINE void pushBack(T&& value) { (void)insert(endConstIter(), std::move(value)); }
    FORCE_INLINE void pushBack(const T& value, SizeType count) {
        (void)insert(endConstIter(), value, count);
    }
    FORCE_INLINE void pushBack(std::initializer_list<T> data) {
        (void)insert(endConstIter(), data.begin(), data.end());
    }
    template<typename ... Arg>
    FORCE_INLINE void emplaceFront(Arg&& ... arg) {
        (void)emplace(firstConstIter(), std::forward<Arg>(arg) ... );
    }
    FORCE_INLINE void pushFront(const T& value) { (void)insert(firstConstIter(), value); }
    FORCE_INLINE void pushFront(T&& value) { (void)insert(firstConstIter(), std::move(value)); }
    FORCE_INLINE void pushFront(const T& value, SizeType count) {
        (void)insert(firstConstIter(), value, count);
    }
    FORCE_INLINE void pushFront(std::initializer_list<T> data) {
        (void)insert(firstConstIter(), data.begin(), data.end());
    }

    template<typename ... Arg>
    FORCE_INLINE Iterator emplace(ConstIterator pos, Arg&& ... arg) {
        return insertNode(iterToNode(pos), allocateNode(std::forward<Arg>(arg) ... ));
    }
    template<typename InputIterator>
    FORCE_INLINE Iterator insert(ConstIterator pos, InputIterator first, InputIterator last) {
        return insertNodes(iterToNode(pos), first, last);
    }
    FORCE_INLINE Iterator insert(ConstIterator pos, const T& value) {
        return insertNode(iterToNode(pos), allocateNode(value));
    }
    FORCE_INLINE Iterator insert(ConstIterator pos, T&& value) {
        return insertNode(iterToNode(pos), allocateNode(std::move(value)));
    }
    FORCE_INLINE Iterator insert(ConstIterator pos, const T& value, SizeType count) {
        return insertNodes(iterToNode(pos), value, count);
    }
    FORCE_INLINE Iterator insert(ConstIterator pos, std::initializer_list<T> data) {
        return insert(pos, data.begin(), data.end());
    }

    FORCE_INLINE void popBack() { (void)erase(lastConstIter()); }
    FORCE_INLINE void popFront() { (void)erase(firstConstIter()); }

    FORCE_INLINE Iterator erase(ConstIterator pos) {
        return eraseNode(iterToNode(pos));
    }
    FORCE_INLINE Iterator erase(ConstIterator first, ConstIterator last) {
        return eraseNodes(iterToNode(first), iterToNode(last));
    }

    FORCE_INLINE void clear() {
        if LIKELY_EXPR(!isEmpty()) {
            (void)erase(firstConstIter(), endConstIter());
        }
    }

    FORCE_INLINE Iterator firstIter() noexcept {
        return nodeToIter(m_start.next);
    }
    FORCE_INLINE Iterator lastIter() noexcept {
        return nodeToIter(m_start.prev);
    }
    FORCE_INLINE Iterator endIter() noexcept {
        return nodeToIter(&m_start);
    }

    FORCE_INLINE ReverseIterator firstReverseIter() noexcept {
        return nodeToReverseIter(m_start.prev);
    }
    FORCE_INLINE ReverseIterator lastReverseIter() noexcept {
        return nodeToReverseIter(m_start.next);
    }
    FORCE_INLINE ReverseIterator endReverseIter() noexcept {
        return nodeToReverseIter(&m_start);
    }

    FORCE_INLINE ConstIterator firstConstIter() const noexcept {
        return nodeToIter(m_start.next);
    }
    FORCE_INLINE ConstIterator lastConstIter() const noexcept {
        return nodeToIter(m_start.prev);
    }
    FORCE_INLINE ConstIterator endConstIter() const noexcept {
        return nodeToIter(&m_start);
    }

    FORCE_INLINE ReverseConstIterator firstReverseConstIter() const noexcept {
        return nodeToReverseIter(m_start.prev);
    }
    FORCE_INLINE ReverseConstIterator lastReverseConstIter() const noexcept {
        return nodeToReverseIter(m_start.next);
    }
    FORCE_INLINE ReverseConstIterator endReverseConstIter() const noexcept {
        return nodeToReverseIter(&m_start);
    }

    FORCE_INLINE ReferenceType firstElement() { return *firstIter(); }
    FORCE_INLINE ReferenceType lastElement() { return *lastIter(); }

    FORCE_INLINE ConstReferenceType firstElement() const { return *firstConstIter(); }
    FORCE_INLINE ConstReferenceType lastElement() const { return *lastConstIter(); }

    FORCE_INLINE SizeType size() const noexcept { return m_size; }
    FORCE_INLINE bool isEmpty() const noexcept { return size() == 0; }

    FORCE_INLINE const AllocatorType& getAllocator() const noexcept {
        return *static_cast<const AllocatorType*>(this);
    }
    FORCE_INLINE AllocatorType& getAllocator() noexcept {
        return *static_cast<AllocatorType*>(this);
    }

private:
    struct Node {
        explicit Node() = default;
        virtual ~Node() = default;

        Node* prev{nullptr};
        Node* next{nullptr};
    };
    struct DataNode final : Node {
        template<typename ... Arg>
        explicit DataNode(Arg&& ... arg):
            Node(), data(std::forward<Arg>(arg) ... )
        {}
        alignas(T) T data;
    };

    FORCE_INLINE Node* iterToNode(ConstIterator it) noexcept {
        assert(it.m_node);
        return it.m_node;
    }

    FORCE_INLINE ConstIterator nodeToIter(const Node* node) const noexcept {
        assert(node);
        return ConstIterator{node, &m_start};
    }

    FORCE_INLINE Iterator nodeToIter(Node* node) noexcept {
        assert(node);
        return Iterator{node, &m_start};
    }

    FORCE_INLINE ReverseIterator nodeToReverseIter(Node* node) noexcept {
        assert(node);
        return ReverseIterator{node, &m_start};
    }

    FORCE_INLINE ReverseConstIterator nodeToReverseIter(const Node* node) const noexcept {
        assert(node);
        return ReverseConstIterator{node, &m_start};
    }

    void copy(const CircularList& other) {
        CircularList copy{};
        for (auto it = other.firstConstIter(); it != other.endConstIter(); ++it) {
            copy.pushBack(*it);
        }
        swap(copy);
    }
    void swap(CircularList& other) noexcept {
        std::swap(m_size, other.m_size);

        auto thisNext = m_start.next;
        auto thisPrev = m_start.prev;
        auto otherNext = other.m_start.next;
        auto otherPrev = other.m_start.prev;

        m_start.next = otherNext;
        m_start.prev = otherPrev;
        if (otherNext != &other.m_start) {
            otherNext->prev = &m_start;
            otherPrev->next = &m_start;
        }

        other.m_start.next = thisNext;
        other.m_start.prev = thisPrev;
        if (thisNext != &m_start) {
            thisNext->prev = &other.m_start;
            thisPrev->next = &other.m_start;
        }
    }

    template<typename ... Arg>
    Node* allocateNode(Arg&& ... arg) {
        auto node = reinterpret_cast<DataNode*>(AllocatorType::allocate(sizeof(DataNode)));
        utils::ScopedGuard guard{[this, &node] {
            if (node)
                AllocatorType::deallocate(reinterpret_cast<std::byte*>(node), sizeof(DataNode));
        }};
        memory::Construct(node, std::forward<Arg>(arg) ... );
        guard.cancel();
        return node;
    }
    void deallocateNode(Node* node) {
        utils::ScopedGuard guard{[this, &node] {
            if (node)
                AllocatorType::deallocate(reinterpret_cast<std::byte*>(node), sizeof(DataNode));
        }};
        memory::Destruct(node);
    }

    Iterator insertNode(Node* node, Node *const newNode) noexcept {
        Node *const prevNode = node->prev;

        newNode->next = node;
        node->prev = newNode;

        newNode->prev = prevNode;
        prevNode->next = newNode;

        ++m_size;
        return nodeToIter(newNode);
    }

    template<typename InputIterator>
    Iterator insertNodes(Node* node, InputIterator first, InputIterator last) {
        assert(node);
        auto currentNode = node;
        while (first != last) {
            auto newNode = allocateNode(*first);
            insertNode(currentNode, newNode);
            currentNode = newNode->next;
            ++first;
        }
        return nodeToIter(node);
    }

    Iterator insertNodes(Node* node, const T& value, SizeType count) {
        assert(node);
        auto currentNode = node;
        while (count > 0) {
            auto newNode = allocateNode(value);
            insertNode(currentNode, newNode);
            currentNode = newNode->next;
            --count;
        }
        return nodeToIter(node);
    }

    Iterator eraseNode(Node* node) {
        auto prevNode = node->prev;
        auto nextNode = node->next;

        prevNode->next = nextNode;
        nextNode->prev = prevNode;

        deallocateNode(node);
        --m_size;
        return nodeToIter(nextNode);
    }

    Iterator eraseNodes(Node* first, Node* last) {
        while (first != last) {
            first = iterToNode(eraseNode(first));
        }
        return nodeToIter(last);
    }

    void setUpCircularList() {
        m_start.next = &m_start;
        m_start.prev = &m_start;
    }

    SizeType m_size{0};
    Node m_start{};
};

} //! namespace atom::containers::dynamic

#endif //! ATOM_CONTAINERS_DYNAMIC_CIRCULAR_LIST_H
