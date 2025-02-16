#include <gtest/gtest.h>

#define ENABLE_LL_TESTS
#if defined(ENABLE_LL_TESTS)

#include "include/cfg/ll.h"

#include <array>

using namespace atom::cfg::grammar;
using namespace atom::cfg::ll;
using namespace atom::ast;


TEST(TestLL, TestParser1) {
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

        expectTree->addChild(std::move(ATree));
        expectTree->addChild(std::move(tTree));
    }
    auto realTree = parser.buildTree(inputString);
    ASSERT_TRUE(realTree);
    EXPECT_EQ(*realTree, *expectTree);
}


TEST(TestLL, TestParser2) {
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

            expectTree->addChild(std::move(ATree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(cTree));
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

            expectTree->addChild(std::move(ATree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(cTree));
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

            expectTree->addChild(std::move(ATree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(cTree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        EXPECT_EQ(*realTree, *expectTree);
    }
}

TEST(TestLL, TestParser3) {
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

            expectTree->addChild(std::move(ATree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(CTree));
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

            expectTree->addChild(std::move(ATree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(CTree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        EXPECT_EQ(*realTree, *expectTree);
    }
}

TEST(TestLL, TestParser4) {
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

            expectTree->addChild(std::move(ATree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(CTree));
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

            expectTree->addChild(std::move(ATree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(CTree));
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

            expectTree->addChild(std::move(ATree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(CTree));
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

            expectTree->addChild(std::move(ATree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(CTree));
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

            expectTree->addChild(std::move(ATree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(CTree));
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

            expectTree->addChild(std::move(ATree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(CTree));
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

            expectTree->addChild(std::move(ATree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(CTree));
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

            expectTree->addChild(std::move(ATree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(CTree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        EXPECT_EQ(*realTree, *expectTree);
    }
}

TEST(TestLL, TestParser5) {
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

            expectTree->addChild(std::move(ATree));
            expectTree->addChild(std::move(bTree));
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

                ATree->addChild(std::move(aTree));
                ATree->addChild(std::move(_ATree));
            }

            auto bTree = std::make_unique<Leaf<char>>(expectTree.get(), b, 'b');

            expectTree->addChild(std::move(ATree));
            expectTree->addChild(std::move(bTree));
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

                    _ATree->addChild(std::move(_aTree));
                    _ATree->addChild(std::move(__ATree));
                }

                ATree->addChild(std::move(aTree));
                ATree->addChild(std::move(_ATree));
            }

            auto bTree = std::make_unique<Leaf<char>>(expectTree.get(), b, 'b');

            expectTree->addChild(std::move(ATree));
            expectTree->addChild(std::move(bTree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        EXPECT_EQ(*realTree, *expectTree);
    }
}

TEST(TestLL, TestParser6) {
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

            expectTree->addChild(std::move(aTree));
            expectTree->addChild(std::move(_STree));
            expectTree->addChild(std::move(bTree));
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

                STree->addChild(std::move(_aTree));
                STree->addChild(std::move(_STree));
                STree->addChild(std::move(_bTree));
            }
            auto bTree = std::make_unique<Leaf<char>>(expectTree.get(), b, 'b');

            expectTree->addChild(std::move(aTree));
            expectTree->addChild(std::move(STree));
            expectTree->addChild(std::move(bTree));
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

            expectTree->addChild(std::move(openTree));
            expectTree->addChild(std::move(STree));
            expectTree->addChild(std::move(closeTree));
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

                STree->addChild(std::move(_openTree));
                STree->addChild(std::move(_STree));
                STree->addChild(std::move(_closeTree));
            }
            auto closeTree = std::make_unique<Leaf<char>>(expectTree.get(), closeParen, ')');

            expectTree->addChild(std::move(openTree));
            expectTree->addChild(std::move(STree));
            expectTree->addChild(std::move(closeTree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        EXPECT_EQ(*realTree, *expectTree);
    }
}

TEST(TestLL, Test8) {
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
        auto expectTree = std::make_unique<Node>(nullptr, S);
        {
            auto ETree = std::make_unique<Node>(expectTree.get(), E);
            ETree->addChild(std::make_unique<Leaf<char>>(ETree.get(), i, 'i'));

            expectTree->addChild(std::move(ETree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'i', '+', 'i'};
        auto expectTree = std::make_unique<Node>(nullptr, S);
        {
            auto ETree = std::make_unique<Node>(expectTree.get(), E);
            {
                auto iTree = std::make_unique<Leaf<char>>(ETree.get(), i, 'i');
                auto plusTree = std::make_unique<Leaf<char>>(ETree.get(), plus, '+');
                auto STree = std::make_unique<Node>(ETree.get(), S);
                {
                    auto _ETree = std::make_unique<Node>(STree.get(), E);
                    _ETree->addChild(std::make_unique<Leaf<char>>(_ETree.get(), i, 'i'));

                    STree->addChild(std::move(_ETree));
                }

                ETree->addChild(std::move(iTree));
                ETree->addChild(std::move(plusTree));
                ETree->addChild(std::move(STree));
            }

            expectTree->addChild(std::move(ETree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'i', '+', 'i', '+', 'i'};
        auto expectTree = std::make_unique<Node>(nullptr, S);
        {
            auto ETree = std::make_unique<Node>(expectTree.get(), E);
            {
                auto iTree = std::make_unique<Leaf<char>>(ETree.get(), i, 'i');
                auto plusTree = std::make_unique<Leaf<char>>(ETree.get(), plus, '+');
                auto STree = std::make_unique<Node>(ETree.get(), S);
                {
                    auto _ETree = std::make_unique<Node>(STree.get(), E);
                    {
                        auto _iTree = std::make_unique<Leaf<char>>(_ETree.get(), i, 'i');
                        auto _plusTree = std::make_unique<Leaf<char>>(_ETree.get(), plus, '+');
                        auto _STree = std::make_unique<Node>(_ETree.get(), S);
                        {
                            auto __ETree = std::make_unique<Node>(_STree.get(), E);
                            __ETree->addChild(std::make_unique<Leaf<char>>(_STree.get(), i, 'i'));

                            _STree->addChild(std::move(__ETree));
                        }

                        _ETree->addChild(std::move(_iTree));
                        _ETree->addChild(std::move(_plusTree));
                        _ETree->addChild(std::move(_STree));
                    }

                    STree->addChild(std::move(_ETree));
                }

                ETree->addChild(std::move(iTree));
                ETree->addChild(std::move(plusTree));
                ETree->addChild(std::move(STree));
            }

            expectTree->addChild(std::move(ETree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'i', '+', '(', 'i', '+', 'i', ')'};
        auto expectTree = std::make_unique<Node>(nullptr, S);
        {
            auto ETree = std::make_unique<Node>(expectTree.get(), E);
            {
                auto iTree = std::make_unique<Leaf<char>>(ETree.get(), i, 'i');
                auto plusTree = std::make_unique<Leaf<char>>(ETree.get(), plus, '+');
                auto STree = std::make_unique<Node>(ETree.get(), S);
                {
                    auto openParenTree = std::make_unique<Leaf<char>>(STree.get(), openParen, '(');
                    auto _ETree = std::make_unique<Node>(STree.get(), E);
                    {
                        auto _iTree = std::make_unique<Leaf<char>>(_ETree.get(), i, 'i');
                        auto _plusTree = std::make_unique<Leaf<char>>(_ETree.get(), plus, '+');
                        auto _STree = std::make_unique<Node>(_ETree.get(), S);
                        {
                            auto __ETree = std::make_unique<Node>(_STree.get(), E);
                            __ETree->addChild(std::make_unique<Leaf<char>>(_STree.get(), i, 'i'));

                            _STree->addChild(std::move(__ETree));
                        }

                        _ETree->addChild(std::move(_iTree));
                        _ETree->addChild(std::move(_plusTree));
                        _ETree->addChild(std::move(_STree));
                    }
                    auto closeParenTree = std::make_unique<Leaf<char>>(STree.get(), closeParen, ')');

                    STree->addChild(std::move(openParenTree));
                    STree->addChild(std::move(_ETree));
                    STree->addChild(std::move(closeParenTree));
                }

                ETree->addChild(std::move(iTree));
                ETree->addChild(std::move(plusTree));
                ETree->addChild(std::move(STree));
            }

            expectTree->addChild(std::move(ETree));
        }
    }
    {
        const std::array inputString = {'(', 'i', '+', 'i', ')', '+', 'i'};
        EXPECT_ANY_THROW(parser.buildTree(inputString));
    }
    {
        const std::array inputString = {'(', 'i', '+', 'i', ')', '+', 'i', '+', 'i'};
        EXPECT_ANY_THROW(parser.buildTree(inputString));
    }
}

TEST(TestLL, Test9) {
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
            auto STree = std::make_unique<Node>(expectTree.get(), S);
            STree->addChild(std::make_unique<Node>(STree.get(), None));

            auto bTree = std::make_unique<Leaf<char>>(expectTree.get(), b, 'b');

            expectTree->addChild(std::move(aTree));
            expectTree->addChild(std::move(STree));
            expectTree->addChild(std::move(bTree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'a', 'a', 'b', 'b'};
        auto expectTree = std::make_unique<Node>(nullptr, S);
        {
            auto aTree1 = std::make_unique<Leaf<char>>(expectTree.get(), a, 'a');
            auto STree1 = std::make_unique<Node>(expectTree.get(), S);
            {
                auto aTree2 = std::make_unique<Leaf<char>>(STree1.get(), a, 'a');
                auto STree2 = std::make_unique<Node>(STree1.get(), S);
                STree2->addChild(std::make_unique<Node>(STree1.get(), None));

                auto bTree2 = std::make_unique<Leaf<char>>(STree1.get(), b, 'b');

                STree1->addChild(std::move(aTree2));
                STree1->addChild(std::move(STree2));
                STree1->addChild(std::move(bTree2));
            }

            auto bTree1 = std::make_unique<Leaf<char>>(expectTree.get(), b, 'b');

            expectTree->addChild(std::move(aTree1));
            expectTree->addChild(std::move(STree1));
            expectTree->addChild(std::move(bTree1));
        }

        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'a', 'a', 'a', 'b', 'b', 'b'};
        auto expectTree = std::make_unique<Node>(nullptr, S);
        {
            auto aTree1 = std::make_unique<Leaf<char>>(expectTree.get(), a, 'a');
            auto STree1 = std::make_unique<Node>(expectTree.get(), S);
            {
                auto aTree2 = std::make_unique<Leaf<char>>(STree1.get(), a, 'a');
                auto STree2 = std::make_unique<Node>(STree1.get(), S);
                {
                    auto aTree3 = std::make_unique<Leaf<char>>(STree2.get(), a, 'a');
                    auto STree3 = std::make_unique<Node>(STree2.get(), S);
                    STree3->addChild(std::make_unique<Node>(STree2.get(), None));

                    auto bTree3 = std::make_unique<Leaf<char>>(STree2.get(), b, 'b');

                    STree2->addChild(std::move(aTree3));
                    STree2->addChild(std::move(STree3));
                    STree2->addChild(std::move(bTree3));
                }

                auto bTree2 = std::make_unique<Leaf<char>>(STree1.get(), b, 'b');

                STree1->addChild(std::move(aTree2));
                STree1->addChild(std::move(STree2));
                STree1->addChild(std::move(bTree2));
            }

            auto bTree1 = std::make_unique<Leaf<char>>(expectTree.get(), b, 'b');

            expectTree->addChild(std::move(aTree1));
            expectTree->addChild(std::move(STree1));
            expectTree->addChild(std::move(bTree1));
        }

        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        EXPECT_EQ(*realTree, *expectTree);
    }
}

TEST(TestLL, Test10) {
    /*
        S -> '(' S ')' | 'a'
    */

    // a, (a), ((a)), (( ... a ... ))
    const Terminal a{"a"}, openParen{"("}, closeParen{")"};

    ProductionRules rules;
    rules.newProduction(S) >> openParen >> S >> closeParen >> Or >> a;

    Parser<char> parser{std::move(rules)};
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

            expectTree->addChild(std::move(openTree));
            expectTree->addChild(std::move(STree));
            expectTree->addChild(std::move(closeTree));
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

                STree->addChild(std::move(_openTree));
                STree->addChild(std::move(_STree));
                STree->addChild(std::move(_closeTree));
            }
            auto closeTree = std::make_unique<Leaf<char>>(expectTree.get(), closeParen, ')');

            expectTree->addChild(std::move(openTree));
            expectTree->addChild(std::move(STree));
            expectTree->addChild(std::move(closeTree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'(', '(', '(', 'a', ')', ')', ')'};
        auto expectTree = std::make_unique<Node>(nullptr, S);
        {
            auto openTree = std::make_unique<Leaf<char>>(expectTree.get(), openParen, '(');
            auto STree = std::make_unique<Node>(expectTree.get(), S);
            {
                auto _openTree = std::make_unique<Leaf<char>>(STree.get(), openParen, '(');
                auto _STree = std::make_unique<Node>(STree.get(), S);
                {
                    auto __openTree = std::make_unique<Leaf<char>>(_STree.get(), openParen, '(');
                    auto __STree = std::make_unique<Node>(_STree.get(), S);
                    __STree->addChild(std::make_unique<Leaf<char>>(__STree.get(), a, 'a'));
                    auto __closeTree = std::make_unique<Leaf<char>>(_STree.get(), closeParen, ')');

                    _STree->addChild(std::move(__openTree));
                    _STree->addChild(std::move(__STree));
                    _STree->addChild(std::move(__closeTree));
                }
                auto _closeTree = std::make_unique<Leaf<char>>(STree.get(), closeParen, ')');

                STree->addChild(std::move(_openTree));
                STree->addChild(std::move(_STree));
                STree->addChild(std::move(_closeTree));
            }
            auto closeTree = std::make_unique<Leaf<char>>(expectTree.get(), closeParen, ')');

            expectTree->addChild(std::move(openTree));
            expectTree->addChild(std::move(STree));
            expectTree->addChild(std::move(closeTree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        EXPECT_EQ(*realTree, *expectTree);
    }
}

TEST(TestLL, TestParrser11) {
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
        const std::array inputString = {'(', 'i', ')'};
        auto expectTree = std::make_unique<Node>(nullptr, S);
        {
            auto ETree = std::make_unique<Node>(expectTree.get(), E);
            {
                auto TTree = std::make_unique<Node>(ETree.get(), T);
                {
                    auto openParenTree = std::make_unique<Leaf<char>>(TTree.get(), openParen, '(');
                    auto _ETree = std::make_unique<Node>(TTree.get(), E);
                    {
                        auto _TTree = std::make_unique<Node>(_ETree.get(), T);
                        _TTree->addChild(std::make_unique<Leaf<char>>(_TTree.get(), i, 'i'));
                        auto _E_Tree = std::make_unique<Node>(_ETree.get(), E_);
                        _E_Tree->addChild(std::make_unique<Node>(_ETree.get(), None));

                        _ETree->addChild(std::move(_TTree));
                        _ETree->addChild(std::move(_E_Tree));
                    }
                    auto closeParenTree = std::make_unique<Leaf<char>>(TTree.get(), closeParen, ')');

                    TTree->addChild(std::move(openParenTree));
                    TTree->addChild(std::move(_ETree));
                    TTree->addChild(std::move(closeParenTree));
                }
                auto E_Tree = std::make_unique<Node>(ETree.get(), E_);
                E_Tree->addChild(std::make_unique<Node>(E_Tree.get(), None));

                ETree->addChild(std::move(TTree));
                ETree->addChild(std::move(E_Tree));
            }

            expectTree->addChild(std::move(ETree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'i'};
        auto expectTree = std::make_unique<Node>(nullptr, S);
        {
            auto ETree = std::make_unique<Node>(expectTree.get(), E);
            {
                auto TTree = std::make_unique<Node>(ETree.get(), T);
                TTree->addChild(std::make_unique<Leaf<char>>(TTree.get(), i, 'i'));
                auto E_Tree = std::make_unique<Node>(ETree.get(), E_);
                E_Tree->addChild(std::make_unique<Node>(E_Tree.get(), None));

                ETree->addChild(std::move(TTree));
                ETree->addChild(std::move(E_Tree));
            }

            expectTree->addChild(std::move(ETree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'i', '+', 'i', '+', 'i'};
        auto expectTree = std::make_unique<Node>(nullptr, S);
        {
            auto ETree = std::make_unique<Node>(expectTree.get(), E);
            {
                auto TTree1 = std::make_unique<Node>(ETree.get(), T);
                TTree1->addChild(std::make_unique<Leaf<char>>(TTree1.get(), i, 'i'));

                auto E_Tree1 = std::make_unique<Node>(ETree.get(), E_);
                {
                    auto plusTree1 = std::make_unique<Leaf<char>>(E_Tree1.get(), plus, '+');
                    auto TTree2 = std::make_unique<Node>(E_Tree1.get(), T);
                    TTree2->addChild(std::make_unique<Leaf<char>>(TTree2.get(), i, 'i'));

                    auto E_Tree2 = std::make_unique<Node>(E_Tree1.get(), E_);
                    {
                        auto plusTree2 = std::make_unique<Leaf<char>>(E_Tree2.get(), plus, '+');
                        auto TTree3 = std::make_unique<Node>(E_Tree2.get(), T);
                        TTree3->addChild(std::make_unique<Leaf<char>>(TTree3.get(), i, 'i'));

                        auto E_Tree3 = std::make_unique<Node>(E_Tree2.get(), E_);
                        E_Tree3->addChild(std::make_unique<Node>(E_Tree3.get(), None));

                        E_Tree2->addChild(std::move(plusTree2));
                        E_Tree2->addChild(std::move(TTree3));
                        E_Tree2->addChild(std::move(E_Tree3));
                    }

                    E_Tree1->addChild(std::move(plusTree1));
                    E_Tree1->addChild(std::move(TTree2));
                    E_Tree1->addChild(std::move(E_Tree2));
                }

                ETree->addChild(std::move(TTree1));
                ETree->addChild(std::move(E_Tree1));
            }

            expectTree->addChild(std::move(ETree));
        }

        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'(', 'i', '+', '(', 'i', '+', '(', 'i', ')', ')'};
        auto expectTree = std::make_unique<Node>(nullptr, S);
        {
            auto ETree = std::make_unique<Node>(expectTree.get(), E);
            {
                auto TTree = std::make_unique<Node>(ETree.get(), T);
                {
                    auto openParenTree = std::make_unique<Leaf<char>>(TTree.get(), openParen, '(');
                    auto _ETree = std::make_unique<Node>(TTree.get(), E);
                    {
                        auto _TTree = std::make_unique<Node>(_ETree.get(), T);
                        _TTree->addChild(std::make_unique<Leaf<char>>(_TTree.get(), i, 'i'));
                        auto _E_Tree = std::make_unique<Node>(_ETree.get(), E_);
                        {
                            auto plusTree = std::make_unique<Leaf<char>>(_E_Tree.get(), plus, '+');
                            auto __TTree = std::make_unique<Node>(_E_Tree.get(), T);
                            {
                                auto _openParenTree = std::make_unique<Leaf<char>>(__TTree.get(), openParen, '(');
                                auto __ETree = std::make_unique<Node>(__TTree.get(), E);
                                {
                                    auto ___TTree = std::make_unique<Node>(__ETree.get(), T);
                                    ___TTree->addChild(std::make_unique<Leaf<char>>(___TTree.get(), i, 'i'));
                                    auto __E_Tree = std::make_unique<Node>(__ETree.get(), E_);
                                    {
                                        auto _plusTree = std::make_unique<Leaf<char>>(__E_Tree.get(), plus, '+');
                                        auto ____TTree = std::make_unique<Node>(__E_Tree.get(), T);
                                        {
                                            auto ____ETree = std::make_unique<Node>(____TTree.get(), E);
                                            {
                                                auto ____TTree = std::make_unique<Node>();
                                                ____TTree->addChild(std::make_unique<Leaf<char>>(____TTree.get(), i, 'i'));

                                                ____ETree->addChild(std::move(____TTree));
                                            }

                                            ____TTree->addChild(std::move(____ETree));
                                        }
                                        auto ___E_Tree = std::make_unique<Node>(__E_Tree.get(), E_);

                                        __E_Tree->addChild(std::move(_plusTree));
                                        __E_Tree->addChild(std::move(____TTree));
                                        __E_Tree->addChild(std::move(___E_Tree));
                                    }

                                    __ETree->addChild(std::move(___TTree));
                                    __ETree->addChild(std::move(__E_Tree));
                                }
                                auto _closeParenTree = std::make_unique<Leaf<char>>(__TTree.get(), closeParen, ')');

                                __TTree->addChild(std::move(_openParenTree));
                                __TTree->addChild(std::move(__ETree));
                                __TTree->addChild(std::move(_closeParenTree));
                            }
                            auto __E_Tree = std::make_unique<Node>(_E_Tree.get(), E_);
                            __E_Tree->addChild(std::make_unique<Node>(__E_Tree.get(), None));

                            _E_Tree->addChild(std::move(plusTree));
                            _E_Tree->addChild(std::move(__TTree));
                            _E_Tree->addChild(std::move(__E_Tree));
                        }

                        _ETree->addChild(std::move(_TTree));
                        _ETree->addChild(std::move(_E_Tree));
                    }
                    auto closeParenTree = std::make_unique<Leaf<char>>(TTree.get(), closeParen, ')');

                    TTree->addChild(std::move(openParenTree));
                    TTree->addChild(std::move(_ETree));
                    TTree->addChild(std::move(closeParenTree));
                }
                auto E_Tree = std::make_unique<Node>(ETree.get(), E_);
                E_Tree->addChild(std::make_unique<Node>(E_Tree.get(), None));

                ETree->addChild(std::move(TTree));
                ETree->addChild(std::move(E_Tree));
            }

            expectTree->addChild(std::move(ETree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_TRUE(realTree);
        //EXPECT_EQ(*realTree, *expectTree);
    }
}

TEST(TestGrammar, TTTT) {
    /*
        S -> A B C;
        A -> a;
        B -> b | e;
        C -> c;
    */
    const Terminal a{"a"}, b{"b"}, c{"c"};
    const NonTerminal A{"A"}, B{"B"}, C{"C"};

    ProductionRules rules;
    rules.newProduction(S) >> A >> B >> C;
    rules.newProduction(A) >> a;
    rules.newProduction(B) >> b >> Or >> None;
    rules.newProduction(C) >> c;

    FirstAndFollowReqHandler ffReqHandler{ rules };
    {
        const auto expect = FirstAndFollowReqHandler::SymbolSetType{ b, c };
        const auto real = ffReqHandler.requestFirst(B);
        EXPECT_EQ(real, expect);
    }
}

TEST(TestGrammar, TestFirstAndFollow0) {
    /*
        S -> A t;
        A -> a;
    */

    // FIRST(A) = { a }
    // FIRST(t) = { t }

    // FOLLOW(S) = { $ }
    // FOLLOW(A) = { t }

    // Terminal symbols = { a, t }
    const Terminal a{"a"}, t{"t"};

    // Non terminal symbols = { S, A }
    const NonTerminal A{"A"};

    ProductionRules rules;
    rules.newProduction(S) >> A >> t;
    rules.newProduction(A) >> a;

    FirstAndFollowReqHandler ffReqHandler{ rules };
    {
        auto it = rules.find(S);
        {
            ASSERT_NE(it, rules.end());
            const auto& [left, prods] = *it;
            ASSERT_EQ(left, S);
            Production _prod{}; _prod >> A >> t; Productions _prods{}; _prods.pushBack(_prod);
            ASSERT_EQ(prods, _prods);
        }
        ++it;
        {
            ASSERT_NE(it, rules.end());
            const auto& [left, prods] = *it;
            ASSERT_EQ(left, A);
            Production _prod{}; _prod >> a; Productions _prods{}; _prods.pushBack(_prod);
            ASSERT_EQ(prods, _prods);
        }
    }
    {
        const auto& first = ffReqHandler.requestFirst(A);
        ASSERT_FALSE(first.empty());
        EXPECT_EQ(*first.cbegin(), a);
    }
    {
        const auto& first = ffReqHandler.requestFirst(S);
        ASSERT_FALSE(first.empty());
        EXPECT_EQ(*first.cbegin(), a);
    }
    {
        const auto& first = ffReqHandler.requestFirst(t);
        ASSERT_FALSE(first.empty());
        EXPECT_EQ(*first.cbegin(), t);
    }
    {
        const auto& first = ffReqHandler.requestFirst(a);
        ASSERT_FALSE(first.empty());
        EXPECT_EQ(*first.cbegin(), a);
    }

    {
        const auto& follow = ffReqHandler.requestFollow(A);
        ASSERT_FALSE(follow.empty());
        EXPECT_EQ(*follow.cbegin(), t);
    }
    {
        const auto& follow = ffReqHandler.requestFollow(S);
        ASSERT_FALSE(follow.empty());
        EXPECT_EQ(*follow.cbegin(), End);
    }
    {
        const auto& follow = ffReqHandler.requestFollow(t);
        EXPECT_TRUE(follow.empty());
    }
    {
        const auto& follow = ffReqHandler.requestFollow(a);
        EXPECT_TRUE(follow.empty());
    }
}

TEST(TestGrammar, TestFirstAndFollow1) {
    /*
        S -> A t;
        A -> a | e;
    */

    // FIRST(S) = FIRST(A) = { a, t }
    // FIRST(A) = { a, t }
    // FIRST(t) = { t }

    // FOLLOW(A) = { t }
    // FOLLOW(S) = { $ }

    // Terminal symbols = { a, t }
    const Terminal a{"a"}, t{"t"};

    // Non terminal symbols = { S, A }
    const NonTerminal A{"A"};

    ProductionRules rules;
    rules.newProduction(S) >> A >> t;
    rules.newProduction(A) >> a >> Or >> None;
    FirstAndFollowReqHandler ffReqHandler{ rules };
    using FirstSetType = FirstAndFollowReqHandler::SymbolSetType;
    {
        auto it = rules.find(S);
        {
            ASSERT_NE(it, rules.end());
            const auto& [left, prods] = *it;
            ASSERT_EQ(left, S);
            Production _prod{}; _prod >> A >> t; Productions _prods{}; _prods.pushBack(_prod);
            ASSERT_EQ(prods, _prods);
        }
        ++it;
        {
            ASSERT_NE(it, rules.end());
            const auto& [left, prods] = *it;
            ASSERT_EQ(left, A);
            Production _firstProd{}; _firstProd >> a; Production _secondProd{}; _secondProd >> None;
            Productions _prods{}; _prods >> _firstProd >> _secondProd;
            ASSERT_EQ(prods, _prods);
        }
    }
    {
        const auto expect = FirstSetType{ a, t };
        const auto real = ffReqHandler.requestFirst(A);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FirstSetType{ a, t };
        const auto real = ffReqHandler.requestFirst(S);
        EXPECT_EQ(real, expect);
    }

    {
        const auto& follow = ffReqHandler.requestFollow(A);
        ASSERT_FALSE(follow.empty());
        EXPECT_EQ(*follow.cbegin(), t);
    }
    {
        const auto& follow = ffReqHandler.requestFollow(S);
        ASSERT_FALSE(follow.empty());
        EXPECT_EQ(*follow.cbegin(), End);
    }
    {
        const auto& follow = ffReqHandler.requestFollow(a);
        EXPECT_TRUE(follow.empty());
    }
}

TEST(TestGrammar, TestFirstAndFollow2) {
    /*
        S -> A B C;
        A -> a | e;
        B -> b | e;
        C -> c | e;
    */

    // FIRST(S) = { a, b, c, ε }
    // FIRST(A) = { a, b, c, ε }
    // FIRST(B) = { b, c, ε }
    // FIRST(C) = { c, ε }

    // FOLLOW(S) = { $ }
    // FOLLOW(A) = { b, c, $ }
    // FOLLOW(B) = { c, $ }
    // FOLLOW(C) = { $ }

    // Terminal symbols = { a, b, c }
    const Terminal a{"a"}, b{"b"}, c{"c"};

    // Non terminal symbols = { S, A, B, C }
    const NonTerminal A{"A"}, B{"B"}, C{"C"};

    ProductionRules rules;
    rules.newProduction(S) >> A >> B >> C;
    rules.newProduction(A) >> a >> Or >> None;
    rules.newProduction(B) >> b >> Or >> None;
    rules.newProduction(C) >> c >> Or >> None;
    FirstAndFollowReqHandler ffReqHandler{ rules };
    using FirstSetType = FirstAndFollowReqHandler::SymbolSetType;
    using FollowSetType = FirstAndFollowReqHandler::SymbolSetType;
    {
        const auto expect = FirstSetType{ c, None };
        const auto real = ffReqHandler.requestFirst(C);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FirstSetType{ b, c, None };
        const auto real = ffReqHandler.requestFirst(B);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FirstSetType{ a, b, c, None };
        const auto real = ffReqHandler.requestFirst(A);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FirstSetType{ a, b, c, None };
        const auto real = ffReqHandler.requestFirst(S);
        EXPECT_EQ(real, expect);
    }

    {
        const auto expect = FollowSetType{ End };
        const auto real = ffReqHandler.requestFollow(S);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FollowSetType{ b, c, End };
        const auto real = ffReqHandler.requestFollow(A);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FollowSetType{ c, End };
        const auto real = ffReqHandler.requestFollow(B);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FollowSetType{ End };
        const auto real = ffReqHandler.requestFollow(C);
        EXPECT_EQ(real, expect);
    }
    {
        const auto follow = ffReqHandler.requestFollow(a);
        EXPECT_TRUE(follow.empty());
    }
}

TEST(TestGrammar, TestFirst3) {
    /*
        S -> A B C
        A -> a
        B -> b
        C -> c
    */
    // FIRST(S) = { a }
    // FIRST(A) = { a }
    // FIRST(B) = { b }
    // FIRST(C) = { c }

    // FOLLOW(S) = { $ }
    // FOLLOW(A) = { b }
    // FOLLOW(B) = { c }
    // FOLLOW(C) = { $ }

    // Terminal symbols = { a, b, c }
    const Terminal a{"a"}, b{"b"}, c{"c"};

    // Non terminal symbols = { S, A, B, C }
    const NonTerminal A{"A"}, B{"B"}, C{"C"};

    ProductionRules rules;
    rules.newProduction(S) >> A >> B >> C;
    rules.newProduction(A) >> a;
    rules.newProduction(B) >> b;
    rules.newProduction(C) >> c;
    FirstAndFollowReqHandler ffReqHandler{ rules };
    using FirstSetType = FirstAndFollowReqHandler::SymbolSetType;
    using FollowSetType = FirstAndFollowReqHandler::SymbolSetType;
    {
        const auto expect = FirstSetType{ c };
        const auto real = ffReqHandler.requestFirst(C);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FirstSetType{ b };
        const auto real = ffReqHandler.requestFirst(B);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FirstSetType{ a };
        const auto real = ffReqHandler.requestFirst(A);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FirstSetType{ a };
        const auto real = ffReqHandler.requestFirst(S);
        EXPECT_EQ(real, expect);
    }

    {
        const auto expect = FollowSetType{ End };
        const auto real = ffReqHandler.requestFollow(S);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FollowSetType{ b };
        const auto real = ffReqHandler.requestFollow(A);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FollowSetType{ c };
        const auto real = ffReqHandler.requestFollow(B);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FollowSetType{ End };
        const auto real = ffReqHandler.requestFollow(C);
        EXPECT_EQ(real, expect);
    }
}

TEST(TestGrammar, TestFirst4) {
    /*
        S -> A B | C D
        A -> a | ε
        B -> b
        C -> c
        D -> d | ε
    */

    // FIRST(S) = { a, b, c }
    // FIRST(A) = { a, b }
    // FIRST(B) = { b }
    // FIRST(C) = { c }
    // FIRST(D) = { d, ε }

    // FOLLOW(S) = { $ }
    // FOLLOW(A) = { b }
    // FOLLOW(B) = { $ }
    // FOLLOW(C) = { d, $ }
    // FOLLOW(D) = { $ }

    // Terminal symbols = { a, b, c }
    const Terminal a{"a"}, b{"b"}, c{"c"}, d{"d"};

    // Non terminal symbols = { S, A, B, C }
    const NonTerminal A{"A"}, B{"B"}, C{"C"}, D{"D"};

    ProductionRules rules;
    rules.newProduction(S) >> A >> B >> Or >> C >> D;
    rules.newProduction(A) >> a >> Or >> None;
    rules.newProduction(B) >> b;
    rules.newProduction(C) >> c;
    rules.newProduction(D) >> d >> Or >> None;
    FirstAndFollowReqHandler ffReqHandler{ rules };
    using FirstSetType = FirstAndFollowReqHandler::SymbolSetType;
    using FollowSetType = FirstAndFollowReqHandler::SymbolSetType;
    {
        const auto expect = FirstSetType{ d, None };
        const auto real = ffReqHandler.requestFirst(D);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FirstSetType{ c };
        const auto real = ffReqHandler.requestFirst(C);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FirstSetType{ b };
        const auto real = ffReqHandler.requestFirst(B);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FirstSetType{ a, b };
        const auto real = ffReqHandler.requestFirst(A);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FirstSetType{ a, b, c };
        const auto real = ffReqHandler.requestFirst(S);
        EXPECT_EQ(real, expect);
    }

    {
        const auto expect = FollowSetType{ End };
        const auto real = ffReqHandler.requestFollow(S);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FollowSetType{ b };
        const auto real = ffReqHandler.requestFollow(A);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FollowSetType{ End };
        const auto real = ffReqHandler.requestFollow(B);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FollowSetType{ d, End };
        const auto real = ffReqHandler.requestFollow(C);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FollowSetType{ End };
        const auto real = ffReqHandler.requestFollow(D);
        EXPECT_EQ(real, expect);
    }
}

#endif //! #if defined(ENABLE_LL_TESTS)
