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

int main() {
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
    const NonTerminal S_{"S`"}, B{"B"};
    const std::array inputString = {'a', /*'b',*/ 'c'};

    ProductionRules rules;
    rules.newProduction(S_) >> S;
    rules.newProduction(S) >> a >> B >> c;
    rules.newProduction(B) >> b >> Or >> None;

    Parser<char> parser{std::move(rules), TokenToTerminal};
    auto root = parser.buildTree(inputString);
    std::cout << *root << std::endl;
    return 0;
}
