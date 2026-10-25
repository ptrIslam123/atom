#ifndef ATOM_SFINAE_HAS_CLASS_METHOD_X_H
#define ATOM_SFINAE_HAS_CLASS_METHOD_X_H

#include <type_traits>
#include <tuple>

/**
 * @brief Macro to declare a SFINAE-based checker for the existence of a method in a class.
 *
 * This macro defines a set of helper structures and a final type trait to check if a class `T`
 * has a method named `METHOD` that can be called with arguments of types `Arg...`.
 *
 ** @details use the declaration in separate namespaces: for example declare inside namespace __X_FILE_details {...}
 *
 * @param METHOD The name of the method to check for (e.g., `foo`, `set`).
 *
 * Usage:
 * - Declare the macro for a specific method:
 *   DECLARE_HAS_CLASS_METHOD(foo)
 * - Use the generated type trait to check for the method:
 *   HAS_CLASS_METHOD_foo<MyClass, int, double, const std::string&>::value
 *
 * Example:
 * @code
 * // Declare a checker for the 'foo' method
 * DECLARE_HAS_CLASS_METHOD(foo)
 *
 * struct MyClass {
 *     void foo(int, double, const std::string&) {}
 * };
 *
 * struct NoMethodClass {};
 *
 * static_assert(HAS_CLASS_METHOD_foo<MyClass, int, double, const std::string&>::value,
 *               "MyClass must have the 'foo' method");
 *
 * static_assert(!HAS_CLASS_METHOD_foo<NoMethodClass, int, double, const std::string&>::value,
 *               "NoMethodClass should not have the 'foo' method");
 * @endcode
 */
#define DECLARE_HAS_CLASS_METHOD(METHOD) \
template<bool, class T, class... Arg> \
    struct _HAS_CLASS_METHOD_##METHOD##_ { \
        static constexpr bool value = false; \
    }; \
\
    template<class T, class... Arg> \
    struct _HAS_CLASS_METHOD_##METHOD##_<true, T, Arg...> { \
        static constexpr bool value = true; \
    }; \
\
    template<class T, class... Arg> \
    struct HAS_CLASS_METHOD_##METHOD { \
    private: \
        template<class U, class = decltype(std::declval<U>().METHOD(std::declval<Arg>()...))> \
        static constexpr bool test(int) { return true; } \
\
        template<class> \
        static constexpr bool test(...) { return false; } \
\
    public: \
        static constexpr bool value = test<T>(0); \
    };


#endif //! ATOM_SFINAE_HAS_CLASS_METHOD_X_H
