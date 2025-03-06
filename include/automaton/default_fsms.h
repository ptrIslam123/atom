#ifndef ATOM_DEFAULT_FSMS_H
#define ATOM_DEFAULT_FSMS_H

#include "include/automaton/fsm.h"

#include <span>
#include <utility>

namespace atom::automaton {

FSM<char> MakeSpecialSymbolFSMAnalyzer(std::span<const std::pair<char/*special symbol*/, State/*final state*/>> data);
FSM<char> MakeNumberFSMAnalyzer(
    const State& integerNumber,
    const State& floatingNumber,
    const State& hexNumber,
    const State& binaryNumber
);
FSM<char> MakeWordFSMAnalyzer(const State& word);
FSM<char> MakeTemporalExpressionsFSMAnalyzer(const State& tempExpr);

} //! namespace atom::automaton

#endif //! ATOM_DEFAULT_FSMS_H
