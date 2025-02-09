#include "include/cfg/node.h"

#include <sstream>

namespace {

using namespace atom::ast;

void Dump(std::ostream& os, const Node *const node, int depth) {
    for (int i = 0; i < depth; ++i) {
        os << "    ";
    }

    os << "|---- " << node->getSymbol() << std::endl;
    for (const auto& child : node->getChilds()) {
        Dump(os, child.get(), depth + 1);
    }
}

} //! namespace

namespace atom::ast {

Node::Node(Node* parent, const SymbolType& symbol):
m_parent(parent),
m_symbol(symbol),
m_childs() {}

const Node::SymbolType& Node::getSymbol() const { return m_symbol; }
const Node::SymbolType& Node::getSymbol() { return m_symbol; }

Node* const Node::getParent() const { return m_parent; }
Node* const Node::getParent() { return m_parent; }

void Node::addChild(std::unique_ptr<Node>&& child) {
    m_childs.push_back(std::move(child));
}

void Node::addChilds(ChildsType&& childs) {
    for (auto& child : childs) {
        m_childs.push_back(std::move(child));
    }
}

void Node::removeChilds() {
    m_childs.clear();
}

void Node::setParent(Node* parent) {
    assert(this != parent);
    m_parent = parent;
}

const Node::ChildsType& Node::getChilds() const { return m_childs; }
const Node::ChildsType& Node::getChilds() { return m_childs; }

std::ostream& Node::operator<<(std::ostream& os) const
{
    Dump(os, this, 0);
    return os;
}

std::ostream& operator<<(std::ostream& os, const Node& node) {
    return node.operator<<(os);
}

bool operator==(const Node& lhs, const Node& rhs) {
    //! I think there is better way for comparing two nodes.
    std::stringstream ss1, ss2;
    ss1 << lhs;
    ss2 << rhs;
    return ss1.str() == ss2.str();
}

} //! namespace atom::ast
