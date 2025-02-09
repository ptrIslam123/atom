#include "include/cfg/grammar.h"

#include "include/utils/assertion.h"

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
Terminal::Terminal(const Symbol& symbol): Symbol(symbol.getData(), symbol.isTerminal()) {}

bool Terminal::operator==(const Terminal& other) const {
    return std::hash<Terminal>{}(*this) == std::hash<Terminal>{}(other);
}

NonTerminal::NonTerminal(std::string_view data): Symbol(data, false) {}
NonTerminal::NonTerminal(const Symbol& symbol): Symbol(symbol.getData(), symbol.isTerminal()) {}
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

//////// class Production

Production::Production(const std::span<const DerivationType> derivations): m_derivations() {
    for (const auto& derivation : derivations) {
        m_derivations.push_back(derivation);
    }
}

Production::IteratorType Production::begin() { return m_derivations.begin(); }
Production::IteratorType Production::end() { return m_derivations.end(); }
Production::IteratorType Production::find(const DerivationType& derivation) {
    for (auto& item : m_derivations) {
        if (item == derivation) {
            return IteratorType{ &item };
        }
    }
    return end();
}

Production::ConstIteratorType Production::cbegin() const { return m_derivations.cbegin(); }
Production::ConstIteratorType Production::cend() const { return m_derivations.cend(); }
Production::ConstIteratorType Production::find(const DerivationType& derivation) const {
    for (auto& item : m_derivations) {
        if (item == derivation) {
            return ConstIteratorType{ &item };
        }
    }
    return cend();
}

Production::ReverseIteratorType Production::rbegin() { return m_derivations.rbegin(); }
Production::ReverseIteratorType Production::rend() { return m_derivations.rend(); }

Production::ConstReverseIteratorType Production::crbegin() const { return m_derivations.crbegin(); }
Production::ConstReverseIteratorType Production::crend() const { return m_derivations.crend(); }

void Production::pushBack(const DerivationType& newDerivation) {
    m_derivations.push_back(newDerivation);
}
std::size_t Production::size() const { return m_derivations.size(); }
std::size_t Production::size() { return static_cast<const Production*>(this)->size(); }

bool Production::isEmpty() const { return size() == 0; }
bool Production::isEmpty() { return static_cast<const Production*>(this)->isEmpty(); }


const Production::DerivationType& Production::back() const {
    assert(!isEmpty());
    return *(--cend());
}

const Production::DerivationType& Production::front() const {
    assert(!isEmpty());
    return *cbegin();
}

Production::DerivationType& Production::back() {
    assert(!isEmpty());
    return *(--end());
}

Production::DerivationType& Production::front() {
    assert(!isEmpty());
    return *begin();
}

bool Production::isSame(const Production& other) const {
    if (other.m_derivations.size() != m_derivations.size()) {
        return false;
    }
    for (auto i = 0; i < m_derivations.size(); ++i) {
        if (m_derivations[i] != other.m_derivations[i]) {
            return false;
        }
    }
    return true;
}

