#ifndef ATOM_SFINAE_ITERATOR_TRAITS_H
#define ATOM_SFINAE_ITERATOR_TRAITS_H

#include "include/sfinae/has_type_operators.h"

namespace atom::iter {

/**
 * @namespace atom::iter
 * @brief Namespace containing iterator-related type traits and utilities.
 *
 * @file iterator_traits.h
 * @brief Modern SFINAE-based iterator traits system
 * @namespace atom::iter
 *
 * @defgroup IteratorTraits Iterator Traits System
 * @brief Modern alternative to std::iterator_traits with SFINAE-based detection
 *
 * @section Motivation and Architectural Differences from STL
 *
 * This iterator traits system provides a fundamentally different approach from the
 * standard STL iterator_traits, focusing on behavioral detection rather than
 * nominal tagging and manual specialization.
 *
 * @subsection stl_approach STL Approach (Traditional)
 * - Relies on explicit tagging through inheritance from iterator categories
 * - Requires manual specialization of iterator_traits for custom types
 * - Based on nominal typing: iterator must explicitly declare its category
 * - Limited to predefined categories (input, output, forward, etc.)
 * - No built-in capability detection - assumes declared category is correct
 *
 * @subsection atom_approach Atom Approach (Modern)
 * - SFINAE-based automatic detection of iterator capabilities
 * - Behavioral typing: detects what operations an iterator actually supports
 * - No need for explicit inheritance or manual tagging
 * - Fine-grained capability checking
 * - Compile-time safety through concept-like checks
 * - Extensible system for custom iterator properties
 *
 * @subsection advantages Advantages of This Approach
 * - Automatic: No need for manual trait specialization in most cases
 * - Accurate: Actually verifies operations rather than trusting declarations
 * - Fine-grained: Check specific capabilities rather than broad categories
 * - Safe: Prevents category mismatches (e.g., random access iterator that can't do arithmetic)
 * - Modern: Uses C++17/20 features (constexpr if, void_t, detection idioms)
 * - Educational: Clearly shows what operations define each iterator category
 *
 * @subsection limitations Current Limitations
 * - Contiguous iterator detection requires explicit specialization (like STL)
 * - Proxy iterators (like std::vector<bool>) may need special handling
 * - Slightly heavier compile-time due to extensive SFINAE checking
 * - More complex implementation than traditional tag-based approach
 *
 *
 * @example Implementing Custom Iterators
 *
 * @section input_iterator Input Iterator Example
 *
 * Minimal input iterator implementation:
 * @code{.cpp}
 * struct InputIter {
 *     // Required: const dereference for reading
 *     const_reference operator*() const;
 *
 *     // Required: prefix increment
 *     InputIter& operator++();
 *
 *     // Required: postfix increment
 *     InputIter operator++(int);
 *
 *     // Required: equality comparison
 *     friend bool operator==(const InputIter& lhs, const InputIter& rhs);
 *     friend bool operator!=(const InputIter& lhs, const InputIter& rhs);
 * };
 * @endcode
 *
 * @section output_iterator Output Iterator Example
 *
 * Minimal output iterator implementation:
 * @code{.cpp}
 * struct OutputIter {
 *     // Required: non-const dereference for writing
 *     reference operator*();
 *
 *     // Required: prefix increment
 *     OutputIter& operator++();
 *
 *     // Required: postfix increment
 *     OutputIter operator++(int);
 * };
 * @endcode
 *
 * @section forward_iterator Forward Iterator Example
 *
 * Minimal forward iterator implementation (supports multi-pass):
 * @code{.cpp}
 * struct ForwardIter {
 *     ForwardIter() = default;
 *
 *     // Required: const dereference for reading
 *     const_reference operator*() const;
 *
 *     // Required: non-const dereference for writing
 *     reference operator*();
 *
 *     // Required: prefix increment
 *     ForwardIter& operator++();
 *
 *     // Required: postfix increment
 *     ForwardIter operator++(int);
 *
 *     // Required: equality comparison
 *     friend bool operator==(const ForwardIter& lhs, const ForwardIter& rhs);
 *     friend bool operator!=(const ForwardIter& lhs, const ForwardIter& rhs);
 * };
 * @endcode
 *
 * @section bidirectional_iterator Bidirectional Iterator Example
 *
 * Minimal bidirectional iterator implementation:
 * @code{.cpp}
 * struct BidirectionalIter {
 *     BidirectionalIter() = default;
 *
 *     // Required: const/non-const dereference
 *     const_reference operator*() const;
 *     reference operator*();
 *
 *     // Required: forward increment operations
 *     BidirectionalIter& operator++();
 *     BidirectionalIter operator++(int);
 *
 *     // Required: backward decrement operations
 *     BidirectionalIter& operator--();
 *     BidirectionalIter operator--(int);
 *
 *     // Required: equality comparison
 *     friend bool operator==(const BidirectionalIter& lhs, const BidirectionalIter& rhs);
 *     friend bool operator!=(const BidirectionalIter& lhs, const BidirectionalIter& rhs);
 * };
 * @endcode
 *
 * @section random_access_iterator Random Access Iterator Example
 *
 * Minimal random access iterator implementation:
 * @code{.cpp}
 * struct RandomAccessIter {
 *     // Required: default constructor
 *     RandomAccessIter() = default;
 *
 *     // Required: const/non-const dereference
 *     const_reference operator*() const;
 *     reference operator*();
 *
 *     // Required: subscript access
 *     const_reference operator[](difference_type n) const;
 *     reference operator[](difference_type n);
 *
 *     // Required: increment/decrement operations
 *     RandomAccessIter& operator++();
 *     RandomAccessIter operator++(int);
 *     RandomAccessIter& operator--();
 *     RandomAccessIter operator--(int);
 *
 *     // Required: arithmetic operations
 *     RandomAccessIter& operator+=(difference_type n);
 *     RandomAccessIter& operator-=(difference_type n);
 *     RandomAccessIter operator+(difference_type n) const;
 *     RandomAccessIter operator-(difference_type n) const;
 *     difference_type operator-(const RandomAccessIter& other) const;
 *
 *     // Required: comparison operations
 *     friend bool operator==(const RandomAccessIter& lhs, const RandomAccessIter& rhs);
 *     friend bool operator!=(const RandomAccessIter& lhs, const RandomAccessIter& rhs);
 *     friend bool operator<(const RandomAccessIter& lhs, const RandomAccessIter& rhs);
 *     friend bool operator<=(const RandomAccessIter& lhs, const RandomAccessIter& rhs);
 *     friend bool operator>(const RandomAccessIter& lhs, const RandomAccessIter& rhs);
 *     friend bool operator>=(const RandomAccessIter& lhs, const RandomAccessIter& rhs);
 * };
 * @endcode
 *
 * @section contiguous_iterator Contiguous Iterator Hint
 *
 * To mark an iterator as contiguous (for optimization purposes):
 * @code{.cpp}
 * // Specialize IsContiguousIterator for your type
 * template<>
 * struct atom::iter::IsContiguousIterator<MyContiguousIterator> : std::true_type {};
 *
 * // Or inherit from a tag (optional, for documentation)
 * struct MyContiguousIterator : atom::iter::RandomAccessIteratorTag {
 *     // ... iterator implementation ...
 * };
 * @endcode
 *
 * @section native_pointers Native Pointers as Iterators
 *
 * All native pointer types are automatically detected as contiguous random access iterators:
 * @code{.cpp}
 * int* ptr = ... ;
 * static_assert(atom::iter::isRandomAccessIterator<int*>); // true
 * static_assert(atom::iter::isContiguousIterator<int*>);   // true
 * static_assert(atom::iter::isOnWriteDerefableIterator<int*>); // true
 * static_assert(atom::iter::isOnReadDerefableIterator<const int*>); // true
 * @endcode
 *
 * @section proxy_iterators Proxy Iterators Note
 *
 * For proxy iterators (like std::vector<bool>::iterator), the system correctly
 * detects the returned proxy type without special handling:
 * @code{.cpp}
 * struct ProxyIterator {
 *     ProxyReference operator*(); // not a real reference
 *     // ...
 * };
 *
 * // ValueType will be ProxyReference, not the underlying value type
 * // This is intentional behavior - algorithms should use IteratorTraits::ValueType
 * @endcode
 */

template<typename Iterator>
class IteratorTraits;

/**
 * @var template<typename Iterator> inline constexpr bool isOnReadDerefableIterator
 * @brief Checks if the iterator can be dereferenced for reading (const-qualified dereference).
 */
template<typename Iterator>
inline constexpr bool isOnReadDerefableIterator = IteratorTraits<Iterator>::isOnReadDerefableIterator();

/**
 * @var template<typename Iterator> inline constexpr bool isOnWriteDerefableIterator
 * @brief Checks if the iterator can be dereferenced for writing (non-const dereference).
 */
template<typename Iterator>
inline constexpr bool isOnWriteDerefableIterator = IteratorTraits<Iterator>::isOnWriteDerefableIterator();

/**
 * @var template<typename Iterator> inline constexpr bool isDerefableIterator
 * @brief Checks if the iterator can be dereferenced (either for reading or writing).
 */
template<typename Iterator>
inline constexpr bool isDerefableIterator = IteratorTraits<Iterator>::isDerefableIterator();

/**
 * @var template<typename Iterator> inline constexpr bool isEqableIterator
 * @brief Checks if the iterator supports equality comparison (== and != operators).
 */
template<typename Iterator>
inline constexpr bool isEqableIterator = IteratorTraits<Iterator>::isEqableIterator();

/**
 * @var template<typename Iterator> inline constexpr bool isComparableIterator
 * @brief Checks if the iterator supports both equality and ordering comparisons.
 */
template<typename Iterator>
inline constexpr bool isComparableIterator = IteratorTraits<Iterator>::isComparableIterator();

/**
 * @var template<typename Iterator> inline constexpr bool isIncrementableIterator
 * @brief Checks if the iterator can be incremented (prefix or postfix ++ operator).
 */
template<typename Iterator>
inline constexpr bool isIncrementableIterator = IteratorTraits<Iterator>::isIncrementableIterator();

/**
 * @var template<typename Iterator> inline constexpr bool isDecrementableIterator
 * @brief Checks if the iterator can be decremented (prefix or postfix -- operator).
 */
template<typename Iterator>
inline constexpr bool isDecrementableIterator = IteratorTraits<Iterator>::isDecrementableIterator();

/**
 * @var template<typename Iterator> inline constexpr bool isRandomIncrementableIterator
 * @brief Checks if the iterator supports random increment (operator+ with DifferenceType).
 */
template<typename Iterator>
inline constexpr bool isRandomIncrementableIterator = IteratorTraits<Iterator>::isRandomIncrementableIterator();

/**
 * @var template<typename Iterator> inline constexpr bool isRandomDecrementableIterator
 * @brief Checks if the iterator supports random decrement (operator- with DifferenceType).
 */
template<typename Iterator>
inline constexpr bool isRandomDecrementableIterator = IteratorTraits<Iterator>::isRandomDecrementableIterator();

/**
 * @var template<typename Iterator> inline constexpr bool isArithmeticableIterator
 * @brief Checks if the iterator supports both random increment and decrement.
 */
template<typename Iterator>
inline constexpr bool isArithmeticableIterator = IteratorTraits<Iterator>::isArithmeticableIterator();

/**
 * @var template<typename Iterator> inline constexpr bool isInputIterator
 * @brief Checks if the iterator satisfies input iterator requirements.
 */
template<typename Iterator>
inline constexpr bool isInputIterator = IteratorTraits<Iterator>::isInputIterator();

/**
 * @var template<typename Iterator> inline constexpr bool isOutputIterator
 * @brief Checks if the iterator satisfies output iterator requirements.
 */
template<typename Iterator>
inline constexpr bool isOutputIterator = IteratorTraits<Iterator>::isOutputIterator();

/**
 * @var template<typename Iterator> inline constexpr bool isForwardIterator
 * @brief Checks if the iterator satisfies forward iterator requirements.
 */
template<typename Iterator>
inline constexpr bool isForwardIterator = IteratorTraits<Iterator>::isForwardIterator();

/**
 * @var template<typename Iterator> inline constexpr bool isBidirectionalIterator
 * @brief Checks if the iterator satisfies bidirectional iterator requirements.
 */
template<typename Iterator>
inline constexpr bool isBidirectionalIterator = IteratorTraits<Iterator>::isBidirectionalIterator();

/**
 * @var template<typename Iterator> inline constexpr bool isRandomAccessIterator
 * @brief Checks if the iterator satisfies random access iterator requirements.
 */
template<typename Iterator>
inline constexpr bool isRandomAccessIterator = IteratorTraits<Iterator>::isRandomAccessIterator();

/**
 * @var template<typename Iterator> inline constexpr bool isUnknownIterator
 * @brief Checks if the iterator doesn't fit any standard category.
 */
template<typename Iterator>
inline constexpr bool isUnknownIterator = IteratorTraits<Iterator>::isUnknownIterator();

/**
 * @class IteratorTraits
 * @brief Traits class that provides information about iterator properties and capabilities.
 *
 * @tparam Iterator The iterator type to analyze.
 *
 * This class provides:
 * - Type aliases (ValueType, DifferenceType, etc.)
 * - Static boolean properties indicating iterator capabilities
 * - Compile-time checks for iterator category
 */
template<typename It>
class IteratorTraits {
private:
    struct Node {};

