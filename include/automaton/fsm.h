#ifndef ATOM_FSM_H
#define ATOM_FSM_H

#include "include/utils/assertion.h"

#include <exception>
#include <span>
#include <optional>
#include <functional>
#include <ostream>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <cstdint>
#include <cassert>

namespace atom::automaton {

class BadFSM final : public std::exception {
public:
    explicit BadFSM(std::string_view msg);
    virtual const char* what() const noexcept;

private:
    std::string m_msg;
};

/**
 * @class State
 * @brief Represents a state in the finite state machine.
 * @final
 */
class State final {
public:
    using IdType = std::uint64_t; ///< The type used for state identifiers.

    explicit State();
    State(const State& ) = default;
    State(State&& ) noexcept = default;
    State& operator=(const State&) = default;
    State& operator=(State&& ) noexcept = default;
    ~State() = default;

    /**
     * @brief Returns the ID of the state.
     * @return The state's ID.
     */
    IdType getId() const;
    std::ostream& operator<<(std::ostream& os) const;

private:
    IdType m_id;
};

/**
 * @brief Returns the initial state `STATE0`.
 * @return The initial state.
 */
const State& STATE0();

std::ostream& operator<<(std::ostream& os, const State& state);
bool operator==(const State& f, const State& s);
bool operator!=(const State& f, const State& s);

} //! namespace atom::automaton

namespace std {

using namespace atom::automaton;

template<>
struct hash<State> {
    std::size_t operator()(const State& state) const {
        return std::hash<State::IdType>{}(state.getId());
    }
};

} //! namespace std

namespace atom::automaton {

/**
 * @class FSM
 * @brief Represents a finite state machine (FSM).
 * @tparam T The type of input values processed by the FSM.
 * @final
 */
template<typename T>
class FSM final {
public:
    using StatesType = std::unordered_set<State>; ///< Type for the set of states.
    using TransitionType = std::function<std::optional<State>(T currentValue)>; ///< Type for transition functions.
    using TransitionsType = std::vector<TransitionType>; ///< Type for a collection of transition functions.
    using TransitionTableType = std::unordered_map<State/*currentState*/, TransitionsType /*transition callback*/>; ///< Type for the transition table.
    using ProcessResultType = std::pair<State,  std::span<const T>>; ///< Type for the result of processing.

    const StatesType& getStates() && noexcept = delete;
    const StatesType& getFinals() && noexcept = delete;
    const TransitionTableType& getTransitionTable() && noexcept = delete;

    /**
     * @brief Constructs an FSM.
     * @param states The set of states.
     * @param finals The set of final (accepting) states.
     * @param transitionTable The transition table.
     */
    explicit FSM(StatesType&& states, StatesType&& finals, TransitionTableType&& transitionTable);

    /**
     * @brief Analyzes an input sequence using the FSM.
     * @param sequence The input sequence to analyze.
     * @return An optional result containing the final state and the processed subsequence.
     * @throws BadFSM If the sequence is empty.
     */
    std::optional<ProcessResultType> analyze(std::span<const T> sequence) noexcept(false);

    /**
     * @brief Returns the set of states(Non-final states).
     * @return The set of states.
     */
    const StatesType& getStates() const & noexcept;

    /**
     * @brief Returns the set of final states.
     * @return The set of final states.
     */
    const StatesType& getFinals() const & noexcept;

    /**
     * @brief Returns the transition table.
     * @return The transition table.
     */
    const TransitionTableType& getTransitionTable() const & noexcept;

private:
    struct Context {
        State currentState{STATE0()};
        std::span<const T> subSequence{};
        std::span<const TransitionType> transitions{};
    };

    enum class SwitchToNextStateResult {
        Success,
        Unsuccess,
        CannotContinue,
    };

    std::optional<Context> makeContext(const State& currentState, std::span<const T> subSequence) const;
    ProcessResultType makeResult(const State& finalState, Context& currentContext, std::span<const T> sequence);
    SwitchToNextStateResult switchToNextState(const State& currentState, Context& currentContext);

    std::vector<Context> m_context;
    StatesType m_states;
    StatesType m_finals;
    TransitionTableType m_transitionTable;
};

template<typename T>
FSM<T>::FSM(StatesType&& states, StatesType&& finals, TransitionTableType&& transitionTable):
m_states(std::move(states)),
m_finals(std::move(finals)),
m_transitionTable(std::move(transitionTable))
{}

template<typename T>
std::optional<typename FSM<T>::ProcessResultType> FSM<T>::analyze(const std::span<const T> sequence) {
    ASSERTION(!sequence.empty(), BadFSM, "Expected non empty sequence for analyze")
    m_context.clear();
    {
        auto startContext = makeContext(STATE0(), sequence);
        if (startContext.has_value()) {
            m_context.push_back(std::move(*startContext));
        } else {
            return std::nullopt;
        }
    }

    while (!m_context.empty()) {
        Context& currentContext = m_context.back();
        auto& subSequence = currentContext.subSequence;
        if (subSequence.empty()) {
            return std::nullopt;
        }

        auto& transitions = currentContext.transitions;
        if (transitions.empty()) {
            m_context.pop_back();
            continue;
        }

        const TransitionType& transition = transitions[0];
        transitions = transitions.subspan(1);

        auto result = transition(*subSequence.begin());
        if (!result.has_value()) {
            continue;
        }

        const State& state = *result;
        if (!m_finals.contains(state)) {
            switch (switchToNextState(state, currentContext)) {
                case SwitchToNextStateResult::Success:
                    break;
                case SwitchToNextStateResult::Unsuccess:
                    m_context.pop_back();
                    break;
                case SwitchToNextStateResult::CannotContinue:
                    return std::nullopt;
                default:
                    assert(false);
            }
        } else {
            return makeResult(state, currentContext, sequence);
        }
    };
    return std::nullopt;
}

template<typename T>
FSM<T>::ProcessResultType FSM<T>::makeResult(
    const State& finalState,
    Context& currentContext,
    const std::span<const T> sequence
) {
    ASSERTION(!m_context.empty(), BadFSM, "Expected non empty context")
    return std::make_pair(finalState, sequence.subspan(0, m_context.size() - 1));
}

template<typename T>
FSM<T>::SwitchToNextStateResult FSM<T>::switchToNextState(const State& currentState, Context& currentContext) {
    if (!currentContext.subSequence.empty()) {
        auto newContext = makeContext(currentState, currentContext.subSequence.subspan(1));
        if (newContext.has_value()) {
            m_context.push_back(std::move(*newContext));
            return SwitchToNextStateResult::Success;
        } else {
            return SwitchToNextStateResult::Unsuccess;
        }
    } else {
        return SwitchToNextStateResult::CannotContinue;
    }
}

template<typename T>
std::optional<typename FSM<T>::Context> FSM<T>::makeContext(const State& currentState, const std::span<const T> subSequence) const {
    Context newContext;
    auto it = m_transitionTable.find(currentState);
    if (it != m_transitionTable.cend()) {
        newContext.transitions = it->second;
        newContext.currentState = currentState;
        newContext.subSequence = subSequence;
        return newContext;
    } else {
        return std::nullopt;
    }
}

template<typename T>
const FSM<T>::StatesType& FSM<T>::getStates() const & noexcept {
    return m_states;
}

template<typename T>
const FSM<T>::StatesType& FSM<T>::getFinals() const & noexcept {
    return m_finals;
}

template<typename T>
const FSM<T>::TransitionTableType& FSM<T>::getTransitionTable() const & noexcept {
    return m_transitionTable;
}

} //! namespace atom::automaton

#endif //! ATOM_FSM_H