std::ostream& Production::operator<<(std::ostream& os) const {
    for (auto it = cbegin(); it != cend(); ++it) {
        const DerivationType& derivation = *it;
        os << derivation;
        if (it + 1 != cend()) {
            os << " ";
        }
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const Production& production) {
    return production.operator<<(os);
}

Production& operator>>(Production& production, const Production::DerivationType& newDerivation) {
    production.pushBack(newDerivation);
    return production;
}

//////// class Productions

Productions::Productions(std::span<const Production> productions): m_productions() {
    for (const auto& production : productions) {
        m_productions.push_back(production);
    }
}

Productions::ConstIteratorType Productions::cbegin() const { return ConstIteratorType{ &m_productions[0] }; }
Productions::ConstIteratorType Productions::cend() const { return ConstIteratorType{ &m_productions[0] + size() }; }
Productions::ConstIteratorType Productions::find(const Production& production) const {
    for (auto it = cbegin(); it != cend(); ++it) {
        if (*it == production) {
            return it;
        }
    }
    return cend();
}

Productions::IteratorType Productions::begin() { return IteratorType{ &m_productions[0] }; }
Productions::IteratorType Productions::end() { return IteratorType{ &m_productions[0] + size() }; }
Productions::IteratorType Productions::find(const Production& production) {
    for (auto it = begin(); it != end(); ++it) {
        if (*it == production) {
            return it;
        }
    }
    return end();
}

void Productions::pushBack(const Production& newProduction) {
    m_productions.push_back(newProduction);
}

std::size_t Productions::size() const { return m_productions.size(); }
std::size_t Productions::size() { return static_cast<const Productions*>(this)->size(); }

bool Productions::isEmpty() const { return size() == 0; }
bool Productions::isEmpty() { return static_cast<const Productions*>(this)->isEmpty(); }

const Production& Productions::back() const {
    assert(!isEmpty());
    return *(cend() - 1);
}

const Production& Productions::front() const {
    assert(!isEmpty());
    return *cbegin();
}

Production& Productions::back() {
    assert(!isEmpty());
    return *(end() - 1);
}

Production& Productions::front() {
    assert(!isEmpty());
    return *begin();
}

std::ostream& Productions::operator<<(std::ostream& os) const {
    for (auto it = cbegin(); it != cend(); ++it) {
        const Production& prod = *it;
        os << prod;
        if (it + 1 != cend()) {
            os << " | ";
        }
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const Productions& productions) {
    return productions.operator<<(os);
}

Productions& operator>>(Productions& productions, const Production& newProduction) {
    productions.pushBack(newProduction);
    return productions;
}

bool Productions::isSame(const Productions& other) const {
    if (size() != other.size()) {
        return false;
    }

    for (auto i = 0; i < size(); ++i) {
        if (*(cbegin() + i) != *(other.cbegin() + i)) {
            return false;
        }
    }
    return true;
}

//////// class ProductionRules

ProductionRules::ProductionRules():
m_rules() {}

ProductionRules::ConstIteratorType ProductionRules::cbegin() const { return ConstIteratorType{ &m_rules[0] }; }
ProductionRules::ConstIteratorType ProductionRules::cend() const { return ConstIteratorType{ &m_rules[0] + size() }; }
ProductionRules::ConstIteratorType ProductionRules::find(const NonTerminal& left) const {
    for (auto it = cbegin(); it != cend(); ++it) {
        if (it->first == left) {
            return it;
        }
    }
    return cend();
}
std::size_t ProductionRules::size() const { return m_rules.size(); }
bool ProductionRules::isEmpty() const { return size() == 0; }

ProductionRules::IteratorType ProductionRules::begin() { return IteratorType{ &m_rules[0] }; }
ProductionRules::IteratorType ProductionRules::end() { return IteratorType{ &m_rules[0] + size() }; }
ProductionRules::IteratorType ProductionRules::find(const NonTerminal& left) {
    for (auto it = begin(); it != end(); ++it) {
        if (it->first == left) {
            return it;
        }
    }
    return end();
}
std::optional<ProductionRules::DerivationType> ProductionRules::findNextDerivation(const DerivationType& derivation) const {
    return static_cast<const ProductionRules*>(this)->findNextDerivation(derivation);
}
std::size_t ProductionRules::size() { return static_cast<const ProductionRules*>(this)->size(); }
bool ProductionRules::isEmpty() { return static_cast<const ProductionRules*>(this)->isEmpty(); }

ProductionRules& ProductionRules::newProduction(const NonTerminal& left) {
    m_rules.push_back(std::make_pair(left, Productions{}));
    return *this;
}

void ProductionRules::pushBack(const Symbol& symbol) {
    assert(!m_rules.empty());
    auto& currentProductions = m_rules.back().second;
    if (currentProductions.isEmpty()) {
        currentProductions.pushBack(Production{});
    }

    auto& currentProduction = currentProductions.back();
    currentProduction.pushBack(symbol);
}

void ProductionRules::pushBack(NonTerminal&& left, Productions&& productions) {
    m_rules.emplace_back(std::move(left), std::move(productions));
}

std::optional<ProductionRules::DerivationType> ProductionRules::findNextDerivation(const DerivationType& derivation) {
    for (const auto& [_, prods] : m_rules) {
        for (auto it = prods.cbegin(); it != prods.cend(); ++it) {
            const Production& prod = *it;
            auto derivIt = prod.find(derivation);
            if (derivIt != prod.cend() && derivIt + 1 != prod.cend()) {
                return *(++derivIt);
            }
        }
    }
    return std::nullopt;
}

std::ostream& ProductionRules::operator<<(std::ostream& os) const {
    for (const auto& [left, prods] : m_rules) {
        os << left << " -> " << prods << ";" << "\n";
    }
    return os;
}

ProductionRules& operator>>(ProductionRules& prodRules, const Symbol& symbol) {
    prodRules.pushBack(symbol);
    return prodRules;
}

std::ostream& operator<<(std::ostream& os, const ProductionRules& prodRules) {
    return prodRules.operator<<(os);
}

} //! namespace atom::cfg::grammar
