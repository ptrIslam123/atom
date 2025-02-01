#include <gtest/gtest.h>

#include "include/cfg/ll.h"

#include <array>

using namespace atom::cfg::grammar;
using namespace atom::cfg::ll;
using namespace atom::ast;

using ParserType = Parser<char>;

namespace {

inline Terminal TokenToTerminal(const char& s) {
    std::string data;
    data.push_back(s);
    return Terminal{std::move(data)};
}

} //! namespace

// TEST(TestLL, Test1) {
//     /*
//         S -> A t;
//         A -> a;
//     */

//     const Terminal a{"a"}, t{"t"};
//     const NonTerminal A{"A"};

//     const std::array inputString = {'a', 't'};

//     ProductionRules rules;
//     rules.newProduction(S) >> A >> t;
//     rules.newProduction(A) >> a;

//     ParserType parser{std::move(rules), TokenToTerminal};
//     auto root = parser.buildTree(inputString);

//     ASSERT_TRUE(root);
//     EXPECT_EQ(root->getSymbol(), S);
//     const auto& childs = root->getChilds();
//     ASSERT_EQ(childs.size(), 2);
//     EXPECT_EQ(childs[0]->getSymbol(), A);
//     EXPECT_EQ(childs[1]->getSymbol(), t);
// }

// TEST(TestLL, Test2) {
//     /*
//         S -> A B C;
//         A -> a | e;
//         B -> b | e;
//         C -> c;
//     */

//     const Terminal a{"a"}, b{"b"}, c{"c"};
//     const NonTerminal A{"A"}, B{"B"}, C{"C"};

//     ProductionRules rules;
//     rules.newProduction(S) >> A >> B >> C;
//     rules.newProduction(A) >> a >> Or >> None;
//     rules.newProduction(B) >> b >> Or >> None;
//     rules.newProduction(C) >> c;

//     ParserType parser{std::move(rules), TokenToTerminal};
//     {
//         const std::array inputString = {'a', 'c'};
//         auto root = parser.buildTree(inputString);
//         ASSERT_TRUE(root);

//         EXPECT_EQ(root->getSymbol(), S);
//         {
//             const auto& childs = root->getChilds(); // S -> A B C;
//             ASSERT_EQ(childs.size(), 3);
//             EXPECT_EQ(childs[0]->getSymbol(), A);
//             EXPECT_EQ(childs[1]->getSymbol(), B);
//             EXPECT_EQ(childs[2]->getSymbol(), C);
//         }
//         {
//             const auto& childs = root->getChilds()[0]->getChilds(); // A -> a;
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), a);
//         }
//         {
//             const auto& childs = root->getChilds()[1]->getChilds(); // B -> e;
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), None);
//         }
//         {
//             const auto& childs = root->getChilds()[2]->getChilds(); // C -> c;
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), c);
//         }
//     }
//     {
//         const std::array inputString = {'a', 'b', 'c'};
//         auto root = parser.buildTree(inputString);
//         ASSERT_TRUE(root);

//         EXPECT_EQ(root->getSymbol(), S);
//         {
//             const auto& childs = root->getChilds(); // S -> A B C;
//             ASSERT_EQ(childs.size(), 3);
//             EXPECT_EQ(childs[0]->getSymbol(), A);
//             EXPECT_EQ(childs[1]->getSymbol(), B);
//             EXPECT_EQ(childs[2]->getSymbol(), C);
//         }
//         {
//             const auto& childs = root->getChilds()[0]->getChilds(); // A -> a;
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), a);
//         }
//         {
//             const auto& childs = root->getChilds()[1]->getChilds(); // B -> b;
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), b);
//         }
//         {
//             const auto& childs = root->getChilds()[2]->getChilds(); // C -> c;
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), c);
//         }
//     }
// }

// TEST(TestLL, Test3) {
//     /*
//         S -> A B C;
//         A -> a;
//         B -> b | e;
//         C -> c;
//     */

//     const Terminal a{"a"}, b{"b"}, c{"c"};
//     const NonTerminal A{"A"}, B{"B"}, C{"C"};

//     ProductionRules rules;
//     rules.newProduction(S) >> A >> B >> C;
//     rules.newProduction(A) >> a;
//     rules.newProduction(B) >> b >> Or >> None;
//     rules.newProduction(C) >> c;

//     ParserType parser{std::move(rules), TokenToTerminal};
//     {
//         const std::array inputString = {'a', 'b', 'c'};
//         auto root = parser.buildTree(inputString);
//         ASSERT_TRUE(root);

//         EXPECT_EQ(root->getSymbol(), S);
//         {
//             const auto& childs = root->getChilds(); // S -> A B C;
//             ASSERT_EQ(childs.size(), 3);
//             EXPECT_EQ(childs[0]->getSymbol(), A);
//             EXPECT_EQ(childs[1]->getSymbol(), B);
//             EXPECT_EQ(childs[2]->getSymbol(), C);
//         }
//         {
//             const auto& childs = root->getChilds()[0]->getChilds(); // A -> a;
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), a);
//         }
//         {
//             const auto& childs = root->getChilds()[1]->getChilds(); // B -> b;
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), b);
//         }
//         {
//             const auto& childs = root->getChilds()[2]->getChilds(); // C -> c;
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), c);
//         }
//     }
//     {
//         const std::array inputString = {'a', 'c'};
//         auto root = parser.buildTree(inputString);
//         ASSERT_TRUE(root);

//         EXPECT_EQ(root->getSymbol(), S);
//         {
//             const auto& childs = root->getChilds(); // S -> A B C;
//             ASSERT_EQ(childs.size(), 3);
//             EXPECT_EQ(childs[0]->getSymbol(), A);
//             EXPECT_EQ(childs[1]->getSymbol(), B);
//             EXPECT_EQ(childs[2]->getSymbol(), C);
//         }
//         {
//             const auto& childs = root->getChilds()[0]->getChilds(); // A -> a;
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), a);
//         }
//         {
//             const auto& childs = root->getChilds()[1]->getChilds(); // B -> e;
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), None);
//         }
//         {
//             const auto& childs = root->getChilds()[2]->getChilds(); // C -> c;
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), c);
//         }
//     }
// }

