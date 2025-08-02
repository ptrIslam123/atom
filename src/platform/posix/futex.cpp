#include "include/platform_api/posix_api/futex.h"

#if defined(__linux__)

#include "include/platform_api/posix_api/sys_error.h"
#include "include/utils/assertion.h"

#include <cerrno>

#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace {

int FutexSysCall(std::uint32_t* uaddr, int futex_op, std::uint32_t val, const struct timespec* timeout = nullptr) {
    return syscall(SYS_futex, reinterpret_cast<void*>(uaddr), futex_op, val, timeout, nullptr, 0);
}

} //! namespace

namespace atom::platform::posix {

void FutexWait(std::atomic<std::uint32_t>& futexAddr, std::uint32_t expectedVal) {
    FutexWait(futexAddr, expectedVal, nullptr);
}

void FutexWait(std::atomic<std::uint32_t>& futexAddr, std::uint32_t expectedVal, const struct timespec* timeout) {
    while (true) {
        int rc = FutexSysCall(reinterpret_cast<std::uint32_t*>(&futexAddr), FUTEX_WAIT, static_cast<std::uint32_t>(expectedVal), timeout);
        if (rc == 0) {
            return;
        }
        if (errno == EAGAIN) {
            return;
        }
        if (errno != EINTR) {
            ASSERTION(false, std::runtime_error, "futex_wait failed: " + GetLastSysErrorMessage())
        }
    }
}

void FutexWakeUpAny(std::atomic<std::uint32_t>& futexAddr) {
    FutexWakeUp(futexAddr, 1);
}

void FutexWakeUp(std::atomic<std::uint32_t>& futexAddr, std::uint32_t wakeUpCount) {
    int rc = FutexSysCall(reinterpret_cast<std::uint32_t*>(&futexAddr), FUTEX_WAKE, wakeUpCount);
    if (rc == -1) {
        ASSERTION(false, std::runtime_error, "futex_wake failed: " + GetLastSysErrorMessage())
    }
}

} //! namespace atom::platform::posix

#endif //! if defined(__linux__)
