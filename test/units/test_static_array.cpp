#include <gtest/gtest.h>

#include "include/containers/static/array.h"

template<class T, std::size_t N>
using ArrayType = atom::containers::StaticArray<T, N>;

TEST(TestStaticArray, TestIteratorsWithEmptyArray) {
    ArrayType<int, 5> array;
    EXPECT_EQ(array.firstIter(), array.endIter());
    EXPECT_EQ(array.firstConstIter(), array.endConstIter());
    EXPECT_EQ(array.firstReverseIter(), array.endReverseIter());
    EXPECT_EQ(array.firstReverseConstIter(), array.endReverseConstIter());
}

TEST(TestStaticArray, TestOverflowExceptions) {
    ArrayType<int, 2> array;
    EXPECT_NO_THROW(array.pushBack(1));
    EXPECT_NO_THROW(array.pushBack(2));
    EXPECT_ANY_THROW(array.pushBack(3));
    EXPECT_ANY_THROW(array.emplaceBack(3));
    EXPECT_ANY_THROW(array.insert(array.firstConstIter(), 0));
}

TEST(TestStaticArray, TestBoundsChecking) {
    ArrayType<int, 5> array;
    array.pushBack(1);
    array.pushBack(2);

    EXPECT_NO_THROW(array[0]);
    EXPECT_NO_THROW(array[1]);
    EXPECT_ANY_THROW(array[2]);
    EXPECT_ANY_THROW(array[100]);

    const auto& constArray = array;
    EXPECT_NO_THROW(constArray[0]);
    EXPECT_ANY_THROW(constArray[2]);
}

TEST(TestStaticArray, TestSubscriptOperator) {
    ArrayType<int, 5> array;
    array.pushBack(10);
    array.pushBack(20);

    EXPECT_EQ(array[0], 10);
    EXPECT_EQ(array[1], 20);

    array[0] = 100;
    EXPECT_EQ(array[0], 100);

    const auto& constArray = array;
    EXPECT_EQ(constArray[0], 100);
}

TEST(TestStaticArray, TestDataAndFrontBack) {
    ArrayType<int, 5> array;
    array.pushBack(1);
    array.pushBack(2);
    array.pushBack(3);

    int* dataPtr = array.data();
    EXPECT_EQ(dataPtr[0], 1);
    EXPECT_EQ(dataPtr[1], 2);

    EXPECT_EQ(array.firstElement(), 1);
    EXPECT_EQ(array.lastElement(), 3);

    const auto& constArray = array;
    EXPECT_EQ(constArray.firstElement(), 1);
    EXPECT_EQ(constArray.lastElement(), 3);
}

TEST(TestStaticArray, TestEmptyAndFull) {
    ArrayType<int, 3> array;
    EXPECT_TRUE(array.isEmpty());
    EXPECT_FALSE(array.isFull());

    array.pushBack(1);
    EXPECT_FALSE(array.isEmpty());
    EXPECT_FALSE(array.isFull());

    array.pushBack(2);
    array.pushBack(3);
    EXPECT_FALSE(array.isEmpty());
    EXPECT_TRUE(array.isFull());
}

TEST(TestStaticArray, TestWithSize) {
    {
        ArrayType<std::uint8_t, 5> array;
        EXPECT_EQ(array.size(), 0);
        EXPECT_EQ(array.capacity(), 5);

        for (auto i = 0; i < array.capacity(); ++i) {
            array.pushBack(i);
        }
        EXPECT_EQ(array.size(), array.capacity());
        auto i = 0;
        for (auto it = array.firstConstIter(); it != array.endConstIter(); ++it, ++i) {
            EXPECT_EQ(*it, i);
        }
    }
    {
        struct Foo {
            explicit Foo(int _value): value(_value) {}
            int value;
        };
        ArrayType<Foo, 3> array;
        EXPECT_EQ(array.size(), 0);
        EXPECT_EQ(array.capacity(), 3);

        for (auto i = 0; i < array.capacity(); ++i) {
            array.pushBack(Foo{i});
        }
        EXPECT_EQ(array.size(), array.capacity());
        auto i = 0;
        for (auto it = array.firstConstIter(); it != array.endConstIter(); ++it, ++i) {
            EXPECT_EQ(it->value, i);
        }
    }
}

TEST(TestStaticArray, TestDestructor) {
    std::size_t counter{0};
    struct Foo {
        Foo(const Foo&) = delete;
        Foo& operator=(const Foo& ) = delete;
        Foo(Foo&& ) noexcept = default;
        Foo& operator=(Foo&& ) noexcept = default;

        explicit Foo(std::size_t& counter): m_counter(&counter) {
            ++(*m_counter);
        }
        ~Foo() {
            if (m_counter)
                --(*m_counter);
        }
        std::size_t* m_counter{nullptr};
    };
    {
        ArrayType<Foo, 10> array;
        for (auto i = 0; i < array.capacity(); ++i) {
            array.emplaceBack(counter);
        }
        EXPECT_EQ(counter, 10);
    }
    EXPECT_EQ(counter, 0);
}

