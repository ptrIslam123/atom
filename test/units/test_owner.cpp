#include <gtest/gtest.h>

#include "include/concurrency/owner.h"

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

TEST(TestOwner, TestConstrcutor) {
    int i = 10;
    std::string str{"foo test string"};
    concurrency::Owner<Foo> foo(i, str);

    auto rFoo = foo.getMutableRef();
    rFoo.accessImmutable([i, str](const Foo& foo) {
        EXPECT_EQ(foo.m_iValue, i);
        EXPECT_EQ(foo.m_sValue, str);
    });
}

TEST(TestOwner, TestModification) {
    int i = 10;
    std::string str{"foo test string"};
    concurrency::Owner<Foo> foo(i, str);

    auto rFoo = foo.getMutableRef();
    rFoo.accessMutable([i, str](Foo& foo) {
        ++foo.m_iValue;

        EXPECT_EQ(foo.m_iValue, i + 1);
        EXPECT_EQ(foo.m_sValue, str);
    });
}

TEST(TestOwner, TestRefCopy) {
    int i = 10;
    std::string str{"foo test string"};
    concurrency::Owner<Foo> foo(i, str);

    auto rfoo1 = foo.getMutableRef();
    auto rfoo2 = foo.getMutableRef();

    rfoo1.accessImmutable([i, str](concurrency::Ref<Foo>::ImmutableValueRefType foo) {
        EXPECT_EQ(foo.m_iValue, i);
        EXPECT_EQ(foo.m_sValue, str);
    });

    rfoo2.accessImmutable([i, str](concurrency::Ref<Foo>::ImmutableValueRefType foo) {
        EXPECT_EQ(foo.m_iValue, i);
        EXPECT_EQ(foo.m_sValue, str);
    });
}
