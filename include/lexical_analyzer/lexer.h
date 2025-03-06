#ifndef ATOM_LEXER_H
#define ATOM_LEXER_H

#include "include/lexical_analyzer/lexem.h"
#include "include/automaton/fsm.h"
#include "include/utils/assertion.h"

#include <span>
#include <exception>
#include <functional>
#include <string_view>
#include <vector>
#include <variant>
#include <sstream>
#include <cassert>

namespace atom::lexical_analyzer {

class BadLexer final : public std::exception {
public:
    explicit BadLexer(std::string_view msg);
    virtual const char* what() const noexcept;

private:
    std::string m_msg;
};

constexpr struct{} SkipLexem;

template<typename L, typename C, typename A = std::allocator<L>>
class Lexer final {
public:
    using Lexem = L;
    using Lexems = std::vector<Lexem, A>;
    using LexemType = Lexem::Type;
    using LexemValue = Lexem::Value;
    using FSMType = automaton::FSM<C>;
    using StateType = automaton::State;
    using AnalyzeResultType = std::variant<decltype(SkipLexem), Lexem>;
    using F = std::function<AnalyzeResultType(const StateType& /*finalState*/, std::string_view /*value*/)>;

    explicit Lexer(const FSMType& fsm, F f);
    Lexems lexemize(std::span<const C> text);

private:
    void onUnsuccessAnalysis(std::span<const C> text);

    FSMType m_fsm;
    F m_f;
};

template<typename L, typename C, typename A>
Lexer<L, C, A>::Lexer(const FSMType& fsm, F f):
m_fsm(fsm),
m_f(f) {}

template<typename L, typename C, typename A>
typename Lexer<L, C, A>::Lexems Lexer<L, C, A>::lexemize(std::span<const C> text) {
    ASSERTION(!text.empty() && text.back() == '\0', std::runtime_error, "")
    Lexems lexems;
    while (!text.empty() && text.front() != '\0') {
        auto result = m_fsm.analyze(text);
        if (result.has_value()) {
            const StateType& finalState = result->first;
            const std::span<const char> value = result->second;
            const std::string_view valueAsStr{value.data(), value.size()};
            auto nextPos = valueAsStr.size();
            if (nextPos + 1 > text.size()) {
                nextPos -= 1;
            }
            text = text.subspan(nextPos);
            auto lexem = m_f(finalState, valueAsStr);
            if (std::holds_alternative<Lexem>(lexem)) {
                lexems.push_back(std::get<1>(lexem));
            } else {
                // skip
            }
        } else {
            onUnsuccessAnalysis(text);
        }
    }
    return lexems;
}

template<typename L, typename C, typename A>
void Lexer<L, C, A>::onUnsuccessAnalysis(std::span<const C> text) {
    std::stringstream ss;
    assert(!text.empty());
    ss << "Invalid input symbol=" << text[0] << " in text=" << std::string_view{text.data(), text.size()};
    ASSERTION(false, BadLexer, ss.str())
}


} //! namespace atom::lexical_analyzer

#endif //! ATOM_LEXER_H
