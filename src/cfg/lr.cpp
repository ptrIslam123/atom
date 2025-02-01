#include "include/cfg/lr.h"

#include "include/utils/assertion.h"

namespace atom::cfg::lr {

Item::Item(NonTerminalType& left, ProductionType& production, IndexType derivationIndex):
m_left(left),
m_production(production),
m_derivationIndex(derivationIndex)
{}

const Item::DerivationType& Item::getCurrentDerivation() const {
    auto it = m_production.cbegin() + m_derivationIndex;
    ASSERTION(it != m_production.cend(), std::runtime_error, "")
    return *it;
}

Item Item::shift() const {
    return Item{m_left, m_production, m_derivationIndex + 1};
}

bool Item::isReducing() const {
    assert(m_derivationIndex <= m_production.size());
    return m_derivationIndex == m_production.size();
}

std::ostream& Item::operator<<(std::ostream& os) const {
    os << m_left << " -> ";
    for (auto it = m_production.cbegin(); it != m_production.cend(); ++it) {
        if (std::distance(m_production.cbegin(), it) == m_derivationIndex) {
            os << ".";
        }

        os << *it;

        if (it + 1 != m_production.cend()) {
            os << " ";
        }
    }
    os.flush();
    return os;
}

std::ostream& operator<<(std::ostream& os, const Item& item) {
    return item.operator<<(os);
}


std::ostream& operator<<(std::ostream& os, const Items& items) {
    os << "{ ";
    for (auto it = items.cbegin(); it != items.cend(); ++it) {
        os << *it << "; ";
    }
    os << " }";
    os.flush();
    return os;
}

ClosureAndGotoReqHandler::ClosureAndGotoReqHandler(ProductionRulesType& prodRules):
m_prodRules(prodRules)
{}

const Items& ClosureAndGotoReqHandler::getGoto(const Items& items, const SymbolType& symbol) {
    for (const auto& item : items) {
        if (item.getCurrentDerivation() == symbol) {
            return getClosure(item.shift());
        }
    }
    ASSERTION(false, std::runtime_error, "")
}

const Items& ClosureAndGotoReqHandler::getClosure(const Item& item) {
    auto it = m_closureCacheTable.find(item);
    if (it != m_closureCacheTable.cend()) {
        return it->second;
    }

    auto items = makeClosure(item);
    it = m_closureCacheTable.insert(std::make_pair(item, items)).first;
    assert(it != m_closureCacheTable.cend());
    return it->second;
}

Items ClosureAndGotoReqHandler::makeClosure(const Item& item) {
    Items items = {item};
    if (item.isReducing()) {
        return items;
    }

    std::vector<SymbolType> searchSymbols = {item.getCurrentDerivation()};
    while (!searchSymbols.empty()) {
        auto searchSymbol = searchSymbols.back();
        searchSymbols.pop_back();

        if (!searchSymbol.isTerminal()) {
            auto it = m_prodRules.find(grammar::NonTerminal{searchSymbol});
            ASSERTION(it != m_prodRules.cend(), std::runtime_error, "")
            auto& [left, prods] = *it;
            for (auto& prod : prods) {
                items.insert(Item{left, prod});
            }
        }
    }
    return items;
}

bool operator==(const Items& lhs, const Items& rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }

    for (const auto& v : lhs) {
        if (!rhs.contains(v)) {
            return false;
        }
    }
    return true;
}

} //! namespace atom::cfg::lr
