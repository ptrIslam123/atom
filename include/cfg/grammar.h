#pragma once

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
#include "include/utils/assertion.h"
#include <cassert>

/**
 * (Context-Free Grammar, CFG) = {Non-terminal Symbols, Terminal Symbols, Start Symbol, Production Rules}
 */
namespace atom::ast::cfg::grammar {

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

    bool operator==(const Terminal& other) const;
};

class NonTerminal final : public Symbol {
public:
    explicit NonTerminal(std::string_view data);

    bool operator==(const NonTerminal& other) const;
};

constexpr struct{} Or;
const Terminal None{"ε"};
const NonTerminal S{"S"};

std::ostream& operator<<(std::ostream& os, const Symbol& symbol);
std::ostream& operator<<(std::ostream& os, const Terminal& terminal);
std::ostream& operator<<(std::ostream& os, const NonTerminal& nonTerminal);

} //! namespace xlt::ast::cfg::grammar

namespace std {

using namespace atom::ast::cfg::grammar;

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

namespace atom::ast::cfg::grammar {

namespace __details {

template<typename T>
class RandomAccessIterator final {
public:
    using iterator_category = std::random_access_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = value_type*;
    using reference = value_type&;
    using const_pointer = const std::remove_cv_t<value_type>&; // to prevent double const prefix
    using const_reference = const std::remove_cv_t<value_type>&; // to prevent double const prefix

    explicit RandomAccessIterator(pointer ptr);

    reference operator*() const;
    pointer operator->() const;

    reference operator*();
    pointer operator->();

    bool operator>(const RandomAccessIterator& other) const;
    bool operator>=(const RandomAccessIterator& other) const;
    bool operator<(const RandomAccessIterator& other) const;
    bool operator<=(const RandomAccessIterator& other) const;
    bool operator==(const RandomAccessIterator& other) const;
    bool operator!=(const RandomAccessIterator& other) const;

    RandomAccessIterator operator+(unsigned int offset);
    RandomAccessIterator operator-(unsigned int offset);

    RandomAccessIterator& operator+=(unsigned int offset);
    RandomAccessIterator& operator-=(unsigned int offset);

    RandomAccessIterator& operator++();
    RandomAccessIterator& operator++(int);

    RandomAccessIterator& operator--();
    RandomAccessIterator& operator--(int);

protected:
    pointer m_ptr;
};

} //! namespace __details

class Production final {
public:
    using DerivationType = Symbol;
    using IteratorType = __details::RandomAccessIterator<DerivationType>;
    using ConstIteratorType = __details::RandomAccessIterator<const DerivationType>;

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

private:
    bool isSame(const Production& other) const;

    std::vector<DerivationType> m_derivations;
};

std::ostream& operator<<(std::ostream& os, const Production& production);
Production& operator>>(Production& producntion, const Production::DerivationType& newDerivation);

class Productions final {
public:
    using ProductionType = Production;
    using DerivationType = ProductionType::DerivationType;
    using IteratorType = __details::RandomAccessIterator<Production>;
    using ConstIteratorType = __details::RandomAccessIterator<const Production>;

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
    using IteratorType = __details::RandomAccessIterator<PairType>;
    using ConstIteratorType = __details::RandomAccessIterator<const PairType>;
    using FirstSetType = std::unordered_set<Terminal>;

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

    std::size_t size() const;
    bool isEmpty() const;
    FirstSetType getFirst(const NonTerminal& nonTerminal) const;

    IteratorType begin();
    IteratorType end();
    IteratorType find(const NonTerminal& left);
    std::size_t size();
    bool isEmpty();
    FirstSetType getFirst(const NonTerminal& nonTerminal);

    std::ostream& operator<<(std::ostream& os) const;

private:
    FirstSetType makeFirst(const NonTerminal& nonTerminal);
    std::optional<Symbol> getNext(const Symbol& symbol);

