#include <gtest/gtest.h>

#include "include/types/pre-defined_types.h"

#include <limits>

using namespace atom::types;

TEST(TestFloatingNumber, TestComp) {
    // {
    //     const auto nan = std::numeric_limits<F32::ValueType>::quiet_NaN();
    //     EXPECT_FALSE(F32(nan).isEqualOpt(F32(1.0)));
    //     EXPECT_FALSE(F32(1.0).isEqualOpt(F32(nan)));
    //     EXPECT_FALSE(F32(nan).isEqualOpt(F32(nan))); // NaN != NaN
    // }
    // {
    //     EXPECT_TRUE(F32{1.0} == F32{1.0});
    //     EXPECT_TRUE(F32{0.0} == F32{-0.0}); // +0 == -0
    // }
    // {
    //     const auto inf = std::numeric_limits<typename F32::ValueType>::infinity();
    //     EXPECT_TRUE(F32{inf} == F32{inf});
    //     EXPECT_FALSE(F32{inf} == F32{-inf});
    //     EXPECT_FALSE(F32{1.0} == F32{inf});
    // }
    // {
    //     EXPECT_FALSE(F32(1.0) == (F32(-1.0)));
    //     EXPECT_TRUE(F32(0.0) == (F32(-0.0)));
    // }
    // {
    //     F32 v1{100.234};
    //     F32 v2{v1 + F32{F32::EPSILON * 2}};
    //     EXPECT_TRUE(v1 < v2);
    // }
    // {
    //     F32 v1{0.0};
    //     F32 v2{v1};
    //     EXPECT_EQ(v1, v2);

    //     v1 = F32{1.02356};
    //     v2 = F32{1.02356};
    //     EXPECT_EQ(v1, v2);

    //     v2 = F32{1.0236};
    //     EXPECT_TRUE(v2 > v1);
    //     EXPECT_NE(v2, v1);

    //     v1 = F32::MAX();
    //     v2 = v1 - F32{1.0};
    //     //EXPECT_NE(v1, v2);
    // }
    // {
    //     EXPECT_EQ(F32{0.1 + 0.2}, F32{0.3});
    //     EXPECT_NE(double(0.1 + 0.2), double(0.3));
    // }
    // {
    //     EXPECT_FALSE(F32{0.1 + 0.2} > F32{0.3});
    //     EXPECT_TRUE(double(0.1 + 0.2) > double(0.3));
    // }
    // {
    //     F32 v1{32};
    //     F32 v2{v1};
    //     //EXPECT_EQ(v1 * v2, v1 * F32{2});

    //     v1 = F32::MAX() / F32{2.0};
    //     EXPECT_EQ(v1 * F32{2.0}, F32::MAX());
    // }
}

TEST(TestNumbers, TestCast) {
    {
        U8 i{0};
        U16 j{0};
        ASSERT_NO_THROW(j.storeOpt(i.load()));
        EXPECT_EQ(j.load(), 0);
    }
}

TEST(TestTypeWrapper, TestWithCastomTypes) {
    using IndexType1 = Number<unsigned int, 0, 1000>;
    using IndexType2 = Number<unsigned int, 0, 100>;

    auto foo = [](IndexType1 index1, IndexType2 index2) {
        // perform some calculations
    };

    foo(IndexType1{1}, IndexType2{4});
    // foo(IndexType2{4}, IndexType1{1}); // Check error in compile time!
}

TEST(TestNumbers, TestWithLimitationValues) {
    {
        Number<int> v{std::numeric_limits<int>::max()};
        EXPECT_EQ(v.load(), std::numeric_limits<int>::max());

        EXPECT_ANY_THROW(v++);
    }
    {
        Number<int> v{std::numeric_limits<int>::max()};
        v -= Number<int>{1};
        EXPECT_EQ(v.load(), std::numeric_limits<int>::max() - 1);

        EXPECT_ANY_THROW(v += Number<int>{5});
    }
    {
        Number<int> v{std::numeric_limits<int>::max() / 2};
        EXPECT_EQ(v.load(), std::numeric_limits<int>::max() / 2);
        EXPECT_ANY_THROW(v *= Number<int>{5});
    }
    {
        EXPECT_ANY_THROW(Number<unsigned int> v{-1});
    }
    {
        Number<unsigned int> v{0};
        EXPECT_ANY_THROW(v -= Number<unsigned int>{1});
    }
    {
        Number<int> v1{1};
        Number<unsigned int> v2{0};
        EXPECT_NO_THROW(v2.storeOpt(v1.load()));
        EXPECT_TRUE(v1.load() == v2.load());
    }
    {
        Number<int> v1{-1};
        Number<unsigned int> v2{0};
        EXPECT_ANY_THROW(v2.storeOpt(v1.load()));
    }
}