TEST(TestStaticArray, TestConstructors) {
    struct Foo {
        explicit Foo(int _value): value(_value) {}
        int value;
    };
    {
        ArrayType<Foo, 64> array;
        EXPECT_EQ(array.size(), 0);
        EXPECT_EQ(array.capacity(), 64);
    }
    {
        ArrayType<Foo, 64> array{Foo{int{101}}, ArrayType<Foo, 64>::SizeType{10}};
        EXPECT_EQ(array.size(), 10);
        for (auto it = array.firstConstIter(); it != array.endConstIter(); ++it) {
            EXPECT_EQ(it->value, int{101});
        }
    }
    {
        ArrayType<Foo, 64> array{{Foo{10}, Foo{20}, Foo{30}}};
        EXPECT_EQ(array.size(), 3);
        for (auto i = 0; i < array.size(); ++i) {
            EXPECT_EQ(array.atUnsafe(i).value, (i + 1) * 10);
        }
    }
}

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
            array.insert(array.endConstIter(), d);
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
        for (int i = 0, j = data.size() - 1; i < data.size(); ++i, --j) {
            EXPECT_EQ(array[i], data[j]);
        }
        array.clear();
        EXPECT_TRUE(array.isEmpty());
    }
    // Check insert into middle
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
    {
        struct Foo {
            explicit Foo(int _id): id(_id) {}
            int id;
        };

        ArrayType<Foo, 1024> array;
        EXPECT_TRUE(array.isEmpty());
        array.emplaceBack(int{10});
        ASSERT_FALSE(array.isEmpty());
        const Foo& foo = array.firstElement();
        EXPECT_EQ(foo.id, int{10});

        (void)array.insert(array.firstConstIter(), Foo{int{20}});
        EXPECT_EQ(array.size(), 2);
        EXPECT_EQ(array.firstElement().id, int{20});
    }
    // Check insert return iterators
    {
        constexpr std::array<int, 5> data = {0, 1, 2, 3, 4};
        array.clear();
        array.pushBack(data);
        ASSERT_EQ(array.size(), data.size());
        auto i = 0;
        for (auto it = array.firstConstIter(); it != array.endConstIter(); ++it, ++i) {
            ASSERT_TRUE(!it.isExpired() && !it.isOutOfRange());
            EXPECT_EQ(*it, data[i]);
        }

        auto it = array.lastIter();
        EXPECT_EQ(*it, 4);

        // insert into the end
        it = array.insert(it, int{5});
        EXPECT_EQ(array.size(), data.size() + 1);
        EXPECT_TRUE(!it.isExpired() && !it.isOutOfRange());
        EXPECT_EQ(*it, int{5});

        it = array.firstIter();
        EXPECT_EQ(*it, 0);

        // insert into the begin
        it = array.insert(it, {{int{6}, int{7}}});
        EXPECT_EQ(array.size(), data.size() + 3);
        EXPECT_TRUE(!it.isExpired() && !it.isOutOfRange());
        EXPECT_EQ(*it, int{6});
        EXPECT_EQ(*(++it), int{7});

        // insert into the middle
        it = array.firstIter() + 4;
        EXPECT_TRUE(!it.isExpired() && !it.isOutOfRange());
        EXPECT_EQ(*it, 2);

        it = array.insert(it, int{10});
        EXPECT_TRUE(!it.isExpired() && !it.isOutOfRange());
        EXPECT_EQ(*it, 10);
    }
}


TEST(TestStaticArray, TestInsertNonTrivialTypes) {
    struct Foo {
        Foo(const Foo& ) = default;
        Foo(Foo&& ) noexcept = default;
        Foo& operator=(const Foo& ) = default;
        Foo& operator=(Foo&& ) noexcept = default;

        explicit Foo(int _value): value(_value) {}
        bool operator==(const Foo& other) const { return value == other.value; }
        int value;
    };

    ArrayType<Foo, 100> array;
    // Check insert into the begin
    {
        array.clear();
        array.emplaceBack(2);
        array.emplaceBack(3);
        array.emplaceBack(4);

        auto it = array.insert(array.firstConstIter(), Foo(1));

        EXPECT_EQ(array.size(), 4);
        EXPECT_EQ(it->value, 1);
        EXPECT_EQ(array[0].value, 1);
        EXPECT_EQ(array[1].value, 2);
        EXPECT_EQ(array[2].value, 3);
        EXPECT_EQ(array[3].value, 4);
    }

    // Check insert into the middle
    {
        array.clear();
        array.emplaceBack(1);
        array.emplaceBack(2);
        array.emplaceBack(4);
        array.emplaceBack(5);

        auto middle_it = array.firstConstIter() + 2;
        auto it = array.insert(middle_it, Foo(3));

        EXPECT_EQ(array.size(), 5);
        EXPECT_EQ(it->value, 3);
        EXPECT_EQ(array[0].value, 1);
        EXPECT_EQ(array[1].value, 2);
        EXPECT_EQ(array[2].value, 3);
        EXPECT_EQ(array[3].value, 4);
        EXPECT_EQ(array[4].value, 5);
    }

    // Check insert into the end
    {
        array.clear();
        array.emplaceBack(1);
        array.emplaceBack(2);

        auto it = array.insert(array.endConstIter(), Foo(3));

        EXPECT_EQ(array.size(), 3);
        EXPECT_EQ(it->value, 3);
        EXPECT_EQ(array[0].value, 1);
        EXPECT_EQ(array[1].value, 2);
        EXPECT_EQ(array[2].value, 3);
    }

    // Check insert range into the begin
    {
        array.clear();
        array.emplaceBack(4);
        array.emplaceBack(5);
        array.emplaceBack(6);

        std::array<Foo, 3> data = {Foo(1), Foo(2), Foo(3)};
        auto it = array.insert(array.firstConstIter(), {data.begin(), data.end()});

        EXPECT_EQ(array.size(), 6);
        EXPECT_EQ(it->value, 1);
        EXPECT_EQ(array[0].value, 1);
        EXPECT_EQ(array[1].value, 2);
        EXPECT_EQ(array[2].value, 3);
        EXPECT_EQ(array[3].value, 4);
        EXPECT_EQ(array[4].value, 5);
        EXPECT_EQ(array[5].value, 6);
    }

    // Check insert range into the middle
    {
        array.clear();
        array.emplaceBack(1);
        array.emplaceBack(5);
        array.emplaceBack(6);

        std::array<Foo, 3> data = {Foo(2), Foo(3), Foo(4)};
        auto middle_it = array.firstConstIter() + 1;
        auto it = array.insert(middle_it, {data.begin(), data.end()});

        EXPECT_EQ(array.size(), 6);
        EXPECT_EQ(it->value, 2);
        EXPECT_EQ(array[0].value, 1);
        EXPECT_EQ(array[1].value, 2);
        EXPECT_EQ(array[2].value, 3);
        EXPECT_EQ(array[3].value, 4);
        EXPECT_EQ(array[4].value, 5);
        EXPECT_EQ(array[5].value, 6);
    }

    // Check insert range into the ebd
    {
        array.clear();
        array.emplaceBack(1);
        array.emplaceBack(2);

        std::array<Foo, 2> data = {Foo(3), Foo(4)};
        auto it = array.insert(array.endConstIter(), {data.begin(), data.end()});

        EXPECT_EQ(array.size(), 4);
        EXPECT_EQ(it->value, 3);
        EXPECT_EQ(array[0].value, 1);
        EXPECT_EQ(array[1].value, 2);
        EXPECT_EQ(array[2].value, 3);
        EXPECT_EQ(array[3].value, 4);
    }
}

