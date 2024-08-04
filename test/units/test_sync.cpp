#include <gtest/gtest.h>

#include "include/concurrency/sync.h"

#include <string>
#include <string_view>

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

using namespace ciengine;

TEST(SyncTest, TestImmutableAccess) {
    concurrency::Sync<Foo> foo(10, "Some text");
    static_assert(std::is_same_v<concurrency::Sync<Foo>::ImmutableValueRefType, const Foo&>);

    foo.accessImmutable([](concurrency::Sync<Foo>::ImmutableValueRefType foo) {
        EXPECT_EQ(foo.m_iValue, 10);
        EXPECT_EQ(foo.m_sValue, "Some text");
    });
}

TEST(SyncTest, TestMutableAccess) {
    concurrency::MutableSync<Foo> foo(10, "Some text");
    static_assert(std::is_same_v<concurrency::MutableSync<Foo>::ImmutableValueRefType, const Foo&>);
    static_assert(std::is_same_v<concurrency::MutableSync<Foo>::MutableValueRefType, Foo&>);

    foo.accessMutable([](concurrency::MutableSync<Foo>::MutableValueRefType foo) {
        EXPECT_EQ(foo.m_iValue, 10);
        EXPECT_EQ(foo.m_sValue, "Some text");

        const auto oldIValue = foo.m_iValue;
        foo.m_iValue = oldIValue * 2;
        foo.m_sValue.clear();

        EXPECT_EQ(foo.m_iValue, oldIValue * 2);
        EXPECT_TRUE(foo.m_sValue.empty());
    });
}
