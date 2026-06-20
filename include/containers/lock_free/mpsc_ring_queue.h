#pragma once

#include "utils/compiler_attr.h"

#include <thread>
#include <atomic>
#include <array>

#include <cstring>
#include <cstddef>
#include <cassert>

namespace atom::containers::lock_free::mpsc {

template<typename T, std::size_t N>
class RingQueue final {
public:
    static_assert(N > 0 && (N & (N - 1)) == 0, "Capacity must be power of two");
    static_assert (N > 1, "Capacity must be > 1");
    static constexpr auto CAPACITY{N};

    using ValueType = T;
    using SizeType = std::size_t;

    RingQueue(const RingQueue&) = delete;
    RingQueue(RingQueue&&) = delete;
    RingQueue& operator=(const RingQueue&) = delete;
    RingQueue& operator=(RingQueue&&) = delete;

    RingQueue() noexcept = default;
    ~RingQueue() noexcept {
        for (auto i = 0; i < size(); ++i) {
            get(i).~T();
        }
        m_readerIndex.store(0);
        m_writerIndex.store(0);
        m_commitIndex.store(0);
    }

    template<typename ... Arg>
    FORCE_INLINE void emplace(Arg&& ... arg) {
        [[maybe_unused]] SizeType available{};
        return emplace(available, std::forward<Arg>(arg) ... );
    }

    template<typename ... Arg>
    void emplace(SizeType& available, Arg&& ... arg) {
        for (;;) {
            if LIKELY_EXPR(tryEmplace(available, std::forward<Arg>(arg) ... )) {
                break;
            }
            std::this_thread::yield();
        }
    }

    FORCE_INLINE void enqueue(const T& data) noexcept {
        [[maybe_unused]] SizeType available{};
        return emplace(available, data);
    }

    FORCE_INLINE void enqueue(SizeType& available, const T& data) noexcept {
        return emplace(available, data);
    }

    FORCE_INLINE bool tryEnqueue(const T& data) noexcept {
        return tryEmplace(data);
    }

    bool tryEnqueue(SizeType& available, const T& data) noexcept {
        return tryEmplace(available, data);
    }

    template<typename ... Arg>
    bool tryEmplace(Arg&& ... arg) noexcept {
        [[maybe_unused]] SizeType available{};
        return tryEmplace(available, std::forward<Arg>(arg) ... );
    }

    template<typename ... Arg>
    bool tryEmplace(SizeType& available, Arg&& ... arg) noexcept {
        const auto readerIndex{m_readerIndex.load(/*std::memory_order_acquire*/)};
        auto writerIndex{m_writerIndex.load(/*std::memory_order_relaxed*/)};
        auto newWriterIndex{writerIndex + 1};

        // 1) CAS-цикл для резервирования уникального слота
        for (;;) {
            available = canEnqueue(readerIndex, writerIndex);
            if UNLIKELY_EXPR(available == 0) {
                return false; // Очередь полна
            }

            // Пытаемся атомарно зарезервировать слот.
            // Только один писатель успешно выполнит CAS и получит уникальный newWriterIndex.
            // Остальные будут повторять попытку с новым значением writerIndex.
            if (m_writerIndex.compare_exchange_weak(writerIndex, newWriterIndex
                                                    /*,std::memory_order_release*/
                                                    /*std::memory_order_relaxed*/)) {
                break; // Успешно зарезервировали слот newWriterIndex
            }
            // CAS провалился - кто-то другой изменил writerIndex.
            // Обновляем newWriterIndex и пробуем снова.
            newWriterIndex = writerIndex + 1;
        }

        // 2) Запись данных в зарезервированный слот
        // Вычисляем физическую позицию в кольцевом буфере через маску.
        // Важно: данные записываются ДО коммита, но читатель их не увидит,
        // потому что читатель смотрит на commitIndex, который еще не продвинут.
        construct(writerIndex & MASK, std::forward<Arg>(arg) ... );

        // 3) Коммит (финализация) записи
        // Теперь нужно сделать данные видимыми для читателя.
        // Ключевая идея: коммит происходит строго в порядке возрастания индексов.
        // Это гарантирует, что читатель всегда видит консистентное состояние.
        for (;;) {
            const auto commitIndex = m_commitIndex.load(/*std::memory_order_acquire*/);
            assert(commitIndex <= newWriterIndex);
            if (commitIndex >= newWriterIndex) {
                return true;  // Уже закоммичено
            }

            // Случай 2: Мы - следующие в очереди на коммит.
            // Это единственный случай, когда мы можем безопасно сделать store.
            if (commitIndex + 1 == newWriterIndex) {
                // ПОЧЕМУ STORE ВМЕСТО CAS:
                // Индексы уникальны (гарантируется CAS на этапе 1). Условие commitIndex + 1 == newWriterIndex
                // может быть истинно только для одного писателя. Поэтому между проверкой и store никто
                // другой не может изменить commitIndex. CAS избыточен.
                m_commitIndex.store(newWriterIndex/*, std::memory_order_release*/);
                break;
            }

            // 3) Мы не следующие в очереди.
            // Уступаем CPU другим потокам, чтобы не жечь циклы впустую.
            std::this_thread::yield();
        }
        return true;
    }

