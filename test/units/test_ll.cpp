#include <gtest/gtest.h>

#include "include/cfg/ll.h"

#include <array>

using namespace atom::cfg::grammar;
using namespace atom::cfg::ll;
using namespace atom::ast;

using ParserType = Parser<char>;

Terminal TokenToTerminal(const char& s) {
    std::string data;
    data.push_back(s);
    return Terminal{std::move(data)};
}

TEST(TestLL, Test1) {
    /*
        S -> A t;
        A -> a;
    */

    const Terminal a{"a"}, t{"t"};
    const NonTerminal A{"A"};

    const std::array inputString = {'a', 't'};

    ProductionRules rules;
    rules.newProduction(S) >> A >> t;
    rules.newProduction(A) >> a;

    ParserType parser{std::move(rules), TokenToTerminal};
    auto root = parser.buildTree(inputString);

    ASSERT_TRUE(root);
    EXPECT_EQ(root->getSymbol(), S);
    const auto& childs = root->getChilds();
    ASSERT_EQ(childs.size(), 2);
    EXPECT_EQ(childs[0]->getSymbol(), A);
    EXPECT_EQ(childs[1]->getSymbol(), t);
}

TEST(TestLL, Test2) {
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

    ParserType parser{std::move(rules), TokenToTerminal};
    {
        const std::array inputString = {'a', 'c'};
        auto root = parser.buildTree(inputString);
        ASSERT_TRUE(root);

        EXPECT_EQ(root->getSymbol(), S);
        {
            const auto& childs = root->getChilds(); // S -> A B C;
            ASSERT_EQ(childs.size(), 3);
            EXPECT_EQ(childs[0]->getSymbol(), A);
            EXPECT_EQ(childs[1]->getSymbol(), B);
            EXPECT_EQ(childs[2]->getSymbol(), C);
        }
        {
            const auto& childs = root->getChilds()[0]->getChilds(); // A -> a;
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), a);
        }
        {
            const auto& childs = root->getChilds()[1]->getChilds(); // B -> e;
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), None);
        }
        {
            const auto& childs = root->getChilds()[2]->getChilds(); // C -> c;
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), c);
        }
    }
    {
        const std::array inputString = {'a', 'b', 'c'};
        auto root = parser.buildTree(inputString);
        ASSERT_TRUE(root);

        EXPECT_EQ(root->getSymbol(), S);
        {
            const auto& childs = root->getChilds(); // S -> A B C;
            ASSERT_EQ(childs.size(), 3);
            EXPECT_EQ(childs[0]->getSymbol(), A);
            EXPECT_EQ(childs[1]->getSymbol(), B);
            EXPECT_EQ(childs[2]->getSymbol(), C);
        }
        {
            const auto& childs = root->getChilds()[0]->getChilds(); // A -> a;
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), a);
        }
        {
            const auto& childs = root->getChilds()[1]->getChilds(); // B -> b;
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), b);
        }
        {
            const auto& childs = root->getChilds()[2]->getChilds(); // C -> c;
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), c);
        }
    }
}

TEST(TestLL, Test3) {
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

    ParserType parser{std::move(rules), TokenToTerminal};
    {
        const std::array inputString = {'a', 'b', 'c'};
        auto root = parser.buildTree(inputString);
        ASSERT_TRUE(root);

        EXPECT_EQ(root->getSymbol(), S);
        {
            const auto& childs = root->getChilds(); // S -> A B C;
            ASSERT_EQ(childs.size(), 3);
            EXPECT_EQ(childs[0]->getSymbol(), A);
            EXPECT_EQ(childs[1]->getSymbol(), B);
            EXPECT_EQ(childs[2]->getSymbol(), C);
        }
        {
            const auto& childs = root->getChilds()[0]->getChilds(); // A -> a;
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), a);
        }
        {
            const auto& childs = root->getChilds()[1]->getChilds(); // B -> b;
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), b);
        }
        {
            const auto& childs = root->getChilds()[2]->getChilds(); // C -> c;
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), c);
        }
    }
    {
        const std::array inputString = {'a', 'c'};
        auto root = parser.buildTree(inputString);
        ASSERT_TRUE(root);

        EXPECT_EQ(root->getSymbol(), S);
        {
            const auto& childs = root->getChilds(); // S -> A B C;
            ASSERT_EQ(childs.size(), 3);
            EXPECT_EQ(childs[0]->getSymbol(), A);
            EXPECT_EQ(childs[1]->getSymbol(), B);
            EXPECT_EQ(childs[2]->getSymbol(), C);
        }
        {
            const auto& childs = root->getChilds()[0]->getChilds(); // A -> a;
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), a);
        }
        {
            const auto& childs = root->getChilds()[1]->getChilds(); // B -> e;
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), None);
        }
        {
            const auto& childs = root->getChilds()[2]->getChilds(); // C -> c;
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), c);
        }
    }
}

