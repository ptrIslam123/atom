#ifndef ATOM_PLATFORM_API_POSIX_MUTEX_H
#define ATOM_PLATFORM_API_POSIX_MUTEX_H

#include "include/platform_api/posix_api/futex.h"

#include <chrono>
#include <cstdint>

namespace atom::platform::posix {

class Mutex final {
public:
    Mutex(const Mutex& ) = delete;
    Mutex(Mutex&& ) = delete;
    Mutex& operator=(const Mutex& ) = delete;
    Mutex& operator=(Mutex&& ) = delete;

    Mutex() noexcept = default;
    ~Mutex() noexcept = default;

    template<typename Rep, typename Period>
    bool tryLockFor(const std::chrono::duration<Rep, Period>& timeout);
    template<typename Rep, typename Period>
    bool tryLockUntil(const std::chrono::duration<Rep, Period>& absTime);
    bool tryLock();
    void lock();
    void unlock();

private:
    enum class Status : std::uint32_t {
        Locked,
        Unlocked,
    };

    std::uint32_t m_status{static_cast<std::uint32_t>(Status::Unlocked)};
};

template<typename Rep, typename Period>
bool Mutex::tryLockFor(const std::chrono::duration<Rep, Period>& timeout) {
    while (!tryLock()) {
        if (FutexWaitFor(&m_status, static_cast<decltype(m_status)>(Status::Unlocked), timeout) == FutexStatus::ByTimeout) {
            return false;
        }
    }
    return true;
}

template<typename Rep, typename Period>
bool Mutex::tryLockUntil(const std::chrono::duration<Rep, Period>& absTime) {
    while (!tryLock()) {
        if (FutexWaitUntil(&m_status, static_cast<decltype(m_status)>(Status::Unlocked), absTime) == FutexStatus::ByTimeout) {
            return false;
        }
    }
    return true;
}

} //! namespace atom::concurrency::synchronizer

#endif //! ATOM_PLATFORM_API_POSIX_MUTEX_H
