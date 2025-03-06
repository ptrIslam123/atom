#ifndef CFG_LL_H
#define CFG_LL_H

#include "include/cfg/grammar.h"
#include "include/cfg/node.h"

#include "include/utils/owner.h"
#include "include/utils/assertion.h"

#include "include/memory/allocators/default_allocator.h"
#include "include/memory/smart_pointers/shared_ptr.h"

#include <functional>
#include <exception>
#include <unordered_set>
#include <span>
#include <vector>
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

    template<typename T>
    std::span<const T>::iterator findStartTokenForDerivation(std::span<const T> tokens, const DerivationType& derivation);

private:
    SymbolSetType makeFirst(const SymbolType& symbol);
    SymbolSetType makeFollow(const SymbolType& symbol);

    ProductionRulesType& m_prodRules;
    std::unordered_map<SymbolType, SymbolSetType> m_firstCacheTable;
    std::unordered_map<SymbolType, SymbolSetType> m_followCacheTable;
};

bool operator==(const FirstAndFollowReqHandler::SymbolSetType& l, const FirstAndFollowReqHandler::SymbolSetType& r);

template<typename T, typename A = memory::allocator::DefaultAllocator>
class Parser final {
public:
    using ProductionRulesType = atom::cfg::grammar::ProductionRules;
    using TokenType = T;
    using TokensType = std::span<const TokenType>;
    using AllocatorType = A;
    using NodeType = atom::ast::Node;
    using NodePtrType = atom::memory::NonAtomicSharedPtr<NodeType, AllocatorType>;

    explicit Parser(ProductionRulesType&& prodRules);
    NodePtrType buildTree(std::span<const T> tokens);

private:
    using ProductionsType = grammar::Productions;
    using ProductionType = grammar::Production;
    using SymbolType = grammar::Symbol;
    using TerminalType = grammar::Terminal;
    using NonTerminalType = grammar::NonTerminal;
    using TokenIndexType = TokensType::size_type;
    using StackType = std::vector<std::pair<SymbolType, NodePtrType>>;

    struct Context {
        NodePtrType currentRootNode;
        std::reference_wrapper<const ProductionsType> prods;
        ProductionsType::ConstIteratorType currentProd;
        StackType stack;
        TokensType tokens;
    };

    const ProductionType& chooseFitProduction(const ProductionsType& productions, NodePtrType currentRootNode);

    void handleTerminal(const TerminalType& symbol, NodePtrType currentRootNode);
    void handleNonTerminal(const NonTerminalType& symbol, NodePtrType currentRootNode);
    void backtrack();

    NodePtrType allocateNode(NodePtrType parent, const SymbolType& symbol);
    NodePtrType allocateLeaf(NodePtrType parent, const TerminalType& symbol, const T& token);

    ProductionRulesType m_prodRules;
    FirstAndFollowReqHandler m_ffRequestHandler;

    std::vector<Context> m_context;
    StackType m_stack;
    TokensType m_tokens;
    bool m_isFirstCall;
};

template<typename T, typename A>
Parser<T, A>::Parser(ProductionRulesType&& prodRules):
m_prodRules(std::move(prodRules)),
m_ffRequestHandler(m_prodRules),
m_stack(),
m_tokens()
{}

template<typename T, typename A>
const Parser<T, A>::ProductionType& Parser<T, A>::chooseFitProduction(const ProductionsType& productions, NodePtrType currentRootNode) {
    using namespace grammar;
    assert(!productions.isEmpty());
    if (productions.size() == 1) {
        return *productions.cbegin();
    } else {
        m_context.push_back(Context{currentRootNode, productions, productions.cbegin(), m_stack, m_tokens});
        return *productions.cbegin();
    }
}

