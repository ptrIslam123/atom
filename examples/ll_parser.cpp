#include "include/cfg/grammar.h"
#include "include/cfg/ll.h"

#include <iostream>
#include <array>
#include <cassert>

using namespace atom::cfg::grammar;
using namespace atom::cfg::ll;
using namespace atom::ast;

std::vector<char> SplitString(const std::string_view inputStr) {
    std::vector<char> result;
    result.reserve(inputStr.size());
    for (const char s : inputStr) {
        if (s != ' ') {
            result.push_back(s);
        }
    }
    return result;
}

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
        const std::array inputString = {'i', '+', '(', 'i', '+', 'i', '+', 'i', ')'};
        auto root = parser.buildTree(inputString);
        std::cout << *root << std::endl;
    }
    {
        const std::array inputString = {'(', 'i', '+', '(', 'i', '+', '(', 'i', ')', ')', ')'};
        auto root = parser.buildTree(inputString);
        std::cout << *root << std::endl;
    }
}

void test4() {
    /*
        S -> ID;
        D -> ID | .ID  | e;
        I -> 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9;
    */

    const Terminal dot{"."}, i0{"0"}, i1{"1"}, i2{"2"}, i3{"3"}, i4{"4"}, i5{"5"}, i6{"6"}, i7{"7"}, i8{"8"}, i9{"9"};
    const NonTerminal I{"I"}, D{"D"};

    ProductionRules rules;
    rules.newProduction(S) >> I >> D;
    rules.newProduction(D) >> I >> D >> Or >> dot >> I >> D >> Or >> None;
    rules.newProduction(I)
        >> i0 >> Or
        >> i1 >> Or
        >> i2 >> Or
        >> i3 >> Or
        >> i4 >> Or
        >> i5 >> Or
        >> i6 >> Or
        >> i7 >> Or
        >> i8 >> Or
        >> i9;

    Parser<char> parser{std::move(rules)};
    {
        const std::array inputString = {'1' ,'2', '5'};
        auto root = parser.buildTree(inputString);
        std::cout << *root << std::endl;
    }
    {
        const std::array inputString = {'1' ,'.', '5', '1'};
        auto root = parser.buildTree(inputString);
        std::cout << *root << std::endl;
    }
}

void test5() {
    /*
        S -> '{' List '}';
        List -> Item | e;
        Item -> 'v' | 'v' ',' Item | 'v' ',' S;

        FIRST(S) = {'{'};
        FIRST(List) = {FIRST(Item), e} = {'v', e};
        FIRST(Item) = {'v'};

        FOLLOW(S) = {$};
        FOLLOW(List) = {'}'};
        FOLLOW(Item) = {$};
    */

    const Terminal v{"v"}, comma{","}, openList{"{"}, closeList{"}"};
    const NonTerminal List{"List"}, Item{"Item"};

    ProductionRules rules;
    rules.newProduction(S) >> openList >> List >> closeList;
    rules.newProduction(List) >> Item >> Or >> None;
    rules.newProduction(Item) >> v >> Or >> v >> comma >> Item >> Or >> v >> comma >> S;
    Parser<char> parser{std::move(rules)};
    {
        auto root = parser.buildTree(SplitString("{}"));
        assert(root != nullptr);
        std::cout << *root << std::endl;
    }
    {
        auto root = parser.buildTree(SplitString("{v}"));
        assert(root != nullptr);
        std::cout << *root << std::endl;
    }
    {
        auto root = parser.buildTree(SplitString("{v, v}"));
        assert(root != nullptr);
        std::cout << *root << std::endl;
    }
    {
        auto root = parser.buildTree(SplitString("{v, v, {v}}"));
        assert(root != nullptr);
        std::cout << *root << std::endl;
    }
}

int main() {
    test5();
    return 0;
}
