#include <gtest/gtest.h>

#include "include/iterator/iterator_traits.h"

#include <type_traits>
#include <initializer_list>

using namespace atom::iter;

TEST(TestIteratorTraits, TestInitializedListIt) {
    using Iterator = typename std::initializer_list<int>::iterator;
    EXPECT_TRUE(isInputIterator<Iterator>);
    EXPECT_TRUE(isRandomAccessIterator<Iterator>);
}

using value_type = int; // just for example
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
    std::vector<int>::iterator it;
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

    EXPECT_TRUE(isInputIterator<Iterator>);
    EXPECT_FALSE((isOutputIterator<Iterator>));
    EXPECT_FALSE((isForwardIterator<Iterator>));
    EXPECT_FALSE((isBidirectionalIterator<Iterator>));
    EXPECT_FALSE((isRandomAccessIterator<Iterator>));
}

TEST(TestIteratorTraits, TestInvalidInputIter) {
    struct InvalidInputIter {
        using value_type = char; // just for example
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using reference = value_type&;
        using const_reference = const value_type&;
        const_reference operator*() const;
        InvalidInputIter& operator++();
        InvalidInputIter operator++(int);
        // friend bool operator==(const InvalidInputIter& lhs, const InvalidInputIter& rhs);
        // friend bool operator!=(const InvalidInputIter& lhs, const InvalidInputIter& rhs);
    };
    EXPECT_FALSE(isInputIterator<InvalidInputIter>);
}

struct OutputIter {
    reference operator*();
    OutputIter& operator++();
    OutputIter operator++(int);
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

    EXPECT_FALSE(isInputIterator<Iterator>);
    EXPECT_TRUE((isOutputIterator<Iterator>));
    EXPECT_FALSE((isForwardIterator<Iterator>));
    EXPECT_FALSE((isBidirectionalIterator<Iterator>));
    EXPECT_FALSE((isRandomAccessIterator<Iterator>));
}

TEST(TestIteratorTraits, TestInvalidOutputIter) {
    struct InvalidOutputIter {
        using value_type = char; // just for example
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using reference = value_type&;
        using const_reference = const value_type&;
        reference operator*();
        // InvalidOutputIter& operator++();
        // InvalidOutputIter operator++(int);
    };
    EXPECT_FALSE(isInputIterator<InvalidOutputIter>);
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

    EXPECT_TRUE(isInputIterator<Iterator>);
    EXPECT_TRUE((isOutputIterator<Iterator>));
    EXPECT_TRUE((isForwardIterator<Iterator>));
    EXPECT_FALSE((isBidirectionalIterator<Iterator>));
    EXPECT_FALSE((isRandomAccessIterator<Iterator>));
}

TEST(TestIteratorTraits, TestInvalidForwardIter) {
    struct InvalidForwardIter1 {
        using value_type = char; // just for example
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using reference = value_type&;
        using const_reference = const value_type&;
        // const_reference operator*() const;
        // reference operator*();
        InvalidForwardIter1& operator++();
        InvalidForwardIter1 operator++(int);
        bool operator==(const InvalidForwardIter1&) const;
        bool operator!=(const InvalidForwardIter1&) const;
        InvalidForwardIter1() = default;
    };
    EXPECT_FALSE(isForwardIterator<InvalidForwardIter1>);

    struct InvalidForwardIter2 {
        using value_type = char; // just for example
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using reference = value_type&;
        using const_reference = const value_type&;
        const_reference operator*() const;
        reference operator*();
        // InvalidForwardIter2& operator++();
        // InvalidForwardIter2 operator++(int);
        bool operator==(const InvalidForwardIter2&) const;
        bool operator!=(const InvalidForwardIter2&) const;
        InvalidForwardIter2() = default;
    };
    EXPECT_FALSE(isForwardIterator<InvalidForwardIter2>);

    struct InvalidForwardIter3 {
        using value_type = char; // just for example
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using reference = value_type&;
        using const_reference = const value_type&;
        const_reference operator*() const;
        reference operator*();
        InvalidForwardIter3& operator++();
        InvalidForwardIter3 operator++(int);
        // bool operator==(const InvalidForwardIter3&) const;
        // bool operator!=(const InvalidForwardIter3&) const;
        InvalidForwardIter3() = default;
    };
    EXPECT_FALSE(isForwardIterator<InvalidForwardIter3>);
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

    EXPECT_TRUE(isInputIterator<Iterator>);
    EXPECT_TRUE((isOutputIterator<Iterator>));
    EXPECT_TRUE((isForwardIterator<Iterator>));
    EXPECT_TRUE((isBidirectionalIterator<Iterator>));
    EXPECT_FALSE((isRandomAccessIterator<Iterator>));
}

