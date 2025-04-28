#ifndef ATOM_FSM_OPS_H
#define ATOM_FSM_OPS_H

#include "include/automaton/fsm.h"
#include "include/utils/assertion.h"

// namespace atom::automaton {

// template<typename T>
// FSM<T> Combine(const FSM<T>& firhighPrioritystFSM, const FSM<T>& lowPriorityFSM);


// template<typename T>
// FSM<T> Combine(const FSM<T>& firhighPrioritystFSM, const FSM<T>& lowPriorityFSM) {
//     using StatesType = FSM<T>::StatesType;
//     using TransitionTableType = FSM<T>::TransitionTableType;
//     using TransitionsType = FSM<T>::TransitionsType;

//     StatesType combinedStates;
//     for (const StatesType& states : {firhighPrioritystFSM.getStates(), lowPriorityFSM.getStates()}) {
//         combinedStates.insert(states.cbegin(), states.cend());
//     }

//     StatesType combinedFinals;
//     for (const StatesType& finals : {firhighPrioritystFSM.getFinals(), lowPriorityFSM.getFinals()}) {
//         combinedFinals.insert(finals.cbegin(), finals.cend());
//     }

//     TransitionTableType combinedTransitions;
//     {
//         for (const auto& transitionTables : {firhighPrioritystFSM.getTransitionTable(), lowPriorityFSM.getTransitionTable()}) {
//             for (const auto& [state, transitions] : transitionTables) {
//                 if (state != STATE0()) {
//                     ASSERTION(!combinedTransitions.contains(state), BadFSM, "Detected fsm states intersection(States must be unqiue)")
//                     combinedTransitions.insert({state, transitions});
//                 } else {
//                     continue;
//                 }
//             }
//         }
//         assert(!combinedTransitions.contains(STATE0()));
//         TransitionsType highPriorityTransitions;
//         {
//             auto it = firhighPrioritystFSM.getTransitionTable().find(STATE0());
//             ASSERTION(it != firhighPrioritystFSM.getTransitionTable().cend() && !it->second.empty(), BadFSM, "The first FSM doesn`t contains STATE0 state")
//             highPriorityTransitions = it->second;
//         }
//         TransitionsType lowPriorityTransitions;
//         {
//             auto it = lowPriorityFSM.getTransitionTable().find(STATE0());
//             ASSERTION(it != lowPriorityFSM.getTransitionTable().cend() && !it->second.empty(), BadFSM, "The second FSM doesn`t contains STATE0 state")
//             lowPriorityTransitions = it->second;
//         }
//         TransitionsType combined;
//         combined.reserve(highPriorityTransitions.size() + lowPriorityTransitions.size());
//         for (const auto& item : {highPriorityTransitions, lowPriorityTransitions}) {
//             combined.insert(combined.end(), item.begin(), item.end());
//         }

//         combinedTransitions.insert({STATE0(), std::move(combined)});
//     }
//     return FSM{std::move(combinedStates), std::move(combinedFinals), std::move(combinedTransitions)};
// }

// } //! namespace atom::automaton

#endif //! ATOM_FSM_OPS_H
