#pragma once

#include "grammar.h"

#include <string>
#include <vector>
#include <memory>
#include <cassert>

namespace xlt::ast {

class Node {
public:
    using ChildsType = std::vector<std::unique_ptr<Node>>;

    explicit Node(Node* parent = nullptr, const cfg::Symbol& symbol = cfg::None{}):
        m_parent(parent),
        m_symbol(symbol),
        m_childs() {}

    Node(Node&& ) noexcept = delete;
    Node(const Node& ) = delete;

    Node& operator=(Node&& ) noexcept = delete;
    Node& operator=(const Node& )= delete;

    const cfg::Symbol& getSymbol() const { return m_symbol; }
    const cfg::Symbol& getSymbol() { return m_symbol; }

    Node* const getParent() const { return m_parent; }
    Node* const getParent() { return m_parent; }

    void addChild(std::unique_ptr<Node>&& child) {
        m_childs.push_back(std::move(child));
    }

    void addChilds(ChildsType&& childs) {
        for (auto& child : childs) {
            m_childs.push_back(std::move(child));
        }
    }

    void removeChilds() {
        m_childs.clear();
    }

    void setParent(Node* parent) {
        assert(this != parent);
        m_parent = parent;
    }

    const ChildsType& getChilds() const { return m_childs; }
    const ChildsType& getChilds() { return m_childs; }

private:
    Node* m_parent{nullptr};
    cfg::Symbol m_symbol;
    ChildsType m_childs;
};

template<typename T>
class Leaf : public Node {
public:
    using TokenType = T;
    explicit Leaf(Node* parent, const T& token): Node(parent, cfg::Terminal{}), m_token(token) {}

    const T& getToken() const { return m_token; }
    const T& getToken() { return m_token; }

private:
    T m_token;
};

} //! namespace xlt::ast
