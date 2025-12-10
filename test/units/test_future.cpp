#include <gtest/gtest.h>

#include "include/concurrency/async/future.h"

#include <string>
#include <thread>
#include <chrono>

using namespace atom::async;

//TEST(TestStaticFuture, Test) {
//    const std::string message{"My test mesage"};
//    Future<std::string> f{};
//    Promise<std::string> p{};
//    Bind(f, p);

//    std::thread t([&p, &message]() {
//        std::this_thread::sleep_for(std::chrono::milliseconds(200));
//        p.set(message);
//    });

//    f.wait();
//    EXPECT_TRUE(f.isReady());
//    EXPECT_EQ(f.get(), message);
//}