    template<typename T, typename = void>
    struct DifferenceTypeGetter {
        using Type = std::ptrdiff_t;
    };

    template<typename T>
    struct DifferenceTypeGetter<T, std::void_t<typename T::DifferenceType>> {
        using Type = typename T::DifferenceType;
    };

    template<typename T>
    struct DifferenceTypeGetter<T, std::void_t<decltype(std::declval<T>() - std::declval<T>())>> {
        using Type = decltype(std::declval<T>() - std::declval<T>());
    };

    template<typename T, typename = void>
    struct ValueTypeGetter {
        using Type = Node;
    };

    template<typename T>
    struct ValueTypeGetter<T, std::void_t<decltype(*std::declval<T>())>> {
        using Type = std::remove_cvref_t<decltype(*std::declval<T>())>;
    };

    template<typename T>
    struct ValueTypeGetter<const T, std::void_t<decltype(*std::declval<const T>())>> {
        using Type = std::remove_cvref_t<decltype(*std::declval<const T>())>;
    };

    using IteratorType = It;
    using ConstIteratorType = const It;
    using ReferenceIteratorType = It&;
    using ConstReferenceIteratorType = const It&;

public:
    /**
     * @typedef Type
     * @brief The difference type of the iterator.
     *
     * Determined in this order:
     * 1. Iterator::DifferenceType member type
     * 2. Type returned by operator- between two iterators
     * 3. std::ptrdiff_t (default)
     */
    using DifferenceType = typename DifferenceTypeGetter<IteratorType>::Type;
    /**
     * @typedef ValueType
     * @brief The value type obtained by dereferencing the iterator.
     *
     * Determined from the return type of operator*, with cv-ref qualifiers removed.
     * If the iterator cannot be dereferenced, defaults to internal Node type.
     */
    using ValueType = typename ValueTypeGetter<IteratorType>::Type;
    //! Reference type
    using ReferenceType = ValueType&;
    //! Const reference type
    using ConstReferenceType = const ValueType&;
    //! Pointer type
    using PointerType = ValueType*;
    //! Const pointer type
    using ConstPointerType = const ValueType*;

