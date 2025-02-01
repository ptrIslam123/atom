#include <gtest/gtest.h>

#include "include/cfg/grammar.h"

using namespace atom::cfg::grammar;

TEST(TestGrammar, TestProduction) {
    const Terminal a{"a"}, b{"b"}, c{"c"}, d{"d"};
    const std::vector<Production::DerivationType> derivations{a, b, d};
    const Production prod{ derivations };
    {
        auto it = prod.cbegin();
        ASSERT_NE(it, prod.cend());
        EXPECT_EQ(*it, static_cast<Symbol>(a));
        ++it;

        ASSERT_NE(it, prod.cend());
        EXPECT_EQ(*it, static_cast<Symbol>(b));
        ++it;

        ASSERT_NE(it, prod.cend());
        EXPECT_EQ(*it, static_cast<Symbol>(d));
        ++it;
        EXPECT_EQ(it, prod.cend());
    }

    {
        auto it = prod.find(static_cast<Symbol>(b));
        EXPECT_NE(it, prod.cend());
        ++it;
        EXPECT_EQ(*it, static_cast<Symbol>(d));

        it = prod.find(c);
        EXPECT_EQ(it, prod.cend());
    }
}

TEST(TestGrammar, TestProductions) {
    const Terminal a{"a"}, b{"b"}, c{"c"}, d{"d"}, g{"g"}, h{"h"};
    const std::vector<Production::DerivationType> derivations1{ a, b, c };
    const std::vector<Production::DerivationType> derivations2{ d, g, h };
    const Production prod1{ derivations1 }, prod2{ derivations2 };
    const Productions prods{ std::span<const Production>{ {prod1, prod2} } };
    {
        auto it = prods.cbegin();
        ASSERT_NE(it, prods.cend());
        EXPECT_EQ(*it, prod1);
        ++it;

        ASSERT_NE(it, prods.cend());
        EXPECT_EQ(*it, prod2);
        ++it;

        EXPECT_EQ(it, prods.cend());
    }
}

TEST(TestGrammar, TestsRules1) {
    /*
        S -> A B;
        A -> a | e;
        B -> b | e;
    */
    // Terminals = { a, b, e }
    const Terminal a{"a"}, b{"b"};

    // NonTerminals = { S, A, B }
    const NonTerminal A{"A"}, B{"B"};

    ProductionRules rules;
    rules.newProduction(S) >> A >> B;
    rules.newProduction(A) >> a >> Or >> None;
    rules.newProduction(B) >> b >> Or >> None;
    {
        auto it = rules.cbegin();
        EXPECT_TRUE((it + 1 > it) && ((it + rules.size()) == rules.cend()));
    }
    {
        auto it = rules.cbegin();
        ASSERT_NE(it, rules.cend());

        {
            const auto& [left, prods] = *it;
            EXPECT_EQ(left, S);
            ASSERT_NE(prods.cbegin(), prods.cend());

            Production prod{}; prod >> A >> B;
            EXPECT_EQ(*prods.cbegin(), prod);
        }
        ++it;
        {
            const auto& [left, prods] = *it;
            EXPECT_EQ(left, A);
            ASSERT_NE(prods.cbegin(), prods.cend());

            Production firstProd{}, secondProd{};
            firstProd >> a; secondProd >> None;

            EXPECT_EQ(*prods.cbegin(), firstProd);
            EXPECT_EQ(*(++prods.cbegin()), secondProd);
        }
    }

    {
        auto it = rules.find(B);
        ASSERT_NE(it, rules.end());

        const auto& [left, _prods] = *it;
        ASSERT_NE(_prods.cbegin(), _prods.cend());

        Production firstProd{}, secondProd;
        firstProd>> b; secondProd >> None;
        EXPECT_EQ(*_prods.cbegin(), firstProd);
        EXPECT_EQ(*(++_prods.cbegin()), secondProd);
    }
}
