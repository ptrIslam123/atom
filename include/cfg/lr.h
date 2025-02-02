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

class BadLR final : public std::exception {
public:
    explicit BadLR(std::string_view msg);
    virtual const char* what() const noexcept;

private:
    std::string m_msg;
};

class Item final {
public:
    using ProductionType = atom::cfg::grammar::Production;
    using NonTerminalType = atom::cfg::grammar::NonTerminal;
    using DerivationType = ProductionType::DerivationType;
    using IndexType = unsigned int;

    explicit Item(NonTerminalType& left, ProductionType& production, IndexType derivationIndex = 0);

    const DerivationType& getCurrentDerivation() const noexcept(false);
    Item shift() const noexcept(true);
    bool isReducing() const noexcept(true);

    std::ostream& operator<<(std::ostream& os) const;

private:
    NonTerminalType& m_left;
    ProductionType& m_production;
    IndexType m_derivationIndex;
};

std::ostream& operator<<(std::ostream& os, const Item& item);

using Items = std::unordered_set<Item>;

std::ostream& operator<<(std::ostream& os, const Items& items);

bool operator==(const Item& lhs, const Item& rhs);
bool operator==(const Items& lhs, const Items& rhs);

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
struct hash<Items> {
    std::size_t operator()(const Items& items) const {
        size_t hash = 0;
        for (const auto& item : items) {
            hash ^= std::hash<Item>{}.operator()(item);
        }
        return hash;
    }
};

} //! namespace std

namespace atom::cfg::lr {

class ClosureAndGotoReqHandler final {
public:
    using ProductionRulesType = atom::cfg::grammar::ProductionRules;
    using ProductionsType = atom::cfg::grammar::Productions;
    using ProductionType = atom::cfg::grammar::Production;
    using DerivationType = ProductionType::DerivationType;
    using SymbolType = atom::cfg::grammar::Symbol;

    explicit ClosureAndGotoReqHandler(ProductionRulesType& prodRules);

    const Items& getClosure(const Items& items) noexcept(false);
    const Items& getClosure(const Item& item) noexcept(false);
    const Items& getGoto(const Items& items, const SymbolType& symbol) noexcept(false);

private:
    Items makeClosure(const Item& item);
    Items makeClosure(const Items& items);

    ProductionRulesType& m_prodRules;
    std::unordered_map<Item, Items> m_itemClosureCacheTable;
    std::unordered_map<Items, Items> m_itemsClosureCachTable;
};

} //! namespace atom::cfg::lr

#endif //! CFG_LR_H
