#ifndef METRICS_HOLDER_H
#define METRICS_HOLDER_H

#include <algorithm>
#include <functional>
#include <stdexcept>
#include <array>
#include <vector>
#include <atomic>
#include <limits>
#include <mutex>

#include <cmath>
#include <cstdint>
#include <cassert>

namespace __metrics_holder_impl_details
{

template <typename R> class CounterWrapper final {
public:
    using ValueType = R;
    CounterWrapper(R data = R())
        : m_data(data)
    {
    }
    R fetch_add(const R data) noexcept
    {
        auto tmp{ m_data };
        m_data += data;
        return tmp;
    }
    R load() const noexcept
    {
        return m_data;
    }
    void store(const R data)
    {
        m_data = data;
    }

private:
    R m_data;
};

template <typename R> class CounterWrapper<std::atomic<R>> final {
public:
    using ValueType = R;
    CounterWrapper(R data = R())
        : m_data(data)
    {
    }
    R fetch_add(const R data) noexcept
    {
        return m_data.fetch_add(data);
    }
    R load() const noexcept
    {
        return m_data.load();
    }
    void store(const R data)
    {
        m_data.store(data);
    }

private:
    std::atomic<R> m_data;
};

inline auto get_current_time_in_ns() noexcept
{
    struct timespec time {};
    clock_gettime(CLOCK_MONOTONIC, &time);
    return time.tv_sec * 1'000'000'000 + time.tv_nsec;
}

} //! namespace __metrics_holder_impl_details

/**
 * @brief Policy enum for selecting metrics accumulation strategy
 * This enumeration determines how metrics values are stored and processed:
 * - HistogramWay: Uses bucket-based distribution for memory efficiency
 * - HeavyWay: Stores all values for precise calculations at higher memory cost
 * @see MetricsHolder class
 */
enum class MetricsAccumulatePolicy {
    /**
     * @brief Histogram-based accumulation policy
     * Stores values in configurable buckets/ranges:
     * - Memory efficient (O(1) space)
     * - Faster calculations
     * - Limited precision (depends on bucket distribution)(D type of MetricsAccumulatePolicy::HistogramWay, T, D>)
     * - Good for monitoring systems with many metrics
     */
    HistogramWay,

    /**
     * @brief Exact-value accumulation policy
     * Stores all individual values:
     * - High memory usage (O(n) space)
     * - Slower calculations (requires sorting)
     * - Perfect precision
     * - Good for detailed analysis of few metrics
     */
    HeavyWay,
};

template <MetricsAccumulatePolicy P, typename T, typename... Args>
class MetricsHolder;

/**
 * @brief A histogram-based metrics accumulator with configurable distribution policy
 * @tparam T Value type for metrics (must be arithmetic type)
 * @tparam D Distribution policy type that determines bucket allocation.
 *           Must provide the following:
 *           - Static constant `BUCKET_NUMBER`: total number of buckets
 *           - Method `distribute(value, min, max)`: returns bucket index for given value
 *           - Method `max_value_of(bucket_index, min, max)`: returns upper bound of bucket
 *           - Method `min_value_of(bucket_index, min, max)`: returns lower bound of bucket
 */
