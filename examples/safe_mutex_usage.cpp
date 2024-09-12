#include "include/concurrency/safe_mutex.h"

#include <iostream>
#include <thread>

using namespace atom;

void case1() {
    /* Normal case, not deadlock */
    int Value = 0;
    concurrency::SafeMutex Mutex;

    auto Foo = [&Value, &Mutex](concurrency::LockStory& lockStory) {
        for (auto i = 0; i < 1000; ++i) {
            Mutex.lock(lockStory);
            ++Value;
            Mutex.unlock(lockStory);
        }
    };

    auto Bar = [&Value, &Mutex](concurrency::LockStory& lockStory) {
        for (auto i = 0; i < 1000; ++i) {
            Mutex.lock(lockStory);
            --Value;
            Mutex.unlock(lockStory);
        }
    };

    std::thread t1{[Foo]{
        concurrency::LockStory lockStory;
        Foo(lockStory);
    }};

    std::thread t2{[Bar]{
        concurrency::LockStory lockStory;
        Bar(lockStory);
    }};

    t1.join(); t2.join();

    assert(Value == 0);
}

void case2() {
    /* Bad case, we have a deadlock */
    int Value = 0;
    concurrency::SafeMutex Mutex1;
    concurrency::SafeMutex Mutex2;

    auto Foo = [&](concurrency::LockStory& lockStory) {
        for (auto i = 0; i < 1000; ++i) {
            Mutex1.lock(lockStory);
            Mutex2.lock(lockStory);

            ++Value;

            Mutex2.unlock(lockStory);
            Mutex1.unlock(lockStory);
        }
    };

    auto Bar = [&](concurrency::LockStory& lockStory) {
        for (auto i = 0; i < 1000; ++i) {
            Mutex2.lock(lockStory);
            Mutex1.lock(lockStory);

            --Value;

            Mutex1.unlock(lockStory);
            Mutex2.unlock(lockStory);
        }
    };

    std::thread t1{[Foo]{
        concurrency::LockStory lockStory;
        Foo(lockStory);
    }};

    std::thread t2{[Bar]{
        concurrency::LockStory lockStroy;
        Bar(lockStroy);
    }};

    t1.join(); t2.join();
    assert(Value == 0);
}

void case3() {
    int Value = 0;
    concurrency::SafeMutex Mutex1;
    concurrency::LockStory lockStory;

    Mutex1.lock(lockStory);
    Mutex1.lock(lockStory);
    Mutex1.unlock(lockStory);
    Mutex1.unlock(lockStory);
}

int main() {
    case1();
    return 0;
}
