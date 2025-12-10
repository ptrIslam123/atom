#ifndef AIRPLAY_UTILS_FUTEX_H
#define AIRPLAY_UTILS_FUTEX_H

#if defined(__linux__)

#include <chrono>
#include <cstdint>
#include <cstddef>

#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 22)
#error "Futex requires Linux kernel version 2.6.22 or later."
#endif //! !defined(__linux__) || LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 22)

/**
 * @brief Provides futex-based synchronization primitives
 *
 * The futex (Fast Userspace Mutex) mechanism allows efficient thread synchronization
 * by combining userspace atomic operations with kernel-based waiting.
 *
 * @note On Linux uses the futex(2) system call directly
 * @note On Windows emulates via WaitOnAddress/WakeByAddress
 * @note All functions are thread-safe and non-blocking in fast path
 *
 * @see https://man7.org/linux/man-pages/man2/futex.2.html (Linux)
 */
namespace atom::platform::posix {

enum class FutexStatus {
    ByWakeUp,
    ByTimeout
};

/**
 * @brief Wakes up exactly one thread/process waiting on the futex address
 *
 * @param futexAddr The atomic variable used as futex word
 * @throws std::runtime_error if wait fails
 */
void FutexWakeUpAny(std::uint32_t& futexAddr);

/**
 * @brief Wakes up @wakeUpCount thread/process waiting on the futex address
 *
 * @param futexAddr The atomic variable used as futex word
 * @param wakeUpCount number of wake up threads/processes
 * @throws std::runtime_error if wait fails
 */
void FutexWakeUp(std::uint32_t& futexAddr, std::uint32_t wakeUpCount);

/**
 * @brief Waits for the futex value to change from expectedVal
 *
 * @param futexAddr The variable to watch
 * @param expectedVal The value to compare against
 * @param timeout waiting timeout (nullptr for infinite wait)
 * @return
 *      ByWakeUp - in case the value of the variable was changed and/or there was a FutexWakeUp*;
 *      ByTimeout - in case the waiting time has expired
 * @throws std::runtime_error if wait fails
 *
 * @note Spurious wakeups are possible - always recheck condition
 */
FutexStatus FutexWait(std::uint32_t& futexAddr, std::uint32_t expectedVal, struct timespec* timeout = nullptr);

/**
 * @brief Waits with relative timeout duration
 *
 * @tparam Rep Duration representation type
 * @tparam Period Duration period
 *
 * @param futexAddr The atomic variable to watch
 * @param expectedVal The value to compare against
 * @param timeout Relative timeout duration
 * @return
 *      ByWakeUp - in case the value of the variable was changed and/or there was a FutexWakeUp*;
 *      ByTimeout - in case the waiting time has expired
 *
 * @throws std::runtime_error if wait fails
 *
 * @note Spurious wakeups are possible - always recheck condition
 */
template<typename Rep, typename Period>
FutexStatus FutexWaitFor(std::uint32_t& futexAddr, std::uint32_t expectedVal, const std::chrono::duration<Rep, Period>& timeout);

/**
 * @brief Waits with absolute timeout duration
 *
 * @tparam Rep Duration representation type
 * @tparam Period Duration period
 *
 * @param futexAddr The atomic variable to watch
 * @param expectedVal The value to compare against
 * @param timeout Relative timeout duration
 * @return
 *      ByWakeUp - in case the value of the variable was changed and/or there was a FutexWakeUp*;
 *      ByTimeout - in case the waiting time has expired
 *
 * @throws std::runtime_error if wait fails
 *
 * @note Spurious wakeups are possible - always recheck condition
 */
template<typename Rep, typename Period>
FutexStatus FutexWaitUntil(std::uint32_t& futexAddr, std::uint32_t expectedVal, const std::chrono::duration<Rep, Period>& absTime);


template<typename Rep, typename Period>
inline FutexStatus FutexWaitFor(std::uint32_t& futexAddr, std::uint32_t expectedVal, const std::chrono::duration<Rep, Period>& timeout) {
    using namespace std::chrono;
    auto ns = duration_cast<nanoseconds>(timeout);
    if (ns.count() <= 0) {
        return FutexStatus::ByTimeout;
    }

    auto sec = duration_cast<seconds>(ns);
    ns -= sec;

    struct timespec ts;
    ts.tv_sec = sec.count();
    ts.tv_nsec = ns.count();
    return FutexWait(futexAddr, expectedVal, &ts);
}

template<typename Rep, typename Period>
inline FutexStatus FutexWaitUntil(std::uint32_t& futexAddr, std::uint32_t expectedVal, const std::chrono::duration<Rep, Period>& absTime) {
    using namespace std::chrono;
    const auto now = steady_clock::now();
    const auto absSteadyTime = time_point_cast<steady_clock::duration>(absTime);
    if (now >= absSteadyTime) {
        return FutexStatus::ByTimeout;
    }
    return FutexWaitFor(futexAddr, expectedVal, absSteadyTime - now);
}

} //! namespace atom::platform::posix

#endif //! if defined(__linux__)

#endif //! AIRPLAY_UTILS_FUTEX_H