// TEST(TestLL, Test4) {
//     /*
//         S -> A B C;
//         A -> a | e;
//         B -> b | e;
//         C -> c | e;
//     */

//     const Terminal a{"a"}, b{"b"}, c{"c"};
//     const NonTerminal A{"A"}, B{"B"}, C{"C"};

//     ProductionRules rules;
//     rules.newProduction(S) >> A >> B >> C;
//     rules.newProduction(A) >> a >> Or >> None;
//     rules.newProduction(B) >> b >> Or >> None;
//     rules.newProduction(C) >> c >> Or >> None;

//     ParserType parser{std::move(rules), TokenToTerminal};
//     {
//         const std::array inputString = {'a', 'b', 'c'};
//         auto root = parser.buildTree(inputString);
//         ASSERT_TRUE(root);
//         EXPECT_EQ(root->getSymbol(), S);
//         {
//             const auto& childs = root->getChilds(); // S -> A B C;
//             ASSERT_EQ(childs.size(), 3);
//             EXPECT_EQ(childs[0]->getSymbol(), A);
//             EXPECT_EQ(childs[1]->getSymbol(), B);
//             EXPECT_EQ(childs[2]->getSymbol(), C);
//         }
//         {
//             const auto& childs = root->getChilds()[0]->getChilds(); // A -> a;
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), a);
//         }
//         {
//             const auto& childs = root->getChilds()[1]->getChilds(); // B -> b;
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), b);
//         }
//         {
//             const auto& childs = root->getChilds()[2]->getChilds(); // C -> c;
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), c);
//         }
//     }
//     {
//         const std::array inputString = {'b', 'c'};
//         auto root = parser.buildTree(inputString);
//         ASSERT_TRUE(root);
//         EXPECT_EQ(root->getSymbol(), S);
//         {
//             const auto& childs = root->getChilds(); // S -> A B C;
//             ASSERT_EQ(childs.size(), 3);
//             EXPECT_EQ(childs[0]->getSymbol(), A);
//             EXPECT_EQ(childs[1]->getSymbol(), B);
//             EXPECT_EQ(childs[2]->getSymbol(), C);
//         }
//         {
//             const auto& childs = root->getChilds()[0]->getChilds(); // A -> e;
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), None);
//         }
//         {
//             const auto& childs = root->getChilds()[1]->getChilds(); // B -> b;
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), b);
//         }
//         {
//             const auto& childs = root->getChilds()[2]->getChilds(); // C -> c;
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), c);
//         }
//     }
//     {
//         const std::array inputString = {'c'};
//         auto root = parser.buildTree(inputString);
//         ASSERT_TRUE(root);
//         EXPECT_EQ(root->getSymbol(), S);
//         {
//             const auto& childs = root->getChilds(); // S -> A B C;
//             ASSERT_EQ(childs.size(), 3);
//             EXPECT_EQ(childs[0]->getSymbol(), A);
//             EXPECT_EQ(childs[1]->getSymbol(), B);
//             EXPECT_EQ(childs[2]->getSymbol(), C);
//         }
//         {
//             const auto& childs = root->getChilds()[0]->getChilds(); // A -> e;
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), None);
//         }
//         {
//             const auto& childs = root->getChilds()[1]->getChilds(); // B -> e;
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), None);
//         }
//         {
//             const auto& childs = root->getChilds()[2]->getChilds(); // C -> c;
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), c);
//         }
//     }
//     {
//         const std::array<char, 0> inputString;
//         auto root = parser.buildTree(inputString);
//         ASSERT_TRUE(root);
//         EXPECT_EQ(root->getSymbol(), S);
//         {
//             const auto& childs = root->getChilds(); // S -> A B C;
//             ASSERT_EQ(childs.size(), 3);
//             EXPECT_EQ(childs[0]->getSymbol(), A);
//             EXPECT_EQ(childs[1]->getSymbol(), B);
//             EXPECT_EQ(childs[2]->getSymbol(), C);
//         }
//         {
//             const auto& childs = root->getChilds()[0]->getChilds(); // A -> e;
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), None);
//         }
//         {
//             const auto& childs = root->getChilds()[1]->getChilds(); // B -> e;
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), None);
//         }
//         {
//             const auto& childs = root->getChilds()[2]->getChilds(); // C -> e;
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), None);
//         }
//     }
// }

// TEST(TestLL, Test5) {
//     /*
//         S -> A b;
//         A -> a A | e;
//     */

//     // a^nb (ab, aab, aaab, aaa...b)

//     const Terminal a{"a"}, b{"b"};
//     const NonTerminal A{"A"};

//     ProductionRules rules;
//     rules.newProduction(S) >> A >> b;
//     rules.newProduction(A) >> a >> A >> Or >> None;

