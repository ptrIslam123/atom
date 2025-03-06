#ifndef ATOM_CONTAINERS_STATIC_RING_QUEUE_H
#define ATOM_CONTAINERS_STATIC_RING_QUEUE_H

#include "include/utils/assertion.h"

#include <type_traits>
#include <stdexcept>
#include <array>
#include <cstddef>
#include <cstring>

namespace atom::containers {

/**
 * @brief A static ring queue (circular buffer) implementation with fixed capacity
 *
 * @tparam T Type of elements stored in the queue
 * @tparam Capacity Maximum number of elements the queue can hold
 *
 * @note When the queue is full, new elements will overwrite the oldest ones
 * (FIFO replacement policy). This provides O(1) insertion without throwing
 * on overflow.
 *
 * @example
 * StaticRingQueue<int, 3> queue;
 * queue.enqueue(10);  // [10]
 * queue.enqueue(20);  // [10, 20]
 * queue.enqueue(30);  // [10, 20, 30] (full)
 * queue.enqueue(40);  // [20, 30, 40] (10 overwritten)
 */
template<typename T, std::size_t Capacity>
class StaticRingQueue final {
public:
    using ValueType = T; //! Type of elements stored in the queue

    explicit StaticRingQueue();
    StaticRingQueue(const StaticRingQueue& other) noexcept(std::is_nothrow_copy_constructible_v<T>);
    StaticRingQueue(StaticRingQueue&& other) noexcept(std::is_nothrow_copy_constructible_v<T>);
    StaticRingQueue& operator=(const StaticRingQueue& other) noexcept(std::is_nothrow_copy_constructible_v<T>);
    StaticRingQueue& operator=(StaticRingQueue&& other) noexcept(std::is_nothrow_copy_constructible_v<T>);
    ~StaticRingQueue();

    /**
     * @brief Constructs element in-place at the end of the queue
     * @tparam Args Types of arguments to construct the element
     * @param args Arguments to forward to element constructor
     *
     * @note If queue is full, overwrites the oldest element
     */
    template<typename... Args>
    void emplace(Args&&... args);

    /**
     * @brief Adds element to the end of the queue (copy)
     * @param value Element to add
     */
    void enqueue(const T& value);
    void enqueue(T&& value);

    /**
     * @brief Removes element from the front of the queue
     * @throws std::runtime_error if queue is empty
     */
    void dequeue();

    /**
     * @brief Removes all elements from the queue
     */
    void clear();

    /**
     * @brief Accesses element at the front of the queue
     * @return Reference to front element
     * @throws std::runtime_error if queue is empty
     */
    T& front();
    const T& front() const;

    /**
     * @brief Accesses element at the back of the queue
     * @return Reference to back element
     * @throws std::runtime_error if queue is empty
     */
    T& back();
    const T& back() const;

    /**
     * @brief Gets current number of elements in the queue
     * @return Number of elements
     */
    std::size_t size() const noexcept;

    /**
     * @brief Return maximum capacity of the queue
     * @return maximum capacity of the queue
     */
    constexpr size_t capacity() const noexcept;

    /**
     * @brief Checks if queue is empty
     * @return true if queue is empty, false otherwise
     */
    bool isEmpty() const noexcept;

    /**
     * @brief Checks if queue is full
     * @return true if queue is full, false otherwise
     */
    bool isFull() const noexcept;

private:
    void swap(StaticRingQueue& other) noexcept(std::is_nothrow_copy_constructible_v<T>);
    void copy(const StaticRingQueue& other) noexcept(std::is_nothrow_copy_constructible_v<T>);