template <typename T, typename D>
class MetricsHolder<MetricsAccumulatePolicy::HistogramWay, T, D> final
    : private D {
public:
    /// Type used for counter values
    using CounterType =
        typename __metrics_holder_impl_details::CounterWrapper<T>::ValueType;
    /// Type of the distribution policy
    using DistributorPolicyType = D;

    /// Number of buckets in the histogram
    static constexpr auto BUCKET_NUMBER = DistributorPolicyType::BUCKET_NUMBER;

    /**
     * @brief Construct a new MetricsHolder object
     * @param min Minimum value to track
     * @param max Maximum value to track
     */
    explicit MetricsHolder(T min, T max);

    /**
     * @brief Add a value to the histogram
     * @param value The value to add
     * @throws std::runtime_error if value is outside [min,max] range
     */
    void add(const T value);

    /**
     * @brief Measure execution time of a function and add it to the histogram
     * @tparam F Callable type
     * @param f Function to measure
     */
    template <typename F> void measure(F &&f);

    /**
     * @brief Get the minimum tracked value
     * @return CounterType Minimum value
     */
    CounterType get_min() const noexcept;

    /**
     * @brief Get the maximum tracked value
     * @return CounterType Maximum value
     */
    CounterType get_max() const noexcept;

    /**
     * @brief Get total number of elements in histogram
     * @return CounterType Total count of elements
     */
    CounterType get_elements() const noexcept;

    /**
     * @brief Get count of elements less than or equal to budget
     * @param budget The threshold value
     * @return CounterType Number of elements ≤ budget
     */
    CounterType get_count(const CounterType budget) const;

    /**
     * @brief Calculate Cumulative Distribution Function value
     * @param percent The threshold value
     * @return double Percentage of elements ≤ percent (0-100)
     * @example CDF(25) = 66.666% (66.666% data ≤ 25)
     */
    double get_CDF(const CounterType percent) const;

    /**
     * @brief Calculate percentile value
     * @param p Percentile to calculate (0-100)
     * @return double The value at p-th percentile
     */
    double get_percentile(const CounterType p) const;

private:
    template <typename R>
    using CounterWrapper = __metrics_holder_impl_details::CounterWrapper<R>;

    std::array<CounterWrapper<T>, BUCKET_NUMBER> m_buckets{};
    CounterWrapper<T> m_elements_number{ 0 };
    const CounterType m_min{ 0 };
    const CounterType m_max{ 0 };
};

/**
 * @brief A heavy-weight metrics accumulator that stores all values for precise calculations
 * @tparam T Value type for metrics
 */
template <typename T>
class MetricsHolder<MetricsAccumulatePolicy::HeavyWay, T> final {
public:
    using CounterType = T;

    /**
     * @brief Construct a new MetricsHolder object
     * @param min Minimum value to track
     * @param max Maximum value to track
     */
    explicit MetricsHolder(const T min, const T max);

    /**
     * @brief Add a value to the histogram
     * @param value The value to add
     * @throws std::runtime_error if value is outside [min,max] range
     */
    void add(const T value);

    /**
     * @brief Measure execution time of a function and add it to the histogram
     * @tparam F Callable type
     * @param f Function to measure
     */
    template <typename F> void measure(F &&f);

    /**
     * @brief Get the minimum tracked value
     * @return CounterType Minimum value
     */
    CounterType get_min() const noexcept;

    /**
     * @brief Get the maximum tracked value
     * @return CounterType Maximum value
     */
    CounterType get_max() const noexcept;

    /**
     * @brief Get total number of elements in histogram
     * @return CounterType Total count of elements
     */
    CounterType get_elements() const noexcept;

    /**
     * @brief Get count of elements less than or equal to budget
     * @param budget The threshold value
     * @return CounterType Number of elements ≤ budget
     */
    T get_count(const T budget) const;

    /**
     * @brief Calculate Cumulative Distribution Function value
     * @param percent The threshold value
     * @return double Percentage of elements ≤ percent (0-100)
     * @example CDF(25) = 66.666% (66.666% data ≤ 25)
     */
    double get_CDF(const T percent) const;

    /**
     * @brief Calculate percentile value
     * @param p Percentile to calculate (0-100)
     * @return double The value at p-th percentile
     */
    double get_percentile(const T percentile) const;

private:
    mutable std::mutex m_mutex;
    std::vector<T> m_data;
    const T m_min{ 0 };
    const T m_max{ 0 };
};

/**
 * @brief A three-zone histogram distributor policy
 * @tparam N Total number of buckets in the histogram
 * @tparam T Value type for distribution
 * @details Distributes values into three zones with different bucket densities:
 * - Zone 1 (60% buckets): 0-1000 range (high resolution)
 * - Zone 2 (30% buckets): 1000-2000 range (medium resolution)
 * - Zone 3 (10% buckets): 2000-max range (low resolution)
 */
