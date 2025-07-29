#ifndef ATOM_UTILS_MEASURE_TIME_H
#define ATOM_UTILS_MEASURE_TIME_H

#include <functional>
#include <chrono>
#include <ctime>

namespace atom::utils {

template <typename F>
inline auto MeasureTimeNanosec(F&& f)
{
    auto get_current_time_in_ns = [] {
        struct timespec time {};
        clock_gettime(CLOCK_MONOTONIC, &time);
        return time.tv_sec * 1'000'000'000 + time.tv_nsec;
    };

    const auto start_time = get_current_time_in_ns();
    std::invoke(std::forward<F>(f));
    const auto end_time = get_current_time_in_ns();
    const auto diff = end_time - start_time;
    assert(diff > 0);
    std::chrono::microseconds{0};
    return diff;
}

template <typename F>
inline auto MeasureTimeMicrosec(F&& f)
{
    return MeasureTimeNanosec(std::forward<F>(f)) / 1'000;  // 1 μs = 1000 ns
}

template <typename F>
inline auto MeasureTimeMillisec(F&& f)
{
    return MeasureTimeNanosec(std::forward<F>(f)) / 1'000'000;  // 1 ms = 1,000,000 ns
}

template <typename F>
inline auto MeasureTimeSec(F&& f)
{
    return MeasureTimeNanosec(std::forward<F>(f)) / 1'000'000'000;  // 1 s = 1,000,000,000 ns
}

} //! namespace atom::utils

#endif //! ATOM_UTILS_MEASURE_TIME_H
