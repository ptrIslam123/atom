#include "include/cfg/grammar.h"
#include "include/cfg/ll.h"

#include <iostream>
#include <cassert>

#include <array>

using namespace atom::cfg::grammar;
using namespace atom::cfg::ll;
using namespace atom::ast;

void test1() {
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

    Parser<char> parser{std::move(rules)};
    const std::array inputString = {'a', /*'b',*/ 'c'};
    auto root = parser.buildTree(inputString);
    std::cout << *root << std::endl;
}

void test2() {
    /*
        S -> (E) | E | e;
        E -> i + S | i;
    */
    const Terminal i{"i"}, plus{"+"}, openParen{"("}, closeParen{")"};
    const NonTerminal E{"E"};

    ProductionRules rules;
    rules.newProduction(S) >> openParen >> E >> closeParen >> Or >> E >> Or >> None;
    rules.newProduction(E) >> i >> plus >> S >> Or >> i;

    Parser<char> parser{std::move(rules)};
    {
        const std::array inputString = {'i'};
        auto root = parser.buildTree(inputString);
        std::cout << *root << std::endl;
    }
    {
        const std::array inputString = {'i', '+', 'i'};
        auto root = parser.buildTree(inputString);
        std::cout << *root << std::endl;
    }
    {
        const std::array inputString = {'i', '+', 'i', '+', 'i'};
        auto root = parser.buildTree(inputString);
        std::cout << *root << std::endl;
    }
    {
        const std::array inputString = {'(','i', ')'};
        auto root = parser.buildTree(inputString);
        std::cout << *root << std::endl;
    }
    {
        const std::array inputString = {'(','i', '+', 'i', ')'};
        auto root = parser.buildTree(inputString);
        std::cout << *root << std::endl;
    }
    {
        const std::array inputString = {'i', '+', '(', 'i', ')'};
        auto root = parser.buildTree(inputString);
        std::cout << *root << std::endl;
    }
}

void test3() {
    /*
        S -> E;
        E  -> T E';
        E' -> + T E' | e;
        T  -> (E) | i;
    */
    const Terminal i{"i"}, plus{"+"}, openParen{"("}, closeParen{")"};
    const NonTerminal E{"E"}, E_{"E`"}, T{"T"};

    ProductionRules rules;
    rules.newProduction(S) >> E;
    rules.newProduction(E) >> T >> E_;
    rules.newProduction(E_) >> plus >> T >> E_ >> Or >> None;
    rules.newProduction(T) >> openParen >> E >> closeParen >> Or >> i;

    Parser<char> parser{std::move(rules)};
    {
        const std::array inputString = {'(','i', ')'};
        auto root = parser.buildTree(inputString);
        std::cout << *root << std::endl;
    }
    {
        const std::array inputString = {'(', 'i', '+', '(', 'i', '+', '(', 'i', ')', ')'};
        auto root = parser.buildTree(inputString);
        std::cout << *root << std::endl;
    }
}

int main() {
    test3();
    return 0;
}
