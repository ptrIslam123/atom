#ifndef ATOM_ALGORITHM_H
#define ATOM_ALGORITHM_H

namespace atom::algorithm {

template<typename T>
struct Less {
    constexpr bool operator()(const T& lhs, const T& rhs) const noexcept {
        return lhs < rhs;
    }
};

template<typename T>
constexpr const T& Clamp(const T& v, const T& lo, const T& hi);

template<typename T, typename Compare >
constexpr const T& clamp(const T& v, const T& lo, const T& hi, Compare compare);



template<typename T>
constexpr const T& Clamp(const T& v, const T& lo, const T& hi) {
    return clamp(v, lo, hi, Less<T>{});
}

template<typename T, typename Compare >
constexpr const T& clamp(const T& v, const T& lo, const T& hi, Compare compare) {
    return compare(v, lo) ? lo : compare(hi, v) ? hi : v;
}

} //! namespace atom::algorithm

#endif //! ATOM_ALGORITHM_H
