#ifndef ATOM_TYPES_NUMBERS_H
#define ATOM_TYPES_NUMBERS_H

#include "include/types/type_wrapper.h"

#include <stdexcept>
#include <limits>
#include <cstdint>

/**
 * @brief A wrapper class for numeric types with range validation and arithmetic operations.
 *
 * This class encapsulates a numeric type `T` and provides a set of operations (arithmetic,
 * bitwise, comparison) while ensuring that values remain within the specified range [Min, Max].
 * It inherits from `TypeWrapper` and uses CRTP to allow static dispatch of methods.
 */
namespace atom::types {

constexpr std::int32_t __NUMBER_OPT__ =
    TYPE_WRAPPER_OPT_EQ         |
    TYPE_WRAPPER_OPT_NE         |
    TYPE_WRAPPER_OPT_LESS       |
    TYPE_WRAPPER_OPT_GREATER    |
    TYPE_WRAPPER_OPT_PLUS       |
    TYPE_WRAPPER_OPT_MINUS      |
    TYPE_WRAPPER_OPT_MULT       |
    TYPE_WRAPPER_OPT_DIV        |
    TYPE_WRAPPER_OPT_MOD        |
    TYPE_WRAPPER_OPT_INC        |
    TYPE_WRAPPER_OPT_DEC        |
    TYPE_WRAPPER_OPT_AND        |
    TYPE_WRAPPER_OPT_OR         |
    TYPE_WRAPPER_OPT_NOT        |
    TYPE_WRAPPER_OPT_BIT_AND    |
    TYPE_WRAPPER_OPT_BIT_OR     |
    TYPE_WRAPPER_OPT_BIT_XOR    |
    TYPE_WRAPPER_OPT_BIT_NOT    |
    TYPE_WRAPPER_OPT_LSHIFT     |
    TYPE_WRAPPER_OPT_RSHIFT     |
    TYPE_WRAPPER_OPT_ASSIGN     |
    TYPE_WRAPPER_OPT_COPY       |
    TYPE_WRAPPER_OPT_MOVE
;

/**
 * @class Number
 * @brief A wrapper class for numeric types with range validation and arithmetic operations.
 *
 * @tparam T The underlying numeric type (e.g., int, float).
 * @tparam Min The minimum allowed value for the type (default: std::numeric_limits<T>::min()).
 * @tparam Max The maximum allowed value for the type (default: std::numeric_limits<T>::max()).
 * @throws std::overflow_error.
 *
 * This class ensures that all operations on the wrapped value respect the range [Min, Max].
 * It provides methods for arithmetic, bitwise, and comparison operations, as well as range
 * validation and safe casting.
 */
template<
    typename T,
    T Min = std::numeric_limits<T>::min(),
    T Max = std::numeric_limits<T>::max()
>
class Number final : public TypeWrapper<T, Number<T, Min, Max>, __NUMBER_OPT__> {
public:
    static_assert(std::is_trivial_v<T> && "error: Number from T must be trivial type");

    using SelfType = Number<T, Min, Max>;
    using BaseType = TypeWrapper<T, SelfType, __NUMBER_OPT__>;

    /**
     * @brief Static method to get the minimum allowed value.
     * @return A Number object initialized with the minimum value.
     */
    static constexpr Number MIN();

    /**
     * @brief Static method to get the maximum allowed value.
     * @return A Number object initialized with the maximum value.
     */
    static constexpr Number MAX();

    /**
     * @brief Static method to get the neutral element for addition (0).
     * @return A Number object initialized with the neutral element for addition.
     */
    static constexpr Number NEUTRAL_FOR_PLUS();

    /**
     * @brief Static method to get the identity element for addition (1).
     * @return A Number object initialized with the identity element for addition.
     */
    static constexpr Number IDENTITY_FOR_PLUS();

    /**
     * @brief Static method to get the neutral element for subtraction (0).
     * @return A Number object initialized with the neutral element for subtraction.
     */
    static constexpr Number NEUTRAL_FOR_MINUS();

    /**
     * @brief Static method to get the identity element for subtraction (1).
     * @return A Number object initialized with the identity element for subtraction.
     */
    static constexpr Number IDENTITY_FOR_MINUS();

