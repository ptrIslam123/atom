#include <gtest/gtest.h>

#include "include/automaton/fsm.h"
#include "include/automaton/fsm_ops.h"
#include "include/automaton/default_fsms.h"

#include <array>
#include <string_view>
#include <span>

using namespace atom::automaton;

// TEST(TestFSM, TestCombinateNumberAndWordAnalyzer) {
//     const State
//         integerNumber,
//         floatingNumber,
//         hexNumber,
//         binaryNumber;

//     const State word;
//     auto fsm = Combine(
//         MakeNumberFSMAnalyzer(integerNumber, floatingNumber, hexNumber, binaryNumber),
//         MakeWordFSMAnalyzer(word)
//     );
//     {
//         const std::array inputString{'1', '2', '3', '4', '5', '\0'};
//         auto result = fsm.analyze(inputString);
//         ASSERT_TRUE(result.has_value());
//         const auto& [finalState, value] = *result;
//         EXPECT_EQ(finalState, integerNumber);
//         const std::string_view expectedValue{inputString.data()};
//         const std::string_view realValue{value.data(), value.size()};
//         EXPECT_EQ(expectedValue, realValue);
//     }
//     {
//         const std::array inputString{'-', '1', '2', '.', '4', '5', '\0'};
//         auto result = fsm.analyze(inputString);
//         ASSERT_TRUE(result.has_value());
//         const auto& [finalState, value] = *result;
//         EXPECT_EQ(finalState, floatingNumber);
//         const std::string_view expectedValue{inputString.data()};
//         const std::string_view realValue{value.data(), value.size()};
//         EXPECT_EQ(expectedValue, realValue);
//     }
//     {
//         const std::array inputString{'0', 'x', '1', 'A', 'F', '\0'};
//         auto result = fsm.analyze(inputString);
//         ASSERT_TRUE(result.has_value());
//         const auto& [finalState, value] = *result;
//         EXPECT_EQ(finalState, hexNumber);
//         const std::string_view expectedValue{inputString.data()};
//         const std::string_view realValue{value.data(), value.size()};
//         EXPECT_EQ(expectedValue, realValue);
//     }
//     {
//         const std::array inputString{'0', 'b', '1', '0', '1', '\0'};
//         auto result = fsm.analyze(inputString);
//         ASSERT_TRUE(result.has_value());
//         const auto& [finalState, value] = *result;
//         EXPECT_EQ(finalState, binaryNumber);
//         const std::string_view expectedValue{inputString.data()};
//         const std::string_view realValue{value.data(), value.size()};
//         EXPECT_EQ(expectedValue, realValue);
//     }
//     {
//         const std::array inputString{'1', '2', '.', 'j', '\0'};
//         auto result = fsm.analyze(inputString);
//         EXPECT_FALSE(result.has_value());
//     }
//     {
//         const std::array inputString{'0' ,'b', '1', '0', '8', '\0'};
//         auto result = fsm.analyze(inputString);
//         EXPECT_FALSE(result.has_value());
//     }

//     {
//         const std::array inputString{'H','e', 'l', 'l', 'o', '\0'};
//         auto result = fsm.analyze(inputString);
//         EXPECT_TRUE(result.has_value());
//         const auto& [finalState, value] = *result;
//         EXPECT_EQ(finalState, word);
//         const std::string_view expectedValue{inputString.data()};
//         const std::string_view realValue{value.data(), value.size()};
//         EXPECT_EQ(expectedValue, realValue);
//     }
//     {
//         const std::array inputString{'M', 'y', '\0'};
//         auto result = fsm.analyze(inputString);
//         EXPECT_TRUE(result.has_value());
//         const auto& [finalState, value] = *result;
//         EXPECT_EQ(finalState, word);
//         const std::string_view expectedValue{inputString.data()};
//         const std::string_view realValue{value.data(), value.size()};
//         EXPECT_EQ(expectedValue, realValue);
//     }
//     {
//         const std::array inputString{'C', 'a', 't', '\0'};
//         auto result = fsm.analyze(inputString);
//         EXPECT_TRUE(result.has_value());
//         const auto& [finalState, value] = *result;
//         EXPECT_EQ(finalState, word);
//         const std::string_view expectedValue{inputString.data()};
//         const std::string_view realValue{value.data(), value.size()};
//         EXPECT_EQ(expectedValue, realValue);
//     }
//     {
//         const std::array inputString{'C', 'a', 't', ' ', '\0'};
//         auto result = fsm.analyze(inputString);
//         EXPECT_TRUE(result.has_value());
//         const auto& [finalState, value] = *result;
//         EXPECT_EQ(finalState, word);
//         const std::string_view expectedValue{"Cat"};
//         const std::string_view realValue{value.data(), value.size()};
//         EXPECT_EQ(expectedValue, realValue);
//     }
//     {
//         const std::array inputString{'V', 'a', 'r', '1', '\0'};
//         auto result = fsm.analyze(inputString);
//         EXPECT_TRUE(result.has_value());
//         const auto& [finalState, value] = *result;
//         EXPECT_EQ(finalState, word);
//         const std::string_view expectedValue{inputString.data()};
//         const std::string_view realValue{value.data(), value.size()};
//         EXPECT_EQ(expectedValue, realValue);
//     }
//     {
//         const std::array inputString{'1', '2', '5', '\0'};
//         auto result = fsm.analyze(inputString);
//         EXPECT_TRUE(result.has_value());
//         const auto& [finalState, value] = *result;
//         EXPECT_EQ(finalState, integerNumber);
//         const std::string_view expectedValue{inputString.data()};
//         const std::string_view realValue{value.data(), value.size()};
//         EXPECT_EQ(expectedValue, realValue);
//     }
// }

