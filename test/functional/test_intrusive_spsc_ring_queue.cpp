#include "include/containers/lock_free/intrusive/spsc_ring_queue.h"
#include "include/utils/assertion.h"

#include <iostream>
#include <stdexcept>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <queue>
#include <optional>
#include <string>
#include <array>
#include <sstream>
#include <random>
#include <algorithm>

#include <cstring>
#include <ctime>
#include <cassert>

using namespace atom::containers::lock_free::spsc;

class ExchangeData {
public:
    static ExchangeData Gen(const std::thread::id& tid) {
        static std::atomic<std::size_t> staticOrderIndex{1};
        ExchangeData data;
        data.tid = tid;
        data.orderIndex = staticOrderIndex.fetch_add(1);

        std::ostringstream oss;
        oss << data.tid << ':' << data.orderIndex;

        const std::string prefix = oss.str();
        auto src{&data.str[0]};
        std::memcpy(src, prefix.data(), prefix.size());
        src += prefix.size();

        const std::string randomStr{GenRandomStr(sizeof(data.str) - prefix.size())};
        std::memcpy(src, randomStr.data(), randomStr.size());

        data.checkSum = std::hash<std::string_view>{}(std::string_view{&data.str[0], 64});
        return data;
    }

    static bool CheckSum(const ExchangeData& data) noexcept {
        return data.checkSum == std::hash<std::string_view>{}(std::string_view{&data.str[0], 64});
    }

private:
    static std::string GenRandomStr(std::size_t n) {
        static const char charset[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";

        static thread_local std::mt19937 rng(
            std::random_device{}() +
            std::hash<std::thread::id>{}(std::this_thread::get_id())
        );

        std::string result;
        result.reserve(n);

        std::uniform_int_distribution<> dist(0, sizeof(charset) - 2);

        for (std::size_t i = 0; i < n; ++i) {
            result += charset[dist(rng)];
        }
        assert(result.size() == n);
        return result;
    }

    std::size_t checkSum{0};
    std::array<char, 64> str{0};
    std::size_t orderIndex{0};
    std::thread::id tid{};
};

constexpr auto OPERATIONS{1024 * 64};

std::atomic<std::size_t> enqueued{0};
std::atomic<std::size_t> dequeued{0};

std::shared_ptr<IntrusiveRingQueue<ExchangeData, OPERATIONS * 2>> Queue;

void ProducerJob() {
    std::array<ExchangeData, OPERATIONS> buffer;
    for (auto i = 0; i < OPERATIONS; ++i) {
        buffer[i] = ExchangeData::Gen(std::this_thread::get_id());
        const auto result = Queue->tryEnqueue(&buffer[i]);
        ASSERTION(result, std::runtime_error, "Could not enqueue")
        enqueued.fetch_add(1);
    }

    std::cout << "Producer finished job! (enqueued=" << enqueued.load() << " data)" << std::endl;
}

void ConsumerJob() {
    for (auto i = 0; i < OPERATIONS; ++i) {
        ExchangeData* data{nullptr};
        while (Queue->empty()) {
            std::this_thread::yield();
        }
        const auto result = Queue->tryDequeue(&data);
        ASSERTION(result, std::runtime_error, "Could not dequeue")
        ASSERTION(data, std::runtime_error, "Dequeue invalid pointer")
        ASSERTION(ExchangeData::CheckSum(*data), std::runtime_error, "Incorrect check sum")
        dequeued.fetch_add(1);
    }

    std::cout << "Consumer finished job! (dequeued=" << dequeued.load() << " data)" << std::endl;
}

void TestEnqueueAndDequeue() {

}

int main() {
    Queue = std::make_shared<IntrusiveRingQueue<ExchangeData, OPERATIONS * 2>>();
    std::thread producer{[] { ProducerJob(); }};
    std::thread consumer{[] { ConsumerJob(); }};

    producer.join();
    consumer.join();

    ASSERTION(enqueued.load() == dequeued.load(), std::runtime_error, "Incorrent counters: enqueued != dequeued")
    return 0;
}
