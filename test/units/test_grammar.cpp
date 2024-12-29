#include <gtest/gtest.h>

#include "include/cfg/grammar.h"

using namespace atom::ast::cfg::grammar;

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
        const auto& first = ffReqHandler.getFirst(A);
        ASSERT_FALSE(first.empty());
        EXPECT_EQ(*first.cbegin(), a);
    }
    {
        const auto& first = ffReqHandler.getFirst(S);
        ASSERT_FALSE(first.empty());
        EXPECT_EQ(*first.cbegin(), a);
    }
    {
        const auto& first = ffReqHandler.getFirst(t);
        ASSERT_FALSE(first.empty());
        EXPECT_EQ(*first.cbegin(), t);
    }
    {
        const auto& first = ffReqHandler.getFirst(a);
        ASSERT_FALSE(first.empty());
        EXPECT_EQ(*first.cbegin(), a);
    }

    {
        const auto& follow = ffReqHandler.getFollow(A);
        ASSERT_FALSE(follow.empty());
        EXPECT_EQ(*follow.cbegin(), t);
    }
    {
        const auto& follow = ffReqHandler.getFollow(S);
        ASSERT_FALSE(follow.empty());
        EXPECT_EQ(*follow.cbegin(), End);
    }
    {
        const auto& follow = ffReqHandler.getFollow(t);
        EXPECT_TRUE(follow.empty());
    }
    {
        const auto& follow = ffReqHandler.getFollow(a);
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
    using FirstSetType = FirstAndFollowReqHandler::FirstSetType;
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
        const auto real = ffReqHandler.getFirst(A);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FirstSetType{ a, t };
        const auto real = ffReqHandler.getFirst(S);
        EXPECT_EQ(real, expect);
    }

    {
        const auto& follow = ffReqHandler.getFollow(A);
        ASSERT_FALSE(follow.empty());
        EXPECT_EQ(*follow.cbegin(), t);
    }
    {
        const auto& follow = ffReqHandler.getFollow(S);
        ASSERT_FALSE(follow.empty());
        EXPECT_EQ(*follow.cbegin(), End);
    }
    {
        const auto& follow = ffReqHandler.getFollow(a);
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
    using FirstSetType = FirstAndFollowReqHandler::FirstSetType;
    using FollowSetType = FirstAndFollowReqHandler::FollowSetType;
    {
        const auto expect = FirstSetType{ c, None };
        const auto real = ffReqHandler.getFirst(C);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FirstSetType{ b, c, None };
        const auto real = ffReqHandler.getFirst(B);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FirstSetType{ a, b, c, None };
        const auto real = ffReqHandler.getFirst(A);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FirstSetType{ a, b, c, None };
        const auto real = ffReqHandler.getFirst(S);
        EXPECT_EQ(real, expect);
    }

    {
        const auto expect = FollowSetType{ End };
        const auto real = ffReqHandler.getFollow(S);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FollowSetType{ b, c, End };
        const auto real = ffReqHandler.getFollow(A);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FollowSetType{ c, End };
        const auto real = ffReqHandler.getFollow(B);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FollowSetType{ End };
        const auto real = ffReqHandler.getFollow(C);
        EXPECT_EQ(real, expect);
    }
    {
        const auto follow = ffReqHandler.getFollow(a);
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
    using FirstSetType = FirstAndFollowReqHandler::FirstSetType;
    using FollowSetType = FirstAndFollowReqHandler::FollowSetType;
    {
        const auto expect = FirstSetType{ c };
        const auto real = ffReqHandler.getFirst(C);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FirstSetType{ b };
        const auto real = ffReqHandler.getFirst(B);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FirstSetType{ a };
        const auto real = ffReqHandler.getFirst(A);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FirstSetType{ a };
        const auto real = ffReqHandler.getFirst(S);
        EXPECT_EQ(real, expect);
    }

    {
        const auto expect = FollowSetType{ End };
        const auto real = ffReqHandler.getFollow(S);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FollowSetType{ b };
        const auto real = ffReqHandler.getFollow(A);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FollowSetType{ c };
        const auto real = ffReqHandler.getFollow(B);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FollowSetType{ End };
        const auto real = ffReqHandler.getFollow(C);
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
    using FirstSetType = FirstAndFollowReqHandler::FirstSetType;
    using FollowSetType = FirstAndFollowReqHandler::FollowSetType;
    {
        const auto expect = FirstSetType{ d, None };
        const auto real = ffReqHandler.getFirst(D);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FirstSetType{ c };
        const auto real = ffReqHandler.getFirst(C);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FirstSetType{ b };
        const auto real = ffReqHandler.getFirst(B);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FirstSetType{ a, b };
        const auto real = ffReqHandler.getFirst(A);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FirstSetType{ a, b, c };
        const auto real = ffReqHandler.getFirst(S);
        EXPECT_EQ(real, expect);
    }

    {
        const auto expect = FollowSetType{ End };
        const auto real = ffReqHandler.getFollow(S);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FollowSetType{ b };
        const auto real = ffReqHandler.getFollow(A);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FollowSetType{ End };
        const auto real = ffReqHandler.getFollow(B);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FollowSetType{ d, End };
        const auto real = ffReqHandler.getFollow(C);
        EXPECT_EQ(real, expect);
    }
    {
        const auto expect = FollowSetType{ End };
        const auto real = ffReqHandler.getFollow(D);
        EXPECT_EQ(real, expect);
    }
}
