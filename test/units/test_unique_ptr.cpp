#include <gtest/gtest.h>

#include "include/memory/smart_pointers/unique_ptr.h"

#include <string>
#include <string_view>

using namespace atom::memory;

namespace {

struct TestAllocator final {
    std::byte* allocate(std::size_t size) { return allocator.allocate(size); }
    void deallocate(std::byte* start) { allocator.deallocate(start); }

    template<typename T, typename ... Arg>
    void construct(T* object, Arg&& ... arg) { allocator.construct(object, std::forward(arg) ...); }

    template<typename T>
    void destruct(T* object) { allocator.destruct(object); }

    atom::memory::allocator::DefaultAllocator allocator;
};

} //! namespace

TEST(TestUniquePtr, Test) {
    constexpr std::string_view testStr = "Hello world from my unique ptr!";
    auto p = UniquePtr<std::string>::Make(testStr);
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(*p, std::string{testStr});
    EXPECT_FALSE(p->empty());
    p->clear(); // clear string
    EXPECT_EQ(*p, std::string{});

    p.clear();
    EXPECT_EQ(p, nullptr);
}

TEST(TestUniquePtr, TestImmutability) {
    constexpr std::string_view testStr = "Hello world from my unique ptr!";
    auto p = UniquePtr<const std::string>::Make(testStr);
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(*p, testStr);

    // *p = "New String!"; /*compile error - Ok!*/
    EXPECT_EQ(p->size(), testStr.size());
    // p->clear(); /*compile error - Ok!*/
}

TEST(TestUniquePtr, TestMoveOperator) {
    constexpr std::string_view testStr = "Hello world from my unique ptr!";
    auto p1 = UniquePtr<std::string>::Make(testStr);
    ASSERT_NE(p1, nullptr);
    EXPECT_EQ(*p1, testStr);

    UniquePtr<const std::string> p2{std::move(p1)};
    ASSERT_EQ(p1, nullptr);
    ASSERT_NE(p2, nullptr);
    EXPECT_EQ(*p2, testStr);
}

TEST(TestUniquePtr, TestClear) {
    static auto counter = 0;
    struct Foo {
        Foo() { ++counter; }
        ~Foo() { --counter; }
    };
    {
        auto ptr = UniquePtr<Foo>::Make();
        EXPECT_EQ(counter, 1);
    }
    EXPECT_EQ(counter, 0);
}

TEST(TestUniquePtr, TestCastomAllocator) {
    atom::utils::Owner<TestAllocator> testAllocator;
    static auto counter = 0;
    struct Foo {
        Foo() { ++counter; }
        ~Foo() { --counter; }
    };
    {
        auto ptr = UniquePtr<Foo, TestAllocator>::Make(testAllocator.borrowMutable());
        EXPECT_EQ(counter, 1);
    }
    EXPECT_EQ(counter, 0);
}

TEST(TestUniquePtr, TestWithPolymorphicType) {
    struct Base {
        virtual int foo() { return m_base; }
        virtual ~Base() { m_base = 0; };

        int m_base{0};
    };
    struct Derived : Base {
        ~Derived() { m_derived = 0; }
        virtual int foo() override { return m_derived; }
        int m_derived{1};
    };

    UniquePtr<Base> base;
    {
        auto derived = UniquePtr<Derived>::Make();
        EXPECT_EQ(derived->foo(), 1);
        base = std::move(derived);
    }
    EXPECT_EQ(base->foo(), 1);
}


TEST(TestUniquePtr, TestDynamicCast) {
    struct Base {
        virtual int foo() { return m_base; }
        virtual ~Base() { m_base = 0; };

        int m_base{0};
    };
    struct Derived : Base {
        ~Derived() { m_derived = 0; }
        virtual int foo() override { return m_derived; }
        int m_derived{1};
    };

    UniquePtr<Base> base{UniquePtr<Derived>::Make()};
    UniquePtr<Derived> derived = DynamicCast<Derived>(std::move(base));
    EXPECT_EQ(derived->foo(), 1);
}

TEST(TestUniquePtr, TestStaticCast) {
    struct Base {
        virtual ~Base() = default;
    };
    struct Derived : Base {};
    UniquePtr<Base> ptr = StaticCast<Base>(UniquePtr<Derived>::Make());
    EXPECT_NE(ptr, nullptr);
}

TEST(TestUniquePtr, TestConstCast) {
    UniquePtr<const int> cptr{UniquePtr<const int>::Make(int{100})};
    EXPECT_EQ(*cptr, 100);

    UniquePtr<int> ptr{ConstCast<int>(std::move(cptr))};
    *ptr += 1;
    EXPECT_EQ(*ptr, 101);
}