    std::array<std::byte, Capacity * sizeof(T)> m_storage{};
    std::size_t m_head{0};
    std::size_t m_tail{0};
    std::size_t m_size{0};
};

template<typename T, std::size_t Capacity>
StaticRingQueue<T, Capacity>::StaticRingQueue():
m_head(0),
m_tail(0),
m_size(0)
{}

template<typename T, std::size_t Capacity>
StaticRingQueue<T, Capacity>::StaticRingQueue(const StaticRingQueue& other) noexcept(std::is_nothrow_copy_constructible_v<T>) {
    (void)this->operator=(other);
}

template<typename T, std::size_t Capacity>
StaticRingQueue<T, Capacity>::StaticRingQueue(StaticRingQueue&& other) noexcept(std::is_nothrow_copy_constructible_v<T>) {
    (void)this->operator=(other);
}

template<typename T, std::size_t Capacity>
StaticRingQueue<T, Capacity>& StaticRingQueue<T, Capacity>::operator=(const StaticRingQueue& other) noexcept(std::is_nothrow_copy_constructible_v<T>) {
    if (this != &other) {
        clear();
        copy(other);
    }
    return *this;
}

template<typename T, std::size_t Capacity>
StaticRingQueue<T, Capacity>& StaticRingQueue<T, Capacity>::operator=(StaticRingQueue&& other) noexcept(std::is_nothrow_copy_constructible_v<T>) {
    if (this != &other) {
        if constexpr (std::is_trivially_copyable_v<T> || std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_copy_assignable_v<T>) {
            (void)this->operator=(other); // just copy
            if constexpr (std::is_trivially_copyable_v<T>) {
                other.m_head = other.m_tail = other.m_size = 0;
            } else {
                other.clear();
            }
        } else if constexpr (std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T>) {
            while (!other.isEmpty()) {
                emplace(std::move(other.front()));
                other.dequeue();
            }
        } else {
            // we have to  make temp instance to saveecontainer`s invariants in case copy/move T rise exception
            StaticRingQueue tmp;
            swap(tmp, other);
            swap(tmp, *this);
        }
    }
    return *this;
}

template<typename T, std::size_t Capacity>
void StaticRingQueue<T, Capacity>::swap(StaticRingQueue& other) noexcept(std::is_nothrow_copy_constructible_v<T>) {
    if constexpr (std::is_trivially_copyable_v<T>) {
        StaticRingQueue tmp;
        std::memcpy(&tmp, this, sizeof(StaticRingQueue));
        std::memcpy(this, &other, sizeof(StaticRingQueue));
        std::memcpy(&other, &tmp, sizeof(StaticRingQueue));
    } else {
        StaticRingQueue temp;
        while (!isEmpty()) {
            temp.emplace(std::move(front()));
            dequeue();
        }

        while (!other.isEmpty()) {
            emplace(std::move(other.front()));
            other.dequeue();
        }

        while (!temp.isEmpty()) {
            other.emplace(std::move(temp.front()));
            temp.dequeue();
        }
    }
}

template<typename T, std::size_t Capacity>
void StaticRingQueue<T, Capacity>::copy(const StaticRingQueue& other) noexcept(std::is_nothrow_copy_constructible_v<T>) {
    if constexpr (std::is_trivial_v<T>) {
        std::memcpy(this, &other, sizeof(other));
    } else if constexpr (std::is_nothrow_copy_constructible_v<T>) {
        for (size_t i = 0; i < other.m_size; ++i) {
            size_t pos = (other.m_head + i) % capacity();
            emplace(*reinterpret_cast<const T*>(&other.m_storage[pos * sizeof(T)]));
        }
    } else {
        std::size_t copied = 0;
        try {
            for (; copied < other.m_size; ++copied) {
                const std::size_t pos = (other.m_head + copied) % capacity();
                emplace(*reinterpret_cast<const T*>(&other.m_storage[pos * sizeof(T)]));
            }
        } catch (...) {
            while (copied-- > 0) {
                m_tail = (m_tail - 1 + capacity()) % capacity();
                reinterpret_cast<T*>(&m_storage[m_tail * sizeof(T)])->~T();
                --m_size;
            }
            throw;
        }
    }
}

template<typename T, std::size_t Capacity>
StaticRingQueue<T, Capacity>::~StaticRingQueue() {
    clear();
}

template<typename T, std::size_t Capacity>
template<typename... Args>
void StaticRingQueue<T, Capacity>::emplace(Args&&... args) {
    if (!isFull()) {}
    else {
        reinterpret_cast<T*>(&m_storage[m_head * sizeof(T)])->~T();
        m_head = (m_head + 1) % capacity();
        --m_size;
    }

    new (&m_storage[m_tail * sizeof(T)]) T(std::forward<Args>(args)...);
    m_tail = (m_tail + 1) % capacity();
    ++m_size;
}

template<typename T, std::size_t Capacity>
void StaticRingQueue<T, Capacity>::enqueue(const T& value) {
    emplace(value);
}

template<typename T, std::size_t Capacity>
void StaticRingQueue<T, Capacity>::enqueue(T&& value) {
    emplace(std::move(value));
}

template<typename T, std::size_t Capacity>
void StaticRingQueue<T, Capacity>::dequeue() {
    ASSERTION(!isEmpty(), std::runtime_error, "Queue is empty")
    reinterpret_cast<T*>(&m_storage[m_head * sizeof(T)])->~T();
    m_head = (m_head + 1) % capacity();
    --m_size;
}

template<typename T, std::size_t Capacity>
T& StaticRingQueue<T, Capacity>::front() {
    ASSERTION(!isEmpty(), std::runtime_error, "Queue is empty")
    return *reinterpret_cast<T*>(&m_storage[m_head * sizeof(T)]);
}

template<typename T, std::size_t Capacity>
const T& StaticRingQueue<T, Capacity>::front() const {
    ASSERTION(!isEmpty(), std::runtime_error, "Queue is empty")
    return *reinterpret_cast<const T*>(&m_storage[m_head * sizeof(T)]);
}

template<typename T, std::size_t Capacity>
T& StaticRingQueue<T, Capacity>::back() {
    ASSERTION(!isEmpty(), std::runtime_error, "Queue is empty")
    return *reinterpret_cast<T*>(&m_storage[((m_tail - 1 + capacity()) % capacity()) * sizeof(T)]);
}

template<typename T, std::size_t Capacity>
const T& StaticRingQueue<T, Capacity>::back() const {
    ASSERTION(!isEmpty(), std::runtime_error, "Queue is empty")
    return *reinterpret_cast<const T*>(&m_storage[((m_tail - 1 + capacity()) % capacity()) * sizeof(T)]);
}

template<typename T, std::size_t Capacity>
bool StaticRingQueue<T, Capacity>::isEmpty() const noexcept {
    return size() == 0;
}

template<typename T, std::size_t Capacity>
bool StaticRingQueue<T, Capacity>::isFull() const noexcept {
    return size() == capacity();
}

template<typename T, std::size_t Capacity>
std::size_t StaticRingQueue<T, Capacity>::size() const noexcept {
    return m_size;
}

template<typename T, std::size_t Capacity>
constexpr std::size_t StaticRingQueue<T, Capacity>::capacity() const noexcept {
    return Capacity;
}

template<typename T, std::size_t Capacity>
void StaticRingQueue<T, Capacity>::clear() {
    while (!isEmpty()) {
        dequeue();
    }
}

} //! namespace atom::containers

#endif //! ATOM_CONTAINERS_STATIC_RING_QUEUE_H