//     ParserType parser{std::move(rules), TokenToTerminal};
//     {
//         const std::array inputString = {'a', 'b'};
//         auto root = parser.buildTree(inputString);
//         ASSERT_TRUE(root);
//         EXPECT_EQ(root->getSymbol(), S);
//         {
//             const auto& childs = root->getChilds(); // S -> A b;
//             ASSERT_EQ(childs.size(), 2);
//             EXPECT_EQ(childs[0]->getSymbol(), A);
//             EXPECT_EQ(childs[1]->getSymbol(), b);
//         }
//         {
//             const auto& childs = root->getChilds()[0]->getChilds(); // A -> a A;
//             ASSERT_EQ(childs.size(), 2);
//             EXPECT_EQ(childs[0]->getSymbol(), a);
//             EXPECT_EQ(childs[1]->getSymbol(), A);
//         }
//         {
//             const auto& childs = root->getChilds()[0]->getChilds()[1]->getChilds(); // A -> e
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), None);
//         }
//     }
//     {
//         const std::array inputString = {'a', 'a', 'b'};
//         auto root = parser.buildTree(inputString);
//         ASSERT_TRUE(root);
//         EXPECT_EQ(root->getSymbol(), S);
//         {
//             const auto& childs = root->getChilds(); // S -> A b;
//             ASSERT_EQ(childs.size(), 2);
//             EXPECT_EQ(childs[0]->getSymbol(), A);
//             EXPECT_EQ(childs[1]->getSymbol(), b);
//         }
//         {
//             const auto& childs = root->getChilds()[0]->getChilds(); // A -> a A;
//             ASSERT_EQ(childs.size(), 2);
//             EXPECT_EQ(childs[0]->getSymbol(), a);
//             EXPECT_EQ(childs[1]->getSymbol(), A);
//         }
//         {
//             const auto& childs = root->getChilds()[0]->getChilds()[1]->getChilds(); // A -> a A;
//             ASSERT_EQ(childs.size(), 2);
//             EXPECT_EQ(childs[0]->getSymbol(), a);
//             EXPECT_EQ(childs[1]->getSymbol(), A);
//         }
//         {
//             const auto& childs = root->getChilds()[0]->getChilds()[1]->getChilds()[1]->getChilds(); // A -> e;
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), None);
//         }
//     }
// }


// TEST(TestLL, Test6) {
//     /*
//         S -> a S b | ε
//     */

//     // a^n b^n (ab, aabb, aaabbb)

//     const Terminal a{"a"}, b{"b"};

//     ProductionRules rules;
//     rules.newProduction(S) >> a >> S >>  b >> Or >> None;

//     ParserType parser{std::move(rules), TokenToTerminal};
//     {
//         const std::array inputString = {'a', 'b'};
//         auto root = parser.buildTree(inputString);
//         ASSERT_TRUE(root);
//         EXPECT_EQ(root->getSymbol(), S);
//         {
//             const auto& childs = root->getChilds(); // S -> a S b;
//             ASSERT_EQ(childs.size(), 3);
//             EXPECT_EQ(childs[0]->getSymbol(), a);
//             EXPECT_EQ(childs[1]->getSymbol(), S);
//             EXPECT_EQ(childs[2]->getSymbol(), b);
//         }
//         {
//             const auto& childs = root->getChilds()[1]->getChilds(); // S -> e;
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), None);
//         }
//     }
//     {
//         const std::array inputString = {'a', 'a', 'b', 'b'};
//         auto root = parser.buildTree(inputString);
//         ASSERT_TRUE(root);
//         {
//             const auto& childs = root->getChilds(); // S -> a S b;
//             ASSERT_EQ(childs.size(), 3);
//             EXPECT_EQ(childs[0]->getSymbol(), a);
//             EXPECT_EQ(childs[1]->getSymbol(), S);
//             EXPECT_EQ(childs[2]->getSymbol(), b);
//         }
//         {
//             const auto& childs = root->getChilds()[1]->getChilds(); // S -> a S b;
//             ASSERT_EQ(childs.size(), 3);
//             EXPECT_EQ(childs[0]->getSymbol(), a);
//             EXPECT_EQ(childs[1]->getSymbol(), S);
//             EXPECT_EQ(childs[2]->getSymbol(), b);
//         }
//         {
//             const auto& childs = root->getChilds()[1]->getChilds()[1]->getChilds(); // S -> e;
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), None);
//         }
//     }
// }

// TEST(TestLL, Test7) {
//     /*
//         S -> a S b | ε
//     */

//     // a^n b^n (ab, aabb, aaabbb)

//     const Terminal a{"a"}, b{"b"};

//     ProductionRules rules;
//     rules.newProduction(S) >> a >> S >> b >> Or >> None;

//     ParserType parser{std::move(rules), TokenToTerminal};
//     {
//         const std::array inputString = {'a', 'b'};
//         auto root = parser.buildTree(inputString);
//         ASSERT_TRUE(root);
//         EXPECT_EQ(root->getSymbol(), S);
//         {
//             const auto& childs = root->getChilds(); // S -> a S b;
//             ASSERT_EQ(childs.size(), 3);
//             EXPECT_EQ(childs[0]->getSymbol(), a);
//             EXPECT_EQ(childs[1]->getSymbol(), S);
//             EXPECT_EQ(childs[2]->getSymbol(), b);
//         }
//         {
//             const auto& childs = root->getChilds()[1]->getChilds(); // S -> ε
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), None);
//         }
//     }
//     {
//         const std::array inputString = {'a', 'a', 'b', 'b'};
//         auto root = parser.buildTree(inputString);
//         ASSERT_TRUE(root);
//         EXPECT_EQ(root->getSymbol(), S);
//         {
//             const auto& childs = root->getChilds(); // S -> a S b;
//             ASSERT_EQ(childs.size(), 3);
//             EXPECT_EQ(childs[0]->getSymbol(), a);
//             EXPECT_EQ(childs[1]->getSymbol(), S);
//             EXPECT_EQ(childs[2]->getSymbol(), b);
//         }
//         {
//             const auto& childs = root->getChilds()[1]->getChilds(); // S -> a S b;
//             ASSERT_EQ(childs.size(), 3);
//             EXPECT_EQ(childs[0]->getSymbol(), a);
//             EXPECT_EQ(childs[1]->getSymbol(), S);
//             EXPECT_EQ(childs[2]->getSymbol(), b);
//         }
//         {
//             const auto& childs = root->getChilds()[1]->getChilds()[1]->getChilds(); // S -> ε
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), None);
//         }
//     }
//     {
//         const std::array inputString = {'a', 'a', 'a', 'b', 'b', 'b'};
//         auto root = parser.buildTree(inputString);
//         ASSERT_TRUE(root);
//         EXPECT_EQ(root->getSymbol(), S);
//         {
//             const auto& childs = root->getChilds(); // S -> a S b;
//             ASSERT_EQ(childs.size(), 3);
//             EXPECT_EQ(childs[0]->getSymbol(), a);
//             EXPECT_EQ(childs[1]->getSymbol(), S);
//             EXPECT_EQ(childs[2]->getSymbol(), b);
//         }
//         {
//             const auto& childs = root->getChilds()[1]->getChilds(); // S -> a S b;
//             ASSERT_EQ(childs.size(), 3);
//             EXPECT_EQ(childs[0]->getSymbol(), a);
//             EXPECT_EQ(childs[1]->getSymbol(), S);
//             EXPECT_EQ(childs[2]->getSymbol(), b);
//         }
//         {
//             const auto& childs = root->getChilds()[1]->getChilds()[1]->getChilds(); // S -> a S b;
//             ASSERT_EQ(childs.size(), 3);
//             EXPECT_EQ(childs[0]->getSymbol(), a);
//             EXPECT_EQ(childs[1]->getSymbol(), S);
//             EXPECT_EQ(childs[2]->getSymbol(), b);
//         }
//         {
//             const auto& childs = root->getChilds()[1]->getChilds()[1]->getChilds()[1]->getChilds(); // S -> ε
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), None);
//         }
//     }
// }

