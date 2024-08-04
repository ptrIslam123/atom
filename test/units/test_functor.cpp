#include <gtest/gtest.h>

#include "include/utils/functor.h"

#include <string>
#include <string_view>
#include <type_traits>

namespace {

struct Foo final {
    explicit Foo(const int iValue, const std::string_view sValue):
    m_iValue(iValue),
    m_sValue(sValue)
    {}

    int m_iValue;
    std::string m_sValue;
};

} //! namespace

using namespace atom;

TEST(TestFunctor, TestDynamicalFunctorId) {
    const auto i = 20;
    const auto str = "test foo string";
    Foo foo{i, str};
    EXPECT_EQ(foo.m_iValue, i);

    utils::dynamical::Functor<int()> f([&foo](void) {
        return foo.m_iValue;
    });
    EXPECT_EQ(f(), i);
}

TEST(TestFunctor, TestDynamicalFunctorSumator) {
    utils::dynamical::Functor<int()> f([](void) { return 10 + 100; });
    EXPECT_EQ(f(), 10 + 100);
}

TEST(TestFunctor, TestStaticalFunctorId) {
    const auto i = 20;
    const auto str = "test foo string";
    Foo foo{i, str};
    EXPECT_EQ(foo.m_iValue, i);

    utils::statical::Functor<sizeof(&foo),int()> f([&foo](void) {
        return foo.m_iValue;
    });
    EXPECT_EQ(f(), i);
}

TEST(TestFunctor, TestStaticalFunctorSumator) {
    utils::statical::Functor<1, int()> f([](void) { return 10 + 100; });
    EXPECT_EQ(f(), 10 + 100);
}
