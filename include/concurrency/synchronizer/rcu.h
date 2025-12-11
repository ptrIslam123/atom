#ifndef ATOM_SYNCHRONIZER_RCU_H
#define ATOM_SYNCHRONIZER_RCU_H

#include <atomic>
#include <thread>
#include <vector>
#include <memory>
#include <iostream>
#include <unistd.h>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <memory>

namespace atom::concurrence::synchronizer {

// /* ---------- публичный класс ---------- */
// template <typename T>
// class RCU {
//     using epoch_t = uint64_t;
//     static constexpr epoch_t EPOCH_BIT = 1ull << 63;

//     /* ---------- глобальная шина ---------- */
//     static std::atomic<epoch_t> g_epoch;
//     static std::mutex           g_gpMutex;   // для смены эпохи
//     static std::mutex           g_regMutex;

//     /* ---------- per-thread контекст ---------- */
//     struct ThreadCtx {
//         std::atomic<epoch_t> epoch{~epoch_t(0)};
//         ThreadCtx*           next{nullptr};
//     };
//     ThreadCtx* g_threadList{nullptr};

//     /* ---------- регистрация ---------- */
//     void reg_thread(ThreadCtx* ctx) {
//         std::lock_guard lk(g_regMutex);
//         ctx->next = g_threadList;
//         g_threadList = ctx;
//     }

//     void unreg_thread(ThreadCtx* ctx) {
//         std::lock_guard lk(g_regMutex);
//         ThreadCtx** p = &g_threadList;
//         while (*p) {
//             if (*p == ctx) { *p = ctx->next; break; }
//             p = &(*p)->next;
//         }
//     }

//     /* ---------- grace-period ---------- */
//     void synchronize() {
//         std::unique_lock lk(g_gpMutex);
//         epoch_t old = g_epoch.fetch_xor(EPOCH_BIT, std::memory_order_seq_cst);
//         while (true) {
//             bool done = true;
//             {
//                 std::lock_guard lreg(g_regMutex);
//                 for (ThreadCtx* p = g_threadList; p; p = p->next) {
//                     epoch_t e = p->epoch.load(std::memory_order_acquire);
//                     if ((e & EPOCH_BIT) == (old & EPOCH_BIT)) { done = false; break; }
//                 }
//             }
//             if (done) return;
//             lk.unlock();
//             std::this_thread::sleep_for(std::chrono::milliseconds(10));
//             lk.lock();
//         }
//     }

//     std::atomic<T*> m_ptr{nullptr};

//     /* ---------- RAII-гард для читателя ---------- */
//     class ReadGuard {
//         T* m_p;
//     public:
//         explicit ReadGuard(T* p) noexcept : m_p(p) {}
//         ~ReadGuard() = default;
//         T& operator*()  const noexcept { return *m_p; }
//         T* operator->() const noexcept { return m_p; }
//         T* get()        const noexcept { return m_p; }
//     };

//     /* ---------- per-thread контекст + регистрация ---------- */
//     struct MyCtx : ThreadCtx {
//         static MyCtx& me(RCU* rcu) {
//             static thread_local MyCtx ctx;
//             static thread_local bool  inited = false;
//             if (!inited) {
//                 rcu->reg_thread(&ctx);
//                 inited = true;
//             }
//             return ctx;
//         }
//         ~MyCtx() { unreg_thread(this); }
//     };

// public:
//     RCU()  = default;
//     ~RCU() { reset(nullptr); }   // удалит текущее при разборе

//     /* чтение: вернуть RAII-обёртку */
//     [[nodiscard]] ReadGuard read() {
//         MyCtx& ctx = MyCtx::me(this);
//         ctx.epoch.store(g_epoch.load(std::memory_order_acquire),
//                         std::memory_order_relaxed);
//         T* p = std::atomic_load_explicit(&m_ptr, std::memory_order_consume);
//         return ReadGuard(p);
//     }

//     /* писатель: атомарная подмена */
//     void reset(T* np) noexcept {
//         std::atomic_store_explicit(&m_ptr, np, std::memory_order_release);
//     }

//     /* писатель: удалить после grace-period */
//     void retire(T* old) {
//         if (!old) return;
//         synchronize();
//         delete old;
//     }
// };













// ==================== Класс RCU-защищенных данных ====================
template<typename T>
class RCUProtected {
private:
    std::atomic<T*> data_ptr;

public:
    // Конструктор
    explicit RCUProtected(T* initial_data = nullptr)
        : data_ptr(initial_data) {}

    // Деструктор
    ~RCUProtected() {
        delete data_ptr.load(std::memory_order_relaxed);
    }

    // ========== API для читателей ==========

    // Получить указатель для чтения (не блокирующий)
    const T* read_begin() const {
        // Читаем текущий указатель с барьером памяти для чтения
        return data_ptr.load(std::memory_order_acquire);
    }

    // Завершить чтение (в реальной RCU здесь была бы синхронизация)
    void read_end() const {
        // В упрощенной версии - пустая операция
        // В реальной RCU: rcu_read_unlock()
    }

    // ========== API для писателей ==========

    // Обновление данных (модифицирующая операция)
    template<typename Updater>
    void update(Updater updater) {
        // 1. Создаем новую копию данных
        T* old_ptr = data_ptr.load(std::memory_order_relaxed);
        T* new_ptr = old_ptr ? new T(*old_ptr) : new T();

        // 2. Модифицируем копию
        updater(new_ptr);

        // 3. Атомарно заменяем указатель (публикация новых данных)
        T* previous = data_ptr.exchange(new_ptr, std::memory_order_acq_rel);

        // 4. Ждем, пока все читатели завершат работу со старыми данными
        synchronize_rcu();

        // 5. Освобождаем старые данные
        delete previous;
    }

    // ========== Синхронизация ==========

    static void synchronize_rcu() {
        // В реальной реализации: ожидание завершения всех критических секций чтения
        // В упрощенной версии - просто задержка
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
};

} //! namespace atom::concurrence::synchronizer

#endif //! ATOM_SYNCHRONIZER_RCU_H
