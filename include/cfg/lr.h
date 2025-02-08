#ifndef CFG_LR_H
#define CFG_LR_H

#include "include/cfg/grammar.h"
#include "include/cfg/node.h"
#include "include/utils/assertion.h"

#include <functional>
#include <ostream>
#include <sstream>
#include <memory>
#include <list>
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
    const NonTerminalType& getLeft() const noexcept(true);
    const ProductionType& getProduction() const noexcept(true);
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
    ClosureAndGotoReqHandler(const ClosureAndGotoReqHandler& ) = delete;
    ClosureAndGotoReqHandler(ClosureAndGotoReqHandler&& ) noexcept = delete;
    ClosureAndGotoReqHandler& operator=(const ClosureAndGotoReqHandler& ) = delete;
    ClosureAndGotoReqHandler& operator=(ClosureAndGotoReqHandler&& ) noexcept = delete;

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

template<typename T, typename A = std::allocator<T>>
class Parser final {
public:
    using SymbolType = atom::cfg::grammar::Symbol;
    using ProductionRulesType = atom::cfg::grammar::ProductionRules;
    using TokenToTerminalFuncType = atom::cfg::grammar::Terminal(*)(const T&);

    explicit Parser(ProductionRulesType&& prodRules, TokenToTerminalFuncType tokenToTerminal);
    std::unique_ptr<atom::ast::Node> buildTree(std::span<const T> tokens);

private:
    using TokenItType = typename std::span<const T>::iterator;

    ProductionRulesType m_prodRules;
    TokenToTerminalFuncType m_tokenToTerminal;
    ClosureAndGotoReqHandler m_cgReqHandler;
};

template<typename T, typename A>
Parser<T, A>::Parser(ProductionRulesType&& prodRules, TokenToTerminalFuncType tokenToTerminal):
m_prodRules(std::move(prodRules)),
m_tokenToTerminal(tokenToTerminal),
m_cgReqHandler(m_prodRules)
{}

template<typename T, typename A>
std::unique_ptr<atom::ast::Node> Parser<T, A>::buildTree(const std::span<const T> tokens)
{
    using namespace grammar;
    if (tokens.empty()) {
        return std::unique_ptr<ast::Node>();
    }

    auto currentTokenIt = tokens.begin();
    std::list<std::unique_ptr<ast::Node>> nodeContext;
    std::list<Items> itemsContext;
    {
        auto& [leftS_, prodsS_] = *m_prodRules.begin();
        const auto& startItems = m_cgReqHandler.getClosure(Item{leftS_, *prodsS_.begin()});
        itemsContext.push_back(startItems);
    }

    do {
        const auto& currentItems = itemsContext.back();
        if (currentItems.size() == 1 && currentItems.begin()->isReducing()) {
            // reduce
            const auto& reducingItem = *currentItems.begin();
            const auto& reducingProduction = reducingItem.getProduction();
            auto subRoot = std::make_unique<ast::Node>(nullptr, reducingItem.getLeft());
            for (auto it = reducingProduction.crbegin(); it != reducingProduction.crend(); ++it) {
                const auto& reducingDerivation = *it;
                if (reducingDerivation == nodeContext.back()->getSymbol()) {
                    auto node = std::move(nodeContext.back());
                    node->setParent(subRoot.get());
                    subRoot->addChild(std::move(node));
                    nodeContext.pop_back();
                } else {
                    assert(false);
                }
            }
            nodeContext.push_back(std::move(subRoot));
            const auto& lastSymbol = nodeContext.back()->getSymbol();
            auto stop = false;
            do {
                itemsContext.pop_back();
                const auto& lastItems = itemsContext.back();
                for (const auto& item : lastItems) {
                    if (item.getCurrentDerivation() == lastSymbol) {
                        stop = true;
                        break;
                    }
                }
            } while (!stop);
            const auto& newItems = m_cgReqHandler.getGoto(itemsContext.back(), nodeContext.back()->getSymbol());
            itemsContext.push_back(newItems);
        } else {
            //shift
            ASSERTION(currentTokenIt != tokens.end(), BadLR, "")
            const auto currentSymbol = *currentTokenIt;
            try {
                const auto& newItems = m_cgReqHandler.getGoto(currentItems, m_tokenToTerminal(currentSymbol));
                // chec newItems
                itemsContext.push_back(newItems);

                nodeContext.push_back(
                    std::make_unique<ast::Leaf<T>>(nullptr, m_tokenToTerminal(currentSymbol), currentSymbol)
                );
                ++currentTokenIt;
            } catch (...) {
                const auto& newItems = m_cgReqHandler.getGoto(currentItems, None);
                itemsContext.push_back(newItems);
                nodeContext.push_back(
                    std::make_unique<ast::Node>(nullptr, None)
                );
            }
        }
    } while (!nodeContext.empty() && nodeContext.back()->getSymbol() != S);
    return std::move(nodeContext.back());
}

} //! namespace atom::cfg::lr

#endif //! CFG_LR_H
