#include "include/automaton/fsm_ops.h"

#include "include/utils/assertion.h"

namespace atom::automaton {

FSM Combine(const FSM& firhighPrioritystFSM, const FSM& lowPriorityFSM) {
    using StatesType = FSM::StatesType;
    using TransitionTableType = FSM::TransitionTableType;
    using TransitionsType = FSM::TransitionsType;

    StatesType combinedStates;
    for (const StatesType& states : {firhighPrioritystFSM.getStates(), lowPriorityFSM.getStates()}) {
        combinedStates.insert(states.cbegin(), states.cend());
    }

    StatesType combinedFinals;
    for (const StatesType& finals : {firhighPrioritystFSM.getFinals(), lowPriorityFSM.getFinals()}) {
        combinedFinals.insert(finals.cbegin(), finals.cend());
    }

    TransitionTableType combinedTransitions;
    {
        for (const auto& transitionTables : {firhighPrioritystFSM.getTransitionTable(), lowPriorityFSM.getTransitionTable()}) {
            for (const auto& [state, transitions] : transitionTables) {
                if (state != State::STATE0()) {
                    ASSERTION(!combinedTransitions.contains(state), BadFSM, "Detected fsm states intersection(States must be unqiue)")
                    combinedTransitions.insert({state, transitions});
                } else {
                    continue;
                }
            }
        }
        assert(!combinedTransitions.contains(State::STATE0()));
        TransitionsType highPriorityTransitions;
        {
            auto it = firhighPrioritystFSM.getTransitionTable().find(State::STATE0());
            ASSERTION(it != firhighPrioritystFSM.getTransitionTable().cend() && !it->second.empty(), BadFSM, "The first FSM doesn`t contains STATE0 state")
            highPriorityTransitions = it->second;
        }
        TransitionsType lowPriorityTransitions;
        {
            auto it = lowPriorityFSM.getTransitionTable().find(State::STATE0());
            ASSERTION(it != lowPriorityFSM.getTransitionTable().cend() && !it->second.empty(), BadFSM, "The second FSM doesn`t contains STATE0 state")
            lowPriorityTransitions = it->second;
        }
        TransitionsType combined;
        combined.reserve(highPriorityTransitions.size() + lowPriorityTransitions.size());
        for (const auto& item : {highPriorityTransitions, lowPriorityTransitions}) {
            combined.insert(combined.end(), item.begin(), item.end());
        }

        combinedTransitions.insert({State::STATE0(), std::move(combined)});
    }
    return FSM{std::move(combinedStates), std::move(combinedFinals), std::move(combinedTransitions)};
}

} //! namespace atom::automaton