TEST(TestStaticArray, TestInsertOnlyMovableNonTrivialTypes) {
    struct Foo {
        Foo(const Foo& ) = delete;
        Foo(Foo&& ) noexcept = default;
        Foo& operator=(const Foo& ) = delete;
        Foo& operator=(Foo&& ) noexcept = default;

        explicit Foo(int _value): value(_value) {}
        bool operator==(const Foo& other) const { return value == other.value; }
        int value;
    };

    ArrayType<Foo, 100> array;
    // Check insert into the begin
    {
        array.clear();
        array.emplaceBack(2);
        array.emplaceBack(3);
        array.emplaceBack(4);

        auto it = array.insert(array.firstConstIter(), Foo(1));

        EXPECT_EQ(array.size(), 4);
        EXPECT_EQ(it->value, 1);
        EXPECT_EQ(array[0].value, 1);
        EXPECT_EQ(array[1].value, 2);
        EXPECT_EQ(array[2].value, 3);
        EXPECT_EQ(array[3].value, 4);
    }

    // Check insert into the middle
    {
        array.clear();
        array.emplaceBack(1);
        array.emplaceBack(2);
        array.emplaceBack(4);
        array.emplaceBack(5);

        auto middle_it = array.firstConstIter() + 2;
        auto it = array.insert(middle_it, Foo(3));

        EXPECT_EQ(array.size(), 5);
        EXPECT_EQ(it->value, 3);
        EXPECT_EQ(array[0].value, 1);
        EXPECT_EQ(array[1].value, 2);
        EXPECT_EQ(array[2].value, 3);
        EXPECT_EQ(array[3].value, 4);
        EXPECT_EQ(array[4].value, 5);
    }

    // Check insert into the end
    {
        array.clear();
        array.emplaceBack(1);
        array.emplaceBack(2);

        auto it = array.insert(array.endConstIter(), Foo(3));

        EXPECT_EQ(array.size(), 3);
        EXPECT_EQ(it->value, 3);
        EXPECT_EQ(array[0].value, 1);
        EXPECT_EQ(array[1].value, 2);
        EXPECT_EQ(array[2].value, 3);
    }

    // Check insert range into the begin
    {
        array.clear();
        array.emplaceBack(4);
        array.emplaceBack(5);
        array.emplaceBack(6);

        std::array<Foo, 3> data = {Foo(1), Foo(2), Foo(3)};
        for (auto i = 0; i < data.size(); ++i) {
            auto it = array.firstConstIter() + i;
            array.insert(it, std::move(data[i]));
        }

        EXPECT_EQ(array.size(), 6);
        EXPECT_EQ(array[0].value, 1);
        EXPECT_EQ(array[1].value, 2);
        EXPECT_EQ(array[2].value, 3);
        EXPECT_EQ(array[3].value, 4);
        EXPECT_EQ(array[4].value, 5);
        EXPECT_EQ(array[5].value, 6);
    }

    // Check insert range into the middle
    {
        array.clear();
        array.emplaceBack(1);
        array.emplaceBack(5);
        array.emplaceBack(6);

        std::array<Foo, 3> data = {Foo(2), Foo(3), Foo(4)};
        for (auto i = 0; i < data.size(); ++i) {
            auto it = array.firstConstIter() + 1 + i;
            array.insert(it, std::move(data[i]));
        }

        EXPECT_EQ(array.size(), 6);
        EXPECT_EQ(array[0].value, 1);
        EXPECT_EQ(array[1].value, 2);
        EXPECT_EQ(array[2].value, 3);
        EXPECT_EQ(array[3].value, 4);
        EXPECT_EQ(array[4].value, 5);
        EXPECT_EQ(array[5].value, 6);
    }

    // Check insert range into the ebd
    {
        array.clear();
        array.emplaceBack(1);
        array.emplaceBack(2);

        std::array<Foo, 2> data = {Foo(3), Foo(4)};
        for (auto i = 0; i < data.size(); ++i) {
            array.insert(array.endConstIter(), std::move(data[i]));
        }

        EXPECT_EQ(array.size(), 4);
        EXPECT_EQ(array[0].value, 1);
        EXPECT_EQ(array[1].value, 2);
        EXPECT_EQ(array[2].value, 3);
        EXPECT_EQ(array[3].value, 4);
    }
}