TEST (TestFSM, TestSpecialSymbols) {
    // '+', '-', '/', '=', '*', '?', '!', ...
    const State plus, minus, mult, division;
    const std::array<const std::pair<char8_t, State>, 4> data = {
        std::make_pair('+', plus),
        std::make_pair('-', minus),
        std::make_pair('*', mult),
        std::make_pair('/', division),
        // ...
    };
    auto fsm = MakeSpecialSymbolFSMAnalyzer(data);
    {
        const std::array<char8_t, 2> inputString{'+', '\0'};
        const auto& [finalState, value] = fsm.analyze(inputString);
        EXPECT_EQ(finalState, plus);
        const std::u8string_view expectedValue{inputString.data(), inputString.size() - 1};
        const std::u8string_view realValue{value.data(), value.size()};
        EXPECT_EQ(expectedValue, realValue);
    }
    {
        const std::array<char8_t, 2> inputString{'-', '\0'};
        const auto& [finalState, value] = fsm.analyze(inputString);
        EXPECT_EQ(finalState, minus);
        const std::u8string_view expectedValue{inputString.data(), inputString.size() - 1};
        const std::u8string_view realValue{value.data(), value.size()};
        EXPECT_EQ(expectedValue, realValue);
    }
    {
        const std::array<char8_t, 2> inputString{'^', '\0'};
        auto result = fsm.analyze(inputString);
        EXPECT_EQ(result.first, State::INVALID());
    }
}

TEST(TestFSM, TestNumberFSM) {
    const State
        integerNumber,
        floatingNumber,
        hexNumber,
        binaryNumber;
    auto fsm = MakeNumberFSMAnalyzer(integerNumber, floatingNumber, hexNumber, binaryNumber);
    {
        const std::array<char8_t, 6> inputString{'1', '2', '3', '4', '5', '\0'};
        const auto& [finalState, value] = fsm.analyze(inputString);
        EXPECT_EQ(finalState, integerNumber);
        const std::u8string_view expectedValue{inputString.data(), inputString.size() - 1};
        const std::u8string_view realValue{value.data(), value.size()};
        EXPECT_EQ(expectedValue, realValue);
    }
    {
        const std::array<char8_t, 7> inputString{'-', '1', '2', '.', '4', '5', '\0'};
        const auto& [finalState, value] = fsm.analyze(inputString);
        EXPECT_EQ(finalState, floatingNumber);
        const std::u8string_view expectedValue{inputString.data(), inputString.size() - 1};
        const std::u8string_view realValue{value.data(), value.size()};
        EXPECT_EQ(expectedValue, realValue);
    }
    {
        const std::array<char8_t, 6> inputString{'0', 'x', '1', 'A', 'F', '\0'};
        const auto& [finalState, value] = fsm.analyze(inputString);
        EXPECT_EQ(finalState, hexNumber);
        const std::u8string_view expectedValue{inputString.data(), inputString.size() - 1};
        const std::u8string_view realValue{value.data(), value.size()};
        EXPECT_EQ(expectedValue, realValue);
    }
    {
        const std::array<char8_t, 6> inputString{'0', 'b', '1', '0', '1', '\0'};
        const auto& [finalState, value] = fsm.analyze(inputString);
        EXPECT_EQ(finalState, binaryNumber);
        const std::u8string_view expectedValue{inputString.data(), inputString.size() - 1};
        const std::u8string_view realValue{value.data(), value.size()};
        EXPECT_EQ(expectedValue, realValue);
    }
    {
        const std::array<char8_t, 2> inputString{'f', '\0'};
        auto result = fsm.analyze(inputString);
        EXPECT_EQ(result.first, State::INVALID());
    }
    {
        const std::array<char8_t, 5> inputString{'1', '2', '.', 'j', '\0'};
        auto result = fsm.analyze(inputString);
        EXPECT_EQ(result.first, State::INVALID());
    }
    {
        const std::array<char8_t, 6> inputString{'0' ,'b', '1', '0', '8', '\0'};
        auto result = fsm.analyze(inputString);
        EXPECT_EQ(result.first, State::INVALID());
    }
}

