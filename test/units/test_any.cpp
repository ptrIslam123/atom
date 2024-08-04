#include <gtest/gtest.h>

#include "include/utils/any.h"

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

TEST(StaticAnyTest, TestCostructAndUseStaticAny) {
    int i = 10;
    std::string str{"test foo string"};
    dynamical::AnyObject any(Foo{i, str});

    auto result = any.getRef<Foo>();
    EXPECT_TRUE(result.has_value());

    concurrency::Ref<Foo> foo = result.value();
    foo.accessImmutable([i, str](const Foo& foo) {
        EXPECT_EQ(foo.m_iValue, i);
        EXPECT_EQ(foo.m_sValue, str);
    });

    foo.accessMutable([i](Foo& foo) {
        ++foo.m_iValue;
        EXPECT_EQ(foo.m_iValue, i + 1);
    });
}

TEST(StaticAnyTest, TestAttemptToGetNonValidTypeDynamically) {
    int i = 10;
    std::string str{"test foo string"};
    dynamical::AnyObject any(Foo{i, str});

    auto foo = any.getRef<std::string>();
    EXPECT_FALSE(foo.has_value());
}

TEST(DynamicAnyTest, TestAttemptToGetNonValidTypeStatically) {
    int i = 10;
    std::string str{"test foo string"};
    statical::AnyObject<1024> any(Foo{i, str});

    auto foo = any.getRef<std::string>();
    EXPECT_FALSE(foo.has_value());
}

TEST(DynamicalAnyTest, TestConstructingAndDestructing) {
    int counter = 0;

    struct Baz {
        Baz(int& counter): counter(counter) {++counter;}
        ~Baz() { --counter; }
        int& counter;
    };

    {
        statical::AnyObject<1024> baz(std::make_unique<Baz>(counter));
        EXPECT_EQ(counter, 1);
    }

    EXPECT_EQ(counter, 0);
}

TEST(StaticAnyTest, TestConstructingAndDestructing) {
    int counter = 0;

    struct Baz {
        Baz(int& counter): counter(counter) {++counter;}
        ~Baz() { --counter; }
        int& counter;
    };

    {
        dynamical::AnyObject baz(std::make_unique<Baz>(counter));
        EXPECT_EQ(counter, 1);
    }

    EXPECT_EQ(counter, 0);
}
