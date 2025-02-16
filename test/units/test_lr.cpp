#include <gtest/gtest.h>

//#define ENABLE_LR_TESTS
#if defined(ENABLE_LR_TESTS)

#include "include/cfg/lr.h"

#include <array>

using namespace atom::cfg::grammar;
using namespace atom::cfg::lr;
using namespace atom::ast;

TEST(TestLR, TestParser1) {
    /*
        S -> A t;
        A -> a;
    */

    const Terminal a{"a"}, t{"t"};
    const NonTerminal A{"A"};

    ProductionRules rules;
    rules.newProduction(S) >> A >> t;
    rules.newProduction(A) >> a;
    Parser<char> parser{std::move(rules)};
    const std::array inputString = {'a', 't'};
    auto expectTree = std::make_unique<Node>(nullptr, S);
    {
        auto ATree = std::make_unique<Node>(expectTree.get(), A);
        ATree->addChild(std::make_unique<Leaf<char>>(ATree.get(), a, 'a'));

        auto tTree = std::make_unique<Leaf<char>>(expectTree.get(), t, 't');

        expectTree->addChild(std::move(tTree));
        expectTree->addChild(std::move(ATree));
    }
    auto realTree = parser.buildTree(inputString);
    ASSERT_TRUE(realTree);
    EXPECT_EQ(*realTree, *expectTree);
}

