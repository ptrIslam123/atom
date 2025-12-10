#include <gtest/gtest.h>

#include "include/containers/dynamic/list.h"
#include "include/iterator/iterator_traits.h"
#include "include/iterator/iterator.h"

using namespace atom::containers::dynamic;
using namespace atom::iter;

namespace {

struct TestAllocator {
    TestAllocator() = default;
    std::byte* allocate(std::size_t size) {
        auto ptr = reinterpret_cast<std::byte*>(malloc(size));
        assert(ptr);
        usememory += size;
        return ptr;
    }

    void deallocate(std::byte* start, std::size_t size) {
        free(static_cast<void*>(start));
        usememory -= size;
    }

    template<typename T, typename ... Arg>
    void construct(T* object, Arg&& ... arg) {
        assert(object);
        new(object) T(std::forward<Arg>(arg) ...);
    }

    template<typename T>
    void destruct(T* object) {
        if (object) {
            object->~T();
        }
    }
    std::size_t usememory{0};
};

} //! namespace

TEST(TestList, TestIteratorTraits) {
    using Iterator = typename List<int>::Iterator;
    using ReverseIterator = typename List<int>::ReverseIterator;
    EXPECT_TRUE(atom::iter::isBidirectionalIterator<Iterator>);
    EXPECT_TRUE(atom::iter::isBidirectionalIterator<ReverseIterator>);

    List<int> list{1, 2, 3, 4, 5, 6, 7, 8};
    EXPECT_EQ(list.size(), 8);
    EXPECT_EQ(atom::iter::Distance(list.firstConstIter(), list.endConstIter()), list.size());

    auto it = list.firstIter();
    atom::iter::Advance(it, 3);
    EXPECT_EQ(*it, int{4});

    it = atom::iter::Next(it);
    EXPECT_EQ(*it, int{5});

    it = atom::iter::Prev(it, 4);
    EXPECT_EQ(*it, int{1});
}

TEST(TestList, TestWithInvalidIterators) {
    List<int> list{1, 2, 3};

    EXPECT_ANY_THROW(*list.endIter());
    EXPECT_ANY_THROW(*list.endConstIter());
    EXPECT_ANY_THROW(*list.endReverseIter());
    EXPECT_ANY_THROW(*list.endReverseConstIter());

    auto it = list.endConstIter();
    EXPECT_NO_THROW(++it);
    // deref it is UB(impl details)
}

TEST(TestList, TestReverseIteratorBasicOperations) {
    List<int> list;
    list.pushBack(1);
    list.pushBack(2);
    list.pushBack(3);

    // Test rbegin/rend
    auto rit = list.firstReverseIter();
    EXPECT_NE(rit, list.endReverseIter());
    EXPECT_EQ(*rit, 3);

    // Test prefix increment
    ++rit;
    EXPECT_EQ(*rit, 2);

    // Test postfix increment
    auto old_rit = rit++;
    EXPECT_EQ(*old_rit, 2);
    EXPECT_EQ(*rit, 1);

    // Test end of reverse traversal
    ++rit;
    EXPECT_EQ(rit, list.endReverseIter());
}

TEST(TestList, TestReverseIteratorTraversal) {
    List<int> list;
    list.pushBack(10);
    list.pushBack(20);
    list.pushBack(30);
    list.pushBack(40);

    // Reverse traversal
    auto rit = list.firstReverseIter();
    EXPECT_EQ(*rit, 40);
    ++rit;
    EXPECT_EQ(*rit, 30);
    ++rit;
    EXPECT_EQ(*rit, 20);
    ++rit;
    EXPECT_EQ(*rit, 10);
    ++rit;
    EXPECT_EQ(rit, list.endReverseIter());
}

TEST(TestList, TestReverseConstIterator) {
    List<int> list;
    list.pushBack(5);
    list.pushBack(6);
    list.pushBack(7);

    const List<int>& const_list = list;

    auto crit = const_list.firstReverseConstIter();
    EXPECT_EQ(*crit, 7);
    ++crit;
    EXPECT_EQ(*crit, 6);
    ++crit;
    EXPECT_EQ(*crit, 5);
    ++crit;
    EXPECT_EQ(crit, const_list.endReverseConstIter());
}