TEST(TestStaticArray, TestInsertOnlyCopyableNonTrivialTypes) {
    struct Foo {
        Foo(const Foo& ) = default;
        Foo(Foo&& ) noexcept = delete;
        Foo& operator=(const Foo& ) = default;
        Foo& operator=(Foo&& ) noexcept = delete;

        explicit Foo(int _value): value(_value) {}
        bool operator==(const Foo& other) const { return value == other.value; }
        int value;
    };

    ArrayType<Foo, 100> array;
    // Check insert into the begin
    {
        array.clear();
        array.emplaceBack(2);
        array.emplaceBack(3);
        array.emplaceBack(4);

        auto it = array.insert(array.firstConstIter(), Foo(1));

        EXPECT_EQ(array.size(), 4);
        EXPECT_EQ(it->value, 1);
        EXPECT_EQ(array[0].value, 1);
        EXPECT_EQ(array[1].value, 2);
        EXPECT_EQ(array[2].value, 3);
        EXPECT_EQ(array[3].value, 4);
    }

    // Check insert into the middle
    {
        array.clear();
        array.emplaceBack(1);
        array.emplaceBack(2);
        array.emplaceBack(4);
        array.emplaceBack(5);

        auto middle_it = array.firstConstIter() + 2;
        auto it = array.insert(middle_it, Foo(3));

        EXPECT_EQ(array.size(), 5);
        EXPECT_EQ(it->value, 3);
        EXPECT_EQ(array[0].value, 1);
        EXPECT_EQ(array[1].value, 2);
        EXPECT_EQ(array[2].value, 3);
        EXPECT_EQ(array[3].value, 4);
        EXPECT_EQ(array[4].value, 5);
    }

    // Check insert into the end
    {
        array.clear();
        array.emplaceBack(1);
        array.emplaceBack(2);

        auto it = array.insert(array.endConstIter(), Foo(3));

        EXPECT_EQ(array.size(), 3);
        EXPECT_EQ(it->value, 3);
        EXPECT_EQ(array[0].value, 1);
        EXPECT_EQ(array[1].value, 2);
        EXPECT_EQ(array[2].value, 3);
    }

    // Check insert of range into the begin
    {
        array.clear();
        array.emplaceBack(4);
        array.emplaceBack(5);
        array.emplaceBack(6);

        std::array<Foo, 3> data = {Foo(1), Foo(2), Foo(3)};
        auto it = array.insert(array.firstConstIter(), {data.begin(), data.end()});

        EXPECT_EQ(array.size(), 6);
        EXPECT_EQ(it->value, 1);
        EXPECT_EQ(array[0].value, 1);
        EXPECT_EQ(array[1].value, 2);
        EXPECT_EQ(array[2].value, 3);
        EXPECT_EQ(array[3].value, 4);
        EXPECT_EQ(array[4].value, 5);
        EXPECT_EQ(array[5].value, 6);
    }

    // Check insert of range into the middle
    {
        array.clear();
        array.emplaceBack(1);
        array.emplaceBack(5);
        array.emplaceBack(6);

        std::array<Foo, 3> data = {Foo(2), Foo(3), Foo(4)};
        auto middle_it = array.firstConstIter() + 1;
        auto it = array.insert(middle_it, {data.begin(), data.end()});

        EXPECT_EQ(array.size(), 6);
        EXPECT_EQ(it->value, 2);
        EXPECT_EQ(array[0].value, 1);
        EXPECT_EQ(array[1].value, 2);
        EXPECT_EQ(array[2].value, 3);
        EXPECT_EQ(array[3].value, 4);
        EXPECT_EQ(array[4].value, 5);
        EXPECT_EQ(array[5].value, 6);
    }

    // Check insert of range into the ebd
    {
        array.clear();
        array.emplaceBack(1);
        array.emplaceBack(2);

        std::array<Foo, 2> data = {Foo(3), Foo(4)};
        auto it = array.insert(array.endConstIter(), {data.begin(), data.end()});

        EXPECT_EQ(array.size(), 4);
        EXPECT_EQ(it->value, 3);
        EXPECT_EQ(array[0].value, 1);
        EXPECT_EQ(array[1].value, 2);
        EXPECT_EQ(array[2].value, 3);
        EXPECT_EQ(array[3].value, 4);
    }
}

TEST(TestStaticArray, TestPushAndEmplaceBack) {
    ArrayType<int, 10> array{5, 6};
    array.pushBack(int{10});
    EXPECT_EQ(array.size(), 3);
    EXPECT_EQ(array.lastElement(), int{10});

    array.emplaceBack(int{20});
    EXPECT_EQ(array.size(), 4);
    EXPECT_EQ(array.lastElement(), int{20});
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
    // Check erase from the middle
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

        auto it = array.firstIter() + 1;
        ASSERT_TRUE(!it.isExpired() && !it.isOutOfRange() && *it == 20);

        it = array.erase(it);
        ASSERT_TRUE(!it.isExpired() && !it.isOutOfRange());
        EXPECT_EQ(*it, 30);
        EXPECT_TRUE(array.size() == 2 && array.atUnsafe(0) == 10 && array.atUnsafe(1) == 30);

        it = array.erase(array.lastConstIter());
        EXPECT_EQ(it, array.endIter());
        EXPECT_TRUE(array.size() == 1 && array.atUnsafe(0) == 10);

        it = array.erase(array.lastConstIter());
        EXPECT_EQ(it, array.endIter());
        EXPECT_TRUE(array.isEmpty());
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
    // Check erase return iterators
    {
        constexpr std::array<int, 7> data = {10, 20, 30, 40, 50, 60, 70};
        array.clear();
        array.pushBack(data);

        auto it = array.firstIter();
        EXPECT_TRUE(!it.isExpired() && !it.isOutOfRange());
        EXPECT_EQ(*it, 10);

        // erase from the begin
        it = array.erase(it, it + 2);
        EXPECT_EQ(array.size(), data.size() - 2);
        EXPECT_TRUE(!it.isExpired() && !it.isOutOfRange());
        EXPECT_EQ(*it, 30);

        // erase from the end
        it = array.lastIter();
        EXPECT_EQ(*it, 70);

        it = array.erase(it);
        EXPECT_EQ(array.size(), data.size() - 3);
        EXPECT_EQ(it, array.endIter());

        // erase from the minddle
        it = array.firstIter() + 2;
        EXPECT_EQ(*it, 50);

        it = array.erase(it, array.endIter());
        EXPECT_EQ(array.size(), 2);
        EXPECT_EQ(it, array.endIter());

        it = array.erase(array.firstIter(), array.endIter());
        EXPECT_TRUE(array.isEmpty());
        EXPECT_EQ(it, array.endIter());
    }
}