TEST(TestLL, Test4) {
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

    ParserType parser{std::move(rules), TokenToTerminal};
    {
        const std::array inputString = {'a', 'b', 'c'};
        auto root = parser.buildTree(inputString);
        ASSERT_TRUE(root);
        EXPECT_EQ(root->getSymbol(), S);
        {
            const auto& childs = root->getChilds(); // S -> A B C;
            ASSERT_EQ(childs.size(), 3);
            EXPECT_EQ(childs[0]->getSymbol(), A);
            EXPECT_EQ(childs[1]->getSymbol(), B);
            EXPECT_EQ(childs[2]->getSymbol(), C);
        }
        {
            const auto& childs = root->getChilds()[0]->getChilds(); // A -> a;
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), a);
        }
        {
            const auto& childs = root->getChilds()[1]->getChilds(); // B -> b;
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), b);
        }
        {
            const auto& childs = root->getChilds()[2]->getChilds(); // C -> c;
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), c);
        }
    }
    {
        const std::array inputString = {'b', 'c'};
        auto root = parser.buildTree(inputString);
        ASSERT_TRUE(root);
        EXPECT_EQ(root->getSymbol(), S);
        {
            const auto& childs = root->getChilds(); // S -> A B C;
            ASSERT_EQ(childs.size(), 3);
            EXPECT_EQ(childs[0]->getSymbol(), A);
            EXPECT_EQ(childs[1]->getSymbol(), B);
            EXPECT_EQ(childs[2]->getSymbol(), C);
        }
        {
            const auto& childs = root->getChilds()[0]->getChilds(); // A -> e;
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), None);
        }
        {
            const auto& childs = root->getChilds()[1]->getChilds(); // B -> b;
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), b);
        }
        {
            const auto& childs = root->getChilds()[2]->getChilds(); // C -> c;
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), c);
        }
    }
    {
        const std::array inputString = {'c'};
        auto root = parser.buildTree(inputString);
        ASSERT_TRUE(root);
        EXPECT_EQ(root->getSymbol(), S);
        {
            const auto& childs = root->getChilds(); // S -> A B C;
            ASSERT_EQ(childs.size(), 3);
            EXPECT_EQ(childs[0]->getSymbol(), A);
            EXPECT_EQ(childs[1]->getSymbol(), B);
            EXPECT_EQ(childs[2]->getSymbol(), C);
        }
        {
            const auto& childs = root->getChilds()[0]->getChilds(); // A -> e;
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), None);
        }
        {
            const auto& childs = root->getChilds()[1]->getChilds(); // B -> e;
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), None);
        }
        {
            const auto& childs = root->getChilds()[2]->getChilds(); // C -> c;
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), c);
        }
    }
    {
        const std::array<char, 0> inputString;
        auto root = parser.buildTree(inputString);
        ASSERT_TRUE(root);
        EXPECT_EQ(root->getSymbol(), S);
        {
            const auto& childs = root->getChilds(); // S -> A B C;
            ASSERT_EQ(childs.size(), 3);
            EXPECT_EQ(childs[0]->getSymbol(), A);
            EXPECT_EQ(childs[1]->getSymbol(), B);
            EXPECT_EQ(childs[2]->getSymbol(), C);
        }
        {
            const auto& childs = root->getChilds()[0]->getChilds(); // A -> e;
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), None);
        }
        {
            const auto& childs = root->getChilds()[1]->getChilds(); // B -> e;
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), None);
        }
        {
            const auto& childs = root->getChilds()[2]->getChilds(); // C -> e;
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), None);
        }
    }
}