TEST(TestList, TestIteratorBasicOperations) {
    List<int> list;
    list.pushBack(1);
    list.pushBack(2);
    list.pushBack(3);

    // Test begin/end
    auto it = list.firstIter();
    EXPECT_NE(it, list.endIter());
    EXPECT_EQ(*it, 1);

    // Test prefix increment
    ++it;
    EXPECT_EQ(*it, 2);

    // Test postfix increment
    auto old_it = it++;
    EXPECT_EQ(*old_it, 2);
    EXPECT_EQ(*it, 3);

    // Test end iterator
    ++it;
    EXPECT_EQ(it, list.endIter());
}

TEST(TestList, TestIteratorTraversal) {
    List<int> list;
    list.pushBack(10);
    list.pushBack(20);
    list.pushBack(30);
    list.pushBack(40);

    // Forward traversal
    auto it = list.firstIter();
    EXPECT_EQ(*it, 10);
    ++it;
    EXPECT_EQ(*it, 20);
    ++it;
    EXPECT_EQ(*it, 30);
    ++it;
    EXPECT_EQ(*it, 40);
    ++it;
    EXPECT_EQ(it, list.endIter());

    // Test operator->
    it = list.firstIter();
    EXPECT_EQ(*it, 10);
    ++it;
    EXPECT_EQ(it.operator->(), &(*it));
}

TEST(TestList, TestIteratorEquality) {
    List<int> list;
    list.pushBack(1);
    list.pushBack(2);

    auto it1 = list.firstIter();
    auto it2 = list.firstIter();

    EXPECT_EQ(it1, it2);
    ++it1;
    EXPECT_NE(it1, it2);

    auto end1 = list.endIter();
    auto end2 = list.endIter();
    EXPECT_EQ(end1, end2);
}

TEST(TestList, TestIteratorOnEmptyList) {
    List<int> list;

    EXPECT_EQ(list.firstIter(), list.endIter());
    EXPECT_EQ(list.firstIter(), list.endIter());

    auto it = list.firstIter();
    auto cit = list.firstIter();
}

TEST(TestList, TestSaveContainerInvariantsWithException) {
    struct Foo {
        Foo() {
            static int counter{0};
            ++counter;
            if (counter >= 3) throw std::runtime_error{"Test excpetion!"};
            v = counter;
        }
        int v{0};
    };

    List<Foo> list;
    EXPECT_NO_THROW(list.emplaceBack());
    EXPECT_NO_THROW(list.emplaceBack());
    EXPECT_ANY_THROW(list.emplaceBack());
    EXPECT_EQ(list.size(), 2);
    auto i = 1;
    for (auto it = list.firstConstIter(); it != list.endConstIter(); ++it, ++i) {
        EXPECT_EQ(it->v, i);
    }
}

TEST(TestList, TestWithException) {
    static int counter{0};
    struct Foo {
        Foo() {
            if (counter++ >= 4) {
                throw std::runtime_error("Test exception!");
            }
        }
        ~Foo() { counter--; }
    };
    {
        counter = 0;
        List<Foo, TestAllocator> list;
        for (auto i = 0; i < 3; ++i) {
            EXPECT_NO_THROW(list.emplaceBack());
            EXPECT_TRUE(list.getAllocator().usememory > 0);
        }
        EXPECT_EQ(list.size(), 3);
        list.clear();
        EXPECT_EQ(list.size(), 0);
        EXPECT_EQ(list.getAllocator().usememory, 0);
    }
    {
        counter = 0;
        List<Foo, TestAllocator> list;
        auto i = 0;
        try {
            for (; i < 7; ++i) {
                list.emplaceBack();
                EXPECT_TRUE(list.size() > 0);
                EXPECT_TRUE(list.getAllocator().usememory > 0);
            }
            EXPECT_TRUE(false);
        } catch (...) {
            EXPECT_EQ(list.size(), 4);
            list.clear();
        }
        EXPECT_EQ(list.size(), 0);
        EXPECT_EQ(list.getAllocator().usememory, 0);
    }
}

TEST(TestList, TestMemoryLeaks) {
    static int counter{0};
    struct Foo {
        Foo() { ++counter; }
        ~Foo() { --counter; }
    };
    EXPECT_EQ(counter, 0);
    {
        List<Foo> list;
        for (auto i = 0; i < 5; ++i) {
            list.emplaceBack();
            EXPECT_EQ(counter, i + 1);
        }
    }
    EXPECT_EQ(counter, 0);
}