TEST(TestLR, TestParser2) {
    /*
        S -> A B c;
        A -> a | e;
        B -> b | e;
    */

    const Terminal a{"a"}, b{"b"}, c{"c"};
    const NonTerminal A{"A"}, B{"B"};

    ProductionRules rules;
    rules.newProduction(S) >> A >> B >> c;
    rules.newProduction(A) >> a >> Or >> None;
    rules.newProduction(B) >> b >> Or >> None;

    Parser<char> parser{std::move(rules)};
    {
        const std::array inputString = {'a', 'b', 'c'};
        auto expectTree = std::make_unique<Node>(nullptr, S);
        {
            auto ATree = std::make_unique<Node>(expectTree.get(), A);
            ATree->addChild(std::make_unique<Leaf<char>>(ATree.get(), a, 'a'));

            auto BTree = std::make_unique<Node>(expectTree.get(), B);
            BTree->addChild(std::make_unique<Leaf<char>>(BTree.get(), b, 'b'));

            auto cTree = std::make_unique<Leaf<char>>(expectTree.get(), c, 'c');

            expectTree->addChild(std::move(cTree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(ATree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'a', 'c'};
        auto expectTree = std::make_unique<Node>(nullptr, S);
        {
            auto ATree = std::make_unique<Node>(expectTree.get(), A);
            ATree->addChild(std::make_unique<Leaf<char>>(ATree.get(), a, 'a'));

            auto BTree = std::make_unique<Node>(expectTree.get(), B);
            BTree->addChild(std::make_unique<Node>(BTree.get(), None));

            auto cTree = std::make_unique<Leaf<char>>(expectTree.get(), c, 'c');

            expectTree->addChild(std::move(cTree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(ATree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'c'};
        auto expectTree = std::make_unique<Node>(nullptr, S);
        {
            auto ATree = std::make_unique<Node>(expectTree.get(), A);
            ATree->addChild(std::make_unique<Node>(ATree.get(), None));

            auto BTree = std::make_unique<Node>(expectTree.get(), B);
            BTree->addChild(std::make_unique<Node>(BTree.get(), None));

            auto cTree = std::make_unique<Leaf<char>>(expectTree.get(), c, 'c');

            expectTree->addChild(std::move(cTree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(ATree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        EXPECT_EQ(*realTree, *expectTree);
    }
}

TEST(TestLR, TestParser3) {
    /*
        S -> A B C;
        A -> a;
        B -> b | e;
        C -> c;
    */
    const Terminal a{"a"}, b{"b"}, c{"c"};
    const NonTerminal S{"S"}, A{"A"}, B{"B"}, C{"C"};
    ProductionRules rules;
    rules.newProduction(S) >> A >> B >> C;
    rules.newProduction(A) >> a;
    rules.newProduction(B) >> b >> Or >> None;
    rules.newProduction(C) >> c;
    Parser<char> parser{std::move(rules)};
    {
        const std::array inputString = {'a', 'b', 'c'};
        auto expectTree = std::make_unique<Node>(nullptr, S);
        {
            auto ATree = std::make_unique<Node>(expectTree.get(), A);
            ATree->addChild(std::make_unique<Leaf<char>>(ATree.get(), a, 'a'));

            auto BTree = std::make_unique<Node>(expectTree.get(), B);
            BTree->addChild(std::make_unique<Leaf<char>>(BTree.get(), b, 'b'));

            auto CTree = std::make_unique<Node>(expectTree.get(), C);
            CTree->addChild(std::make_unique<Leaf<char>>(CTree.get(), c, 'c'));

            expectTree->addChild(std::move(CTree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(ATree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'a', 'c'};
        auto expectTree = std::make_unique<Node>(nullptr, S);
        {
            auto ATree = std::make_unique<Node>(expectTree.get(), A);
            ATree->addChild(std::make_unique<Leaf<char>>(ATree.get(), a, 'a'));

            auto BTree = std::make_unique<Node>(expectTree.get(), B);
            BTree->addChild(std::make_unique<Node>(BTree.get(), None));

            auto CTree = std::make_unique<Node>(expectTree.get(), C);
            CTree->addChild(std::make_unique<Leaf<char>>(CTree.get(), c, 'c'));

            expectTree->addChild(std::move(CTree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(ATree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        EXPECT_EQ(*realTree, *expectTree);
    }
}

TEST(TestLR, TestParser4) {
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
    Parser<char> parser{std::move(rules)};
    {
        const std::array inputString = {'a', 'b', 'c'};
        auto expectTree = std::make_unique<Node>(nullptr, S);
        {
            auto ATree = std::make_unique<Node>(expectTree.get(), A);
            ATree->addChild(std::make_unique<Leaf<char>>(ATree.get(), a, 'a'));
            auto BTree = std::make_unique<Node>(expectTree.get(), B);
            BTree->addChild(std::make_unique<Leaf<char>>(BTree.get(), b, 'b'));
            auto CTree = std::make_unique<Node>(expectTree.get(), C);
            CTree->addChild(std::make_unique<Leaf<char>>(CTree.get(), c, 'c'));

            expectTree->addChild(std::move(CTree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(ATree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'a', 'b'};
        auto expectTree = std::make_unique<Node>(nullptr, S);
        {
            auto ATree = std::make_unique<Node>(expectTree.get(), A);
            ATree->addChild(std::make_unique<Leaf<char>>(ATree.get(), a, 'a'));
            auto BTree = std::make_unique<Node>(expectTree.get(), B);
            BTree->addChild(std::make_unique<Leaf<char>>(BTree.get(), b, 'b'));
            auto CTree = std::make_unique<Node>(expectTree.get(), C);
            CTree->addChild(std::make_unique<Node>(CTree.get(), None));

            expectTree->addChild(std::move(CTree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(ATree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'a', 'c'};
        auto expectTree = std::make_unique<Node>(nullptr, S);
        {
            auto ATree = std::make_unique<Node>(expectTree.get(), A);
            ATree->addChild(std::make_unique<Leaf<char>>(ATree.get(), a, 'a'));
            auto BTree = std::make_unique<Node>(expectTree.get(), B);
            BTree->addChild(std::make_unique<Node>(BTree.get(), None));
            auto CTree = std::make_unique<Node>(expectTree.get(), C);
            CTree->addChild(std::make_unique<Leaf<char>>(CTree.get(), c, 'c'));

            expectTree->addChild(std::move(CTree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(ATree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'b', 'c'};
        auto expectTree = std::make_unique<Node>(nullptr, S);
        {
            auto ATree = std::make_unique<Node>(expectTree.get(), A);
            ATree->addChild(std::make_unique<Node>(ATree.get(), None));
            auto BTree = std::make_unique<Node>(expectTree.get(), B);
            BTree->addChild(std::make_unique<Leaf<char>>(BTree.get(), b, 'b'));
            auto CTree = std::make_unique<Node>(expectTree.get(), C);
            CTree->addChild(std::make_unique<Leaf<char>>(CTree.get(), c, 'c'));

            expectTree->addChild(std::move(CTree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(ATree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'a'};
        auto expectTree = std::make_unique<Node>(nullptr, S);
        {
            auto ATree = std::make_unique<Node>(expectTree.get(), A);
            ATree->addChild(std::make_unique<Leaf<char>>(ATree.get(), a, 'a'));
            auto BTree = std::make_unique<Node>(expectTree.get(), B);
            BTree->addChild(std::make_unique<Node>(BTree.get(), None));
            auto CTree = std::make_unique<Node>(expectTree.get(), C);
            CTree->addChild(std::make_unique<Node>(CTree.get(), None));

            expectTree->addChild(std::move(CTree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(ATree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'b'};
        auto expectTree = std::make_unique<Node>(nullptr, S);
        {
            auto ATree = std::make_unique<Node>(expectTree.get(), A);
            ATree->addChild(std::make_unique<Node>(ATree.get(), None));
            auto BTree = std::make_unique<Node>(expectTree.get(), B);
            BTree->addChild(std::make_unique<Leaf<char>>(BTree.get(), b, 'b'));
            auto CTree = std::make_unique<Node>(expectTree.get(), C);
            CTree->addChild(std::make_unique<Node>(CTree.get(), None));

            expectTree->addChild(std::move(CTree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(ATree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'c'};
        auto expectTree = std::make_unique<Node>(nullptr, S);
        {
            auto ATree = std::make_unique<Node>(expectTree.get(), A);
            ATree->addChild(std::make_unique<Node>(ATree.get(), None));
            auto BTree = std::make_unique<Node>(expectTree.get(), B);
            BTree->addChild(std::make_unique<Node>(BTree.get(), None));
            auto CTree = std::make_unique<Node>(expectTree.get(), C);
            CTree->addChild(std::make_unique<Leaf<char>>(CTree.get(), c, 'c'));

            expectTree->addChild(std::move(CTree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(ATree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array<char, 0> inputString = {};
        auto expectTree = std::make_unique<Node>(nullptr, S);
        {
            auto ATree = std::make_unique<Node>(expectTree.get(), A);
            ATree->addChild(std::make_unique<Node>(ATree.get(), None));
            auto BTree = std::make_unique<Node>(expectTree.get(), B);
            BTree->addChild(std::make_unique<Node>(BTree.get(), None));
            auto CTree = std::make_unique<Node>(expectTree.get(), C);
            CTree->addChild(std::make_unique<Node>(CTree.get(), None));

            expectTree->addChild(std::move(CTree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(ATree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        EXPECT_EQ(*realTree, *expectTree);
    }
}

TEST(TestLR, TestParser5) {
    /*
        S -> A b;
        A -> a A | e;
    */
    const Terminal a{"a"}, b{"b"};
    const NonTerminal S{"S"}, A{"A"};
    ProductionRules rules;
    rules.newProduction(S) >> A >> b;
    rules.newProduction(A) >> a >> A >> Or >> None;
    Parser<char> parser{std::move(rules)};
    {
        const std::array inputString = {'b'};
        auto expectTree = std::make_unique<Node>(nullptr, S);
        {
            auto ATree = std::make_unique<Node>(expectTree.get(), A);
            ATree->addChild(std::make_unique<Node>(ATree.get(), None));

            auto bTree = std::make_unique<Leaf<char>>(expectTree.get(), b, 'b');

            expectTree->addChild(std::move(bTree));
            expectTree->addChild(std::move(ATree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'a', 'b'};
        auto expectTree = std::make_unique<Node>(nullptr, S);
        {
            auto ATree = std::make_unique<Node>(expectTree.get(), A);
            {
                auto aTree = std::make_unique<Leaf<char>>(ATree.get(), a, 'a');
                auto _ATree = std::make_unique<Node>(ATree.get(), A);
                _ATree->addChild(std::make_unique<Node>(ATree.get(), None));

                ATree->addChild(std::move(_ATree));
                ATree->addChild(std::move(aTree));
            }

            auto bTree = std::make_unique<Leaf<char>>(expectTree.get(), b, 'b');

            expectTree->addChild(std::move(bTree));
            expectTree->addChild(std::move(ATree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'a', 'a', 'b'};
        auto expectTree = std::make_unique<Node>(nullptr, S);
        {
            auto ATree = std::make_unique<Node>(expectTree.get(), A);
            {
                auto aTree = std::make_unique<Leaf<char>>(ATree.get(), a, 'a');
                auto _ATree = std::make_unique<Node>(ATree.get(), A);
                {
                    auto _aTree = std::make_unique<Leaf<char>>(_ATree.get(), a, 'a');
                    auto __ATree = std::make_unique<Node>(_ATree.get(), A);
                    __ATree->addChild(std::make_unique<Node>(__ATree.get(), None));

                    _ATree->addChild(std::move(__ATree));
                    _ATree->addChild(std::move(_aTree));
                }

                ATree->addChild(std::move(_ATree));
                ATree->addChild(std::move(aTree));
            }

            auto bTree = std::make_unique<Leaf<char>>(expectTree.get(), b, 'b');

            expectTree->addChild(std::move(bTree));
            expectTree->addChild(std::move(ATree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        EXPECT_EQ(*realTree, *expectTree);
    }
}

TEST(TestLR, TestParser6) {
    /*
        S -> a S b | ε
    */

    // a^n b^n (ab, aabb, aaabbb)

    const Terminal a{"a"}, b{"b"};

    ProductionRules rules;
    rules.newProduction(S) >> a >> S >>  b >> Or >> None;
    Parser<char> parser{std::move(rules)};
    {
        const std::array inputString = {'a', 'b'};
        auto expectTree = std::make_unique<Node>(nullptr, S);
        {
            auto aTree = std::make_unique<Leaf<char>>(expectTree.get(), a, 'a');
            auto _STree = std::make_unique<Node>(expectTree.get(), S);
            _STree->addChild(std::make_unique<Node>(_STree.get(), None));
            auto bTree = std::make_unique<Leaf<char>>(expectTree.get(), b, 'b');

            expectTree->addChild(std::move(bTree));
            expectTree->addChild(std::move(_STree));
            expectTree->addChild(std::move(aTree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'a', 'a', 'b', 'b'};
        auto expectTree = std::make_unique<Node>(nullptr, S);
        {
            auto aTree = std::make_unique<Leaf<char>>(expectTree.get(), a, 'a');
            auto STree = std::make_unique<Node>(expectTree.get(), S);
            {
                auto _aTree = std::make_unique<Leaf<char>>(STree.get(), a, 'a');
                auto _STree = std::make_unique<Node>(STree.get(), S);
                _STree->addChild(std::make_unique<Node>(_STree.get(), None));
                auto _bTree = std::make_unique<Leaf<char>>(STree.get(), b, 'b');

                STree->addChild(std::move(_bTree));
                STree->addChild(std::move(_STree));
                STree->addChild(std::move(_aTree));
            }
            auto bTree = std::make_unique<Leaf<char>>(expectTree.get(), b, 'b');

            expectTree->addChild(std::move(bTree));
            expectTree->addChild(std::move(STree));
            expectTree->addChild(std::move(aTree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        EXPECT_EQ(*realTree, *expectTree);
    }
}

TEST(TestLL, Test7) {
    /*
        S -> '(' S ')' | 'a'
    */

    const Terminal a{"a"}, openParen{"("}, closeParen{")"};

    ProductionRules rules;
    rules.newProduction(S) >> openParen >> S >> closeParen >> Or >> a;
    Parser<char> parser{std::move(rules)};;
    {
        const std::array inputString = {'a'};
        auto expectTree = std::make_unique<Node>(nullptr, S);
        expectTree->addChild(std::make_unique<Leaf<char>>(expectTree.get(), a, 'a'));
        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'(', 'a', ')'};
        auto expectTree = std::make_unique<Node>(nullptr, S);
        {
            auto openTree = std::make_unique<Leaf<char>>(expectTree.get(), openParen, '(');
            auto STree = std::make_unique<Node>(expectTree.get(), S);
            STree->addChild(std::make_unique<Leaf<char>>(STree.get(), a, 'a'));
            auto closeTree = std::make_unique<Leaf<char>>(expectTree.get(), closeParen, ')');

            expectTree->addChild(std::move(closeTree));
            expectTree->addChild(std::move(STree));
            expectTree->addChild(std::move(openTree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'(', '(', 'a', ')', ')'};
        auto expectTree = std::make_unique<Node>(nullptr, S);
        {
            auto openTree = std::make_unique<Leaf<char>>(expectTree.get(), openParen, '(');
            auto STree = std::make_unique<Node>(expectTree.get(), S);
            {
                auto _openTree = std::make_unique<Leaf<char>>(STree.get(), openParen, '(');
                auto _STree = std::make_unique<Node>(STree.get(), S);
                _STree->addChild(std::make_unique<Leaf<char>>(_STree.get(), a, 'a'));
                auto _closeTree = std::make_unique<Leaf<char>>(STree.get(), closeParen, ')');

                STree->addChild(std::move(_closeTree));
                STree->addChild(std::move(_STree));
                STree->addChild(std::move(_openTree));
            }
            auto closeTree = std::make_unique<Leaf<char>>(expectTree.get(), closeParen, ')');

            expectTree->addChild(std::move(closeTree));
            expectTree->addChild(std::move(STree));
            expectTree->addChild(std::move(openTree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        EXPECT_EQ(*realTree, *expectTree);
    }
}

TEST(TestLL, Test10) {
    /*
        S -> (E) | E | e;
        E -> i + E | i; /// правая рекурсия!!!
    */
    const Terminal i{"i"}, plus{"+"}, openParen{"("}, closeParen{")"};
    const NonTerminal E{"E"};

    ProductionRules rules;
    rules.newProduction(S) >> openParen >> E >> closeParen >> Or >> E >> Or >> None;
    rules.newProduction(E) >> i >> plus >> E >> Or >> i;

    Parser<char> parser{std::move(rules)};
    // {
    //     const std::array inputString = {'i'};
    //     auto expectTree = std::make_unique<Node>(nullptr, S);
    //     {
    //         auto ETree = std::make_unique<Node>(expectTree.get(), E);
    //         ETree->addChild(std::make_unique<Leaf<char>>(ETree.get(), i, 'i'));

    //         expectTree->addChild(std::move(ETree));
    //     }
    //     auto realTree = parser.buildTree(inputString);
    //     ASSERT_TRUE(realTree);
    //     EXPECT_EQ(*realTree, *expectTree);
    // }
    // {
    //     const std::array inputString = {'i', '+', 'i'};
    //     auto expectTree = std::make_unique<Node>(nullptr, S);
    //     {
    //         auto ETree = std::make_unique<Node>(expectTree.get(), E);
    //         {
    //             auto iTree = std::make_unique<Leaf<char>>(ETree.get(), i, 'i');
    //             auto plusTree = std::make_unique<Leaf<char>>(ETree.get(), plus, '+');
    //             auto _ETree = std::make_unique<Node>(ETree.get(), E);
    //             _ETree->addChild(std::make_unique<Leaf<char>>(_ETree.get(), i, 'i'));

    //             ETree->addChild(std::move(_ETree));
    //             ETree->addChild(std::move(plusTree));
    //             ETree->addChild(std::move(iTree));
    //         }

    //         expectTree->addChild(std::move(ETree));
    //     }
    //     auto realTree = parser.buildTree(inputString);
    //     ASSERT_TRUE(realTree);
    //     EXPECT_EQ(*realTree, *expectTree);
    // }
    // {
    //     const std::array inputString = {'i', '+', '(', 'i', '+', 'i', ')'};
    //     auto expectTree = std::make_unique<Node>(nullptr, S);
    //     {
    //     }
    //     auto realTree = parser.buildTree(inputString);
    //     ASSERT_TRUE(realTree);
    //     EXPECT_EQ(*realTree, *expectTree);
    // }
}

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

        const auto realItems = cgReqHandler.requestClosure(Item{leftS_, *prodsS_.begin()});
        ASSERT_TRUE(realItems.has_value());
        EXPECT_EQ(expectItems, *realItems);
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

        const auto realItems = cgReqHandler.requestClosure(Item{leftS, *prodsS.begin(), 1}); // S -> a .B c;
        ASSERT_TRUE(realItems.has_value());
        EXPECT_EQ(expectItems, *realItems);
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
        const auto I0 = cgReqHandler.requestClosure(startItem); // I0 = { S` -> .S; S -> .a B c; }
        ASSERT_TRUE(I0.has_value());
        const auto I1 = cgReqHandler.requestGoto(*I0, a);
        ASSERT_TRUE(I1.has_value());
        EXPECT_EQ(expectI1, *I1);
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

        const auto I0 = recursiveCgReqHandler.requestClosure(Item{leftS_, *prodsS_.begin()});
        ASSERT_TRUE(I0.has_value());
        EXPECT_EQ(expectI0, *I0);

        Items expectI1; // goto(I0, S) = closure({ S` -> S.; A -> S .b }) = I1 = { S` -> S.; A -> S .b; }
        {
            expectI1.insert(Item{leftS_, *prodsS_.begin(), 1}); // S` -> S.;
            expectI1.insert(Item{leftA, *prodsA.begin(), 1}); // A -> S .b;
        }
        const auto I1 = recursiveCgReqHandler.requestGoto(*I0, S);
        ASSERT_TRUE(I1.has_value());
        EXPECT_EQ(expectI1, *I1);
    }
}

TEST(TestsLR, TestEmptyGrammar) {
    ProductionRules rules;
    ClosureAndGotoReqHandler cgReqHandler{rules};

    Production prod;
    NonTerminal X{"X"};

    EXPECT_THROW(cgReqHandler.requestClosure(Item{X, prod}), BadLR);
}

TEST(TestsLR, TestNonexistentSymbol) {
    const Terminal a{"a"}, b{"b"}, c{"c"};
    const NonTerminal S_{"S`"}, B{"B"}, S{"S"};
    ProductionRules rules;
    rules.newProduction(S_) >> S;
    rules.newProduction(S) >> a >> B >> c;
    rules.newProduction(B) >> b >> Or >> None;

    ClosureAndGotoReqHandler cgReqHandler{rules};

    auto it = rules.begin();
    auto& [leftS_, prodsS_] = *it;
    const Item startItem{leftS_, *prodsS_.begin()};
    const auto I0 = cgReqHandler.requestClosure(startItem);
    ASSERT_TRUE(I0.has_value());
    auto I1 = cgReqHandler.requestGoto(*I0, Terminal{"NonExistTerminal"});
    ASSERT_FALSE(I1.has_value());
}

#endif //! ENABLE_LR_TESTS
