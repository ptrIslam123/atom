#pragma once

#include "containers/lock-free/spsc_ring_queue.h"

#include <vector>
#include <atomic>
#include <span>
#include <thread>
#include <type_traits>

#include <cassert>
#include <cstddef>
#include <cstdint>

namespace atom::containers::lock_free::mpsc {

class RingQueue final {
public:
    template<typename Func>
    bool tryWrite(std::span<std::byte> data, Func&& func) {
        thread_local Writer writer;
        register_writer(writer);
        return writer.tryWrite(data);
    }

    template<typename Func>
    bool tryRead() {

    }

private:
    void register_writer(Writer& writer) {
        // TODO
    }

    struct Writer {
        std::uint64_t getLocalcTimestamp() {
            // нужно возвращать число тиков с предыдущего вызова и максимально быстро, per-thread clock monotonic
            return 0; // TODO
        }

        std::uint64_t getGlobalTimestamp() {
            // тут нужн глобальное и полноценное время но тоже максимально быстро
            return 0; // TODO
        }

        bool tryWrite(std::span<std::byte> data) {
            return queue.tryWrite(data.size() + 2 * sizeof(std::uint64_t),
                                  [&](std::span<std::byte> mem1, std::span<std::byte> mem2, std::size_t queueSize) {
                                      if (queueSize == 0) {
                                          gTimestamp = get_global_timestamp();
                                      }
                                      const auto lTimestamp = get_localc_timestamp();
                                      // serialize: gTimestamp, lTimestamp, data
                                  });
        }

        bool tryRead(std::span<std::byte> data) {
            return queue.tryRead(data.size(),
                                 [&](std::span<std::byte> mem1, std::span<std::byte> mem2, std::size_t queueSize) {
                                     // recopy from mem1, mem2 to data
                                 });
        }

    private:
        lock_free::spsc::RingQueue<std::byte> m_queue;
        std::uint64_t m_gTimestamp;
    };

    struct Reader {
        std::vector<Writer*> writers;

        bool tryRead(std::vector<std::byte>& data) {
            std::vector<std::byte> accumulater;
            for (auto writer : writers) {
                writer->tryRead(accumulater);
            }

            // restore global order for records in accumulater
            // data.insert(data.end(), accumulater.begin(), accumulater.end());
            return true;
        }
    };
};