TEST(TestLL, Test5) {
    /*
        S -> A b;
        A -> a A | e;
    */

    // a^nb (ab, aab, aaab, aaa...b)

    const Terminal a{"a"}, b{"b"};
    const NonTerminal A{"A"};

    ProductionRules rules;
    rules.newProduction(S) >> A >> b;
    rules.newProduction(A) >> a >> A >> Or >> None;

    ParserType parser{std::move(rules), TokenToTerminal};
    {
        const std::array inputString = {'a', 'b'};
        auto root = parser.buildTree(inputString);
        ASSERT_TRUE(root);
        EXPECT_EQ(root->getSymbol(), S);
        {
            const auto& childs = root->getChilds(); // S -> A b;
            ASSERT_EQ(childs.size(), 2);
            EXPECT_EQ(childs[0]->getSymbol(), A);
            EXPECT_EQ(childs[1]->getSymbol(), b);
        }
        {
            const auto& childs = root->getChilds()[0]->getChilds(); // A -> a A;
            ASSERT_EQ(childs.size(), 2);
            EXPECT_EQ(childs[0]->getSymbol(), a);
            EXPECT_EQ(childs[1]->getSymbol(), A);
        }
        {
            const auto& childs = root->getChilds()[0]->getChilds()[1]->getChilds(); // A -> e
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), None);
        }
    }
    {
        const std::array inputString = {'a', 'a', 'b'};
        auto root = parser.buildTree(inputString);
        ASSERT_TRUE(root);
        EXPECT_EQ(root->getSymbol(), S);
        {
            const auto& childs = root->getChilds(); // S -> A b;
            ASSERT_EQ(childs.size(), 2);
            EXPECT_EQ(childs[0]->getSymbol(), A);
            EXPECT_EQ(childs[1]->getSymbol(), b);
        }
        {
            const auto& childs = root->getChilds()[0]->getChilds(); // A -> a A;
            ASSERT_EQ(childs.size(), 2);
            EXPECT_EQ(childs[0]->getSymbol(), a);
            EXPECT_EQ(childs[1]->getSymbol(), A);
        }
        {
            const auto& childs = root->getChilds()[0]->getChilds()[1]->getChilds(); // A -> a A;
            ASSERT_EQ(childs.size(), 2);
            EXPECT_EQ(childs[0]->getSymbol(), a);
            EXPECT_EQ(childs[1]->getSymbol(), A);
        }
        {
            const auto& childs = root->getChilds()[0]->getChilds()[1]->getChilds()[1]->getChilds(); // A -> e;
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), None);
        }
    }
}


TEST(TestLL, Test6) {
    /*
        S -> a S b | ε
    */

    // a^n b^n (ab, aabb, aaabbb)

    const Terminal a{"a"}, b{"b"};

    ProductionRules rules;
    rules.newProduction(S) >> a >> S >>  b >> Or >> None;

    ParserType parser{std::move(rules), TokenToTerminal};
    {
        const std::array inputString = {'a', 'b'};
        auto root = parser.buildTree(inputString);
        ASSERT_TRUE(root);
        EXPECT_EQ(root->getSymbol(), S);
        {
            const auto& childs = root->getChilds(); // S -> a S b;
            ASSERT_EQ(childs.size(), 3);
            EXPECT_EQ(childs[0]->getSymbol(), a);
            EXPECT_EQ(childs[1]->getSymbol(), S);
            EXPECT_EQ(childs[2]->getSymbol(), b);
        }
        {
            const auto& childs = root->getChilds()[1]->getChilds(); // S -> e;
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), None);
        }
    }
    {
        const std::array inputString = {'a', 'a', 'b', 'b'};
        auto root = parser.buildTree(inputString);
        ASSERT_TRUE(root);
        {
            const auto& childs = root->getChilds(); // S -> a S b;
            ASSERT_EQ(childs.size(), 3);
            EXPECT_EQ(childs[0]->getSymbol(), a);
            EXPECT_EQ(childs[1]->getSymbol(), S);
            EXPECT_EQ(childs[2]->getSymbol(), b);
        }
        {
            const auto& childs = root->getChilds()[1]->getChilds(); // S -> a S b;
            ASSERT_EQ(childs.size(), 3);
            EXPECT_EQ(childs[0]->getSymbol(), a);
            EXPECT_EQ(childs[1]->getSymbol(), S);
            EXPECT_EQ(childs[2]->getSymbol(), b);
        }
        {
            const auto& childs = root->getChilds()[1]->getChilds()[1]->getChilds(); // S -> e;
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), None);
        }
    }
}

