#pragma once

#include "grammar.h"

#include <ostream>
#include <vector>
#include <memory>
#include <cassert>

namespace atom::ast {

class Node {
public:
    using ChildsType = std::vector<std::unique_ptr<Node>>;
    using SymbolType = atom::cfg::grammar::Symbol;

    explicit Node(Node* parent = nullptr, const SymbolType& symbol = atom::cfg::grammar::None);

    Node(Node&& ) noexcept = delete;
    Node(const Node& ) = delete;
    Node& operator=(Node&& ) noexcept = delete;
    Node& operator=(const Node& )= delete;

    const SymbolType& getSymbol() const;
    const SymbolType& getSymbol();

    Node* const getParent() const;
    Node* const getParent();

    void addChild(std::unique_ptr<Node>&& child);
    void addChilds(ChildsType&& childs);

    void removeChilds();
    void setParent(Node* parent);

    const ChildsType& getChilds() const;
    const ChildsType& getChilds();

    std::ostream& operator<<(std::ostream& os) const;

private:

    Node* m_parent;
    SymbolType m_symbol;
    ChildsType m_childs;
};

template<typename T>
class Leaf final : public Node {
public:
    using SymbolType = atom::cfg::grammar::Symbol;
    using TerminalType = atom::cfg::grammar::Terminal;
    using TokenType = T;
    explicit Leaf(Node* parent, const TerminalType& symbol, const T& token): Node(parent, symbol), m_token(token) {}

    const T& getToken() const { return m_token; }
    const T& getToken() { return m_token; }

private:
    T m_token;
};


std::ostream& operator<<(std::ostream& os, const Node& node);

} //! namespace atom::ast
