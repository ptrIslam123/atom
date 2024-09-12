#include "include/concurrency/safe_mutex.h"

#include "include/utils/assertion.h"

#include <array>

namespace {

struct Node;
struct NodeHash;

using namespace atom::concurrency;

struct Node final {
    const SafeMutex* fromMutex;
    const SafeMutex* toMutex;

    explicit Node(const SafeMutex* _fromMutex, const SafeMutex* _toMutex);
    bool operator==(const Node& other) const;
};

struct NodeHash final {
    explicit NodeHash() = default;
    std::size_t operator()(const Node& node) const noexcept;
};


Node::Node(const SafeMutex* _fromMutex, const SafeMutex* _toMutex):
fromMutex(_fromMutex), toMutex(_toMutex)
{}

inline bool Node::Node::operator==(const Node& other) const {
    return NodeHash{}.operator()(other) == NodeHash{}.operator()(*this);
}

inline std::size_t NodeHash::NodeHash::operator()(const Node& node) const noexcept {
    return ((std::uintptr_t)(node.fromMutex) + (std::uintptr_t)(node.toMutex)) % std::numeric_limits<std::size_t>::max();
}

} //! namespace

namespace atom::concurrency {

void MakeGraph(const LockStory::StoryListType& first, const LockStory::StoryListType& second, LockStory::GraphType& graph) {
    const std::array w = { first, second };
    for (const auto& storyStorage : w) {
        auto currentIt = storyStorage.cbegin();
        auto nextIt = std::next(currentIt);
        while (nextIt != storyStorage.cend()) {
            assert(*currentIt != nullptr && *nextIt != nullptr);

            graph.insert(std::make_pair(*currentIt, *nextIt));
            ++currentIt;
            ++nextIt;
        }
    }
}

bool CheckIntersections(const LockStory::GraphType& graph) {
    if (graph.size() == 1) {
        const SafeMutex* fromMutex = graph.cbegin()->first;
        const SafeMutex* toMutex = graph.cbegin()->second;
        return (fromMutex == toMutex);
    }

    std::unordered_set<Node, NodeHash> visited;
    for (auto it = graph.begin(); it != graph.end(); ++it) {
        const SafeMutex* fromMutex = it->first;
        const SafeMutex* toMutex = it->second;
        const Node node{ fromMutex, toMutex };

        const auto foundNodeIt = visited.find(node);
        if (foundNodeIt == visited.end()) {
            auto [_, inserted] = visited.insert(node);
            assert(inserted);
        } else {
            // To make sure that really same graph edges (in case hash collision)
            if (foundNodeIt->fromMutex == toMutex && foundNodeIt->toMutex == fromMutex) {
                return true;
            }
        }
    }

    return false;
}

#ifndef NDEBUG

LockStory::LockStory(std::size_t storySizeHint):
m_storage() {
    m_storage.reserve(storySizeHint);
}

void LockStory::add(const SafeMutex& safeMutex) {
    m_storage.push_back(&safeMutex);
}

void LockStory::remove(const SafeMutex& safeMutex) {
    const auto it = std::find(m_storage.cbegin(), m_storage.cend(), &safeMutex);
    if (it != m_storage.cend()) {
        m_storage.erase(it);
    }
}

SafeMutex::SafeMutex():
m_graph(),
m_currentOwner(nullptr),
m_waitQueue(),
m_condVar(),
m_mutex(),
m_flag(false)
{}

void SafeMutex::lock(LockStory& lockStory)
{
    std::unique_lock lock{ m_mutex };
    lockStory.add(*this);

    if (!m_currentOwner) {
        m_currentOwner = &lockStory;
    } else {
        {
            m_graph.clear();
            MakeGraph(m_currentOwner->m_storage, lockStory.m_storage, m_graph);
            PANIC(CheckIntersections(m_graph));
        }

        m_waitQueue.push(&lockStory);
        m_condVar.wait(lock, [this]{ return m_flag; });
        m_waitQueue.pop();
        m_flag = false;
        m_currentOwner = &lockStory;
    }
}

void SafeMutex::unlock(LockStory& lockStory)
{
    assert(m_currentOwner == &lockStory);
    {
        std::lock_guard lock{ m_mutex };
        lockStory.remove(*this);

        if (m_waitQueue.empty()) {
            m_currentOwner = nullptr;
            lockStory.remove(*this);
            return;
        }

        m_flag = true;
    }
    m_condVar.notify_one();
}

#endif //! ifdef NDEBUG

} //! namespace atom::concurrency
