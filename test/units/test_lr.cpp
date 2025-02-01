#include <gtest/gtest.h>


#include "include/cfg/lr.h"

#include <array>

using namespace atom::cfg::grammar;
using namespace atom::cfg::lr;
using namespace atom::ast;

namespace {

inline Terminal TokenToTerminal(const char& s) {
    std::string data;
    data.push_back(s);
    return Terminal{std::move(data)};
}

} //! namespace

TEST(TestsLR, TestClosureAndGoto1) {
    /*
        S` -> S;
        S -> a B c;
        B -> b | e;
    */

    const Terminal a{"a"}, b{"b"}, c{"c"};
    const NonTerminal S_{"S`"}, B{"B"};

    ProductionRules rules;
    rules.newProduction(S_) >> S;
    rules.newProduction(S) >> a >> B >> c;
    rules.newProduction(B) >> b >> Or >> None;

    ClosureAndGotoReqHandler cgReqHandler{rules};
    {
        auto it = rules.begin();
        auto& [left, prods] = *it;
        const Item startItem{left, *prods.begin()};

        ++it;
        auto& [nleft, nprods] = *it;
        Items expectItems; // { S` -> .S; S -> .a B c; }
        expectItems.insert(startItem);
        expectItems.insert(Item{ nleft, *nprods.begin() });

        const auto realItems = cgReqHandler.getClosure(startItem);
        EXPECT_EQ(expectItems, realItems);
    }
    {
        auto it = rules.begin();
        ++it;
        auto& [left, prods] = *it;
        const Item startItem{left, *prods.begin(), 1}; // { S -> a .B c }

        ++it;
        auto& [nleft, nprods] = *it;
        Items expectItems; // { S -> a .B c; B -> .b; B -> .e; }
        expectItems.insert(startItem);
        expectItems.insert(Item{nleft, *nprods.begin()});
        expectItems.insert(Item{nleft, *(++nprods.begin())});

        const auto realItems = cgReqHandler.getClosure(startItem);
        EXPECT_EQ(expectItems, realItems);
    }
    {
        auto it = rules.begin();
        Items expectI1;
        {

        }

        auto& [left, prods] = *it;
        const Item startItem{left, *prods.begin()};
        const auto I0 = cgReqHandler.getClosure(startItem);  // I0 = { S` -> .S; S -> .a B c; }
        const auto I1 = cgReqHandler.getGoto(I0, a);         // I1 = goto(I0, a) = closure({ S -> a .B c }) = { S -> a .B c; B -> .b; B -> .e; }
        EXPECT_EQ(expectI1, I1);
    }
}


