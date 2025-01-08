#include "include/cfg/grammar.h"
#include "include/cfg/ll.h"

#include <iostream>
#include <cassert>

#include <array>

using namespace atom::cfg::grammar;
using namespace atom::cfg::ll;
using namespace atom::ast;

using ParserType = Parser<char>;

Terminal TokenToTerminal(const char& s) {
    std::string data;
    data.push_back(s);
    return Terminal{std::move(data)};
}

int main() {
    /*
        S -> A B C;
        A -> a | e;
        B -> b | e;
        C -> c;
    */

    const Terminal a{"a"}, b{"b"}, c{"c"};
    const NonTerminal A{"A"}, B{"B"}, C{"C"};

    ProductionRules rules;
    rules.newProduction(S) >> A >> B >> C;
    rules.newProduction(A) >> a >> Or >> None;
    rules.newProduction(B) >> b >> Or >> None;
    rules.newProduction(C) >> c;

    ParserType parser{std::move(rules), TokenToTerminal};
   const std::array inputString = {'a', 'c'};
    auto root = parser.buildTree(inputString);
    std::cout << *root << std::endl;
    return 0;
}
