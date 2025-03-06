#include <gtest/gtest.h>

// #include "include/memory/type_convertor.h"

#include <limits>

// using namespace atom::memory;

// TEST(TestTypeCasting, TestPrimitives) {
//     {
//         int v1{100};
//         short v2 = TypeConvertor<int, short>::Convert(v1);
//         EXPECT_EQ(v2, 100);
//     }
//     {
//         short v1{10};
//         TypeConvertor<short, int> convertor{};
//         ASSERT_NO_THROW(convertor.Convert(v1));
//         int v2 = convertor.Convert(v1);
//         EXPECT_EQ(v2, 10);
//     }
//     {
//         TypeConvertor<int, short> convertor{};
//         EXPECT_ANY_THROW(convertor.Convert(std::numeric_limits<int>::max()));
//     }
//     {
//         TypeConvertor<int, short> convertor{};
//         ASSERT_NO_THROW(convertor.Convert(int(std::numeric_limits<short>::max())));
//         EXPECT_EQ(convertor.Convert(int(std::numeric_limits<short>::max())), std::numeric_limits<short>::max());
//     }
// }

// TEST(TestTypeCasting, TestUnAndSignedPrimitives) {
//     {
//         TypeConvertor<unsigned int, int> convertor;
//         unsigned int v1 = 100;
//         //ASSERT_NO_THROW((void)convertor.Convert(v1));
//         int v2 = convertor.Convert(v1);
//         EXPECT_EQ(v2, 100);
//     }
// }
