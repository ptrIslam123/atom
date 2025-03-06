#include <iostream>
#include <string>
#include <cassert>

#include "include/lexical_analyzer/default_lexer.h"

using namespace atom;
using namespace automaton;
using namespace lexical_analyzer;

std::string jsonSample = R"(
{
  "users": [
    {
      "id": 1,
      "name": "John Doe",
      "email": "john.doe@example.com",
      "age": 28,
      "address": {
        "city": "New York",
        "street": "5th Avenue"
      }
    },
    {
      "id": 2,
      "name": "Jane Smith",
      "email": "jane.smith@example.com",
      "age": 34,
      "address": {
        "city": "Los Angeles",
        "street": "Sunset Boulevard"
      }
    }
  ]
}
)";

int main() {
    try {
        jsonSample.push_back('\0');
        auto lexer = MakeDefaultLexer();
        auto lexems = lexer.lexemize(jsonSample);
        assert(!lexems.empty());
        for (const auto& lexem : lexems) {
            std::cout << lexem << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown error" << std::endl;
    }
    return 0;
}
