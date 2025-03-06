#include "include/automaton/default_fsms.h"

namespace atom::automaton {

FSM<char> MakeSpecialSymbolFSMAnalyzer(std::span<const std::pair<char/*special symbol*/, State/*final state*/>> data) {
    using StatesType = FSM<char>::StatesType;
    using TransitionTableType = FSM<char>::TransitionTableType;
    using TransitionsType = FSM<char>::TransitionsType;

    std::unordered_map<char/*special symbol*/, State/*non final state*/> symbolToNonFinalState;
    symbolToNonFinalState.reserve(data.size());
    StatesType states;
    states.reserve(data.size() + 1);
    states.emplace(STATE0());

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

        transitions.emplace(nonFinalState, TransitionsType{[finalState](char s) -> std::optional<State> {
            return std::make_optional(finalState);
        }});
    }

    const auto [_, success] = transitions.emplace(STATE0(), TransitionsType{[symbolToNonFinalState = std::move(symbolToNonFinalState)](char s) -> std::optional<State> {
        auto it = symbolToNonFinalState.find(s);
        if (it != symbolToNonFinalState.cend()) {
            return std::make_optional(it->second);
        } else {
            return std::nullopt;
        }
    }});
    assert(success);
    return FSM{std::move(states), std::move(finals), std::move(transitions)};
}

FSM<char> MakeNumberFSMAnalyzer(
    const State& integerNumber,
    const State& floatingNumber,
    const State& hexNumber,
    const State& binaryNumber
    ) {
    using StatesType = FSM<char>::StatesType;
    using TransitionTableType = FSM<char>::TransitionTableType;
    using TransitionsType = FSM<char>::TransitionsType;

    const State minus, zero, hexDigit, binaryDigit, integerDigit, floatingDigit,
        dot,    // 0.012 | 0,012
        x,      // 0x1FFCB
        b;      // 0b101010

    StatesType states = {STATE0(), minus, zero, hexDigit, binaryDigit, integerDigit, floatingDigit, dot, x, b};
    StatesType finals = {integerNumber, floatingNumber, hexNumber, binaryNumber};

    TransitionTableType transitions = {
        {STATE0(), TransitionsType{[zero, minus, integerDigit](char s) -> std::optional<State> {
             if (s == '0') {
                 return std::make_optional(zero);
             } else if (s == '-') {
                 return std::make_optional(minus);
             } else if (std::isdigit(s)) {
                 return std::make_optional(integerDigit);
             } else {
                 return std::nullopt;
             }
         }}},
        {minus, TransitionsType{[integerDigit](char s) -> std::optional<State> {
             if (std::isdigit(s)) {
                 return std::make_optional(integerDigit);
             } else {
                 return std::nullopt;
             }
         }}},
        {integerDigit, TransitionsType{[integerNumber, integerDigit, dot](char s) -> std::optional<State> {
             if (std::isdigit(s)) {
                 return std::make_optional(integerDigit);
             } else if (s == '.') {
                 return std::make_optional(dot);
             } else {
                 return std::make_optional(integerNumber);
             }
         }}},
        {dot, TransitionsType{[floatingDigit](char s) -> std::optional<State> {
             if (std::isdigit(s)) {
                 return std::make_optional(floatingDigit);
             } else {
                 return std::nullopt;
             }
         }}},
        {floatingDigit, TransitionsType{[floatingDigit, floatingNumber](char s) -> std::optional<State> {
             if (std::isdigit(s)) {
                 return std::make_optional(floatingDigit);
             } else {
                 return std::make_optional(floatingNumber);
             }
         }}},
        {zero, TransitionsType{[binaryDigit, hexDigit, dot](char s) -> std::optional<State> {
             if (s == 'x') {
                 return std::make_optional(hexDigit);
             } else if (s == 'b') {
                 return std::make_optional(binaryDigit);
             } else if (s == '.') {
                 return std::make_optional(dot);
             } else {
                 return std::nullopt;
             }
         }}},
        {hexDigit, TransitionsType{[hexDigit, hexNumber](char s) -> std::optional<State> {
             static const std::unordered_set<char> HEX_SYMBOLS = {'A', 'B', 'C', 'D', 'E', 'F', 'a', 'b', 'c', 'd', 'e', 'f',};
             if (std::isdigit(s) || HEX_SYMBOLS.contains(s)) {
                 return std::make_optional(hexDigit);
             } else {
                 return std::make_optional(hexNumber);
             }
         }}},
        {binaryDigit, TransitionsType{[binaryDigit, binaryNumber](char s) -> std::optional<State> {
             if (s == '1' || s == '0') {
                 return std::make_optional(binaryDigit);
             } else if (std::isdigit(s)) {
                 return std::nullopt;
             } else {
                 return std::make_optional(binaryNumber);
             }
         }}}
    };
    return FSM{std::move(states), std::move(finals), std::move(transitions)};
}

