#include <iostream>

#include "include/cfg/grammar.h"
#include "include/cfg/node.h"
#include "include/cfg/lr.h"

using namespace atom::cfg::grammar;
using namespace atom::cfg::lr;
using namespace atom::ast;

Terminal TokenToTerminal(const char& s) {
    std::string data;
    data.push_back(s);
    return Terminal{std::move(data)};
}

void test1() {
    /*
        S` -> S;
        S -> a B c;
        B -> b | e;

        I0 = closure({S` -> .S}) = {S` -> .S; S -> .a B c}  | shift
        I1 = goto(I0, a) = closure({S -> a .B c}) = {S -> a .B c; B -> .b; B -> .e} |   shift
        I2 = goto(I1, b) = closure({B -> b.}) = {B -> b.}   | reduce
        I2` = goto(I1, c) = closure({B -> e.}) ???
        I3 = goto(I1, B) = closure({S -> a B .c}) = {S -> a B .c}   | shift
        I4 = goto(I3, c) = closure({S -> a B c.}) = {S -> a B c.}   | reduce
        I5 = goto(I0, S) = closure({S` -> S.}) = {S` -> S.} | reduce

        на shift:
            * Заносим в стек символ из буфера и переходим к следующему состоянию на основе текущего состояния и текущего символа
                newItems = goto(currentItems, currentSymbol)

        на reduce:
            * символ X для запроса goto определяется как нетерминал, созданный на этапе свертке(B -> b) -  после свертки мы будет запрашивать:
                Ii+1 = goto(Ii, B)
            * множество пунктов(I) для запроса goto определяется тем что мы удаляем из стека состояния(множества пунктов), соответсвующие длине правой части правила.
            Это означает, что мы возвращаемся к состоянию, которое было активным до начала обработки всей цепочки
    */

    const Terminal a{"a"}, b{"b"}, c{"c"};
    const NonTerminal B{"B"};
    const std::array inputString = {'a', 'b', 'c'};

    ProductionRules rules;
    rules.newProduction(S) >> a >> B >> c;
    rules.newProduction(B) >> b >> Or >> None;

    Parser<char> parser{std::move(rules), TokenToTerminal};
    auto root = parser.buildTree(inputString);
    assert(root);
    std::cout << *root << std::endl;
}

void test2() {
    /*
        S` -> S;
        S -> A B c;
        A -> a | e;
        B -> b | e;
    */

    const Terminal a{"a"}, b{"b"}, c{"c"};
    const NonTerminal A{"A"}, B{"B"};
    const std::array inputString = {/*'a',*/ /*'b',*/ 'c'};

    ProductionRules rules;
    rules.newProduction(S) >> A >> B >> c;
    rules.newProduction(A) >> a >> Or >> None;
    rules.newProduction(B) >> b >> Or >> None;

    Parser<char> parser{std::move(rules), TokenToTerminal};
    auto root = parser.buildTree(inputString);
    assert(root);
    std::cout << *root << std::endl;
}

void test3() {
    /*
        S -> A b;
        A -> a A | e;
    */
    const Terminal a{"a"}, b{"b"};
    const NonTerminal S{"S"}, A{"A"};
    const std::array inputString = {'a', 'a', 'b'};

    ProductionRules rules;
    rules.newProduction(S) >> A >> b;
    rules.newProduction(A) >> a >> A >> Or >> None;
    Parser<char> parser{std::move(rules), TokenToTerminal};

    auto root = parser.buildTree(inputString);
    assert(root);
    std::cout << *root << std::endl;
}

void test4() {
    /*
        S -> A B C;
        A -> a | e;
        B -> b | e;
        C -> c | e;
    */
    const Terminal a{"a"}, b{"b"}, c{"c"};
    const NonTerminal S{"S"}, A{"A"}, B{"B"}, C{"C"};
    ProductionRules rules;
    rules.newProduction(S) >> A >> B >> C;
    rules.newProduction(A) >> a >> Or >> None;
    rules.newProduction(B) >> b >> Or >> None;
    rules.newProduction(C) >> c >> Or >> None;
    Parser<char> parser{std::move(rules), TokenToTerminal};
    {
        const std::array inputString = {'a', 'b', 'c'};
        auto root = parser.buildTree(inputString);
        std::cout << *root << std::endl;
    }
    {
        const std::array<char, 0> inputString = {};
        auto root = parser.buildTree(inputString);
        std::cout << *root << std::endl;
    }
}

void test5() {
    /*
        S -> a S b | ε
    */

    // a^n b^n (ab, aabb, aaabbb)

    const Terminal a{"a"}, b{"b"};
    ProductionRules rules;
    rules.newProduction(S) >> a >> S >>  b >> Or >> None;
    Parser<char> parser{std::move(rules), TokenToTerminal};
    {
        const std::array inputString = {'a', 'b'};
        auto root = parser.buildTree(inputString);
        std::cout << *root << std::endl;
    }
    {
        const std::array inputString = {'a', 'a', 'b', 'b'};
        auto root = parser.buildTree(inputString);
        std::cout << *root << std::endl;
    }
}

int main() {
    test5();
    return 0;
}