    /**
     * @fn static constexpr bool isOnReadDerefableIterator() noexcept
     * @brief Checks if const iterator can be dereferenced for reading.
     *
     * @return true if const iterator has operator*() returning a const reference.
     */
    static constexpr bool isOnReadDerefableIterator() noexcept {
        return has_operator_deref_v<ConstIteratorType, ConstReferenceType>;
    }

    /**
     * @fn static constexpr bool isOnWriteDerefableIterator() noexcept
     * @brief Checks if iterator can be dereferenced for writing.
     *
     * @return true if iterator has operator*() returning a non-const reference.
     */
    static constexpr bool isOnWriteDerefableIterator() noexcept {
        return has_operator_deref_v<IteratorType, ReferenceType>;
    }

    /**
     * @fn static constexpr bool isDerefableIterator() noexcept
     * @brief Checks if iterator can be dereferenced at all.
     *
     * @return true if either readable or writable.
     */
    static constexpr bool isDerefableIterator() noexcept {
        return (isOnReadDerefableIterator() || isOnWriteDerefableIterator());
    }

    /**
     * @fn static constexpr bool isIncrementableIterator() noexcept
     * @brief Checks if iterator supports increment operations.
     *
     * @return true if has prefix++ or postfix++ operator.
     */
    static constexpr bool isIncrementableIterator() noexcept {
        // It& operator++();
        // It operator++(int);
        return (has_operator_prefix_plus_plus_v<IteratorType, ReferenceIteratorType> ||
                has_operator_postfix_plus_plus_v<IteratorType, IteratorType>);
    }