template <std::size_t N, typename T> struct ThreeZoneDistributor {
private:
    static constexpr T ZONE1_END = 1000;
    static constexpr T ZONE2_END = 2000;

    static constexpr std::size_t ZONE1_BUCKETS = N * 6 / 10; // 50%
    static constexpr std::size_t ZONE2_BUCKETS = N * 3 / 10; // 30%
    static constexpr std::size_t ZONE3_BUCKETS =
        N - ZONE1_BUCKETS - ZONE2_BUCKETS; // 20%

public:
    /// Total number of buckets in the histogram
    static constexpr auto BUCKET_NUMBER = N;

    /**
     * @brief Distributes a value into the appropriate bucket
     * @param value The value to distribute
     * @param min Minimum value in histogram range
     * @param max Maximum value in histogram range
     * @return std::size_t Index of the bucket for this value
     * @throws std::runtime_error if value is outside [min,max] range
     */
    std::size_t distribute(const T value, const T min, const T max) const
    {
        if (value < min || value > max) {
            throw std::runtime_error("Value out of range");
        }

        if (value <= ZONE1_END) {
            // Zone 1: 0-1000
            const T bucket_width = ZONE1_END / ZONE1_BUCKETS;
            const auto bucket_index =
                static_cast<std::size_t>(value / bucket_width);
            return std::min(bucket_index, ZONE1_BUCKETS - 1);
        } else if (value <= ZONE2_END) {
            // Zone 2: 1000-2000
            const T range = ZONE2_END - ZONE1_END;
            const T bucket_width = range / ZONE2_BUCKETS;
            const T normalized_value = value - ZONE1_END;
            const auto bucket_index =
                ZONE1_BUCKETS +
                static_cast<std::size_t>(normalized_value / bucket_width);
            return std::min(bucket_index, ZONE1_BUCKETS + ZONE2_BUCKETS - 1);
        } else {
            // Zone 3: 2000-max
            const T range = max - ZONE2_END;
            const T bucket_width = range / ZONE3_BUCKETS;
            const T normalized_value = value - ZONE2_END;
            const auto bucket_index =
                ZONE1_BUCKETS + ZONE2_BUCKETS +
                static_cast<std::size_t>(normalized_value / bucket_width);
            return std::min(bucket_index, N - 1);
        }
    }

    /**
     * @brief Gets the maximum value contained in a bucket
     * @param bucket_index Index of the bucket (0 to N-1)
     * @param min Minimum value in histogram range
     * @param max Maximum value in histogram range
     * @return T Maximum value that would fall into this bucket
     * @throws std::runtime_error if bucket_index is out of range
     */
    T max_value_of(const std::size_t bucket_index, const T min,
                   const T max) const
    {
        if (bucket_index >= N)
            throw std::runtime_error("Bucket index out of range");

        if (bucket_index < ZONE1_BUCKETS) {
            // Zone 1
            const T bucket_width = ZONE1_END / ZONE1_BUCKETS;
            return bucket_width * (bucket_index + 1);
        } else if (bucket_index < ZONE1_BUCKETS + ZONE2_BUCKETS) {
            // Zone 2
            const std::size_t zone2_index = bucket_index - ZONE1_BUCKETS;
            const T bucket_width = (ZONE2_END - ZONE1_END) / ZONE2_BUCKETS;
            return ZONE1_END + bucket_width * (zone2_index + 1);
        } else {
            // Zone 3
            const std::size_t zone3_index =
                bucket_index - ZONE1_BUCKETS - ZONE2_BUCKETS;
            const T bucket_width = (max - ZONE2_END) / ZONE3_BUCKETS;
            return ZONE2_END + bucket_width * (zone3_index + 1);
        }
    }

    /**
     * @brief Gets the minimum value contained in a bucket
     * @param bucket_index Index of the bucket (0 to N-1)
     * @param min Minimum value in histogram range
     * @param max Maximum value in histogram range
     * @return T Minimum value that would fall into this bucket
     * @throws std::runtime_error if bucket_index is out of range
     */
    T min_value_of(const std::size_t bucket_index, const T min,
                   const T max) const
    {
        if (bucket_index >= N)
            throw std::runtime_error("Bucket index out of range");
        if (bucket_index == 0)
            return min;
        return max_value_of(bucket_index - 1, min, max);
    }
};