TEST(TestList, TestMoveOperations) {
    // Test move constructor
    {
        List<int> list1{1, 2, 3};
        List<int> list2(std::move(list1));
        EXPECT_TRUE(list1.isEmpty());
        EXPECT_EQ(list2.size(), 3);
        auto i = 1;
        for (auto it = list2.firstConstIter(); it != list2.endConstIter(); ++it, ++i) {
            EXPECT_EQ(*it, i);
        }
    }
    // Test move assignment
    {
        List<int> list1{1, 2, 3};
        List<int> list2;
        list2 = std::move(list1);
        EXPECT_TRUE(list1.isEmpty());
        EXPECT_EQ(list2.size(), 3);
        auto i = 1;
        for (auto it = list2.firstConstIter(); it != list2.endConstIter(); ++it, ++i) {
            EXPECT_EQ(*it, i);
        }
    }
}

TEST(TestList, TestCopyOperations) {
    // Test copy constructor
    {
        List<int> list1{1, 2, 3};
        List<int> list2(list1);
        EXPECT_EQ(list1.size(), 3);
        EXPECT_EQ(list2.size(), 3);

        auto it1 = list1.firstConstIter();
        auto it2 = list2.firstConstIter();
        while (it1 != list1.endConstIter() && it2 != list2.endConstIter()) {
            EXPECT_EQ(*it1, *it2);
            ++it1; ++it2;
        }
    }
    // Test copy assignment
    {
        List<int> list1{1, 2, 3};
        List<int> list2;
        list2 = list1;
        EXPECT_EQ(list1.size(), 3);
        EXPECT_EQ(list2.size(), 3);

        auto it1 = list1.firstConstIter();
        auto it2 = list2.firstConstIter();
        while (it1 != list1.endConstIter() && it2 != list2.endConstIter()) {
            EXPECT_EQ(*it1, *it2);
            ++it1; ++it2;
        }
    }
}

TEST(TestList, TestConstructors) {
    {
        List<int> list;
        EXPECT_TRUE(list.isEmpty());
        EXPECT_EQ(list.size(), 0);
    }
    {
        List<int> list{1, 2, 3, 4};
        EXPECT_EQ(list.size(), 4);
        auto i = 1;
        for (auto it = list.firstConstIter(); it != list.endConstIter(); ++it) {
            EXPECT_EQ(*it, i);
            ++i;
        }
    }
    {
        List<int> list(int{10}, List<int>::SizeType{5});
        EXPECT_EQ(list.size(), 5);
        for (auto it = list.firstConstIter(); it != list.endConstIter(); ++it) {
            EXPECT_EQ(*it, int{10});
        }
    }
}

TEST(TestList, TestDestructor) {
    static int counter{0};
    struct Foo {
        Foo() { ++counter; }
        ~Foo() { --counter; }
    };
    EXPECT_EQ(counter, 0);
    {
        List<Foo> list;
        for (auto i = 0; i < 5; ++i) {
            list.emplaceBack();
        }
        EXPECT_EQ(list.size(), 5);
        EXPECT_EQ(counter, 5);
    }
    EXPECT_EQ(counter, 0);
}

TEST(TestList, TestEraseRange) {
    // Test erase range from the begin
    {
        List<int> list;
        list.pushBack(1);
        list.pushBack(2);
        list.pushBack(3);
        list.pushBack(4);
        list.pushBack(5);

        auto firstIt = list.firstConstIter();
        auto lastIt = firstIt; ++lastIt; ++lastIt;
        EXPECT_EQ(*firstIt, int{1});
        EXPECT_EQ(*lastIt, int{3});

        auto it = list.erase(firstIt, lastIt);
        EXPECT_EQ(list.size(), 3);
        EXPECT_EQ(*it, int{3});

        it = list.firstIter();
        EXPECT_EQ(*it, int{3});
        ++it;
        EXPECT_EQ(*it, int{4});
    }
    // Test erase range from the middle
    {
        List<int> list;
        list.pushBack(1);
        list.pushBack(2);
        list.pushBack(3);
        list.pushBack(4);
        list.pushBack(5);
        list.pushBack(6);
        EXPECT_EQ(list.size(), 6);

        auto firstIt = Next(list.firstConstIter(), 1);
        auto lastIt = Next(list.firstConstIter(), 4);

        EXPECT_EQ(*firstIt, int{2});
        EXPECT_EQ(*lastIt, int{5});

        auto it = list.erase(firstIt, lastIt);
        EXPECT_EQ(list.size(), 3);
        EXPECT_EQ(*it, int{5});

        it = list.firstIter();
        EXPECT_EQ(*it, int{1});
        ++it;
        EXPECT_EQ(*it, int{5});
        ++it;
        EXPECT_EQ(*it, int{6});
        ++it;
        EXPECT_EQ(it, list.endIter());
    }
    // Test erase range from the end
    {
        List<int> list;
        list.pushBack(1);
        list.pushBack(2);
        list.pushBack(3);
        list.pushBack(4);
        list.pushBack(5);
        list.pushBack(6);

        auto firstIt = Next(list.firstConstIter(), 3);
        auto lastIt = list.endConstIter();

        EXPECT_EQ(*firstIt, int{4});

        auto it = list.erase(firstIt, lastIt);
        EXPECT_EQ(list.size(), 3);
        EXPECT_EQ(it, list.endIter());

        it = list.firstIter();
        EXPECT_EQ(*it, int{1});
        ++it;
        EXPECT_EQ(*it, int{2});
        ++it;
        EXPECT_EQ(*it, int{3});
        ++it;
        EXPECT_EQ(it, list.endIter());
    }
}