// TEST(TestLL, Test8) {
//     /*
//         S -> i T
//         T -> + S T | * S T | ε
//     */

//     const Terminal i{"i"}, plus{"+"}, mult{"*"};
//     const NonTerminal T{"T"};

//     ProductionRules rules;
//     rules.newProduction(S) >> i >> T;
//     rules.newProduction(T) >> plus >> S >> T >> Or >> mult >> S >> T >> Or >> None;

//     ParserType parser{std::move(rules), TokenToTerminal};
//     {
//         const std::array inputString = {'i'};
//         auto root = parser.buildTree(inputString);
//         ASSERT_TRUE(root);
//         EXPECT_EQ(root->getSymbol(), S);
//         {
//             const auto& childs = root->getChilds(); // S -> i T
//             ASSERT_EQ(childs.size(), 2);
//             EXPECT_EQ(childs[0]->getSymbol(), i);
//             EXPECT_EQ(childs[1]->getSymbol(), T);
//         }
//         {
//             const auto& childs = root->getChilds()[1]->getChilds(); // T -> ε
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), None);
//         }
//     }
//     {
//         const std::array inputString = {'i', '+', 'i'};
//         auto root = parser.buildTree(inputString);
//         ASSERT_TRUE(root);
//         EXPECT_EQ(root->getSymbol(), S);
//         {
//             const auto& childs = root->getChilds(); // S -> i T
//             ASSERT_EQ(childs.size(), 2);
//             EXPECT_EQ(childs[0]->getSymbol(), i);
//             EXPECT_EQ(childs[1]->getSymbol(), T);
//         }
//         {
//             const auto& childs = root->getChilds()[1]->getChilds(); // T -> + S T
//             ASSERT_EQ(childs.size(), 3);
//             EXPECT_EQ(childs[0]->getSymbol(), plus);
//             EXPECT_EQ(childs[1]->getSymbol(), S);
//             EXPECT_EQ(childs[2]->getSymbol(), T);
//         }
//         {
//             const auto& childs = root->getChilds()[1]->getChilds()[1]->getChilds(); // S -> i T
//             ASSERT_EQ(childs.size(), 2);
//             EXPECT_EQ(childs[0]->getSymbol(), i);
//             EXPECT_EQ(childs[1]->getSymbol(), T);
//         }
//         {
//             const auto& childs = root->getChilds()[1]->getChilds()[1]->getChilds()[1]->getChilds(); // T -> ε
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), None);
//         }
//         {
//             const auto& childs = root->getChilds()[1]->getChilds()[2]->getChilds(); // T -> ε
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), None);
//         }
//     }
//     {
//         const std::array inputString = {'i', '*', 'i', '+', 'i'};
//         auto root = parser.buildTree(inputString);
//         ASSERT_TRUE(root);
//         EXPECT_EQ(root->getSymbol(), S);
//         {
//             const auto& childs = root->getChilds(); // S -> i T
//             ASSERT_EQ(childs.size(), 2);
//             EXPECT_EQ(childs[0]->getSymbol(), i);
//             EXPECT_EQ(childs[1]->getSymbol(), T);
//         }
//         {
//             const auto& childs = root->getChilds()[1]->getChilds(); // T -> * S T
//             ASSERT_EQ(childs.size(), 3);
//             EXPECT_EQ(childs[0]->getSymbol(), mult);
//             EXPECT_EQ(childs[1]->getSymbol(), S);
//             EXPECT_EQ(childs[2]->getSymbol(), T);
//         }
//         {
//             const auto& childs = root->getChilds()[1]->getChilds()[1]->getChilds(); // S -> i T
//             ASSERT_EQ(childs.size(), 2);
//             EXPECT_EQ(childs[0]->getSymbol(), i);
//             EXPECT_EQ(childs[1]->getSymbol(), T);
//         }
//         {
//             const auto& childs = root->getChilds()[1]->getChilds()[1]->getChilds()[1]->getChilds(); // T -> + S T
//             ASSERT_EQ(childs.size(), 3);
//             EXPECT_EQ(childs[0]->getSymbol(), plus);
//             EXPECT_EQ(childs[1]->getSymbol(), S);
//             EXPECT_EQ(childs[2]->getSymbol(), T);
//         }
//         {
//             const auto& childs = root->getChilds()[1]->getChilds()[1]->getChilds()[1]->getChilds()[1]->getChilds(); // S -> i T
//             ASSERT_EQ(childs.size(), 2);
//             EXPECT_EQ(childs[0]->getSymbol(), i);
//             EXPECT_EQ(childs[1]->getSymbol(), T);
//         }
//         {
//             const auto& childs = root->getChilds()[1]->getChilds()[1]->getChilds()[1]->getChilds()[1]->getChilds()[1]->getChilds(); // T -> ε
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), None);
//         }
//         {
//             const auto& childs = root->getChilds()[1]->getChilds()[1]->getChilds()[1]->getChilds()[2]->getChilds(); // T -> ε
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), None);
//         }
//     }
// }

