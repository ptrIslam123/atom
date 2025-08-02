#include <gtest/gtest.h>

#include "include/containers/static/array.h"

template<class T, std::size_t N>
using ArrayType = atom::containers::StaticArray<T, N>;

TEST(TestStaticArray, TestInsert) {
    ArrayType<int, 1024> array;
    EXPECT_EQ(array.size(), 0);
    EXPECT_TRUE(array.isEmpty());
    EXPECT_EQ(array.capacity(), 1024);

    // Check insert into back
    {
        constexpr std::array<int, 10> data = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
        for (auto i = 0; i < data.size(); ++i) {
            const auto d = data[i];

            EXPECT_EQ(array.size(), i);
            array.pushBack(d);
            EXPECT_EQ(array.size(), i + 1);

            auto it = array.lastConstIter();
            EXPECT_EQ(*it, d);
        }
        for (auto i = 0; i < data.size(); ++i) {
            EXPECT_EQ(array[i], data[i]);
        }
        array.clear();
        EXPECT_TRUE(array.isEmpty());
    }
    // Check insert into begin
    {
        array.clear();
        EXPECT_TRUE(array.isEmpty());
        constexpr std::array<int, 10> data = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
        for (auto i = 0; i < data.size(); ++i) {
            const auto d = data[i];
            auto it = array.insert(array.firstConstIter(), d);
            EXPECT_EQ(*it, d);
        }
        for (auto i = 0; i < data.size(); ++i) {
            EXPECT_EQ(array[i], data[i]);
        }
        array.clear();
        EXPECT_TRUE(array.isEmpty());
    }
    // Check insert into midle
    {
        array.clear();
        EXPECT_TRUE(array.isEmpty());
        constexpr std::array<int, 2> data = {3, 4};
        array.pushBack(int{1});
        array.pushBack(int{2});
        array.pushBack(int{5});
        array.pushBack(int{6});
        array.insert(array.firstConstIter() + 2, data);
        ASSERT_EQ(array.size(), 6);
        for (auto i = 0; i < array.size(); ++i) {
            EXPECT_EQ(array.atUnsafe(i), (i + 1));
        }
    }
    // Check emplace
//    {
//        struct Foo {
//            explicit Foo(int _id, std::string _data): id(_id), data(_data) {}
//            int id;
//            std::string data;
//        };

//        ArrayType<Foo, 1024> array;
//        array.emplaceBack(int{10}, std::string{"test data!"});
//        ASSERT_FALSE(array.isEmpty());
//        const Foo& foo = array.firstElement();
//        EXPECT_EQ(foo.id, int{10});
//        EXPECT_EQ(foo.data, std::string{"test data!"});
//    }
}

TEST(TestStaticArray, TestErase) {
    constexpr std::array<int, 5> initialData = {10, 20, 30, 40, 50};
    ArrayType<int, 1024> array;
    EXPECT_EQ(array.capacity(), 1024);
    array.pushBack(initialData);
    ASSERT_EQ(array.size(), 5);
    for (auto i = 0; i < initialData.size(); ++i) {
        EXPECT_EQ(array.atUnsafe(i), (i + 1) * 10);
    }

    // Check erase from the end
    {
        constexpr std::array<int, 5> data = {10, 20, 30, 40};
        array.erase(array.lastConstIter());
        EXPECT_EQ(array.size(), 4);
        for (auto i = 0 ; i < array.size(); ++i) {
            EXPECT_EQ(array.atUnsafe(i), data[i]);
        }
    }
    // Check erase from the begin
    {
        constexpr std::array<int, 5> data = {20, 30, 40};
        array.erase(array.firstConstIter());
        EXPECT_EQ(array.size(), 3);
        for (auto i = 0 ; i < array.size(); ++i) {
            EXPECT_EQ(array.atUnsafe(i), data[i]);
        }
    }
    // Check erase from the midle
    {
        constexpr std::array<int, 5> data = {20, 40};
        auto it = array.firstConstIter() + 1;
        array.erase(it);
        EXPECT_EQ(array.size(), 2);
        for (auto i = 0 ; i < array.size(); ++i) {
            EXPECT_EQ(array.atUnsafe(i), data[i]);
        }
    }
    // Check erase range
    {
        array.clear();
        EXPECT_TRUE(array.isEmpty());
        array.pushBack(100);
        array.pushBack(200);
        array.pushBack(300);
        array.pushBack(400);
        array.pushBack(500);
        ASSERT_EQ(array.size(), 5);

        auto first = array.firstConstIter() + 1; // from 200
        auto last = array.firstConstIter() + 4; // to 400
        ASSERT_LE(last, array.lastConstIter());

        array.erase(first, last);

        EXPECT_EQ(array.size(), 2);
        EXPECT_EQ(array.atUnsafe(0), 100);
        EXPECT_EQ(array.atUnsafe(1), 500);
    }
    {
        array.clear();
        ASSERT_TRUE(array.isEmpty());
        array.pushBack(10);
        array.pushBack(20);
        array.pushBack(30);

        auto it = array.erase(array.firstConstIter() + 1);
        //ASSERT_TRUE(it.isValid());
        EXPECT_EQ(*it, 30);

        it = array.erase(array.lastConstIter());
        //EXPECT_FALSE(it.isValid());

        it = array.erase(array.lastConstIter());
        //EXPECT_FALSE(it.isValid());
    }

//    // Check erase with invalid iterator
//    {
//        array.clear();
//        array.pushBack(std::array<int, 3>{10, 20, 30});
//        EXPECT_ANY_THROW(array.erase(array.endConstIter()));
//        EXPECT_ANY_THROW(array.erase(array.endConstIter()) + 1);
//        auto it = array.firstConstIter();
//        array.erase(it);
//        EXPECT_ANY_THROW(array.erase(it));
//        EXPECT_ANY_THROW(array.erase(array.endConstIter() + 100));
//        array.clear();
//    }

    // Check clear
    {
        array.pushBack(std::array<int, 5>{10, 20, 30, 40, 50});
        array.clear();
        EXPECT_TRUE(array.isEmpty());
        EXPECT_EQ(array.size(), 0);
    }
}