TEST(TestList, TestErase) {
    // Test erase from the begin
    {
        List<int> list;
        list.pushBack(1);
        list.pushBack(2);
        list.pushBack(3);
        EXPECT_EQ(list.size(), 3);

        auto it = list.erase(list.firstIter());
        EXPECT_EQ(list.size(), 2);
        EXPECT_EQ(*it, 2);
    }
    // Test erase from the middle
    {
        List<int> list;
        list.pushBack(1);
        list.pushBack(2);
        list.pushBack(3);
        EXPECT_EQ(list.size(), 3);

        auto it = list.firstConstIter();
        ++it;
        it = list.erase(it);
        EXPECT_EQ(list.size(), 2);
        EXPECT_EQ(*it, int{3});

        it = list.firstConstIter();
        EXPECT_EQ(*it, int{1});
        ++it;
        EXPECT_EQ(*it, int{3});
        ++it;
        EXPECT_EQ(it, list.endConstIter());
    }
    // Test erase from the end
    {
        List<int> list;
        list.pushBack(1);
        list.pushBack(2);
        list.pushBack(3);
        EXPECT_EQ(list.size(), 3);

        auto it = list.erase(list.lastConstIter());
        EXPECT_EQ(it, list.endConstIter());
        EXPECT_EQ(list.size(), 2);

        it = list.firstIter();
        EXPECT_EQ(*it, int{1});
        ++it;
        EXPECT_EQ(*it, int{2});
        ++it;
        EXPECT_EQ(it, list.endConstIter());
    }
}

TEST(TestList, TestPopOperations) {
    // Test popFront
    {
        List<int> list;
        list.pushBack(1);
        list.pushBack(2);
        list.pushBack(3);

        list.popFront();
        EXPECT_EQ(list.size(), 2);
        EXPECT_EQ(list.firstElement(), 2);
        EXPECT_EQ(list.lastElement(), 3);

        list.popFront();
        EXPECT_EQ(list.size(), 1);
        EXPECT_EQ(list.firstElement(), 3);
        EXPECT_EQ(list.lastElement(), 3);

        list.popFront();
        EXPECT_TRUE(list.isEmpty());
    }
    // Test popBack
    {
        List<int> list;
        list.pushBack(1);
        list.pushBack(2);
        list.pushBack(3);

        list.popBack();
        EXPECT_EQ(list.size(), 2);
        EXPECT_EQ(list.firstElement(), 1);
        EXPECT_EQ(list.lastElement(), 2);

        list.popBack();
        EXPECT_EQ(list.size(), 1);
        EXPECT_EQ(list.firstElement(), 1);
        EXPECT_EQ(list.lastElement(), 1);

        list.popBack();
        EXPECT_TRUE(list.isEmpty());
    }
}

TEST(TestList, TestEmplace) {
    struct Foo {
        explicit Foo(int _iv, char _bv): iv(_iv), bv(_bv) {}
        int iv{};
        char bv{};
    };
    List<Foo> list;
    for (auto i = 0; i < 5; ++i) {
        list.emplaceBack(int{i}, static_cast<char>(i));
        auto it = list.lastConstIter();
        EXPECT_EQ(it->iv, i);
        EXPECT_EQ(it->bv, static_cast<char>(i));
    }

    auto it = list.firstConstIter();
    ++it; ++it; ++it;
    it = list.emplace(it, int{100}, char{100});
    EXPECT_EQ(it->iv, int{100});
    EXPECT_EQ(it->bv, char{100});

    it = list.firstConstIter();
    EXPECT_EQ(it->iv, int{0});
    ++it;
    EXPECT_EQ(it->iv, int{1});
    ++it;
    EXPECT_EQ(it->iv, int{2});
    ++it;
    EXPECT_EQ(it->iv, int{100});
    ++it;
    EXPECT_EQ(it->iv, int{3});
    ++it;
    EXPECT_EQ(it->iv, int{4});
    ++it;
    EXPECT_EQ(it, list.endConstIter());
}

