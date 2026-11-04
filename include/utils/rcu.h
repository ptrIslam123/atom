#pragma once

#include <urcu-bp.h>
#include <atomic>
#include <memory>
#include <mutex>
#include <utility>

namespace atom::containers::lock_free::mpsc {

/**
 * @brief Универсальная RAII-обёртка над liburcu (flavor bp) для защиты
 *        указателя на данные типа T.
 *
 * Читатели получают lock-free доступ через Snapshot.
 * Писатели публикуют новую версию через store() и ждут grace period.
 *
 * @tparam T Тип защищаемых данных. Должен быть копируемым или перемещаемым.
 */
template<typename T>
class Rcu final {
public:
    /**
     * @brief RAII-объект для чтения. Держит read-side critical section.
     * Пока Snapshot жив, данные, на которые он указывает, не будут освобождены.
     */
    class Snapshot final {
    public:
        explicit Snapshot(T* ptr) noexcept
            : m_ptr(ptr)
        {
            rcu_read_lock();  // начало критической секции
        }

        ~Snapshot() {
            rcu_read_unlock(); // конец критической секции
        }

        Snapshot(const Snapshot&) = delete;
        Snapshot& operator=(const Snapshot&) = delete;
        Snapshot(Snapshot&&) = delete;
        Snapshot& operator=(Snapshot&&) = delete;

        const T* operator->() const noexcept { return m_ptr; }
        const T& operator*() const noexcept { return *m_ptr; }
        [[nodiscard]] const T* get() const noexcept { return m_ptr; }

    private:
        T* m_ptr;
    };

    /**
     * @brief Создаёт RCU-хранилище с начальным значением.
     * @param initial Начальные данные (будут перемещены внутрь).
     */
    explicit Rcu(T initial)
        : m_ptr(new T(std::move(initial)))
    {}

    ~Rcu() {
        // При уничтожении нужно дождаться всех читателей и удалить данные
        synchronize_rcu();
        delete m_ptr.load(std::memory_order_relaxed);
    }

    // Запрещаем копирование и перемещение, чтобы гарантировать
    // время жизни m_ptr (liburcu требует, чтобы данные жили
    // дольше всех потоков, которые их читают).
    Rcu(const Rcu&) = delete;
    Rcu(Rcu&&) = delete;
    Rcu& operator=(const Rcu&) = delete;
    Rcu& operator=(Rcu&&) = delete;

    /**
     * @brief Получает lock-free снапшот текущих данных.
     *
     * Вызов не блокирует другие потоки. Возвращённый Snapshot
     * гарантирует, что данные не будут изменены или освобождены,
     * пока он существует.
     */
    [[nodiscard]] Snapshot load() const {
        rcu_read_lock();
        T* ptr = rcu_dereference(m_ptr);
        rcu_read_unlock();
        return Snapshot(ptr);
    }

    /**
     * @brief Публикует новую версию данных.
     *
     * Старая версия будет освобождена после завершения grace period
     * (когда все читатели, видевшие старую версию, закончат).
     *
     * @param new_value Новые данные (будут перемещены внутрь).
     */
    void store(T new_value) {
        std::lock_guard<std::mutex> lock(m_mutex);

        T* old = m_ptr.load(std::memory_order_relaxed);
        T* next = new T(std::move(new_value));

        rcu_assign_pointer(m_ptr, next);
        synchronize_rcu();
        delete old;
    }

    /**
     * @brief Публикует новую версию через функцию-модификатор.
     *
     * Это удобно, когда нужно создать копию текущих данных,
     * изменить её и опубликовать.
     *
     * @param modifier Функция, принимающая копию текущих данных
     *                 и возвращающая новую версию.
     */
    template <typename Modifier>
    void update(Modifier&& modifier) {
        auto current = load();
        T new_value = *current; // Копируем
        store(std::forward<Modifier>(modifier)(std::move(new_value)));
    }

private:
    // liburcu работает с обычными указателями через макросы,
    // но для атомарности храним в std::atomic.
    mutable std::atomic<T*> m_ptr;
    mutable std::mutex m_mutex;
};

} // namespace atom::containers::lock_free::mpsc