    std::vector<std::pair<NonTerminal, Productions>> m_rules;
};

std::ostream& operator<<(std::ostream& os, const ProductionRules& prodRules);
ProductionRules& operator>>(ProductionRules& prodRules, const Symbol& symbol);
inline ProductionRules& operator>>(ProductionRules& prodRules, const decltype(Or)&) {
    prodRules.pushBack(Or);
    return prodRules;
}

bool operator==(const ProductionRules::FirstSetType& l, const ProductionRules::FirstSetType& r);

namespace __details {

template<typename T>
RandomAccessIterator<T>::RandomAccessIterator(const pointer ptr): m_ptr(ptr) {}

template<typename T>
RandomAccessIterator<T>::reference RandomAccessIterator<T>::operator*() const { return *m_ptr; }

template<typename T>
RandomAccessIterator<T>::pointer RandomAccessIterator<T>::operator->() const { return m_ptr; }

template<typename T>
RandomAccessIterator<T>::reference RandomAccessIterator<T>::operator*() { return *m_ptr; }

template<typename T>
RandomAccessIterator<T>::pointer RandomAccessIterator<T>::operator->() { return m_ptr; }

template<typename T>
typename RandomAccessIterator<T>::RandomAccessIterator RandomAccessIterator<T>::operator+(const unsigned int offset) {
    return RandomAccessIterator<T>{ m_ptr + offset };
}

template<typename T>
typename RandomAccessIterator<T>::RandomAccessIterator RandomAccessIterator<T>::operator-(unsigned int offset) {
    return RandomAccessIterator<T>{ m_ptr - offset };
}

template<typename T>
typename RandomAccessIterator<T>::RandomAccessIterator& RandomAccessIterator<T>::operator+=(const unsigned int offset) {
    m_ptr += offset;
    return *this;
}

template<typename T>
typename RandomAccessIterator<T>::RandomAccessIterator& RandomAccessIterator<T>::operator-=(unsigned int offset) {
    m_ptr -= offset;
    return *this;
}

template<typename T>
typename RandomAccessIterator<T>::RandomAccessIterator& RandomAccessIterator<T>::operator++() { return this->operator+=(1); }

template<typename T>
typename RandomAccessIterator<T>::RandomAccessIterator& RandomAccessIterator<T>::operator++(int) {
    auto tmp = *this;
    (void)this->operator+=(1);
    return tmp;
}

template<typename T>
typename RandomAccessIterator<T>::RandomAccessIterator& RandomAccessIterator<T>::operator--() { return this->operator-=(1); }

template<typename T>
typename RandomAccessIterator<T>::RandomAccessIterator& RandomAccessIterator<T>::operator--(int) {
    auto tmp = *this;
    (void)this->operator-=(1);
    return tmp;
}

template<typename T>
bool operator>(const RandomAccessIterator<T>& l, const RandomAccessIterator<T>& r) {
    return l.operator>(r);
}

template<typename T>
bool operator>=(const RandomAccessIterator<T>& l, const RandomAccessIterator<T>& r) {
    return l.operator>=(r);
}

template<typename T>
bool operator<(const RandomAccessIterator<T>& l, const RandomAccessIterator<T>& r) {
    return l.operator<(r);
}

template<typename T>
bool operator<=(const RandomAccessIterator<T>& l, const RandomAccessIterator<T>& r) {
    return l.operator<=(r);
}

template<typename T>
bool operator==(const RandomAccessIterator<T>& l, const RandomAccessIterator<T>& r) {
    return l.operator==(r);
}

template<typename T>
bool operator!=(const RandomAccessIterator<T>& l, const RandomAccessIterator<T>& r) {
    return l.operator!=(r);
}

template<typename T>
bool RandomAccessIterator<T>::operator>(const RandomAccessIterator<T>& other) const {
    return m_ptr > other.m_ptr;
}

template<typename T>
bool RandomAccessIterator<T>::operator>=(const RandomAccessIterator<T>& other) const {
    return m_ptr >= other.m_ptr;
}

template<typename T>
bool RandomAccessIterator<T>::operator<(const RandomAccessIterator<T>& other) const {
    return m_ptr < other.m_ptr;
}

template<typename T>
bool RandomAccessIterator<T>::operator<=(const RandomAccessIterator<T>& other) const {
    return m_ptr <= other.m_ptr;
}

template<typename T>
bool RandomAccessIterator<T>::operator==(const RandomAccessIterator<T>& other) const {
    return m_ptr == other.m_ptr;
}

template<typename T>
bool RandomAccessIterator<T>::operator!=(const RandomAccessIterator<T>& other) const {
    return m_ptr != other.m_ptr;
}

} //! namespace __details

} //! namespace atom::ast::cfg::grammar
