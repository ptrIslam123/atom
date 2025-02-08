#pragma once

#include <string>
#include <string_view>
#include <ostream>
#include <functional>
#include <ostream>
#include <vector>
#include <span>
#include <optional>
#include <cassert>

namespace atom::cfg::grammar {

/**
 * (Context-Free Grammar, CFG) = {Non-terminal Symbols, Terminal Symbols, Start Symbol, Production Rules}
 */
class Symbol {
public:
    std::string_view getData() const;
    std::string_view getData();

    bool isTerminal() const;
    bool isTerminal();

    bool operator==(const Symbol& other) const;

protected:
    explicit Symbol(std::string_view data, bool isTerminal);

private:
    std::string m_data;
    bool m_isTerminal;
};

class Terminal final : public Symbol {
public:
    explicit Terminal(std::string_view data);
    explicit Terminal(const Symbol& symbol);

    bool operator==(const Terminal& other) const;
};

class NonTerminal final : public Symbol {
public:
    explicit NonTerminal(std::string_view data);
    explicit NonTerminal(const Symbol& symbol);

    bool operator==(const NonTerminal& other) const;
};

constexpr struct{} Or;
const Terminal None{"ε"};
const Terminal End{ "$" };
const NonTerminal S{"S"};

std::ostream& operator<<(std::ostream& os, const Symbol& symbol);
std::ostream& operator<<(std::ostream& os, const Terminal& terminal);
std::ostream& operator<<(std::ostream& os, const NonTerminal& nonTerminal);

} //! namespace atom::cfg::grammar

namespace std {

using namespace atom::cfg::grammar;

template<>
struct hash<Symbol> {
    std::size_t operator()(const Symbol& symbol) const {
        return std::hash<std::string_view>{}(symbol.getData());
    }
};

template<>
struct hash<Terminal> {
    std::size_t operator()(const Terminal& symbol) const {
        return std::hash<std::string_view>{}(symbol.getData());
    }
};

template<>
struct hash<NonTerminal> {
    std::size_t operator()(const NonTerminal& symbol) const {
        return std::hash<std::string_view>{}(symbol.getData());
    }
};

} //! namespace std

namespace atom::cfg::grammar {

class Production final {
public:
    using DerivationType = Symbol;
    using IteratorType = std::vector<DerivationType>::iterator;
    using ConstIteratorType = std::vector<DerivationType>::const_iterator;
    using ReverseIteratorType = std::vector<DerivationType>::reverse_iterator;
    using ConstReverseIteratorType = std::vector<DerivationType>::const_reverse_iterator;

    explicit Production(std::span<const DerivationType> derivations = {});

    IteratorType begin();
    IteratorType end();
    IteratorType find(const DerivationType& derivation);

    ConstIteratorType cbegin() const;
    ConstIteratorType cend() const;
    ConstIteratorType find(const DerivationType& derivation) const;

    ReverseIteratorType rbegin();
    ReverseIteratorType rend();

    ConstReverseIteratorType crbegin() const;
    ConstReverseIteratorType crend() const;

    void pushBack(const DerivationType& newDerivation);

    std::size_t size() const;
    std::size_t size();

    bool isEmpty() const;
    bool isEmpty();

    const DerivationType& back() const;
    const DerivationType& front() const;
    DerivationType& back();
    DerivationType& front();

    friend bool operator==(const Production& l, const Production& r) { return l.isSame(r); }
    friend bool operator!=(const Production& l, const Production& r) { return !l.isSame(r); }

    std::ostream& operator<<(std::ostream& os) const;

    const std::vector<DerivationType>& getDerivations() const { return m_derivations; }
    const std::vector<DerivationType>& getDerivations() { return m_derivations; }

private:
    bool isSame(const Production& other) const;

    std::vector<DerivationType> m_derivations;
};

std::ostream& operator<<(std::ostream& os, const Production& production);
Production& operator>>(Production& production, const Production::DerivationType& newDerivation);

class Productions final {
public:
    using ProductionType = Production;
    using DerivationType = ProductionType::DerivationType;
    using IteratorType = std::vector<Production>::iterator;
    using ConstIteratorType = std::vector<Production>::const_iterator;

    explicit Productions(std::span<const Production> productions = {});
    ConstIteratorType cbegin() const;
    ConstIteratorType cend() const;
    ConstIteratorType find(const Production& production) const;
    std::size_t size() const;
    bool isEmpty() const;
    const Production& back() const;
    const Production& front() const;

    IteratorType begin();
    IteratorType end();
    IteratorType find(const Production& production);

    void pushBack(const Production& newProduction);
    std::size_t size();
    bool isEmpty();
    Production& back();
    Production& front();

    friend bool operator==(const Productions& l, const Productions& r) { return l.isSame(r); }
    friend bool operator!=(const Productions& l, const Productions& r) { return !l.isSame(r); }

    std::ostream& operator<<(std::ostream& os) const;

private:
    bool isSame(const Productions& other) const;

    std::vector<Production> m_productions;
};

std::ostream& operator<<(std::ostream& os, const Productions& productions);
Productions& operator>>(Productions& productions, const Production& newProduction);

class ProductionRules final {
public:
    using ProductionsType = Productions;
    using ProductionType = Productions::ProductionType;
    using DerivationType = Production::DerivationType;
    using PairType = std::pair<NonTerminal, Productions>;
    using IteratorType =std::vector<std::pair<NonTerminal, Productions>>::iterator;
    using ConstIteratorType = std::vector<std::pair<NonTerminal, Productions>>::const_iterator;

    explicit ProductionRules();
    ProductionRules& newProduction(const NonTerminal& left);
    void pushBack(const Symbol& symbol);
    void pushBack(const decltype(Or)& ) {
        assert(!m_rules.empty());
        auto& currentProductions = m_rules.back().second;
        assert(!currentProductions.isEmpty());
        currentProductions.pushBack(Production{});
    }

    ConstIteratorType cbegin() const;
    ConstIteratorType cend() const;
    ConstIteratorType find(const NonTerminal& left) const;
    std::optional<DerivationType> findNextDerivation(const DerivationType& derivation) const;
    std::size_t size() const;
    bool isEmpty() const;

    IteratorType begin();
    IteratorType end();
    IteratorType find(const NonTerminal& left);
    std::optional<DerivationType> findNextDerivation(const DerivationType& derivation);
    std::size_t size();
    bool isEmpty();

    std::ostream& operator<<(std::ostream& os) const;

private:
    std::vector<std::pair<NonTerminal, Productions>> m_rules;
};

std::ostream& operator<<(std::ostream& os, const ProductionRules& prodRules);
ProductionRules& operator>>(ProductionRules& prodRules, const Symbol& symbol);
inline ProductionRules& operator>>(ProductionRules& prodRules, const decltype(Or)&) {
    prodRules.pushBack(Or);
    return prodRules;
}

} //! namespace atom::cfg::grammar
