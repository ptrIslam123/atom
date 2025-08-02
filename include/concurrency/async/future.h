#ifndef ATOM_ASYNC_STATIC_FUTURE_H
#define ATOM_ASYNC_STATIC_FUTURE_H

#include "include/platform_api/posix_api/futex.h"
#include "include/memory/allocators/default_allocator.h"
#include "include/utils/assertion.h"

#include <chrono>
#include <atomic>
#include <array>
#include <type_traits>
#include <exception>
#include <cstdint>

#define STATE_NONE 0
#define STATE_UNBIND 1
#define STATE_BIND 2
#define STATE_IN_PROCESS_SETTING 3
#define STATE_SET 4

namespace atom::async {

template<typename T>
class Future;
template<typename T>
class DynamicFuture;
template<typename T>
class Promise;

template<typename T>
void Bind(Future<T>& f, Promise<T>& p);
template<typename T>
void Unbind(Future<T>& f, Promise<T>& p);

template<typename T>
class Promise final {
public:
    explicit Promise() = default;
    Promise(const Promise& other) noexcept = default;
    Promise(Promise&& other) noexcept = default;
    Promise& operator=(const Promise& other) noexcept = default;
    Promise& operator=(Promise&& other) noexcept = default;
    ~Promise() {

    }

    template<typename R>
    void set(R&& value) {

    }

    template<typename ... Arg>
    void emplace(Arg&& ... arg) {

    }

    bool isCanceled() const noexcept {

    }

private:
    friend void Bind<T>(Future<T>&, Promise<T>&);
    friend void Unbind<T>(Future<T>&, Promise<T>&);

    std::atomic<bool> m_isCanceled{false};
    std::atomic<std::uint32_t>* m_state{nullptr};
    std::span<std::byte> m_storage{};
};

template<typename T>
class Future final {
public:
    explicit Future() = default;
    Future(const Future& other) noexcept = delete;
    Future(Future&& other) noexcept = delete;
    Future& operator=(const Future& other) noexcept = delete;
    Future& operator=(Future&& other) noexcept = delete;
    ~Future() {

    }

    template<typename Rep, typename Period>
    bool waitFor(const std::chrono::duration<Rep, Period>& timeout) {

    }


    template<typename Rep, typename Period>
    bool waitUntil(const std::chrono::duration<Rep, Period>& timeout) {

    }

    void wait() {

    }

    void cancel() {

    }

    bool isReady() const noexcept {

    }

    T& get() {
        ASSERTION(isReady(), std::runtime_error, "Attept to get is not ready value yet")
        return *reinterpret_cast<T*>(m_data.data());
    }

    const T& get() const {
        ASSERTION(isReady(), std::runtime_error, "Attept to get is not ready value yet")
        return *reinterpret_cast<const T*>(m_data.data());
    }

private:
    friend void Bind<T>(Future<T>&, Promise<T>&);
    friend void Unbind<T>(Future<T>&, Promise<T>&);

    std::atomic<bool>* m_isCanceled{nullptr};
    std::atomic<std::uint32_t> m_state;
    std::array<std::byte, sizeof(T)> m_data;
};

template<typename T>
void Bind(Future<T>& f, Promise<T>& p) {
    f.m_isCanceled = &p.m_isCanceled;

    p.m_state = &f.m_state;
    p.m_storage = f.m_data;
}

template<typename T>
void Unbind(Future<T>& f, Promise<T>& p) {

}

} //! namespace atom::async

#endif //! ATOM_ASYNC_STATIC_FUTURE_H
