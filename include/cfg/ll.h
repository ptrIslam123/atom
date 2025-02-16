#ifndef CFG_LL_H
#define CFG_LL_H

#include "include/cfg/grammar.h"
#include "include/cfg/node.h"
#include "include/utils/assertion.h"

#include <functional>
#include <exception>
#include <unordered_set>
#include <span>
#include <vector>
#include <memory>
#include <cassert>

namespace atom::cfg::ll {

class BadLL final : public std::exception {
public:
    explicit BadLL(std::string_view msg);
    virtual const char* what() const noexcept;

private:
    std::string m_msg;
};

class FirstAndFollowReqHandler final {
public:
    using ProductionRulesType = atom::cfg::grammar::ProductionRules;
    using ProductionsType = atom::cfg::grammar::Productions;
    using ProductionType = atom::cfg::grammar::Production;
    using DerivationType = ProductionRulesType::DerivationType;
    using SymbolType = atom::cfg::grammar::Symbol;
    using SymbolSetType = std::unordered_set<SymbolType>;

    explicit FirstAndFollowReqHandler(ProductionRulesType& prodRules);

    const SymbolSetType& requestFirst(const SymbolType& symbol);
    const SymbolSetType& requestFollow(const SymbolType& symbol);

private:
    SymbolSetType makeFirst(const SymbolType& symbol);
    SymbolSetType makeFollow(const SymbolType& symbol);

    ProductionRulesType& m_prodRules;
    std::unordered_map<SymbolType, SymbolSetType> m_firstCacheTable;
    std::unordered_map<SymbolType, SymbolSetType> m_followCacheTable;
};

bool operator==(const FirstAndFollowReqHandler::SymbolSetType& l, const FirstAndFollowReqHandler::SymbolSetType& r);

template<typename T, typename A = std::allocator<T>>
class Parser final {
public:
    using ProductionRulesType = atom::cfg::grammar::ProductionRules;

    explicit Parser(ProductionRulesType&& prodRules);
    std::unique_ptr<atom::ast::Node> buildTree(std::span<const T> tokens);

private:
    using ProductionsType = grammar::Productions;
    using ProductionType = grammar::Production;
    using SymbolType = grammar::Symbol;
    using TerminalType = grammar::Terminal;
    using NonTerminalType = grammar::NonTerminal;

    void handleTerminal(const TerminalType& symbol, ast::Node* currentRootNode);
    void handleNonTerminal(const NonTerminalType& symbol, ast::Node* currentRootNode);
    const ProductionType& chooseFitProduction(const ProductionsType& productions);

