#include <gtest/gtest.h>

#include "include/cfg/ll.h"

#include <array>

using namespace atom::cfg::grammar;
using namespace atom::cfg::ll;
using namespace atom::ast;

using NodePtrType = atom::memory::NonAtomicSharedPtr<Node>;
using SymbolType = atom::cfg::grammar::Symbol;

NodePtrType MakeNode(NodePtrType parent, const SymbolType& symbol) {
    return NodePtrType::Make(parent, symbol);
}

NodePtrType MakeLeaf(NodePtrType parent, const SymbolType& symbol, const char token) {
    return atom::memory::NonAtomicSharedPtr<Leaf<char>>::Make(parent, static_cast<Terminal>(symbol), token);
}

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
    auto expectTree = MakeNode(NodePtrType{}, S);
    {
        auto ATree = MakeNode(expectTree, A);
        ATree->addChild(MakeLeaf(ATree, a, 'a'));

        auto tTree = MakeLeaf(expectTree, t, 't');

        expectTree->addChild(std::move(ATree));
        expectTree->addChild(std::move(tTree));
    }
    auto realTree = parser.buildTree(inputString);
    ASSERT_NE(realTree, nullptr);
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
        auto expectTree = MakeNode(NodePtrType{}, S);
        {
            auto ATree = MakeNode(expectTree, A);
            ATree->addChild(MakeLeaf(ATree, a, 'a'));

            auto BTree = MakeNode(expectTree, B);
            BTree->addChild(MakeLeaf(BTree, b, 'b'));

            auto cTree = MakeLeaf(expectTree, c, 'c');

            expectTree->addChild(std::move(ATree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(cTree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'a', 'c'};
        auto expectTree = MakeNode(NodePtrType{}, S);
        {
            auto ATree = MakeNode(expectTree, A);
            ATree->addChild(MakeLeaf(ATree, a, 'a'));

            auto BTree = MakeNode(expectTree, B);
            BTree->addChild(MakeNode(BTree, None));

            auto cTree = MakeLeaf(expectTree, c, 'c');

            expectTree->addChild(std::move(ATree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(cTree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'c'};
        auto expectTree = MakeNode(NodePtrType{}, S);
        {
            auto ATree = MakeNode(expectTree, A);
            ATree->addChild(MakeNode(ATree, None));

            auto BTree = MakeNode(expectTree, B);
            BTree->addChild(MakeNode(BTree, None));

            auto cTree = MakeLeaf(expectTree, c, 'c');

            expectTree->addChild(std::move(ATree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(cTree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
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
    const NonTerminal A{"A"}, B{"B"}, C{"C"};
    ProductionRules rules;
    rules.newProduction(S) >> A >> B >> C;
    rules.newProduction(A) >> a;
    rules.newProduction(B) >> b >> Or >> None;
    rules.newProduction(C) >> c;
    Parser<char> parser{std::move(rules)};
    {
        const std::array inputString = {'a', 'b', 'c'};
        auto expectTree = MakeNode(NodePtrType{}, S);
        {
            auto ATree = MakeNode(expectTree, A);
            ATree->addChild(MakeLeaf(ATree, a, 'a'));

            auto BTree = MakeNode(expectTree, B);
            BTree->addChild(MakeLeaf(BTree, b, 'b'));

            auto CTree = MakeNode(expectTree, C);
            CTree->addChild(MakeLeaf(CTree, c, 'c'));

            expectTree->addChild(std::move(ATree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(CTree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'a', 'c'};
        auto expectTree = MakeNode(NodePtrType{}, S);
        {
            auto ATree = MakeNode(expectTree, A);
            ATree->addChild(MakeLeaf(ATree, a, 'a'));

            auto BTree = MakeNode(expectTree, B);
            BTree->addChild(MakeNode(BTree, None));

            auto CTree = MakeNode(expectTree, C);
            CTree->addChild(MakeLeaf(CTree, c, 'c'));

            expectTree->addChild(std::move(ATree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(CTree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
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
    const NonTerminal A{"A"}, B{"B"}, C{"C"};
    ProductionRules rules;
    rules.newProduction(S) >> A >> B >> C;
    rules.newProduction(A) >> a >> Or >> None;
    rules.newProduction(B) >> b >> Or >> None;
    rules.newProduction(C) >> c >> Or >> None;
    Parser<char> parser{std::move(rules)};
    {
        const std::array inputString = {'a', 'b', 'c'};
        auto expectTree = MakeNode(NodePtrType{}, S);
        {
            auto ATree = MakeNode(expectTree, A);
            ATree->addChild(MakeLeaf(ATree, a, 'a'));
            auto BTree = MakeNode(expectTree, B);
            BTree->addChild(MakeLeaf(BTree, b, 'b'));
            auto CTree = MakeNode(expectTree, C);
            CTree->addChild(MakeLeaf(CTree, c, 'c'));

            expectTree->addChild(std::move(ATree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(CTree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'a', 'b'};
        auto expectTree = MakeNode(NodePtrType{}, S);
        {
            auto ATree = MakeNode(expectTree, A);
            ATree->addChild(MakeLeaf(ATree, a, 'a'));
            auto BTree = MakeNode(expectTree, B);
            BTree->addChild(MakeLeaf(BTree, b, 'b'));
            auto CTree = MakeNode(expectTree, C);
            CTree->addChild(MakeNode(CTree, None));

            expectTree->addChild(std::move(ATree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(CTree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'a', 'c'};
        auto expectTree = MakeNode(NodePtrType{}, S);
        {
            auto ATree = MakeNode(expectTree, A);
            ATree->addChild(MakeLeaf(ATree, a, 'a'));
            auto BTree = MakeNode(expectTree, B);
            BTree->addChild(MakeNode(BTree, None));
            auto CTree = MakeNode(expectTree, C);
            CTree->addChild(MakeLeaf(CTree, c, 'c'));

            expectTree->addChild(std::move(ATree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(CTree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'b', 'c'};
        auto expectTree = MakeNode(NodePtrType{}, S);
        {
            auto ATree = MakeNode(expectTree, A);
            ATree->addChild(MakeNode(ATree, None));
            auto BTree = MakeNode(expectTree, B);
            BTree->addChild(MakeLeaf(BTree, b, 'b'));
            auto CTree = MakeNode(expectTree, C);
            CTree->addChild(MakeLeaf(CTree, c, 'c'));

            expectTree->addChild(std::move(ATree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(CTree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'a'};
        auto expectTree = MakeNode(NodePtrType{}, S);
        {
            auto ATree = MakeNode(expectTree, A);
            ATree->addChild(MakeLeaf(ATree, a, 'a'));
            auto BTree = MakeNode(expectTree, B);
            BTree->addChild(MakeNode(BTree, None));
            auto CTree = MakeNode(expectTree, C);
            CTree->addChild(MakeNode(CTree, None));

            expectTree->addChild(std::move(ATree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(CTree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'b'};
        auto expectTree = MakeNode(NodePtrType{}, S);
        {
            auto ATree = MakeNode(expectTree, A);
            ATree->addChild(MakeNode(ATree, None));
            auto BTree = MakeNode(expectTree, B);
            BTree->addChild(MakeLeaf(BTree, b, 'b'));
            auto CTree = MakeNode(expectTree, C);
            CTree->addChild(MakeNode(CTree, None));

            expectTree->addChild(std::move(ATree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(CTree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'c'};
        auto expectTree = MakeNode(NodePtrType{}, S);
        {
            auto ATree = MakeNode(expectTree, A);
            ATree->addChild(MakeNode(ATree, None));
            auto BTree = MakeNode(expectTree, B);
            BTree->addChild(MakeNode(BTree, None));
            auto CTree = MakeNode(expectTree, C);
            CTree->addChild(MakeLeaf(CTree, c, 'c'));

            expectTree->addChild(std::move(ATree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(CTree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array<char, 0> inputString = {};
        auto expectTree = MakeNode(NodePtrType{}, S);
        {
            auto ATree = MakeNode(expectTree, A);
            ATree->addChild(MakeNode(ATree, None));
            auto BTree = MakeNode(expectTree, B);
            BTree->addChild(MakeNode(BTree, None));
            auto CTree = MakeNode(expectTree, C);
            CTree->addChild(MakeNode(CTree, None));

            expectTree->addChild(std::move(ATree));
            expectTree->addChild(std::move(BTree));
            expectTree->addChild(std::move(CTree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
        EXPECT_EQ(*realTree, *expectTree);
    }
}

TEST(TestLL, TestParser5) {
    /*
        S -> A b;
        A -> a A | e;
    */
    const Terminal a{"a"}, b{"b"};
    const NonTerminal A{"A"};
    ProductionRules rules;
    rules.newProduction(S) >> A >> b;
    rules.newProduction(A) >> a >> A >> Or >> None;
    Parser<char> parser{std::move(rules)};
    {
        const std::array inputString = {'b'};
        auto expectTree = MakeNode(NodePtrType{}, S);
        {
            auto ATree = MakeNode(expectTree, A);
            ATree->addChild(MakeNode(ATree, None));

            auto bTree = MakeLeaf(expectTree, b, 'b');

            expectTree->addChild(std::move(ATree));
            expectTree->addChild(std::move(bTree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'a', 'b'};
        auto expectTree = MakeNode(NodePtrType{}, S);
        {
            auto ATree = MakeNode(expectTree, A);
            {
                auto aTree = MakeLeaf(ATree, a, 'a');
                auto _ATree = MakeNode(ATree, A);
                _ATree->addChild(MakeNode(ATree, None));

                ATree->addChild(std::move(aTree));
                ATree->addChild(std::move(_ATree));
            }

            auto bTree = MakeLeaf(expectTree, b, 'b');

            expectTree->addChild(std::move(ATree));
            expectTree->addChild(std::move(bTree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'a', 'a', 'b'};
        auto expectTree = MakeNode(NodePtrType{}, S);
        {
            auto ATree = MakeNode(expectTree, A);
            {
                auto aTree = MakeLeaf(ATree, a, 'a');
                auto _ATree = MakeNode(ATree, A);
                {
                    auto _aTree = MakeLeaf(_ATree, a, 'a');
                    auto __ATree = MakeNode(_ATree, A);
                    __ATree->addChild(MakeNode(__ATree, None));

                    _ATree->addChild(std::move(_aTree));
                    _ATree->addChild(std::move(__ATree));
                }

                ATree->addChild(std::move(aTree));
                ATree->addChild(std::move(_ATree));
            }

            auto bTree = MakeLeaf(expectTree, b, 'b');

            expectTree->addChild(std::move(ATree));
            expectTree->addChild(std::move(bTree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
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
    rules.newProduction(S) >> a >> S >> b >> Or >> None;
    Parser<char> parser{std::move(rules)};
    {
        const std::array inputString = {'a', 'b'};
        auto expectTree = MakeNode(NodePtrType{}, S);
        {
            auto aTree = MakeLeaf(expectTree, a, 'a');
            auto _STree = MakeNode(expectTree, S);
            _STree->addChild(MakeNode(_STree, None));
            auto bTree = MakeLeaf(expectTree, b, 'b');

            expectTree->addChild(std::move(aTree));
            expectTree->addChild(std::move(_STree));
            expectTree->addChild(std::move(bTree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'a', 'a', 'b', 'b'};
        auto expectTree = MakeNode(NodePtrType{}, S);
        {
            auto aTree = MakeLeaf(expectTree, a, 'a');
            auto STree = MakeNode(expectTree, S);
            {
                auto _aTree = MakeLeaf(STree, a, 'a');
                auto _STree = MakeNode(STree, S);
                _STree->addChild(MakeNode(_STree, None));
                auto _bTree = MakeLeaf(STree, b, 'b');

                STree->addChild(std::move(_aTree));
                STree->addChild(std::move(_STree));
                STree->addChild(std::move(_bTree));
            }
            auto bTree = MakeLeaf(expectTree, b, 'b');

            expectTree->addChild(std::move(aTree));
            expectTree->addChild(std::move(STree));
            expectTree->addChild(std::move(bTree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
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
    Parser<char> parser{std::move(rules)};
    {
        const std::array inputString = {'a'};
        auto expectTree = MakeNode(NodePtrType{}, S);
        expectTree->addChild(MakeLeaf(expectTree, a, 'a'));
        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'(', 'a', ')'};
        auto expectTree = MakeNode(NodePtrType{}, S);
        {
            auto openTree = MakeLeaf(expectTree, openParen, '(');
            auto STree = MakeNode(expectTree, S);
            STree->addChild(MakeLeaf(STree, a, 'a'));
            auto closeTree = MakeLeaf(expectTree, closeParen, ')');

            expectTree->addChild(std::move(openTree));
            expectTree->addChild(std::move(STree));
            expectTree->addChild(std::move(closeTree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'(', '(', 'a', ')', ')'};
        auto expectTree = MakeNode(NodePtrType{}, S);
        {
            auto openTree = MakeLeaf(expectTree, openParen, '(');
            auto STree = MakeNode(expectTree, S);
            {
                auto _openTree = MakeLeaf(STree, openParen, '(');
                auto _STree = MakeNode(STree, S);
                _STree->addChild(MakeLeaf(_STree, a, 'a'));
                auto _closeTree = MakeLeaf(STree, closeParen, ')');

                STree->addChild(std::move(_openTree));
                STree->addChild(std::move(_STree));
                STree->addChild(std::move(_closeTree));
            }
            auto closeTree = MakeLeaf(expectTree, closeParen, ')');

            expectTree->addChild(std::move(openTree));
            expectTree->addChild(std::move(STree));
            expectTree->addChild(std::move(closeTree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
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
        auto expectTree = MakeNode(NodePtrType{}, S);
        {
            auto ETree = MakeNode(expectTree, E);
            ETree->addChild(MakeLeaf(ETree, i, 'i'));

            expectTree->addChild(std::move(ETree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'i', '+', 'i'};
        auto expectTree = MakeNode(NodePtrType{}, S);
        {
            auto ETree = MakeNode(expectTree, E);
            {
                auto iTree = MakeLeaf(ETree, i, 'i');
                auto plusTree = MakeLeaf(ETree, plus, '+');
                auto STree = MakeNode(ETree, S);
                {
                    auto _ETree = MakeNode(STree, E);
                    _ETree->addChild(MakeLeaf(_ETree, i, 'i'));

                    STree->addChild(std::move(_ETree));
                }

                ETree->addChild(std::move(iTree));
                ETree->addChild(std::move(plusTree));
                ETree->addChild(std::move(STree));
            }

            expectTree->addChild(std::move(ETree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'i', '+', 'i', '+', 'i'};
        auto expectTree = MakeNode(NodePtrType{}, S);
        {
            auto ETree = MakeNode(expectTree, E);
            {
                auto iTree = MakeLeaf(ETree, i, 'i');
                auto plusTree = MakeLeaf(ETree, plus, '+');
                auto STree = MakeNode(ETree, S);
                {
                    auto _ETree = MakeNode(STree, E);
                    {
                        auto _iTree = MakeLeaf(_ETree, i, 'i');
                        auto _plusTree = MakeLeaf(_ETree, plus, '+');
                        auto _STree = MakeNode(_ETree, S);
                        {
                            auto __ETree = MakeNode(_STree, E);
                            __ETree->addChild(MakeLeaf(__ETree, i, 'i'));

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
        ASSERT_NE(realTree, nullptr);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'i', '+', '(', 'i', '+', 'i', ')'};
        auto expectTree = MakeNode(NodePtrType{}, S);
        {
            auto ETree = MakeNode(expectTree, E);
            {
                auto iTree = MakeLeaf(ETree, i, 'i');
                auto plusTree = MakeLeaf(ETree, plus, '+');
                auto STree = MakeNode(ETree, S);
                {
                    auto openParenTree = MakeLeaf(STree, openParen, '(');
                    auto _ETree = MakeNode(STree, E);
                    {
                        auto _iTree = MakeLeaf(_ETree, i, 'i');
                        auto _plusTree = MakeLeaf(_ETree, plus, '+');
                        auto _STree = MakeNode(_ETree, S);
                        {
                            auto __ETree = MakeNode(_STree, E);
                            __ETree->addChild(MakeLeaf(__ETree, i, 'i'));

                            _STree->addChild(std::move(__ETree));
                        }

                        _ETree->addChild(std::move(_iTree));
                        _ETree->addChild(std::move(_plusTree));
                        _ETree->addChild(std::move(_STree));
                    }
                    auto closeParenTree = MakeLeaf(STree, closeParen, ')');

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
        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
        EXPECT_EQ(*realTree, *expectTree);
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
    rules.newProduction(S) >> a >> S >> b >> Or >> None;

    Parser<char> parser{std::move(rules)};
    {
        const std::array inputString = {'a', 'b'};
        auto expectTree = MakeNode(NodePtrType{}, S);
        {
            auto aTree = MakeLeaf(expectTree, a, 'a');
            auto STree = MakeNode(expectTree, S);
            STree->addChild(MakeNode(STree, None));

            auto bTree = MakeLeaf(expectTree, b, 'b');

            expectTree->addChild(std::move(aTree));
            expectTree->addChild(std::move(STree));
            expectTree->addChild(std::move(bTree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'a', 'a', 'b', 'b'};
        auto expectTree = MakeNode(NodePtrType{}, S);
        {
            auto aTree1 = MakeLeaf(expectTree, a, 'a');
            auto STree1 = MakeNode(expectTree, S);
            {
                auto aTree2 = MakeLeaf(STree1, a, 'a');
                auto STree2 = MakeNode(STree1, S);
                STree2->addChild(MakeNode(STree1, None));

                auto bTree2 = MakeLeaf(STree1, b, 'b');

                STree1->addChild(std::move(aTree2));
                STree1->addChild(std::move(STree2));
                STree1->addChild(std::move(bTree2));
            }

            auto bTree1 = MakeLeaf(expectTree, b, 'b');

            expectTree->addChild(std::move(aTree1));
            expectTree->addChild(std::move(STree1));
            expectTree->addChild(std::move(bTree1));
        }

        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'a', 'a', 'a', 'b', 'b', 'b'};
        auto expectTree = MakeNode(NodePtrType{}, S);
        {
            auto aTree1 = MakeLeaf(expectTree, a, 'a');
            auto STree1 = MakeNode(expectTree, S);
            {
                auto aTree2 = MakeLeaf(STree1, a, 'a');
                auto STree2 = MakeNode(STree1, S);
                {
                    auto aTree3 = MakeLeaf(STree2, a, 'a');
                    auto STree3 = MakeNode(STree2, S);
                    STree3->addChild(MakeNode(STree2, None));

                    auto bTree3 = MakeLeaf(STree2, b, 'b');

                    STree2->addChild(std::move(aTree3));
                    STree2->addChild(std::move(STree3));
                    STree2->addChild(std::move(bTree3));
                }

                auto bTree2 = MakeLeaf(STree1, b, 'b');

                STree1->addChild(std::move(aTree2));
                STree1->addChild(std::move(STree2));
                STree1->addChild(std::move(bTree2));
            }

            auto bTree1 = MakeLeaf(expectTree, b, 'b');

            expectTree->addChild(std::move(aTree1));
            expectTree->addChild(std::move(STree1));
            expectTree->addChild(std::move(bTree1));
        }

        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
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
        auto expectTree = MakeNode(NodePtrType{}, S);
        expectTree->addChild(MakeLeaf(expectTree, a, 'a'));
        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'(', 'a', ')'};
        auto expectTree = MakeNode(NodePtrType{}, S);
        {
            auto openTree = MakeLeaf(expectTree, openParen, '(');
            auto STree = MakeNode(expectTree, S);
            STree->addChild(MakeLeaf(STree, a, 'a'));
            auto closeTree = MakeLeaf(expectTree, closeParen, ')');

            expectTree->addChild(std::move(openTree));
            expectTree->addChild(std::move(STree));
            expectTree->addChild(std::move(closeTree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'(', '(', 'a', ')', ')'};
        auto expectTree = MakeNode(NodePtrType{}, S);
        {
            auto openTree = MakeLeaf(expectTree, openParen, '(');
            auto STree = MakeNode(expectTree, S);
            {
                auto _openTree = MakeLeaf(STree, openParen, '(');
                auto _STree = MakeNode(STree, S);
                _STree->addChild(MakeLeaf(_STree, a, 'a'));
                auto _closeTree = MakeLeaf(STree, closeParen, ')');

                STree->addChild(std::move(_openTree));
                STree->addChild(std::move(_STree));
                STree->addChild(std::move(_closeTree));
            }
            auto closeTree = MakeLeaf(expectTree, closeParen, ')');

            expectTree->addChild(std::move(openTree));
            expectTree->addChild(std::move(STree));
            expectTree->addChild(std::move(closeTree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'(', '(', '(', 'a', ')', ')', ')'};
        auto expectTree = MakeNode(NodePtrType{}, S);
        {
            auto openTree = MakeLeaf(expectTree, openParen, '(');
            auto STree = MakeNode(expectTree, S);
            {
                auto _openTree = MakeLeaf(STree, openParen, '(');
                auto _STree = MakeNode(STree, S);
                {
                    auto __openTree = MakeLeaf(_STree, openParen, '(');
                    auto __STree = MakeNode(_STree, S);
                    __STree->addChild(MakeLeaf(__STree, a, 'a'));
                    auto __closeTree = MakeLeaf(_STree, closeParen, ')');

                    _STree->addChild(std::move(__openTree));
                    _STree->addChild(std::move(__STree));
                    _STree->addChild(std::move(__closeTree));
                }
                auto _closeTree = MakeLeaf(STree, closeParen, ')');

                STree->addChild(std::move(_openTree));
                STree->addChild(std::move(_STree));
                STree->addChild(std::move(_closeTree));
            }
            auto closeTree = MakeLeaf(expectTree, closeParen, ')');

            expectTree->addChild(std::move(openTree));
            expectTree->addChild(std::move(STree));
            expectTree->addChild(std::move(closeTree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
        EXPECT_EQ(*realTree, *expectTree);
    }
}

TEST(TestLL, TestParser11) {
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
        auto expectTree = MakeNode(NodePtrType{}, S);
        {
            auto ETree = MakeNode(expectTree, E);
            {
                auto TTree = MakeNode(ETree, T);
                {
                    auto openParenTree = MakeLeaf(TTree, openParen, '(');
                    auto _ETree = MakeNode(TTree, E);
                    {
                        auto _TTree = MakeNode(_ETree, T);
                        _TTree->addChild(MakeLeaf(_TTree, i, 'i'));
                        auto _E_Tree = MakeNode(_ETree, E_);
                        _E_Tree->addChild(MakeNode(_E_Tree, None));

                        _ETree->addChild(std::move(_TTree));
                        _ETree->addChild(std::move(_E_Tree));
                    }
                    auto closeParenTree = MakeLeaf(TTree, closeParen, ')');

                    TTree->addChild(std::move(openParenTree));
                    TTree->addChild(std::move(_ETree));
                    TTree->addChild(std::move(closeParenTree));
                }
                auto E_Tree = MakeNode(ETree, E_);
                E_Tree->addChild(MakeNode(E_Tree, None));

                ETree->addChild(std::move(TTree));
                ETree->addChild(std::move(E_Tree));
            }

            expectTree->addChild(std::move(ETree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'i'};
        auto expectTree = MakeNode(NodePtrType{}, S);
        {
            auto ETree = MakeNode(expectTree, E);
            {
                auto TTree = MakeNode(ETree, T);
                TTree->addChild(MakeLeaf(TTree, i, 'i'));
                auto E_Tree = MakeNode(ETree, E_);
                E_Tree->addChild(MakeNode(E_Tree, None));

                ETree->addChild(std::move(TTree));
                ETree->addChild(std::move(E_Tree));
            }

            expectTree->addChild(std::move(ETree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'i', '+', 'i', '+', 'i'};
        auto expectTree = MakeNode(NodePtrType{}, S);
        {
            auto ETree = MakeNode(expectTree, E);
            {
                auto TTree1 = MakeNode(ETree, T);
                TTree1->addChild(MakeLeaf(TTree1, i, 'i'));

                auto E_Tree1 = MakeNode(ETree, E_);
                {
                    auto plusTree1 = MakeLeaf(E_Tree1, plus, '+');
                    auto TTree2 = MakeNode(E_Tree1, T);
                    TTree2->addChild(MakeLeaf(TTree2, i, 'i'));

                    auto E_Tree2 = MakeNode(E_Tree1, E_);
                    {
                        auto plusTree2 = MakeLeaf(E_Tree2, plus, '+');
                        auto TTree3 = MakeNode(E_Tree2, T);
                        TTree3->addChild(MakeLeaf(TTree3, i, 'i'));

                        auto E_Tree3 = MakeNode(E_Tree2, E_);
                        E_Tree3->addChild(MakeNode(E_Tree3, None));

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
        ASSERT_NE(realTree, nullptr);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'(', 'i', '+', '(', 'i', '+', '(', 'i', ')', ')', ')'};
        auto expectTree = MakeNode(NodePtrType{}, S);
        {
            auto ETree = MakeNode(expectTree, E);
            {
                auto TTree = MakeNode(ETree, T);
                {
                    auto openParenLeaf = MakeLeaf(TTree, openParen, '(');
                    auto innerETree = MakeNode(TTree, E);
                    {
                        auto firstTTree = MakeNode(innerETree, T);
                        firstTTree->addChild(MakeLeaf(firstTTree, i, 'i'));

                        auto firstE_PrimeTree = MakeNode(innerETree, E_);
                        {
                            auto plusLeaf = MakeLeaf(firstE_PrimeTree, plus, '+');

                            auto secondTTree = MakeNode(firstE_PrimeTree, T);
                            {
                                auto secondOpenParenLeaf = MakeLeaf(secondTTree, openParen, '(');

                                auto secondInnerETree = MakeNode(secondTTree, E);
                                {
                                    auto thirdTTree = MakeNode(secondInnerETree, T);
                                    thirdTTree->addChild(MakeLeaf(thirdTTree, i, 'i'));

                                    auto secondE_PrimeTree = MakeNode(secondInnerETree, E_);
                                    {
                                        auto secondPlusLeaf = MakeLeaf(secondE_PrimeTree, plus, '+');

                                        auto thirdInnerTTree = MakeNode(secondE_PrimeTree, T);
                                        {
                                            auto thirdOpenParenLeaf = MakeLeaf(thirdInnerTTree, openParen, '(');

                                            auto thirdInnerETree = MakeNode(thirdInnerTTree, E);
                                            {
                                                auto fourthTTree = MakeNode(thirdInnerETree, T);
                                                fourthTTree->addChild(MakeLeaf(fourthTTree, i, 'i'));

                                                auto thirdE_PrimeTree = MakeNode(thirdInnerETree, E_);
                                                thirdE_PrimeTree->addChild(MakeNode(thirdE_PrimeTree, None));

                                                thirdInnerETree->addChild(std::move(fourthTTree));
                                                thirdInnerETree->addChild(std::move(thirdE_PrimeTree));
                                            }

                                            auto thirdCloseParenLeaf = MakeLeaf(thirdInnerTTree, closeParen, ')');

                                            thirdInnerTTree->addChild(std::move(thirdOpenParenLeaf));
                                            thirdInnerTTree->addChild(std::move(thirdInnerETree));
                                            thirdInnerTTree->addChild(std::move(thirdCloseParenLeaf));
                                        }

                                        auto fourthE_PrimeTree = MakeNode(secondE_PrimeTree, E_);
                                        fourthE_PrimeTree->addChild(MakeNode(fourthE_PrimeTree, None));

                                        secondE_PrimeTree->addChild(std::move(secondPlusLeaf));
                                        secondE_PrimeTree->addChild(std::move(thirdInnerTTree));
                                        secondE_PrimeTree->addChild(std::move(fourthE_PrimeTree));
                                    }

                                    secondInnerETree->addChild(std::move(thirdTTree));
                                    secondInnerETree->addChild(std::move(secondE_PrimeTree));
                                }

                                auto secondCloseParenLeaf = MakeLeaf(secondTTree, closeParen, ')');

                                secondTTree->addChild(std::move(secondOpenParenLeaf));
                                secondTTree->addChild(std::move(secondInnerETree));
                                secondTTree->addChild(std::move(secondCloseParenLeaf));
                            }

                            auto thirdE_PrimeTree = MakeNode(firstE_PrimeTree, E_);
                            thirdE_PrimeTree->addChild(MakeNode(thirdE_PrimeTree, None));

                            firstE_PrimeTree->addChild(std::move(plusLeaf));
                            firstE_PrimeTree->addChild(std::move(secondTTree));
                            firstE_PrimeTree->addChild(std::move(thirdE_PrimeTree));
                        }

                        innerETree->addChild(std::move(firstTTree));
                        innerETree->addChild(std::move(firstE_PrimeTree));
                    }

                    auto closeParenLeaf = MakeLeaf(TTree, closeParen, ')');

                    TTree->addChild(std::move(openParenLeaf));
                    TTree->addChild(std::move(innerETree));
                    TTree->addChild(std::move(closeParenLeaf));
                }

                auto outerE_PrimeTree = MakeNode(ETree, E_);
                outerE_PrimeTree->addChild(MakeNode(outerE_PrimeTree, None));

                ETree->addChild(std::move(TTree));
                ETree->addChild(std::move(outerE_PrimeTree));
            }

            expectTree->addChild(std::move(ETree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
        EXPECT_EQ(*realTree, *expectTree);
    }
}

TEST(TestLL, TestParser12) {
    /*
        S -> I D;
        D -> I D | .I D | e;
        I -> 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9;
    */
    const Terminal dot{"."}, i0{"0"}, i1{"1"}, i2{"2"}, i3{"3"}, i4{"4"},
        i5{"5"}, i6{"6"}, i7{"7"}, i8{"8"}, i9{"9"};
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
        const std::array inputString = {'1', '2', '3'};
        auto expectTree = MakeNode(NodePtrType{}, S);
        {
            auto ITree = MakeNode(expectTree, I);
            ITree->addChild(MakeLeaf(ITree, i1, '1'));
            auto DTree = MakeNode(expectTree, D);
            {
                auto _ITree = MakeNode(DTree, I);
                _ITree->addChild(MakeLeaf(_ITree, i2, '2'));
                auto _DTree = MakeNode(DTree, D);
                {
                    auto __ITree = MakeNode(_DTree, I);
                    __ITree->addChild(MakeLeaf(__ITree, i3, '3'));
                    auto __DTree = MakeNode(_DTree, D);
                    __DTree->addChild(MakeNode(__DTree, None));

                    _DTree->addChild(std::move(__ITree));
                    _DTree->addChild(std::move(__DTree));
                }

                DTree->addChild(std::move(_ITree));
                DTree->addChild(std::move(_DTree));
            }

            expectTree->addChild(std::move(ITree));
            expectTree->addChild(std::move(DTree));
        }
        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
        EXPECT_EQ(*realTree, *expectTree);
    }
    {
        const std::array inputString = {'1', '.', '3'};
        auto expectTree = MakeNode(NodePtrType{}, S);
        {
            auto ITree = MakeNode(expectTree, I);
            ITree->addChild(MakeLeaf(ITree, i1, '1'));

            auto DTree = MakeNode(expectTree, D);
            {
                DTree->addChild(MakeLeaf(DTree, dot, '.'));

                auto _ITree = MakeNode(DTree, I);
                _ITree->addChild(MakeLeaf(_ITree, i3, '3'));

                auto _DTree = MakeNode(DTree, D);
                _DTree->addChild(MakeNode(_DTree, None));

                DTree->addChild(std::move(_ITree));
                DTree->addChild(std::move(_DTree));
            }

            expectTree->addChild(std::move(ITree));
            expectTree->addChild(std::move(DTree));
        }

        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
        EXPECT_EQ(*realTree, *expectTree);
    }
}

TEST(TestLL, Test13) {
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
        const std::array inputString = {'{','}'};
        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
    }
    {
        const std::array inputString = {'{', 'v', '}'};
        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
    }
    {
        const std::array inputString = {'{', 'v', ',', 'v', '}'};
        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
    }
    {
        const std::array inputString = {'{', 'v', ',', 'v', ',', '{', 'v', '}', '}'};
        auto realTree = parser.buildTree(inputString);
        ASSERT_NE(realTree, nullptr);
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
