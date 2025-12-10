#include <gtest/gtest.h>

#include "include/containers/fixed/stack.h"

using namespace atom::containers::fixed;

template<class T>
using StackType = Stack<T, 64>;

TEST(TestFixedStack, TestConstructors) {
    {
        Stack<int, 24> stack{};
        EXPECT_EQ(stack.size(), 0);
        EXPECT_EQ(stack.capacity(), 24);
    }
    {
        StackType<int> stack(int{100}, StackType<int>::SizeType{3});
        EXPECT_EQ(stack.size(), 3);

        while(!stack.isEmpty()) {
            EXPECT_EQ(stack.top(), int{100});
            stack.pop();
        }
        EXPECT_EQ(stack.size(), 0);
    }
    {
        StackType<int> stack{{int{10}, int{20}, int{30}}};
        EXPECT_EQ(stack.size(), 3);
        EXPECT_EQ(stack.top(), int{30});

        stack.pop();
        EXPECT_EQ(stack.size(), 2);
        EXPECT_EQ(stack.top(), int{20});

        stack.pop();
        EXPECT_EQ(stack.size(), 1);
        EXPECT_EQ(stack.top(), int{10});

        stack.pop();
        EXPECT_TRUE(stack.isEmpty());
    }
}

TEST(TestFixedStack, TestCopyConstructor) {
    struct Foo {
        Foo(int _value): value(_value) {}
        int value;
    };
    StackType<Foo> s1{{Foo{1}, Foo{2}, Foo{3}}};
    EXPECT_EQ(s1.size(), 3);

    auto s2{s1};
    EXPECT_EQ(s1.size(), s2.size());
    EXPECT_EQ(s1.capacity(), s2.capacity());

    EXPECT_TRUE(s1.top().value == s2.top().value && s1.top().value == int{3});
    s1.pop(); s2.pop();
    EXPECT_TRUE(s1.top().value == s2.top().value && s1.top().value == int{2});
    s1.pop(); s2.pop();
    EXPECT_TRUE(s1.top().value == s2.top().value && s1.top().value == int{1});
    s1.pop(); s2.pop();
    EXPECT_TRUE(s1.size() == 0 && s2.size() == 0);
}

TEST(TestFixedStack, TestCopyOperator) {
    struct Foo {
        Foo(int _value): value(_value) {}
        int value;
    };
    StackType<Foo> s1{{Foo{1}, Foo{2}, Foo{3}}};
    EXPECT_EQ(s1.size(), 3);

    auto s2 = s1;
    EXPECT_EQ(s1.size(), s2.size());
    EXPECT_EQ(s1.capacity(), s2.capacity());

    EXPECT_TRUE(s1.top().value == s2.top().value && s1.top().value == int{3});
    s1.pop(); s2.pop();
    EXPECT_TRUE(s1.top().value == s2.top().value && s1.top().value == int{2});
    s1.pop(); s2.pop();
    EXPECT_TRUE(s1.top().value == s2.top().value && s1.top().value == int{1});
    s1.pop(); s2.pop();
    EXPECT_TRUE(s1.size() == 0 && s2.size() == 0);
}

TEST(TestFixedStack, TestMoveConstructor) {
    struct Foo {
        Foo(const Foo&) = delete;
        Foo& operator=(const Foo&) = delete;

        Foo(int _value): value(_value) {}
        Foo(Foo&& other):value(other.value) { other.value = -1; }
        Foo& operator=(Foo&& other) {
            value = other.value;
            other.value = -1;
            return *this;
        }

        int value;
    };
    StackType<Foo> s1;
    s1.emplace(int{1}); s1.emplace(int{2}); s1.emplace(int{3});
    EXPECT_EQ(s1.size(), 3);

    auto s2{std::move(s1)};
    EXPECT_EQ(s2.size(), 3);
    EXPECT_EQ(s1.size(), 0);
    EXPECT_EQ(s1.capacity(), s2.capacity());

    EXPECT_EQ(s2.top().value, int{3});
    s2.pop();

    EXPECT_EQ(s2.top().value, int{2});
    s2.pop();

    EXPECT_EQ(s2.top().value, int{1});
    s2.pop();
}