TEST(TestIteratorTraits, TestInvalidBidirectionalIter) {
    struct InvalidBidirectionalIter1 {
        const_reference operator*() const;
        reference operator*();
        InvalidBidirectionalIter1& operator++();
        InvalidBidirectionalIter1 operator++(int);
        // InvalidBidirectionalIter1& operator--();
        // InvalidBidirectionalIter1 operator--(int);
        bool operator==(const InvalidBidirectionalIter1&) const;
        bool operator!=(const InvalidBidirectionalIter1&) const;
        InvalidBidirectionalIter1() = default;
    };
    struct InvalidBidirectionalIter2 {
        const_reference operator*() const;
        reference operator*();
        // InvalidBidirectionalIter2& operator++();
        // InvalidBidirectionalIter2 operator++(int);
        InvalidBidirectionalIter2& operator--();
        InvalidBidirectionalIter2 operator--(int);
        bool operator==(const InvalidBidirectionalIter2&) const;
        bool operator!=(const InvalidBidirectionalIter2&) const;
        InvalidBidirectionalIter2() = default;
    };
    EXPECT_FALSE(isBidirectionalIterator<InvalidBidirectionalIter2>);
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

    EXPECT_TRUE((has_operator_prefix_plus_plus_v<Iterator, Iterator&>));
    ASSERT_TRUE((has_operator_postfix_plus_plus_v<Iterator, Iterator>));
    EXPECT_TRUE((has_operator_prefix_minus_minus_v<Iterator, Iterator&>));
    EXPECT_TRUE((has_operator_postfix_minus_minus_v<Iterator, Iterator>));

    EXPECT_TRUE((has_operator_plus_v<Iterator, Iterator&, difference_type>));
    EXPECT_TRUE((has_operator_minus_v<Iterator, Iterator&, difference_type>));
    EXPECT_TRUE((has_operator_nonmodify_plus_v<const Iterator, Iterator, difference_type>));
    EXPECT_TRUE((has_operator_nonmodify_minus_v<const Iterator, Iterator, difference_type>));
    EXPECT_TRUE((has_operator_nonmodify_minus_v<const Iterator, difference_type, const Iterator&>));

    EXPECT_TRUE((has_operator_eq_v<Iterator, const Iterator&>));
    EXPECT_TRUE((has_operator_not_eq_v<Iterator, const Iterator&>));
    EXPECT_TRUE((has_operator_less_v<Iterator, const Iterator&>));
    EXPECT_TRUE((has_operator_less_eq_v<Iterator, const Iterator&>));
    EXPECT_TRUE((has_operator_greater_v<Iterator, const Iterator&>));
    EXPECT_TRUE((has_operator_greater_eq_v<Iterator, const Iterator&>));

    EXPECT_TRUE(isInputIterator<Iterator>);
    EXPECT_TRUE((isOutputIterator<Iterator>));
    EXPECT_TRUE((isForwardIterator<Iterator>));
    EXPECT_TRUE((isBidirectionalIterator<Iterator>));
    EXPECT_TRUE((isRandomAccessIterator<Iterator>));
}

TEST(TestIteratorTraits, TestInvalidRandomAccessIter) {
    struct InvalidRandomAccessIter {
        const_reference operator*() const;
        reference operator*();

        InvalidRandomAccessIter& operator++();
        InvalidRandomAccessIter operator++(int);
        InvalidRandomAccessIter& operator--();
        InvalidRandomAccessIter operator--(int);

        InvalidRandomAccessIter& operator+=(difference_type n);
        InvalidRandomAccessIter& operator-=(difference_type n);
        InvalidRandomAccessIter operator+(difference_type n) const;
        InvalidRandomAccessIter operator-(difference_type n) const;
        difference_type operator-(const RandomAccessIter& other) const;

        bool operator==(const InvalidRandomAccessIter&);
        bool operator!=(const InvalidRandomAccessIter&);
        bool operator<(const InvalidRandomAccessIter&);
        bool operator<=(const InvalidRandomAccessIter&);
        bool operator>(const InvalidRandomAccessIter&);
        bool operator>=(const InvalidRandomAccessIter&);

        InvalidRandomAccessIter() = default;
    };
    EXPECT_FALSE(isRandomAccessIterator<InvalidRandomAccessIter>);
}

TEST(TestIteratorTraits, TestNonIterators) {
    struct NotIterator {};
    EXPECT_TRUE(isUnknownIterator<NotIterator>);
}