    /**
     * @fn static constexpr bool isDecrementableIterator() noexcept
     * @brief Checks if iterator supports decrement operations.
     *
     * @return true if has prefix-- or postfix-- operator.
     */
    static constexpr bool isDecrementableIterator() noexcept {
        // It& operator--();
        // It operator--(int);
        return (has_operator_prefix_minus_minus_v<IteratorType, ReferenceIteratorType> ||
                has_operator_postfix_minus_minus_v<IteratorType, IteratorType>);
    }

    /**
     * @fn static constexpr bool isRandomIncrementableIterator() noexcept
     * @brief Checks if iterator supports random increment.
     *
     * @return true if has operator+ with DifferenceType parameter.
     */
    static constexpr bool isRandomIncrementableIterator() noexcept {
        // It operator+(difference_type) const;
        // It& operator+=(difference_type);
        return ((has_operator_plus_v<IteratorType, ReferenceIteratorType, const DifferenceType&> ||
                 has_operator_plus_v<IteratorType, ReferenceIteratorType, DifferenceType>) &&
                (has_operator_nonmodify_plus_v<ConstIteratorType, IteratorType, DifferenceType> ||
                 has_operator_nonmodify_plus_v<ConstIteratorType, IteratorType, const DifferenceType&>));
    }

    /**
     * @fn static constexpr bool isRandomDecrementableIterator() noexcept
     * @brief Checks if iterator supports random decrement.
     *
     * @return true if has operator- with DifferenceType parameter.
     */
    static constexpr bool isRandomDecrementableIterator() noexcept {
        // It operator-(difference_type) const;
        // It& operator-=(difference_type);
        // difference_type operator-(It) const;
        return ((has_operator_minus_v<IteratorType, ReferenceIteratorType, const DifferenceType&> ||
                 has_operator_minus_v<IteratorType, ReferenceIteratorType, DifferenceType>) &&
                (has_operator_nonmodify_minus_v<ConstIteratorType, IteratorType, DifferenceType> ||
                 has_operator_nonmodify_minus_v<ConstIteratorType, IteratorType, const DifferenceType&>) &&
                (has_operator_nonmodify_minus_v<ConstIteratorType, DifferenceType, IteratorType> ||
                 has_operator_nonmodify_minus_v<ConstIteratorType, DifferenceType, ConstIteratorType&>));
    }