TEST(TestStaticArray, TestEraseNonTrivialTypes) {
    struct Foo {
        Foo(const Foo& ) = default;
        Foo(Foo&& ) noexcept = default;
        Foo& operator=(const Foo& ) = default;
        Foo& operator=(Foo&& ) noexcept = default;

        explicit Foo(int _value): value(_value) {}
        bool operator==(const Foo& other) const { return value == other.value; }
        int value;
    };

    ArrayType<Foo, 100> array;

    // Check erase from the begin
    {
        array.clear();
        array.emplaceBack(10);
        array.emplaceBack(20);
        array.emplaceBack(30);
        array.emplaceBack(40);

        auto it = array.erase(array.firstConstIter());

        EXPECT_EQ(array.size(), 3);
        EXPECT_EQ(it->value, 20);
        EXPECT_EQ(array[0].value, 20);
        EXPECT_EQ(array[1].value, 30);
        EXPECT_EQ(array[2].value, 40);
    }

    // Check erase from the middle
    {
        array.clear();
        array.emplaceBack(10);
        array.emplaceBack(20);
        array.emplaceBack(30);
        array.emplaceBack(40);

        auto middle_it = array.firstConstIter() + 1;
        auto it = array.erase(middle_it);

        EXPECT_EQ(array.size(), 3);
        EXPECT_EQ(it->value, 30);
        EXPECT_EQ(array[0].value, 10);
        EXPECT_EQ(array[1].value, 30);
        EXPECT_EQ(array[2].value, 40);
    }

    // Check erase from the end
    {
        array.clear();
        array.emplaceBack(10);
        array.emplaceBack(20);
        array.emplaceBack(30);

        auto it = array.erase(array.lastConstIter());

        EXPECT_EQ(array.size(), 2);
        EXPECT_EQ(it, array.endIter());
        EXPECT_EQ(array[0].value, 10);
        EXPECT_EQ(array[1].value, 20);
    }

    // Check erase range from the begin
    {
        array.clear();
        array.emplaceBack(10);
        array.emplaceBack(20);
        array.emplaceBack(30);
        array.emplaceBack(40);
        array.emplaceBack(50);

        auto first = array.firstConstIter();
        auto last = array.firstConstIter() + 2;
        auto it = array.erase(first, last);

        EXPECT_EQ(array.size(), 3);
        EXPECT_EQ(it->value, 30);
        EXPECT_EQ(array[0].value, 30);
        EXPECT_EQ(array[1].value, 40);
        EXPECT_EQ(array[2].value, 50);
    }

    // Check erase range from the middle
    {
        array.clear();
        array.emplaceBack(10);
        array.emplaceBack(20);
        array.emplaceBack(30);
        array.emplaceBack(40);
        array.emplaceBack(50);

        auto first = array.firstConstIter() + 1;
        auto last = array.firstConstIter() + 3;
        auto it = array.erase(first, last);

        EXPECT_EQ(array.size(), 3);
        EXPECT_EQ(it->value, 40);
        EXPECT_EQ(array[0].value, 10);
        EXPECT_EQ(array[1].value, 40);
        EXPECT_EQ(array[2].value, 50);
    }

    // Check erase range from the end
    {
        array.clear();
        array.emplaceBack(10);
        array.emplaceBack(20);
        array.emplaceBack(30);
        array.emplaceBack(40);
        array.emplaceBack(50);

        auto first = array.firstConstIter() + 2;
        auto last = array.endConstIter();
        auto it = array.erase(first, last);

        EXPECT_EQ(array.size(), 2);
        EXPECT_EQ(it, array.endIter());
        EXPECT_EQ(array[0].value, 10);
        EXPECT_EQ(array[1].value, 20);
    }

    // Check erase all elements
    {
        array.clear();
        array.emplaceBack(10);
        array.emplaceBack(20);
        array.emplaceBack(30);

        auto it = array.erase(array.firstConstIter(), array.endConstIter());

        EXPECT_EQ(array.size(), 0);
        EXPECT_EQ(it, array.endIter());
        EXPECT_TRUE(array.isEmpty());
    }
    {
        array.clear();
        array.emplaceBack(42);

        auto it = array.erase(array.firstConstIter());

        EXPECT_EQ(array.size(), 0);
        EXPECT_EQ(it, array.endIter());
        EXPECT_TRUE(array.isEmpty());
    }
    {
        array.clear();
        array.emplaceBack(10);
        array.emplaceBack(20);
        array.emplaceBack(30);
        array.emplaceBack(40);
        array.emplaceBack(50);

        auto it = array.erase(array.firstConstIter());
        EXPECT_EQ(it->value, 20);
        EXPECT_EQ(array.size(), 4);

        it = array.erase(array.firstConstIter() + 1);
        EXPECT_EQ(it->value, 40);
        EXPECT_EQ(array.size(), 3);

        it = array.erase(array.lastConstIter());
        EXPECT_EQ(it, array.endIter());
        EXPECT_EQ(array.size(), 2);

        EXPECT_EQ(array[0].value, 20);
        EXPECT_EQ(array[1].value, 40);
    }

    {
        array.clear();
        array.emplaceBack(100);
        array.emplaceBack(200);
        array.emplaceBack(300);
        array.emplaceBack(400);

        auto const_it = array.firstConstIter() + 1;
        auto result_it = array.erase(const_it);
        EXPECT_EQ(result_it->value, 300);
        EXPECT_EQ(array.size(), 3);

        auto mut_it = array.firstIter() + 1;
        result_it = array.erase(mut_it);
        EXPECT_EQ(result_it->value, 400);
        EXPECT_EQ(array.size(), 2);

        EXPECT_EQ(array[0].value, 100);
        EXPECT_EQ(array[1].value, 400);
    }
    {
        array.clear();
        array.emplaceBack(1);
        array.emplaceBack(2);

        EXPECT_ANY_THROW(array.erase(array.endConstIter()));

        auto it = array.erase(array.firstConstIter());
        EXPECT_EQ(it->value, 2);

        EXPECT_NO_THROW(array.erase(array.firstConstIter()));
        it += 100;
        EXPECT_ANY_THROW(array.erase(it));
    }
}