// TEST(TestLL, Test9) {
//     /*
//         S -> '(' S ')' | 'a'
//     */

//     const Terminal a{"a"}, openParen{"("}, closeParen{")"};

//     ProductionRules rules;
//     rules.newProduction(S) >> openParen >> S >> closeParen >> Or >> a;

//     ParserType parser{std::move(rules), TokenToTerminal};
//     {
//         const std::array inputString = {'a'};
//         auto root = parser.buildTree(inputString);
//         ASSERT_TRUE(root);
//         EXPECT_EQ(root->getSymbol(), S);
//         {
//             const auto& childs = root->getChilds(); // S -> 'a'
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), a);
//         }
//     }
//     {
//         const std::array inputString = {'(', 'a', ')'};
//         auto root = parser.buildTree(inputString);
//         ASSERT_TRUE(root);
//         EXPECT_EQ(root->getSymbol(), S);
//         {
//             const auto& childs = root->getChilds(); // S -> '(' S ')'
//             ASSERT_EQ(childs.size(), 3);
//             EXPECT_EQ(childs[0]->getSymbol(), openParen);
//             EXPECT_EQ(childs[1]->getSymbol(), S);
//             EXPECT_EQ(childs[2]->getSymbol(), closeParen);
//         }
//         {
//             const auto& childs = root->getChilds()[1]->getChilds(); // S -> 'a'
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), a);
//         }
//     }
//     {
//         const std::array inputString = {'(', '(', 'a', ')', ')'};
//         auto root = parser.buildTree(inputString);
//         ASSERT_TRUE(root);
//         EXPECT_EQ(root->getSymbol(), S);
//         {
//             const auto& childs = root->getChilds(); // S -> '(' S ')'
//             ASSERT_EQ(childs.size(), 3);
//             EXPECT_EQ(childs[0]->getSymbol(), openParen);
//             EXPECT_EQ(childs[1]->getSymbol(), S);
//             EXPECT_EQ(childs[2]->getSymbol(), closeParen);
//         }
//         {
//             const auto& childs = root->getChilds()[1]->getChilds(); // S -> '(' S ')'
//             ASSERT_EQ(childs.size(), 3);
//             EXPECT_EQ(childs[0]->getSymbol(), openParen);
//             EXPECT_EQ(childs[1]->getSymbol(), S);
//             EXPECT_EQ(childs[2]->getSymbol(), closeParen);
//         }
//         {
//             const auto& childs = root->getChilds()[1]->getChilds()[1]->getChilds(); // S -> 'a'
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), a);
//         }
//     }
//     {
//         const std::array inputString = {'(', '(', '(', 'a', ')', ')', ')'};
//         auto root = parser.buildTree(inputString);
//         ASSERT_TRUE(root);
//         EXPECT_EQ(root->getSymbol(), S);
//         {
//             const auto& childs = root->getChilds(); // S -> '(' S ')'
//             ASSERT_EQ(childs.size(), 3);
//             EXPECT_EQ(childs[0]->getSymbol(), openParen);
//             EXPECT_EQ(childs[1]->getSymbol(), S);
//             EXPECT_EQ(childs[2]->getSymbol(), closeParen);
//         }
//         {
//             const auto& childs = root->getChilds()[1]->getChilds(); // S -> '(' S ')'
//             ASSERT_EQ(childs.size(), 3);
//             EXPECT_EQ(childs[0]->getSymbol(), openParen);
//             EXPECT_EQ(childs[1]->getSymbol(), S);
//             EXPECT_EQ(childs[2]->getSymbol(), closeParen);
//         }
//         {
//             const auto& childs = root->getChilds()[1]->getChilds()[1]->getChilds(); // S -> '(' S ')'
//             ASSERT_EQ(childs.size(), 3);
//             EXPECT_EQ(childs[0]->getSymbol(), openParen);
//             EXPECT_EQ(childs[1]->getSymbol(), S);
//             EXPECT_EQ(childs[2]->getSymbol(), closeParen);
//         }
//         {
//             const auto& childs = root->getChilds()[1]->getChilds()[1]->getChilds()[1]->getChilds(); // S -> 'a'
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), a);
//         }
//     }
// }

// TEST(TestLL, Test10) {
//     /*
//         S -> (E) | E | e;
//         E -> i + E | i;
//     */

//     const Terminal i{"i"}, plus{"+"}, openParen{"("}, closeParen{")"};
//     const NonTerminal E{"E"};

//     ProductionRules rules;
//     rules.newProduction(S) >> openParen >> E >> closeParen >> Or >> E >> Or >> None;
//     rules.newProduction(E) >> i >> plus >> E >> Or >> i;

