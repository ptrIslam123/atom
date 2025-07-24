#include <thread>
#include <chrono>
#include <queue>
#include <iostream>
#include <random>
#include <cmath>
#include <cstdlib>

#include "include/memory/allocators/lock_free/static_allocator.h"

enum class OptType : std::uint8_t {
    Allocate = 0,
    Deallocate
};

constexpr std::size_t THREAD_NUMBER = 128;
constexpr std::size_t ALLOCATION_NUMBER = 1024 * 64;
constexpr std::size_t DEALLOCATION_NUMBER = ALLOCATION_NUMBER;

constexpr std::size_t N64 = 1024 * 64;
constexpr std::size_t N128 = 1024 * 64;
constexpr std::size_t N512 = 1024 * 64;
constexpr std::size_t N1024 = 1024 * 64;
constexpr std::size_t N4096 = 1024 * 32;
constexpr std::size_t N8192 = 1024;
constexpr std::size_t N16384 = 0;
constexpr std::size_t N32768 = 0;
constexpr std::size_t N65536 = 0;
using Allocator = atom::memory::allocator::lock_free::StaticAllocator<void*, N64, N128, N512, N1024, N4096, N8192, N16384, N32768, N65536>;

struct Malloc {
    std::byte* allocate(std::size_t size) {
        auto ptr = malloc(size);
        if (ptr)
            return reinterpret_cast<std::byte*>(ptr);
        else
            throw std::runtime_error("Bad malloc");
    }

    void deallocate(std::byte* ptr, std::size_t /*size*/) {
        free(reinterpret_cast<void*>(ptr));
    }

    void dumpStats(std::ostream& /*os*/) {

    }
};

template<typename F>
void MeasurePerfInNanosec(F&& f) {
    const auto start = std::chrono::high_resolution_clock::now();
    std::forward<F>(f)();
    const auto end = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "Execution time: " << duration.count() << " nanosec\n";
}

std::size_t getMemReqSize() {
    static std::size_t index = 0;
    static const auto randMemReqStorage = [] {
        std::random_device rd;
        std::mt19937 gen(rd());

        std::vector<std::size_t> buffer;
        buffer.reserve(ALLOCATION_NUMBER);
        auto generate = [&buffer, &gen](const double percent, const std::size_t from, const std::size_t to) {
            std::uniform_int_distribution<std::size_t> rand{from, to};
            const auto n = static_cast<std::size_t>(percent / 100.0 * ALLOCATION_NUMBER);
            for (auto i = 0; i < n; ++i) {
                buffer.push_back(rand(gen));
            }
        };
        generate(70.0, 0, 1023);      // 0–1023 (70%)
        generate(27.0, 1024, 4095);   // 1024–4095 (27%)
        generate(3.0, 4096, 8192);    // 4096–8192 (3%)
        std::shuffle(buffer.begin(), buffer.end(), gen);
        return buffer;
    }();
    assert(!randMemReqStorage.empty());
    const auto result = randMemReqStorage[index % randMemReqStorage.size()];
    ++index;
    return result;
}

OptType getOpt() {
    static std::size_t index = 0;
    static const auto opts = [] {
        std::random_device rd;
        std::mt19937 gen(rd());

        constexpr std::size_t totalOps = ALLOCATION_NUMBER + DEALLOCATION_NUMBER;
        std::vector<OptType> buffer;
        buffer.reserve(totalOps);

        for (auto i = 0; i < ALLOCATION_NUMBER; ++i) {
            buffer.push_back(OptType::Allocate);
        }
        for (auto i = 0; i < DEALLOCATION_NUMBER; ++i) {
            buffer.push_back(OptType::Deallocate);
        }
        std::shuffle(buffer.begin(), buffer.end(), gen);
        return buffer;
    }();
    assert(!opts.empty());
    const auto result = opts[index % opts.size()];
    ++index;
    return result;
}

template<typename A>
void Test(A& allocator) {
    constexpr std::size_t TotalOps = ALLOCATION_NUMBER + DEALLOCATION_NUMBER;
    constexpr std::size_t OpsPerThread = TotalOps / THREAD_NUMBER;
    std::array<std::thread, THREAD_NUMBER> threads;
    for (std::size_t i = 0; i < THREAD_NUMBER; ++i) {
        threads[i] = std::thread([i, &allocator] {
            std::vector<std::pair<std::byte*, std::size_t>> allocated;
            allocated.reserve(OpsPerThread);

            const auto start = i * OpsPerThread;
            const auto end = start + OpsPerThread;

            for (std::size_t j = start; j < end; ++j) {
                switch (getOpt()) {
                    case OptType::Allocate: {
                        const auto size = getMemReqSize();
                        allocated.push_back(std::make_pair(allocator.allocate(size), size));
                        break;
                    }
                    case OptType::Deallocate: {
                        if (!allocated.empty()) {
                            const auto [ptr, size] = allocated.back();
                            allocator.deallocate(ptr, size);
                            allocated.pop_back();
                        }
                        break;
                    }
                } // end switch
            } // end for

            // Cleanup
            for (auto it = allocated.rbegin(); it != allocated.rend(); ++it) {
                allocator.deallocate(it->first, it->second);
            }
        }); // end thread
    } // end for

    for (auto& thread : threads) {
        if (thread.joinable())
            thread.join();
    }

    allocator.dumpStats(std::cout);
}

int main() {
    (void)getOpt();
    (void)getMemReqSize();

    auto allocator = std::make_unique<Allocator>();
    auto malloc = std::make_unique<Malloc>();

    std::cout << "Start malloc perf test" << "\n";
    MeasurePerfInNanosec([malloc = malloc.get()] {
        Test<Malloc>(*malloc);
    });

    std::cout << "\n\n";

    std::cout << "Start static lock free allocator perf test" << "\n";
    MeasurePerfInNanosec([allocator = allocator.get()] {
        Test<Allocator>(*allocator);
    });
    return 0;
}
