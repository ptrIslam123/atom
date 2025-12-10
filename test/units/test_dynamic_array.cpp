#include <gtest/gtest.h>

#include "include/containers/dynamic/array.h"

#include <array>

template<typename T>
using ArrayType = atom::containers::DynamicArray<T>;

TEST(TestDynamicArray, TestCopyArray) {
    ArrayType<int> array{10, 20, 30, 40, 50};
    ArrayType<int> copy{array};
    ASSERT_EQ(array.size(), copy.size());
    ASSERT_NE(&array[0], &copy[0]);
    for (auto i = 0; i < array.size(); ++i) {
        const auto& v1 = array[i];
        const auto& v2 = copy[i];
        EXPECT_EQ(v1, v2);
        EXPECT_NE(&v1, &v2);
    }
}

TEST(TestDynamicArray, TestMoveArray) {
    ArrayType<int> array1{10, 20, 30, 40, 50};
    ArrayType<int> array2{std::move(array1)};
    ASSERT_EQ(array2.size(), 5);
    ASSERT_TRUE(array1.isEmpty());
    for (auto i = 0; i < array2.size(); ++i) {
        EXPECT_EQ(array2[i], (i + 1) * 10);
    }
}

TEST(TestDynamicArray, TestReserve) {
    ArrayType<int> array;
    EXPECT_EQ(array.size(), 0);
    EXPECT_EQ(array.capacity(), 0);

    array.reserve(64);
    EXPECT_EQ(array.size(), 0);
    EXPECT_EQ(array.capacity(), 64);
}

TEST(TestDynamicArray, TestResize) {
    ArrayType<int> array;
    EXPECT_EQ(array.size(), 0);
    EXPECT_EQ(array.capacity(), 0);

    // Check incr array
    {
        array.pushBack(std::array<int, 3>{10, 20, 30});
        EXPECT_EQ(array.size(), 3);

        array.resize(5);
        EXPECT_EQ(array.size(), 5);
        for (auto i = 0; i < 3; ++i) {
            EXPECT_EQ(array.atUnsafe(i), (i + 1) * 10);
        }
        for (auto i = 3; i < array.size(); ++i) {
            EXPECT_EQ(array.atUnsafe(i), int{});
        }
    }
    // Check decr array
    {
        array.resize(3);
        EXPECT_EQ(array.size(), 3);
        for (auto i = 0; i < 3; ++i) {
            EXPECT_EQ(array.atUnsafe(i), (i + 1) * 10);
        }
    }
}

TEST(TestDynamicArray, TestshrinkToFit) {
    ArrayType<int> array;
    EXPECT_EQ(array.size(), 0);
    EXPECT_EQ(array.capacity(), 0);

    array.resize(10);
    EXPECT_EQ(array.size(), 10);
    for (auto i = 0; i < array.size(); ++i) {
        EXPECT_EQ(array.atUnsafe(i), int{});
    }

    array.reserve(20);
    EXPECT_EQ(array.size(), 10);
    EXPECT_TRUE(20 <= array.capacity());

    array.shrinkToFit();
    EXPECT_EQ(array.size(), 10);
    EXPECT_EQ(array.capacity(), 10);
}

