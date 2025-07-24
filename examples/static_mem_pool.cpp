#include <thread>
#include <chrono>
#include <queue>
#include <iostream>
#include <random>
#include <cstdlib>

#include "include/memory/allocators/lock_free/static_memory_pool.h"
#include "include/memory/allocators/lock_free/static_allocator.h"

constexpr auto Id = 0;
constexpr auto MaxBlockSize = 1024;
constexpr auto Capacity = 1024 * 1024;
constexpr auto CacheSize = Capacity / 4;
using MemoryPool = atom::memory::allocator::lock_free::StaticMemoryPool<Id, MaxBlockSize, Capacity, CacheSize>;

struct Malloc {
    std::byte* allocate() {
        auto ptr = malloc(MaxBlockSize);
        if (ptr)
            return reinterpret_cast<std::byte*>(ptr);
        else
            throw std::runtime_error("Bad malloc");
    }

    void deallocate(std::byte* ptr) {
        free(reinterpret_cast<void*>(ptr));
    }

    void dumpStats(std::ostream& /*os*/) {

    }
};

MemoryPool gMemPool;
Malloc gMalloc;

template<typename F>
void MeasurePerfInNanosec(F&& f) {
    const auto start = std::chrono::high_resolution_clock::now();
    std::forward<F>(f)();
    const auto end = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "Execution time: " << duration.count() << " nanosec\n";
}

template<typename A, std::size_t ThreadNumber, std::size_t AllocationNumber, std::size_t DeallocatingNumber>
void Test(A& allocator) {
    static_assert(ThreadNumber > 0, "Must have at least one thread");
    static_assert(AllocationNumber >= DeallocatingNumber, "Cannot deallocate more than allocated");

    constexpr std::size_t TotalOps = AllocationNumber + DeallocatingNumber;
    static_assert(TotalOps % ThreadNumber == 0, "Operations must divide evenly across threads");

    enum class OptType : std::uint8_t { Allocate = 0, Deallocate };
    std::array<OptType, TotalOps> opts;
    for (auto i = 0; i < AllocationNumber; ++i) {
        opts[i] = OptType::Allocate;
    }
    for (auto i = 0; i < DeallocatingNumber; ++i) {
        opts[i + AllocationNumber] = OptType::Deallocate;
    }
    {
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(opts.begin(), opts.end(), g);
    }
    constexpr std::size_t OpsPerThread = TotalOps / ThreadNumber;
    std::array<std::thread, ThreadNumber> threads;

    for (std::size_t i = 0; i < ThreadNumber; ++i) {
        threads[i] = std::thread([i, &opts, &allocator] {
            std::vector<std::byte*> allocated;
            allocated.reserve(OpsPerThread);

            const auto start = i * OpsPerThread;
            const auto end = start + OpsPerThread;

            for (std::size_t j = start; j < end; ++j) {
                switch (opts[j]) {
                    case OptType::Allocate:
                        allocated.push_back(allocator.allocate());
                        break;
                    case OptType::Deallocate:
                        if (!allocated.empty()) {
                            allocator.deallocate(allocated.back());
                            allocated.pop_back();
                        }
                        break;
                } // end switch
            } // end for

            // Cleanup
            for (auto it = allocated.rbegin(); it != allocated.rend(); ++it) {
                allocator.deallocate(*it);
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
    std::cout << "Start Malloc perf test" << "\n";
    MeasurePerfInNanosec([] {
        Test<Malloc, 128, 1024 * 1024, 1024 * 1024>(gMalloc);
    });

    std::cout << "\n\n";

    std::cout << "Start Static Memory pool perf test" << "\n";
    MeasurePerfInNanosec([] {
        Test<MemoryPool, 128, 1024 * 1024, 1024 * 1024>(gMemPool);
    });
    return 0;
}