template <typename T, typename D>
MetricsHolder<MetricsAccumulatePolicy::HistogramWay, T, D>::MetricsHolder(
    const T min, const T max)
    : m_buckets()
    , m_elements_number(0)
    , m_min(min)
    , m_max(max)
{
}

template <typename T, typename D>
void MetricsHolder<MetricsAccumulatePolicy::HistogramWay, T, D>::add(
    const T value)
{
    if (value >= m_min && value <= m_max) {
        const auto index =
            DistributorPolicyType::distribute(value, m_min, m_max);
        if (index < m_buckets.size()) {
            m_buckets[index].fetch_add(1);
            m_elements_number.fetch_add(1);
        } else {
            throw std::runtime_error(
                "MetricsHolder: Value must be in range [0, backets.size())");
        }
    } else {
        throw std::runtime_error(
            "MetricsHolder: Value must be in range [min, max]: value=" +
            std::to_string(value));
    }
}

template <typename T, typename D>
template <typename F>
void MetricsHolder<MetricsAccumulatePolicy::HistogramWay, T, D>::measure(F &&f)
{
    using namespace __metrics_holder_impl_details;
    const auto start_time = get_current_time_in_ns();
    std::invoke(std::forward<F>(f));
    auto end_time = get_current_time_in_ns();
    const auto diff = end_time - start_time;
    assert(diff > 0);
    add(diff);
}

template <typename T, typename D>
typename MetricsHolder<MetricsAccumulatePolicy::HistogramWay, T, D>::CounterType
MetricsHolder<MetricsAccumulatePolicy::HistogramWay, T, D>::get_min()
    const noexcept
{
    return m_min;
}

template <typename T, typename D>
typename MetricsHolder<MetricsAccumulatePolicy::HistogramWay, T, D>::CounterType
MetricsHolder<MetricsAccumulatePolicy::HistogramWay, T, D>::get_max()
    const noexcept
{
    return m_max;
}

template <typename T, typename D>
typename MetricsHolder<MetricsAccumulatePolicy::HistogramWay, T, D>::CounterType
MetricsHolder<MetricsAccumulatePolicy::HistogramWay, T, D>::get_elements()
    const noexcept
{
    return m_elements_number.load();
}

template <typename T, typename D>
typename MetricsHolder<MetricsAccumulatePolicy::HistogramWay, T, D>::CounterType
MetricsHolder<MetricsAccumulatePolicy::HistogramWay, T, D>::get_count(
    const CounterType budget) const
{
    const auto bucket_index =
        DistributorPolicyType::distribute(budget, get_min(), get_max());
    T count = 0;
    for (auto i = 0; i <= bucket_index; ++i) {
        count += m_buckets[i].load();
    }
    return count;
}

template <typename T, typename D>
double MetricsHolder<MetricsAccumulatePolicy::HistogramWay, T, D>::get_CDF(
    const CounterType percent) const
{
    return (static_cast<double>(get_count(percent)) /
            static_cast<double>(get_elements()) * 100.0);
}

