#include <gtest/gtest.h>

// #include "include/lexical_analyzer/lexer.h"
// #include "include/lexical_analyzer/default_lexer.h"

// using namespace atom;
// using namespace automaton;
// using namespace lexical_analyzer;

// namespace {

// using LexemsType = decltype(MakeDefaultLexer().lexemize({}));
// using LexemType = LexemsType::value_type;

// bool Eq(const LexemsType& f, const LexemsType& s) {
//     return false;
// }

// bool Eq(const LexemType& f, const LexemType& s) {
//     return f == s;
// }

// std::vector<char> SplitString(const std::string_view input) {
//     std::vector<char> result;
//     result.reserve(input.size() + 1);
//     for (const char s : input) {
//         result.push_back(s);
//     }
//     result.push_back('\0'); // EOF
//     return result;
// }

// } //! namespace

// TEST(TestLexer, TestBaseLexer) {
//     using DefaultLexer = decltype(MakeDefaultLexer());
//     using DefaultLexem = DefaultLexer::Lexem;
//     using DefaultLexemType = DefaultLexer::LexemType;

//     auto lexer = MakeDefaultLexer();
//     {
//         const auto text = SplitString(
//             "edge-triggered(1)The epoll API performs a similar task to poll(2): monitoring"
//             "multiple file descriptors to see if I/O is possible on any of"
//             "them.  The epoll API can be used either as an edge-triggered or a"
//             "level-triggered interface and scales well to large numbers of"
//             "watched file descriptors."
//         );
//         auto lexems = lexer.lexemize(text);
//         EXPECT_FALSE(lexems.empty());
//     }
//     {
//         const auto text = SplitString(
//             "mid-2010s.           In 2019 mid-2010s, Social Finance Inc. (SoFi) is an online financial company founded in 2011 by Stanford Business School students."
//             "The group used an alumni-funded lending model to connect recent grads with alumni in their community."
//             "Initially, SoFi`s main product was student loan financing, but it began offering personal loans and mortgages in the mid-2010s."
//                                           "           In 2019, it introduced SoFi Money, which became SoFi Checking and Savings."
//         );
//         auto lexems = lexer.lexemize(text);
//         EXPECT_FALSE(lexems.empty());
//     }
//     {
//         const auto text = SplitString(
//         "5,25 0.01% TD Bank offers multiple checking accounts, but only one of them earns interest, albeit a minimal amount."
//         "The Beyond Checking account pays 0.01% APY and charges a $25 monthly maintenance fee."
//         "It offers a few ways to waive the fee, such as having a $2,500 daily average balance or $5.000 in monthly direct deposits."
//                                           "It also charges a $35 overdraft fee, though there is a grace period."
//                                           "TD Bank has a much smaller ATM network than SoFi, but it does have physical branches along the East Coast."
//         );
//         auto lexems = lexer.lexemize(text);
//         EXPECT_FALSE(lexems.empty());
//     }
//     {
//         const auto text = SplitString("def var123 = 23; def var3 = @attr1 + @attr2 | pipe .");
//         auto lexems = lexer.lexemize(text);
//         ASSERT_EQ(lexems.size(), 16);
//         EXPECT_TRUE(Eq(lexems[0], DefaultLexem{DefaultLexemType::Word, "def"}));
//         EXPECT_TRUE(Eq(lexems[1], DefaultLexem{DefaultLexemType::Word, "var123"}));
//         EXPECT_TRUE(Eq(lexems[2], DefaultLexem{DefaultLexemType::Eq, "="}));
//         EXPECT_TRUE(Eq(lexems[3], DefaultLexem{DefaultLexemType::Integer, "23"}));
//         EXPECT_TRUE(Eq(lexems[4], DefaultLexem{DefaultLexemType::SemiColon, ";"}));
//         EXPECT_TRUE(Eq(lexems[5], DefaultLexem{DefaultLexemType::Word, "def"}));
//         EXPECT_TRUE(Eq(lexems[6], DefaultLexem{DefaultLexemType::Word, "var3"}));
//         EXPECT_TRUE(Eq(lexems[7], DefaultLexem{DefaultLexemType::Eq, "="}));
//         EXPECT_TRUE(Eq(lexems[8], DefaultLexem{DefaultLexemType::At, "@"}));
//         EXPECT_TRUE(Eq(lexems[9], DefaultLexem{DefaultLexemType::Word, "attr1"}));
//         EXPECT_TRUE(Eq(lexems[10], DefaultLexem{DefaultLexemType::Plus, "+"}));
//         EXPECT_TRUE(Eq(lexems[11], DefaultLexem{DefaultLexemType::At, "@"}));
//         EXPECT_TRUE(Eq(lexems[12], DefaultLexem{DefaultLexemType::Word, "attr2"}));
//         EXPECT_TRUE(Eq(lexems[13], DefaultLexem{DefaultLexemType::Pipe, "|"}));
//         EXPECT_TRUE(Eq(lexems[14], DefaultLexem{DefaultLexemType::Word, "pipe"}));
//         EXPECT_TRUE(Eq(lexems[15], DefaultLexem{DefaultLexemType::Dot, "."}));
//     }
//     {
//         std::string jsonSample = R"(
//         {
//           "users": [
//             {
//               "id": 1,
//               "name": "John Doe",
//               "email": "john.doe@example.com",
//               "age": 28,
//               "address": {
//                 "city": "New York",
//                 "street": "5th Avenue"
//               }
//             },
//             {
//               "id": 2,
//               "name": "Jane Smith",
//               "email": "jane.smith@example.com",
//               "age": 34,
//               "address": {
//                 "city": "Los Angeles",
//                 "street": "Sunset Boulevard"
//               }
//             }
//           ]
//         }
//         )";
//         jsonSample.push_back('\0');
//         auto lexems = lexer.lexemize(jsonSample);
//         EXPECT_FALSE(lexems.empty());
//     }
//     {
//         std::string xmlSample = R"(
//         <root>
//           <users>
//             <user>
//               <id>1</id>
//               <name>John Doe</name>
//               <email>john.doe@example.com</email>
//               <age>28</age>
//               <address>
//                 <city>New York</city>
//                 <street>5th Avenue</street>
//               </address>
//             </user>
//             <user>
//               <id>2</id>
//               <name>Jane Smith</name>
//               <email>jane.smith@example.com</email>
//               <age>34</age>
//               <address>
//                 <city>Los Angeles</city>
//                 <street>Sunset Boulevard</street>
//               </address>
//             </user>
//           </users>
//         </root>
//         )";
//         xmlSample.push_back('\0');
//         auto lexems = lexer.lexemize(xmlSample);
//         EXPECT_FALSE(lexems.empty());
//     }
// }