    FORCE_INLINE void dequeue(T& data) noexcept {
        [[maybe_unused]] SizeType available{};
        return dequeue(data, available);
    }

    void dequeue(T& data, SizeType& available) noexcept {
        for (;;) {
            if LIKELY_EXPR(tryDequeue(data, available)) {
                break;
            }
            std::this_thread::yield();
        }
    }

    FORCE_INLINE bool tryDequeue(T& data) noexcept {
        [[maybe_unused]] SizeType available{};
        return tryDequeue(data, available);
    }

    bool tryDequeue(T& data, SizeType& available) noexcept {
        // 1) БЫСТРЫЙ ПУТЬ: используем кэшированный readerIndex
        auto commitIndex = m_commitIndex.load(std::memory_order_acquire);
        available = canDequeue(m_cachedReaderIndex, commitIndex);
        if (available > 0) {
            data = get(m_cachedReaderIndex & MASK);
            ++m_cachedReaderIndex;
            m_readerIndex.store(m_cachedReaderIndex, std::memory_order_release);
            return true;
        }

        // 2) МЕДЛЕННЫЙ ПУТЬ: синхронизируем кэш
        // Загружаем актуальный readerIndex
        const auto readerIndex = m_readerIndex.load(std::memory_order_acquire);
        available = canDequeue(readerIndex, commitIndex);
        if (available > 0) {
            m_cachedReaderIndex = readerIndex;
            data = get(m_cachedReaderIndex & MASK);
            ++m_cachedReaderIndex;
            m_readerIndex.store(m_cachedReaderIndex, std::memory_order_release);
            return true;
        }

        // Данных нет, обновляем кэш
        m_cachedReaderIndex = readerIndex;
        return false;
    }

    FORCE_INLINE SizeType size() const noexcept {
        const auto commitIdx{m_commitIndex.load(std::memory_order_acquire)};
        const auto readerIndex{m_readerIndex.load(std::memory_order_acquire)};
        // Нельзя тут обновлять @m_cachedReaderIndex, так как size может быть вызван из другого
        // потока(не писатлеля), что противоречит задумке.
        return commitIdx - readerIndex;
    }

    FORCE_INLINE constexpr SizeType capacity() const noexcept {
        return CAPACITY;
    }

    FORCE_INLINE bool empty() const noexcept {
        return size() == 0;
    }

private:
    template<typename ... Arg>
    FORCE_INLINE void construct(SizeType index, Arg&& ... arg) {
        new(&get(index)) T{std::forward<Arg>(arg) ... };
    }

    FORCE_INLINE T& get(SizeType index) {
        return *reinterpret_cast<T*>(m_buffer.data() + index * sizeof(T));
    }

    FORCE_INLINE const T& get(SizeType index) const {
        return *reinterpret_cast<const T*>(m_buffer.data() + index * sizeof(T));
    }

    FORCE_INLINE constexpr SizeType canEnqueue(SizeType readerIndex, SizeType writerIndex) const noexcept {
        return CAPACITY - (writerIndex - readerIndex);
    }

    FORCE_INLINE constexpr SizeType canDequeue(SizeType readerIndex, SizeType commitIndex) const noexcept {
        return commitIndex - readerIndex;
    }

    static constexpr auto CACHELINE_SIZE{64};
    static constexpr auto MASK{N - 1};

    SizeType m_cachedReaderIndex{0};

    alignas(CACHELINE_SIZE) std::atomic<SizeType> m_commitIndex{0};
    alignas(CACHELINE_SIZE) std::atomic<SizeType> m_writerIndex{0};
    alignas(CACHELINE_SIZE) std::atomic<SizeType> m_readerIndex{0};
    alignas(CACHELINE_SIZE) std::array<std::byte, N * sizeof(T)> m_buffer;
};

} //! namespace atom::containers::lock_free::mpsc