FSM<char> MakeWordFSMAnalyzer(const State& word) {
    using StatesType = FSM<char>::StatesType;
    using TransitionTableType = FSM<char>::TransitionTableType;
    using TransitionsType = FSM<char>::TransitionsType;

    const State
        symbol,   // a,b,c, ...
        dash,       // -
        x,            // `
        underscore,
        symbolAfterSpecialSymbols;

    StatesType states = {symbol, dash, x, underscore, symbolAfterSpecialSymbols};
    StatesType finals = {word};
    TransitionTableType transitions = {
        {STATE0(), TransitionsType{[symbol](char s) {
             return (std::isalpha(s) ? std::make_optional(symbol) : std::nullopt);
         }}},
        {symbol, TransitionsType{[symbol, word, dash, x, underscore](char s) -> std::optional<State> {
             if (std::isalpha(s) || std::isdigit(s)) {
                 return std::make_optional(symbol);
             } else if (s == '-') {
                 return std::make_optional(dash);
             } else if (s == '`') {
                 return std::make_optional(x);
             } else if (s == '_') {
                 return std::make_optional(underscore);
             } else {
                 return std::make_optional(word);
             }
         }}},
        {symbolAfterSpecialSymbols, TransitionsType{[symbolAfterSpecialSymbols, word](char s) -> std::optional<State> {
             if (std::isalpha(s) || std::isdigit(s)) {
                 return std::make_optional(symbolAfterSpecialSymbols);
             } else {
                 return std::make_optional(word);
             }
         }}},
        {dash, TransitionsType{[symbolAfterSpecialSymbols](char s) -> std::optional<State> {
             if (std::isalpha(s)) {
                 return std::make_optional(symbolAfterSpecialSymbols);
             } else {
                 return std::nullopt;
             }
         }}},
        {x, TransitionsType{[symbolAfterSpecialSymbols](char s) -> std::optional<State> {
             return (std::isalpha(s) ? std::make_optional(symbolAfterSpecialSymbols) : std::nullopt);
         }}},
        {underscore, TransitionsType{[symbolAfterSpecialSymbols](char s) -> std::optional<State> {
             if (std::isalpha(s) || std::isdigit(s)) {
                 return std::make_optional(symbolAfterSpecialSymbols);
             } else {
                 return std::nullopt;
             }
         }}}
    };
    return FSM{std::move(states), std::move(finals), std::move(transitions)};
}

FSM<char> MakeTemporalExpressionsFSMAnalyzer(const State& tempExpr) {
    /*
     * early-2000s — начало 2000-х (примерно 2000–2003).
     * mid-1990s — середина 1990-х (примерно 1994–1996).
     * late-19th century — конец XIX века (примерно 1880–1899).
     */

    using StatesType = FSM<char>::StatesType;
    using TransitionTableType = FSM<char>::TransitionTableType;
    using TransitionsType = FSM<char>::TransitionsType;

    const State early, mid, late, separator, time, final;
    StatesType states = {mid, early, late, final};
    StatesType finals = {tempExpr};
    TransitionTableType transitions = {
        {STATE0(), TransitionsType{[early, mid, late](char s) -> std::optional<State> {
             switch (s) {
             case 'e': return std::make_optional(early);
             case 'm': return std::make_optional(mid);
             case 'l': return std::make_optional(late);
             default: return std::nullopt;
             }
         }}},
        {early, TransitionsType{[early, separator](char s) -> std::optional<State> {
             switch (s) {
             case 'a':
             case 'r':
             case 'l': return std::make_optional(early);
             case 'y': return std::make_optional(separator);
             default: return std::nullopt;
             }
         }}},
        {mid, TransitionsType{[mid, separator](char s) -> std::optional<State>{
             switch (s) {
             case 'i': return std::make_optional(mid);
             case 'd': return std::make_optional(separator);
             default: return std::nullopt;
             }
         }}},
        {late, TransitionsType{[late, separator](char s) -> std::optional<State> {
             switch (s) {
             case 'a':
             case 't': return std::make_optional(late);
             case 'e': return std::make_optional(separator);
             default: return std::nullopt;
             }
         }}},
        {separator, TransitionsType{[time](char s) -> std::optional<State> {
             return (s == '-' ? std::make_optional(time) : std::nullopt);
         }}},
        {time, TransitionsType{[time, final](char s) -> std::optional<State> {
             if (std::isdigit(s)) {
                 return std::make_optional(time);
             } else if (s == 's') {
                 return std::make_optional(final);
             } else if (s == 't') {
                 return std::make_optional(final);
             } else if (s == 'h') {
                 return std::make_optional(final);
             } else {
                 return std::nullopt;
             }
         }}},
        {final, TransitionsType{[tempExpr](char s) -> std::optional<State> {
             return std::make_optional(tempExpr);
         }}}
    };
    return FSM{std::move(states), std::move(finals), std::move(transitions)};
}

} //! namespace atom::automaton
