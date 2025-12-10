#ifndef ATOM_ASYNC_STATIC_FUTURE_H
#define ATOM_ASYNC_STATIC_FUTURE_H

#include "include/platform_api/posix_api/futex.h"
#include "include/memory/allocators/default_allocator.h"
#include "include/utils/assertion.h"

#include <chrono>
#include <atomic>
#include <array>
#include <span>
#include <type_traits>
#include <exception>
#include <cstdint>

namespace atom::async {

//template<typename T>
//class Future;
//template<typename T>
//class DynamicFuture;
//template<typename T>
//class Promise;

//template<typename T>
//void Bind(Future<T>& f, Promise<T>& p);
//template<typename T>
//void Unbind(Future<T>& f, Promise<T>& p);

//enum FutureStatus {
//    Unbind,
//    Bind,
//    InProcessSetting,
//    Set,
//};

//template<typename T>
//class Promise final {
//public:
//    explicit Promise() = default;
//    Promise(const Promise& other) noexcept = delete;
//    Promise(Promise&& other) noexcept = delete;
//    Promise& operator=(const Promise& other) noexcept = delete;
//    Promise& operator=(Promise&& other) noexcept = delete;
//    ~Promise() {}

//    template<typename R>
//    void set(R&& value) {

//    }

//    template<typename ... Arg>
//    void emplace(Arg&& ... arg) {

//    }

//private:
//    friend void Bind<T>(Future<T>&, Promise<T>&);
//    friend void Unbind<T>(Future<T>&, Promise<T>&);
//};

//template<typename T>
//class Future final {
//public:
//    explicit Future() = default;
//    Future(const Future& other) noexcept = default;
//    Future(Future&& other) noexcept = default;
//    Future& operator=(const Future& other) noexcept = default;
//    Future& operator=(Future&& other) noexcept = default;
//    ~Future() {
//    }

//    template<typename Rep, typename Period>
//    bool waitFor(const std::chrono::duration<Rep, Period>& timeout) {

//    }


//    template<typename Rep, typename Period>
//    bool waitUntil(const std::chrono::duration<Rep, Period>& timeout) {

//    }

//    void wait() {
//    }

//    bool isReady() const noexcept {
//        return static_cast<FutureStatus>(m_pStatus->load()) == FutureStatus::Set;
//    }

//    T& get() {

//    }

//    const T& get() const {

//    }

//private:
//    friend void Bind<T>(Future<T>&, Promise<T>&);
//    friend void Unbind<T>(Future<T>&, Promise<T>&);

//    std::atomic<std::uint32_t>* m_pStatus{nullptr};
//    std::array<std::byte, sizeof(T)>* m_pData{nullptr};
//};

//template<typename T>
//void Bind(Future<T>& f, Promise<T>& p) {

//}

//template<typename T>
//void Unbind(Future<T>& f, Promise<T>& p) {

//}

} //! namespace atom::async

#endif //! ATOM_ASYNC_STATIC_FUTURE_H