    /**
     * @fn static constexpr bool isArithmeticableIterator() noexcept
     * @brief Checks if iterator supports both random increment and decrement.
     *
     * @return true if both random increment and decrement are supported.
     */
    static constexpr bool isArithmeticableIterator() noexcept {
        return (isRandomIncrementableIterator() && isRandomDecrementableIterator());
    }

    /**
     * @fn static constexpr bool isEqableIterator() noexcept
     * @brief Checks if iterator supports equality comparisons.
     *
     * @return true if has == or != operators.
     */
    static constexpr bool isEqableIterator() noexcept {
        // bool operator==(It) const;
        // bool operator!=(It) const;
        return ((has_operator_eq_v<ConstIteratorType, ConstReferenceIteratorType> ||
                has_operator_eq_v<ConstIteratorType, IteratorType>) ||
                (has_operator_not_eq_v<ConstIteratorType, ConstReferenceIteratorType&> ||
                has_operator_not_eq_v<ConstIteratorType, IteratorType>));
    }

    /**
     * @fn static constexpr bool isOrderableIterator() noexcept
     * @brief Checks if iterator supports ordering comparisons.
     *
     * @return true if has <, >, <=, or >= operators.
     */
    static constexpr bool isOrderableIterator() noexcept {
        // bool operator<(It) const;
        // bool operator<=(It) const;
        // bool operator>(It) const;
        // bool operator>=(It) const;
        return (has_operator_less_v<ConstIteratorType, ConstIteratorType> ||
                has_operator_less_v<ConstIteratorType, IteratorType> ||
                has_operator_greater_v<ConstIteratorType, ConstReferenceIteratorType> ||
                has_operator_greater_v<ConstIteratorType, IteratorType>);
    }

