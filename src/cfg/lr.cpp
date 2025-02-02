#include "include/cfg/lr.h"

#include "include/utils/assertion.h"

namespace atom::cfg::lr {

BadLR::BadLR(std::string_view msg):
m_msg(msg) {}

const char* BadLR::what() const noexcept {
    return m_msg.data();
}

Item::Item(NonTerminalType& left, ProductionType& production, IndexType derivationIndex):
m_left(left),
m_production(production),
m_derivationIndex(derivationIndex)
{}

const Item::DerivationType& Item::getCurrentDerivation() const {
    auto it = m_production.cbegin() + m_derivationIndex;
    ASSERTION(it != m_production.cend(), BadLR, "Out of production range")
    return *it;
}

Item Item::shift() const noexcept {
    return Item{m_left, m_production, m_derivationIndex + 1};
}

bool Item::isReducing() const noexcept {
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
m_prodRules(prodRules),
m_itemClosureCacheTable(),
m_itemsClosureCachTable()
{}

const Items& ClosureAndGotoReqHandler::getGoto(const Items& items, const SymbolType& symbol) {
    Items newItems;
    for (const auto& item : items) {
        if (item.getCurrentDerivation() == symbol) {
            newItems.insert(item.shift());
        }
    }

    if (!newItems.empty()) {
        return getClosure(newItems);
    } else {
        ASSERTION(false, BadLR, "Could not find fit items: Invalid items or/and symbol")
    }
}

const Items& ClosureAndGotoReqHandler::getClosure(const Items& items) {
    auto it = m_itemsClosureCachTable.find(items);
    if (it != m_itemsClosureCachTable.cend()) {
        return it->second;
    }

    auto newItems = makeClosure(items);
    it = m_itemsClosureCachTable.insert(std::make_pair(items, newItems)).first;
    assert(it != m_itemsClosureCachTable.cend());
    return it->second;
}

const Items& ClosureAndGotoReqHandler::getClosure(const Item& item) {
    auto it = m_itemClosureCacheTable.find(item);
    if (it != m_itemClosureCacheTable.cend()) {
        return it->second;
    }

    auto items = makeClosure(item);
    it = m_itemClosureCacheTable.insert(std::make_pair(item, items)).first;
    assert(it != m_itemClosureCacheTable.cend());
    return it->second;
}

Items ClosureAndGotoReqHandler::makeClosure(const Items& items) {
    Items result = {items};
    for (const auto& item : items) {
        const auto tmpItems = getClosure(item);
        result.insert(tmpItems.begin(), tmpItems.end());
    }
    return result;
}

Items ClosureAndGotoReqHandler::makeClosure(const Item& item) {
    ASSERTION(!m_prodRules.isEmpty(), BadLR, "Production rules can`t be empty(Invalid grammar)")
    Items items = {item};
    if (item.isReducing()) {
        return items;
    }

    std::vector<SymbolType> searchSymbols = {item.getCurrentDerivation()};
    std::unordered_set<SymbolType> applyedSymbols;
    while (!searchSymbols.empty()) {
        auto searchSymbol = searchSymbols.back();
        searchSymbols.pop_back();

        if (!searchSymbol.isTerminal()) {
            auto it = m_prodRules.find(grammar::NonTerminal{searchSymbol});
            ASSERTION(it != m_prodRules.cend(),BadLR, "Could not find productions for search symbol(Invalid grammar)")
            auto& [left, prods] = *it;
            for (auto& prod : prods) {
                const auto& firstDerivation = *prod.begin();
                if (!firstDerivation.isTerminal() && !applyedSymbols.contains(firstDerivation)) {
                    searchSymbols.push_back(firstDerivation);
                    applyedSymbols.insert(firstDerivation);
                }
                items.insert(Item{left, prod});
            }
        }
    }
    return items;
}

bool operator==(const Item& lhs, const Item& rhs) {
    return std::hash<Item>{}.operator()(lhs) == std::hash<Item>{}.operator()(rhs);
}

bool operator==(const Items& lhs, const Items& rhs) {
    return std::hash<Items>{}.operator()(lhs) == std::hash<Items>{}.operator()(rhs);
}

} //! namespace atom::cfg::lr