TEST(TestFSM, TestWordFMS) {
    const State word;
    auto fsm = MakeWordFSMAnalyzer(word);
    {
        const std::array<char8_t, 6> inputString{'H','e', 'l', 'l', 'o', '\0'};
        const auto& [finalState, value] = fsm.analyze(inputString);
        EXPECT_EQ(finalState, word);
        const std::u8string_view expectedValue{inputString.data(), inputString.size() - 1};
        const std::u8string_view realValue{value.data(), value.size()};
        EXPECT_EQ(expectedValue, realValue);
    }
    {
        const std::array<char8_t, 3> inputString{'M', 'y', '\0'};
        const auto& [finalState, value] = fsm.analyze(inputString);
        EXPECT_EQ(finalState, word);
        const std::u8string_view expectedValue{inputString.data(), inputString.size() - 1};
        const std::u8string_view realValue{value.data(), value.size()};
        EXPECT_EQ(expectedValue, realValue);
    }
    {
        const std::array<char8_t, 4> inputString{'C', 'a', 't', '\0'};
        const auto& [finalState, value] = fsm.analyze(inputString);
        EXPECT_EQ(finalState, word);
        const std::u8string_view expectedValue{inputString.data(), inputString.size() - 1};
        const std::u8string_view realValue{value.data(), value.size()};
        EXPECT_EQ(expectedValue, realValue);
    }
    {
        const std::array<char8_t, 5> inputString{'C', 'a', 't', ' ', '\0'};
        const auto& [finalState, value] = fsm.analyze(inputString);
        EXPECT_EQ(finalState, word);
        const std::u8string_view expectedValue{inputString.data(), inputString.size() - 2};
        const std::u8string_view realValue{value.data(), value.size()};
        EXPECT_EQ(expectedValue, realValue);
    }
    {
        const std::array<char8_t, 5> inputString{'V', 'a', 'r', '1', '\0'};
        const auto& [finalState, value] = fsm.analyze(inputString);
        EXPECT_EQ(finalState, word);
        const std::u8string_view expectedValue{inputString.data(), inputString.size() - 1};
        const std::u8string_view realValue{value.data(), value.size()};
        EXPECT_EQ(expectedValue, realValue);
    }
    {
        const std::array<char8_t, 4> inputString{'1', '2', '5', '\0'};
        auto result = fsm.analyze(inputString);
        EXPECT_EQ(result.first, State::INVALID());
    }
    {
        const std::array<char8_t, 4> inputString{'1', '2', '5', '\0'};
        auto result = fsm.analyze(inputString);
        EXPECT_EQ(result.first, State::INVALID());
    }
}

TEST(TestFSM, TestTempExpr) {
    const State tempExpr;
    auto fsm = MakeTemporalExpressionsFSMAnalyzer(tempExpr);
    {
        constexpr std::array<char8_t, 12> inputString{'e', 'a', 'r', 'l', 'y', '-', '2', '0', '0', '0', 's', '\0'};
        const auto& [finalState, value] = fsm.analyze(inputString);
        EXPECT_EQ(finalState, tempExpr);
        const std::u8string_view expectedValue{inputString.data(), inputString.size() - 1};
        const std::u8string_view realValue{value.data(), value.size()};
        EXPECT_EQ(expectedValue, realValue);
    }
    {
        constexpr std::array<char8_t, 11> inputString{'l', 'a', 't', 'e', '-', '1', '9', '9', '0', 's', '\0'};
        const auto& [finalState, value] = fsm.analyze(inputString);
        EXPECT_EQ(finalState, tempExpr);
        const std::u8string_view expectedValue{inputString.data(), inputString.size() - 1};
        const std::u8string_view realValue{value.data(), value.size()};
        EXPECT_EQ(expectedValue, realValue);
    }
    {
        constexpr std::array<char8_t, 10> inputString{'m', 'i', 'd', '-', '1', '9', '9', '0', 's', '\0'};
        const auto& [finalState, value] = fsm.analyze(inputString);
        EXPECT_EQ(finalState, tempExpr);
        const std::u8string_view expectedValue{inputString.data(), inputString.size() - 1};
        const std::u8string_view realValue{value.data(), value.size()};
        EXPECT_EQ(expectedValue, realValue);
    }
}