//     ParserType parser{std::move(rules), TokenToTerminal};
//     {
//         const std::array inputString = {'i'};
//         auto root = parser.buildTree(inputString);
//         ASSERT_TRUE(root);
//         EXPECT_EQ(root->getSymbol(), S);
//         {
//             const auto& childs = root->getChilds();  // S -> E;
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), E);
//         }
//         {
//             const auto& childs = root->getChilds()[0]->getChilds();  // E -> i;
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), i);
//         }
//     }
//     {
//         const std::array inputString = {'i', '+', 'i'};
//         auto root = parser.buildTree(inputString);
//         ASSERT_TRUE(root);
//         EXPECT_EQ(root->getSymbol(), S);
//         {
//             const auto& childs = root->getChilds(); // S -> E;
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), E);
//         }
//         {
//             const auto& childs = root->getChilds()[0]->getChilds(); // E -> i + E;
//             ASSERT_EQ(childs.size(), 3);
//             EXPECT_EQ(childs[0]->getSymbol(), i);
//             EXPECT_EQ(childs[1]->getSymbol(), plus);
//             EXPECT_EQ(childs[2]->getSymbol(), E);
//         }
//         {
//             const auto& childs = root->getChilds()[0]->getChilds()[2]->getChilds(); // E -> i;
//             ASSERT_EQ(childs.size(), 1);
//             EXPECT_EQ(childs[0]->getSymbol(), i);
//         }
//     }
//     {
//         const std::array inputString = {'i', '+', '(', 'i', '+', 'i', ')'};
//         auto root = parser.buildTree(inputString);
//         // ASSERT_TRUE(root);
//         // EXPECT_EQ(root->getSymbol(), S);
//         // {
//         //     const auto& childs = root->getChilds(); // S -> E;
//         //     ASSERT_EQ(childs.size(), 1);
//         //     EXPECT_EQ(childs[0]->getSymbol(), E);
//         // }

//         // {
//         //     const auto& childs = root->getChilds()[0]->getChilds(); // E -> i + E;
//         //     ASSERT_EQ(childs.size(), 3);
//         //     EXPECT_EQ(childs[0]->getSymbol(), i);
//         //     EXPECT_EQ(childs[1]->getSymbol(), plus);
//         //     EXPECT_EQ(childs[2]->getSymbol(), E);
//         // }

//         // {
//         //     const auto& childs = root->getChilds()[0]->getChilds()[2]->getChilds(); // E -> (E);
//         //     ASSERT_EQ(childs.size(), 3);
//         //     EXPECT_EQ(childs[0]->getSymbol(), openParen);
//         //     EXPECT_EQ(childs[1]->getSymbol(), E);
//         //     EXPECT_EQ(childs[2]->getSymbol(), closeParen);
//         // }
//         // {
//         //     const auto& childs = root->getChilds()[0]->getChilds()[2]->getChilds()[1]->getChilds(); // E -> i + E;
//         //     ASSERT_EQ(childs.size(), 3);
//         //     EXPECT_EQ(childs[0]->getSymbol(), i);
//         //     EXPECT_EQ(childs[1]->getSymbol(), plus);
//         //     EXPECT_EQ(childs[2]->getSymbol(), E);
//         // }
//         // {
//         //     const auto& childs = root->getChilds()[0]->getChilds()[2]->getChilds()[1]->getChilds()[2]->getChilds(); // E -> i;
//         //     ASSERT_EQ(childs.size(), 1);
//         //     EXPECT_EQ(childs[0]->getSymbol(), i);
//         // }
//     }
// }

// TEST(TestGrammar, TTTT) {
//     /*
//         S -> A B C;
//         A -> a;
//         B -> b | e;
//         C -> c;
//     */
//     const Terminal a{"a"}, b{"b"}, c{"c"};
//     const NonTerminal A{"A"}, B{"B"}, C{"C"};

//     ProductionRules rules;
//     rules.newProduction(S) >> A >> B >> C;
//     rules.newProduction(A) >> a;
//     rules.newProduction(B) >> b >> Or >> None;
//     rules.newProduction(C) >> c;

//     FirstAndFollowReqHandler ffReqHandler{ rules };
//     using FirstSetType = FirstAndFollowReqHandler::FirstSetType;
//     {
//         const auto expect = FirstSetType{ b, c };
//         const auto real = ffReqHandler.getFirst(B);
//         EXPECT_EQ(real, expect);
//     }
// }

// TEST(TestGrammar, TestFirstAndFollow0) {
//     /*
//         S -> A t;
//         A -> a;
//     */

//     // FIRST(A) = { a }
//     // FIRST(t) = { t }

//     // FOLLOW(S) = { $ }
//     // FOLLOW(A) = { t }

//     // Terminal symbols = { a, t }
//     const Terminal a{"a"}, t{"t"};

//     // Non terminal symbols = { S, A }
//     const NonTerminal A{"A"};

//     ProductionRules rules;
//     rules.newProduction(S) >> A >> t;
//     rules.newProduction(A) >> a;

//     FirstAndFollowReqHandler ffReqHandler{ rules };
//     {
//         auto it = rules.find(S);
//         {
//             ASSERT_NE(it, rules.end());
//             const auto& [left, prods] = *it;
//             ASSERT_EQ(left, S);
//             Production _prod{}; _prod >> A >> t; Productions _prods{}; _prods.pushBack(_prod);
//             ASSERT_EQ(prods, _prods);
//         }
//         ++it;
//         {
//             ASSERT_NE(it, rules.end());
//             const auto& [left, prods] = *it;
//             ASSERT_EQ(left, A);
//             Production _prod{}; _prod >> a; Productions _prods{}; _prods.pushBack(_prod);
//             ASSERT_EQ(prods, _prods);
//         }
//     }
//     {
//         const auto& first = ffReqHandler.getFirst(A);
//         ASSERT_FALSE(first.empty());
//         EXPECT_EQ(*first.cbegin(), a);
//     }
//     {
//         const auto& first = ffReqHandler.getFirst(S);
//         ASSERT_FALSE(first.empty());
//         EXPECT_EQ(*first.cbegin(), a);
//     }
//     {
//         const auto& first = ffReqHandler.getFirst(t);
//         ASSERT_FALSE(first.empty());
//         EXPECT_EQ(*first.cbegin(), t);
//     }
//     {
//         const auto& first = ffReqHandler.getFirst(a);
//         ASSERT_FALSE(first.empty());
//         EXPECT_EQ(*first.cbegin(), a);
//     }

