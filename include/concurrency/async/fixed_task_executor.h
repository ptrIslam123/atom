#ifndef ATOM_ASYNC_TASK_EXECUTOR_H
#define ATOM_ASYNC_TASK_EXECUTOR_H

#include "include/containers/lock_free/spsc_ring_queue.h"

#include <array>
#include <thread>

#include <cstddef>

namespace atom::async::fixed {

template<std::size_t Executors, std::size_t QueueCapacity>
class TaskExecutor final {
public:
    static constexpr auto EXECUTORS{Executors};
    static constexpr auto QUEUE_CAPACITY{QueueCapacity};

    using Executor = std::jthread;
    using ExecutorQueue = containers::lock_free::spsc::RingQueue<QueueCapacity>; // ??? need mpsc::RingQueue


    class Task final {

    };

    void start() {

    }

    void stop() {

    }

    void enqueue() {
    }

private:
    ExecutorQueue& findFitQueue() {}

    std::array<Executor, Executors> m_executors;
    std::array<ExecutorQueue, Executors> m_queues;
};

} //! namespace atom::async::fixed

#endif //! ATOM_ASYNC_TASK_EXECUTOR_H
