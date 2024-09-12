#include <gtest/gtest.h>

#include "include/concurrency/safe_mutex.h"
#include <iostream>

using namespace atom;
using LockGraphType = concurrency::LockStory::GraphType;
using LockStoryListType = concurrency::LockStory::StoryListType;
using SafeMutex = concurrency::SafeMutex;

TEST(TestSafeMutex, TestMakeGrapth) {
    SafeMutex m1, m2, m3, m4, m5;
    LockGraphType graph;

    LockStoryListType l1 = {&m1, &m2, &m3, &m4};
    LockStoryListType l2 = {&m5, &m4, &m3};
    concurrency::MakeGraph(l1, l2, graph);

    auto it = graph.find({&m1});
    EXPECT_FALSE(it == graph.cend());
    EXPECT_EQ(it->second, &m2);

    it = graph.find({&m2});
    EXPECT_FALSE(it == graph.cend());
    EXPECT_EQ(it->second, &m3);

    it = graph.find({&m3});
    EXPECT_FALSE(it == graph.cend());
    EXPECT_EQ(it->second, &m4);



    it = graph.find({&m5});
    EXPECT_FALSE(it == graph.cend());
    EXPECT_EQ(it->second, &m4);

    it = graph.find({&m4});
    EXPECT_FALSE(it == graph.cend());
    EXPECT_EQ(it->second, &m3);
}

TEST(TestSafeMutex, TestDetectingSimpleDeadlock) {
    SafeMutex m1, m2;
    LockGraphType graph;

    LockStoryListType l1 = {&m1, &m2};
    LockStoryListType l2 = {&m2, &m1};
    concurrency::MakeGraph(l1, l2, graph);
    EXPECT_FALSE(graph.empty());

    auto it = graph.find({&m1});
    EXPECT_FALSE(it == graph.cend());
    EXPECT_EQ(it->second, &m2);

    it = graph.find({&m2});
    EXPECT_FALSE(it == graph.cend());
    EXPECT_EQ(it->second, &m1);

    EXPECT_TRUE(concurrency::CheckIntersections(graph));
}

TEST(TestSafeMutex, TestDetectNoDeadlock) {
    SafeMutex m1, m2, m3, m4, m5;
    LockGraphType graph;

    LockStoryListType l1 = {&m1, &m2, &m3, &m4};
    LockStoryListType l2 = {&m5, &m4, /*&m3*/};
    concurrency::MakeGraph(l1, l2, graph);
    EXPECT_FALSE(graph.empty());
    EXPECT_FALSE(concurrency::CheckIntersections(graph));
}

TEST(TestSafeMutex, TestSelfDeadlockWithOneMutex) {
    SafeMutex m1;
    LockGraphType graph;
    LockStoryListType l1 = {&m1, &m1};
    concurrency::MakeGraph(l1, LockStoryListType{}, graph);
    EXPECT_FALSE(graph.empty());
    EXPECT_EQ(graph.size(), 1);
    EXPECT_TRUE(concurrency::CheckIntersections(graph));
}
