#include "include/lexical_analyzer/default_lexer.h"

#include "include/automaton/fsm.h"
#include "include/automaton/fsm_ops.h"
#include "include/automaton/default_fsms.h"

namespace atom::lexical_analyzer {

Lexer<Lexem, char> MakeDefaultLexer() {
    using namespace atom::automaton;
    using DefaultLexer = Lexer<Lexem, char>;
    using DefaultLexem = DefaultLexer::Lexem;
    using DefaultLexemType = DefaultLexer::LexemType;

    // for numbers
    const State
        integerNumber,
        floatingNumber,
        hexNumber,
        binaryNumber;

    // for words
    const State word;

    // for temporal expressions
    const State tempExpr;

    // for special symbols
    const State
        dot,
        comma,
        semiColon,
        space,
        openBracket,
        closeBracket,
        openSquareBracket,
        closeSquareBracket,
        openCurlyBracket,
        closeCurlyBracket,
        devision,
        subtraction,
        exclamationPoint,
        questionMark,
        percent,
        caret,
        plus,
        minus,
        eq,
        less,
        greater,
        tilde,
        hash,
        underscore,
        quote,
        apostrophe,
        ampersand,
        dollar,
        pipe,
        backslash,
        at,
        backtick,
        newLine,
        backSpace,
        carriageReturn,
        fromFeed,
        alert,
        tab,
        verticalTab,
        colon;

    const auto specialSymbolsData = std::array {
        std::make_pair('.', dot),
        std::make_pair(',', comma),
        std::make_pair(';', semiColon),
        std::make_pair(' ', space),
        std::make_pair('(', openBracket),
        std::make_pair(')', closeBracket),
        std::make_pair('[', openSquareBracket),
        std::make_pair(']', closeSquareBracket),
        std::make_pair('{', openCurlyBracket),
        std::make_pair('}', closeCurlyBracket),
        std::make_pair('%', percent),
        std::make_pair('!', exclamationPoint),
        std::make_pair('?', questionMark),
        std::make_pair('/', devision),
        std::make_pair('*', subtraction),
        std::make_pair('+', plus),
        std::make_pair('^', caret),
        std::make_pair('-', minus),
        std::make_pair(':', colon),
        std::make_pair('\"', quote),
        std::make_pair('\'', apostrophe),
        std::make_pair('#', hash),
        std::make_pair('~', tilde),
        std::make_pair('&', ampersand),
        std::make_pair('_', underscore),
        std::make_pair('$', dollar),
        std::make_pair('=', eq),
        std::make_pair('<', less),
        std::make_pair('>', greater),
        std::make_pair('|', pipe),
        std::make_pair('\\', backslash),
        std::make_pair('@', at),
        std::make_pair('`', backtick),
        std::make_pair('\n', newLine),
        std::make_pair('\b', backSpace),
        std::make_pair('\r', carriageReturn),
        std::make_pair('\f', fromFeed),
        std::make_pair('\a', alert),
        std::make_pair('\t', tab),
        std::make_pair('\v', verticalTab),
    };

    FSM fsmForNumbers = MakeNumberFSMAnalyzer(integerNumber, floatingNumber, hexNumber, binaryNumber);
    FSM fsmForWord = MakeWordFSMAnalyzer(word);
    FSM fsmTempExpr = MakeTemporalExpressionsFSMAnalyzer(tempExpr);
    FSM fsmsForSpecialSymbols = MakeSpecialSymbolFSMAnalyzer({specialSymbolsData});

    FSM mainFsm = Combine(
        fsmForNumbers, Combine(fsmTempExpr, Combine(fsmsForSpecialSymbols, fsmForWord))
    );

    auto f = [integerNumber,
              floatingNumber,
              hexNumber,
              binaryNumber,

              word,

              tempExpr,

              dot,
              comma,
              semiColon,
              space,
              openBracket,
              closeBracket,
              openSquareBracket,
              closeSquareBracket,
              openCurlyBracket,
              closeCurlyBracket,
              devision,
              subtraction,
              exclamationPoint,
              questionMark,
              percent,
              caret,
              plus,
              minus,
              eq,
              less,
              greater,
              tilde,
              hash,
              underscore,
              quote,
              apostrophe,
              ampersand,
              dollar,
              pipe,
              backslash,
              at,
              backtick,
              colon
    ](const State& finalState, const std::string_view value) -> DefaultLexer::AnalyzeResultType {
        static const std::unordered_map<State::IdType, DefaultLexemType> lexems = {
            {integerNumber.getId(), DefaultLexemType::Integer},
            {floatingNumber.getId(), DefaultLexemType::Float},
            {hexNumber.getId(), DefaultLexemType::Hex},
            {binaryNumber.getId(), DefaultLexemType::Binary},

            {word.getId(), DefaultLexemType::Word},

            {tempExpr.getId(), DefaultLexemType::TempExpr},

            {dot.getId(), DefaultLexemType::Dot},
            {comma.getId(), DefaultLexemType::Comma},
            {semiColon.getId(), DefaultLexemType::SemiColon},
            // {space.getId(), DefaultLexemType::Space}, // skip this symbol
            {openBracket.getId(), DefaultLexemType::OpenBracket},
            {closeBracket.getId(), DefaultLexemType::CloseBracket},
            {openSquareBracket.getId(), DefaultLexemType::OpenSquareBracket},
            {closeSquareBracket.getId(), DefaultLexemType::CloseSquareBracket},
            {openCurlyBracket.getId(), DefaultLexemType::OpenCurlyBracket},
            {closeCurlyBracket.getId(), DefaultLexemType::CloseCurlyBracket},
            {devision.getId(), DefaultLexemType::Devision},
            {subtraction.getId(), DefaultLexemType::Subtraction},
            {exclamationPoint.getId(), DefaultLexemType::ExclamationPoint},
            {questionMark.getId(), DefaultLexemType::QuestionMark},
            {percent.getId(), DefaultLexemType::Percent},
            {caret.getId(), DefaultLexemType::Caret},
            {plus.getId(), DefaultLexemType::Plus},
            {minus.getId(), DefaultLexemType::Minus},
            {eq.getId(), DefaultLexemType::Eq},
            {less.getId(), DefaultLexemType::Less},
            {greater.getId(), DefaultLexemType::Greater},
            {tilde.getId(), DefaultLexemType::Tilde},
            {hash.getId(), DefaultLexemType::Hash},
            {underscore.getId(), DefaultLexemType::Underscore},
            {quote.getId(), DefaultLexemType::Quote},
            {apostrophe.getId(), DefaultLexemType::Apostrophe},

            {ampersand.getId(), DefaultLexemType::Ampersand},
            {dollar.getId(), DefaultLexemType::Dollar},
            {pipe.getId(), DefaultLexemType::Pipe},
            {backslash.getId(), DefaultLexemType::Backslash},
            {at.getId(), DefaultLexemType::At},
            {backtick.getId(), DefaultLexemType::Backtick},
            {colon.getId(), DefaultLexemType::Colon}
        };
        auto it = lexems.find(finalState.getId());
        if (it != lexems.cend()) {
            return DefaultLexem{it->second, std::string{value.data(), value.size()}};
        } else {
            return SkipLexem;
        }
    };
    return DefaultLexer{std::move(mainFsm), f};
}

} //! namespace atom::lexical_analyzer
