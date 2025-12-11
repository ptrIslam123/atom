#ifndef ATOMC_CONTAINERS_LOCK_FREE_DYNAMIC_QUEUE_H
#define ATOMC_CONTAINERS_LOCK_FREE_DYNAMIC_QUEUE_H

#include "include/utils/compiler_attr.h"
#include "include/utils/assertion.h"
#include "include/memory/allocators/default_allocator.h"

#include <atomic>
#include <new>
#include <cassert>

namespace atom::containers::lock_free::dynamic {

template<typename T, typename A = memory::allocator::DefaultAllocator>
class Queue final {
public:
    using ElementType = T;
    using AllocatorType = A;

    Queue() noexcept {
        Node* stub = allocate(T{});
        m_head.store(stub, std::memory_order_relaxed);
        m_tail.store(stub, std::memory_order_relaxed);
    }

    ~Queue() {
        T dummy;
        while (dequeue(dummy)) { /* опустошаем */ }
        Node* last = m_head.load(std::memory_order_relaxed);
        deallocate(last);                    // освободили фиктивный
    }

    void enqueue(const T& data) {
        auto newNode = allocate(data);
        auto curTailNode = m_tail.load(std::memory_order_acquire);

        for (;;) {
            auto curTailNextNode = curTailNode->next.load(std::memory_order_acquire);
            if LIKELY_EXPR(!curTailNextNode) {
                if (curTailNode->next.compare_exchange_weak(
                        curTailNextNode, newNode,
                        std::memory_order_release,
                        std::memory_order_acquire
                )) {
                    /*
                     *  Мы уже успешно прицепили newNode к curTailNode->next
                     *  (curTailNode->next.compare_exchange_weak вернул true).
                     *  Теперь новый узел навсегда в очереди, и все последующие потоки будут видеть его.
                     *  Глобальный tail может быть ещё на curTailNode (если никто не подвинул) или уже на каком-то
                     *  следующем узле (если кто-то помог).
                     *
                     *  Нам не важно, останется ли именно наш CAS тем, кто физически передвинет tail.
                     *  В обоих случаях наш узел остаётся в очереди, а текущий поток может спокойно выйти – догнать tail при необходимости будут либо последующие вставляющие потоки, либо dequeue.
                     */
                    m_tail.compare_exchange_weak(
                        curTailNode, newNode,
                        std::memory_order_release,
                        std::memory_order_relaxed
                    );
                    break;
                }
            } else {
                /*  tail может отстать по двум причинам:
                 *
                 *  1. Разрыв между двумя CAS внутри одного enqueue.
                 *      Поток A успешно прицепил узел за X:
                 *          X->next.compare_exchange_weak(..., newNode)  // УСПЕХ
                 *      но ещё НЕ успел обновить глобальный tail:
                 *          tail->compare_exchange_weak(X, newNode)      // ЕЩЎ НЕ ВЫЗВАН
                 *      В этот момент tail указывает на X, а X->next уже ≠ nullptr.
                 *
                 *  2.  Конкурентный поток «вставил и ушёл».
                 *      Поток B вставил узел за Y, обновил Y->next, обновил tail и завершился.
                 *      Поток C читает старое значение tail (Y), видит Y->next ≠ nullptr,
                 *      но сам ещё ничего не вставлял. Опять tail ≠ фактическому хвосту.
                 *
                 *  В обоих случаях очередь продолжает работать корректно, но tail
                 *  висит позади. Чтобы не крутиться вхолостую, мы САМИ CAS-ом
                 *  подтягиваем tail ближе к реальному хвосту и продолжаем цикл
                 *  уже с актуальным curTailNode.
                 */
                m_tail.compare_exchange_weak(
                    curTailNode, curTailNextNode,
                    std::memory_order_release,
                    std::memory_order_relaxed
                );
            }
        }
    }

    bool dequeue(T& out) {
        for (;;) {
            auto headNode = m_head.load(std::memory_order_acquire);
            auto tailNode = m_tail.load(std::memory_order_acquire);
            auto nextNode = headNode->next.load(std::memory_order_acquire);

            /* 1. Убеждаемся, что head не изменился */
            if UNLIKELY_EXPR(headNode != m_head.load(std::memory_order_acquire)) continue;

            /* 2. Если head == tail, очередь либо пуста, либо tail отстал */
            if (headNode == tailNode) {
                if (nextNode == nullptr)          // действительно пуста
                    return false;

                /* tail отстал – помогаем двигать */
                m_tail.compare_exchange_weak(
                    tailNode, nextNode,
                    std::memory_order_release,
                    std::memory_order_relaxed
                );
                continue;
            }

            /* 3. Есть элемент – пытаемся сдвинуть head */
            if (m_head.compare_exchange_weak(
                    headNode, nextNode,
                    std::memory_order_release,
                    std::memory_order_relaxed))
            {
                out = std::move(nextNode->data);  // данные хранятся в *следующем* узле
                deallocate(headNode);             // освободили фиктивный/старый узел
                return true;
            }
        }
    }

private:
    static constexpr auto CACHELINE_SIZE{64 /*std::hardware_destructive_interference_size*/};

    struct Node {
        std::atomic<Node*> next{nullptr};
        T data{};

        Node(const T& _data)
            : data(_data)
            , next(nullptr)
        {}
    };

    Node* allocate(const T& data) {
        Node* node =  m_allocator.allocate(sizeof(T));
        ASSERTION(node, std::runtime_error, "Bad alloc")
        m_allocator.template construct<Node>(node, data);
        return node;
    }

    void deallocate(Node* node) {
        if (node) {
            m_allocator.template destruct<Node>(node);
            m_allocator.deallocate(node);
        }
    }

    alignas(CACHELINE_SIZE) std::atomic<Node*> m_head;
    alignas(CACHELINE_SIZE) std::atomic<Node*> m_tail;
    AllocatorType m_allocator;
};

} // namespace atom::containers::lock_free::dynamic

#endif //! ATOMC_CONTAINERS_LOCK_FREE_DYNAMIC_QUEUE_H
