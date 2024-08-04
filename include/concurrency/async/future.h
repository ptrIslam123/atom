#ifndef ASYNC_FUTURE_H
#define ASYNC_FUTURE_H

#include "include/utils/lazy.h"

#include <condition_variable>
#include <mutex>
#include <optional>
#include <chrono>

namespace atom::concurrency::async {

/*
Пришел к выводу что если хранить shared state не на куче, а взять ownership одному из сущностей(futrue && promise)
то у нас нету адекватного способа гарнтировать корректность управления памятю(что я имею ввиду):
Тот кто будет взадеть shared state должен дождаться пока дургая сторона не оповестит о том что дальнейшие попытки обращения к shared state
ломают инварант механизма управления памятю. Если этот кейс acceptable, то окей. В таком случае надо рассмотреть возмождность
поддержания механизма пулинга futures(по аналогии с rust tokio) иначе подобный механизм мало применим.
Одно из потенциально оч продуктиных использовании подобных local::Future<T> есть комбинация с io::Poll.
Я могу сделать такой механизм:

void io::sheduler::SheduleReading
(
    utils::LazyConstructed<async::local::Future<BufferType>>& future,
    const SocketType socket,
    const async::ThreadPoll::IdType id
) {
    global::Manager::GetInstance()->getIOPoll().onReadEvent(socket, [&future, id](const SocketType socket) {
        global::Manager::GetInstance()->GetThreadPoll(id).pushTask(future, [socket](async::local::Promise<BufferType>& promise) {
            // read from native socket
            BufferType buffer;
            const auto readBytes = utils::net::low_level::Read(socket, std::span<BufferType::ValueType>{ buffer.data(), buffer.size() });
            if (readBytes == 0) {
                global::Manager::GetInsatnce()->getIOPoll().removeCallback(socket);
            } else {
                promise.setResult(std::move(buffer));
            }
        });
    }, [&future, id](const SocketType socket) {
        (void)global::Manager::GetInstance()->GetThreadPoll(id).cancelTask(future));
        switch (future.waitFoCancelation()) {
            case async::local::Future<BufferType>::State::Ready:
            case async::local::Future<BufferType>::State::Canceled: {
                break;
            }
            default: PANIC("Bad future")
        }
        utils::net::low_level::CloseSocket(socket);
    });
}

void io::sheduler::CancelReading
(
    utils::LazyConstructed<async::local::Future<BufferType>>& future,
    const SocketType socket,
    const async::ThreadPoll::IdType id
) {

}

Usage:

const SocketType socket = ...;

utils::LazyConstructed<async::local::Future<BufferType>> futureRequestBuffer;
io::sheduler::SheduleReading(futureRequestBuffer, socket, async::ThreadPool::IO);
assert(futureRequestBuffer.wasConstructed());

///
1) you can wait completing of read operation:
if (futureRequestBuffer->wait() == async::local::Future<BufferType>::Ready) {
    const auto requestBuffer = futureRequestBuffer->getResult();
    assert(requestBuffer.has_value());

    processRequest(std::move(requetsBuffer));
    futureRequestBuffer.destroy();
}

2) you can wait completing for timeout
if (futureRequestBuffer->waitFor(timeout) == async::local::Future<BufferType>::Ready) {
    const auto requestBuffer = futureRequestBuffer->getResult();
    assert(requestBuffer.has_value());

    processRequest(std::move(requetsBuffer));
    futureRequestBuffer.destroy();
} else {
    io::sheduler::CancelReading(futureRequestBuffer, socket, async::ThreadPool::IO);
}

*/

template<typename T>
class Future;

template<typename T>
class Promise;

template<typename T>
void Bind(utils::LazyConstructed<Future<T>>& future, utils::LazyConstructed<Promise<T>>& promise);

template<typename T>
class Future final {
public:
    enum class Status {
        Ready,
        TimeoutExpired,
        Canceled,
    };

    std::optional<T> getResult();
    void waitForCancellation();
    template<typename Rep, typename Period>
    Status waitFor(const std::chrono::duration<Rep,Period>& timeout);
    Status wait();

private:
    friend void Bind(utils::LazyConstructed<Future<T>>& future, utils::LazyConstructed<Promise<T>>& promise);
    explicit Future();

    enum class State {
        WaitResult,
        Completed,
        RequestCancelation,
        Canceled,
    };
};

template<typename T>
class Promise final {
public:
    Promise(const Promise& ) = delete;
    Promise& operator=(const Promise& ) = delete;

    Promise(Promise&& other) noexcept;
    Promise& operator=(Promise&& other) noexcept;

    void setResult(T&& value);
    void setResult(const T& value);
    bool wasCanceled();

private:
    friend void Bind(utils::LazyConstructed<Future<T>>& future, utils::LazyConstructed<Promise<T>>& promise);
};

template<typename T>
Future<T>::Future()
{}

template<typename T>
std::optional<T> Future<T>::getResult()
{

}

template<typename T>
void Future<T>::waitForCancellation()
{}

template<typename T>
template<typename Rep, typename Period>
typename Future<T>::Status Future<T>::waitFor(const std::chrono::duration<Rep,Period>& timeout)
{}

template<typename T>
typename Future<T>::Status Future<T>::wait()
{}


template<typename T>
void Bind(utils::LazyConstructed<Future<T>>& future, utils::LazyConstructed<Promise<T>>& promise) {

}

} //! namespace atom::concurrency::async

#endif //! ASYNC_FUTURE_H
