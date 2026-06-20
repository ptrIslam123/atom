#include <gtest/gtest.h>

#include "include/containers/lock_free/mpsc_ring_queue.h"

#include <vector>
#include <array>

template<class T, std::size_t N>
using Queue = atom::containers::lock_free::mpsc::RingQueue<T, N>;

// ============================================================================
// Тесты для базовых операций enqueue/dequeue
// ============================================================================

TEST(TestMPSCRingQueue, EnqueueDequeueSingleElement) {
    Queue<int, 16> queue;

    // Проверяем, что очередь пуста
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);

    // Добавляем элемент
    queue.enqueue(42);

    // Проверяем, что очередь не пуста
    EXPECT_FALSE(queue.empty());
    EXPECT_EQ(queue.size(), 1);

    // Извлекаем элемент
    int value{0};
    queue.dequeue(value);

    // Проверяем, что значение корректно
    EXPECT_EQ(value, 42);
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
}

TEST(TestMPSCRingQueue, EnqueueDequeueElements) {
    Queue<int, 16> queue;

    for (auto i = 0; i < queue.capacity(); ++i) {
        queue.enqueue(i);
        EXPECT_EQ(queue.size(), i + 1);
    }

    EXPECT_EQ(queue.size(), queue.capacity());

    for (int i = 0; i < queue.capacity(); ++i) {
        int value;
        queue.dequeue(value);
        EXPECT_EQ(value, i);
    }

    EXPECT_TRUE(queue.empty());
}

TEST(TestMPSCRingQueue, TryEnqueueTryDequeueSingleElement) {
    Queue<int, 16> queue;

    // Пытаемся извлечь из пустой очереди
    int value{0};
    EXPECT_FALSE(queue.tryDequeue(value));
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);

    // Добавляем элемент через tryEnqueue
    EXPECT_TRUE(queue.tryEnqueue(100));
    EXPECT_FALSE(queue.empty());
    EXPECT_EQ(queue.size(), 1);

    // Извлекаем через tryDequeue
    EXPECT_TRUE(queue.tryDequeue(value));
    EXPECT_EQ(value, 100);
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
}

// ============================================================================
// Тесты для множественных операций
// ============================================================================

TEST(TestMPSCRingQueue, EnqueueDequeueMultipleElements) {
    Queue<int, 128> queue;
    const int COUNT = 64;

    // Добавляем элементы
    for (int i = 0; i < COUNT; ++i) {
        queue.enqueue(i);
        EXPECT_EQ(queue.size(), i + 1);
    }

    EXPECT_FALSE(queue.empty());
    EXPECT_EQ(queue.size(), COUNT);

    // Извлекаем элементы и проверяем порядок (FIFO)
    for (int i = 0; i < COUNT; ++i) {
        int value{0};
        queue.dequeue(value);
        EXPECT_EQ(value, i);
        EXPECT_EQ(queue.size(), COUNT - i - 1);
    }

    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
}

TEST(TestMPSCRingQueue, TryEnqueueTryDequeueMultipleElements) {
    Queue<int, 128> queue;
    const int COUNT = 64;

    // Добавляем элементы через tryEnqueue
    for (int i = 0; i < COUNT; ++i) {
        EXPECT_TRUE(queue.tryEnqueue(i));
        EXPECT_EQ(queue.size(), i + 1);
    }

    EXPECT_FALSE(queue.empty());
    EXPECT_EQ(queue.size(), COUNT);

    // Извлекаем через tryDequeue
    for (int i = 0; i < COUNT; ++i) {
        int value{0};
        EXPECT_TRUE(queue.tryDequeue(value));
        EXPECT_EQ(value, i);
        EXPECT_EQ(queue.size(), COUNT - i - 1);
    }

    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
}

// ============================================================================
// Тесты FIFO порядка
// ============================================================================

TEST(TestMPSCRingQueue, FIFOOrder) {
    Queue<int, 16> queue;
    std::vector<int> input = {10, 20, 30, 40, 50};
    std::vector<int> output;

    // Добавляем все элементы
    for (int val : input) {
        queue.enqueue(val);
    }

    EXPECT_EQ(queue.size(), input.size());

    // Извлекаем все элементы
    while (!queue.empty()) {
        int value{0};
        queue.dequeue(value);
        output.push_back(value);
    }

    // Проверяем, что порядок сохранен
    EXPECT_EQ(output, input);
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
}

TEST(TestMPSCRingQueue, FIFOOrderWithInterleavedOperations) {
    Queue<int, 16> queue;
    std::vector<int> expected = {20, 30, 40, 50};

    // Добавляем 3 элемента
    queue.enqueue(10);
    queue.enqueue(20);
    queue.enqueue(30);

    EXPECT_EQ(queue.size(), 3);

    // Извлекаем 1 элемент
    int value{0};
    queue.dequeue(value);
    EXPECT_EQ(value, 10);
    EXPECT_EQ(queue.size(), 2);

    // Добавляем еще 2 элемента
    queue.enqueue(40);
    queue.enqueue(50);

    EXPECT_EQ(queue.size(), 4);

    // Извлекаем оставшиеся 4 элемента
    for (int exp : expected) {
        queue.dequeue(value);
        EXPECT_EQ(value, exp);
    }

    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
}