TEST(TestLL, Test7) {
    /*
        S -> a S b | ε
    */

    // a^n b^n (ab, aabb, aaabbb)

    const Terminal a{"a"}, b{"b"};

    ProductionRules rules;
    rules.newProduction(S) >> a >> S >> b >> Or >> None;

    ParserType parser{std::move(rules), TokenToTerminal};
    {
        const std::array inputString = {'a', 'b'};
        auto root = parser.buildTree(inputString);
        ASSERT_TRUE(root);
        EXPECT_EQ(root->getSymbol(), S);
        {
            const auto& childs = root->getChilds(); // S -> a S b;
            ASSERT_EQ(childs.size(), 3);
            EXPECT_EQ(childs[0]->getSymbol(), a);
            EXPECT_EQ(childs[1]->getSymbol(), S);
            EXPECT_EQ(childs[2]->getSymbol(), b);
        }
        {
            const auto& childs = root->getChilds()[1]->getChilds(); // S -> ε
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), None);
        }
    }
    {
        const std::array inputString = {'a', 'a', 'b', 'b'};
        auto root = parser.buildTree(inputString);
        ASSERT_TRUE(root);
        EXPECT_EQ(root->getSymbol(), S);
        {
            const auto& childs = root->getChilds(); // S -> a S b;
            ASSERT_EQ(childs.size(), 3);
            EXPECT_EQ(childs[0]->getSymbol(), a);
            EXPECT_EQ(childs[1]->getSymbol(), S);
            EXPECT_EQ(childs[2]->getSymbol(), b);
        }
        {
            const auto& childs = root->getChilds()[1]->getChilds(); // S -> a S b;
            ASSERT_EQ(childs.size(), 3);
            EXPECT_EQ(childs[0]->getSymbol(), a);
            EXPECT_EQ(childs[1]->getSymbol(), S);
            EXPECT_EQ(childs[2]->getSymbol(), b);
        }
        {
            const auto& childs = root->getChilds()[1]->getChilds()[1]->getChilds(); // S -> ε
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), None);
        }
    }
    {
        const std::array inputString = {'a', 'a', 'a', 'b', 'b', 'b'};
        auto root = parser.buildTree(inputString);
        ASSERT_TRUE(root);
        EXPECT_EQ(root->getSymbol(), S);
        {
            const auto& childs = root->getChilds(); // S -> a S b;
            ASSERT_EQ(childs.size(), 3);
            EXPECT_EQ(childs[0]->getSymbol(), a);
            EXPECT_EQ(childs[1]->getSymbol(), S);
            EXPECT_EQ(childs[2]->getSymbol(), b);
        }
        {
            const auto& childs = root->getChilds()[1]->getChilds(); // S -> a S b;
            ASSERT_EQ(childs.size(), 3);
            EXPECT_EQ(childs[0]->getSymbol(), a);
            EXPECT_EQ(childs[1]->getSymbol(), S);
            EXPECT_EQ(childs[2]->getSymbol(), b);
        }
        {
            const auto& childs = root->getChilds()[1]->getChilds()[1]->getChilds(); // S -> a S b;
            ASSERT_EQ(childs.size(), 3);
            EXPECT_EQ(childs[0]->getSymbol(), a);
            EXPECT_EQ(childs[1]->getSymbol(), S);
            EXPECT_EQ(childs[2]->getSymbol(), b);
        }
        {
            const auto& childs = root->getChilds()[1]->getChilds()[1]->getChilds()[1]->getChilds(); // S -> ε
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), None);
        }
    }
}

