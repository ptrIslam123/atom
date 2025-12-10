#ifndef FUNCTIONAL_TEST_EXCHANGE_DATA_H
#define FUNCTIONAL_TEST_EXCHANGE_DATA_H

#include <thread>
#include <atomic>
#include <array>
#include <span>
#include <string>
#include <sstream>
#include <random>
#include <functional>

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cassert>

class ExchangeData {
public:
    static constexpr auto SIZE{64 + 2 * sizeof(std::size_t) + sizeof(std::thread::id)};

    ExchangeData() = default;
    ExchangeData(const ExchangeData& ) = default;
    ExchangeData(ExchangeData&& ) = default;
    ExchangeData& operator=(const ExchangeData& ) = default;
    ExchangeData& operator=(ExchangeData&& ) = default;

    bool operator==(const ExchangeData& other) const noexcept {
        return (orderIndex == other.orderIndex) &&
                (checkSum == other.checkSum) &&
                (std::memcmp(str.data(), other.str.data(), str.size()) == 0) &&
                (tid == tid);
    }

    bool operator!=(const ExchangeData& other) const noexcept {
        return !operator==(other);
    }

    std::size_t getOrder() const noexcept {
        return orderIndex;
    }

    static ExchangeData Gen() {
        static std::atomic<std::size_t> staticOrderIndex{1};
        ExchangeData data;
        data.tid = std::this_thread::get_id();
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

    static void Serialize(const ExchangeData& data, std::span<std::byte, ExchangeData::SIZE> buffer) noexcept {
        assert(!buffer.empty());
        std::size_t offset = 0;

        std::memcpy(buffer.data() + offset, &data.orderIndex, sizeof(data.orderIndex));
        offset += sizeof(data.orderIndex);

        std::memcpy(buffer.data() + offset, &data.checkSum, sizeof(data.checkSum));
        offset += sizeof(data.checkSum);

        std::memcpy(buffer.data() + offset, data.str.data(), data.str.size());
        offset += data.str.size();

        const std::thread::id tid = data.tid;
        std::memcpy(buffer.data() + offset, &tid, sizeof(tid));
    }

    static void Deserialize(std::span<std::byte, ExchangeData::SIZE> buffer, ExchangeData& data) noexcept {
        assert(!buffer.empty());

        std::size_t offset = 0;

        std::memcpy(&data.orderIndex, buffer.data() + offset, sizeof(data.orderIndex));
        offset += sizeof(data.orderIndex);

        std::memcpy(&data.checkSum, buffer.data() + offset, sizeof(data.checkSum));
        offset += sizeof(data.checkSum);

        std::memcpy(data.str.data(), buffer.data() + offset, data.str.size());
        offset += data.str.size();

        std::memcpy(&data.tid, buffer.data() + offset, sizeof(data.tid));
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

    std::size_t orderIndex{0};
    std::size_t checkSum{0};
    std::array<char, 64> str{0};
    std::thread::id tid{};
};

#endif //! FUNCTIONAL_TEST_EXCHANGE_DATA_H
