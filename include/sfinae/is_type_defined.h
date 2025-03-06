#ifndef ATOM_SFINAE_IS_TYPE_DEFINED_H
#define ATOM_SFINAE_IS_TYPE_DEFINED_H

#include <type_traits>

/**
 * @namespace atom::sfinae
 * @brief A namespace for SFINAE-based utilities.
 */
namespace atom::sfinae {

/**
 * @struct IS_TYPE_DEFINED
 *
 * @brief Checks if a type T is defined at compile time.
 *
 * This template struct evaluates to `std::true_type` if the type T is defined,
 * and `std::false_type` otherwise. It uses `std::void_t` to enable SFINAE-based
 * detection of whether the type T exists.
 *
 * @tparam T The type to check for existence.
 * @tparam void_t A helper parameter that defaults to `void`.
 *
 * @details
 * - The primary template defaults to `std::false_type`, assuming the type does not exist.
 * - A specialization exists for when `T` is a valid type, in which case it evaluates to `std::true_type`.
 *
 * Example usage:
 * @code{.cpp}
 * #include "is_type_defined.h"
 * #include <iostream>
 *
 * struct DefinedType {};
 * // Uncomment the following line to test an undefined type:
 * // struct UndefinedType;
 *
 * int main() {
 *     std::cout << std::boolalpha;
 *     std::cout << "DefinedType exists: "
 *               << atom::sfinae::IS_TYPE_DEFINED<DefinedType>::value << "\n";
 *
 *     // Uncommenting the following line will cause a compilation error
 *     // because UndefinedType is not defined:
 *     // std::cout << "UndefinedType exists: "
 *     //           << atom::sfinae::IS_TYPE_DEFINED<UndefinedType>::value << "\n";
 *
 *     return 0;
 * }
 * @endcode
 *
 * Output:
 * @code{.txt}
 * DefinedType exists: true
 * @endcode
 *
 * Note: If you attempt to use `is_type_defined` with an undefined type, the compiler
 * will produce an error during template substitution.
 */
template <typename T, typename = void>
struct IS_TYPE_DEFINED : std::false_type {};

template <typename T>
struct IS_TYPE_DEFINED<T, std::void_t<T>> : std::true_type {};

} // namespace atom::sfinae

#endif // ATOM_SFINAE_IS_TYPE_DEFINED_H
