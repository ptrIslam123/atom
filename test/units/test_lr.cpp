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
    const NonTerminal S_{"S`"}, B{"B"}, S{"S"};
    ProductionRules rules;
    rules.newProduction(S_) >> S;
    rules.newProduction(S) >> a >> B >> c;
    rules.newProduction(B) >> b >> Or >> None;
    ClosureAndGotoReqHandler cgReqHandler{rules};

    // closure(S` -> .S)
    {
        auto it = rules.begin();
        auto& [leftS_, prodsS_] = *it;
        ++it;
        auto& [leftS, prodsS] = *it;
        Items expectItems; // { S` -> .S; S -> .a B c; }
        {
            expectItems.insert(Item{leftS_, *prodsS_.begin()});     // S` -> .S;
            expectItems.insert(Item{leftS, *prodsS.begin()});       // S -> .a B c;
        }

        const auto realItems = cgReqHandler.getClosure(Item{leftS_, *prodsS_.begin()});
        EXPECT_EQ(expectItems, realItems);
    }

    // closure(S -> a .B c)
    {
        auto it = ++rules.begin();
        auto& [leftS, prodsS] = *it;
        ++it;
        auto& [leftB, prodsB] = *it;
        Items expectItems; // { S -> a .B c; B -> .b; B -> .e; }
        {
            expectItems.insert(Item{leftS, *prodsS.begin(), 1});    // S -> a .B c;
            expectItems.insert(Item{leftB, *prodsB.begin()});       // B -> .b;
            expectItems.insert(Item{leftB, *(++prodsB.begin())});   // B -> .e;
        }

        const auto realItems = cgReqHandler.getClosure(Item{leftS, *prodsS.begin(), 1}); // S -> a .B c;
        EXPECT_EQ(expectItems, realItems);
    }

    // goto(I0, a)
    {
        auto it = ++rules.begin();
        Items expectI1;
        {
            // I1 = { S -> a .B c; B -> .b; B -> .e; }
            auto& [leftS, prodsS] = *it;
            ++it;
            auto& [leftB, prodsB] = *it;

            expectI1.insert(Item{leftS, *prodsS.begin(), 1});       // S -> a .B c
            expectI1.insert(Item{leftB, *prodsB.begin()});          // B -> .b
            expectI1.insert(Item{leftB, *(++prodsB.begin())});      // B -> .e
        }

        auto it0 = rules.begin();
        auto& [left, prods] = *it0;
        const Item startItem{left, *prods.begin()}; // S` -> .S
        const auto I0 = cgReqHandler.getClosure(startItem); // I0 = { S` -> .S; S -> .a B c; }
        const auto I1 = cgReqHandler.getGoto(I0, a);
        EXPECT_EQ(expectI1, I1);
    }
}

TEST(TestLR, TestClosureAndGoto2GrammarWithCycle) {
    /*
        S` -> S;
        S -> A a | e;
        A -> S b;
    */
    const Terminal a{"a"}, b{"b"};
    const NonTerminal S_{"S`"}, A{"A"};
    ProductionRules rules;
    rules.newProduction(S_) >> S;
    rules.newProduction(S) >> A >> a >> Or >> None;
    rules.newProduction(A) >> S >> b;
    ClosureAndGotoReqHandler recursiveCgReqHandler{rules};

    // goto(I0, S)
    {
        auto it = rules.begin();
        auto& [leftS_, prodsS_] = *it;
        ++it;
        auto& [leftS, prodsS] = *it;
        ++it;
        auto& [leftA, prodsA] = *it;

        Items expectI0; // { S` -> .S; S -> .Aa; S -> .e; A -> .S b; }
        {
            expectI0.insert(Item{leftS_, *prodsS_.begin()});    // S` -> .S;
            expectI0.insert(Item{leftS, *prodsS.begin()});      // S -> .A a;
            expectI0.insert(Item{leftS, *(++prodsS.begin())});  // S -> .e;
            expectI0.insert(Item{leftA, *prodsA.begin()});      // A -> .S b;
        }

        const auto I0 = recursiveCgReqHandler.getClosure(Item{leftS_, *prodsS_.begin()});
        EXPECT_EQ(expectI0, I0);

        Items expectI1; // goto(I0, S) = closure({ S` -> S.; A -> S .b }) = I1 = { S` -> S.; A -> S .b; }
        {
            expectI1.insert(Item{leftS_, *prodsS_.begin(), 1}); // S` -> S.;
            expectI1.insert(Item{leftA, *prodsA.begin(), 1}); // A -> S .b;
        }
        const auto I1 = recursiveCgReqHandler.getGoto(I0, S);
        EXPECT_EQ(expectI1, I1);
    }
}

TEST(TestsLR, TestEmptyGrammar) {
    ProductionRules rules;
    ClosureAndGotoReqHandler cgReqHandler{rules};

    Production prod;
    NonTerminal X{"X"};

    EXPECT_THROW(cgReqHandler.getClosure(Item{X, prod}), BadLR);
}

TEST(TestsLR, TestNonexistentSymbol) {
    const Terminal a{"a"}, b{"b"}, c{"c"};
    const NonTerminal S_{"S`"}, B{"B"}, S{"S"};
    ProductionRules rules;
    rules.newProduction(S_) >> S;
    rules.newProduction(S) >> a >> B >> c;
    rules.newProduction(B) >> b >> Or >> None;

    ClosureAndGotoReqHandler cgReqHandler{rules};

    // Тест: goto с несуществующим символом
    auto it = rules.begin();
    auto& [leftS_, prodsS_] = *it;
    const Item startItem{leftS_, *prodsS_.begin()};
    const auto I0 = cgReqHandler.getClosure(startItem);
    EXPECT_THROW(cgReqHandler.getGoto(I0, Terminal{"NonExistTerminal"}), BadLR);
}
