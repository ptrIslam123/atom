#include <gtest/gtest.h>

#include "include/sfinae/has_type_operators.h"

TEST(TestHashClassOperators, TestOperatorDeref) {
    struct Obj1 {
        Obj1& operator*();
    };
    struct Obj2 {
        int operator*();
    };
    struct Obj3 {};
    struct Obj4 {
        const Obj4& operator*() const;
        Obj4& operator*();
    };

    EXPECT_TRUE((has_operator_deref_v<Obj1, Obj1&>));
    EXPECT_TRUE((has_operator_deref_v<Obj2, int>));
    EXPECT_FALSE((has_operator_deref_v<Obj3, int>));
    EXPECT_TRUE((has_operator_deref_v<Obj4, Obj4&>));
    EXPECT_TRUE((has_operator_deref_v<const Obj4, const Obj4&>));
    EXPECT_TRUE((has_operator_deref_v<int*, int&>));
    EXPECT_TRUE((has_operator_deref_v<const int*, const int&>));
}

TEST(TestHashClassOperators, TestOperatorNotEq) {
    struct Obj1  {
        bool operator!=(const Obj1) const;
    };

    struct Obj2 {
        bool operator!=(int) const;
    };
    struct Obj3 {};

    EXPECT_TRUE((has_operator_not_eq_v<Obj1, const Obj1&>));
    EXPECT_TRUE((has_operator_not_eq_v<Obj2, int>));
    EXPECT_FALSE((has_operator_not_eq_v<Obj3, const Obj3&>));
}

TEST(TestHashClassOperators, TestOperatorEq) {
    struct Obj1  {
        bool operator==(const Obj1) const;
    };

    struct Obj2 {
        bool operator==(int) const;
    };
    struct Obj3 {};

    EXPECT_TRUE((has_operator_eq_v<Obj1, const Obj1&>));
    EXPECT_TRUE((has_operator_eq_v<Obj2, int>));
    EXPECT_FALSE((has_operator_eq_v<Obj3, const Obj3&>));
}

TEST(TestHashClassOperators, TestOperatorGreaterEq) {
    struct Obj1 {
        bool operator>=(const Obj1& other) const;
    };
    struct Obj2 {
        bool operator>=(int otherValue) const;
    };
    struct Obj3 {};
    EXPECT_TRUE((has_operator_greater_eq_v<Obj1, const Obj1&>));
    EXPECT_TRUE((has_operator_greater_eq_v<Obj2, int>));
    EXPECT_FALSE((has_operator_greater_eq_v<Obj3, const Obj3&>));
}

TEST(TestHashClassOperators, TestOperatorGreater) {
    struct Obj1 {
        bool operator>(const Obj1& other) const;
    };
    struct Obj2 {
        bool operator>(int otherValue) const;
    };
    struct Obj3 {};
    EXPECT_TRUE((has_operator_greater_v<Obj1, const Obj1&>));
    EXPECT_TRUE((has_operator_greater_v<Obj2, int>));
    EXPECT_FALSE((has_operator_greater_v<Obj3, const Obj3&>));
}

TEST(TestHashClassOperators, TestOperatorLessEq) {
    struct Obj1 {
        bool operator<=(const Obj1& other) const;
    };
    struct Obj2 {
        bool operator<=(int otherValue) const;
    };
    struct Obj3 {};
    EXPECT_TRUE((has_operator_less_eq_v<Obj1, const Obj1&>));
    EXPECT_TRUE((has_operator_less_eq_v<Obj2, int>));
    EXPECT_FALSE((has_operator_less_eq_v<Obj3, const Obj3&>));
}

TEST(TestHashClassOperators, TestOperatorLess) {
    struct Obj1 {
        bool operator<(const Obj1& other) const;
    };
    struct Obj2 {
        bool operator<(int otherValue) const;
    };
    struct Obj3 {};
    EXPECT_TRUE((has_operator_less_v<Obj1, const Obj1&>));
    EXPECT_TRUE((has_operator_less_v<Obj2, int>));
    EXPECT_FALSE((has_operator_less_v<Obj3, const Obj3&>));
}