TEST(TestFixedStack, TestDestructor) {
    int counter{0};
    struct Foo {
        Foo(int& counter): m_counter(&counter) { ++(*m_counter); }
        ~Foo() { if (m_counter) --(*m_counter); }
        int* m_counter{nullptr};
    };
    EXPECT_EQ(counter, 0);
    {
        StackType<Foo> stack;
        for (auto i = 0; i < stack.capacity(); ++i) {
            EXPECT_EQ(stack.size(), i);
            EXPECT_NO_THROW(stack.emplace(counter));
            EXPECT_EQ(stack.size(), i + 1);
        }
        EXPECT_EQ(counter, stack.capacity());
        EXPECT_TRUE(stack.isFull());
        EXPECT_ANY_THROW(stack.emplace(counter));
    }
    EXPECT_EQ(counter, 0);
}

TEST(TestFixedStack, TestClear) {
    StackType<int> stack;
    for (auto i = 0; i < stack.capacity(); ++i) {
        EXPECT_EQ(stack.size(), i);
        EXPECT_NO_THROW(stack.push(int{i}));
        EXPECT_EQ(stack.size(), i + 1);
        EXPECT_EQ(stack.top(), i);
    }

    EXPECT_TRUE(stack.isFull());
    EXPECT_ANY_THROW(stack.push(int{1000}));

    stack.clear();
    EXPECT_TRUE(stack.isEmpty());
    EXPECT_EQ(stack.size(), 0);
}

TEST(TestFixedStack, TestPushAndPop) {
    StackType<int> stack;
    EXPECT_TRUE(stack.isEmpty());

    stack.push(int{10});
    EXPECT_EQ(stack.size(), 1);
    EXPECT_EQ(stack.top(), int{10});

    stack.push(int{20});
    EXPECT_EQ(stack.size(), 2);
    EXPECT_EQ(stack.top(), int{20});

    stack.push(int{30});
    EXPECT_EQ(stack.size(), 3);
    EXPECT_EQ(stack.top(), int{30});

    stack.pop();
    EXPECT_EQ(stack.size(), 2);
    EXPECT_EQ(stack.top(), int{20});

    stack.pop();
    EXPECT_EQ(stack.size(), 1);
    EXPECT_EQ(stack.top(), int{10});

    stack.pop();
    EXPECT_EQ(stack.size(), 0);
}

TEST(TestFixedStack, TestPushAndPopWithNonTrivialType) {
    struct Foo {
        Foo(int _value): value(_value) {}
        int value;
    };

    StackType<Foo> stack;
    EXPECT_TRUE(stack.isEmpty());

    stack.emplace(int{10});
    EXPECT_EQ(stack.size(), 1);
    EXPECT_EQ(stack.top().value, int{10});

    stack.emplace(int{20});
    EXPECT_EQ(stack.size(), 2);
    EXPECT_EQ(stack.top().value, int{20});

    stack.emplace(int{30});
    EXPECT_EQ(stack.size(), 3);
    EXPECT_EQ(stack.top().value, int{30});

    stack.pop();
    EXPECT_EQ(stack.size(), 2);
    EXPECT_EQ(stack.top().value, int{20});

    stack.pop();
    EXPECT_EQ(stack.size(), 1);
    EXPECT_EQ(stack.top().value, int{10});

    stack.pop();
    EXPECT_EQ(stack.size(), 0);
}

TEST(TestFixedStack, TestEmptyStack) {
    StackType<int> stack;
    EXPECT_ANY_THROW((void)stack.top());
    EXPECT_ANY_THROW(stack.pop());
}

TEST(TestFixedStack, TestStackSize) {
    Stack<int, 3> stack;
    EXPECT_EQ(stack.capacity(), 3);

    for (auto i = 0; i < stack.capacity(); ++i) {
        EXPECT_NO_THROW(stack.push(int{i}));
    }
    EXPECT_ANY_THROW(stack.push(int{1000}));
}
