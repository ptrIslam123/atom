#include "include/automaton/fsm.h"

#include <atomic>
#include <limits>
#include <cassert>

namespace {

std::uint64_t AllocateUniqueId() {
    constexpr auto ID_LIMIT = std::numeric_limits<atom::automaton::State::IdType>().max();
    static std::atomic<atom::automaton::State::IdType> counter{0};
    auto currentCounter = counter.load();
    for (;;) {
        if (currentCounter < ID_LIMIT && counter.compare_exchange_weak(currentCounter, currentCounter + 1)) {
            break;
        } else if (currentCounter < ID_LIMIT) {
            continue;
        } else {
            ASSERTION(false, std::runtime_error, "Unique ID limit reached")
        }
    }
    return currentCounter;
}

std::size_t GetUTF8CharLength(const char8_t firstByte) {
    if ((firstByte & 0b10000000) == 0b00000000) return 1;
    if ((firstByte & 0b11100000) == 0b11000000) return 2;
    if ((firstByte & 0b11110000) == 0b11100000) return 3;
    if ((firstByte & 0b11111000) == 0b11110000) return 4;
    return 0;
}

} //! namespace

namespace atom::automaton {

BadFSM::BadFSM(std::string_view msg):
m_msg("BadFSM:: " + std::string{msg}) {}

const char* BadFSM::what() const noexcept {
    return m_msg.data();
}

FSM::FSM(StatesType&& states, StatesType&& finals, TransitionTableType&& transitionTable):
m_states(std::move(states)),
m_finals(std::move(finals)),
m_transitionTable(std::move(transitionTable))
{}

typename FSM::ProcessResultType FSM::analyze(const std::span<const SymbolType> sequence) {
    ASSERTION(!sequence.empty(), BadFSM, "Expected non empty sequence for analyze")
    m_context.clear();
    std::size_t analyzedSequenceLength = 0;
    const auto& invalidState = State::INVALID();
    {
        auto startContext = makeContext(State::STATE0(), sequence);
        if (startContext.has_value()) {
            m_context.push_back(std::move(*startContext));
        } else {
            return ProcessResultType{invalidState, {}};
        }
    }

    while (!m_context.empty()) {
        Context& currentContext = m_context.back();
        auto& subSequence = currentContext.subSequence;
        if (subSequence.empty()) {
            return ProcessResultType{invalidState, {}};
        }

        auto& transitions = currentContext.transitions;
        if (transitions.empty()) {
            m_context.pop_back();
            continue;
        }

        const TransitionType& transition = transitions[0];
        transitions = transitions.subspan(1);

        const std::size_t lenght = GetUTF8CharLength(subSequence.front());
        ASSERTION(((lenght >= 1 && lenght <= 4) && (lenght <= subSequence.size())), BadFSM, "Invalid utf-8 symbol length")
        const State& state = transition(&*subSequence.begin(), lenght);
        if (state == invalidState) {
            continue;
        }

        analyzedSequenceLength += lenght;

        if (!m_finals.contains(state)) {
            switch (switchToNextState(state, currentContext, lenght)) {
                case SwitchToNextStateResult::Success:
                    break;
                case SwitchToNextStateResult::Unsuccess:
                    m_context.pop_back();
                    break;
                case SwitchToNextStateResult::CannotContinue:
                    return ProcessResultType{invalidState, {}};
                default:
                    assert(false);
            }
        } else {
            ASSERTION(analyzedSequenceLength <= sequence.size(), BadFSM, "Expected non empty context")
            return ProcessResultType{state, sequence.subspan(0, analyzedSequenceLength - 1)};
        }
    };
    return ProcessResultType{invalidState, {}};
}

FSM::SwitchToNextStateResult FSM::switchToNextState(const State& currentState, Context& currentContext, const std::size_t length) {
    if (!currentContext.subSequence.empty()) {
        auto newContext = makeContext(currentState, currentContext.subSequence.subspan(length));
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

std::optional<typename FSM::Context> FSM::makeContext(const State& currentState, const std::span<const SymbolType> subSequence) const {
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

const FSM::StatesType& FSM::getStates() const & noexcept {
    return m_states;
}

const FSM::StatesType& FSM::getFinals() const & noexcept {
    return m_finals;
}

const FSM::TransitionTableType& FSM::getTransitionTable() const & noexcept {
    return m_transitionTable;
}

const State& State::STATE0()
{
    static const State state0;
    return state0;
}

const State& State::INVALID()
{
    static const State invalid;
    return invalid;
}

State::State():
m_id(AllocateUniqueId()) {}


State::State(const State& other) {
    (void)this->operator=(other);
}

State::State(State&& other) noexcept {
    (void)this->operator=(std::move(other));
}

State& State::operator=(const State& other) {
    m_id = other.m_id;
    return *this;
}

State& State::operator=(State&& other) noexcept {
    return this->operator=(other);
}

State::IdType State::getId() const {
    return m_id;
}

std::ostream& State::operator<<(std::ostream& os) const {
    os << "atom::automaton::State{" << getId() << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const State& state) {
    return state.operator<<(os);
}

bool operator==(const State& f, const State& s) {
    return std::hash<State>{}.operator()(f) == std::hash<State>{}.operator()(s);
}

bool operator!=(const State& f, const State& s) {
    return !(f == s);
}

} //! namespace atom::automaton