    /**
     * @brief Constructor to initialize the Number with a value.
     * @tparam D The type of the input value.
     * @param data The value to initialize the Number with.
     * @throws std::overflow_error if the value is out of the allowed range [Min, Max].
     */
    template<typename D>
    explicit Number(D data);

    /**
     * @brief Constructor to initialize the Number with a value of type T.
     * @param data The value to initialize the Number with.
     * @throws std::overflow_error if the value is out of the allowed range [Min, Max].
     */
    explicit Number(T data);
    ~Number() = default;

    // Cmp operations
    /**
     * @brief Checks if the current object is equal to another object with optimization/checks
     * @return true If objects are considered equal otherwise false
     */
    bool isEqualOpt(const BaseType& other) const;

    /**
     * @brief Checks if the current object is less than another object with optimization/checks
     * @return true If current object is less than other otherwise false
     */
    bool isLessOpt(const BaseType& other) const;

    /**
     * @brief Store a new value in the Number.
     * @tparam D The type of the input value.
     * @param data The value to store.
     * @throws std::overflow_error if the value is out of the allowed range [Min, Max].
     */
    template<typename D, typename = std::__enable_if_t<!std::is_const_v<T>>>
    constexpr std::enable_if_t<std::is_trivial_v<D>, void> storeOpt(const D& data);

    // Arithmetic operations
    /**
     * @brief Perform addition with another Number.
     * @param other The other Number to add.
     * @throws std::overflow_error if the result exceeds the allowed range [Min, Max].
     */
    template<typename = std::__enable_if_t<!std::is_const_v<T>>>
    constexpr void plusOpt(const BaseType& other);

    /**
     * @brief Perform subtraction with another Number.
     * @param other The other Number to subtract.
     * @throws std::overflow_error if the result exceeds the allowed range [Min, Max].
     */
    template<typename = std::__enable_if_t<!std::is_const_v<T>>>
    constexpr void minusOpt(const BaseType& other);

    /**
     * @brief Perform multiplication with another Number.
     * @param other The other Number to multiply.
     * @throws std::overflow_error if the result exceeds the allowed range [Min, Max].
     */
    template<typename = std::__enable_if_t<!std::is_const_v<T>>>
    constexpr void multOpt(const BaseType& other);

    /**
     * @brief Perform division with another Number.
     * @param other The other Number to divide by.
     * @throws std::overflow_error if division by zero occurs or the result is invalid.
     */
    template<typename = std::__enable_if_t<!std::is_const_v<T>>>
    constexpr void divisionOpt(const BaseType& other);

    /**
     * @brief Perform modulus operation with another Number.
     * @param other The other Number to take modulus with.
     * @throws std::overflow_error if modulus by zero occurs.
     */
    template<typename = std::__enable_if_t<!std::is_const_v<T>>>
    constexpr void modOpt(const BaseType& other);

    // Bitwise operations
    /**
     * @brief Perform bitwise AND operation with another Number.
     * @param other The other Number to perform AND with.
     */
    template<typename = std::__enable_if_t<!std::is_const_v<T>>>
    constexpr void andOpt(const BaseType& other) noexcept;

    /**
     * @brief Perform bitwise OR operation with another Number.
     * @param other The other Number to perform OR with.
     */
    template<typename = std::__enable_if_t<!std::is_const_v<T>>>
    constexpr void orOpt(const BaseType& other) noexcept;

    /**
     * @brief Perform bitwise XOR operation with another Number.
     * @param other The other Number to perform XOR with.
     */
    template<typename = std::__enable_if_t<!std::is_const_v<T>>>
    constexpr void xorOpt(const BaseType& other) noexcept;

    /**
     * @brief Perform left shift operation with another Number.
     * @param other The other Number providing the shift amount.
     */
    template<typename = std::__enable_if_t<!std::is_const_v<T>>>
    constexpr void leftShiftOpt(const BaseType& other) noexcept;