TEST(TestLL, Test8) {
    /*
        S -> i T
        T -> + S T | * S T | ε
    */

    const Terminal i{"i"}, plus{"+"}, mult{"*"};
    const NonTerminal T{"T"};

    ProductionRules rules;
    rules.newProduction(S) >> i >> T;
    rules.newProduction(T) >> plus >> S >> T >> Or >> mult >> S >> T >> Or >> None;

    ParserType parser{std::move(rules), TokenToTerminal};
    {
        const std::array inputString = {'i'};
        auto root = parser.buildTree(inputString);
        ASSERT_TRUE(root);
        EXPECT_EQ(root->getSymbol(), S);
        {
            const auto& childs = root->getChilds(); // S -> i T
            ASSERT_EQ(childs.size(), 2);
            EXPECT_EQ(childs[0]->getSymbol(), i);
            EXPECT_EQ(childs[1]->getSymbol(), T);
        }
        {
            const auto& childs = root->getChilds()[1]->getChilds(); // T -> ε
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), None);
        }
    }
    {
        const std::array inputString = {'i', '+', 'i'};
        auto root = parser.buildTree(inputString);
        ASSERT_TRUE(root);
        EXPECT_EQ(root->getSymbol(), S);
        {
            const auto& childs = root->getChilds(); // S -> i T
            ASSERT_EQ(childs.size(), 2);
            EXPECT_EQ(childs[0]->getSymbol(), i);
            EXPECT_EQ(childs[1]->getSymbol(), T);
        }
        {
            const auto& childs = root->getChilds()[1]->getChilds(); // T -> + S T
            ASSERT_EQ(childs.size(), 3);
            EXPECT_EQ(childs[0]->getSymbol(), plus);
            EXPECT_EQ(childs[1]->getSymbol(), S);
            EXPECT_EQ(childs[2]->getSymbol(), T);
        }
        {
            const auto& childs = root->getChilds()[1]->getChilds()[1]->getChilds(); // S -> i T
            ASSERT_EQ(childs.size(), 2);
            EXPECT_EQ(childs[0]->getSymbol(), i);
            EXPECT_EQ(childs[1]->getSymbol(), T);
        }
        {
            const auto& childs = root->getChilds()[1]->getChilds()[1]->getChilds()[1]->getChilds(); // T -> ε
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), None);
        }
        {
            const auto& childs = root->getChilds()[1]->getChilds()[2]->getChilds(); // T -> ε
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), None);
        }
    }
    {
        const std::array inputString = {'i', '*', 'i', '+', 'i'};
        auto root = parser.buildTree(inputString);
        ASSERT_TRUE(root);
        EXPECT_EQ(root->getSymbol(), S);
        {
            const auto& childs = root->getChilds(); // S -> i T
            ASSERT_EQ(childs.size(), 2);
            EXPECT_EQ(childs[0]->getSymbol(), i);
            EXPECT_EQ(childs[1]->getSymbol(), T);
        }
        {
            const auto& childs = root->getChilds()[1]->getChilds(); // T -> * S T
            ASSERT_EQ(childs.size(), 3);
            EXPECT_EQ(childs[0]->getSymbol(), mult);
            EXPECT_EQ(childs[1]->getSymbol(), S);
            EXPECT_EQ(childs[2]->getSymbol(), T);
        }
        {
            const auto& childs = root->getChilds()[1]->getChilds()[1]->getChilds(); // S -> i T
            ASSERT_EQ(childs.size(), 2);
            EXPECT_EQ(childs[0]->getSymbol(), i);
            EXPECT_EQ(childs[1]->getSymbol(), T);
        }
        {
            const auto& childs = root->getChilds()[1]->getChilds()[1]->getChilds()[1]->getChilds(); // T -> + S T
            ASSERT_EQ(childs.size(), 3);
            EXPECT_EQ(childs[0]->getSymbol(), plus);
            EXPECT_EQ(childs[1]->getSymbol(), S);
            EXPECT_EQ(childs[2]->getSymbol(), T);
        }
        {
            const auto& childs = root->getChilds()[1]->getChilds()[1]->getChilds()[1]->getChilds()[1]->getChilds(); // S -> i T
            ASSERT_EQ(childs.size(), 2);
            EXPECT_EQ(childs[0]->getSymbol(), i);
            EXPECT_EQ(childs[1]->getSymbol(), T);
        }
        {
            const auto& childs = root->getChilds()[1]->getChilds()[1]->getChilds()[1]->getChilds()[1]->getChilds()[1]->getChilds(); // T -> ε
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), None);
        }
        {
            const auto& childs = root->getChilds()[1]->getChilds()[1]->getChilds()[1]->getChilds()[2]->getChilds(); // T -> ε
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), None);
        }
    }
}

