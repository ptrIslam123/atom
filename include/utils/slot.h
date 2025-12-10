#ifndef ATOM_UTILS_SLOT_H
#define ATOM_UTILS_SLOT_H

#include <utility>

namespace atom::utils {

/**
 * @brief A lightweight, non-owning, type-erased function slot for binding member functions.
 *
 * `Slot` is a minimalistic alternative to `std::function` when you only need to bind
 * a single member function to an object instance (i.e., a method call + receiver).
 * It stores a pointer to the object and a static thunk that forwards the call.
 *
 * Key properties:
 * - Zero heap allocation (all storage is inline).
 * - No virtual calls — dispatch via plain function pointer.
 * - Does *not* manage object lifetime (raw pointer to object is stored).
 * - Supports perfect forwarding of arguments.
 * - Designed for tight integration in event/callback systems where overhead matters.
 *
 * Safety note: The bound object must outlive the `Slot`. No ownership is taken.
 *
 * @tparam Ret The return type of the callable.
 * @tparam Args Parameter types of the callable.
 *
 * Usage:
 * @code{.cpp}
 * struct MyClass {
 *     int foo(int x, double y) { return x + static_cast<int>(y); }
 * };
 *
 * MyClass obj;
 * Slot<int(int, double)> slot;
 * slot.bind<MyClass, &MyClass::foo>(obj);
 *
 * int result = slot(10, 3.14); // calls obj.foo(10, 3.14)
 * @endcode
 */
template<typename Sig>
class Slot;

template<typename Ret, typename ... Arg>
class Slot<Ret(Arg ... )> {
public:
    /**
     * @brief Binds a member function pointer to a specific object instance.
     *
     * @tparam Obj Type of the object to bind (deduced).
     * @tparam memfn Pointer to member function (must be a non-type template parameter).
     * @param obj Reference to the object instance whose method will be called.
     *
     * @note `memfn` must be a compile-time constant expression (hence template NTTP).
     *       This enables zero-cost abstraction: the thunk is generated at compile time.
     */
    template<class Obj, auto memfn>
    void bind(Obj& obj);

    /**
     * @brief Invokes the bound member function with given arguments.
     *
     * If no method has been bound (i.e., `bind()` not called), behavior is undefined
     * (dereferences null function pointer).
     *
     * @param args Arguments forwarded to the bound member function.
     * @return Return value of the bound method.
     */
    Ret operator()(const Arg& ... args) {
        return method(self, std::forward<Arg>(args)...);
    }

private:
    using Method = Ret(*)(void*, Arg ... );
    void* self{nullptr};   ///< Pointer to bound object (non-owning).
    Method method{nullptr};  ///< Static thunk that casts `self` and calls `memfn`.
};

template<class Ret, class... Args>
template<class Obj, auto memfn>
void Slot<Ret(Args...)>::bind(Obj& obj) {
    method = +[](void* self, Args... args) -> Ret {
        return (static_cast<Obj*>(self)->*memfn)(std::forward<Args>(args)...);
    };
    self = &obj;
}

}  // namespace atom::utils

#endif  // ATOM_UTILS_SLOT_H
