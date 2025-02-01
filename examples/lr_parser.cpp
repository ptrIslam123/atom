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
    */

    const Terminal a{"a"}, b{"b"}, c{"c"};
    const NonTerminal S_{"S`"}, B{"B"}, D{"D"};

    const std::array inputString = {'a', 'b', 't'};

    ProductionRules rules;
    rules.newProduction(S) >> D >> B >> c;
    rules.newProduction(D) >> a;
    rules.newProduction(B) >> b >> Or >> None;


    ClosureAndGotoReqHandler cgReqHandler{rules};
    auto&[left, prods] = *rules.begin();
    auto i0 = cgReqHandler.getClosure(Item{left, *prods.begin()});
    std::cout << i0 << std::endl;
    return 0;
}