TEST(TestIteratorTraits, TestWithProxyIterator) {
    struct BoolVector {
        // like a std::vector<bool>::reference
        class PseudoReference {
        public:
            PseudoReference();
            PseudoReference(bool value);
            PseudoReference(const PseudoReference& other);
            PseudoReference(PseudoReference&& other);
            PseudoReference& operator=(const PseudoReference& other);
            PseudoReference& operator=(PseudoReference&& other);
            PseudoReference& operator=(bool value);
            const PseudoReference& operator=(bool value) const;
            operator bool() const;
            bool* operator&();
            const bool* operator&() const;

        private:
            friend BoolVector;
            PseudoReference(std::size_t index, void* buffer);
            // ...
        };

        struct Iterator {
            // operator*() return PseudoReference instead of read ValueType&
            PseudoReference operator*();
        };

        PseudoReference operator[](std::size_t index);
        Iterator begin();
    };

    //! WHY: in my opinion, there is no point in distinguishing between the return type of
    //! iterators when dereferencing, since, if necessary, any pseudo reference (like a std::vector<bool>::reference)
    //! can simulate the semantics of a regular reference (native), so I don’t see any point
    //! in special logic for this case yet.
    EXPECT_FALSE((std::is_same_v<bool, typename IteratorTraits<BoolVector::Iterator>::ValueType>));
}

TEST(TestIteratorTraits, TestNativePointersAsIterators) {
    EXPECT_TRUE((std::is_same_v<int, typename IteratorTraits<int*>::ValueType>));
    EXPECT_TRUE((std::is_same_v<std::ptrdiff_t, typename IteratorTraits<int*>::DifferenceType>));
    EXPECT_TRUE((std::is_same_v<int*, typename IteratorTraits<int*>::PointerType>));
    EXPECT_TRUE((std::is_same_v<const int*, typename IteratorTraits<int*>::ConstPointerType>));
    EXPECT_TRUE((std::is_same_v<int&, typename IteratorTraits<int*>::ReferenceType>));
    EXPECT_TRUE((std::is_same_v<const int&, typename IteratorTraits<int*>::ConstReferenceType>));

    EXPECT_TRUE(isIncrementableIterator<int*>);
    EXPECT_TRUE(isDecrementableIterator<int*>);
    EXPECT_TRUE(isRandomIncrementableIterator<int*>);
    EXPECT_TRUE(isRandomDecrementableIterator<int*>);
    EXPECT_TRUE(isArithmeticableIterator<int*>);
    EXPECT_TRUE(isIncrementableIterator<const int*>);
    EXPECT_TRUE(isDecrementableIterator<const int*>);
    EXPECT_TRUE(isRandomIncrementableIterator<const int*>);
    EXPECT_TRUE(isRandomDecrementableIterator<const int*>);
    EXPECT_TRUE(isArithmeticableIterator<const int*>);

    EXPECT_TRUE(isOnWriteDerefableIterator<int*>);
    EXPECT_TRUE(isOnWriteDerefableIterator<int *const>);
    EXPECT_FALSE(isOnWriteDerefableIterator<const int*>);
    EXPECT_TRUE(isOnReadDerefableIterator<const int*>);
    EXPECT_TRUE(isOnReadDerefableIterator<int *const>);
    EXPECT_TRUE(isOnReadDerefableIterator<int const * const>);
    EXPECT_TRUE(isOnReadDerefableIterator<int*>);
    EXPECT_TRUE(isDerefableIterator<int*>); // int* can be readable or writable

    EXPECT_TRUE(isEqableIterator<int*>);
    EXPECT_TRUE(isEqableIterator<const int*>);
    EXPECT_TRUE(isEqableIterator<int *const>);
    EXPECT_TRUE(isEqableIterator<int const * const>);
    EXPECT_TRUE(isComparableIterator<int*>);
    EXPECT_TRUE(isComparableIterator<const int*>);
    EXPECT_TRUE(isComparableIterator<int *const>);
    EXPECT_TRUE(isComparableIterator<int const*const>);

    EXPECT_TRUE(isRandomAccessIterator<int*>);
}

TEST(TestIteratorTraits, TestValueType) {
    struct It1 {
        using ValueType = int;
        const int& operator*() const;
        It1& operator++();
        It1 operator++(int);
    };
    EXPECT_TRUE((std::is_same_v<IteratorTraits<It1>::ValueType, int>));

    struct It2 {
        const int& operator*() const;
        It2& operator++();
        It2 operator++(int);
    };
    EXPECT_TRUE((std::is_same_v<IteratorTraits<It2>::ValueType, int>));
}

TEST(TestIteratorTraits, TestDifferenceType) {
    struct It1 {
        using DifferenceType = std::ptrdiff_t;
        const int& operator*() const;
        It1& operator++();
        It1 operator++(int);
    };
    EXPECT_TRUE((std::is_same_v<IteratorTraits<It1>::DifferenceType, std::ptrdiff_t>));

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
    EXPECT_TRUE((std::is_same_v<IteratorTraits<It3>::DifferenceType, std::ptrdiff_t>));
}
