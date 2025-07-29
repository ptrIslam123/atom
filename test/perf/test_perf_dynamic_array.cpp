#include "include/containers/dynamic/array.h"
#include "include/utils/measure_time.h"

#include <vector>
#include <type_traits>
#include <typeinfo>
#include <iostream>

/*
замер perf:
$ sudo perf stat -e cache-references,cache-misses,L1-dcache-loads,L1-dcache-load-misses,LLC-loads,LLC-load-misses -e cycles,instructions,stalled-cycles-frontend,stalled-cycles-backend,branch-misses,branch-load-misses ./test_perf_dynamic_array
Vector type name: N4atom10containers12DynamicArrayIiNS_6memory9allocator16DefaultAllocatorENS0_35DefaultDynamicArrayGrowthPolicyTypeIS4_EEEE
push back number: 100 & 1000 & 10000 & 100000 & 1000000 & 10000000
perf time(in nanosec): 23538 & 111954 & 1154560 & 10230276 & 89998616 & 86386953


 Performance counter stats for './test_perf_dynamic_array':

     <not counted>      cpu_atom/cache-references/                                              (0,00%)
           570 536      cpu_core/cache-references/                                              (89,97%)
     <not counted>      cpu_atom/cache-misses/                                                  (0,00%)
           367 277      cpu_core/cache-misses/           #   64,37% of all cache refs           (89,93%)
     <not counted>      cpu_atom/L1-dcache-loads/                                               (0,00%)
     1 077 150 172      cpu_core/L1-dcache-loads/                                               (89,96%)
   <not supported>      cpu_atom/L1-dcache-load-misses/
           174 476      cpu_core/L1-dcache-load-misses/  #    0,02% of all L1-dcache accesses   (89,98%)
     <not counted>      cpu_atom/LLC-loads/                                                     (0,00%)
            43 000      cpu_core/LLC-loads/                                                     (89,96%)
     <not counted>      cpu_atom/LLC-load-misses/                                               (0,00%)
            12 720      cpu_core/LLC-load-misses/        #   29,58% of all LL-cache accesses    (89,93%)
     <not counted>      cpu_atom/cycles/                                                        (0,00%)
       784 872 692      cpu_core/cycles/                                                        (79,90%)
     <not counted>      cpu_atom/instructions/                                                  (0,00%)
     2 869 138 499      cpu_core/instructions/           #    3,66  insn per cycle              (89,95%)
   <not supported>      cpu_atom/stalled-cycles-frontend/
   <not supported>      cpu_core/stalled-cycles-frontend/
   <not supported>      cpu_atom/stalled-cycles-backend/
   <not supported>      cpu_core/stalled-cycles-backend/
     <not counted>      cpu_atom/branch-misses/                                                 (0,00%)
            34 731      cpu_core/branch-misses/                                                 (90,06%)
     <not counted>      cpu_atom/branch-load-misses/                                            (0,00%)
            32 090      cpu_core/branch-load-misses/                                            (90,32%)

       0,190931154 seconds time elapsed

       0,186445000 seconds user
       0,002991000 seconds sys

Вывод по perf:
    Эффективность кэширования:

        L1-кэш: 174,476 промахов (0.02% от 1.07 млрд доступов) - отличный показатель

        LLC (Last Level Cache): 12,720 промахов (29.58% от обращений) - высокий процент, стоит оптимизировать

        Общие cache-misses: 367,277 (64.37% от cache-references) - высоковато

    Инструкции и циклы:

        3.66 инструкций/цикл - хороший показатель (близко к теоретическому максимуму для современных CPU)

        34,731 branch-misses - приемлемо для такого объёма операций

    Время выполнения:

        Общее время: ~191ms (для 10M операций push_back)

        ~19.1ns на операцию в среднем

Проблемные места:

    Высокий процент промахов LLC (L3-кэш):

        29.58% промахов в последнем уровне кэша указывает на:

            Частые реаллокации массива

            Проблемы с локальностью данных

            Неоптимальный доступ к памяти

    Высокий процент общих cache-misses (64.37%):

        Сигнализирует о неэффективном использовании кэшей CPU

    Несимметричная загрузка ядер:

        Все значимые события зафиксированы только на cpu_core

        cpu_atom события не зафиксированы (возможно, не задействован)
*/

using namespace atom::utils;
using ElementType = int;
using AtomVector= atom::containers::DynamicArray<ElementType>;
using StdVector = std::vector<ElementType>;

template<class Vector>
void TestPerfPushBack(const std::size_t n) {
    Vector buffer;
    for (auto i = 0; i < n; ++i) {
        if constexpr (std::is_same_v<Vector, AtomVector>) {
            buffer.pushBack(i);
        } else {
            buffer.push_back(i);
        }
    }
}

template<class Vector>
void TestPushBackAmortization() {
    constexpr std::size_t n1 = 100;
    constexpr std::size_t n2 = n1 * 10;
    constexpr std::size_t n3 = n1 * 100;
    constexpr std::size_t n4 = n1 * 1000;
    constexpr std::size_t n5 = n1 * 10000;
    constexpr std::size_t n6 = n1 * 100000;

    const auto time1 = MeasureTimeNanosec([&] {
        TestPerfPushBack<Vector>(n1);
    });

    const auto time2 = MeasureTimeNanosec([&] {
        TestPerfPushBack<Vector>(n2);
    });

    const auto time3 = MeasureTimeNanosec([&] {
        TestPerfPushBack<Vector>(n3);
    });

    const auto time4 = MeasureTimeNanosec([&] {
        TestPerfPushBack<Vector>(n4);
    });

    const auto time5 = MeasureTimeNanosec([&] {
        TestPerfPushBack<Vector>(n5);
    });

    const auto time6 = MeasureTimeNanosec([&] {
        TestPerfPushBack<Vector>(n5);
    });

    std::cout << "Vector type name: " << typeid(Vector).name() << "\n";
    std::cout << "push back number: " << n1 << " & " << n2 << " & " << n3 << " & " << n4 << " & " << n5 << " & " << n6 << "\n";
    std::cout << "perf time(in nanosec): " << time1 << " & " << time2 << " & " << time3 << " & " << time4 << " & " << time5 << " & " << time6 << "\n";
    std::cout << std::endl;
}

int main() {
    //TestPushBackAmortization<StdVector>();
    TestPushBackAmortization<AtomVector>();
    return 0;
}
