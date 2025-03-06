#ifndef ATOM_SFINAE_HAS_CLASS_INNER_TYPE_H
#define ATOM_SFINAE_HAS_CLASS_INNER_TYPE_H

#include <type_traits>

/**
 * @def DECLARE_HAS_CLASS_INNER_TYPE(X)
 *
 * @brief A macro to generate a compile-time check for the existence of an inner type X in a given type T.
 *
 * This macro creates a structure in the `atom::sfinae` namespace that can be used to check whether a type T
 * contains an inner type X at compile time.
 *
 * @details use the declaration in separate namespaces: for example declare inside namespace __X_FILE_details {...}
 *
 * @param X The name of the inner type whose existence needs to be checked.
 *
 * @details
 * The macro generates the following components:
 * - A primary template `HAS_CLASS_INNERT_TYPE_##X` that defaults to `std::false_type`.
 * - A specialized template `HAS_CLASS_INNERT_TYPE_##X` that evaluates to `std::true_type` if type T contains the inner type X.
 *
 * Example usage:
 * @code{.cpp}
 * struct Foo {
 *     using X = int;
 * };
 *
 * struct Bar {};
 *
 * DECLARE_HAS_CLASS_INNER_TYPE(X)
 *
 * static_assert(HAS_CLASS_INNERT_TYPE_X<Foo>::value, "Foo must have an inner type X");
 * static_assert(!HAS_CLASS_INNERT_TYPE_X<Bar>::value, "Bar must not have an inner type X");
 * @endcode
 */
#define DECLARE_HAS_CLASS_INNER_TYPE(X)                                                 \
    template <typename T, typename = void>                                              \
    struct HAS_CLASS_INNERT_TYPE_##X : std::false_type {};                              \
    template <typename T##X>                                                            \
    struct HAS_CLASS_INNERT_TYPE_##X<T, std::void_t<typename T::X>> : std::true_type {};

#endif //! ATOM_SFINAE_HAS_CLASS_INNER_TYPE_H
