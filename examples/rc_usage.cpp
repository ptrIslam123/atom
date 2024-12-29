#include <iostream>
#include <vector>
#include <array>
#include <thread>

#include "include/utils/lazy.h"
#include "include/utils/owner.h"

using namespace atom::utils;

void UseDanglingReference() {
    auto getRef = [] {
        Owner<int> localVar{ 10 };
        // do something with localVar;
        return localVar.getRef();
    };
    auto ref = getRef();
    ref.accessImmutable([](int v) {
        std::cout << v << std::endl;
    });
}

void CorrectUsingRef1() {
    MutableOwner<int> localVar{ 10 };
    {
        auto ref = localVar.getMutableRef();
        ref.accessMutable([](int& v) {
            v = v * v;
        });
    }
    {
        auto ref = localVar.getRef();
        ref.accessImmutable([](const int& v) {
            std::cout << v << std::endl;
        });
    }
    {
        auto r1 = localVar.getRef();
        auto r2 = r1;
    }
}

// void CorrectUsingRef2() {
//     MutableOwner<int> localVar{ 10 };
//     auto r1 = localVar.getRef();
//     auto r2 = r1;

//     r1.accessImmutable([](const int& v) {
//         std::cout << v << std::endl;
//     });
// }

// void UsageRef() {
//     MutableOwner<int> data{ 100 };
//     std::thread workerThread([ ref = data.getMutableRef() ]() mutable {
//         std::this_thread::sleep_for(std::chrono::milliseconds(100)); // some hard work
//         ref.accessMutable([](int& data) {
//             data = data * data;
//         });
//     });

//     workerThread.detach();
//     // do something another!
// }

int main() {
    UseDanglingReference();
    return 0;
}