TEST(TestDynamicArray, TestErase) {
    constexpr std::array<int, 5> initialData = {100, 200, 300, 400, 500};
    ArrayType<int> array;
    array.reserve(initialData.size());
    EXPECT_EQ(array.capacity(), initialData.size());
    array.pushBack(initialData);
    ASSERT_EQ(array.size(), 5);

    // Check erase from the end
    {
        array.erase(array.lastConstIter());
        EXPECT_EQ(array.size(), 4);
        EXPECT_EQ(array.lastElement(), 400);
    }

    // Check erase from the begin
    {
        array.erase(array.firstConstIter());
        EXPECT_EQ(array.size(), 3);
        EXPECT_EQ(array.firstElement(), 200);
    }

    // Check erase from the midle
    {
        auto it = array.firstConstIter() + 1;
        array.erase(it);
        EXPECT_EQ(array.size(), 2);
        EXPECT_EQ(array.atUnsafe(0), 200);
        EXPECT_EQ(array.atUnsafe(1), 400);
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
        ASSERT_TRUE(it.isValid());
        EXPECT_EQ(*it, 30);

        it = array.erase(array.lastConstIter());
        EXPECT_FALSE(it.isValid());

        it = array.erase(array.lastConstIter());
        EXPECT_FALSE(it.isValid());
    }

    // Check erase with invalid iterator
    {
        array.clear();
        array.pushBack(std::array<int, 3>{10, 20, 30});
        EXPECT_ANY_THROW(array.erase(array.endConstIter()));
        EXPECT_ANY_THROW(array.erase(array.endConstIter()) + 1);
        auto it = array.firstConstIter();
        array.erase(it);
        EXPECT_ANY_THROW(array.erase(it));
        EXPECT_ANY_THROW(array.erase(array.endConstIter() + 100));
        array.clear();
    }

    // Check clear
    {
        array.pushBack(std::array<int, 5>{10, 20, 30, 40, 50});
        array.clear();
        EXPECT_TRUE(array.isEmpty());
        EXPECT_EQ(array.size(), 0);
    }
}

TEST(TestDynamicArray, TestInsert) {
    ArrayType<int> array;
    EXPECT_EQ(array.size(), 0);
    EXPECT_EQ(array.capacity(), 0);

    // Check insert into back
    {
        constexpr std::array<int, 10> data = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
        array.reserve(data.size());
        EXPECT_EQ(array.size(), 0);
        EXPECT_EQ(array.capacity(), data.size());

        for (auto i = 0; i < data.size(); ++i) {
            const auto d = data[i];

            EXPECT_EQ(array.size(), i);
            array.pushBack(d);
            EXPECT_EQ(array.size(), i + 1);

            auto it = array.lastConstIter();
            ASSERT_TRUE(it.isValid());
            ASSERT_FALSE(it.isExpired());
            EXPECT_EQ(*it, d);
        }
        array.clear();
        EXPECT_TRUE(array.isEmpty());
    }
    // Check pushBack with invalid interator
    {
        ArrayType<int> array;
        array.resize(10);
        ASSERT_EQ(array.size(), 10);
        auto it = array.lastConstIter();
        EXPECT_TRUE(it.isValid());
        EXPECT_FALSE(it.isExpired());

        it += 1;
        EXPECT_EQ(it, array.endConstIter());
        EXPECT_NO_THROW(array.insert(it, int{}));

        it += 1;
        EXPECT_GT(it, array.endConstIter());
        EXPECT_ANY_THROW(array.insert(it, int{}));

        it += 100;
        EXPECT_ANY_THROW(array.insert(it, int{}));
    }
    // Check insert into begin
    {
        constexpr std::array<int, 10> data = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
        for (auto i = 0; i < data.size(); ++i) {
            const auto d = data[i];
            auto it = array.insert(array.firstConstIter(), d);
            ASSERT_TRUE(it.isValid());
            ASSERT_FALSE(it.isExpired());
            EXPECT_EQ(*it, d);
        }
        array.clear();
        EXPECT_TRUE(array.isEmpty());
    }
    // Check insert into midle
    {
        constexpr std::array<int, 2> data = {300, 400};
        array.pushBack(int{100});
        array.pushBack(int{200});
        array.pushBack(int{500});
        array.pushBack(int{600});
        array.insert(array.firstConstIter() + 2, data);
        ASSERT_EQ(array.size(), 6);
        for (auto i = 0; i < array.size(); ++i) {
            EXPECT_EQ(array.atUnsafe(i), (i + 1) * 100);
        }
    }
    // Check insert with invalid interrator
    {
        ArrayType<int> array;
        auto it = array.firstConstIter();
        array.pushBack(1);  // 'it' will invalidated after pushBack
        EXPECT_ANY_THROW(array.insert(it, 2));
    }
    // Check emplace
    {
        struct Foo {
            explicit Foo(int _id, std::string _data): id(_id), data(_data) {}
            int id;
            std::string data;
        };

        ArrayType<Foo> array;
        array.emplaceBack(int{10}, std::string{"test data!"});
        ASSERT_FALSE(array.isEmpty());
        const Foo& foo = array.firstElement();
        EXPECT_EQ(foo.id, int{10});
        EXPECT_EQ(foo.data, std::string{"test data!"});
    }
}