TEST(TestLL, Test9) {
    /*
        S -> '(' S ')' | 'a'
    */

    const Terminal a{"a"}, openParen{"("}, closeParen{")"};

    ProductionRules rules;
    rules.newProduction(S) >> openParen >> S >> closeParen >> Or >> a;

    ParserType parser{std::move(rules), TokenToTerminal};
    {
        const std::array inputString = {'a'};
        auto root = parser.buildTree(inputString);
        ASSERT_TRUE(root);
        EXPECT_EQ(root->getSymbol(), S);
        {
            const auto& childs = root->getChilds(); // S -> 'a'
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), a);
        }
    }
    {
        const std::array inputString = {'(', 'a', ')'};
        auto root = parser.buildTree(inputString);
        ASSERT_TRUE(root);
        EXPECT_EQ(root->getSymbol(), S);
        {
            const auto& childs = root->getChilds(); // S -> '(' S ')'
            ASSERT_EQ(childs.size(), 3);
            EXPECT_EQ(childs[0]->getSymbol(), openParen);
            EXPECT_EQ(childs[1]->getSymbol(), S);
            EXPECT_EQ(childs[2]->getSymbol(), closeParen);
        }
        {
            const auto& childs = root->getChilds()[1]->getChilds(); // S -> 'a'
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), a);
        }
    }
    {
        const std::array inputString = {'(', '(', 'a', ')', ')'};
        auto root = parser.buildTree(inputString);
        ASSERT_TRUE(root);
        EXPECT_EQ(root->getSymbol(), S);
        {
            const auto& childs = root->getChilds(); // S -> '(' S ')'
            ASSERT_EQ(childs.size(), 3);
            EXPECT_EQ(childs[0]->getSymbol(), openParen);
            EXPECT_EQ(childs[1]->getSymbol(), S);
            EXPECT_EQ(childs[2]->getSymbol(), closeParen);
        }
        {
            const auto& childs = root->getChilds()[1]->getChilds(); // S -> '(' S ')'
            ASSERT_EQ(childs.size(), 3);
            EXPECT_EQ(childs[0]->getSymbol(), openParen);
            EXPECT_EQ(childs[1]->getSymbol(), S);
            EXPECT_EQ(childs[2]->getSymbol(), closeParen);
        }
        {
            const auto& childs = root->getChilds()[1]->getChilds()[1]->getChilds(); // S -> 'a'
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), a);
        }
    }
    {
        const std::array inputString = {'(', '(', '(', 'a', ')', ')', ')'};
        auto root = parser.buildTree(inputString);
        ASSERT_TRUE(root);
        EXPECT_EQ(root->getSymbol(), S);
        {
            const auto& childs = root->getChilds(); // S -> '(' S ')'
            ASSERT_EQ(childs.size(), 3);
            EXPECT_EQ(childs[0]->getSymbol(), openParen);
            EXPECT_EQ(childs[1]->getSymbol(), S);
            EXPECT_EQ(childs[2]->getSymbol(), closeParen);
        }
        {
            const auto& childs = root->getChilds()[1]->getChilds(); // S -> '(' S ')'
            ASSERT_EQ(childs.size(), 3);
            EXPECT_EQ(childs[0]->getSymbol(), openParen);
            EXPECT_EQ(childs[1]->getSymbol(), S);
            EXPECT_EQ(childs[2]->getSymbol(), closeParen);
        }
        {
            const auto& childs = root->getChilds()[1]->getChilds()[1]->getChilds(); // S -> '(' S ')'
            ASSERT_EQ(childs.size(), 3);
            EXPECT_EQ(childs[0]->getSymbol(), openParen);
            EXPECT_EQ(childs[1]->getSymbol(), S);
            EXPECT_EQ(childs[2]->getSymbol(), closeParen);
        }
        {
            const auto& childs = root->getChilds()[1]->getChilds()[1]->getChilds()[1]->getChilds(); // S -> 'a'
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), a);
        }
    }
}

