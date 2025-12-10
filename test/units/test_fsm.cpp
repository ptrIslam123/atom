#include <gtest/gtest.h>

#include "include/automaton/fsm.h"
#include "include/automaton/fsm_ops.h"

#include <array>
#include <string_view>
#include <span>

using namespace atom::automaton;

namespace {

FSM MakeSpecialSymbolFSMAnalyzer(std::span<const std::pair<FSM::SymbolType/*special symbol*/, State/*final state*/>> data) {
    using SymbolType = FSM::SymbolType;
    using StatesType = FSM::StatesType;
    using TransitionTableType = FSM::TransitionTableType;
    using TransitionsType = FSM::TransitionsType;

    std::unordered_map<char/*special symbol*/, State/*non final state*/> symbolToNonFinalState;
    symbolToNonFinalState.reserve(data.size());
    StatesType states;
    states.reserve(data.size() + 1);
    states.emplace(State::STATE0());

    StatesType finals;
    finals.reserve(data.size());

    TransitionTableType transitions;
    transitions.reserve(data.size() + 1);

    for (auto it = data.begin(); it != data.end(); ++it) {
        const char& ss = it->first;
        const State& finalState = it->second;
        const State nonFinalState;
        ASSERTION(!symbolToNonFinalState.contains(ss), BadFSM, "repetitions of special character")
        symbolToNonFinalState.emplace(ss, nonFinalState);

        states.emplace(nonFinalState);
        finals.emplace(finalState);

        transitions.emplace(nonFinalState, TransitionsType{[finalState](const SymbolType* s, const std::size_t n) -> State {
                                return finalState;
                            }});
    }

    const auto [_, success] = transitions.emplace(State::STATE0(), TransitionsType{[symbolToNonFinalState = std::move(symbolToNonFinalState)](const SymbolType* s, const std::size_t n) -> State {
                                                      ASSERTION(n == 1, BadFSM, "Expected symbol length == 1 for this analyzer")
                                                      auto it = symbolToNonFinalState.find(*s);
                                                      if (it != symbolToNonFinalState.cend()) {
                                                          return it->second;
                                                      } else {
                                                          return State::INVALID();
                                                      }
                                                  }});
    assert(success);
    return FSM{std::move(states), std::move(finals), std::move(transitions)};
}

FSM MakeNumberFSMAnalyzer(
    const State& integerNumber,
    const State& floatingNumber,
    const State& hexNumber,
    const State& binaryNumber
) {
    using SymbolType = FSM::SymbolType;
    using StatesType = FSM::StatesType;
    using TransitionTableType = FSM::TransitionTableType;
    using TransitionsType = FSM::TransitionsType;

    const State minus, zero, hexDigit, binaryDigit, integerDigit, floatingDigit,
        dot,    // 0.012 | 0,012
        x,      // 0x1FFCB
        b;      // 0b101010

    StatesType states = {State::STATE0(), minus, zero, hexDigit, binaryDigit, integerDigit, floatingDigit, dot, x, b};
    StatesType finals = {integerNumber, floatingNumber, hexNumber, binaryNumber};

    TransitionTableType transitions = {
        {State::STATE0(), TransitionsType{[zero, minus, integerDigit](const SymbolType* s, const std::size_t n) -> State {
             ASSERTION(s && n == 1, BadFSM, "Expected symbol length for number analyzer 1 byte")
             if (*s == '0') {
                 return zero;
             } else if (*s == '-') {
                 return minus;
             } else if (std::isdigit(*s)) {
                 return integerDigit;
             } else {
                 return State::INVALID();
             }
         }}},
        {minus, TransitionsType{[integerDigit](const SymbolType* s, const std::size_t n) -> State {
             ASSERTION(s && n == 1, BadFSM, "Expected symbol length for number analyzer 1 byte")
             if (std::isdigit(*s)) {
                 return integerDigit;
             } else {
                 return State::INVALID();
             }
         }}},
        {integerDigit, TransitionsType{[integerNumber, integerDigit, dot](const SymbolType* s, const std::size_t n) -> State {
             ASSERTION(s && n == 1, BadFSM, "Expected symbol length for number analyzer 1 byte")
             if (std::isdigit(*s)) {
                 return integerDigit;
             } else if (*s == '.') {
                 return dot;
             } else {
                 return integerNumber;
             }
         }}},
        {dot, TransitionsType{[floatingDigit](const SymbolType* s, const std::size_t n) -> State {
             ASSERTION(s && n == 1, BadFSM, "Expected symbol length for number analyzer 1 byte")
             if (std::isdigit(*s)) {
                 return floatingDigit;
             } else {
                 return State::INVALID();
             }
         }}},
        {floatingDigit, TransitionsType{[floatingDigit, floatingNumber](const SymbolType* s, const std::size_t n) -> State {
             ASSERTION(s && n == 1, BadFSM, "Expected symbol length for number analyzer 1 byte")
             if (std::isdigit(*s)) {
                 return floatingDigit;
             } else {
                 return floatingNumber;
             }
         }}},
        {zero, TransitionsType{[binaryDigit, hexDigit, dot](const SymbolType* s, const std::size_t n) -> State {
             ASSERTION(s && n == 1, BadFSM, "Expected symbol length for number analyzer 1 byte")
             if (*s == 'x') {
                 return hexDigit;
             } else if (*s == 'b') {
                 return binaryDigit;
             } else if (*s == '.') {
                 return dot;
             } else {
                 return State::INVALID();
             }
         }}},
        {hexDigit, TransitionsType{[hexDigit, hexNumber](const SymbolType* s, const std::size_t n) -> State {
             ASSERTION(s && n == 1, BadFSM, "Expected symbol length for number analyzer 1 byte")
             static const std::unordered_set<SymbolType> HEX_SYMBOLS = {'A', 'B', 'C', 'D', 'E', 'F', 'a', 'b', 'c', 'd', 'e', 'f',};
             if (std::isdigit(*s) || HEX_SYMBOLS.contains(*s)) {
                 return hexDigit;
             } else {
                 return hexNumber;
             }
         }}},
        {binaryDigit, TransitionsType{[binaryDigit, binaryNumber](const SymbolType* s, const std::size_t n) -> State {
             ASSERTION(s && n == 1, BadFSM, "Expected symbol length for number analyzer 1 byte")
             if (*s == '1' || *s == '0') {
                 return binaryDigit;
             } else if (std::isdigit(*s)) {
                 return State::INVALID();
             } else {
                 return binaryNumber;
             }
         }}}
    };
    return FSM{std::move(states), std::move(finals), std::move(transitions)};
}

FSM MakeWordFSMAnalyzer(const State& word) {
    using SymbolType = FSM::SymbolType;
    using StatesType = FSM::StatesType;
    using TransitionTableType = FSM::TransitionTableType;
    using TransitionsType = FSM::TransitionsType;

    const State
        symbol,   // a,b,c, ...
        dash,       // -
        x,            // `
        underscore,
        symbolAfterSpecialSymbols;

    StatesType states = {symbol, dash, x, underscore, symbolAfterSpecialSymbols};
    StatesType finals = {word};
    TransitionTableType transitions = {
        {State::STATE0(), TransitionsType{[symbol](const SymbolType* s, const std::size_t n) -> State {
             if (n == 1) {
                 return (std::isalpha(*s) ? symbol : State::INVALID());
             } else {
                 assert(false); // todo
             }
         }}},
        {symbol, TransitionsType{[symbol, word, dash, x, underscore](const SymbolType* s, const std::size_t n) -> State {
             if (n == 1) {
                 if (std::isalpha(*s) || std::isdigit(*s)) {
                     return symbol;
                 } else if (*s == '-') {
                     return dash;
                 } else if (*s == '`') {
                     return x;
                 } else if (*s == '_') {
                     return underscore;
                 } else {
                     return word;
                 }
             } else {
                 assert(false); // todo
             }
         }}},
        {symbolAfterSpecialSymbols, TransitionsType{[symbolAfterSpecialSymbols, word](const SymbolType* s, const std::size_t n) -> State {
             if (n == 1) {
                 if (std::isalpha(*s) || std::isdigit(*s)) {
                     return symbolAfterSpecialSymbols;
                 } else {
                     return word;
                 }
             } else {
                 assert(false); // todo
             }
         }}},
        {dash, TransitionsType{[symbolAfterSpecialSymbols](const SymbolType* s, const std::size_t n) -> State {
             if (n == 1) {
                 if (std::isalpha(*s)) {
                     return symbolAfterSpecialSymbols;
                 } else {
                     return State::INVALID();
                 }
             } else {
                 assert(false); // todo
             }
         }}},
        {x, TransitionsType{[symbolAfterSpecialSymbols](const SymbolType* s, const std::size_t n) -> State {
             if (n == 1) {
                 return (std::isalpha(*s) ? symbolAfterSpecialSymbols : State::INVALID());
             } else {
                 assert(false);
             }
         }}},
        {underscore, TransitionsType{[symbolAfterSpecialSymbols](const SymbolType* s, const std::size_t n) -> State {
             if (n == 1) {
                 if (std::isalpha(*s) || std::isdigit(*s)) {
                     return symbolAfterSpecialSymbols;
                 } else {
                     return State::INVALID();
                 }
             } else {
                 assert(false); // todo
             }
         }}}
    };
    return FSM{std::move(states), std::move(finals), std::move(transitions)};
}

FSM MakeTemporalExpressionsFSMAnalyzer(const State& tempExpr) {
    /*
    * early-2000s — начало 2000-х (примерно 2000–2003).
    * mid-1990s — середина 1990-х (примерно 1994–1996).
    * late-19th century — конец XIX века (примерно 1880–1899).
    */
    using SymbolType = FSM::SymbolType;
    using StatesType = FSM::StatesType;
    using TransitionTableType = FSM::TransitionTableType;
    using TransitionsType = FSM::TransitionsType;

    const State early, mid, late, separator, time, final;
    StatesType states = {mid, early, late, final};
    StatesType finals = {tempExpr};
    TransitionTableType transitions = {
        {State::STATE0(), TransitionsType{[early, mid, late](const SymbolType* s, const std::size_t n) -> State {
             ASSERTION(s && n == 1, BadFSM,  "Unexpected symbol length")
             switch (*s) {
             case 'e': return early;
             case 'm': return mid;
             case 'l': return late;
             default: return State::INVALID();
             }
         }}},
        {early, TransitionsType{[early, separator](const SymbolType* s, const std::size_t n) -> State {
             ASSERTION(s && n == 1, BadFSM,  "Unexpected symbol length")
             switch (*s) {
             case 'a':
             case 'r':
             case 'l': return early;
             case 'y': return separator;
             default: return State::INVALID();
             }
         }}},
        {mid, TransitionsType{[mid, separator](const SymbolType* s, const std::size_t n) -> State {
             ASSERTION(s && n == 1, BadFSM,  "Unexpected symbol length")
             switch (*s) {
             case 'i': return mid;
             case 'd': return separator;
             default: return State::INVALID();
             }
         }}},
        {late, TransitionsType{[late, separator](const SymbolType* s, const std::size_t n) -> State {
             ASSERTION(s && n == 1, BadFSM,  "Unexpected symbol length")
             switch (*s) {
             case 'a':
             case 't': return late;
             case 'e': return separator;
             default: return State::INVALID();
             }
         }}},
        {separator, TransitionsType{[time](const SymbolType* s, const std::size_t n) -> State {
             ASSERTION(s && n == 1, BadFSM,  "Unexpected symbol length")
             return (*s == '-' ? time : State::INVALID());
         }}},
        {time, TransitionsType{[time, final](const SymbolType* s, const std::size_t n) -> State {
             ASSERTION(s && n == 1, BadFSM,  "Unexpected symbol length")
             if (std::isdigit(*s)) {
                 return time;
             } else if (*s == 's') {
                 return final;
             } else if (*s == 't') {
                 return final;
             } else if (*s == 'h') {
                 return final;
             } else {
                 return State::INVALID();
             }
         }}},
        {final, TransitionsType{[tempExpr](const SymbolType* s, const std::size_t n) -> State {
             return tempExpr;
         }}}
    };
    return FSM{std::move(states), std::move(finals), std::move(transitions)};
}

} //! namespace

TEST(TestFSM, TestCombinateNumberAndWordAnalyzer) {
    const State
        integerNumber,
        floatingNumber,
        hexNumber,
        binaryNumber;

    const State word;
    auto fsm = Combine(
        MakeNumberFSMAnalyzer(integerNumber, floatingNumber, hexNumber, binaryNumber),
        MakeWordFSMAnalyzer(word)
    );
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
        const std::array<char8_t, 5> inputString{'1', '2', '.', 'j', '\0'};
        auto result = fsm.analyze(inputString);
        EXPECT_EQ(result.first, State::INVALID());
    }
    {
        const std::array<char8_t, 6> inputString{'0' ,'b', '1', '0', '8', '\0'};
        auto result = fsm.analyze(inputString);
        EXPECT_EQ(result.first, State::INVALID());
    }

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
        const auto& [finalState, value] = fsm.analyze(inputString);
        EXPECT_EQ(finalState, integerNumber);
        const std::u8string_view expectedValue{inputString.data(), inputString.size() - 1};
        const std::u8string_view realValue{value.data(), value.size()};
        EXPECT_EQ(expectedValue, realValue);
    }
}

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
