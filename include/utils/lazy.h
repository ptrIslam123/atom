#ifndef LAZY_H
#define LAZY_H

#include "include/utils/assertion.h"

#include <array>
#include <string>
#include <string_view>
#include <exception>
#include <typeinfo>
#include <type_traits>
#include <new>
#include <memory>
#include <cstring>
#include <cstdint>
#include <cassert>

namespace ciengine::utils {

template<typename T>
class BadLazyConstructed final : public std::exception {
public:
    explicit BadLazyConstructed(std::string_view msg);
    virtual const char* what() const noexcept;

private:
    std::string m_msg;
};

/**
 * @brief This class is used for lazy initialization. This class implement lazy initialization idiom.
 * @details This class contains buffer, enough size, to store object of type T.
 * The size of this class is always sizeof(T) + 1 byte(for binary flag)
 * @warning  This class really call constructor of the object type T then user call Lazy& operator=(T) or Lazy(T).
 * We should not access to internal object of this class before call Lazy& operator=(T) or Lazy(T).
 * Accessing to internal object of this class is Undefined behavior!
 * @warning !This class IS NOT THREAD SAFE.
 */
template<typename T>
class LazyConstructed final {
public:
    using ValueType = T;

    explicit LazyConstructed();
    ~LazyConstructed();

    LazyConstructed(const LazyConstructed& ) = delete;
    LazyConstructed(LazyConstructed&& ) noexcept = delete;
    LazyConstructed& operator=(const LazyConstructed& ) = delete;
    LazyConstructed& operator=(LazyConstructed&& ) noexcept = delete;

    template<typename ... Args>
    void construct(Args&& ... args);
    void destruct();

    T& operator*() & noexcept(false);
    const T& operator*() const & noexcept(false);

    T* operator->() & noexcept(false);
    const T* operator->() const & noexcept(false);

    T* get() & noexcept(false);
    const T* get() const & noexcept(false);

    T intoValue() noexcept(false);

    operator bool() const;
    bool wasConstructed() const;

private:
    T* access();
    const T* access() const;

    std::array<std::uint8_t, sizeof(T)> m_storage;
    bool m_wasConstructed;
};


template<typename T>
LazyConstructed<T>::LazyConstructed():
m_storage(),
m_wasConstructed(false)
{
    std::memset(m_storage.data(), 0, m_storage.size());
}

template<typename T>
LazyConstructed<T>::~LazyConstructed()
{
    destruct();
}

template<typename T>
template<typename ... Args>
void LazyConstructed<T>::construct(Args&& ... args)
{
    if (!wasConstructed()) {
        new(access()) T(std::forward<Args>(args) ...);
        m_wasConstructed = true;
    } else {
        ASSERTION(false, BadLazyConstructed<T>, "Attempt to construct the lazy twise");
    }
}

template<typename T>
void LazyConstructed<T>::destruct()
{
    if (wasConstructed()) {
        access()->~T();
        m_wasConstructed = false;
    }
}

template<typename T>
T& LazyConstructed<T>::operator*() & noexcept(false)
{
    return *get();
}

template<typename T>
const T& LazyConstructed<T>::operator*() const & noexcept(false)
{
    return *get();
}

template<typename T>
T* LazyConstructed<T>::operator->() & noexcept(false)
{
    return get();
}

template<typename T>
const T* LazyConstructed<T>::operator->() const & noexcept(false)
{
    return get();
}

template<typename T>
T* LazyConstructed<T>::get() & noexcept(false)
{
    if (wasConstructed()) {
        return access();
    } else {
        ASSERTION(false, BadLazyConstructed<T>, "Attempt to access to has not constructed yet object")
    }
}

template<typename T>
const T* LazyConstructed<T>::get() const & noexcept(false)
{
    if (wasConstructed()) {
        return access();
    } else {
        ASSERTION(false, BadLazyConstructed<T>, "Attempt to access to has not constructed yet object")
    }
}

template<typename T>
T* LazyConstructed<T>::access()
{
    return reinterpret_cast<T*>(m_storage.data());
}

template<typename T>
const T* LazyConstructed<T>::access() const
{
    return reinterpret_cast<T*>(m_storage.data());
}

template<typename T>
T LazyConstructed<T>::intoValue() noexcept(false)
{
    return std::move(*get());
}

template<typename T>
LazyConstructed<T>::operator bool() const
{
    return wasConstructed();
}

template<typename T>
bool LazyConstructed<T>::wasConstructed() const
{
    return m_wasConstructed;
}


template<typename T>
BadLazyConstructed<T>::BadLazyConstructed(std::string_view msg):
m_msg()
{
    m_msg = std::string(typeid(T()).name()) + "::" + std::string(msg);
}

template<typename T>
const char* BadLazyConstructed<T>::what() const noexcept
{
    return m_msg.data();
}

} //! namespace namespace ciengine::utils


#endif //! LAZY_H
