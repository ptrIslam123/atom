#ifndef ATOM_CONCURRENCY_SYNCHRONIZER_CLASSIC_MUTEX_H
#define ATOM_CONCURRENCY_SYNCHRONIZER_CLASSIC_MUTEX_H

#include <atomic>

#include <cstdint>

namespace atom::concurrency::synchronizer {

class ClassisMutex final {
public:
    ClassisMutex(const ClassisMutex& ) = delete;
    ClassisMutex(ClassisMutex&& ) = delete;
    ClassisMutex& operator=(const ClassisMutex& ) = delete;
    ClassisMutex& operator=(ClassisMutex&& ) = delete;

    ClassisMutex() noexcept = default;
    ~ClassisMutex() noexcept = default;

    void lock() {
        while (!tryLock()) {
            m_status.wait(Status::Locked);
        }
    }

    bool tryLock() {
        auto expected{Status::UnLocked};
        return m_status.compare_exchange_strong(expected, Status::Locked);
    }

    void unlock() {
        m_status.store(Status::UnLocked, std::memory_order_release);
        m_status.notify_one();
    }

private:
    enum class Status {
        Locked,
        UnLocked,
    };
    std::atomic<Status> m_status{Status::UnLocked};
};

} //! namespace atom::concurrency::synchronizer

#endif //! ATOM_CONCURRENCY_SYNCHRONIZER_CLASSIC_MUTEX_H
