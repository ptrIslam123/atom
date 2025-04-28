#ifndef ATOM_DEFAULT_FSMS_H
#define ATOM_DEFAULT_FSMS_H

#include "include/automaton/fsm.h"

#include <span>
#include <utility>

namespace atom::automaton {

FSM MakeSpecialSymbolFSMAnalyzer(std::span<const std::pair<FSM::SymbolType/*special symbol*/, State/*final state*/>> data);
FSM MakeNumberFSMAnalyzer(
   const State& integerNumber,
   const State& floatingNumber,
   const State& hexNumber,
   const State& binaryNumber
);
FSM MakeWordFSMAnalyzer(const State& word);
FSM MakeTemporalExpressionsFSMAnalyzer(const State& tempExpr);

} //! namespace atom::automaton

#endif //! ATOM_DEFAULT_FSMS_H
