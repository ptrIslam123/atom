#pragma once

#include "include/utils/assertion.h"
#include "include/cfg/symbols.h"

#include <ostream>
#include <string_view>
#include <vector>
#include <span>
#include <optional>
#include <iterator>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <type_traits>
#include <cassert>

namespace atom::cfg::grammar {

class Production final {
public:
    using DerivationType = Symbol;
    using IteratorType = std::vector<DerivationType>::iterator;
    using ConstIteratorType = std::vector<DerivationType>::const_iterator;

    explicit Production(std::span<const DerivationType> derivations = {});

    IteratorType begin();
    IteratorType end();
    IteratorType find(const DerivationType& derivation);

    ConstIteratorType cbegin() const;
    ConstIteratorType cend() const;
    ConstIteratorType find(const DerivationType& derivation) const;

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

class FirstAndFollowReqHandler final {
public:
    using DerivationType = ProductionRules::DerivationType;
    using FirstSetType = std::unordered_set<Symbol>;
    using FollowSetType = std::unordered_set<Symbol>;

    explicit FirstAndFollowReqHandler(ProductionRules& prodRules);

    const FirstSetType& getFirst(const Symbol& symbol);
    const FollowSetType& getFollow(const Symbol& symbol);

private:
    FirstSetType makeFirst(const Symbol& symbol);
    FollowSetType makeFollow(const Symbol& symbol);

    ProductionRules m_prodRules;
    std::unordered_map<Symbol, FirstSetType> m_firstTableCache;
    std::unordered_map<Symbol, FollowSetType> m_followTableCache;
};

bool operator==(const FirstAndFollowReqHandler::FirstSetType& l, const FirstAndFollowReqHandler::FirstSetType& r);

} //! namespace atom::cfg::grammar
