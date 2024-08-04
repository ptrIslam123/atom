#include "include/concurrency/owner.h"

#include <array>
#include <memory>
#include <optional>
#include <new>
#include <cstdint>
#include <cstring>

namespace atom {

namespace statical {

template<int N>
class AnyObject final {
public:
    static constexpr auto OBJECT_SIZE = N;

    template<class T>
    explicit AnyObject(T&& object);
    ~AnyObject();

    template<typename T>
    std::optional<concurrency::Ref<T>> getRef() && = delete;

    template<typename T>
    std::optional<concurrency::Ref<T>> getRef() &;

    template<typename T>
    std::optional<const concurrency::Ref<T>> getRef() const &;

private:
    using ObjectHolderStorageType = std::array<std::uint8_t, N>;

    struct BaseObjectHolder {
        virtual ~BaseObjectHolder() = default;
    };

    template<typename T>
    struct RealObjectHolder final : public BaseObjectHolder {
        using ValueType = T;
        using OwnerType = concurrency::Owner<T>;

        RealObjectHolder(T&& object): m_object(std::forward<T>(object)) {}
        concurrency::Owner<T> m_object;
    };

    template<typename T>
    void constructObjectHolder(T&& object);
    void destructObjectHolder();
    BaseObjectHolder* castObjectHolderStorage();

    ObjectHolderStorageType m_objectHolderStorage;
};

} //! namespace statical

namespace dynamical {

class AnyObject final {
public:
    template<class T>
    explicit AnyObject(T&& object);

    template<typename T>
    std::optional<concurrency::Ref<T>> getRef() && = delete;

    template<typename T>
    std::optional<concurrency::Ref<T>> getRef() &;

    template<typename T>
    std::optional<const concurrency::Ref<T>> getRef() const &;

private:
    struct BaseObjectHolder {
        virtual ~BaseObjectHolder() = default;
    };

    template<typename T>
    struct RealObjectHolder final : public BaseObjectHolder {
        using ValueType = T;
        using OwnerType = concurrency::Owner<T>;

        RealObjectHolder(T&& object): m_object(std::forward<T>(object)) {}
        concurrency::Owner<T> m_object;
    };

    std::unique_ptr<BaseObjectHolder> m_objectHolder;
};

} //! namespace dynamical


namespace statical {

template<int N>
template<typename T>
AnyObject<N>::AnyObject(T&& object):
m_objectHolderStorage()
{
    static_assert(sizeof(T) <= N && "Attempt to storge to static::AnyObject the object bigger than object storage itself");
    std::memset(m_objectHolderStorage.data(), 0, m_objectHolderStorage.size());
    constructObjectHolder<T>(std::forward<T>(object));
}

template<int N>
AnyObject<N>::~AnyObject()
{
    destructObjectHolder();
}

template<int N>
template<typename T>
std::optional<concurrency::Ref<T>> AnyObject<N>::getRef() & {
    auto realObjectHolder = dynamic_cast<RealObjectHolder<T>*>(castObjectHolderStorage());
    if (realObjectHolder) {
        auto ref = realObjectHolder->m_object.getMutableRef();
        return std::make_optional(ref);
    } else {
        return std::nullopt;
    }
}

template<int N>
template<typename T>
std::optional<const concurrency::Ref<T>> AnyObject<N>::getRef() const & {
    auto realObjectHolder = dynamic_cast<RealObjectHolder<T>*>(castObjectHolderStorage());
    if (realObjectHolder) {
        auto ref = realObjectHolder->m_object.getMutableRef();
        return std::make_optional(ref);
    } else {
        return std::nullopt;
    }
}

template<int N>
template<typename T>
void AnyObject<N>::constructObjectHolder(T&& object)
{
    new (m_objectHolderStorage.data()) RealObjectHolder<T>(std::forward<T>(object));
}

template<int N>
void AnyObject<N>::destructObjectHolder()
{
    castObjectHolderStorage()->~BaseObjectHolder();
}

template<int N>
typename AnyObject<N>::BaseObjectHolder* AnyObject<N>::castObjectHolderStorage()
{
    return reinterpret_cast<BaseObjectHolder*>(m_objectHolderStorage.data());
}

} //! namespace statical impl section

namespace dynamical {

template<class T>
AnyObject::AnyObject(T&& object):
m_objectHolder(std::make_unique<RealObjectHolder<T>>(std::forward<T>(object)))
{}

template<typename T>
std::optional<concurrency::Ref<T>> AnyObject::getRef() & {
    auto realObjectHolder = dynamic_cast<RealObjectHolder<T>*>(m_objectHolder.get());
    if (realObjectHolder) {
        auto ref = realObjectHolder->m_object.getMutableRef();
        return std::make_optional(ref);
    } else {
        return std::nullopt;
    }
}

template<typename T>
std::optional<const concurrency::Ref<T>> AnyObject::getRef() const & {
    auto realObjectHolder = dynamic_cast<RealObjectHolder<T>*>(m_objectHolder.get());
    if (realObjectHolder) {
        auto ref = realObjectHolder->m_object.getMutableRef();
        return std::make_optional(ref);
    } else {
        return std::nullopt;
    }
}

} //! namespace dynamical impl section

} //! namespace atom
