#include <gtest/gtest.h>

#include "include/utils/owner.h"

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


using namespace atom::utils::bc;

TEST(TestOwner, TestOwner) {
    Owner<Foo> foo{0, "test"};
    auto ref = foo.borrowMutable();
    ref.accessMutable([](Foo& _foo) {
        EXPECT_EQ(_foo.m_iValue, 0);
        EXPECT_EQ(_foo.m_sValue, "test");

        _foo.m_iValue = 10;
        EXPECT_EQ(_foo.m_iValue, 10);
    });

    // foo.getValue(); // Compile error - OK!

    foo.borrowMutable();
    auto cpRef{ref};
    auto cref = foo.borrowImmutable();

    // ref = cref; // Compile error - OK!
}

TEST(TestOwner, TestRef) {
    Owner<int> owner{int{10}};

    auto ref = owner.borrowMutable();
    auto cref = owner.borrowImmutable();
    EXPECT_EQ(ref.getValue(), cref.getValue());
}