template <typename T, typename D>
double
MetricsHolder<MetricsAccumulatePolicy::HistogramWay, T, D>::get_percentile(
    const CounterType p) const
{
    const auto max = get_max();
    const auto min = get_min();
    const auto elements = get_elements();
    const auto get_max_bucket_value = [min, max, this] {
        return DistributorPolicyType::max_value_of(m_buckets.size() - 1, min,
                                                   max);
    };
    if (elements == 0) {
        return 0.0;
    }
    if (p <= 0) {
        return min;
    }
    if (p >= 100) {
        return get_max_bucket_value();
    }

    const double target_count = static_cast<double>(p) / 100 * elements;
    double count = 0;
    for (auto i = 0; i < m_buckets.size(); ++i) {
        const auto bucket_count = static_cast<double>(m_buckets[i].load());
        count += bucket_count;
        if (count >= target_count) {
            const auto max_value =
                DistributorPolicyType::max_value_of(i, min, max);
            const auto min_value =
                DistributorPolicyType::min_value_of(i, min, max);
            return (max_value + min_value) / 2;
        }
    }
    return get_max_bucket_value();
}

template <typename T>
MetricsHolder<MetricsAccumulatePolicy::HeavyWay, T>::MetricsHolder(T min, T max)
    : m_mutex()
    , m_data()
    , m_min(min)
    , m_max(max)
{
}

template <typename T>
void MetricsHolder<MetricsAccumulatePolicy::HeavyWay, T>::add(T value)
{
    std::lock_guard lock{ m_mutex };
    m_data.push_back(value);
}

template <typename T>
template <typename F>
void MetricsHolder<MetricsAccumulatePolicy::HeavyWay, T>::measure(F &&f)
{
    using namespace __metrics_holder_impl_details;
    const auto start_time = get_current_time_in_ns();
    std::invoke(std::forward<F>(f));
    const auto end_time = get_current_time_in_ns();
    const auto diff = end_time - start_time;
    assert(diff > 0);
    add(diff);
}

template <typename T>
T MetricsHolder<MetricsAccumulatePolicy::HeavyWay, T>::get_min() const noexcept
{
    return m_min;
}

template <typename T>
T MetricsHolder<MetricsAccumulatePolicy::HeavyWay, T>::get_max() const noexcept
{
    return m_max;
}

template <typename T>
T MetricsHolder<MetricsAccumulatePolicy::HeavyWay, T>::get_elements()
    const noexcept
{
    std::lock_guard lock{ m_mutex };
    return m_data.size();
}

template <typename T>
T MetricsHolder<MetricsAccumulatePolicy::HeavyWay, T>::get_count(T budget) const
{
    std::lock_guard lock{ m_mutex };
    return static_cast<T>(get_CDF(budget) / 100.0 * m_data.size());
}

template <typename T>
double
MetricsHolder<MetricsAccumulatePolicy::HeavyWay, T>::get_CDF(T percent) const
{
    std::lock_guard lock{ m_mutex };
    if (m_data.empty() || percent <= m_min || percent >= m_max) {
        return 0.0;
    }
    const auto count =
        std::count_if(m_data.begin(), m_data.end(),
                      [percent](T val) { return val <= percent; });
    return (static_cast<double>(count) / m_data.size()) * 100.0;
}

template <typename T>
double MetricsHolder<MetricsAccumulatePolicy::HeavyWay, T>::get_percentile(
    T percentile) const
{
    std::vector<T> sorted_data{ m_data };
    {
        std::lock_guard lock{ m_mutex };
        if (m_data.empty()) {
            return 0.0;
        }
        if (percentile <= 0) {
            return m_min;
        }
        if (percentile >= 100) {
            return m_max;
        }

        sorted_data = std::vector<T>{ m_data };
    }
    std::sort(sorted_data.begin(), sorted_data.end());

    const auto rank =
        static_cast<double>(percentile) / 100.0 * (sorted_data.size() - 1);
    const auto lower_index = static_cast<size_t>(std::floor(rank));
    const auto remainder = rank - lower_index;

    if (remainder < std::numeric_limits<double>::epsilon()) {
        return sorted_data[lower_index];
    } else {
        return sorted_data[lower_index] +
               remainder *
                   (sorted_data[lower_index + 1] - sorted_data[lower_index]);
    }
}

#endif //! METRICS_HOLDER_H
