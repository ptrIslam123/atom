#include <gtest/gtest.h>

#include "include/memory/smart_pointers/shared_ptr.h"

namespace {

struct TestAllocator final {
    std::byte* allocate(std::size_t size) { return allocator.allocate(size); }
    void reallocate(std::byte*& start, std::size_t size) { allocator.reallocate(start, size); }
    void deallocate(std::byte* start) { allocator.deallocate(start); }

    template<typename T, typename ... Arg>
    void construct(T* object, Arg&& ... arg) { allocator.construct(object, std::forward(arg) ...); }

    template<typename T>
    void destruct(T* object) { allocator.destruct(object); }

    atom::memory::allocator::DefaultAllocator allocator;
};

} //! namespace

template<typename T, typename A = atom::memory::allocator::DefaultAllocator>
using SharedPtr = atom::memory::NonAtomicSharedPtr<T, A>;

template<typename T, typename A = atom::memory::allocator::DefaultAllocator>
using WeakPtr = atom::memory::NonAtomicWeakPtr<T, A>;

TEST(TestSharedPtr, TestConstructors) {
    {
        SharedPtr<int> ptr;
        EXPECT_EQ(ptr, nullptr);
        EXPECT_EQ(ptr.strongRefCount(), 0);
        EXPECT_EQ(ptr.weakRefCount(), 0);
    }
    {
        auto ptr1 = SharedPtr<int>::Make(100);
        EXPECT_NE(ptr1, nullptr);
        EXPECT_EQ(ptr1.strongRefCount(), 1);
        EXPECT_EQ(ptr1.weakRefCount(), 0);

        SharedPtr<int> ptr2{ptr1};
        EXPECT_NE(ptr2, nullptr);
        EXPECT_EQ(ptr2.strongRefCount(), 2);
        EXPECT_EQ(ptr2.weakRefCount(), 0);
        EXPECT_EQ(ptr1, ptr2);
        EXPECT_EQ(*ptr1, *ptr2);
    }
    {
        auto ptr1 = SharedPtr<int>::Make(100);
        EXPECT_NE(ptr1, nullptr);
        EXPECT_EQ(ptr1.strongRefCount(), 1);
        EXPECT_EQ(ptr1.weakRefCount(), 0);

        SharedPtr<int> ptr2{ptr1};
        EXPECT_NE(ptr2, nullptr);
        EXPECT_EQ(ptr2.strongRefCount(), 2);
        EXPECT_EQ(ptr2.weakRefCount(), 0);

        SharedPtr<int> ptr3{std::move(ptr2)};
        EXPECT_NE(ptr3, nullptr);
        EXPECT_EQ(ptr2, nullptr);
        EXPECT_EQ(ptr3.strongRefCount(), 2);
        EXPECT_EQ(ptr3.weakRefCount(), 0);
    }
}

TEST(TestSharedPtr, TestOperators) {
    {
        auto ptr1 = SharedPtr<int>::Make(100);
        EXPECT_NE(ptr1, nullptr);
        EXPECT_EQ(ptr1.strongRefCount(), 1);
        EXPECT_EQ(ptr1.weakRefCount(), 0);

        SharedPtr<int> ptr2 = ptr1;
        EXPECT_NE(ptr2, nullptr);
        EXPECT_EQ(ptr2.strongRefCount(), 2);
        EXPECT_EQ(ptr2.weakRefCount(), 0);
        EXPECT_EQ(ptr1, ptr2);
        EXPECT_EQ(*ptr1, *ptr2);
    }
    {
        auto ptr1 = SharedPtr<int>::Make(100);
        EXPECT_NE(ptr1, nullptr);
        EXPECT_EQ(ptr1.strongRefCount(), 1);
        EXPECT_EQ(ptr1.weakRefCount(), 0);

        SharedPtr<int> ptr2 = ptr1;
        EXPECT_NE(ptr2, nullptr);
        EXPECT_EQ(ptr2.strongRefCount(), 2);
        EXPECT_EQ(ptr2.weakRefCount(), 0);

        SharedPtr<int> ptr3 = std::move(ptr2);
        EXPECT_NE(ptr3, nullptr);
        EXPECT_EQ(ptr2, nullptr);
        EXPECT_EQ(ptr3.strongRefCount(), 2);
        EXPECT_EQ(ptr3.weakRefCount(), 0);
    }
}

