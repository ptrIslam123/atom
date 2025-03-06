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
    Object<const Foo> foo(10, "Some text");
    foo.accessImmutable([](const Foo& foo) {
        EXPECT_EQ(foo.m_iValue, 10);
        EXPECT_EQ(foo.m_sValue, "Some text");
    });

    // foo.accessMutable([](Foo& foo) {}); // compile error!
}

TEST(SyncTest, TestMutableAccess) {
    Object<Foo> foo{10, "Some text"};
    foo.accessMutable([](Object<Foo>::MutableRefType foo) {
        EXPECT_EQ(foo.m_iValue, 10);
        EXPECT_EQ(foo.m_sValue, "Some text");

        const auto oldIValue = foo.m_iValue;
        foo.m_iValue = oldIValue * 2;
        foo.m_sValue.clear();

        EXPECT_EQ(foo.m_iValue, oldIValue * 2);
        EXPECT_TRUE(foo.m_sValue.empty());
    });
}