TEST(TestNumbers, TestOpt) {
    {
        Number<int> v{0};
        EXPECT_TRUE(v.load() == 0);
    }
    {
        Number<int> v1{10};
        Number<int> v2{v1};
        EXPECT_TRUE(v2.load() == 10);
    }
    {
        Number<int> v1{10};
        ++v1;
        EXPECT_TRUE(v1.load() == 11);
        v1 -= Number<int>{3};
        EXPECT_TRUE(v1.load() == 8);
    }
    {
        Number<int> v1{10};
        v1 *= Number<int>{2};
        EXPECT_TRUE(v1.load() == 20);
        v1 /= Number<int>{10};
        EXPECT_TRUE(v1.load() == 2);
    }
    {
        Number<int> v1{10};
        Number<int> v2{v1};
        v2 *= Number<int>{2};

        EXPECT_TRUE(v2 > v1);
        EXPECT_NE(v2, v1);

        v2 = Number<int>{10};
        EXPECT_EQ(v1, v2);
    }
    {
        Number<int> v1{20};
        Number<int> v2{5};
        Number<int> v3 = v1 - v2;
        EXPECT_TRUE(v3.load() == 15);
    }
    {
        Number<int> v1{3};
        Number<int> v2{4};
        Number<int> v3 = v1 * v2;
        EXPECT_TRUE(v3.load() == 12);
    }
    {
        Number<int> v1{20};
        Number<int> v2{4};
        Number<int> v3 = v1 / v2;
        EXPECT_TRUE(v3.load() == 5);
    }
    {
        Number<int> v1{20};
        Number<int> v2{3};
        Number<int> v3 = v1 % v2;
        EXPECT_TRUE(v3.load() == 2);
    }
    {
        Number<int> v1{10};
        Number<int> v2 = v1++;
        EXPECT_TRUE(v1.load() == 11);
        EXPECT_TRUE(v2.load() == 10);
    }
    {
        Number<int> v1{10};
        Number<int> v2 = v1.operator++(int{});
        EXPECT_TRUE(v1.load() == 11);
        EXPECT_TRUE(v2.load() == 10);
    }
    {
        Number<int> v1{10};
        Number<int> v2 = v1--;
        EXPECT_TRUE(v1.load() == 9);
        EXPECT_TRUE(v2.load() == 10);
    }
    {
        Number<int> v1{5};
        Number<int> v2{10};
        EXPECT_TRUE(v1 < v2);
        EXPECT_FALSE(v2 < v1);
    }
    {
        Number<int> v1{5};
        Number<int> v2{5};
        EXPECT_TRUE(v1 <= v2);
        EXPECT_TRUE(v2 <= v1);
    }
    {
        Number<int> v1{10};
        Number<int> v2{5};
        EXPECT_TRUE(v1 >= v2);
        EXPECT_FALSE(v2 >= v1);
    }
    {
        Number<int> v1{10};
        Number<int> v2{5};
        v1 += v2;
        EXPECT_TRUE(v1.load() == 15);
    }
    {
        Number<int> v1{10};
        Number<int> v2{5};
        v1 -= v2;
        EXPECT_TRUE(v1.load() == 5);
    }
    {
        Number<int> v1{10};
        Number<int> v2{5};
        v1 *= v2;
        EXPECT_TRUE(v1.load() == 50);
    }
    {
        Number<int> v1{10};
        Number<int> v2{5};
        v1 /= v2;
        EXPECT_TRUE(v1.load() == 2);
    }
    {
        Number<int> v1{10};
        Number<int> v2{3};
        v1 %= v2;
        EXPECT_TRUE(v1.load() == 1);
    }
    {
        Number<int> v1{1};
        v1 <<= Number<int>{2};
        EXPECT_TRUE(v1.load() == 4);
    }
    {
        Number<int> v1{8};
        v1 >>= Number<int>{2};
        EXPECT_TRUE(v1.load() == 2);
    }
    {
        Number<int> v1{0b1100};
        Number<int> v2{0b1010};
        v1 &= v2;
        EXPECT_TRUE(v1.load() == 0b1000);
    }
    {
        Number<int> v1{0b1100};
        Number<int> v2{0b1010};
        v1 |= v2;
        EXPECT_TRUE(v1.load() == 0b1110);
    }
    {
        Number<int> v1{0b1100};
        Number<int> v2{0b1010};
        v1 ^= v2;
        EXPECT_TRUE(v1.load() == 0b0110);
    }
    {
        Number<int> v1{0b1100};
        v1 = ~v1;
        EXPECT_TRUE(v1.load() == static_cast<int>(~0b1100));
    }
}
