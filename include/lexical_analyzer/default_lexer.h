#ifndef ATOM_LEXICAL_ANALYZER_DEFAULT_LEXER_H
#define ATOM_LEXICAL_ANALYZER_DEFAULT_LEXER_H

#include "include/lexical_analyzer/lexer.h"
#include "include/lexical_analyzer/lexem.h"

namespace atom::lexical_analyzer {

Lexer<Lexem, char> MakeDefaultLexer();

} //! namespace atom::lexical_analyzer

#endif //! ATOM_LEXICAL_ANALYZER_DEFAULT_LEXER_H
