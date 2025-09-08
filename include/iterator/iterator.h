#ifndef ATOM_ITERATOR_H
#define ATOM_ITERATOR_H

#include "include/iterator/iterator_traits.h"
#include "include/utils/compiler_attr.h"

#include <cassert>

namespace atom::iter {

/**
 * @namespace atom::iter
 * @brief Namespace containing iterator utilities and algorithms
 *
 * Provides type-safe, optimized iterator operations with support for all
 * standard iterator categories. All functions are constexpr and optimized
 * for specific iterator capabilities.
 */

/**
 * @brief Advances an iterator by a specified distance
 *
 * @tparam Iterator Type of the iterator. Must meet the requirements of at least InputIterator.
 * @tparam Distance Type of the distance. Can be any integral type.
 *
 * @param[in,out] it Iterator to advance. Will be modified in-place.
 * @param[in] distance Number of positions to advance. Can be negative for bidirectional iterators.
 *
 * @note Time complexity depends on iterator category:
 *       - O(1) for RandomAccess and Contiguous iterators
 *       - O(|n|) for Bidirectional, Forward, and Input iterators
 *
 * @throws None (uses assert for precondition violations)
 *
 * @pre For non-bidirectional iterators, distance must be non-negative.
 * @pre Iterator must be dereferenceable if distance > 0.
 *
 * @post Iterator is advanced by exactly distance positions (if no bounds violated).
 */
template<typename Iterator, typename Distance>
constexpr void Advance(Iterator& it, Distance distance);

/**
 * @brief Calculates the number of elements between two iterators
 *
 * @tparam Iterator Type of the iterator. Must meet the requirements of at least InputIterator.
 *
 * @param[in] first Beginning of the range (inclusive)
 * @param[in] last End of the range (exclusive)
 *
 * @return typename sfinae::IteratorTraits<Iterator>::DifferenceType
 *         Number of elements in the range [first, last). Can be negative if last comes before first.
 *
 * @note Time complexity depends on iterator category:
 *       - O(1) for RandomAccess and Contiguous iterators
 *       - O(n) for Bidirectional, Forward, and Input iterators
 *
 * @throws None
 *
 * @pre [first, last) must be a valid range.
 * @pre For bidirectional iterators, the range direction can be determined.
 */
template<typename Iterator>
constexpr typename IteratorTraits<Iterator>::DifferenceType
Distance(Iterator first, Iterator last);

/**
 * @brief Returns an iterator advanced by n positions from the given iterator
 *
 * @tparam Iterator Type of the iterator. Must meet the requirements of at least InputIterator.
 *
 * @param[in] it Base iterator to advance from.
 * @param[in] n Number of positions to advance (default: 1).
 *
 * @return Iterator New iterator advanced by n positions.
 *
 * @note This function does not modify the original iterator.
 * @note For negative n, iterator must be bidirectional.
 *
 * @throws None (uses assert for precondition violations)
 *
 * @pre For non-bidirectional iterators, n must be non-negative.
 * @pre Iterator must allow advancement by n positions without going out of bounds.
 */
template<typename Iterator>
constexpr FORCE_INLINE Iterator
Next(Iterator it, typename IteratorTraits<Iterator>::DifferenceType n = 1);

/**
 * @brief Returns an iterator moved backwards by n positions from the given iterator
 *
 * @tparam Iterator Type of the iterator. Must be at least BidirectionalIterator.
 *
 * @param[in] it Base iterator to move backwards from.
 * @param[in] n Number of positions to move backwards (default: 1).
 *
 * @return Iterator New iterator moved backwards by n positions.
 *
 * @note This is equivalent to Next(it, -n) but more readable for backward movement.
 * @note Iterator must be bidirectional or random access.
 *
 * @throws None (uses assert for precondition violations)
 *
 * @pre Iterator must be bidirectional or random access.
 * @pre n must be non-negative for clarity (internally uses negative advancement).
 * @pre Iterator must allow backward movement by n positions.
 */
template<typename Iterator>
constexpr Iterator FORCE_INLINE
Prev(Iterator it, typename IteratorTraits<Iterator>::DifferenceType n = 1);

///////////////////////////// Impl /////////////////////////////
template<typename Iterator>
constexpr typename IteratorTraits<Iterator>::DifferenceType
Distance(Iterator first, Iterator last) {
    typename IteratorTraits<Iterator>::DifferenceType result{};
    if constexpr (isRandomAccessIterator<Iterator>) {
        result = last - first;
    } else {
        while (first != last) {
            ++first;
            ++result;
        }
    }
    return result;
}

template<typename Iterator>
constexpr Iterator FORCE_INLINE
Prev(Iterator it, typename IteratorTraits<Iterator>::DifferenceType n) {
    Advance(it, -n);
    return it;
}

template<typename Iterator>
constexpr Iterator FORCE_INLINE
Next(Iterator it, typename IteratorTraits<Iterator>::DifferenceType n) {
    Advance(it, n);
    return it;
}

template<typename Iterator, typename Distance>
constexpr void Advance(Iterator& it, Distance distance) {
    using namespace ::atom::iter;
    if constexpr (isRandomAccessIterator<Iterator>) {
        it += distance;
    } else if (isBidirectionalIterator<Iterator>) {
        if (distance > 0) {
            do {
                ++it; --distance;
            } while(distance > 0);
        } else if (distance < 0) {
            do {
                --it; ++distance;
            } while(distance < 0);
        }
    } else if (isInputIterator<Iterator> || isOutputIterator<Iterator>) {
        if constexpr (std::is_signed_v<Distance>) {
            if UNLIKELY_EXPR(distance < 0) {
                assert(false && "distance must be >= 0");
                return;
            }
        }
        while (distance > 0) {
            ++it;
            --distance;
        }
    } else {
        static_assert(isUnknownIterator<Iterator> && "Unknown iterator category");
    }
}

} //! namespace atom::iter

#endif //! ATOM_ITERATOR_H
