#include "include/cfg/grammar.h"
#include <list>
#include <stack>
#include <cassert>

namespace atom::ast::cfg::grammar {


//////// class Production

Production::Production(const std::span<const DerivationType> derivations): m_derivations() {
    for (const auto& derivation : derivations) {
        m_derivations.push_back(derivation);
    }
}

Production::IteratorType Production::begin() { return IteratorType{ &m_derivations[0] }; }
Production::IteratorType Production::end() { return IteratorType{ &m_derivations[0] + size() }; }
Production::IteratorType Production::find(const DerivationType& derivation) {
    for (auto& item : m_derivations) {
        if (item == derivation) {
            return IteratorType{ &item };
        }
    }
    return end();
}

Production::ConstIteratorType Production::cbegin() const { return ConstIteratorType{ &m_derivations[0] }; }
Production::ConstIteratorType Production::cend() const { return ConstIteratorType{ &m_derivations[0] + size() }; }
Production::ConstIteratorType Production::find(const DerivationType& derivation) const {
    for (auto& item : m_derivations) {
        if (item == derivation) {
            return ConstIteratorType{ &item };
        }
    }
    return cend();
}

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


//////// class FirstAndFollowReqHandler


FirstAndFollowReqHandler::FirstAndFollowReqHandler(ProductionRules& prodRules):
m_prodRules(prodRules),
m_firstTableCache() {}

const FirstAndFollowReqHandler::FollowSetType& FirstAndFollowReqHandler::getFollow(const Symbol& symbol) {
    static const FollowSetType emptyFollow;
    if (symbol.isTerminal()) {
        return emptyFollow;
    }

    auto it = m_followTableCache.find(symbol);
    if (it != m_followTableCache.cend()) {
        return it->second;
    }

    auto follow = makeFollow(symbol);
    if (follow.empty()) {
        return emptyFollow;
    }

    it = m_followTableCache.insert(std::make_pair(symbol, follow)).first;
    assert(it != m_followTableCache.cend());
    return it->second;
}

const FirstAndFollowReqHandler::FirstSetType& FirstAndFollowReqHandler::getFirst(const Symbol& symbol) {
    static const FirstSetType emptyFirst;
    auto it = m_firstTableCache.find(symbol);
    if (it != m_firstTableCache.cend()) {
        return it->second;
    }

    auto first = makeFirst(symbol);
    if (first.empty()) {
        return emptyFirst;
    }

    it = m_firstTableCache.insert(std::make_pair(symbol, first)).first;
    assert(it != m_firstTableCache.cend());
    return it->second;
}

FirstAndFollowReqHandler::FirstSetType FirstAndFollowReqHandler::makeFirst(const Symbol& symbol) {
    FirstSetType first;
    std::list<Symbol> context;
    context.push_back(symbol);

    while (!context.empty()) {
        const auto searchSymbol = context.back();
        context.pop_back();

        if (searchSymbol.isTerminal()) {
            first.insert(Terminal{ searchSymbol.getData() });
            continue;
        }

        auto it = m_prodRules.find(NonTerminal{ searchSymbol.getData() });
        if (it == m_prodRules.end()) {
            assert(false);
            return first;
        }
        
        const auto& [_, prods] = *it;
        for (auto prodIt = prods.cbegin(); prodIt != prods.cend(); ++prodIt) {
            FirstSetType tmpFirst;
            const Production& prod = *prodIt;
            assert(!prod.isEmpty());

            const DerivationType& firstDerivation = *prod.cbegin();
            if (firstDerivation.isTerminal()) {
                tmpFirst.insert(Terminal{ firstDerivation.getData() });
            } else {
                auto cacheIt = m_firstTableCache.find(firstDerivation);
                if (cacheIt != m_firstTableCache.cend()) {
                    tmpFirst.insert(cacheIt->second.begin(), cacheIt->second.end());
                } else {
                    context.push_back(static_cast<Symbol>(firstDerivation));
                }
            }

            if (auto _it = tmpFirst.find(None);
                _it != tmpFirst.end()) {
                tmpFirst.erase(_it);

                const auto nextSymbol = m_prodRules.findNextDerivation(searchSymbol);
                if (nextSymbol.has_value() && !nextSymbol->isTerminal()) {
                    auto cacheIt = m_firstTableCache.find(*nextSymbol);
                    if (cacheIt != m_firstTableCache.cend()) {
                        tmpFirst.insert(cacheIt->second.begin(), cacheIt->second.end());
                    } else {
                        context.push_back(static_cast<Symbol>(firstDerivation));
                    }
                } else if (nextSymbol.has_value() && nextSymbol->isTerminal()) {
                    tmpFirst.insert(Terminal{ nextSymbol->getData() });
                } else if (!nextSymbol.has_value()) {
                    tmpFirst.insert(None);
                }

                first.insert(tmpFirst.cbegin(), tmpFirst.cend());
            }

            first.insert(tmpFirst.cbegin(), tmpFirst.cend());
        }
    }
    return first;
}

FirstAndFollowReqHandler::FollowSetType FirstAndFollowReqHandler::makeFollow(const Symbol& symbol) {
    FollowSetType follow;
    const auto nextSymbol = m_prodRules.findNextDerivation(symbol);
    if (!nextSymbol.has_value()) {
        follow.insert(End);
        return follow;
    }

    const auto& first = getFirst(static_cast<Symbol>(*nextSymbol));
    if (first.empty()) {
        follow.insert(End);
        return follow;
    }

    follow.insert(first.cbegin(), first.cend());
    if (auto it = follow.find(None); it != follow.cend()) {
        follow.erase(it);
        follow.insert(End);
    } 
    return follow;
}

bool operator==(const FirstAndFollowReqHandler::FirstSetType& l, const FirstAndFollowReqHandler::FirstSetType& r) {
    if (l.size() != r.size()) {
        return false;
    }

    for (const auto& item : l) {
        if (r.find(item) == r.cend()) {
            return false;
        }
    }

    return true;
}

} //! namespace atom::ast::cfg::grammar
