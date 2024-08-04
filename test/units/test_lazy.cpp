#include <gtest/gtest.h>

#include "include/utils/lazy.h"

#include <string>
#include <string_view>

namespace {

struct Foo final {
    explicit Foo(const int iValue, const std::string_view sValue):
    m_iValue(iValue),
    m_sValue(sValue)
    {}

    Foo(const Foo& ) = delete;
    Foo& operator=(const Foo& ) noexcept = delete;

    Foo(Foo&& other) {
        this->operator=(std::move(other));
    }

    Foo& operator=(Foo&& other) {
        m_iValue = std::move(other.m_iValue);
        m_sValue = std::move(other.m_sValue);
        return *this;
    }

    int m_iValue;
    std::string m_sValue;
};

} //! namespace

using namespace atom;

TEST(LazyTest, TestEmptyLazy) {
    utils::LazyConstructed<Foo> foo;
    EXPECT_FALSE(foo.wasConstructed());
}

TEST(LazyTest, TestAccessToEmptyLazy) {
    utils::LazyConstructed<Foo> foo;
    EXPECT_THROW(foo.get(), utils::BadLazyConstructed<Foo>);
}

TEST(LazyTest, TestConstructedLazy) {
    constexpr auto str = "Test foo string!";
    constexpr auto i = 10;
    utils::LazyConstructed<Foo> foo;

    foo.construct(i, str);
    EXPECT_TRUE(foo.wasConstructed());
    EXPECT_EQ(foo->m_iValue, i);
    EXPECT_EQ(foo->m_sValue, str);
}

TEST(LazyTest, TestConstructedLazyTwise) {
    constexpr auto str = "Test foo string!";
    constexpr auto i = 10;
    utils::LazyConstructed<Foo> foo;

    foo.construct(i, str);
    EXPECT_TRUE(foo.wasConstructed());

    EXPECT_THROW(foo.construct(i, str), utils::BadLazyConstructed<Foo>);
}

TEST(TestLazy, TestConversationConstructedLazyToOriginalObject) {
    constexpr auto str = "Test foo string!";
    constexpr auto i = 10;
    utils::LazyConstructed<Foo> foo;

    foo.construct(i, str);
    Foo fooOriginal = foo.intoValue();
    EXPECT_EQ(fooOriginal.m_iValue, i);
    EXPECT_EQ(fooOriginal.m_sValue, str);
}
