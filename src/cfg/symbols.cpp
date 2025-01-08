#include "include/cfg/symbols.h"

#include "include/utils/assertion.h"

#include <stdexcept>
#include <cassert>

namespace atom::cfg::grammar {

std::string_view Symbol::getData() const { return m_data; }
std::string_view Symbol::getData() { return m_data; }

bool Symbol::isTerminal() const { return m_isTerminal; }
bool Symbol::isTerminal() { return m_isTerminal; }

bool Symbol::operator==(const Symbol& other) const {
    return std::hash<Symbol>{}(*this) == std::hash<Symbol>{}(other);
}

Symbol::Symbol(std::string_view data, bool isTerminal):
m_data(data), m_isTerminal(isTerminal) {
    assert(!m_data.empty());
    ASSERTION(!m_data.empty(), std::runtime_error, "Empty symbol")
}

Terminal::Terminal(std::string_view data): Symbol(data, true) {}

bool Terminal::operator==(const Terminal& other) const {
    return std::hash<Terminal>{}(*this) == std::hash<Terminal>{}(other);
}

NonTerminal::NonTerminal(std::string_view data): Symbol(data, false) {}

bool NonTerminal::operator==(const NonTerminal& other) const {
    return std::hash<NonTerminal>{}(*this) == std::hash<NonTerminal>{}(other);
}

std::ostream& operator<<(std::ostream& os, const Symbol& symbol) {
    os << (symbol.isTerminal() ? "Terminal" : "NonTerminal");
    os << "(" << symbol.getData() << ")";
    return os;
}
std::ostream& operator<<(std::ostream& os, const Terminal& terminal) { return os << static_cast<Symbol>(terminal); }
std::ostream& operator<<(std::ostream& os, const NonTerminal& nonTerminal) { return os << static_cast<Symbol>(nonTerminal); }

} //! namespace atom::cfg::grammar
