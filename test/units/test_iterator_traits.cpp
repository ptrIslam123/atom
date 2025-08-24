#include <gtest/gtest.h>

#include "include/iterator/iterator_traits.h"
#include "include/containers/fixed/array.h"

#include <type_traits>

using namespace atom::containers::fixed;
using namespace atom::iter;

using value_type = int; // for example
using difference_type = std::ptrdiff_t;
using pointer = value_type*;
using const_pointer = const value_type*;
using reference = value_type&;
using const_reference = const value_type&;

struct InputIter {
    const_reference operator*() const;
    InputIter& operator++();
    InputIter operator++(int);

    friend bool operator==(const InputIter& lhs, const InputIter& rhs);
    friend bool operator!=(const InputIter& lhs, const InputIter& rhs);
};

TEST(TestIteratorTraits, TestInputIter) {
    using Iterator = InputIter;
    EXPECT_TRUE((std::is_same_v<value_type, typename IteratorTraits<Iterator>::ValueType>));
    EXPECT_TRUE((std::is_same_v<difference_type, typename IteratorTraits<Iterator>::DifferenceType>));
    EXPECT_TRUE((std::is_same_v<pointer, typename IteratorTraits<Iterator>::PointerType>));
    EXPECT_TRUE((std::is_same_v<const_pointer, typename IteratorTraits<Iterator>::ConstPointerType>));
    EXPECT_TRUE((std::is_same_v<reference, typename IteratorTraits<Iterator>::ReferenceType>));
    EXPECT_TRUE((std::is_same_v<const_reference, typename IteratorTraits<Iterator>::ConstReferenceType>));

    EXPECT_TRUE((has_operator_deref_v<const Iterator, const_reference>));
    EXPECT_TRUE((has_operator_prefix_plus_plus_v<Iterator, Iterator&>));
    EXPECT_TRUE((has_operator_postfix_plus_plus_v<Iterator, Iterator>));
    EXPECT_TRUE((has_operator_eq_v<Iterator, const Iterator&>));
    EXPECT_TRUE((has_operator_not_eq_v<Iterator, const Iterator&>));

    EXPECT_TRUE(isInputIterator<InputIter>);
}

struct OutputIter {
    reference operator*();
    OutputIter& operator++();
    OutputIter operator++(int);

    friend bool operator==(const OutputIter& lhs, const OutputIter& rhs);
    friend bool operator!=(const OutputIter& lhs, const OutputIter& rhs);
};

TEST(TestIteratorTraits, TestOutputIter) {
    using Iterator = OutputIter;
    EXPECT_TRUE((std::is_same_v<value_type, typename IteratorTraits<Iterator>::ValueType>));
    EXPECT_TRUE((std::is_same_v<difference_type, typename IteratorTraits<Iterator>::DifferenceType>));
    EXPECT_TRUE((std::is_same_v<pointer, typename IteratorTraits<Iterator>::PointerType>));
    EXPECT_TRUE((std::is_same_v<const_pointer, typename IteratorTraits<Iterator>::ConstPointerType>));
    EXPECT_TRUE((std::is_same_v<reference, typename IteratorTraits<Iterator>::ReferenceType>));
    EXPECT_TRUE((std::is_same_v<const_reference, typename IteratorTraits<Iterator>::ConstReferenceType>));

    EXPECT_TRUE((has_operator_deref_v<Iterator, reference>));
    EXPECT_TRUE((has_operator_prefix_plus_plus_v<Iterator, Iterator&>));
    EXPECT_TRUE((has_operator_postfix_plus_plus_v<Iterator, Iterator>));
    EXPECT_TRUE((has_operator_eq_v<Iterator, const Iterator&>));
    EXPECT_TRUE((has_operator_not_eq_v<Iterator, const Iterator&>));

    EXPECT_TRUE(isOutputIterator<Iterator>);
}

struct ForwardIter {
    const_reference operator*() const;
    reference operator*();
    ForwardIter& operator++();
    ForwardIter operator++(int);

