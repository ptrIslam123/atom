#pragma once

#include "include/cfg/grammar.h"
#include "include/memory/smart_pointers/shared_ptr.h"

#include <ostream>
#include <vector>
#include <cassert>

namespace atom::ast {

class Node {
public:
    using NodePtrType = atom::memory::NonAtomicSharedPtr<Node>;
    using ChildsType = std::vector<NodePtrType>;
    using SymbolType = atom::cfg::grammar::Symbol;

    explicit Node(NodePtrType parent = NodePtrType{nullptr}, const SymbolType& symbol = atom::cfg::grammar::None);

    Node(Node&& ) noexcept = delete;
    Node(const Node& ) = delete;
    Node& operator=(Node&& ) noexcept = delete;
    Node& operator=(const Node& )= delete;

    const SymbolType& getSymbol() const;
    const SymbolType& getSymbol();

    NodePtrType const getParent() const;
    NodePtrType const getParent();

    void addChild(NodePtrType child);
    void addChilds(ChildsType&& childs);

    void removeChilds();
    void setParent(NodePtrType parent);

    const ChildsType& getChilds() const;
    const ChildsType& getChilds();

    std::ostream& operator<<(std::ostream& os) const;

private:

    NodePtrType m_parent;
    SymbolType m_symbol;
    ChildsType m_childs;
};

template<typename T>
class Leaf final : public Node {
public:
    using NodePtrType = Node::NodePtrType;
    using SymbolType = Node::SymbolType;
    using TerminalType = atom::cfg::grammar::Terminal;
    using TokenType = T;

    explicit Leaf(NodePtrType parent, const TerminalType& symbol, const T& token): Node(parent, symbol), m_token(token) {}

    const T& getToken() const { return m_token; }
    const T& getToken() { return m_token; }

private:
    T m_token;
};

std::ostream& operator<<(std::ostream& os, const Node& node);
bool operator==(const Node& lhs, const Node& rhs);

} //! namespace atom::ast
