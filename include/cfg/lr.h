#ifndef CFG_LR_H
#define CFG_LR_H

#include "include/cfg/grammar.h"
#include "include/cfg/node.h"

#include <functional>
#include <ostream>
#include <sstream>
#include <unordered_set>
#include <unordered_map>

namespace atom::cfg::lr {

class Item final {
public:
    using ProductionType = atom::cfg::grammar::Production;
    using NonTerminalType = atom::cfg::grammar::NonTerminal;
    using DerivationType = ProductionType::DerivationType;
    using IndexType = unsigned int;

    explicit Item(NonTerminalType& left, ProductionType& production, IndexType derivationIndex = 0);

    const DerivationType& getCurrentDerivation() const;

    Item shift() const;
    bool isReducing() const;

    std::ostream& operator<<(std::ostream& os) const;

private:
    NonTerminalType& m_left;
    ProductionType& m_production;
    IndexType m_derivationIndex;
};

std::ostream& operator<<(std::ostream& os, const Item& item);

} //! namespace atom::cfg::lr

namespace std {

using namespace atom::cfg::lr;

template<>
struct hash<Item> {
    std::size_t operator()(const Item& item) const {
        //! TODO: I think there is better way for hashing Item`s structure instance.
        std::stringstream ss;
        ss << item;
        return std::hash<std::string>{}.operator()(ss.str());
    }
};

template<>
struct equal_to<Item> {
    bool operator()(const Item& lhs, const Item& rhs) const {
        return std::hash<Item>{}.operator()(lhs) == std::hash<Item>{}.operator()(rhs);
    }
};

} //! namespace std

namespace atom::cfg::lr {

using Items = std::unordered_set<Item>;

std::ostream& operator<<(std::ostream& os, const Items& items);

class ClosureAndGotoReqHandler final {
public:
    using ProductionRulesType = atom::cfg::grammar::ProductionRules;
    using ProductionsType = atom::cfg::grammar::Productions;
    using ProductionType = atom::cfg::grammar::Production;
    using DerivationType = ProductionType::DerivationType;
    using SymbolType = atom::cfg::grammar::Symbol;

    explicit ClosureAndGotoReqHandler(ProductionRulesType& prodRules);

    const Items& getClosure(const Item& item);
    const Items& getGoto(const Items& items, const SymbolType& symbol);

private:
    Items makeClosure(const Item& item);

    ProductionRulesType& m_prodRules;
    std::unordered_map<Item, Items> m_closureCacheTable;
};

bool operator==(const Items& lhs, const Items& rhs);

} //! namespace atom::cfg::lr

#endif //! CFG_LR_H