TEST(TestLL, Test10) {
    /*
        S -> (E) | E | e;
        E -> i + E | i;
    */

    const Terminal i{"i"}, plus{"+"}, openParen{"("}, closeParen{")"};
    const NonTerminal E{"E"};

    ProductionRules rules;
    rules.newProduction(S) >> openParen >> E >> closeParen >> Or >> E >> Or >> None;
    rules.newProduction(E) >> i >> plus >> E >> Or >> i;

    ParserType parser{std::move(rules), TokenToTerminal};
    {
        const std::array inputString = {'i'};
        auto root = parser.buildTree(inputString);
        ASSERT_TRUE(root);
        EXPECT_EQ(root->getSymbol(), S);
        {
            const auto& childs = root->getChilds();  // S -> E;
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), E);
        }
        {
            const auto& childs = root->getChilds()[0]->getChilds();  // E -> i;
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), i);
        }
    }
    {
        const std::array inputString = {'i', '+', 'i'};
        auto root = parser.buildTree(inputString);
        ASSERT_TRUE(root);
        EXPECT_EQ(root->getSymbol(), S);
        {
            const auto& childs = root->getChilds(); // S -> E;
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), E);
        }
        {
            const auto& childs = root->getChilds()[0]->getChilds(); // E -> i + E;
            ASSERT_EQ(childs.size(), 3);
            EXPECT_EQ(childs[0]->getSymbol(), i);
            EXPECT_EQ(childs[1]->getSymbol(), plus);
            EXPECT_EQ(childs[2]->getSymbol(), E);
        }
        {
            const auto& childs = root->getChilds()[0]->getChilds()[2]->getChilds(); // E -> i;
            ASSERT_EQ(childs.size(), 1);
            EXPECT_EQ(childs[0]->getSymbol(), i);
        }
    }
    {
        const std::array inputString = {'i', '+', '(', 'i', '+', 'i', ')'};
        auto root = parser.buildTree(inputString);
        // ASSERT_TRUE(root);
        // EXPECT_EQ(root->getSymbol(), S);
        // {
        //     const auto& childs = root->getChilds(); // S -> E;
        //     ASSERT_EQ(childs.size(), 1);
        //     EXPECT_EQ(childs[0]->getSymbol(), E);
        // }

        // {
        //     const auto& childs = root->getChilds()[0]->getChilds(); // E -> i + E;
        //     ASSERT_EQ(childs.size(), 3);
        //     EXPECT_EQ(childs[0]->getSymbol(), i);
        //     EXPECT_EQ(childs[1]->getSymbol(), plus);
        //     EXPECT_EQ(childs[2]->getSymbol(), E);
        // }

        // {
        //     const auto& childs = root->getChilds()[0]->getChilds()[2]->getChilds(); // E -> (E);
        //     ASSERT_EQ(childs.size(), 3);
        //     EXPECT_EQ(childs[0]->getSymbol(), openParen);
        //     EXPECT_EQ(childs[1]->getSymbol(), E);
        //     EXPECT_EQ(childs[2]->getSymbol(), closeParen);
        // }
        // {
        //     const auto& childs = root->getChilds()[0]->getChilds()[2]->getChilds()[1]->getChilds(); // E -> i + E;
        //     ASSERT_EQ(childs.size(), 3);
        //     EXPECT_EQ(childs[0]->getSymbol(), i);
        //     EXPECT_EQ(childs[1]->getSymbol(), plus);
        //     EXPECT_EQ(childs[2]->getSymbol(), E);
        // }
        // {
        //     const auto& childs = root->getChilds()[0]->getChilds()[2]->getChilds()[1]->getChilds()[2]->getChilds(); // E -> i;
        //     ASSERT_EQ(childs.size(), 1);
        //     EXPECT_EQ(childs[0]->getSymbol(), i);
        // }
    }
}