// ============================================================================
// Тесты для empty() и size()
// ============================================================================

TEST(TestMPSCRingQueue, EmptyAndSize) {
    Queue<int, 32> queue;

    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);

    // Добавляем элемент
    queue.enqueue(1);
    EXPECT_FALSE(queue.empty());
    EXPECT_EQ(queue.size(), 1);

    // Добавляем еще
    queue.enqueue(2);
    EXPECT_FALSE(queue.empty());
    EXPECT_EQ(queue.size(), 2);

    // Извлекаем один
    int value{0};
    queue.dequeue(value);
    EXPECT_FALSE(queue.empty());
    EXPECT_EQ(queue.size(), 1);
    EXPECT_EQ(value, 1);

    // Извлекаем второй
    queue.dequeue(value);
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
    EXPECT_EQ(value, 2);
}

TEST(TestMPSCRingQueue, SizeAfterMultipleOperations) {
    Queue<int, 32> queue;

    // Добавляем 10 элементов
    for (int i = 0; i < 10; ++i) {
        queue.enqueue(i);
        EXPECT_EQ(queue.size(), i + 1);
    }

    // Извлекаем 5 элементов
    for (int i = 0; i < 5; ++i) {
        int value{0};
        queue.dequeue(value);
        EXPECT_EQ(queue.size(), 10 - i - 1);
        EXPECT_EQ(value, i);
    }

    // Добавляем еще 5 элементов
    for (int i = 0; i < 5; ++i) {
        queue.enqueue(i + 100);
        EXPECT_EQ(queue.size(), 5 + i + 1);
    }

    // Итоговый размер должен быть 10 (5 оставшихся + 5 новых)
    EXPECT_EQ(queue.size(), 10);
    EXPECT_FALSE(queue.empty());

    // Извлекаем все оставшиеся
    int count = 0;
    while (!queue.empty()) {
        int value{0};
        queue.dequeue(value);
        ++count;
        EXPECT_EQ(queue.size(), 10 - count);
    }

    EXPECT_EQ(count, 10);
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
}

// ============================================================================
// Тесты для capacity()
// ============================================================================

TEST(TestMPSCRingQueue, Capacity) {
    Queue<int, 8> queue;
    EXPECT_EQ(queue.capacity(), 8);

    Queue<int, 16> queue2;
    EXPECT_EQ(queue2.capacity(), 16);

    Queue<int, 32> queue3;
    EXPECT_EQ(queue3.capacity(), 32);
}

// ============================================================================
// Тесты для полной очереди (переполнение)
// ============================================================================

TEST(TestMPSCRingQueue, FullQueueTryEnqueue) {
    using Queue = Queue<int, 4>;
    Queue queue;

    // Заполняем очередь до максимума
    for (int i = 0; i < Queue::CAPACITY; ++i) {
        EXPECT_TRUE(queue.tryEnqueue(i));
    }

    EXPECT_EQ(queue.size(), Queue::CAPACITY);
    EXPECT_FALSE(queue.empty());

    // Попытка добавить еще один элемент должна вернуть false
    EXPECT_FALSE(queue.tryEnqueue(100));
    EXPECT_EQ(queue.size(), Queue::CAPACITY);

    // Извлекаем один элемент
    int value{0};
    EXPECT_TRUE(queue.tryDequeue(value));
    EXPECT_EQ(value, 0);
    EXPECT_EQ(queue.size(), Queue::CAPACITY - 1);

    // Теперь можно добавить новый элемент
    EXPECT_TRUE(queue.tryEnqueue(100));
    EXPECT_EQ(queue.size(), Queue::CAPACITY);
}

TEST(TestMPSCRingQueue, FullQueueEnqueueBlocks) {
    using Queue = Queue<int, 4>;
    Queue queue;

    // Заполняем очередь до максимума
    for (int i = 0; i < Queue::CAPACITY; ++i) {
        EXPECT_TRUE(queue.tryEnqueue(i));
    }

    // Добавляем элемент (должен блокироваться до появления места)
    // В этом тесте мы проверим, что enqueue не падает,
    // но мы не можем проверить блокировку в однопоточном тесте
    // Просто добавим его через дефолтный enqueue (он будет ждать)

    // Создаем поток для потребителя, который освободит место
    std::thread consumer([&queue]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        int value{0};
        queue.dequeue(value);
        EXPECT_EQ(value, 0);
    });

    // Добавляем элемент (должен ждать, пока consumer освободит место)
    queue.enqueue(100);

    consumer.join();

    EXPECT_EQ(queue.size(), Queue::CAPACITY);
}

// ============================================================================
// Тесты для пустой очереди
// ============================================================================

TEST(TestMPSCRingQueue, EmptyQueueTryDequeue) {
    Queue<int, 16> queue;

    // Попытка извлечь из пустой очереди
    int value{0};
    EXPECT_FALSE(queue.tryDequeue(value));
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
}

// ============================================================================
// Тесты для различных типов данных
// ============================================================================

