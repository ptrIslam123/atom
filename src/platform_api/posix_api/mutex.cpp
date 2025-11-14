#include "include/platform_api/posix_api/mutex.h"

namespace atom::platform::posix {

bool Mutex::tryLock() {
    constexpr auto weakMode{false};
    auto expected{static_cast<decltype(m_status)>(Status::Unlocked)};
    return __atomic_compare_exchange_n(
                &m_status,
                &expected,
                weakMode,
                static_cast<decltype(m_status)>(Status::Locked),
                __ATOMIC_ACQUIRE,
                __ATOMIC_RELAXED
    );
}

void Mutex::lock() {
    while (!tryLock()) {
        (void)FutexWait(m_status, static_cast<decltype(m_status)>(Status::Unlocked));
    }
}

void Mutex::unlock() {
    __atomic_store_n(&m_status, static_cast<decltype(m_status)>(Status::Unlocked), __ATOMIC_RELEASE);
    FutexWakeUpAny(m_status);
}

} //! namespace atom::platform::posix