    /**
     * @fn static constexpr bool isComparableIterator() noexcept
     * @brief Checks if iterator supports both equality and ordering comparisons.
     *
     * @return true if both equality and ordering comparisons are supported.
     */
    static constexpr bool isComparableIterator() noexcept {
        return (isEqableIterator() && isOrderableIterator());
    }

    /**
     * @fn static constexpr bool isInputIterator() noexcept
     * @brief Checks if iterator meets input iterator requirements.
     *
     * @return true if incrementable and readable.
     */
    static constexpr bool isInputIterator() noexcept {
        return (isIncrementableIterator() &&
                isEqableIterator() &&
                isOnReadDerefableIterator());
    }

    /**
     * @fn static constexpr bool isOutputIterator() noexcept
     * @brief Checks if iterator meets output iterator requirements.
     *
     * @return true if incrementable and writable.
     */
    static constexpr bool isOutputIterator() noexcept {
        return (isIncrementableIterator() && isOnWriteDerefableIterator());
    }

    /**
     * @fn static constexpr bool isForwardIterator() noexcept
     * @brief Checks if iterator meets forward iterator requirements.
     *
     * @return true if incrementable, comparable, readable, and writable.
     */
    static constexpr bool isForwardIterator() noexcept {
        return (isIncrementableIterator() &&
                isEqableIterator() &&
                isOnReadDerefableIterator() &&
                isOnWriteDerefableIterator());
    }

    /**
     * @fn static constexpr bool isBidirectionalIterator() noexcept
     * @brief Checks if iterator meets bidirectional iterator requirements.
     *
     * @return true if incrementable, decrementable, comparable, readable, and writable.
     */
    static constexpr bool isBidirectionalIterator() noexcept {
        return (isIncrementableIterator() &&
                isDecrementableIterator() &&
                isEqableIterator() &&
                isOnReadDerefableIterator() &&
                isOnWriteDerefableIterator());
    }

    /**
     * @fn static constexpr bool isRandomAccessIterator() noexcept
     * @brief Checks if iterator meets random access iterator requirements.
     *
     * @return true if supports arithmetic, comparisons, reading, and writing.
     */
    static constexpr bool isRandomAccessIterator() noexcept {
        return (isComparableIterator() &&
                isOnReadDerefableIterator() &&
                isOnWriteDerefableIterator() &&
                isArithmeticableIterator() &&
                (has_operator_square_brackets_v<IteratorType, ReferenceType> &&
                has_operator_square_brackets_v<ConstIteratorType, ConstReferenceType>));
    }

    /**
    * @fn static constexpr bool isUnknownIterator() noexcept
    * @brief Checks if iterator doesn't fit any standard category.
    *
    * @return true if not any of the standard iterator types.
    */
    static constexpr bool isUnknownIterator() noexcept {
        return (!isRandomAccessIterator() &&
                !isBidirectionalIterator() &&
                !isForwardIterator() &&
                !isOutputIterator() &&
                !isInputIterator());
    }
};

template<typename It>
class IteratorTraits<It*> {
public:
    using DifferenceType = std::ptrdiff_t;
    using ValueType = std::remove_pointer_t<It>;
    using ReferenceType = ValueType&;
    using ConstReferenceType = const ValueType&;
    using PointerType = ValueType*;
    using ConstPointerType = const ValueType*;

    static constexpr bool isOnReadDerefableIterator() noexcept {
        return true;
    }

    static constexpr bool isOnWriteDerefableIterator() noexcept {
        return true;
    }

    static constexpr bool isDerefableIterator() noexcept {
        return true;
    }

    static constexpr bool isIncrementableIterator() noexcept {
        return true;
    }

    static constexpr bool isDecrementableIterator() noexcept {
        return true;
    }

    static constexpr bool isRandomIncrementableIterator() noexcept {
        return true;
    }

    static constexpr bool isRandomDecrementableIterator() noexcept {
        return true;
    }

    static constexpr bool isArithmeticableIterator() noexcept {
        return true;
    }

    static constexpr bool isEqableIterator() noexcept {
        return true;
    }