TEST(TestMPSCRingQueue, DifferentTypes) {
    // Тест с int
    {
        Queue<int, 16> queue;
        queue.enqueue(42);
        int value{0};
        queue.dequeue(value);
        EXPECT_EQ(value, 42);
    }

    // Тест с double
    {
        Queue<double, 16> queue;
        queue.enqueue(3.14);
        double value{0.0};
        queue.dequeue(value);
        EXPECT_DOUBLE_EQ(value, 3.14);
    }

    // Тест с bool
    {
        Queue<bool, 16> queue;
        queue.enqueue(true);
        bool value{false};
        queue.dequeue(value);
        EXPECT_TRUE(value);
    }

    // Тест с char
    {
        Queue<char, 16> queue;
        queue.enqueue('A');
        char value{0};
        queue.dequeue(value);
        EXPECT_EQ(value, 'A');
    }
}

// ============================================================================
// Тесты для пользовательских типов
// ============================================================================

struct TestStruct {
    int a;
    double b;
    char c;

    TestStruct() : a(0), b(0.0), c('\0') {}
    TestStruct(int a_, double b_, char c_) : a(a_), b(b_), c(c_) {}

    bool operator==(const TestStruct& other) const {
        return a == other.a && b == other.b && c == other.c;
    }
};

TEST(TestMPSCRingQueue, CustomType) {
    using Queue = Queue<TestStruct, 32>;
    Queue queue;

    TestStruct input(10, 3.14, 'X');

    // Добавляем
    queue.enqueue(input);
    EXPECT_EQ(queue.size(), 1);

    // Извлекаем
    TestStruct output;
    queue.dequeue(output);

    // Проверяем
    EXPECT_EQ(output, input);
    EXPECT_TRUE(queue.empty());
}

TEST(TestMPSCRingQueue, MultipleCustomTypes) {
    using Queue = Queue<TestStruct, 64>;
    Queue queue;

    std::vector<TestStruct> data;
    for (int i = 0; i < 20; ++i) {
        data.emplace_back(i, i * 1.5, static_cast<char>('A' + i % 26));
    }

    // Добавляем все
    for (const auto& item : data) {
        queue.enqueue(item);
    }

    EXPECT_EQ(queue.size(), data.size());

    // Извлекаем все и проверяем порядок
    for (const auto& expected : data) {
        TestStruct actual;
        queue.dequeue(actual);
        EXPECT_EQ(actual, expected);
    }

    EXPECT_TRUE(queue.empty());
}

// ============================================================================
// Тесты для метода tryDequeue с reference
// ============================================================================

TEST(TestMPSCRingQueue, TryDequeueReference) {
    Queue<int, 16> queue;

    // Пустая очередь
    int value{0};
    EXPECT_FALSE(queue.tryDequeue(value));
    EXPECT_EQ(value, 0); // Значение не должно измениться

    // Добавляем элемент
    queue.enqueue(42);

    // Извлекаем
    EXPECT_TRUE(queue.tryDequeue(value));
    EXPECT_EQ(value, 42);
    EXPECT_TRUE(queue.empty());
}

// ============================================================================
// Тесты для метода enqueue и dequeue с reference
// ============================================================================

TEST(TestMPSCRingQueue, EnqueueDequeueReference) {
    Queue<int, 16> queue;

    int value{0};
    queue.enqueue(42);
    queue.dequeue(value);
    EXPECT_EQ(value, 42);
    EXPECT_TRUE(queue.empty());
}

// ============================================================================
// Тесты с большим количеством операций (wrap-around)
// ============================================================================

TEST(TestMPSCRingQueue, WrapAround) {
    using Queue = Queue<int, 4>;
    Queue queue;

    // Выполняем много циклов добавления/извлечения
    for (int round = 0; round < 10; ++round) {
        for (int i = 0; i < 4; ++i) {
            EXPECT_TRUE(queue.tryEnqueue(i + round * 10));
            EXPECT_EQ(queue.size(), i + 1);
        }

        EXPECT_EQ(queue.size(), 4);

        for (int i = 0; i < 4; ++i) {
            int value{0};
            EXPECT_TRUE(queue.tryDequeue(value));
            EXPECT_EQ(value, i + round * 10);
            EXPECT_EQ(queue.size(), 3 - i);
        }

        EXPECT_TRUE(queue.empty());
    }
}

// ============================================================================
// Тесты для проверки, что конструкторы и деструкторы работают
// ============================================================================
struct TestStructure {
    bool& constructed;
    bool& destructed;

    TestStructure(bool& _constructed, bool& _destructed):
        constructed(_constructed),
        destructed(_destructed)
    {
        constructed = true;
    }

    ~TestStructure() {
        destructed = true;
    }
};
TEST(TestMPSCRingQueue, ConstructorDestructor) {
    bool constructed{false};
    bool destructed{false};
    {
        Queue<TestStructure, 16> queue;

        // Добавляем в очередь (копирование)
        queue.emplace(constructed, destructed);
        EXPECT_TRUE(constructed);
    }

    EXPECT_TRUE(destructed);
}