    friend bool operator==(const ForwardIter& lhs, const ForwardIter& rhs);
    friend bool operator!=(const ForwardIter& lhs, const ForwardIter& rhs);

    ForwardIter() = default; // Default constructor is need for multi-pass
};

TEST(TestIteratorTraits, TestForwardIter) {
    using Iterator = ForwardIter;
    EXPECT_TRUE((std::is_same_v<value_type, typename IteratorTraits<Iterator>::ValueType>));
    EXPECT_TRUE((std::is_same_v<difference_type, typename IteratorTraits<Iterator>::DifferenceType>));
    EXPECT_TRUE((std::is_same_v<pointer, typename IteratorTraits<Iterator>::PointerType>));
    EXPECT_TRUE((std::is_same_v<const_pointer, typename IteratorTraits<Iterator>::ConstPointerType>));
    EXPECT_TRUE((std::is_same_v<reference, typename IteratorTraits<Iterator>::ReferenceType>));
    EXPECT_TRUE((std::is_same_v<const_reference, typename IteratorTraits<Iterator>::ConstReferenceType>));

    EXPECT_TRUE((has_operator_deref_v<Iterator, reference>));
    EXPECT_TRUE((has_operator_deref_v<const Iterator, const_reference>));
    EXPECT_TRUE((has_operator_prefix_plus_plus_v<Iterator, Iterator&>));
    EXPECT_TRUE((has_operator_postfix_plus_plus_v<Iterator, Iterator>));
    EXPECT_TRUE((has_operator_eq_v<Iterator, const Iterator&>));
    EXPECT_TRUE((has_operator_not_eq_v<Iterator, const Iterator&>));

    EXPECT_TRUE(isForwardIterator<Iterator>);
}

struct BidirectionalIter {
    const_reference operator*() const;
    reference operator*();

    BidirectionalIter& operator++();
    BidirectionalIter operator++(int);

    BidirectionalIter& operator--();
    BidirectionalIter operator--(int);

    friend bool operator==(const BidirectionalIter& lhs, const BidirectionalIter& rhs);
    friend bool operator!=(const BidirectionalIter& lhs, const BidirectionalIter& rhs);

    BidirectionalIter() = default;
};

TEST(TestIteratorTraits, TestBidirectionalIter) {
    using Iterator = BidirectionalIter;
    EXPECT_TRUE((std::is_same_v<value_type, typename IteratorTraits<Iterator>::ValueType>));
    EXPECT_TRUE((std::is_same_v<difference_type, typename IteratorTraits<Iterator>::DifferenceType>));
    EXPECT_TRUE((std::is_same_v<pointer, typename IteratorTraits<Iterator>::PointerType>));
    EXPECT_TRUE((std::is_same_v<const_pointer, typename IteratorTraits<Iterator>::ConstPointerType>));
    EXPECT_TRUE((std::is_same_v<reference, typename IteratorTraits<Iterator>::ReferenceType>));
    EXPECT_TRUE((std::is_same_v<const_reference, typename IteratorTraits<Iterator>::ConstReferenceType>));

    EXPECT_TRUE((has_operator_deref_v<Iterator, reference>));
    EXPECT_TRUE((has_operator_deref_v<const Iterator, const_reference>));
    EXPECT_TRUE((has_operator_prefix_plus_plus_v<Iterator, Iterator&>));
    EXPECT_TRUE((has_operator_postfix_plus_plus_v<Iterator, Iterator>));
    EXPECT_TRUE((has_operator_prefix_minus_minus_v<Iterator, Iterator&>));
    EXPECT_TRUE((has_operator_postfix_minus_minus_v<Iterator, Iterator>));
    EXPECT_TRUE((has_operator_eq_v<Iterator, const Iterator&>));
    EXPECT_TRUE((has_operator_not_eq_v<Iterator, const Iterator&>));

    EXPECT_TRUE((isIncrementableIterator<Iterator>));
    EXPECT_TRUE((isOnReadDerefableIterator<InputIter>));

    EXPECT_TRUE(isBidirectionalIterator<Iterator>);
}

