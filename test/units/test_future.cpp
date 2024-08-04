#include <gtest/gtest.h>

#include "include/concurrency/async/future.h"
#include "include/utils/lazy.h"

#include <thread>
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

using namespace atom;
using namespace atom::concurrency;

//TEST(FutureTest, TestAsyncConputations) {
//    constexpr auto str = "Test foo string!";
//    constexpr auto i = 10;

//    utils::LazyConstructed<async::Future<Foo>> f;
//    utils::LazyConstructed<async::Promise<Foo>> p;
//    async::Bind(f, p);

//    EXPECT_TRUE(f);
//    EXPECT_TRUE(p);

//    std::thread([p = std::move(p.intoValue())]() mutable {
//        // do some work
//        std::this_thread::sleep_for(std::chrono::microseconds(100));
//        // ...
//        if (!p.wasCanceled()) {
//            p.setResult(Foo{ i, str });
//        }
//    }).detach();

//    switch (f->waitFor(std::chrono::microseconds(50))) {
//        using Status = async::Future<Foo>::Status;
//        case Status::Ready: {
//            const auto result = f->getResult();
//            EXPECT_TRUE(result.has_value());
//            EXPECT_EQ(result->m_iValue, i);
//            EXPECT_EQ(result->m_sValue, str);
//            break;
//        }
//        case Status::TimeoutExpired: {
//            f->waitForCancellation();
//            break;
//        }
//        case Status::Canceled: {
//            break;
//        }
//    }
//}

