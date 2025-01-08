#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <string>
#include <string_view>
#include <ostream>
#include <functional>

/**
 * (Context-Free Grammar, CFG) = {Non-terminal Symbols, Terminal Symbols, Start Symbol, Production Rules}
 */
namespace atom::cfg::grammar {

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

#endif //! SYMBOLS_H
