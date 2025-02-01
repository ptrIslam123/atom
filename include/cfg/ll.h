#ifndef CFG_LL_H
#define CFG_LL_H

#include "include/cfg/grammar.h"
#include "include/cfg/node.h"
#include "include/utils/assertion.h"

#include <algorithm>
#include <unordered_set>
#include <span>
#include <list>
#include <memory>
#include <cassert>

namespace atom::cfg::ll {

class FirstAndFollowReqHandler final {
public:
    using ProductionRulesType = atom::cfg::grammar::ProductionRules;
    using ProductionsType = atom::cfg::grammar::Productions;
    using ProductionType = atom::cfg::grammar::Production;
    using DerivationType = ProductionRulesType::DerivationType;
    using SymbolType = atom::cfg::grammar::Symbol;
    using SymbolSetType = std::unordered_set<SymbolType>;

    explicit FirstAndFollowReqHandler(ProductionRulesType& prodRules);

    const SymbolSetType& getFirst(const SymbolType& symbol);
    const SymbolSetType& getFollow(const SymbolType& symbol);

private:
    SymbolSetType makeFirst(const SymbolType& symbol);
    SymbolSetType makeFollow(const SymbolType& symbol);

    ProductionRulesType& m_prodRules;
    std::unordered_map<SymbolType, SymbolSetType> m_firstCacheTable;
    std::unordered_map<SymbolType, SymbolSetType> m_followCacheTable;
};

bool operator==(const FirstAndFollowReqHandler::SymbolSetType& l, const FirstAndFollowReqHandler::SymbolSetType& r);

template<typename T>
class Parser final {
public:
    using ProductionRulesType = atom::cfg::grammar::ProductionRules;
    using TokenToTerminalType = atom::cfg::grammar::Terminal(*)(const T&);

    explicit Parser(ProductionRulesType&& prodRules, TokenToTerminalType tokenToTerminal):
    m_tokenToTerminal(tokenToTerminal),
    m_prodRules(prodRules),
    m_ffReqHandler(m_prodRules) {}

    std::unique_ptr<atom::ast::Node> buildTree(std::span<const T> tokens) {
        using namespace atom::cfg::grammar;
        using namespace atom::ast;

        m_tokens = tokens;
        m_tokenIndex = 0;
        m_isFirstCall = true;

        ASSERTION(m_prodRules.cbegin() != m_prodRules.cend(), std::runtime_error, "Invalid grammar: Empty Productions Rules")

        auto rootNode = std::make_unique<ast::Node>(nullptr, static_cast<Symbol>(m_prodRules.begin()->first));
        m_context.push_back(std::make_pair(rootNode->getSymbol(), rootNode.get()));
        
        while (!m_context.empty()) {
            const auto[symbol, currentRootNode] = m_context.back();
            m_context.pop_back();

            if (!symbol.isTerminal()) {
                handleNonTerminal(currentRootNode, NonTerminal{ symbol.getData() });
            } else {
                handleTerminal(currentRootNode, Terminal{ symbol.getData() });
            }
        }

        return std::move(rootNode);
    }

private:
    void handleNonTerminal(ast::Node* currentRootNode, const grammar::NonTerminal& symbol) {
        using namespace atom::cfg::grammar;
        using namespace atom::ast;

        if (!m_isFirstCall) {
            auto nonTerminalNode = std::make_unique<Node>(currentRootNode, symbol);
            auto newRootNode = nonTerminalNode.get();
            currentRootNode->addChild(std::move(nonTerminalNode));
            currentRootNode = newRootNode;
        }
        m_isFirstCall = false;

        std::vector<Production> fitProductions;
        auto it = m_prodRules.find(NonTerminal{ symbol.getData() });
        ASSERTION(it != m_prodRules.end(), std::runtime_error, "")
        const Productions& productions = it->second;

        for (auto prodIt = productions.cbegin(); prodIt != productions.cend(); ++prodIt) {
            auto isProductionFit = true;
            const auto& production = *prodIt;
            for (auto i = 0; i < production.size(); ++i) {

                auto tokenIt = m_tokens.begin() + m_tokenIndex + i;
                const auto& terminal = (tokenIt != m_tokens.end() ? m_tokenToTerminal(*tokenIt) : End);
                const auto& derivation = *(production.cbegin() + i);
                const auto& fset = (derivation != None ? m_ffReqHandler.getFirst(derivation) : m_ffReqHandler.getFollow(symbol));
                if (!fset.contains(terminal)) {
                    isProductionFit = false;
                    break;
                }


            }

            if (isProductionFit) {
                ASSERTION(fitProductions.empty(), std::runtime_error, "")
                fitProductions.push_back(production);
            }
        }

        ASSERTION(!fitProductions.empty(), std::runtime_error, "")

        const Production& fitProduction = *fitProductions.cbegin();
        const auto& derivations = fitProduction.getDerivations();
        for (auto derivIt = derivations.crbegin(); derivIt != derivations.crend(); ++derivIt) {
            m_context.push_back(std::make_pair(static_cast<Symbol>(*derivIt), currentRootNode));
        }
    }

    void handleTerminal(ast::Node* currentRootNode, const grammar::Terminal& symbol) {
        using namespace atom::cfg::grammar;
        using namespace atom::ast;
        if (symbol != None) {
            auto tokenIt = m_tokens.begin() + m_tokenIndex;
            ASSERTION(tokenIt != m_tokens.end(), std::runtime_error, "")

            const auto terminal = m_tokenToTerminal(*tokenIt);
            ASSERTION(terminal == symbol, std::runtime_error, "")

            const auto& token = m_tokens[++m_tokenIndex];
            auto terminalNode = std::make_unique<Leaf<T>>(currentRootNode, terminal, token);
            currentRootNode->addChild(std::move(terminalNode));
        } else {
            auto terminalNode = std::make_unique<Node>(currentRootNode, None);
            currentRootNode->addChild(std::move(terminalNode));
        }
    }

    TokenToTerminalType m_tokenToTerminal;
    ProductionRulesType m_prodRules;
    FirstAndFollowReqHandler m_ffReqHandler;
    std::list<std::pair<grammar::Symbol, ast::Node*>> m_context;
    std::span<const T> m_tokens;
    int m_tokenIndex;
    bool m_isFirstCall;
};

} //! namespace atom::cfg::ll

#endif //! CFG_LL_H