template<typename T, typename A>
void Parser<T, A>::backtrack() {
    using namespace grammar;
    if (m_context.empty()) {
        ASSERTION(false, BadLL, "")
    }

    Context& context = m_context.back();
    m_tokens = std::move(context.tokens);
    m_stack = std::move(context.stack);
    context.currentRootNode->removeChilds();

    auto prodIt = ++context.currentProd;
    if (prodIt != context.prods.get().cend()) {
        Context newContext{context.currentRootNode, context.prods, prodIt, m_stack, m_tokens};
        m_context.pop_back();
        m_context.push_back(std::move(newContext));

        const Production& prod = *prodIt;
        for (auto it = prod.crbegin(); it != prod.crend(); ++it) {
            const Production::DerivationType& derivation = *it;
            m_stack.push_back({static_cast<SymbolType>(derivation), context.currentRootNode});
        }
    } else {
        m_context.pop_back();
        backtrack();
    }
}

template<typename T, typename A>
void Parser<T, A>::handleTerminal(const TerminalType& symbol, NodePtrType currentRootNode) {
    using namespace ast;
    using namespace grammar;

    if (symbol == None) {
        currentRootNode->addChild(allocateNode(currentRootNode, None));
        return;
    }

    auto tokenIt = m_tokens.begin();
    if (tokenIt == m_tokens.end()) {
        backtrack();
        return;
    }

    const auto& token = *tokenIt;
    auto termianl = TokenToTerminal<T>(token);
    if (symbol != termianl) {
        backtrack();
        return;
    }

    auto terminalNode = allocateLeaf(currentRootNode, termianl, token);
    currentRootNode->addChild(terminalNode);
    m_tokens = m_tokens.subspan(1);
}

template<typename T, typename A>
void Parser<T, A>::handleNonTerminal(const NonTerminalType& symbol, NodePtrType currentRootNode) {
    using namespace ast;
    using namespace grammar;

    auto it = m_prodRules.find(symbol);
    ASSERTION(it != m_prodRules.cend(), BadLL, "")

    if (!m_isFirstCall) {
        auto nonTerminalNode = allocateNode(currentRootNode, symbol);
        currentRootNode->addChild(nonTerminalNode);
        currentRootNode = nonTerminalNode;
    }
    m_isFirstCall = false;

    const auto& [_, prods] = *it;
    const Production& prod = chooseFitProduction(prods, currentRootNode);
    for (auto it = prod.crbegin(); it != prod.crend(); ++it) {
        const Production::DerivationType& derivation = *it;
        m_stack.push_back({static_cast<SymbolType>(derivation), currentRootNode});
    }
}

template<typename T, typename A>
Parser<T, A>::NodePtrType Parser<T, A>::buildTree(std::span<const T> tokens) {
    using namespace ast;
    using namespace grammar;

    m_isFirstCall = true;
    m_tokens = tokens;
    m_stack.clear();

    auto rootNode = allocateNode(NodePtrType{}, m_prodRules.begin()->first);
    m_stack.push_back({rootNode->getSymbol(), rootNode});

    for (;;) {
        if (m_stack.empty() && m_tokens.empty()) {
            break;
        } else if (m_stack.empty() && !m_tokens.empty()) {
            backtrack();
        } else if (!m_stack.empty()) {
            const auto [currentSymbol, currentRootNode] = m_stack.back();
            m_stack.pop_back();

            if (!currentSymbol.isTerminal()) {
                handleNonTerminal(NonTerminal{currentSymbol}, currentRootNode);
            } else {
                handleTerminal(Terminal{currentSymbol}, currentRootNode);
            }
        } else {
            ASSERTION(false, BadLL, "")
        }
    }
    return rootNode;
}

template<typename T, typename A>
Parser<T, A>::NodePtrType Parser<T, A>::allocateNode(NodePtrType parent, const SymbolType& symbol) {
    return NodePtrType::Make(parent, symbol);
}

template<typename T, typename A>
Parser<T, A>::NodePtrType Parser<T, A>::allocateLeaf(NodePtrType parent, const TerminalType& symbol, const T& token) {
    return memory::NonAtomicSharedPtr<ast::Leaf<T>>::Make(parent, symbol, token);
}

} //! namespace atom::cfg::ll

#endif //! CFG_LL_H
