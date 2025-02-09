#ifndef CFG_LR_H
#define CFG_LR_H

#include "include/cfg/grammar.h"
#include "include/cfg/node.h"
#include "include/utils/assertion.h"

#include <functional>
#include <ostream>
#include <sstream>
#include <memory>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <cassert>

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
    using RequestResultType = std::optional<std::reference_wrapper<Items>>;

    explicit ClosureAndGotoReqHandler(ProductionRulesType& prodRules);
    ClosureAndGotoReqHandler(const ClosureAndGotoReqHandler& ) = delete;
    ClosureAndGotoReqHandler(ClosureAndGotoReqHandler&& ) noexcept = delete;
    ClosureAndGotoReqHandler& operator=(const ClosureAndGotoReqHandler& ) = delete;
    ClosureAndGotoReqHandler& operator=(ClosureAndGotoReqHandler&& ) noexcept = delete;

    RequestResultType requestClosure(const Items& items);
    RequestResultType requestClosure(const Item& item);
    RequestResultType requestGoto(const Items& items, const SymbolType& symbol);

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

    /*
     * @brief GrammarAugmentation - Процесс преобразования исходной грамматики S→… в грамматику S′→S, где S′ является новым стартовым символом,
     * называется "аугментацией грамматики"  (Grammar Augmentation).
     */
    static ProductionRulesType GrammarAugmentation(ProductionRulesType&& initialProdRules);

    const Items& makeInitialItems();
    void shiftSymbol(const Items& currentItems, grammar::Symbol&& currentSymbol);
    void shift(const Items& currentItems);
    void reduce(const Item& reducingItem);

    ProductionRulesType m_prodRules;
    TokenToTerminalFuncType m_tokenToTerminal;
    ClosureAndGotoReqHandler m_cgReqHandler;
    std::span<const T> m_tokens;
    std::span<const T>::size_type m_currentTokenIndex;
    std::vector<std::unique_ptr<ast::Node>> m_nodeContext;
    std::vector<Items> m_itemsContext;
};

template<typename T, typename A>
Parser<T, A>::Parser(ProductionRulesType&& prodRules, TokenToTerminalFuncType tokenToTerminal):
m_prodRules(GrammarAugmentation(std::move(prodRules))),
m_tokenToTerminal(tokenToTerminal),
m_cgReqHandler(m_prodRules),
m_tokens(),
m_currentTokenIndex(0),
m_nodeContext(),
m_itemsContext()
{}

template<typename T, typename A>
const Items& Parser<T, A>::makeInitialItems() {
    auto& [leftS_, prodsS_] = *m_prodRules.begin();
    auto result = m_cgReqHandler.requestClosure(Item{leftS_, *prodsS_.begin()});
    ASSERTION(result.has_value(), BadLR, "")
    return *result;
}

template<typename T, typename A>
void Parser<T, A>::shiftSymbol(const Items& currentItems, grammar::Symbol&& currentSymbol) {
    using namespace grammar;
    using namespace ast;
    // update states context(add the new state)
    {
        ClosureAndGotoReqHandler::RequestResultType result;
        for (auto i = 0; i < 2; ++i) {
            result = m_cgReqHandler.requestGoto(currentItems, currentSymbol);
            if (result.has_value()) {
                break;
            } else {
                // we try to use e item in case the first goto request failed as default!
                currentSymbol = None;
            }
        }
        ASSERTION(result.has_value(), BadLR, "")
        m_itemsContext.push_back(*result);
    }
    // update symbols context(add the new symbol or e)
    {
        if (currentSymbol != None) {
            auto newNode = std::make_unique<Leaf<T>>(nullptr, Terminal{currentSymbol}, *(m_tokens.begin() + m_currentTokenIndex));
            m_nodeContext.push_back(std::move(newNode));
            ++m_currentTokenIndex;
        } else {
            auto newNode = std::make_unique<Node>(nullptr, None);
            m_nodeContext.push_back(std::move(newNode));
        }
    }
}

template<typename T, typename A>
void Parser<T, A>::shift(const Items& currentItems) {
    using namespace grammar;
    auto currentTokenIt = m_tokens.begin() + m_currentTokenIndex;
    if (currentTokenIt != m_tokens.end()) {
        shiftSymbol(currentItems, m_tokenToTerminal(*currentTokenIt));
    } else {
        shiftSymbol(currentItems, Symbol{None});
    }
}

template<typename T, typename A>
void Parser<T, A>::reduce(const Item& reducingItem) {
    using namespace grammar;
    using namespace ast;
    const auto& reducingProduction = reducingItem.getProduction();
    ASSERTION(reducingProduction.size() <= m_nodeContext.size(), BadLR, "")
    ASSERTION(reducingProduction.size() <= m_itemsContext.size(), BadLR, "")

    auto newRootNode = std::make_unique<Node>(nullptr, reducingItem.getLeft());
    for (auto it = reducingProduction.crbegin(); it != reducingProduction.crend(); ++it) {
        auto node = std::move(m_nodeContext.back());
        ASSERTION(node->getSymbol() == *it, BadLR, "")

        node->setParent(newRootNode.get());
        newRootNode->addChild(std::move(node));

        m_nodeContext.pop_back();
        m_itemsContext.pop_back();
    }
    m_nodeContext.push_back(std::move(newRootNode));

    ASSERTION(!m_itemsContext.empty(), BadLR, "")
    auto result = m_cgReqHandler.requestGoto(m_itemsContext.back(), m_nodeContext.back()->getSymbol());
    ASSERTION(result.has_value(), BadLR, "")
    m_itemsContext.push_back(*result);
}

template<typename T, typename A>
std::unique_ptr<atom::ast::Node> Parser<T, A>::buildTree(const std::span<const T> tokens)
{
    using namespace grammar;
    m_nodeContext.clear();
    m_itemsContext.clear();
    m_tokens = tokens;
    m_currentTokenIndex = 0;
    m_itemsContext.push_back(makeInitialItems());

    do {
        const auto& currentItems = m_itemsContext.back();
        if (currentItems.size() == 1 && currentItems.begin()->isReducing()) {
            reduce(*currentItems.begin());
        } else {
            shift(currentItems);
        }
    } while (!m_nodeContext.empty() && m_nodeContext.back()->getSymbol() != S);
    return std::move(m_nodeContext.back());
}

template<typename T, typename A>
Parser<T, A>::ProductionRulesType Parser<T, A>::GrammarAugmentation(ProductionRulesType&& initialProdRules) {
    using namespace grammar;
    ProductionRulesType newProdRules;
    newProdRules.newProduction(S_) >> initialProdRules.begin()->first;
    for (auto&& [left, prods] : initialProdRules) {
        newProdRules.pushBack(std::move(left), std::move(prods));
    }
    return newProdRules;
}

} //! namespace atom::cfg::lr

#endif //! CFG_LR_H
