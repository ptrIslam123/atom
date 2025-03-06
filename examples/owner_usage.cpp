#include <iostream>

#include "include/utils/owner.h"

using namespace atom::utils;

void BadUseCase() {
    auto getDanglingReference = [] {
        Owner<int> obj{int{10}};
        return obj.borrowMutable();
    };

    auto ref = getDanglingReference(); // We expect PANIC!
    std::cout << ref << std::endl;
}

void GoodUseCase() {
    Owner<int> obj{int{10}};
    {
        auto cref = obj.borrowImmutable();
        std::cout << *cref << std::endl;
    }
}

int main() {
    GoodUseCase();
    BadUseCase();
    return 0;
}
