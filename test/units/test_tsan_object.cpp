#include <gtest/gtest.h>

#include "include/utils/tsan_object.h"

#include <string>
#include <string_view>

using namespace atom::utils::tsan;

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

TEST(SyncTest, TestImmutableAccess) {
    Object<Foo> foo(10, "Some text");
    static_assert(std::is_same_v<Object<Foo>::ImmutableValueRefType, const Foo&>);

    foo.accessImmutable([](Object<Foo>::ImmutableValueRefType foo) {
        EXPECT_EQ(foo.m_iValue, 10);
        EXPECT_EQ(foo.m_sValue, "Some text");
    });
}

TEST(SyncTest, TestMutableAccess) {
    MutableObject<Foo> foo(10, "Some text");
    static_assert(std::is_same_v<MutableObject<Foo>::ImmutableValueRefType, const Foo&>);
    static_assert(std::is_same_v<MutableObject<Foo>::MutableValueRefType, Foo&>);

    foo.accessMutable([](MutableObject<Foo>::MutableValueRefType foo) {
        EXPECT_EQ(foo.m_iValue, 10);
        EXPECT_EQ(foo.m_sValue, "Some text");

        const auto oldIValue = foo.m_iValue;
        foo.m_iValue = oldIValue * 2;
        foo.m_sValue.clear();

        EXPECT_EQ(foo.m_iValue, oldIValue * 2);
        EXPECT_TRUE(foo.m_sValue.empty());
    });
}