//     {
//         const auto& follow = ffReqHandler.getFollow(A);
//         ASSERT_FALSE(follow.empty());
//         EXPECT_EQ(*follow.cbegin(), t);
//     }
//     {
//         const auto& follow = ffReqHandler.getFollow(S);
//         ASSERT_FALSE(follow.empty());
//         EXPECT_EQ(*follow.cbegin(), End);
//     }
//     {
//         const auto& follow = ffReqHandler.getFollow(t);
//         EXPECT_TRUE(follow.empty());
//     }
//     {
//         const auto& follow = ffReqHandler.getFollow(a);
//         EXPECT_TRUE(follow.empty());
//     }
// }

// TEST(TestGrammar, TestFirstAndFollow1) {
//     /*
//         S -> A t;
//         A -> a | e;
//     */

//     // FIRST(S) = FIRST(A) = { a, t }
//     // FIRST(A) = { a, t }
//     // FIRST(t) = { t }

//     // FOLLOW(A) = { t }
//     // FOLLOW(S) = { $ }

//     // Terminal symbols = { a, t }
//     const Terminal a{"a"}, t{"t"};

//     // Non terminal symbols = { S, A }
//     const NonTerminal A{"A"};

//     ProductionRules rules;
//     rules.newProduction(S) >> A >> t;
//     rules.newProduction(A) >> a >> Or >> None;
//     FirstAndFollowReqHandler ffReqHandler{ rules };
//     using FirstSetType = FirstAndFollowReqHandler::FirstSetType;
//     {
//         auto it = rules.find(S);
//         {
//             ASSERT_NE(it, rules.end());
//             const auto& [left, prods] = *it;
//             ASSERT_EQ(left, S);
//             Production _prod{}; _prod >> A >> t; Productions _prods{}; _prods.pushBack(_prod);
//             ASSERT_EQ(prods, _prods);
//         }
//         ++it;
//         {
//             ASSERT_NE(it, rules.end());
//             const auto& [left, prods] = *it;
//             ASSERT_EQ(left, A);
//             Production _firstProd{}; _firstProd >> a; Production _secondProd{}; _secondProd >> None;
//             Productions _prods{}; _prods >> _firstProd >> _secondProd;
//             ASSERT_EQ(prods, _prods);
//         }
//     }
//     {
//         const auto expect = FirstSetType{ a, t };
//         const auto real = ffReqHandler.getFirst(A);
//         EXPECT_EQ(real, expect);
//     }
//     {
//         const auto expect = FirstSetType{ a, t };
//         const auto real = ffReqHandler.getFirst(S);
//         EXPECT_EQ(real, expect);
//     }

//     {
//         const auto& follow = ffReqHandler.getFollow(A);
//         ASSERT_FALSE(follow.empty());
//         EXPECT_EQ(*follow.cbegin(), t);
//     }
//     {
//         const auto& follow = ffReqHandler.getFollow(S);
//         ASSERT_FALSE(follow.empty());
//         EXPECT_EQ(*follow.cbegin(), End);
//     }
//     {
//         const auto& follow = ffReqHandler.getFollow(a);
//         EXPECT_TRUE(follow.empty());
//     }
// }

// TEST(TestGrammar, TestFirstAndFollow2) {
//     /*
//         S -> A B C;
//         A -> a | e;
//         B -> b | e;
//         C -> c | e;
//     */

//     // FIRST(S) = { a, b, c, ε }
//     // FIRST(A) = { a, b, c, ε }
//     // FIRST(B) = { b, c, ε }
//     // FIRST(C) = { c, ε }

//     // FOLLOW(S) = { $ }
//     // FOLLOW(A) = { b, c, $ }
//     // FOLLOW(B) = { c, $ }
//     // FOLLOW(C) = { $ }

//     // Terminal symbols = { a, b, c }
//     const Terminal a{"a"}, b{"b"}, c{"c"};

//     // Non terminal symbols = { S, A, B, C }
//     const NonTerminal A{"A"}, B{"B"}, C{"C"};

//     ProductionRules rules;
//     rules.newProduction(S) >> A >> B >> C;
//     rules.newProduction(A) >> a >> Or >> None;
//     rules.newProduction(B) >> b >> Or >> None;
//     rules.newProduction(C) >> c >> Or >> None;
//     FirstAndFollowReqHandler ffReqHandler{ rules };
//     using FirstSetType = FirstAndFollowReqHandler::FirstSetType;
//     using FollowSetType = FirstAndFollowReqHandler::FollowSetType;
//     {
//         const auto expect = FirstSetType{ c, None };
//         const auto real = ffReqHandler.getFirst(C);
//         EXPECT_EQ(real, expect);
//     }
//     {
//         const auto expect = FirstSetType{ b, c, None };
//         const auto real = ffReqHandler.getFirst(B);
//         EXPECT_EQ(real, expect);
//     }
//     {
//         const auto expect = FirstSetType{ a, b, c, None };
//         const auto real = ffReqHandler.getFirst(A);
//         EXPECT_EQ(real, expect);
//     }
//     {
//         const auto expect = FirstSetType{ a, b, c, None };
//         const auto real = ffReqHandler.getFirst(S);
//         EXPECT_EQ(real, expect);
//     }

//     {
//         const auto expect = FollowSetType{ End };
//         const auto real = ffReqHandler.getFollow(S);
//         EXPECT_EQ(real, expect);
//     }
//     {
//         const auto expect = FollowSetType{ b, c, End };
//         const auto real = ffReqHandler.getFollow(A);
//         EXPECT_EQ(real, expect);
//     }
//     {
//         const auto expect = FollowSetType{ c, End };
//         const auto real = ffReqHandler.getFollow(B);
//         EXPECT_EQ(real, expect);
//     }
//     {
//         const auto expect = FollowSetType{ End };
//         const auto real = ffReqHandler.getFollow(C);
//         EXPECT_EQ(real, expect);
//     }
//     {
//         const auto follow = ffReqHandler.getFollow(a);
//         EXPECT_TRUE(follow.empty());
//     }
// }