TEST(TestList, TestInsertRange) {
    {
        List<int> list{1, 2, 7, 8};
        EXPECT_EQ(list.size(), 4);

        auto it = list.firstConstIter();
        ++it; ++it;
        EXPECT_EQ(*it, int{7});
        it = list.insert(it, {3, 4, 5, 6});
        EXPECT_EQ(*it, int{7});
        EXPECT_EQ(list.size(), 8);

        it = list.firstIter();
        EXPECT_EQ(*it, int{1});
        ++it;
        EXPECT_EQ(*it, int{2});
        ++it;
        EXPECT_EQ(*it, int{3});
        ++it;
        EXPECT_EQ(*it, int{4});
        ++it;
        EXPECT_EQ(*it, int{5});
        ++it;
        EXPECT_EQ(*it, int{6});
        ++it;
        EXPECT_EQ(*it, int{7});
        ++it;
        EXPECT_EQ(*it, int{8});
        ++it;
        EXPECT_EQ(it, list.endIter());
    }
    {
        List<int> list{1, 2, 7, 8};
        EXPECT_EQ(list.size(), 4);
        {
            auto it = list.firstIter();
            EXPECT_EQ(*it, int{1});
            ++it;
            EXPECT_EQ(*it, int{2});
            ++it;
            EXPECT_EQ(*it, int{7});
            ++it;
            EXPECT_EQ(*it, int{8});
            ++it;
            EXPECT_EQ(it, list.endIter());
        }

        list.pushBack({9, 10, 11});
        {
            auto it = list.firstIter();
            EXPECT_EQ(*it, int{1});
            ++it;
            EXPECT_EQ(*it, int{2});
            ++it;
            EXPECT_EQ(*it, int{7});
            ++it;
            EXPECT_EQ(*it, int{8});
            ++it;
            EXPECT_EQ(*it, int{9});
            ++it;
            EXPECT_EQ(*it, int{10});
            ++it;
            EXPECT_EQ(*it, int{11});
            ++it;
            EXPECT_EQ(it, list.endIter());
        }
    }
}

TEST(TestList, TestPushFrontRange) {
    {
        List<int> list;
        list.pushBack(0);
        EXPECT_EQ(list.size(), 1);

        list.pushFront({int{1}, int{2}, int{3}});
        EXPECT_EQ(list.size(), 4);

        auto it = list.firstConstIter();
        EXPECT_EQ(*it, int{1});
        ++it;
        EXPECT_EQ(*it, int{2});
        ++it;
        EXPECT_EQ(*it, int{3});
        ++it;
        EXPECT_EQ(*it, int{0});
        ++it;
        EXPECT_EQ(it, list.endConstIter());
    }
    {
        List<int> list{4, 5, 6};
        EXPECT_EQ(list.size(), 3);
        auto i = 4;
        for (auto it = list.firstConstIter(); it != list.endConstIter(); ++it, ++i) {
            EXPECT_EQ(*it, i);
        }

        list.pushFront(int{111}, List<int>::SizeType{3});
        EXPECT_EQ(list.size(), 6);

        for (auto j = 0; j < 3; ++j) {
            auto it = Next(list.firstConstIter(), j);
            EXPECT_EQ(*it, int{111});
        }
    }
}

