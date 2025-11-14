#include "include/platform_api/posix_api/futex.h"

#if defined(__linux__)

#include "include/platform_api/posix_api/sys_error.h"
#include "include/utils/assertion.h"

#include <cerrno>

#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace {

int FutexSysCall(std::uint32_t* uaddr, int futex_op, std::uint32_t val, const struct timespec* timeout) {
    return syscall(SYS_futex, reinterpret_cast<void*>(uaddr), futex_op, val, timeout, nullptr, 0);
}

} //! namespace

namespace atom::platform::posix {

FutexStatus FutexWait(std::uint32_t& futexAddr, std::uint32_t expectedVal, struct timespec* timeout) {
    while (true) {
        int rc = FutexSysCall(&futexAddr, FUTEX_WAIT, expectedVal, timeout);
        if (rc == 0) {
            return FutexStatus::ByWakeUp;
        }
        switch (errno) {
            case ETIMEDOUT: return FutexStatus::ByTimeout;
            case EAGAIN: return FutexStatus::ByWakeUp;
            case EINTR: continue;
            default: ASSERTION(false, std::runtime_error, "futex_wait failed: " + GetLastSysErrorMessage())
        }
    }
}

void FutexWakeUp(std::uint32_t& futexAddr, std::uint32_t wakeUpCount) {
    int rc = FutexSysCall(&futexAddr, FUTEX_WAKE, wakeUpCount, nullptr);
    if (rc == -1) {
        ASSERTION(false, std::runtime_error, "futex_wake failed: " + GetLastSysErrorMessage())
    }
}

void FutexWakeUpAny(std::uint32_t& futexAddr) {
    FutexWakeUp(futexAddr, 1);
}

} //! namespace atom::platform::posix

#endif //! if defined(__linux__)