// TEST(TestGrammar, TestFirst3) {
//     /*
//         S -> A B C
//         A -> a
//         B -> b
//         C -> c
//     */
//     // FIRST(S) = { a }
//     // FIRST(A) = { a }
//     // FIRST(B) = { b }
//     // FIRST(C) = { c }

//     // FOLLOW(S) = { $ }
//     // FOLLOW(A) = { b }
//     // FOLLOW(B) = { c }
//     // FOLLOW(C) = { $ }

//     // Terminal symbols = { a, b, c }
//     const Terminal a{"a"}, b{"b"}, c{"c"};

//     // Non terminal symbols = { S, A, B, C }
//     const NonTerminal A{"A"}, B{"B"}, C{"C"};

//     ProductionRules rules;
//     rules.newProduction(S) >> A >> B >> C;
//     rules.newProduction(A) >> a;
//     rules.newProduction(B) >> b;
//     rules.newProduction(C) >> c;
//     FirstAndFollowReqHandler ffReqHandler{ rules };
//     using FirstSetType = FirstAndFollowReqHandler::FirstSetType;
//     using FollowSetType = FirstAndFollowReqHandler::FollowSetType;
//     {
//         const auto expect = FirstSetType{ c };
//         const auto real = ffReqHandler.getFirst(C);
//         EXPECT_EQ(real, expect);
//     }
//     {
//         const auto expect = FirstSetType{ b };
//         const auto real = ffReqHandler.getFirst(B);
//         EXPECT_EQ(real, expect);
//     }
//     {
//         const auto expect = FirstSetType{ a };
//         const auto real = ffReqHandler.getFirst(A);
//         EXPECT_EQ(real, expect);
//     }
//     {
//         const auto expect = FirstSetType{ a };
//         const auto real = ffReqHandler.getFirst(S);
//         EXPECT_EQ(real, expect);
//     }

//     {
//         const auto expect = FollowSetType{ End };
//         const auto real = ffReqHandler.getFollow(S);
//         EXPECT_EQ(real, expect);
//     }
//     {
//         const auto expect = FollowSetType{ b };
//         const auto real = ffReqHandler.getFollow(A);
//         EXPECT_EQ(real, expect);
//     }
//     {
//         const auto expect = FollowSetType{ c };
//         const auto real = ffReqHandler.getFollow(B);
//         EXPECT_EQ(real, expect);
//     }
//     {
//         const auto expect = FollowSetType{ End };
//         const auto real = ffReqHandler.getFollow(C);
//         EXPECT_EQ(real, expect);
//     }
// }

// TEST(TestGrammar, TestFirst4) {
//     /*
//         S -> A B | C D
//         A -> a | ε
//         B -> b
//         C -> c
//         D -> d | ε
//     */

//     // FIRST(S) = { a, b, c }
//     // FIRST(A) = { a, b }
//     // FIRST(B) = { b }
//     // FIRST(C) = { c }
//     // FIRST(D) = { d, ε }

//     // FOLLOW(S) = { $ }
//     // FOLLOW(A) = { b }
//     // FOLLOW(B) = { $ }
//     // FOLLOW(C) = { d, $ }
//     // FOLLOW(D) = { $ }

//     // Terminal symbols = { a, b, c }
//     const Terminal a{"a"}, b{"b"}, c{"c"}, d{"d"};

//     // Non terminal symbols = { S, A, B, C }
//     const NonTerminal A{"A"}, B{"B"}, C{"C"}, D{"D"};

//     ProductionRules rules;
//     rules.newProduction(S) >> A >> B >> Or >> C >> D;
//     rules.newProduction(A) >> a >> Or >> None;
//     rules.newProduction(B) >> b;
//     rules.newProduction(C) >> c;
//     rules.newProduction(D) >> d >> Or >> None;
//     FirstAndFollowReqHandler ffReqHandler{ rules };
//     using FirstSetType = FirstAndFollowReqHandler::FirstSetType;
//     using FollowSetType = FirstAndFollowReqHandler::FollowSetType;
//     {
//         const auto expect = FirstSetType{ d, None };
//         const auto real = ffReqHandler.getFirst(D);
//         EXPECT_EQ(real, expect);
//     }
//     {
//         const auto expect = FirstSetType{ c };
//         const auto real = ffReqHandler.getFirst(C);
//         EXPECT_EQ(real, expect);
//     }
//     {
//         const auto expect = FirstSetType{ b };
//         const auto real = ffReqHandler.getFirst(B);
//         EXPECT_EQ(real, expect);
//     }
//     {
//         const auto expect = FirstSetType{ a, b };
//         const auto real = ffReqHandler.getFirst(A);
//         EXPECT_EQ(real, expect);
//     }
//     {
//         const auto expect = FirstSetType{ a, b, c };
//         const auto real = ffReqHandler.getFirst(S);
//         EXPECT_EQ(real, expect);
//     }

//     {
//         const auto expect = FollowSetType{ End };
//         const auto real = ffReqHandler.getFollow(S);
//         EXPECT_EQ(real, expect);
//     }
//     {
//         const auto expect = FollowSetType{ b };
//         const auto real = ffReqHandler.getFollow(A);
//         EXPECT_EQ(real, expect);
//     }
//     {
//         const auto expect = FollowSetType{ End };
//         const auto real = ffReqHandler.getFollow(B);
//         EXPECT_EQ(real, expect);
//     }
//     {
//         const auto expect = FollowSetType{ d, End };
//         const auto real = ffReqHandler.getFollow(C);
//         EXPECT_EQ(real, expect);
//     }
//     {
//         const auto expect = FollowSetType{ End };
//         const auto real = ffReqHandler.getFollow(D);
//         EXPECT_EQ(real, expect);
//     }
// }
