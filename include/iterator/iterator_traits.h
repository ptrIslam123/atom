#ifndef ATOM_SFINAE_ITERATOR_TRAITS_H
#define ATOM_SFINAE_ITERATOR_TRAITS_H

#include "include/sfinae/has_type_operators.h"
#include "include/sfinae/has_class_inner_type.h"

namespace atom::iter {

struct InputIteratorTag {};
struct OutputIteratorTag {};
struct ForwardIteratorTag {};
struct BidirectionalIteratorTag {};
struct RandomAccessIteratorTag {};
struct ContiguousIteratorTag {};
struct UnknownIteratorTag { UnknownIteratorTag() = delete; };

template<typename Iterator>
class IteratorTraits {
private:
    DECLARE_HAS_CLASS_INNER_TYPE(DifferenceType)
    DECLARE_HAS_CLASS_INNER_TYPE(ValueType)

    static constexpr bool hasDifferenceType = HAS_CLASS_INNERT_TYPE_DifferenceType<Iterator>::value;
    static constexpr bool hasValueType = HAS_CLASS_INNERT_TYPE_ValueType<Iterator>::value;

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
        using Type = void;
    };

    template<typename T>
    struct ValueTypeGetter<T, std::void_t<typename T::ValueType>> {
        using Type = typename T::ValueType;
    };

    template<typename T>
    struct ValueTypeGetter<T, std::void_t<decltype(*std::declval<T>())>> {
        using Type = std::remove_cvref_t<decltype(*std::declval<T>())>;
    };

    template<typename T>
    struct ValueTypeGetter<const T, std::void_t<decltype(*std::declval<const T>())>> {
        using Type = std::remove_cvref_t<decltype(*std::declval<const T>())>;
    };

public:
    using DifferenceType = typename DifferenceTypeGetter<Iterator>::Type;
    using ValueType = typename ValueTypeGetter<Iterator>::Type;
    using ReferenceType = ValueType&;
    using ConstReferenceType = const ValueType&;
    using PointerType = ValueType*;
    using ConstPointerType = const ValueType*;

    static constexpr bool isOnReadDerefableIterator() noexcept {
        return has_operator_deref_v<const Iterator, ConstReferenceType>;
    }

    static constexpr bool isOnWriteDerefableIterator() noexcept {
        return has_operator_deref_v<Iterator, ReferenceType>;
    }

    static constexpr bool isDerefableIterator() noexcept {
        return (isOnReadDerefableIterator() && isOnWriteDerefableIterator());
    }

    static constexpr bool isIncrementableIterator() noexcept {
        return (has_operator_prefix_plus_plus_v<Iterator, Iterator&> ||
                has_operator_postfix_plus_plus_v<Iterator, Iterator>);
    }

    static constexpr bool isRandomIncrementableIterator() noexcept {
        return (has_operator_plus_v<Iterator, Iterator, const DifferenceType&> ||
                has_operator_plus_v<Iterator, Iterator, DifferenceType>);
    }

    static constexpr bool isDecrementableIterator() noexcept {
        return (has_operator_prefix_minus_minus_v<Iterator, Iterator&> ||
                has_operator_postfix_minus_minus_v<Iterator, Iterator>);
    }

    static constexpr bool isRandomDecrementableIterator() noexcept {
        return (has_operator_minus_v<Iterator, Iterator, const DifferenceType&> ||
                has_operator_minus_v<Iterator, Iterator, DifferenceType>);
    }

    static constexpr bool isArithmeticableIterator() noexcept {
        return (isRandomIncrementableIterator() &&
                isRandomDecrementableIterator());
    }

    static constexpr bool isEqableIterator() noexcept {
        return (has_operator_eq_v<Iterator, const Iterator&> ||
                has_operator_eq_v<Iterator, Iterator> ||
                has_operator_not_eq_v<Iterator, const Iterator&> ||
                has_operator_not_eq_v<Iterator, Iterator>);
    }

    static constexpr bool isOrderableIterator() noexcept {
        return (has_operator_less_v<Iterator, const Iterator> ||
                has_operator_less_v<Iterator, Iterator> ||
                has_operator_greater_v<Iterator, const Iterator&> ||
                has_operator_greater_v<Iterator, Iterator>);
    }

    static constexpr bool isComparableIterator() noexcept {
        return (isEqableIterator() &&
                isOrderableIterator());
    }

    static constexpr bool isInputIterator() noexcept {
        return (isIncrementableIterator() &&
                isOnReadDerefableIterator() &&
                !isOnWriteDerefableIterator());
    }

    static constexpr bool isOutputIterator() noexcept {
        return (isIncrementableIterator() &&
                isOnWriteDerefableIterator() &&
                !isOnReadDerefableIterator());
    }

    static constexpr bool isForwardIterator() noexcept {
        return (isIncrementableIterator() &&
                isEqableIterator() &&
                isDerefableIterator());
    }

    static constexpr bool isBidirectionalIterator() noexcept {
        return (isIncrementableIterator() &&
                isDecrementableIterator() &&
                isEqableIterator() &&
                isDerefableIterator());
    }

    static constexpr bool isRandomAccessIterator() noexcept {
        return (isArithmeticableIterator() &&
                isComparableIterator() &&
                isDerefableIterator());
    }

    static constexpr bool isContiguousIterator() noexcept {
        return (isRandomAccessIterator() &&
                std::is_base_of_v<ContiguousIteratorTag, Iterator>);
    }

    static constexpr bool isUnknownIterator() noexcept {
        return (!isContiguousIterator() &&
                !isRandomAccessIterator() &&
                !isBidirectionalIterator() &&
                !isForwardIterator() &&
                !isOutputIterator() &&
                !isInputIterator());
    }
};

template<typename Iterator>
inline constexpr bool isOnReadDerefableIterator = IteratorTraits<Iterator>::isOnReadDerefableIterator();

template<typename Iterator>
inline constexpr bool isIncrementableIterator = IteratorTraits<Iterator>::isIncrementableIterator();

template<typename Iterator>
inline constexpr bool isDecrementableIterator = IteratorTraits<Iterator>::isDecrementableIterator();

template<typename Iterator>
inline constexpr bool isInputIterator = IteratorTraits<Iterator>::isInputIterator();

template<typename Iterator>
inline constexpr bool isOutputIterator = IteratorTraits<Iterator>::isOutputIterator();

template<typename Iterator>
inline constexpr bool isForwardIterator = IteratorTraits<Iterator>::isForwardIterator();

template<typename Iterator>
inline constexpr bool isBidirectionalIterator = IteratorTraits<Iterator>::isBidirectionalIterator();

template<typename Iterator>
inline constexpr bool isRandomAccessIterator = IteratorTraits<Iterator>::isRandomAccessIterator();

template<typename Iterator>
inline constexpr bool isContiguousIterator = IteratorTraits<Iterator>::isContiguousIterator();

template<typename Iterator>
inline constexpr bool isUnknownIterator = IteratorTraits<Iterator>::isUnknownIterator();

} //! namespace atom::iter

#endif //! ATOM_SFINAE_ITERATOR_TRAITS_H
