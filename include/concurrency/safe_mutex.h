#ifndef CONCURRENCY_SAFE_MUTEX_H
#define CONCURRENCY_SAFE_MUTEX_H

#include <mutex>
#include <condition_variable>
#include <list>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <algorithm>
#include <cassert>

#include <iostream>

namespace atom::concurrency {

#ifdef NDEBUG

class SafeMutex;
class LockStory;

class LockStory final {
public:
    using GraphType = std::unordered_map<const SafeMutex*, const SafeMutex*>;
    using StoryListType = std::vector<const SafeMutex*>;

    static constexpr auto DEFAULT_STORY_LITS_SIZE = 0;

    explicit LockStory(std::size_t /*storySizeHint*/ = DEFAULT_STORY_LITS_SIZE) {}
};

class SafeMutex final {
public:
    explicit SafeMutex(): m_mutex() {}
    inline void lock(LockStory& lockStory) { m_mutex.lock(); }
    inline void unlock(LockStory& lockStory) { m_mutex.unlock(); }

private:
    std::mutex m_mutex;
};

#else

class SafeMutex;
class LockStory;

class LockStory final {
public:
    using GraphType = std::unordered_map<const SafeMutex*, const SafeMutex*>;
    using StoryListType = std::vector<const SafeMutex*>;

    static constexpr auto DEFAULT_STORY_LITS_SIZE = 5;

    explicit LockStory(std::size_t storySizeHint = DEFAULT_STORY_LITS_SIZE);

private:
    friend SafeMutex;

    void add(const SafeMutex& safeMutex);
    void remove(const SafeMutex& safeMutex);

    StoryListType m_storage;
};

class SafeMutex final {
public:
    explicit SafeMutex();
    void lock(LockStory& lockStory);
    void unlock(LockStory& lockStory);

private:
    LockStory::GraphType m_graph;
    LockStory* m_currentOwner;
    std::queue<LockStory*, std::list<LockStory*>> m_waitQueue;
    std::condition_variable m_condVar;
    std::mutex m_mutex;
    bool m_flag;
};

#endif //! ifdef NDEBUG

void MakeGraph(const LockStory::StoryListType& first, const LockStory::StoryListType& second, LockStory::GraphType& graph);
bool CheckIntersections(const LockStory::GraphType& graph);

} //! namespace atom::concurrency

#endif //! CONCURRENCY_SAFE_MUTEX_H