TEST(TestList, TestPushBackRange) {
    {
        List<int> list;
        EXPECT_TRUE(list.isEmpty());

        list.pushBack({int{1}, int{2}, int{3}, int{4}});
        EXPECT_EQ(list.size(), 4);

        auto it = list.firstConstIter();
        EXPECT_EQ(*it, int{1});
        ++it;
        EXPECT_EQ(*it, int{2});
        ++it;
        EXPECT_EQ(*it, int{3});
        ++it;
        EXPECT_EQ(*it, int{4});
        ++it;
        EXPECT_EQ(it, list.endConstIter());
    }
    {
        List<int> list;
        list.pushBack(0);
        list.pushBack(1);
        EXPECT_EQ(list.size(), 2);
        auto i = 0;
        for (auto it = list.firstConstIter(); it != list.endConstIter(); ++it, ++i) {
            const auto& v = *it;
            EXPECT_EQ(v, i);
        }

        list.pushBack({int{2}, int{3}, int{4}});
        EXPECT_EQ(list.size(), 5);
        i = 0;
        for (auto it = list.firstConstIter(); it != list.endConstIter(); ++it, ++i) {
            const auto& v = *it;
            EXPECT_EQ(v, i);
        }
    }
    {
        List<int> list;
        list.pushBack(0);
        list.pushBack(1);
        EXPECT_EQ(list.size(), 2);
        auto i = 0;
        for (auto it = list.firstConstIter(); it != list.endConstIter(); ++it, ++i) {
            const auto& v = *it;
            EXPECT_EQ(v, i);
        }

        list.pushBack(int{15}, List<int>::SizeType{4});
        EXPECT_EQ(list.size(), 6);

        auto it = list.firstConstIter();
        ++it; ++it;
        EXPECT_EQ(*it, int{15});
        for (; it != list.endConstIter(); ++it) {
            EXPECT_EQ(*it, int{15});
        }

        EXPECT_EQ(Distance(list.firstConstIter(), it), 6);
    }
}

TEST(TestList, TestInsert) {
    // Check insert into the end
    {
        List<int> list;
        EXPECT_TRUE(list.isEmpty());

        list.pushBack(int{1});
        EXPECT_EQ(list.size(), 1);
        EXPECT_EQ(list.lastElement(), int{1});

        list.pushBack(int{2});
        EXPECT_EQ(list.size(), 2);
        EXPECT_EQ(list.lastElement(), int{2});

        list.pushBack(int{3});
        EXPECT_EQ(list.size(), 3);
        EXPECT_EQ(list.lastElement(), int{3});
    }
    // Check insert into the begin
    {
        List<int> list;
        EXPECT_TRUE(list.isEmpty());

        list.pushFront(int{3});
        EXPECT_EQ(list.size(), 1);
        EXPECT_EQ(list.firstElement(), int{3});

        list.pushFront(int{2});
        EXPECT_EQ(list.size(), 2);
        EXPECT_EQ(list.firstElement(), int{2});

        list.pushFront(int{1});
        EXPECT_EQ(list.size(), 3);
        EXPECT_EQ(list.firstElement(), int{1});
    }
    // Check insert into the middle
    {
        List<int> list;
        list.pushBack(int{1});
        list.pushBack(int{2});
        list.pushBack(int{3});
        list.pushBack(int{4});
        EXPECT_EQ(list.size(), 4);

        auto it = list.firstIter();
        ++it; ++it;
        EXPECT_EQ(*it, int{3});
        it = list.insert(it, int{10});
        EXPECT_EQ(*it, int{10});

        it = list.firstIter();
        EXPECT_EQ(*it, int{1});
        ++it;
        EXPECT_EQ(*it, int{2});
        ++it;
        EXPECT_EQ(*it, int{10});
        ++it;
        EXPECT_EQ(*it, int{3});
        ++it;
        EXPECT_EQ(*it, int{4});

        it = list.insert(it, int{10});
        EXPECT_EQ(*it, int{10});
        EXPECT_NE(it, list.endIter());
    }
    {
        List<int> emptyList;
        auto it = emptyList.insert(emptyList.firstConstIter(), int{42});
        EXPECT_EQ(*it, 42);
        EXPECT_EQ(emptyList.size(), 1);
    }
    {
        List<int> list;
        list.pushBack(2);
        list.pushBack(3);
        list.pushBack(4);
        auto it = list.insert(list.firstConstIter(), int{1});
        EXPECT_EQ(*it, int{1});

        it = list.firstIter();
        EXPECT_EQ(*it, int{1});
        ++it;
        EXPECT_EQ(*it, int{2});
        ++it;
        EXPECT_EQ(*it, int{3});
        ++it;
        EXPECT_EQ(*it, int{4});
    }
    {
        List<int> list;
        list.pushBack(1);
        list.pushBack(2);
        list.pushBack(3);
        auto it = list.insert(list.endConstIter(), 4);
        EXPECT_EQ(*it, int{4});

        it = list.firstIter();
        EXPECT_EQ(*it, int{1});
        ++it;
        EXPECT_EQ(*it, int{2});
        ++it;
        EXPECT_EQ(*it, int{3});
        ++it;
        EXPECT_EQ(*it, int{4});
    }
}
