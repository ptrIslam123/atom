#include <gtest/gtest.h>

#include "include/iterator/iterator.h"
#include "include/containers/fixed/array.h"

using namespace atom::containers::fixed;
using namespace atom::iter;

// TEST(TestIterator, TestAdvance) {
//     Array<int, 64> array{int{10}, int{20}, int{30}, int{40}, int{50}};
//     EXPECT_EQ(array.size(), 5);

//     auto it = array.firstConstIter();
//     EXPECT_EQ(*it, array.firstElement());

//     Advance(it, 3);
//     EXPECT_EQ(*it, *(array.firstConstIter() + 3));

//     it = array.lastConstIter();
//     EXPECT_EQ(*it, array.lastElement());

//     Advance(it, -2);
//     EXPECT_EQ(*it, *(array.lastConstIter() - 2));
// }

// TEST(TestIterator, TestDistance) {
//     Array<int, 64> array{int{10}, int{20}, int{30}, int{40}, int{50}};
//     EXPECT_EQ(array.size(), 5);
//     EXPECT_EQ(Distance(array.firstConstIter(), array.lastConstIter()), array.size());
//     EXPECT_EQ(Distance(array.firstConstIter(), array.firstConstIter() + 3), 3);
//     EXPECT_EQ(Distance(array.firstConstIter(), array.firstConstIter() + 4), 4);
// }
