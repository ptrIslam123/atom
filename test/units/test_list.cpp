#include <gtest/gtest.h>

#include "include/containers/dynamic/list.h"

using namespace atom::containers::dynamic;

// TEST(TestList, TestInsert) {
//     // Check insert into the end
//     {
//         List<int> list;
//         EXPECT_TRUE(list.isEmpty());

//         list.pushBack(int{1});
//         EXPECT_EQ(list.size(), 1);
//         EXPECT_EQ(list.lastElement(), int{1});

//         list.pushBack(int{2});
//         EXPECT_EQ(list.size(), 2);
//         EXPECT_EQ(list.lastElement(), int{2});

//         list.pushBack(int{3});
//         EXPECT_EQ(list.size(), 3);
//         EXPECT_EQ(list.lastElement(), int{3});
//     }
//     // Check insert into the begin
//     {
//         List<int> list;
//         EXPECT_TRUE(list.isEmpty());

//         list.pushFront(int{3});
//         EXPECT_EQ(list.size(), 1);
//         EXPECT_EQ(list.firstElement(), int{3});

//         list.pushFront(int{2});
//         EXPECT_EQ(list.size(), 2);
//         EXPECT_EQ(list.firstElement(), int{2});

//         list.pushFront(int{1});
//         EXPECT_EQ(list.size(), 3);
//         EXPECT_EQ(list.firstElement(), int{1});
//     }
//     // Checl insert into the middle
//     {
//         List<int> list;
//         list.pushBack(int{1});
//         list.pushBack(int{2});
//         list.pushBack(int{6});
//         list.pushBack(int{7});
//         EXPECT_EQ(list.size(), 4);

//         auto it = list.firstIter();
//         it++;
//         it = list.insert(it, int{3});
//         it = list.insert(it, int{4});
//         it = list.insert(it, int{5});
//         EXPECT_EQ(list.size(), 7);

//         auto i = 1;
//         for (auto it = list.firstConstIter(); it != list.endConstIter(); ++it, ++i) {
//             EXPECT_EQ(*it, i);
//         }
//     }
// }