struct RandomAccessIter {
    const_reference operator*() const;
    reference operator*();
    const_reference operator[](difference_type n) const;
    reference operator[](difference_type n);

    RandomAccessIter& operator++();
    RandomAccessIter operator++(int);
    RandomAccessIter& operator--();
    RandomAccessIter operator--(int);

    RandomAccessIter& operator+=(difference_type n);
    RandomAccessIter& operator-=(difference_type n);
    RandomAccessIter operator+(difference_type n) const;
    RandomAccessIter operator-(difference_type n) const;
    difference_type operator-(const RandomAccessIter& other) const;

    friend bool operator==(const RandomAccessIter& lhs, const RandomAccessIter& rhs);
    friend bool operator!=(const RandomAccessIter& lhs, const RandomAccessIter& rhs);
    friend bool operator<(const RandomAccessIter& lhs, const RandomAccessIter& rhs);
    friend bool operator<=(const RandomAccessIter& lhs, const RandomAccessIter& rhs);
    friend bool operator>(const RandomAccessIter& lhs, const RandomAccessIter& rhs);
    friend bool operator>=(const RandomAccessIter& lhs, const RandomAccessIter& rhs);

    RandomAccessIter() = default;
};

TEST(TestIteratorTraits, TestRandomAccessIter) {
    using Iterator = RandomAccessIter;
    EXPECT_TRUE((std::is_same_v<value_type, typename IteratorTraits<Iterator>::ValueType>));
    EXPECT_TRUE((std::is_same_v<difference_type, typename IteratorTraits<Iterator>::DifferenceType>));
    EXPECT_TRUE((std::is_same_v<pointer, typename IteratorTraits<Iterator>::PointerType>));
    EXPECT_TRUE((std::is_same_v<const_pointer, typename IteratorTraits<Iterator>::ConstPointerType>));
    EXPECT_TRUE((std::is_same_v<reference, typename IteratorTraits<Iterator>::ReferenceType>));
    EXPECT_TRUE((std::is_same_v<const_reference, typename IteratorTraits<Iterator>::ConstReferenceType>));

    EXPECT_TRUE((has_operator_deref_v<Iterator, reference>));
    EXPECT_TRUE((has_operator_deref_v<const Iterator, const_reference>));

    EXPECT_TRUE((has_operator_square_brackets_v<const Iterator, const_reference>));
    EXPECT_TRUE((has_operator_square_brackets_v<Iterator, reference>));

    EXPECT_TRUE((has_operator_prefix_plus_plus_v<Iterator, Iterator&>));
    ASSERT_TRUE((has_operator_postfix_plus_plus_v<Iterator, Iterator>));
    EXPECT_TRUE((has_operator_prefix_minus_minus_v<Iterator, Iterator&>));
    EXPECT_TRUE((has_operator_postfix_minus_minus_v<Iterator, Iterator>));
    EXPECT_TRUE((has_operator_plus_v<const Iterator, Iterator, difference_type>));
    EXPECT_TRUE((has_operator_minus_v<const Iterator, Iterator, difference_type>));
    EXPECT_TRUE((has_operator_minus_v<const Iterator, difference_type, const Iterator&>));

    EXPECT_TRUE((has_operator_eq_v<Iterator, const Iterator&>));
    EXPECT_TRUE((has_operator_not_eq_v<Iterator, const Iterator&>));
    EXPECT_TRUE((has_operator_less_v<Iterator, const Iterator&>));
    EXPECT_TRUE((has_operator_less_eq_v<Iterator, const Iterator&>));
    EXPECT_TRUE((has_operator_greater_v<Iterator, const Iterator&>));
    EXPECT_TRUE((has_operator_greater_eq_v<Iterator, const Iterator&>));

    EXPECT_TRUE(isRandomAccessIterator<Iterator>);
}

