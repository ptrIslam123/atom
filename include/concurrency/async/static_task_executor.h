#ifndef ATOM_ASYNC_TASK_EXECUTOR_H
#define ATOM_ASYNC_TASK_EXECUTOR_H

#include <include/platform_api/posix_api/futex.h>
#include <functional>
#include <atomic>
#include <thread>
#include <array>
#include <cstddef>
#include <cstdint>

namespace atom::async {

//template<std::size_t N, std::size_t QCapacity>
//class StaticTaskExecutor final {
//public:
//    void start() {
//        for (auto i = 0; i < N; ++i) {
//            m_executors[i] = std::thread{[this, id = i] { execute(id); }};
//        }
//    }

//    void stop() {
//        for (auto i = 0; i < N; ++i) {
//            enqueue(std::function<void(){}>);
//        }
//    }

//    bool enqueue(std::function<void()> f) {
//        return m_queues[getId()].push(Task{std::move(f)});
//    }

//private:
//    struct Promise final {};

//    template<class T, std::size_t N>
//    struct LockFreeQueue final {
//        bool isEmpty() const noexcept {
//            //! TODO
//        }

//        bool isFull() const noexcept {
//            //! TODO
//        }

//        T pop() {
//            //! TODO
//        }

//        bool push(const T& data) {
//            //! TODO
//        }
//    };

//    using IdType = std::uint32_t;
//    using TaskQueue = LockFreeQueue<Task, QCapacity>;

//    IdType getId() const noexcept {
//        return IdType{};//! TODO
//    }

//    void execute(const IdType id) {
//        using namespace atom::platform::posix;
//        auto& queue = m_queues[id];
//        auto& taskCount = m_taskCount[id];
//        for (;;) {
//            if (queue.isEmpty()) {
//                FutexWait(taskCount, taskCount.load());
//            } else {
//                auto&& task = queue.pop();
//                if (task) {
//                    tast();
//                    taskCount.feat_sub(1);
//                } else {
//                    break;
//                }
//            }
//        }
//    }

//    struct Task final {};

//    std::array<std::atomic<std::uint32_t>, N> m_taskCount{0}; // maybe rte counter for faster increment & decrement?
//    std::array<TaskQueue, N> m_queues;
//    std::array<std::thread, N> m_executors;
//};

} //! namespace atom::async

#endif //! ATOM_ASYNC_TASK_EXECUTOR_H
