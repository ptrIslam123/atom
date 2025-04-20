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
    using ParenPtrType = atom::memory::NonAtomicWeakPtr<Node>;
    using ChildsType = std::vector<NodePtrType>;
    using SymbolType = atom::cfg::grammar::Symbol;

    explicit Node(ParenPtrType parent, const SymbolType& symbol = atom::cfg::grammar::None):
    m_parent(parent),
    m_symbol(symbol),
    m_childs()
    {}
    virtual ~Node() {
        //removeChilds();
    }

    Node(Node&& ) noexcept = delete;
    Node(const Node& ) = delete;
    Node& operator=(Node&& ) noexcept = delete;
    Node& operator=(const Node& )= delete;

    const SymbolType& getSymbol() const { return m_symbol; }
    const SymbolType& getSymbol() { return m_symbol; }

    NodePtrType const getParent() const { return m_parent.tryLock(); }
    NodePtrType const getParent() { return m_parent.tryLock(); }

    void addChild(NodePtrType child) {
        m_childs.push_back(std::move(child));
    }
    void addChilds(ChildsType&& childs) {
        for (auto& child : childs) {
            addChild(child);
        }
    }

    void removeChilds() {
        m_childs.clear();
    }
    void setParent(ParenPtrType parent) {
        m_parent = parent;
    }

    const ChildsType& getChilds() const { return m_childs; }
    const ChildsType& getChilds() { return m_childs; }

    std::ostream& operator<<(std::ostream& os) const;

private:
    ParenPtrType m_parent;
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
    ~Leaf() = default;

    const T& getToken() const { return m_token; }
    const T& getToken() { return m_token; }

private:
    T m_token;
};

std::ostream& operator<<(std::ostream& os, const Node& node);
bool operator==(const Node& lhs, const Node& rhs);

} //! namespace atom::ast