struct ContiguousIter : ContiguousIteratorTag {
    const_reference operator*() const;
    reference operator*();
    const_reference operator[](difference_type n) const;
    reference operator[](difference_type n);

    ContiguousIter& operator++();
    ContiguousIter operator++(int);
    ContiguousIter& operator--();
    ContiguousIter operator--(int);

    ContiguousIter& operator+=(difference_type n);
    ContiguousIter& operator-=(difference_type n);
    ContiguousIter operator+(difference_type n) const;
    ContiguousIter operator-(difference_type n) const;
    difference_type operator-(const ContiguousIter& other) const;

    friend bool operator==(const ContiguousIter& lhs, const ContiguousIter& rhs);
    friend bool operator!=(const ContiguousIter& lhs, const ContiguousIter& rhs);
    friend bool operator<(const ContiguousIter& lhs, const ContiguousIter& rhs);
    friend bool operator<=(const ContiguousIter& lhs, const ContiguousIter& rhs);
    friend bool operator>(const ContiguousIter& lhs, const ContiguousIter& rhs);
    friend bool operator>=(const ContiguousIter& lhs, const ContiguousIter& rhs);

    ContiguousIter() = default;
};

TEST(TestIteratorTraits, TestContiguousIter) {
    using Iterator = ContiguousIter;
    EXPECT_TRUE((std::is_same_v<value_type, typename IteratorTraits<Iterator>::ValueType>));
    EXPECT_TRUE((std::is_same_v<difference_type, typename IteratorTraits<Iterator>::DifferenceType>));
    EXPECT_TRUE((std::is_same_v<pointer, typename IteratorTraits<Iterator>::PointerType>));
    EXPECT_TRUE((std::is_same_v<const_pointer, typename IteratorTraits<Iterator>::ConstPointerType>));
    EXPECT_TRUE((std::is_same_v<reference, typename IteratorTraits<Iterator>::ReferenceType>));
    EXPECT_TRUE((std::is_same_v<const_reference, typename IteratorTraits<Iterator>::ConstReferenceType>));

    EXPECT_TRUE(isContiguousIterator<Iterator>);
}

TEST(TestIteratorTraits, TestFixedArrayIters) {
    using It = typename Array<int, 64>::Iterator;
    EXPECT_TRUE(isContiguousIterator<It>);
}

TEST(TestIteratorTraits, TestValueType) {
    struct It1 {
        using DifferenceType = int;
        const int& operator*() const;
        It1& operator++();
        It1 operator++(int);
    };
    EXPECT_TRUE((std::is_same_v<IteratorTraits<It1>::DifferenceType, int>));

    struct It2 {
        const int& operator*() const;
        It2& operator++();
        It2 operator++(int);
    };
    EXPECT_TRUE((std::is_same_v<IteratorTraits<It2>::DifferenceType, std::ptrdiff_t>));

    struct It3 {
        const int& operator*() const;
        int operator-(It3) const;
        It3& operator++();
        It3 operator++(int);
    };
    EXPECT_TRUE((std::is_same_v<IteratorTraits<It3>::DifferenceType, int>));
}

TEST(TestIteratorTraits, TestDifferenceType) {
    struct It1 {
        using DifferenceType = int;
        const int& operator*() const;
        It1& operator++();
        It1 operator++(int);
    };
    EXPECT_TRUE((std::is_same_v<IteratorTraits<It1>::DifferenceType, int>));

    struct It2 {
        const int& operator*() const;
        It2& operator++();
        It2 operator++(int);
    };
    EXPECT_TRUE((std::is_same_v<IteratorTraits<It2>::DifferenceType, std::ptrdiff_t>));

    struct It3 {
        const int& operator*() const;
        int operator-(It3) const;
        It3& operator++();
        It3 operator++(int);
    };
    EXPECT_TRUE((std::is_same_v<IteratorTraits<It3>::DifferenceType, int>));
}
