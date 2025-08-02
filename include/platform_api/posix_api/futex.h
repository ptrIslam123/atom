#ifndef AIRPLAY_UTILS_FUTEX_H
#define AIRPLAY_UTILS_FUTEX_H

#if defined(__linux__)

#include <chrono>
#include <atomic>
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

/**
 * @brief Wakes up exactly one thread/process waiting on the futex address
 *
 * @param futexAddr The atomic variable used as futex word
 * @throws std::runtime_error if wait fails
 */
void FutexWakeUpAny(std::atomic<std::uint32_t>& futexAddr);

/**
 * @brief Wakes up @wakeUpCount thread/process waiting on the futex address
 *
 * @param futexAddr The atomic variable used as futex word
 * @param wakeUpCount number of wake up threads/processes
 * @throws std::runtime_error if wait fails
 */
void FutexWakeUp(std::atomic<std::uint32_t>& futexAddr, std::uint32_t wakeUpCount);

/**
 * @brief Waits for the futex value to change from expectedVal
 *
 * @param futexAddr The atomic variable to watch
 * @param expectedVal The value to compare against
 * @param timeout Absolute timeout (nullptr for infinite wait)
 *
 * @throws std::runtime_error if wait fails (except for timeout)
 *
 * @note Spurious wakeups are possible - always recheck condition
 */
void FutexWait(std::atomic<std::uint32_t>& futexAddr, std::uint32_t expectedVal);
void FutexWait(std::atomic<std::uint32_t>& futexAddr, std::uint32_t expectedVal, const struct timespec* timeout);

/**
 * @brief Waits with relative timeout duration
 *
 * @tparam Rep Duration representation type
 * @tparam Period Duration period
 *
 * @param futexAddr The atomic variable to watch
 * @param expectedVal The value to compare against
 * @param timeout Relative timeout duration
 *
 * @throws std::runtime_error if wait fails
 */
template<typename Rep, typename Period>
void FutexWaitFor(std::atomic<std::uint32_t>& futexAddr, std::uint32_t expectedVal, const std::chrono::duration<Rep, Period>& timeout);


template<typename Rep, typename Period>
inline void FutexWaitFor(std::atomic<std::uint32_t>& futexAddr, std::uint32_t expectedVal, const std::chrono::duration<Rep, Period>& timeout) {
    using namespace std::chrono;
    auto ns = duration_cast<nanoseconds>(timeout);
    auto sec = duration_cast<seconds>(ns);
    ns -= sec;

    struct timespec ts;
    ts.tv_sec = sec.count();
    ts.tv_nsec = ns.count();
    FutexWait(futexAddr, expectedVal, &ts);
}

} //! namespace atom::platform::posix

#endif //! if defined(__linux__)

#endif //! AIRPLAY_UTILS_FUTEX_H