TEST(TestStaticArray, TestEraseOnlyMovableNonTrivialTypes) {
    struct Foo {
        Foo(const Foo& ) = delete;
        Foo(Foo&& ) noexcept = default;
        Foo& operator=(const Foo& ) = delete;
        Foo& operator=(Foo&& ) noexcept = default;

        explicit Foo(int _value): value(_value) {}
        bool operator==(const Foo& other) const { return value == other.value; }
        int value;
    };

    ArrayType<Foo, 100> array;

    // Check erase from the begin
    {
        array.clear();
        array.emplaceBack(10);
        array.emplaceBack(20);
        array.emplaceBack(30);
        array.emplaceBack(40);

        auto it = array.erase(array.firstConstIter());

        EXPECT_EQ(array.size(), 3);
        EXPECT_EQ(it->value, 20);
        EXPECT_EQ(array[0].value, 20);
        EXPECT_EQ(array[1].value, 30);
        EXPECT_EQ(array[2].value, 40);
    }

    // Check erase from the middle
    {
        array.clear();
        array.emplaceBack(10);
        array.emplaceBack(20);
        array.emplaceBack(30);
        array.emplaceBack(40);

        auto middle_it = array.firstConstIter() + 1;
        auto it = array.erase(middle_it);

        EXPECT_EQ(array.size(), 3);
        EXPECT_EQ(it->value, 30);
        EXPECT_EQ(array[0].value, 10);
        EXPECT_EQ(array[1].value, 30);
        EXPECT_EQ(array[2].value, 40);
    }

    // Check erase from the end
    {
        array.clear();
        array.emplaceBack(10);
        array.emplaceBack(20);
        array.emplaceBack(30);

        auto it = array.erase(array.lastConstIter());

        EXPECT_EQ(array.size(), 2);
        EXPECT_EQ(it, array.endIter());
        EXPECT_EQ(array[0].value, 10);
        EXPECT_EQ(array[1].value, 20);
    }

    // Check erase range from the begin
    {
        array.clear();
        array.emplaceBack(10);
        array.emplaceBack(20);
        array.emplaceBack(30);
        array.emplaceBack(40);
        array.emplaceBack(50);

        auto first = array.firstConstIter();
        auto last = array.firstConstIter() + 2;
        auto it = array.erase(first, last);

        EXPECT_EQ(array.size(), 3);
        EXPECT_EQ(it->value, 30);
        EXPECT_EQ(array[0].value, 30);
        EXPECT_EQ(array[1].value, 40);
        EXPECT_EQ(array[2].value, 50);
    }

    // Check erase range from the middle
    {
        array.clear();
        array.emplaceBack(10);
        array.emplaceBack(20);
        array.emplaceBack(30);
        array.emplaceBack(40);
        array.emplaceBack(50);

        auto first = array.firstConstIter() + 1;
        auto last = array.firstConstIter() + 3;
        auto it = array.erase(first, last);

        EXPECT_EQ(array.size(), 3);
        EXPECT_EQ(it->value, 40);
        EXPECT_EQ(array[0].value, 10);
        EXPECT_EQ(array[1].value, 40);
        EXPECT_EQ(array[2].value, 50);
    }

    // Check erase range from the end
    {
        array.clear();
        array.emplaceBack(10);
        array.emplaceBack(20);
        array.emplaceBack(30);
        array.emplaceBack(40);
        array.emplaceBack(50);

        auto first = array.firstConstIter() + 2;
        auto last = array.endConstIter();
        auto it = array.erase(first, last);

        EXPECT_EQ(array.size(), 2);
        EXPECT_EQ(it, array.endIter());
        EXPECT_EQ(array[0].value, 10);
        EXPECT_EQ(array[1].value, 20);
    }

    // Check erase all elements
    {
        array.clear();
        array.emplaceBack(10);
        array.emplaceBack(20);
        array.emplaceBack(30);

        auto it = array.erase(array.firstConstIter(), array.endConstIter());

        EXPECT_EQ(array.size(), 0);
        EXPECT_EQ(it, array.endIter());
        EXPECT_TRUE(array.isEmpty());
    }
    {
        array.clear();
        array.emplaceBack(42);

        auto it = array.erase(array.firstConstIter());

        EXPECT_EQ(array.size(), 0);
        EXPECT_EQ(it, array.endIter());
        EXPECT_TRUE(array.isEmpty());
    }
    {
        array.clear();
        array.emplaceBack(10);
        array.emplaceBack(20);
        array.emplaceBack(30);
        array.emplaceBack(40);
        array.emplaceBack(50);

        auto it = array.erase(array.firstConstIter());
        EXPECT_EQ(it->value, 20);
        EXPECT_EQ(array.size(), 4);

        it = array.erase(array.firstConstIter() + 1);
        EXPECT_EQ(it->value, 40);
        EXPECT_EQ(array.size(), 3);

        it = array.erase(array.lastConstIter());
        EXPECT_EQ(it, array.endIter());
        EXPECT_EQ(array.size(), 2);

        EXPECT_EQ(array[0].value, 20);
        EXPECT_EQ(array[1].value, 40);
    }

    {
        array.clear();
        array.emplaceBack(100);
        array.emplaceBack(200);
        array.emplaceBack(300);
        array.emplaceBack(400);

        auto const_it = array.firstConstIter() + 1;
        auto result_it = array.erase(const_it);
        EXPECT_EQ(result_it->value, 300);
        EXPECT_EQ(array.size(), 3);

        auto mut_it = array.firstIter() + 1;
        result_it = array.erase(mut_it);
        EXPECT_EQ(result_it->value, 400);
        EXPECT_EQ(array.size(), 2);

        EXPECT_EQ(array[0].value, 100);
        EXPECT_EQ(array[1].value, 400);
    }
    {
        array.clear();
        array.emplaceBack(1);
        array.emplaceBack(2);

        EXPECT_ANY_THROW(array.erase(array.endConstIter()));

        auto it = array.erase(array.firstConstIter());
        EXPECT_EQ(it->value, 2);

        EXPECT_NO_THROW(array.erase(array.firstConstIter()));
        it += 100;
        EXPECT_ANY_THROW(array.erase(it));
    }
}

