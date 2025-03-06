#include <thread>
#include <mutex>

#include "include/utils/tsan_object.h"

using namespace atom::utils::tsan;

constexpr auto N = 100000;
Object<int> GCounter;
std::mutex GMutex;

void DataRaceTest() {
   std::thread t1{[] {
       for (auto i = 0; i < N; ++i) {
           int newValue = GCounter.getValue() + 1;
           GCounter.setValue(newValue);
       }
   }};

   std::thread t2{[]() {
       for (auto i = 0; i < N; ++i) {
           int newValue = GCounter.getValue() - 1;
           GCounter.setValue(newValue);
       }
   }};

   t1.join(), t2.join();
   assert(GCounter.getValue() == 0);
}

void NoDataRaceTest() {
   std::thread t1{[] {
       for (auto i = 0; i < N; ++i) {
           std::lock_guard lock{ GMutex };
           int newValue = GCounter.getValue() + 1;
           GCounter.setValue(newValue);
       }
   }};

   std::thread t2{[]() {
       for (auto i = 0; i < N; ++i) {
           std::lock_guard lock{ GMutex };
           int newValue = GCounter.getValue() - 1;
           GCounter.setValue(newValue);
       }
   }};

   t1.join(), t2.join();
   assert(GCounter.getValue() == 0);
}

struct Foo {};

int main() {
    DataRaceTest();
    return 0;
}
