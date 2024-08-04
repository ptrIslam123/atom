#ifndef FUNCTOR_H
#define FUNCTOR_H

#include "include/concurrency/owner.h"
#include "include/utils/lazy.h"

#include <type_traits>
#include <array>
#include <memory>
#include <new>

namespace atom::utils {

namespace statical {

template<int N, typename T>
class Functor;

template<int N, typename R, typename ... Args>
class Functor<N, R(Args ...)> final {
public:
    using ReturnType = R;
    using CallbackType = R(Args ...);

    template<typename F>
    explicit Functor(F&& f);
    ~Functor();

    R operator()(Args ...);

private:
    struct BaseFunctorHolder {
        virtual R call(Args ... args) = 0;
        virtual ~BaseFunctorHolder() = default;
    };

    template<typename F>
    struct RealFunctorHolder final : public BaseFunctorHolder {
        RealFunctorHolder(F&& fucntor): m_functor(std::forward<F>(fucntor)) {}
        ~RealFunctorHolder() = default;

        R call(Args ... args) override {
            return m_functor.operator()(args ...);
        }

        F m_functor;
    };

    static constexpr auto STORAGE_SIZE = N + (2 * sizeof(void*));
    using FunctorHolderStorageType = std::array<std::uint8_t, STORAGE_SIZE>;

    template<typename T>
    void constructFunctorHolder(T&& object);
    void destructFunctorHolder();
    BaseFunctorHolder* castFunctorHolderStorage();

    FunctorHolderStorageType m_functorHolderStorage;
};

} //! namespace statical

namespace dynamical {

template<typename T>
class Functor;

template<typename R, typename ... Args>
class Functor<R(Args ...)> final {
public:
    template<typename F>
    explicit Functor(F&& f);

    R operator()(Args ...);

private:
    struct BaseFunctortHolder {
        virtual R call(Args ... args) = 0;
        virtual ~BaseFunctortHolder() = default;
    };

    template<typename F>
    struct RealFunctorHolder final : public BaseFunctortHolder {
        RealFunctorHolder(F&& functor): m_functor(std::forward<F>(functor)) {}
        R call(Args ... args) {
            return m_functor.operator()(args ...);
        }

        F m_functor;
    };

    std::unique_ptr<BaseFunctortHolder> m_functorHolder;
};

} //! namespace dynamical

namespace statical {

template<int N, typename R, typename ... Args>
template<typename F>
Functor<N, R(Args ...)>::Functor(F&& f):
m_functorHolderStorage()
{
    static_assert(std::is_invocable<R(Args ...), Args...>::value && "The F type must be invocable(functor)");
    static_assert(STORAGE_SIZE >= sizeof(RealFunctorHolder<F>));
    static_assert(sizeof(F) <= N && "The Functor storage size(N - parametr) must be equal or/and bigger than fucntor");
    std::memset(m_functorHolderStorage.data(), 0, m_functorHolderStorage.size());
    constructFunctorHolder<F>(std::forward<F>(f));
}

template<int N, typename R, typename ... Args>
Functor<N, R(Args ...)>::~Functor()
{
    destructFunctorHolder();
}

template<int N, typename R, typename ... Args>
R Functor<N, R(Args ...)>::operator()(Args ... args)
{
    return castFunctorHolderStorage()->call(args ...);
}

template<int N, typename R, typename ... Args>
template<typename T>
void Functor<N, R(Args ...)>::constructFunctorHolder(T&& object)
{
    new (m_functorHolderStorage.data()) RealFunctorHolder<T>(std::forward<T>(object));
}

template<int N, typename R, typename ... Args>
void Functor<N, R(Args ...)>::destructFunctorHolder()
{
    castFunctorHolderStorage()->~BaseFunctorHolder();
}

template<int N, typename R, typename ... Args>
typename Functor<N, R(Args ...)>::BaseFunctorHolder* Functor<N, R(Args ...)>::castFunctorHolderStorage()
{
    return reinterpret_cast<BaseFunctorHolder*>(m_functorHolderStorage.data());
}

} //! namespace statical

namespace dynamical {

template<typename R, typename ... Args>
template<typename F>
Functor<R(Args ...)>::Functor(F&& f):
m_functorHolder(std::make_unique<RealFunctorHolder<F>>(std::forward<F>(f)))
{
    static_assert(std::is_invocable<R(Args ...), Args...>::value && "The F type must be invocable(functor)");
}

template<typename R, typename ... Args>
R Functor<R(Args ...)>::operator()(Args ... args)
{
    return m_functorHolder->call(args ...);
}

} //! namespace dynamical

} //! namespace atom::utils

#endif //! FUNCTOR