TEST(TestStaticArray, TestEraseOnlyCopyableNonTrivialTypes) {
    struct Foo {
        Foo(const Foo& ) = default;
        Foo(Foo&& ) = delete;
        Foo& operator=(const Foo& ) = default;
        Foo& operator=(Foo&& ) = delete;

        explicit Foo(int _value): value(_value) {}
        bool operator==(const Foo& other) const { return value == other.value; }
        int value;
    };

    ArrayType<Foo, 100> array;

    // Check erase from the begin
    {
        array.clear();
        array.emplaceBack(10);
        array.emplaceBack(20);
        array.emplaceBack(30);
        array.emplaceBack(40);

        auto it = array.erase(array.firstConstIter());

        EXPECT_EQ(array.size(), 3);
        EXPECT_EQ(it->value, 20);
        EXPECT_EQ(array[0].value, 20);
        EXPECT_EQ(array[1].value, 30);
        EXPECT_EQ(array[2].value, 40);
    }

    // Check erase from the middle
    {
        array.clear();
        array.emplaceBack(10);
        array.emplaceBack(20);
        array.emplaceBack(30);
        array.emplaceBack(40);

        auto middle_it = array.firstConstIter() + 1;
        auto it = array.erase(middle_it);

        EXPECT_EQ(array.size(), 3);
        EXPECT_EQ(it->value, 30);
        EXPECT_EQ(array[0].value, 10);
        EXPECT_EQ(array[1].value, 30);
        EXPECT_EQ(array[2].value, 40);
    }

    // Check erase from the end
    {
        array.clear();
        array.emplaceBack(10);
        array.emplaceBack(20);
        array.emplaceBack(30);

        auto it = array.erase(array.lastConstIter());

        EXPECT_EQ(array.size(), 2);
        EXPECT_EQ(it, array.endIter());
        EXPECT_EQ(array[0].value, 10);
        EXPECT_EQ(array[1].value, 20);
    }

    // Check erase range from the begin
    {
        array.clear();
        array.emplaceBack(10);
        array.emplaceBack(20);
        array.emplaceBack(30);
        array.emplaceBack(40);
        array.emplaceBack(50);

        auto first = array.firstConstIter();
        auto last = array.firstConstIter() + 2;
        auto it = array.erase(first, last);

        EXPECT_EQ(array.size(), 3);
        EXPECT_EQ(it->value, 30);
        EXPECT_EQ(array[0].value, 30);
        EXPECT_EQ(array[1].value, 40);
        EXPECT_EQ(array[2].value, 50);
    }

    // Check erase range from the middle
    {
        array.clear();
        array.emplaceBack(10);
        array.emplaceBack(20);
        array.emplaceBack(30);
        array.emplaceBack(40);
        array.emplaceBack(50);

        auto first = array.firstConstIter() + 1;
        auto last = array.firstConstIter() + 3;
        auto it = array.erase(first, last);

        EXPECT_EQ(array.size(), 3);
        EXPECT_EQ(it->value, 40);
        EXPECT_EQ(array[0].value, 10);
        EXPECT_EQ(array[1].value, 40);
        EXPECT_EQ(array[2].value, 50);
    }

    // Check erase range from the end
    {
        array.clear();
        array.emplaceBack(10);
        array.emplaceBack(20);
        array.emplaceBack(30);
        array.emplaceBack(40);
        array.emplaceBack(50);

        auto first = array.firstConstIter() + 2;
        auto last = array.endConstIter();
        auto it = array.erase(first, last);

        EXPECT_EQ(array.size(), 2);
        EXPECT_EQ(it, array.endIter());
        EXPECT_EQ(array[0].value, 10);
        EXPECT_EQ(array[1].value, 20);
    }

    // Check erase all elements
    {
        array.clear();
        array.emplaceBack(10);
        array.emplaceBack(20);
        array.emplaceBack(30);

        auto it = array.erase(array.firstConstIter(), array.endConstIter());

        EXPECT_EQ(array.size(), 0);
        EXPECT_EQ(it, array.endIter());
        EXPECT_TRUE(array.isEmpty());
    }
    {
        array.clear();
        array.emplaceBack(42);

        auto it = array.erase(array.firstConstIter());

        EXPECT_EQ(array.size(), 0);
        EXPECT_EQ(it, array.endIter());
        EXPECT_TRUE(array.isEmpty());
    }
    {
        array.clear();
        array.emplaceBack(10);
        array.emplaceBack(20);
        array.emplaceBack(30);
        array.emplaceBack(40);
        array.emplaceBack(50);

        auto it = array.erase(array.firstConstIter());
        EXPECT_EQ(it->value, 20);
        EXPECT_EQ(array.size(), 4);

        it = array.erase(array.firstConstIter() + 1);
        EXPECT_EQ(it->value, 40);
        EXPECT_EQ(array.size(), 3);

        it = array.erase(array.lastConstIter());
        EXPECT_EQ(it, array.endIter());
        EXPECT_EQ(array.size(), 2);

        EXPECT_EQ(array[0].value, 20);
        EXPECT_EQ(array[1].value, 40);
    }

    {
        array.clear();
        array.emplaceBack(100);
        array.emplaceBack(200);
        array.emplaceBack(300);
        array.emplaceBack(400);

        auto const_it = array.firstConstIter() + 1;
        auto result_it = array.erase(const_it);
        EXPECT_EQ(result_it->value, 300);
        EXPECT_EQ(array.size(), 3);

        auto mut_it = array.firstIter() + 1;
        result_it = array.erase(mut_it);
        EXPECT_EQ(result_it->value, 400);
        EXPECT_EQ(array.size(), 2);

        EXPECT_EQ(array[0].value, 100);
        EXPECT_EQ(array[1].value, 400);
    }
    {
        array.clear();
        array.emplaceBack(1);
        array.emplaceBack(2);

        EXPECT_ANY_THROW(array.erase(array.endConstIter()));

        auto it = array.erase(array.firstConstIter());
        EXPECT_EQ(it->value, 2);

        EXPECT_NO_THROW(array.erase(array.firstConstIter()));
        it += 100;
        EXPECT_ANY_THROW(array.erase(it));
    }
}

