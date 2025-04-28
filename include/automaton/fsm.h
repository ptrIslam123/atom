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
    State(const State& other);
    State(State&& other) noexcept;
    State& operator=(const State& other);
    State& operator=(State&& other) noexcept;
    ~State() = default;

    /**
     * @brief Returns the ID of the state.
     * @return The state's ID.
     */
    IdType getId() const;
    std::ostream& operator<<(std::ostream& os) const;

    /**
     * @brief Returns the initial state `STATE0`.
     * @return The initial state.
     */
    static const State& STATE0();
    
    /**
     * @brief Returns the invalid state.
     * @return The invalid state.
     */
    static const State& INVALID();

private:
    IdType m_id;
};

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
 * @final
 */
class FSM final {
public:
    using SymbolType = char8_t;
    using StatesType = std::unordered_set<State>; ///< Type for the set of states.
    using TransitionType = std::function<State(const SymbolType* startSymbol, std::size_t length)>; ///< Type for transition functions.
    using TransitionsType = std::vector<TransitionType>; ///< Type for a collection of transition functions.
    using TransitionTableType = std::unordered_map<State/*currentState*/, TransitionsType /*transition callback*/>; ///< Type for the transition table.
    using ProcessResultType = std::pair<State,  std::span<const SymbolType>>; ///< Type for the result of processing.

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
    ProcessResultType analyze(std::span<const SymbolType> sequence);

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
        State currentState{State::STATE0()};
        std::span<const SymbolType> subSequence;
        std::span<const TransitionType> transitions;
    };

    enum class SwitchToNextStateResult {
        Success,
        Unsuccess,
        CannotContinue,
    };

    std::optional<Context> makeContext(const State& currentState, std::span<const SymbolType> subSequence) const;
    SwitchToNextStateResult switchToNextState(const State& currentState, Context& currentContext, std::size_t length);

    std::vector<Context> m_context;
    StatesType m_states;
    StatesType m_finals;
    TransitionTableType m_transitionTable;
};

} //! namespace atom::automaton

#endif //! ATOM_FSM_H