TEST(TestSharedPtr, TestClear) {
    static int counter = 0;
    struct Foo {
        Foo() { ++counter; }
        ~Foo() { --counter; }
    };

    {
        SharedPtr<Foo> ptr{};
        EXPECT_EQ(ptr, nullptr);

        ptr = SharedPtr<Foo>::Make();
        EXPECT_NE(ptr, nullptr);
        EXPECT_EQ(counter, 1);
        ptr.clear();
    }
    EXPECT_EQ(counter, 0);

    {
        auto ptr = SharedPtr<Foo>::Make();
        {
            EXPECT_EQ(counter, 1);
            auto copy{ptr};
            EXPECT_EQ(counter, 1);
        }
        EXPECT_EQ(counter, 1);
    }
    EXPECT_EQ(counter, 0);

    {
        auto ptr = SharedPtr<Foo>::Make();
        EXPECT_NE(ptr, nullptr);

        {
            std::vector<WeakPtr<Foo>> weaks;
            for (auto i = 0; i < 10; ++i) {
                weaks.emplace_back(ptr);
            }
            EXPECT_EQ(ptr.strongRefCount(), 1);
            EXPECT_EQ(ptr.weakRefCount(), 10);
            EXPECT_EQ(counter, 1);
        }
        EXPECT_EQ(ptr.weakRefCount(), 0);
        EXPECT_EQ(counter, 1);
    }
    EXPECT_EQ(counter, 0);
}

TEST(TestSharedPtr, TestWithPolymorphicType) {
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

    SharedPtr<Base> base;
    {
        SharedPtr<Derived> derived = SharedPtr<Derived>::Make();
        EXPECT_EQ(derived->foo(), 1);
        base = std::move(derived);
        EXPECT_EQ(base->foo(), 1);
    }
    EXPECT_EQ(base->foo(), 1);
}

TEST(TestWeakPtr, TestConstrcutors) {
    {
        WeakPtr<int> ptr{SharedPtr<int>{}};
        EXPECT_EQ(ptr.weakRefCount(), 0);
        EXPECT_TRUE(ptr.isExpired());
    }
    {
        auto ptr1 = SharedPtr<int>::Make(10);
        WeakPtr<int> ptr2{ptr1};
        EXPECT_FALSE(ptr2.isExpired());
    }
}

TEST(TestSharedPtr, TestStaticCasts) {
    struct Base {
        virtual ~Base() {}
        virtual int foo() { return 0; }
    };
    struct Derived : Base {
        virtual int foo() { return 1; }
    };

    auto derived = SharedPtr<Derived>::Make();
    SharedPtr<Base> base = StaticCast<Base>(derived);
    EXPECT_EQ(base.strongRefCount(), 2);
    EXPECT_EQ(base->foo(), 1);
}

TEST(TestSharedPtr, TestDynamicAndStaticCasts) {
    struct Base {
        virtual ~Base() {}
        virtual int foo() { return 0; }
    };
    struct Derived : Base {
        virtual int foo() { return 1; }
    };

    auto derived = SharedPtr<Derived>::Make();
    auto base = StaticCast<Base>(derived);
    EXPECT_EQ(base.strongRefCount(), 2);

    auto back = DynamicCast<Derived>(base);
    EXPECT_NE(back, nullptr);
}

TEST(TestSharedPtr, TestReinterpretCast) {
    auto ptr = SharedPtr<int>::Make(42);
    auto charPtr = ReinterpretCast<char>(ptr);
    EXPECT_NE(charPtr, nullptr);
}

TEST(TestSharedPtr, TestCustomAllocator) {
    atom::utils::Owner<TestAllocator> testAllocator;
    static auto counter = 0;
    struct Foo {
        Foo() { ++counter; }
        ~Foo() { --counter; }
    };
    {
        auto ptr = SharedPtr<Foo, TestAllocator>::Make(testAllocator.borrowMutable());
        EXPECT_EQ(counter, 1);
    }
    EXPECT_EQ(counter, 0);
}