TEST(TestStaticArray, TestPopBack) {
    ArrayType<int, 5> array{5, 6, 7};
    EXPECT_EQ(array.size(), 3);
    EXPECT_EQ(array.lastElement(), 7);

    array.popBack();
    EXPECT_EQ(array.size(), 2);
    EXPECT_EQ(array.lastElement(), 6);

    array.popBack();
    EXPECT_EQ(array.size(), 1);
    EXPECT_EQ(array.lastElement(), 5);

    array.popBack();
    EXPECT_EQ(array.size(), 0);
}

TEST(TestStaticArray, TestIterators) {
    constexpr std::array<int, 10> data = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    ArrayType<int, 1024> array;
    auto it = array.firstIter();
    ASSERT_TRUE(it.isOutOfRange());

    it += 4;

    ASSERT_GT(it, array.firstIter());

    for (auto i = 0; i < data.size(); ++i) {
        const auto v = data[i];
        array.pushBack(v);

        auto firstIt = array.firstIter();
        auto lastIt = array.lastIter();

        ASSERT_TRUE(!firstIt.isExpired() && !firstIt.isOutOfRange());
        ASSERT_TRUE(!lastIt.isExpired() && !lastIt.isOutOfRange());

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
            ASSERT_TRUE(!it.isExpired() && !it.isOutOfRange());
            EXPECT_EQ(*it, i++);
        }
    }
}

TEST(TestStaticArray, TestIteratorArithmetic) {
    ArrayType<int, 1024> array{{10, 20, 30, 40, 50}};
    auto it = array.firstIter();
    EXPECT_EQ(*(it + 2), 30);
    EXPECT_EQ(*(it + 3), 40);
    EXPECT_EQ(*array.lastIter(), 50);

    it += 3;
    EXPECT_EQ(*it, 40);
    it -= 2;
    EXPECT_EQ(*it, 20);
}

TEST(TestStaticArray, TestReverseIterators) {
    {
        constexpr std::array<int, 5> data = {10, 20, 30, 40, 50};
        ArrayType<int, 1024> array{data};
        auto rFirstIt = array.firstReverseIter();
        auto rLastIt = array.lastReverseIter();
        EXPECT_EQ(*rFirstIt, 50);
        EXPECT_EQ(*rLastIt, 10);

        ++rFirstIt;
        --rLastIt;
        EXPECT_EQ(*rFirstIt, 40);
        EXPECT_EQ(*rLastIt, 20);

        auto i = 5;
        for (auto it = array.firstReverseIter(); it != array.endReverseIter(); ++it, --i) {
            EXPECT_EQ(*it, i * 10);
        }

        // Check arithmetic
        {
            auto cit = array.firstReverseConstIter();
            EXPECT_EQ(*cit, int{50});

            cit += 3;
            EXPECT_EQ(*cit, int{20});

            cit -= 2;
            EXPECT_EQ(*cit, int{40});

            EXPECT_GT(cit, array.firstReverseConstIter());
            EXPECT_EQ(cit - array.firstReverseConstIter(), 1);

            cit += 100;
            EXPECT_TRUE(cit.isOutOfRange());

            EXPECT_TRUE(array.firstReverseIter() < array.lastReverseIter());
            EXPECT_TRUE(array.firstReverseIter() <= array.lastReverseIter());
            EXPECT_TRUE(array.lastReverseIter() > array.firstReverseIter());
            EXPECT_TRUE(array.lastReverseIter() >= array.firstReverseIter());

            EXPECT_TRUE(array.firstReverseConstIter() < array.lastReverseConstIter());
            EXPECT_TRUE(array.firstReverseConstIter() <= array.lastReverseConstIter());
            EXPECT_TRUE(array.lastReverseConstIter() > array.firstReverseConstIter());
            EXPECT_TRUE(array.lastReverseConstIter() >= array.firstReverseConstIter());
        }

        // Check reverse iter to usual iter
        {
            auto rIt = array.firstReverseIter();
            EXPECT_EQ(*rIt, *array.lastIter());
            ++rIt;

            EXPECT_EQ(*rIt, *(--array.lastIter()));

            rIt = array.lastReverseIter();
            EXPECT_EQ(*rIt, *array.firstIter());

            ++rIt;
            EXPECT_EQ(rIt, array.endReverseIter());
        }
        // Check insert with const reverse iter
        {
            auto rIt = array.firstReverseConstIter();
            EXPECT_EQ(*rIt, *array.lastConstIter());
            ++rIt;

            EXPECT_EQ(*rIt, *(--array.lastConstIter()));

            rIt = array.lastReverseConstIter();
            EXPECT_EQ(*rIt, *array.firstConstIter());

            ++rIt;
            EXPECT_EQ(rIt, array.endReverseConstIter());
        }
    }
    {
        ArrayType<int, 1024> array;
        auto rFirstIt = array.firstReverseIter();
        auto rLastIt = array.lastReverseIter();
        EXPECT_TRUE(rFirstIt == array.endReverseIter() && rLastIt == array.endReverseIter());
    }
    // Check invalidation of reverse iterator
    {
        ArrayType<int, 1024> array;
        auto it = array.firstReverseConstIter();
        array.insert(it.reverse(), int{10});
        EXPECT_TRUE(it.isExpired());
        EXPECT_ANY_THROW(*it);
        EXPECT_ANY_THROW(array.insert(it.reverse(), int{20}));
    }
}
