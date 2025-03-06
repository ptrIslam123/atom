#ifndef ATOM_LEXER_TOKEN_H
#define ATOM_LEXER_TOKEN_H

#include <ostream>

namespace atom::lexical_analyzer {

template<typename T, typename V>
class BasicLexem final {
public:
    using Type = T;
    using Value = V;

    explicit BasicLexem(const T& type, const V& value);
    const T& getType() const;
    const V& getValue() const;

private:
    T m_type;
    V m_value;
};

template<typename T, typename V>
std::ostream& operator<<(std::ostream& os, const BasicLexem<T, V>& lexem);

template<typename T, typename V>
bool operator==(const BasicLexem<T, V>& f, const BasicLexem<T, V>& s);

template<typename T, typename V>
bool operator!=(const BasicLexem<T, V>& f, const BasicLexem<T, V>& s);


enum class LexemType {
    None = 0,

    Integer,
    Float,
    Hex,
    Binary,

    Word,
    TempExpr,

    Dot,
    Comma,
    SemiColon,
    OpenBracket,
    CloseBracket,
    OpenSquareBracket,
    CloseSquareBracket,
    OpenCurlyBracket,
    CloseCurlyBracket,
    Devision,
    Subtraction,
    ExclamationPoint,
    QuestionMark,
    Percent,
    Caret,
    Plus,
    Minus,
    Eq,
    Less,
    Greater,
    Tilde,
    Hash,
    Underscore,
    Quote,
    Apostrophe,
    Ampersand,
    Dollar,
    Pipe,
    Backslash,
    At,
    Backtick,
    Colon,
};

inline std::ostream& operator<<(std::ostream& os, const LexemType lexemType) {
    return os << static_cast<unsigned>(lexemType);
}

using Lexem = BasicLexem<LexemType, std::string>;

template<typename T, typename V>
BasicLexem<T, V>::BasicLexem(const T& type, const V& value):
m_type(type),
m_value(value)
{}

template<typename T, typename V>
const T& BasicLexem<T, V>::getType() const {
    return m_type;
}

template<typename T, typename V>
const V& BasicLexem<T, V>::getValue() const {
    return m_value;
}

template<typename T, typename V>
std::ostream& operator<<(std::ostream& os, const BasicLexem<T, V>& lexem)
{
    os << "atom::lexical_analyzer::Lexem{type=" << lexem.getType() << ", value=" << lexem.getValue() << "}";
    return os;
}

template<typename T, typename V>
bool operator==(const BasicLexem<T, V>& f, const BasicLexem<T, V>& s) {
    return (f.getType() == s.getType()) && (f.getValue() == f.getValue());
}

template<typename T, typename V>
bool operator!=(const BasicLexem<T, V>& f, const BasicLexem<T, V>& s) {
    return !(f == s);
}

} //! namespace atom::lexical_analyzer

#endif //! ATOM_LEXER_TOKEN_H
