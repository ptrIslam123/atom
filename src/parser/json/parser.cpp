// #include "include/parser/json/parser.h"

// #include "include/lexical_analyzer/default_lexer.h"

// namespace {

// using namespace atom::cfg::grammar;
// using namespace atom::cfg::ll;
// using namespace atom::ast;

// const Terminal
//     OpenTag{"<"},
//     CloseTag{">"},
//     Slash{"/"},
//     Eq{"Eq"},
//     Word{"Word"},
//     Number{"Number"},
//     Quote{"\""},
//     Apostrophe{"\'"};

// const NonTerminal
//     Doc{"Doc"},
//     Element{"Element"},
//     TagName{"TagName"},
//     Content{"Content"},
//     Data{"Data"},
//     AttrList{"AttrList"},
//     Attr{"Attr"},
//     AttrName{"AttrName"},
//     AttrValue{"AttrValue"};

// } //! namespace

// namespace atom::cfg::grammar {

// template<>
// Terminal TokenToTerminal<lexical_analyzer::Lexem>(const lexical_analyzer::Lexem& lexem) {
//     using Type = lexical_analyzer::Lexem::Type;
//     switch (lexem.getType()) {
//     case Type::Less: return OpenTag;
//     case Type::Greater: return CloseTag;
//     case Type::Devision: return Slash;
//     case Type::Eq: return Eq;
//     case Type::Word: return Word;
//     case Type::Integer:
//     case Type::Float:
//     case Type::Hex:
//     case Type::Binary: return Number;
//     case Type::Quote: return Quote;
//     case Type::Apostrophe: return Apostrophe;
//     default: {
//         assert(false);
//     }
//     }
// }

// } //! atom::cfg::grammar

// namespace atom::parser::json {

// Parser::Parser():
//     m_lexicalAnalyzer(MakeLexicalAnalyzer()),
//     m_syntaxAnalyzer(MakeSyntaxAnalyzer())
// {}

// void Parser::parse(const std::filesystem::path& contentPath) {

// }

// void Parser::parse(std::string&& content) {
//     if (content.empty()) {
//         return;
//     }

//     if (content.back() != '\0') {
//         content.push_back('\0'); // EOF
//     }

//     auto lexems = m_lexicalAnalyzer.lexemize(content);
//     if (lexems.empty()) {
//         return;
//     }

//     // auto ast = m_syntaxAnalyzer.buildTree(lexems);
//     // if (!ast) {
//     //     return;
//     // }
// }

// Parser::LexicalAnalyzer Parser::MakeLexicalAnalyzer() {
//     using namespace lexical_analyzer;
//     return MakeDefaultLexer();
// }

// Parser::SyntaxAnalyzer Parser::MakeSyntaxAnalyzer() {
//     using namespace atom::cfg::grammar;
//     using namespace atom::cfg::ll;
//     using namespace atom::ast;
//     /*
//         JSON       -> Object | Array
//         Object     -> '{' PairList '}'
//             PairList   -> Pair | Pair ',' PairList | e;
//                 Pair       -> String ':' Value

//         Array      -> '[' ValueList ']'
//             ValueList  -> Value | Value ',' ValueList | e;

//         Value      -> String | Number | Object | Array | Bool | 'null'
//     */
//     ProductionRules rules;
//     // S -> Doc
//     rules.newProduction(S) >> Doc;

//     // Doc -> Element
//     rules.newProduction(Doc) >> Element;

//     // Element -> '<' TagName AttrList '>' Content '</' TagName '>' | '<' TagName AttrList '/>'
//     rules.newProduction(Element)
//         >> OpenTag >> TagName >> AttrList >> CloseTag >> Content >> OpenTag >> Slash >> TagName >> CloseTag >> Or
//         >> OpenTag >> TagName >> AttrList >> Slash >> CloseTag;

//     // TagName -> Word
//     rules.newProduction(TagName) >> Word;

//     // AttrList -> AttrName '=' AttrValue AttrList | e
//     rules.newProduction(AttrList) >> AttrName >> Eq >> AttrValue >> AttrList >> Or >> None;

//     // AttrName -> Word
//     rules.newProduction(AttrName) >> Word;

//     // AttrValue -> Data;
//     rules.newProduction(AttrValue) >> Data;

//     // Content -> Element Content | Data Content | e;
//     rules.newProduction(Content) >> Element >> Content >> Or >> Data >> Content >> Or >> None;

//     // Data -> Word | "Word" | 'Word' | Number
//     rules.newProduction(Data) >> Word >> Or >> Quote >> Word >> Quote >> Or >> Apostrophe >> Word >> Apostrophe >> Number;

//     ///// Есть подозрние что ll парсера не достаточно для синтаксического анализа xml данных
//     //SyntaxAnalyzer parser{std::move(rules)};
//     //return parser;
// }

// } //! namespace atom::parser::json