TEST(TestDynamicArray, TestIterators) {
    constexpr std::array<int, 10> data = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    ArrayType<int> array;
    auto firstIt = array.firstIter();
    ASSERT_FALSE(firstIt.isValid());
    ASSERT_EQ(firstIt, array.firstIter());

    firstIt += 4;

    ASSERT_GT(firstIt, array.firstIter());
    ASSERT_FALSE(firstIt.isValid());

    for (auto i = 0; i < data.size(); ++i) {
        const auto v = data[i];
        array.pushBack(v);

        auto firstIt = array.firstIter();
        auto lastIt = array.lastIter();

        ASSERT_TRUE(firstIt.isValid());
        ASSERT_TRUE(lastIt.isValid());

        ASSERT_FALSE(firstIt.isExpired());
        ASSERT_FALSE(lastIt.isExpired());

        if (i == 0 /* first pushBack */) {
            EXPECT_EQ(firstIt, lastIt);
            EXPECT_EQ(*firstIt, *lastIt);
            EXPECT_EQ(*firstIt, v);
        } else /* other pushBack`s */{
            EXPECT_NE(firstIt, lastIt);
            EXPECT_NE(*firstIt, *lastIt);
            auto it = firstIt + i;
            EXPECT_EQ(*it, v);
        }
    }
    // for-each
    {
        array.clear();
        array.pushBack(std::array<int, 5>{0, 1, 2, 3, 4});
        EXPECT_EQ(array.size(), 5);
        int i = 0;
        for (auto it = array.firstConstIter(); it < array.endConstIter(); ++it) {
            ASSERT_TRUE(it.isValid());
            ASSERT_FALSE(it.isExpired());
            EXPECT_EQ(*it, i++);
        }
    }
}

TEST(TestDynamicArray, TestIteratorArithmetic) {
    ArrayType<int> array{10, 20, 30, 40, 50};
    auto it = array.firstIter();
    EXPECT_EQ(*(it + 2), 30);
    EXPECT_EQ(*(it + 3), 40);
    EXPECT_EQ(*array.lastIter(), 50);

    it += 3;
    EXPECT_EQ(*it, 40);
    it -= 2;
    EXPECT_EQ(*it, 20);
}

TEST(TestDynamicArray, TestReverseIterators) {
    constexpr std::array<int, 5> data = {10, 20, 30, 40, 50};
    ArrayType<int> array;
    {
        auto rfirst = array.firstConstReserveIter();
        auto rlast = array.lastConstReserveIter();

        ASSERT_FALSE(rfirst.isValid());
        ASSERT_FALSE(rlast.isValid());
        ASSERT_EQ(rfirst, rlast);
    }

    array.pushBack(data);
    ASSERT_EQ(array.size(), data.size());
    {
        auto rfirst = array.firstReverseIter();
        auto rlast = array.lastReverseIter();

        ASSERT_TRUE(rfirst.isValid());
        ASSERT_TRUE(rlast.isValid());
        ASSERT_NE(rfirst, rlast);

        EXPECT_EQ(*rfirst, array.lastElement());
        EXPECT_EQ(*rlast, array.firstElement());
    }
    // Reverse for-each
    {
        array.clear();
        array.pushBack(std::array<int, 5>{0, 1, 2, 3, 4});
        EXPECT_EQ(array.size(), 5);
        int i = 4;
        for (auto it = array.firstConstReserveIter(); it < array.endConstReserveIter(); ++it) {
            ASSERT_TRUE(it.isValid());
            ASSERT_FALSE(it.isExpired());
            EXPECT_EQ(*it, i--);
        }
    }
}

TEST(TestDynamicArray, TestReverseIteratorArithmetic) {
    ArrayType<int> array{10, 20, 30, 40, 50};
    auto rit = array.firstReverseIter();
    EXPECT_EQ(*array.firstReverseIter(), 50);
    EXPECT_EQ(*(array.firstReverseIter() + 2), 30);
    EXPECT_EQ(*array.lastReverseIter(), 10);

    rit += 3;
    EXPECT_EQ(*rit, 20);
    rit -= 2;
    EXPECT_EQ(*rit, 40);
}
