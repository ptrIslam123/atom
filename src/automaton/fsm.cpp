#include "include/automaton/fsm.h"

#include <atomic>
#include <limits>
#include <cassert>

namespace {

std::uint64_t AllocateUniqueId() noexcept(false) {
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

} //! namespace

namespace atom::automaton {

BadFSM::BadFSM(std::string_view msg):
m_msg("BadFSM:: " + std::string{msg}) {}

const char* BadFSM::what() const noexcept {
    return m_msg.data();
}

const State& STATE0()
{
    static const State state0;
    return state0;
}

State::State():
m_id(AllocateUniqueId()) {}

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
