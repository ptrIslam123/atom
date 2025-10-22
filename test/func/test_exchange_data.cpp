#include <gtest/gtest.h>

#include <atomic>
#include <thread>
#include <vector>
#include <algorithm>

#include "exchange_data.h"

TEST(ExchangeDataTest, TestEq) {
    ExchangeData original{ExchangeData::Gen()};
    ExchangeData copy{original};
    EXPECT_EQ(copy, original);
}

TEST(ExchangeDataTest, SerializationDeserialization) {
    for (auto i = 0 ; i < 10; ++i) {
        std::array<std::byte, ExchangeData::SIZE> buffer;
        ExchangeData original{ExchangeData::Gen()};

        ExchangeData::Serialize(original, buffer);

        ExchangeData restored;
        ExchangeData::Deserialize(buffer, restored);

        EXPECT_TRUE(ExchangeData::CheckSum(restored));
        EXPECT_EQ(original, restored);
    }
}