TEST(TestHashClassOperators, TestOperatorNonModifyPlus) {
    struct Counter1 {
        Counter1 operator+(const Counter1& other) const;
    };
    struct Counter2 {};
    struct Counter3 {
        Counter3 operator+(int n) const;
    };

    EXPECT_TRUE((has_operator_nonmodify_plus_v<const Counter1, Counter1, const Counter1&>));
    EXPECT_FALSE((has_operator_nonmodify_plus_v<Counter2, Counter2, const Counter2&>));
    EXPECT_TRUE((has_operator_nonmodify_plus_v<const Counter3, Counter3, int>));
}

TEST(TestHashClassOperators, TestOperatorNonModifyMinus) {
    struct Counter1 {
        Counter1 operator-(const Counter1& other) const;
    };
    struct Counter2 {};
    struct Counter3 {
        Counter3 operator-(int n) const;
    };

    EXPECT_TRUE((has_operator_nonmodify_minus_v<const Counter1, Counter1, const Counter1&>));
    EXPECT_FALSE((has_operator_nonmodify_minus_v<Counter2, Counter2, const Counter2&>));
    EXPECT_TRUE((has_operator_nonmodify_minus_v<const Counter3, Counter3, int>));
}

TEST(TestHashClassOperators, TestOperatorDecr) {
    struct Counter1 {
        Counter1& operator--();
        Counter1 operator--(int);
    };
    struct Counter2 {};

    EXPECT_TRUE((has_operator_prefix_minus_minus_v<Counter1, Counter1&>));
    EXPECT_TRUE((has_operator_postfix_minus_minus_v<Counter1, Counter1>));

    EXPECT_FALSE((has_operator_prefix_minus_minus_v<Counter2, Counter2&>));
    EXPECT_FALSE((has_operator_postfix_minus_minus_v<Counter2, Counter2&>));
}

TEST(TestHashClassOperators, TestOperatorIncr) {
    struct Counter1 {
        Counter1& operator++();
        Counter1 operator++(int);
    };
    struct Counter2 {};

    EXPECT_TRUE((has_operator_prefix_plus_plus_v<Counter1, Counter1&>));
    EXPECT_TRUE((has_operator_postfix_plus_plus_v<Counter1, Counter1>));

    EXPECT_FALSE((has_operator_prefix_plus_plus_v<Counter2, Counter2&>));
    EXPECT_FALSE((has_operator_postfix_plus_plus_v<Counter2, Counter2&>));
}

TEST(TestHashClassOperators, TestOperatorPlus) {
    struct Obj1 {
        Obj1& operator+=(const Obj1& other);
    };
    struct Obj2 {
        Obj2& operator+=(Obj2 other);
    };
    struct Obj3 {};
    struct Obj4 {
        Obj4& operator+=(int n);
    };

    EXPECT_TRUE((has_operator_plus_v<Obj1, Obj1&, const Obj1&>));
    EXPECT_TRUE((has_operator_plus_v<Obj2, Obj2&, Obj2>));
    EXPECT_FALSE((has_operator_plus_v<Obj3, Obj3, const Obj3&>));
    EXPECT_TRUE((has_operator_plus_v<Obj4, Obj4&, int>));
}

TEST(TestHashClassOperators, TestOperatorMinus) {
    struct Obj1 {
        Obj1 operator-(const Obj1& other) const;
    };
    struct Obj2 {
        Obj2 operator-(Obj2 other) const;
    };
    struct Obj3 {};
    struct Obj4 {
        Obj4 operator-(int n) const;
    };

    EXPECT_TRUE((has_operator_nonmodify_minus_v<Obj1, Obj1, const Obj1&>));
    EXPECT_TRUE((has_operator_nonmodify_minus_v<Obj2, Obj2, Obj2>));
    EXPECT_FALSE((has_operator_nonmodify_minus_v<Obj3, Obj3, const Obj3&>));
    EXPECT_TRUE((has_operator_nonmodify_minus_v<Obj4, Obj4, int>));
}