    ProductionRulesType m_prodRules;
    FirstAndFollowReqHandler m_ffRequestHandler;
    std::vector<std::pair<grammar::Symbol, ast::Node*>> m_context;
    std::span<const T> m_tokens;
    std::span<const T>::size_type m_currentTokenIndex;
    bool m_isFirstCall;
};

template<typename T, typename A>
Parser<T, A>::Parser(ProductionRulesType&& prodRules):
m_prodRules(std::move(prodRules)),
m_ffRequestHandler(m_prodRules),
m_context(),
m_tokens(),
m_currentTokenIndex(0)
{}

template<typename T, typename A>
const Parser<T, A>::ProductionType& Parser<T, A>::chooseFitProduction(const ProductionsType& productions) {
    using namespace grammar;
    std::size_t maxProductionSize = 0;
    Productions::ConstIteratorType defaultProduction = productions.cend();
    std::vector<Productions::ConstIteratorType> fitProductions;
    fitProductions.reserve(productions.size());
    for (auto it = productions.cbegin(); it != productions.cend(); ++it) {
        const Production& production = *it;
        if (production.size() == 1 && *production.cbegin() == None && defaultProduction == productions.cend()) {
            defaultProduction = it;
        }
        maxProductionSize = std::max(maxProductionSize, production.size());
        fitProductions.push_back(it);
    }

    auto offset = 0;
    for (; fitProductions.size() > 1 && offset < maxProductionSize; ++offset) {
        for (auto it = fitProductions.cbegin(); it != fitProductions.cend(); ) {
            const Production& production = **it;
            auto derivationIt = production.cbegin() + offset;
            {
                const auto n = std::distance(derivationIt, production.cend());
                if (n == 0 || n < 0) {
                    ++it;
                    continue;
                }
            }
            const auto& derivation = *derivationIt;
            auto tokenIt = m_tokens.begin() + m_currentTokenIndex + offset;
            if (tokenIt != m_tokens.end()) {
                const auto& first = m_ffRequestHandler.requestFirst(derivation);
                ASSERTION(!first.empty(), BadLL, "")
                if (!first.contains(TokenToTerminal<T>(*tokenIt))) {
                    fitProductions.erase(it);
                } else {
                    ++it;
                }
            } else {
                const auto& first = m_ffRequestHandler.requestFirst(derivation);
                const auto& follow = m_ffRequestHandler.requestFollow(derivation);
                ASSERTION(!first.empty() || !follow.empty(), BadLL, "")
                if (!first.contains(None) || !follow.contains(End)) {
                    fitProductions.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }

    ASSERTION(!fitProductions.empty() || defaultProduction != productions.cend(), BadLL, "")
    if (fitProductions.empty() && defaultProduction != productions.cend()) {
        fitProductions.push_back(defaultProduction);
    }

    int maxProdIndex = 0;
    int maxProdSize = fitProductions[0]->size();
    for (auto i = 1; i < fitProductions.size(); ++i) {
        const Production& currentProd = *fitProductions[i];
        if (currentProd.size() > maxProdSize) {
            maxProdSize = currentProd.size();
            maxProdIndex = i;
        }
    }
    return *fitProductions[maxProdIndex];
}

template<typename T, typename A>
void Parser<T, A>::handleTerminal(const TerminalType& symbol, ast::Node* currentRootNode) {
    using namespace ast;
    using namespace grammar;

    ASSERTION(m_currentTokenIndex <= m_tokens.size(), BadLL, "")
    auto tokenIt = m_tokens.begin() + m_currentTokenIndex;
    if (symbol != None && tokenIt != m_tokens.end()) {
        const auto& token = *tokenIt;
        auto termianl = TokenToTerminal<T>(token);
        ASSERTION(symbol == termianl, BadLL, "")

        auto terminalNode = std::make_unique<Leaf<T>>(currentRootNode, termianl, token);
        currentRootNode->addChild(std::move(terminalNode));
        ++m_currentTokenIndex;
    } else {
        //ASSERTION(symbol == None && m_ffRequestHandler.requestFollow(symbol).contains(End), BadLL, "")
        currentRootNode->addChild(std::make_unique<Node>(currentRootNode, None));
    }
}

template<typename T, typename A>
void Parser<T, A>::handleNonTerminal(const NonTerminalType& symbol, ast::Node* currentRootNode) {
    using namespace ast;
    using namespace grammar;

    auto it = m_prodRules.find(symbol);
    ASSERTION(it != m_prodRules.cend(), BadLL, "")

    if (!m_isFirstCall) {
        auto nonTerminalNode = std::make_unique<Node>(currentRootNode, symbol);
        auto newRootNode = nonTerminalNode.get();
        currentRootNode->addChild(std::move(nonTerminalNode));
        currentRootNode = newRootNode;
    }
    m_isFirstCall = false;

    const auto& [_, prods] = *it;
    const Production& prod = chooseFitProduction(prods);
    for (auto it = prod.crbegin(); it != prod.crend(); ++it) {
        const Production::DerivationType& derivation = *it;
        m_context.push_back({static_cast<SymbolType>(derivation), currentRootNode});
    }
}

template<typename T, typename A>
std::unique_ptr<atom::ast::Node> Parser<T, A>::buildTree(std::span<const T> tokens) {
    using namespace ast;
    using namespace grammar;

    m_isFirstCall = true;
    m_tokens = tokens;
    m_currentTokenIndex = 0;
    m_context.clear();

    auto rootNode = std::make_unique<ast::Node>(nullptr, m_prodRules.begin()->first);
    m_context.push_back({rootNode->getSymbol(), rootNode.get()});

    do {
        const auto[currentSymbol, currentRootNode] = m_context.back();
        m_context.pop_back();

        if (!currentSymbol.isTerminal()) {
            handleNonTerminal(NonTerminal{currentSymbol}, currentRootNode);
        } else {
            handleTerminal(Terminal{currentSymbol}, currentRootNode);
        }
    } while (!m_context.empty());

    ASSERTION(m_tokens.begin() + m_currentTokenIndex == m_tokens.end(), BadLL, "")
    return rootNode;
}

} //! namespace atom::cfg::ll

#endif //! CFG_LL_H