    /**
     * @brief Perform right shift operation with another Number.
     * @param other The other Number providing the shift amount.
     */
    template<typename = std::__enable_if_t<!std::is_const_v<T>>>
    constexpr void rightShiftOpt(const BaseType& other) noexcept;

private:
    // Helper methods
    template<typename D>
    bool isValidRange(const D& data) const;

    template<typename D = T>
    bool isValidRange(const T& data) const;

    constexpr bool isSumSafe(T v1, T v2);
    constexpr bool isSubSafe(T v1, T v2);
    constexpr bool isMultSafe(T v1, T v2);
    constexpr bool isDivisionSafe(int v1, int v2);
    constexpr bool isModSafe(int v1, int v2);
};

// Static methods for constants
template<typename T, T Min, T Max>
constexpr Number<T, Min, Max> Number<T, Min, Max>::MIN() {
    return Number{Min};
}

template<typename T, T Min, T Max>
constexpr Number<T, Min, Max> Number<T, Min, Max>::MAX() {
    return Number{Max};
}

template<typename T, T Min, T Max>
constexpr Number<T, Min, Max> Number<T, Min, Max>::NEUTRAL_FOR_PLUS() {
    return Number{0};
}

template<typename T, T Min, T Max>
constexpr Number<T, Min, Max> Number<T, Min, Max>::IDENTITY_FOR_PLUS() {
    return Number{1};
}

template<typename T, T Min, T Max>
constexpr Number<T, Min, Max> Number<T, Min, Max>::NEUTRAL_FOR_MINUS() {
    return Number{0};
}

template<typename T, T Min, T Max>
constexpr Number<T, Min, Max> Number<T, Min, Max>::IDENTITY_FOR_MINUS() {
    return Number{1};
}

// Constructors
template<typename T, T Min, T Max>
template<typename D>
Number<T, Min, Max>::Number(D data) : BaseType() {
    storeOpt(data);
}

template<typename T, T Min, T Max>
Number<T, Min, Max>::Number(T data) : BaseType() {
    storeOpt(data);
}

template<typename T, T Min, T Max>
bool Number<T, Min, Max>::isEqualOpt(const BaseType& other) const {
    return BaseType::m_data == other.m_data;
}

template<typename T, T Min, T Max>
bool Number<T, Min, Max>::isLessOpt(const BaseType& other) const {
    return BaseType::m_data < other.m_data;
}

// Store operation
template<typename T, T Min, T Max>
template<typename D, typename>
constexpr std::enable_if_t<std::is_trivial_v<D>, void> Number<T, Min, Max>::storeOpt(const D& data) {
    if (isValidRange<D>(data)) {
        BaseType::m_data = data;
    } else {
        throw std::overflow_error{"Overflow error 'storeOpt'"};
    }
}

// Arithmetic operations
template<typename T, T Min, T Max>
template<typename>
constexpr void Number<T, Min, Max>::plusOpt(const BaseType& other) {
    if (isSumSafe(BaseType::m_data, other.m_data)) {
        BaseType::m_data += other.m_data;
    } else {
        throw std::overflow_error{"Overflow error 'plusOpt'"};
    }
}

template<typename T, T Min, T Max>
template<typename>
constexpr void Number<T, Min, Max>::minusOpt(const BaseType& other) {
    if (isSubSafe(BaseType::m_data, other.m_data)) {
        BaseType::m_data -= other.m_data;
    } else {
        throw std::overflow_error{"Overflow error 'minusOpt'"};
    }
}

template<typename T, T Min, T Max>
template<typename>
constexpr void Number<T, Min, Max>::multOpt(const BaseType& other) {
    if (isMultSafe(BaseType::m_data, other.m_data)) {
        BaseType::m_data *= other.m_data;
    } else {
        throw std::overflow_error{"Overflow error 'multOpt'"};
    }
}

template<typename T, T Min, T Max>
template<typename>
constexpr void Number<T, Min, Max>::divisionOpt(const BaseType& other) {
    if (isDivisionSafe(BaseType::m_data, other.m_data)) {
        BaseType::m_data /= other.m_data;
    } else {
        throw std::overflow_error{"Overflow error 'divisionOpt'"};
    }
}

template<typename T, T Min, T Max>
template<typename>
constexpr void Number<T, Min, Max>::modOpt(const BaseType& other) {
    if (isModSafe(BaseType::m_data, other.m_data)) {
        BaseType::m_data %= other.m_data;
    } else {
        throw std::overflow_error{"Overflow error 'modOpt'"};
    }
}

// Bitwise operations
template<typename T, T Min, T Max>
template<typename>
constexpr void Number<T, Min, Max>::andOpt(const BaseType& other) noexcept {
    BaseType::m_data &= other.m_data;
}

template<typename T, T Min, T Max>
template<typename>
constexpr void Number<T, Min, Max>::orOpt(const BaseType& other) noexcept {
    BaseType::m_data |= other.m_data;
}

template<typename T, T Min, T Max>
template<typename>
constexpr void Number<T, Min, Max>::xorOpt(const BaseType& other) noexcept {
    BaseType::m_data ^= other.m_data;
}

template<typename T, T Min, T Max>
template<typename>
constexpr void Number<T, Min, Max>::leftShiftOpt(const BaseType& other) noexcept {
    BaseType::m_data <<= other.m_data;
}

template<typename T, T Min, T Max>
template<typename>
constexpr void Number<T, Min, Max>::rightShiftOpt(const BaseType& other) noexcept {
    BaseType::m_data >>= other.m_data;
}

// Helper methods
template<typename T, T Min, T Max>
template<typename D>
bool Number<T, Min, Max>::isValidRange(const D& data) const {
    if constexpr (sizeof(T) >= sizeof(D) &&
                  ((std::is_signed_v<T> == std::is_signed_v<D>) || (!std::is_signed_v<T> == !std::is_signed_v<D>))) {
        return true;
    }

    if constexpr (std::is_signed_v<T> && std::is_signed_v<D>) {
        return data >= Min && data <= Max;
    } else if constexpr (!std::is_signed_v<T> && !std::is_signed_v<D>) {
        return data <= Max;
    } else if constexpr (std::is_signed_v<T> && !std::is_signed_v<D>) {
        return data <= static_cast<std::make_unsigned_t<T>>(Max);
    } else if constexpr (!std::is_signed_v<T> && std::is_signed_v<D>) {
        return data >= 0 && static_cast<std::make_unsigned_t<D>>(data) <= Max;
    }
    return false;
}

template<typename T, T Min, T Max>
template<typename D>
bool Number<T, Min, Max>::isValidRange(const T& data) const {
    return (data >= Min && data <= Max);
}

template<typename T, T Min, T Max>
constexpr bool Number<T, Min, Max>::isSumSafe(T v1, T v2) {
    if (v1 > 0 && v2 > Max - v1) {
        return false;
    }
    if (v1 < 0 && v2 < Min - v1) {
        return false;
    }
    return true;
}

template<typename T, T Min, T Max>
constexpr bool Number<T, Min, Max>::isSubSafe(T v1, T v2) {
    if (v2 < 0 && v1 > Max + v2) {
        return false;
    }
    if (v2 > 0 && v1 < Min + v2) {
        return false;
    }
    return true;
}

template<typename T, T Min, T Max>
constexpr bool Number<T, Min, Max>::isMultSafe(T v1, T v2) {
    if (v1 > 0 && v2 > 0 && v1 > Max / v2) {
        return false;
    }
    if (v1 > 0 && v2 < 0 && v2 < Min / v1) {
        return false;
    }
    if (v1 < 0 && v2 > 0 && v1 < Min / v2) {
        return false;
    }
    if (v1 < 0 && v2 < 0 && v1 < Max / v2) {
        return false;
    }
    return true;
}

template<typename T, T Min, T Max>
constexpr bool Number<T, Min, Max>::isDivisionSafe(int v1, int v2) {
    if (v2 == 0) {
        return false;
    }
    if (v1 == Min && v2 == -1) {
        return false;
    }
    return true;
}

template<typename T, T Min, T Max>
constexpr bool Number<T, Min, Max>::isModSafe(int v1, int v2) {
    return v2 != 0;
}

} //! namespace atom::types

#endif //! ATOM_TYPES_NUMBERS_H