    static constexpr bool isOrderableIterator() noexcept {
        return true;
    }

    static constexpr bool isComparableIterator() noexcept {
        return true;
    }

    static constexpr bool isInputIterator() noexcept {
        return true;
    }

    static constexpr bool isOutputIterator() noexcept {
        return true;
    }

    static constexpr bool isForwardIterator() noexcept {
        return true;
    }

    static constexpr bool isBidirectionalIterator() noexcept {
        return true;
    }

    static constexpr bool isRandomAccessIterator() noexcept {
        return true;
    }

    static constexpr bool isUnknownIterator() noexcept {
        return false;
    }
};

template<typename It>
class IteratorTraits<It const*> {
public:
    using DifferenceType = std::ptrdiff_t;
    using ValueType = std::remove_pointer_t<It>;
    using ReferenceType = ValueType&;
    using ConstReferenceType = const ValueType&;
    using PointerType = ValueType*;
    using ConstPointerType = const ValueType*;

    static constexpr bool isOnReadDerefableIterator() noexcept {
        return true;
    }

    static constexpr bool isOnWriteDerefableIterator() noexcept {
        return false;
    }

    static constexpr bool isDerefableIterator() noexcept {
        return true;
    }

    static constexpr bool isIncrementableIterator() noexcept {
        return true;
    }

    static constexpr bool isDecrementableIterator() noexcept {
        return true;
    }

    static constexpr bool isRandomIncrementableIterator() noexcept {
        return true;
    }

    static constexpr bool isRandomDecrementableIterator() noexcept {
        return true;
    }

    static constexpr bool isArithmeticableIterator() noexcept {
        return true;
    }

    static constexpr bool isEqableIterator() noexcept {
        return true;
    }

    static constexpr bool isOrderableIterator() noexcept {
        return true;
    }

    static constexpr bool isComparableIterator() noexcept {
        return true;
    }

    static constexpr bool isInputIterator() noexcept {
        return true;
    }

    static constexpr bool isOutputIterator() noexcept {
        return isOnWriteDerefableIterator();
    }

    static constexpr bool isForwardIterator() noexcept {
        return true;
    }

    static constexpr bool isBidirectionalIterator() noexcept {
        return true;
    }

    static constexpr bool isRandomAccessIterator() noexcept {
        return true;
    }

    static constexpr bool isUnknownIterator() noexcept {
        return false;
    }
};

template<typename It>
class IteratorTraits<It *const> {
public:
    using DifferenceType = std::ptrdiff_t;
    using ValueType = std::remove_pointer_t<It>;
    using ReferenceType = ValueType&;
    using ConstReferenceType = const ValueType&;
    using PointerType = ValueType*;
    using ConstPointerType = const ValueType*;

    static constexpr bool isOnReadDerefableIterator() noexcept {
        return true;
    }

    static constexpr bool isOnWriteDerefableIterator() noexcept {
        return true;
    }

    static constexpr bool isDerefableIterator() noexcept {
        return true;
    }

    static constexpr bool isIncrementableIterator() noexcept {
        return true;
    }

    static constexpr bool isDecrementableIterator() noexcept {
        return true;
    }

    static constexpr bool isRandomIncrementableIterator() noexcept {
        return true;
    }

    static constexpr bool isRandomDecrementableIterator() noexcept {
        return true;
    }

    static constexpr bool isArithmeticableIterator() noexcept {
        return true;
    }

    static constexpr bool isEqableIterator() noexcept {
        return true;
    }

    static constexpr bool isOrderableIterator() noexcept {
        return true;
    }

    static constexpr bool isComparableIterator() noexcept {
        return true;
    }

    static constexpr bool isInputIterator() noexcept {
        return true;
    }

    static constexpr bool isOutputIterator() noexcept {
        return isOnWriteDerefableIterator();
    }

    static constexpr bool isForwardIterator() noexcept {
        return true;
    }

    static constexpr bool isBidirectionalIterator() noexcept {
        return true;
    }

    static constexpr bool isRandomAccessIterator() noexcept {
        return true;
    }

    static constexpr bool isUnknownIterator() noexcept {
        return false;
    }
};

} //! namespace atom::iter

#endif //! ATOM_SFINAE_ITERATOR_TRAITS_H
