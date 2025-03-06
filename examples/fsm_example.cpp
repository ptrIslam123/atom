#include "include/automaton/fsm.h"
#include "include/automaton/fsm_ops.h"
#include "include/automaton/default_fsms.h"

#include <iostream>
#include <span>
#include <array>
#include <vector>
#include <string>
#include <cctype>
#include <cassert>

using namespace atom::automaton;
using StatesType = FSM<char>::StatesType;
using TransitionTableType = FSM<char>::TransitionTableType;
using TransitionsType = FSM<char>::TransitionsType;

constexpr char SEOF = '$';
static inline bool Eq(const std::span<const char>& f, const std::span<const char>& s) {
    return std::string_view{f.begin(), f.end()} == std::string_view{s.begin(), s.end()};
}


void test1() {
    static const State
        Minus, Zero,
        HexDigit, BinaryDigit, IntegerDigit, FloatDigit,
        Dot, X, B,
        HexNumber, BinaryNumber, IntegerNumber, FloatNumber;

    StatesType states = {STATE0(), Minus, Zero, HexDigit, BinaryDigit, IntegerDigit, FloatDigit, Dot, X, B};
    StatesType finals = {HexNumber, BinaryNumber, IntegerNumber, FloatNumber};

    TransitionTableType transitions = {
        {STATE0(), TransitionsType{[](char s) -> std::optional<State> {
             if (s == '0') {
                 return std::make_optional(Zero);
             } else if (s == '-') {
                 return std::make_optional(Minus);
             } else if (std::isdigit(s)) {
                 return std::make_optional(IntegerDigit);
             } else {
                 return std::nullopt;
             }
         }}},
        {Minus, TransitionsType{[](char s) -> std::optional<State> {
             if (std::isdigit(s)) {
                 return std::make_optional(IntegerDigit);
             } else {
                 return std::nullopt;
             }
         }}},
        {IntegerDigit, TransitionsType{[](char s) -> std::optional<State> {
             if (std::isdigit(s)) {
                 return std::make_optional(IntegerDigit);
             } else if (s == '.') {
                 return std::make_optional(Dot);
             } else if (s == SEOF) {
                 return std::make_optional(IntegerNumber);
             } else {
                 return std::nullopt;
             }
         }}},
        {Dot, TransitionsType{[](char s) -> std::optional<State> {
             if (std::isdigit(s)) {
                 return std::make_optional(FloatDigit);
             } else {
                 return std::nullopt;
             }
         }}},
        {FloatDigit, TransitionsType{[](char s) -> std::optional<State> {
             if (std::isdigit(s)) {
                 return std::make_optional(FloatDigit);
             } else if (s == SEOF) {
                 return std::make_optional(FloatNumber);
             } else {
                 return std::nullopt;
             }
         }}},
        {Zero, TransitionsType{[](char s) -> std::optional<State> {
             if (s == 'x') {
                 return std::make_optional(HexDigit);
             } else if (s == 'b') {
                 return std::make_optional(BinaryDigit);
             } else {
                 return std::nullopt;
             }
         }}},
        {HexDigit, TransitionsType{[](char s) -> std::optional<State> {
             static const std::unordered_set<char> HEX_SYMBOLS = {'A', 'B', 'C', 'D', 'E', 'F', 'a', 'b', 'c', 'd', 'e', 'f',};
             if (std::isdigit(s) || HEX_SYMBOLS.contains(s)) {
                 return std::make_optional(HexDigit);
             } else if (s == SEOF) {
                 return std::make_optional(HexNumber);
             } else {
                 return std::nullopt;
             }
         }}},
        {BinaryDigit, TransitionsType{[](char s) -> std::optional<State> {
             if (s == '1' || s == '0') {
                 return std::make_optional(BinaryDigit);
             } else if (s == SEOF) {
                 return std::make_optional(BinaryNumber);
             } else {
                 return std::nullopt;
             }
         }}}
    };
    // "+/-1223", "+/-12.54", "0x123FFC", "0b01010110"
    FSM<char> fsm{std::move(states), std::move(finals), std::move(transitions)};
    {
        const std::array inputString{'0', 'x', '1', 'A', 'F', SEOF};
        auto result = fsm.analyze(inputString);
        assert(result.has_value());
    }
}

void test2() {
    const State plus, minus;
    std::array data = {std::make_pair('+', plus), std::make_pair('-', minus)};
    FSM<char> fsm = MakeSpecialSymbolFSMAnalyzer(data);
    {
        const std::array inputString{'+', ' '};
        auto result = fsm.analyze(inputString);
        assert(result.has_value());
    }
    {
        const std::array inputString{'-', ' '};
        auto result = fsm.analyze(inputString);
        assert(result.has_value());
    }
    {
        const std::array inputString{'^', ' '};
        auto result = fsm.analyze(inputString);
        assert(!result.has_value());
    }
}

void test3() {
    const State
        integerNumber,
        floatingNumber,
        hexNumber,
        binaryNumber;
    FSM<char> numberFsm = MakeNumberFSMAnalyzer(integerNumber, floatingNumber, hexNumber, binaryNumber);

    const State word;
    FSM<char> wordFsm = MakeWordFSMAnalyzer(word);

    FSM<char> fsm = Combine(numberFsm, wordFsm);
    {
        const std::array inputString{'H', 'i', ' '};
        auto result = fsm.analyze(inputString);
        assert(result.has_value());
    }
    {
        const std::array inputString{'1', '2', '3', '4', '5', ' '};
        auto result = fsm.analyze(inputString);
        assert(result.has_value());
    }
}

int main() {
    test3();
    return 0;
}
