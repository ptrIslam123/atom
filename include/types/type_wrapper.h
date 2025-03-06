#ifndef ATOM_TYPE_WRAPPER_H
#define ATOM_TYPE_WRAPPER_H

#include "include/sfinae/has_class_method.h"

#include <utility>
#include <type_traits>
#include <cstdint>


/**
 * @brief A generic wrapper class for encapsulating and extending functionality of types.
 *
 * This class provides a unified interface for working with various data types,
 * including arithmetic, logical, bitwise, and comparison operations. It uses the
 * CRTP (Curiously Recurring Template Pattern) to allow derived classes to extend
 * or override behavior while maintaining static dispatch.
 */
namespace atom::types {

namespace __type_wrapper_details {

// Declare macros for checking class methods
DECLARE_HAS_CLASS_METHOD(NEUTRAL_FOR_PLUS)
DECLARE_HAS_CLASS_METHOD(IDENTITY_FOR_PLUS)
DECLARE_HAS_CLASS_METHOD(NEUTRAL_FOR_MINUS)
DECLARE_HAS_CLASS_METHOD(IDENTITY_FOR_MINUS)

DECLARE_HAS_CLASS_METHOD(isEqualOpt)
DECLARE_HAS_CLASS_METHOD(isLessOpt)
DECLARE_HAS_CLASS_METHOD(storeOpt)
DECLARE_HAS_CLASS_METHOD(plusOpt)
DECLARE_HAS_CLASS_METHOD(minusOpt)
DECLARE_HAS_CLASS_METHOD(multOpt)
DECLARE_HAS_CLASS_METHOD(divisionOpt)
DECLARE_HAS_CLASS_METHOD(modOpt)
DECLARE_HAS_CLASS_METHOD(andOpt)
DECLARE_HAS_CLASS_METHOD(orOpt)
DECLARE_HAS_CLASS_METHOD(xorOpt)
DECLARE_HAS_CLASS_METHOD(leftShiftOpt)
DECLARE_HAS_CLASS_METHOD(rightShiftOpt)

} //! namespace __type_wrapper_details

/**
 * @enum TYPE_WRAPPER_OPT
 * @brief Enumeration of supported operations for TypeWrapper.
 *
 * Each operation is represented by a bit flag, allowing fine-grained control
 * over which operations are enabled for a specific TypeWrapper instance.
 */
enum TYPE_WRAPPER_OPT : std::int32_t {
    // Comparison operators
    TYPE_WRAPPER_OPT_EQ       = 1 << 0,   ///< Equality operator (==)
    TYPE_WRAPPER_OPT_NE       = 1 << 1,   ///< Inequality operator (!=)
    TYPE_WRAPPER_OPT_LESS     = 1 << 2,   ///< Less-than operator (<)
    TYPE_WRAPPER_OPT_GREATER  = 1 << 4,   ///< Greater-than operator (>)

    // Arithmetic operators
    TYPE_WRAPPER_OPT_PLUS     = 1 << 7,   ///< Addition operator (+)
    TYPE_WRAPPER_OPT_MINUS    = 1 << 8,   ///< Subtraction operator (-)
    TYPE_WRAPPER_OPT_MULT     = 1 << 9,   ///< Multiplication operator (*)
    TYPE_WRAPPER_OPT_DIV      = 1 << 10,  ///< Division operator (/)
    TYPE_WRAPPER_OPT_MOD      = 1 << 11,  ///< Modulus operator (%)
    TYPE_WRAPPER_OPT_INC      = 1 << 12,  ///< Increment operator (++)
    TYPE_WRAPPER_OPT_DEC      = 1 << 13,  ///< Decrement operator (--)

    // Logical operators
    TYPE_WRAPPER_OPT_AND      = 1 << 14,  ///< Logical AND operator (&&)
    TYPE_WRAPPER_OPT_OR       = 1 << 15,  ///< Logical OR operator (||)
    TYPE_WRAPPER_OPT_NOT      = 1 << 16,  ///< Logical NOT operator (!)

    // Bitwise operators
    TYPE_WRAPPER_OPT_BIT_AND  = 1 << 17,  ///< Bitwise AND operator (&)
    TYPE_WRAPPER_OPT_BIT_OR   = 1 << 18,  ///< Bitwise OR operator (|)
    TYPE_WRAPPER_OPT_BIT_XOR  = 1 << 19,  ///< Bitwise XOR operator (^)
    TYPE_WRAPPER_OPT_BIT_NOT  = 1 << 20,  ///< Bitwise NOT operator (~)
    TYPE_WRAPPER_OPT_LSHIFT   = 1 << 21,  ///< Left shift operator (<<)
    TYPE_WRAPPER_OPT_RSHIFT   = 1 << 22,  ///< Right shift operator (>>)

    // Assignment operators
    TYPE_WRAPPER_OPT_ASSIGN   = 1 << 23,  ///< Assignment operator (=)

    // Copy/Move operations
    TYPE_WRAPPER_OPT_COPY     = 1 << 25,  ///< Copy operation
    TYPE_WRAPPER_OPT_MOVE     = 1 << 26,  ///< Move operation
};

/**
 * @class TypeWrapper
 * @brief A generic wrapper class for encapsulating and extending functionality of types.
 *
 * @tparam T The underlying type being wrapped.
 * @tparam Derived The derived class using CRTP (Curiously Recurring Template Pattern).
 * @tparam OPT Bitmask of enabled operations from TYPE_WRAPPER_OPT.
 *
 * This class provides a unified interface for working with various data types,
 * including arithmetic, logical, bitwise, and comparison operations. It uses
 * CRTP to allow derived classes to extend or override behavior while maintaining
 * static dispatch. Operations are enabled or disabled based on the OPT bitmask.
 */
template<typename T, typename Derived, std::int32_t OPT>
class TypeWrapper {
public:
    static_assert(!std::is_void_v<T> && !std::is_volatile_v<T>, "TypeWrapper does not support void or volatile types.");
    using ValueType = std::remove_const_t<T>;

    ~TypeWrapper();

    // Comparison operators
    bool operator==(const Derived& other) const;
    bool operator!=(const Derived& other) const;
    bool operator<(const Derived& other) const;
    bool operator<=(const Derived& other) const;
    bool operator>(const Derived& other) const;
    bool operator>=(const Derived& other) const;

    // Arithmetic operators
    Derived operator+(const Derived& other) const;
    Derived operator-(const Derived& other) const;
    Derived operator*(const Derived& other) const;
    Derived operator/(const Derived& other) const;
    Derived operator%(const Derived& other) const;
    Derived& operator++();    // Pre-increment
    Derived operator++(int); // Post-increment
    Derived& operator--();    // Pre-decrement
    Derived operator--(int); // Post-decrement
    Derived& operator+=(const Derived& other);
    Derived& operator-=(const Derived& other);
    Derived& operator*=(const Derived& other);
    Derived& operator/=(const Derived& other);
    Derived& operator%=(const Derived& other);

    // Logical operators
    bool operator!() const;
    bool operator&&(const Derived& other) const;
    bool operator||(const Derived& other) const;

    // Bitwise operators
    Derived operator~() const;
    Derived operator&(const Derived& other) const;
    Derived operator|(const Derived& other) const;
    Derived operator^(const Derived& other) const;
    Derived operator<<(const Derived& other) const;
    Derived operator>>(const Derived& other) const;

    //TypeWrapper& operator~=(const TypeWrapper& other);
    Derived& operator&=(const Derived& other);
    Derived& operator|=(const Derived& other);
    Derived& operator^=(const Derived& other);
    Derived& operator<<=(const Derived& other);
    Derived& operator>>=(const Derived& other);

    // Assignment operators
    template<typename ... Arg>
    TypeWrapper(Arg&& ... arg);
    TypeWrapper(const TypeWrapper& other);
    TypeWrapper(TypeWrapper&& other);
    TypeWrapper& operator=(const TypeWrapper& other);
    TypeWrapper& operator=(TypeWrapper&& other);

    /**
     * @brief Load the underlying value.
     * @return Const reference to the stored value.
     */
    const T& load() const;

    ///
    /// Aka virtual methods. The derived class have to override these methods.
    ///
    bool isEqualOpt(const TypeWrapper& other) const;
    bool isLessOpt(const TypeWrapper& other) const;

    /**
     * @brief Store a new value in the wrapper.
     * @tparam D Type of the data to store.
     * @param data The value to store.
     */
    template<typename D, typename = std::__enable_if_t<!std::is_const_v<T>>>
    constexpr void storeOpt(const D& data);
    template<typename D, typename = std::__enable_if_t<!std::is_const_v<T>>>
    constexpr void storeOpt(D&& data);

    /**
     * @brief Perform addition with another TypeWrapper.
     * @param other The other TypeWrapper to add.
     */
    template<typename = std::__enable_if_t<!std::is_const_v<T>>>
    constexpr void plusOpt(const TypeWrapper& other);
    template<typename = std::__enable_if_t<!std::is_const_v<T>>>
    constexpr void plusOpt(TypeWrapper&& other);

    /**
     * @brief Perform subtraction with another TypeWrapper.
     * @param other The other TypeWrapper to subtract.
     */
    template<typename = std::__enable_if_t<!std::is_const_v<T>>>
    constexpr void minusOpt(const TypeWrapper& other);
    template<typename = std::__enable_if_t<!std::is_const_v<T>>>
    constexpr void minusOpt(TypeWrapper&& other);

    /**
     * @brief Perform multiplication with another TypeWrapper.
     * @param other The other TypeWrapper to multiply.
     */
    template<typename = std::__enable_if_t<!std::is_const_v<T>>>
    constexpr void multOpt(const TypeWrapper& other);
    template<typename = std::__enable_if_t<!std::is_const_v<T>>>
    constexpr void multOpt(TypeWrapper&& other);

    /**
     * @brief Perform division with another TypeWrapper.
     * @param other The other TypeWrapper to divide.
     */
    template<typename = std::__enable_if_t<!std::is_const_v<T>>>
    constexpr void divisionOpt(const TypeWrapper& other);
    template<typename = std::__enable_if_t<!std::is_const_v<T>>>
    constexpr void divisionOpt(TypeWrapper&& other);

    /**
     * @brief Perform modulus with another TypeWrapper.
     * @param other The other TypeWrapper to take modulus.
     */
    template<typename = std::__enable_if_t<!std::is_const_v<T>>>
    constexpr void modOpt(const TypeWrapper& other);
    template<typename = std::__enable_if_t<!std::is_const_v<T>>>
    constexpr void modOpt(TypeWrapper&& other);

    /**
     * @brief Perform a bitwise AND operation with another TypeWrapper.
     * @param other The other TypeWrapper to perform the AND operation with.
     */
    template<typename = std::__enable_if_t<!std::is_const_v<T>>>
    constexpr void andOpt(const TypeWrapper& other);
    template<typename = std::__enable_if_t<!std::is_const_v<T>>>
    constexpr void andOpt(TypeWrapper&& other);

    /**
     * @brief Perform a bitwise OR operation with another TypeWrapper.
     * @param other The other TypeWrapper to perform the OR operation with.
     */
    template<typename = std::__enable_if_t<!std::is_const_v<T>>>
    constexpr void orOpt(const TypeWrapper& other);
    template<typename = std::__enable_if_t<!std::is_const_v<T>>>
    constexpr void orOpt(TypeWrapper&& other);

    /**
     * @brief Perform a bitwise XOR operation with another TypeWrapper.
     * @param other The other TypeWrapper to perform the XOR operation with.
     */
    template<typename = std::__enable_if_t<!std::is_const_v<T>>>
    constexpr void xorOpt(const TypeWrapper& other);
    template<typename = std::__enable_if_t<!std::is_const_v<T>>>
    constexpr void xorOpt(TypeWrapper&& other);

    /**
     * @brief Perform a left shift operation with another TypeWrapper.
     * @param other The other TypeWrapper providing the shift amount.
     */
    template<typename = std::__enable_if_t<!std::is_const_v<T>>>
    constexpr void leftShiftOpt(const TypeWrapper& other);
    template<typename = std::__enable_if_t<!std::is_const_v<T>>>
    constexpr void leftShiftOpt(TypeWrapper&& other);

    /**
     * @brief Perform a left shift operation with an rvalue TypeWrapper.
     * @param other An rvalue reference to another TypeWrapper providing the shift amount.
     */
    template<typename = std::__enable_if_t<!std::is_const_v<T>>>
    constexpr void rightShiftOpt(const TypeWrapper& other);
    template<typename = std::__enable_if_t<!std::is_const_v<T>>>
    constexpr void rightShiftOpt(TypeWrapper&& other);

private:
    friend Derived;

    /**
     * @brief Get a pointer to the derived class.
     * @return Pointer to the derived class instance.
     */
    const Derived* getDerived() const { return static_cast<const Derived*>(this); }
    Derived* getDerived() { return static_cast<Derived*>(this); }

    T m_data; ///< The underlying data being wrapped.
};

template<typename T, typename Derived, std::int32_t OPT>
TypeWrapper<T, Derived, OPT>::~TypeWrapper() = default;

template<typename T, typename Derived, std::int32_t OPT>
template<typename ... Arg>
TypeWrapper<T, Derived, OPT>::TypeWrapper(Arg&& ... arg):
m_data(std::forward<Arg>(arg) ...) {}

// Comparison operators
template<typename T, typename Derived, std::int32_t OPT>
bool TypeWrapper<T, Derived, OPT>::operator==(const Derived& other) const {
    if constexpr ((OPT & TYPE_WRAPPER_OPT_EQ) != 0) {
        return isEqualOpt(other);
    } else {
        static_assert((OPT & TYPE_WRAPPER_OPT_EQ) != 0, "Equality operator is not enabled");
    }
}

template<typename T, typename Derived, std::int32_t OPT>
bool TypeWrapper<T, Derived, OPT>::operator!=(const Derived& other) const {
    if constexpr ((OPT & TYPE_WRAPPER_OPT_NE) != 0) {
        return !(isEqualOpt(other));
    } else {
        static_assert((OPT & TYPE_WRAPPER_OPT_NE) != 0, "Inequality operator is not enabled");
    }
}

template<typename T, typename Derived, std::int32_t OPT>
bool TypeWrapper<T, Derived, OPT>::operator<(const Derived& other) const {
    if constexpr ((OPT & TYPE_WRAPPER_OPT_LESS) != 0) {
        return isLessOpt(other);
    } else {
        static_assert((OPT & TYPE_WRAPPER_OPT_LESS) != 0, "Less-than operator is not enabled");
    }
}

template<typename T, typename Derived, std::int32_t OPT>
bool TypeWrapper<T, Derived, OPT>::operator<=(const Derived& other) const {
    if constexpr ((OPT & TYPE_WRAPPER_OPT_LESS) != 0 && (OPT & TYPE_WRAPPER_OPT_EQ) != 0) {
        return (isLessOpt(other) || isEqualOpt(other));
    } else {
        static_assert((OPT & TYPE_WRAPPER_OPT_LESS) != 0 && (OPT & TYPE_WRAPPER_OPT_EQ) != 0, "Less-than-or-equal operator is not enabled");
    }
}

template<typename T, typename Derived, std::int32_t OPT>
bool TypeWrapper<T, Derived, OPT>::operator>(const Derived& other) const {
    if constexpr ((OPT & TYPE_WRAPPER_OPT_GREATER) != 0) {
        return !(isLessOpt(other) || isEqualOpt(other));
    } else {
        static_assert((OPT & TYPE_WRAPPER_OPT_GREATER) != 0, "Greater-than operator is not enabled");
    }
}

template<typename T, typename Derived, std::int32_t OPT>
bool TypeWrapper<T, Derived, OPT>::operator>=(const Derived& other) const {
    if constexpr ((OPT & TYPE_WRAPPER_OPT_GREATER) != 0 && (OPT & TYPE_WRAPPER_OPT_EQ) != 0) {
        return (!isLessOpt(other) || isEqualOpt(other));
    } else {
        static_assert((OPT & TYPE_WRAPPER_OPT_GREATER) != 0 && (OPT & TYPE_WRAPPER_OPT_EQ) != 0, "Greater-than-or-equal operator is not enabled");
    }
}

// Arithmetic operators
template<typename T, typename Derived, std::int32_t OPT>
Derived TypeWrapper<T, Derived, OPT>::operator+(const Derived& other) const {
    if constexpr ((OPT & TYPE_WRAPPER_OPT_PLUS) != 0) {
        Derived cp{m_data};
        cp.plusOpt(other);
        return cp;
    } else {
        static_assert((OPT & TYPE_WRAPPER_OPT_PLUS) != 0, "Addition operator is not enabled");
    }
}

template<typename T, typename Derived, std::int32_t OPT>
Derived TypeWrapper<T, Derived, OPT>::operator-(const Derived& other) const {
    if constexpr ((OPT & TYPE_WRAPPER_OPT_MINUS) != 0) {
        Derived cp{m_data};
        cp.minusOpt(other);
        return cp;
    } else {
        static_assert((OPT & TYPE_WRAPPER_OPT_MINUS) != 0, "Subtraction operator is not enabled");
    }
}

template<typename T, typename Derived, std::int32_t OPT>
Derived TypeWrapper<T, Derived, OPT>::operator*(const Derived& other) const {
    if constexpr ((OPT & TYPE_WRAPPER_OPT_MULT) != 0) {
        Derived cp{m_data};
        cp.multOpt(other);
        return cp;
    } else {
        static_assert((OPT & TYPE_WRAPPER_OPT_MULT) != 0, "Multiplication operator is not enabled");
    }
}

template<typename T, typename Derived, std::int32_t OPT>
Derived TypeWrapper<T, Derived, OPT>::operator/(const Derived& other) const {
    if constexpr ((OPT & TYPE_WRAPPER_OPT_DIV) != 0) {
        Derived cp{m_data};
        cp.divisionOpt(other);
        return cp;
    } else {
        static_assert((OPT & TYPE_WRAPPER_OPT_DIV) != 0, "Division operator is not enabled");
    }
}

template<typename T, typename Derived, std::int32_t OPT>
Derived TypeWrapper<T, Derived, OPT>::operator%(const Derived& other) const {
    if constexpr ((OPT & TYPE_WRAPPER_OPT_MOD) != 0) {
        Derived cp{m_data};
        cp.modOpt(other);
        return cp;
    } else {
        static_assert((OPT & TYPE_WRAPPER_OPT_MOD) != 0, "Modulus operator is not enabled");
    }
}

template<typename T, typename Derived, std::int32_t OPT>
Derived& TypeWrapper<T, Derived, OPT>::operator++() {
    if constexpr ((OPT & TYPE_WRAPPER_OPT_INC) != 0) {
        using namespace __type_wrapper_details;
        static_assert(HAS_CLASS_METHOD_IDENTITY_FOR_PLUS<Derived>::value && "error: Derived class had to have 'IDENTITY_FOR_PLUS' static method");
        plusOpt(Derived::IDENTITY_FOR_PLUS());
        return static_cast<Derived&>(*this);
    } else {
        static_assert((OPT & TYPE_WRAPPER_OPT_INC) != 0, "Pre-increment operator is not enabled");
    }
}

template<typename T, typename Derived, std::int32_t OPT>
Derived TypeWrapper<T, Derived, OPT>::operator++(int) {
    if constexpr ((OPT & TYPE_WRAPPER_OPT_INC) != 0) {
        using namespace __type_wrapper_details;
        static_assert(HAS_CLASS_METHOD_IDENTITY_FOR_PLUS<Derived>::value && "error: Derived class had to have 'IDENTITY_FOR_PLUS' static method");
        Derived cp{m_data};
        plusOpt(Derived::IDENTITY_FOR_PLUS());
        return cp;
    } else {
        static_assert((OPT & TYPE_WRAPPER_OPT_INC) != 0, "Post-increment operator is not enabled");
    }
}

template<typename T, typename Derived, std::int32_t OPT>
Derived& TypeWrapper<T, Derived, OPT>::operator--() {
    if constexpr ((OPT & TYPE_WRAPPER_OPT_DEC) != 0) {
        using namespace __type_wrapper_details;
        static_assert(HAS_CLASS_METHOD_IDENTITY_FOR_MINUS<Derived>::value && "error: Derived class had to have 'IDENTITY_FOR_MINUS' static method");
        minusOpt(Derived::IDENTITY_FOR_MINUS());
        return *this;
    } else {
        static_assert((OPT & TYPE_WRAPPER_OPT_DEC) != 0, "Pre-decrement operator is not enabled");
    }
}

template<typename T, typename Derived, std::int32_t OPT>
Derived TypeWrapper<T, Derived, OPT>::operator--(int) {
    if constexpr ((OPT & TYPE_WRAPPER_OPT_DEC) != 0) {
        using namespace __type_wrapper_details;
        static_assert(HAS_CLASS_METHOD_IDENTITY_FOR_MINUS<Derived>::value && "error: Derived class had to have 'IDENTITY_FOR_MINUS' static method");
        Derived cp{m_data};
        minusOpt(Derived::IDENTITY_FOR_MINUS());
        return cp;
    } else {
        static_assert((OPT & TYPE_WRAPPER_OPT_DEC) != 0, "Post-decrement operator is not enabled");
    }
}

// Logical operators
template<typename T, typename Derived, std::int32_t OPT>
bool TypeWrapper<T, Derived, OPT>::operator!() const {
    if constexpr ((OPT & TYPE_WRAPPER_OPT_NOT) != 0) {
        return !m_data;
    } else {
        static_assert((OPT & TYPE_WRAPPER_OPT_NOT) != 0, "Logical NOT operator is not enabled");
    }
}

template<typename T, typename Derived, std::int32_t OPT>
bool TypeWrapper<T, Derived, OPT>::operator||(const Derived& other) const {
    if constexpr ((OPT & TYPE_WRAPPER_OPT_OR) != 0) {
        return m_data || other.m_data;
    } else {
        static_assert((OPT & TYPE_WRAPPER_OPT_OR) != 0, "Logical OR operator is not enabled");
    }
}

// Bitwise operators
template<typename T, typename Derived, std::int32_t OPT>
Derived TypeWrapper<T, Derived, OPT>::operator~() const {
    if constexpr ((OPT & TYPE_WRAPPER_OPT_BIT_NOT) != 0) {
        return Derived(~m_data);
    } else {
        static_assert((OPT & TYPE_WRAPPER_OPT_BIT_NOT) != 0, "Bitwise NOT operator is not enabled");
    }
}

template<typename T, typename Derived, std::int32_t OPT>
Derived TypeWrapper<T, Derived, OPT>::operator&(const Derived& other) const {
    if constexpr ((OPT & TYPE_WRAPPER_OPT_BIT_AND) != 0) {
        Derived cp{m_data};
        cp.andOpt(other);
        return cp;
    } else {
        static_assert((OPT & TYPE_WRAPPER_OPT_BIT_AND) != 0, "Bitwise AND operator is not enabled");
    }
}

template<typename T, typename Derived, std::int32_t OPT>
Derived TypeWrapper<T, Derived, OPT>::operator|(const Derived& other) const {
    if constexpr ((OPT & TYPE_WRAPPER_OPT_BIT_OR) != 0) {
        Derived cp{m_data};
        cp.orOpt(other);
        return cp;
    } else {
        static_assert((OPT & TYPE_WRAPPER_OPT_BIT_OR) != 0, "Bitwise OR operator is not enabled");
    }
}

template<typename T, typename Derived, std::int32_t OPT>
Derived TypeWrapper<T, Derived, OPT>::operator^(const Derived& other) const {
    if constexpr ((OPT & TYPE_WRAPPER_OPT_BIT_XOR) != 0) {
        Derived cp{m_data};
        cp.xorOpt(other);
        return cp;
    } else {
        static_assert((OPT & TYPE_WRAPPER_OPT_BIT_XOR) != 0, "Bitwise XOR operator is not enabled");
    }
}

template<typename T, typename Derived, std::int32_t OPT>
Derived TypeWrapper<T, Derived, OPT>::operator<<(const Derived& other) const {
    if constexpr ((OPT & TYPE_WRAPPER_OPT_LSHIFT) != 0) {
        Derived cp{m_data};
        cp.leftShiftOpt(other);
        return cp;
    } else {
        static_assert((OPT & TYPE_WRAPPER_OPT_LSHIFT) != 0, "Left shift operator is not enabled");
    }
}

template<typename T, typename Derived, std::int32_t OPT>
Derived TypeWrapper<T, Derived, OPT>::operator>>(const Derived& other) const {
    if constexpr ((OPT & TYPE_WRAPPER_OPT_RSHIFT) != 0) {
        Derived cp{m_data};
        cp.rightShiftOpt(other);
        return cp;
    } else {
        static_assert((OPT & TYPE_WRAPPER_OPT_RSHIFT) != 0, "Right shift operator is not enabled");
    }
}

template<typename T, typename Derived, std::int32_t OPT>
Derived& TypeWrapper<T, Derived, OPT>::operator&=(const Derived& other) {
    if constexpr ((OPT & TYPE_WRAPPER_OPT_AND) != 0) {
        andOpt(other);
        return static_cast<Derived&>(*this);
    } else {
        static_assert((OPT & TYPE_WRAPPER_OPT_AND) != 0, "Bitwise AND assignment operator is not enabled");
    }
}

template<typename T, typename Derived, std::int32_t OPT>
Derived& TypeWrapper<T, Derived, OPT>::operator|=(const Derived& other) {
    if constexpr ((OPT & TYPE_WRAPPER_OPT_OR) != 0) {
        orOpt(other);
        return static_cast<Derived&>(*this);
    } else {
        static_assert((OPT & TYPE_WRAPPER_OPT_OR) != 0, "Bitwise OR assignment operator is not enabled");
    }
}

template<typename T, typename Derived, std::int32_t OPT>
Derived& TypeWrapper<T, Derived, OPT>::operator^=(const Derived& other) {
    if constexpr ((OPT & TYPE_WRAPPER_OPT_BIT_XOR) != 0) {
        xorOpt(other);
        return static_cast<Derived&>(*this);
    } else {
        static_assert((OPT & TYPE_WRAPPER_OPT_BIT_XOR) != 0, "Bitwise XOR assignment operator is not enabled");
    }
}

template<typename T, typename Derived, std::int32_t OPT>
Derived& TypeWrapper<T, Derived, OPT>::operator<<=(const Derived& other) {
    if constexpr ((OPT & TYPE_WRAPPER_OPT_LSHIFT) != 0) {
        leftShiftOpt(other);
        return static_cast<Derived&>(*this);
    } else {
        static_assert((OPT & TYPE_WRAPPER_OPT_LSHIFT) != 0, "Left shift assignment operator is not enabled");
    }
}

template<typename T, typename Derived, std::int32_t OPT>
Derived& TypeWrapper<T, Derived, OPT>::operator>>=(const Derived& other) {
    if constexpr ((OPT & TYPE_WRAPPER_OPT_RSHIFT) != 0) {
        rightShiftOpt(other);
        return static_cast<Derived&>(*this);
    } else {
        static_assert((OPT & TYPE_WRAPPER_OPT_RSHIFT) != 0, "Right shift assignment operator is not enabled");
    }
}

// Assignment operators
template<typename T, typename Derived, std::int32_t OPT>
Derived& TypeWrapper<T, Derived, OPT>::operator+=(const Derived& other) {
    if constexpr ((OPT & TYPE_WRAPPER_OPT_PLUS) != 0) {
        plusOpt(other);
        return static_cast<Derived&>(*this);
    } else {
        static_assert((OPT & TYPE_WRAPPER_OPT_PLUS) != 0, "Addition assignment operator is not enabled");
    }
}

template<typename T, typename Derived, std::int32_t OPT>
Derived& TypeWrapper<T, Derived, OPT>::operator-=(const Derived& other) {
    if constexpr ((OPT & TYPE_WRAPPER_OPT_MINUS) != 0) {
        minusOpt(other);
        return static_cast<Derived&>(*this);
    } else {
        static_assert((OPT & TYPE_WRAPPER_OPT_MINUS) != 0, "Subtraction assignment operator is not enabled");
    }
}

template<typename T, typename Derived, std::int32_t OPT>
Derived& TypeWrapper<T, Derived, OPT>::operator*=(const Derived& other) {
    if constexpr ((OPT & TYPE_WRAPPER_OPT_MULT) != 0) {
        multOpt(other);
        return static_cast<Derived&>(*this);
    } else {
        static_assert((OPT & TYPE_WRAPPER_OPT_MULT) != 0, "Multiplication assignment operator is not enabled");
    }
}

template<typename T, typename Derived, std::int32_t OPT>
Derived& TypeWrapper<T, Derived, OPT>::operator/=(const Derived& other) {
    if constexpr ((OPT & TYPE_WRAPPER_OPT_DIV) != 0) {
        divisionOpt(other);
        return static_cast<Derived&>(*this);
    } else {
        static_assert((OPT & TYPE_WRAPPER_OPT_DIV) != 0, "Division assignment operator is not enabled");
    }
}

template<typename T, typename Derived, std::int32_t OPT>
Derived& TypeWrapper<T, Derived, OPT>::operator%=(const Derived& other) {
    if constexpr ((OPT & TYPE_WRAPPER_OPT_MOD) != 0) {
        modOpt(other);
        return static_cast<Derived&>(*this);
    } else {
        static_assert((OPT & TYPE_WRAPPER_OPT_MOD) != 0, "Modulus assignment operator is not enabled");
    }
}


// Copy/Move operators
template<typename T, typename Derived, std::int32_t OPT>
TypeWrapper<T, Derived, OPT>::TypeWrapper(const TypeWrapper& other) {
    if constexpr ((OPT & TYPE_WRAPPER_OPT_COPY) != 0) {
        storeOpt(other.m_data);
    } else {
        static_assert((OPT & TYPE_WRAPPER_OPT_COPY) != 0, "Copy constructor is not enabled");
    }
}

template<typename T, typename Derived, std::int32_t OPT>
TypeWrapper<T, Derived, OPT>::TypeWrapper(TypeWrapper&& other) {
    if constexpr ((OPT & TYPE_WRAPPER_OPT_MOVE) != 0) {
        storeOpt(std::move(other.m_data));
    } else {
        static_assert((OPT & TYPE_WRAPPER_OPT_MOVE) != 0, "Move constructor is not enabled");
    }
}

template<typename T, typename Derived, std::int32_t OPT>
TypeWrapper<T, Derived, OPT>& TypeWrapper<T, Derived, OPT>::operator=(const TypeWrapper& other) {
    if constexpr ((OPT & TYPE_WRAPPER_OPT_COPY) != 0) {
        if (this != &other) {
            storeOpt(other.m_data);
        }
        return *this;
    } else {
        static_assert((OPT & TYPE_WRAPPER_OPT_COPY) != 0, "Copy assignment operator is not enabled");
    }
}

template<typename T, typename Derived, std::int32_t OPT>
TypeWrapper<T, Derived, OPT>& TypeWrapper<T, Derived, OPT>::operator=(TypeWrapper&& other) {
    if constexpr ((OPT & TYPE_WRAPPER_OPT_MOVE) != 0) {
        if (this != &other) {
            storeOpt(std::move(other.m_data));
        }
        return *this;
    } else {
        static_assert((OPT & TYPE_WRAPPER_OPT_MOVE) != 0, "Move assignment operator is not enabled");
    }
}

template<typename T, typename Derived, std::int32_t OPT>
bool TypeWrapper<T, Derived, OPT>::isEqualOpt(const TypeWrapper& other) const {
    using namespace __type_wrapper_details;
    static_assert(HAS_CLASS_METHOD_isEqualOpt<Derived, const TypeWrapper&>::value &&
                  "error: static assertion failed: Error: Derived class does not implement the 'isEqualOpt' method");
    return getDerived()->isEqualOpt(other);
}

template<typename T, typename Derived, std::int32_t OPT>
bool TypeWrapper<T, Derived, OPT>::isLessOpt(const TypeWrapper& other) const {
    using namespace __type_wrapper_details;
    static_assert(HAS_CLASS_METHOD_isLessOpt<Derived, const TypeWrapper&>::value &&
                  "error: static assertion failed: Error: Derived class does not implement the 'isLessOpt' method");
    return getDerived()->isLessOpt(other);
}

template<typename T, typename Derived, std::int32_t OPT>
template<typename D, typename>
constexpr void TypeWrapper<T, Derived, OPT>::storeOpt(const D& data) {
    using namespace __type_wrapper_details;
    static_assert((HAS_CLASS_METHOD_storeOpt<Derived, const D&>::value ||  HAS_CLASS_METHOD_storeOpt<Derived, D&&>::value) &&
                  "error: static assertion failed: Error: Derived class does not implement the 'storeOpt' method");
    if constexpr (HAS_CLASS_METHOD_storeOpt<Derived, const D&>::value) {
        getDerived()->storeOpt(data);
    } else if constexpr (HAS_CLASS_METHOD_storeOpt<Derived, D&&>::value) {
        getDerived()->storeOpt(data);
    }
}

template<typename T, typename Derived, std::int32_t OPT>
template<typename D, typename>
constexpr void TypeWrapper<T, Derived, OPT>::storeOpt(D&& data) {
    using namespace __type_wrapper_details;
    static_assert((HAS_CLASS_METHOD_storeOpt<Derived, const D&>::value ||  HAS_CLASS_METHOD_storeOpt<Derived, D&&>::value) &&
                  "error: static assertion failed: Error: Derived class does not implement the 'storeOpt' method");
    if constexpr (HAS_CLASS_METHOD_storeOpt<Derived, D&&>::value) {
        getDerived()->storeOpt(std::move(data));
    } else if constexpr (HAS_CLASS_METHOD_storeOpt<Derived, const D&>::value) {
        getDerived()->storeOpt(data);
    }
}

template<typename T, typename Derived, std::int32_t OPT>
template<typename>
constexpr void TypeWrapper<T, Derived, OPT>::plusOpt(const TypeWrapper& data) {
    using namespace __type_wrapper_details;
    static_assert((HAS_CLASS_METHOD_plusOpt<Derived, TypeWrapper&&>::value || HAS_CLASS_METHOD_plusOpt<Derived, const TypeWrapper&>::value) &&
                  "error: static assertion failed: Error: Derived class does not implement the 'plusOpt' method");
    getDerived()->plusOpt(data);
}

template<typename T, typename Derived, std::int32_t OPT>
template<typename>
constexpr void TypeWrapper<T, Derived, OPT>::plusOpt(TypeWrapper&& data) {
    using namespace __type_wrapper_details;
    static_assert((HAS_CLASS_METHOD_plusOpt<Derived, TypeWrapper&&>::value || HAS_CLASS_METHOD_plusOpt<Derived, const TypeWrapper&>::value) &&
                  "error: static assertion failed: Error: Derived class does not implement the 'plusOpt' method");
    getDerived()->plusOpt(std::move(data));
}

template<typename T, typename Derived, std::int32_t OPT>
template<typename>
constexpr void TypeWrapper<T, Derived, OPT>::minusOpt(const TypeWrapper& data) {
    using namespace __type_wrapper_details;
    static_assert(HAS_CLASS_METHOD_minusOpt<Derived, const TypeWrapper&>::value
                  && "error: static assertion failed: Error: Derived class does not implement the 'minusOpt' method");
    getDerived()->minusOpt(data);
}

template<typename T, typename Derived, std::int32_t OPT>
template<typename>
constexpr void TypeWrapper<T, Derived, OPT>::minusOpt(TypeWrapper&& data) {
    using namespace __type_wrapper_details;
    static_assert(HAS_CLASS_METHOD_minusOpt<Derived, TypeWrapper&&>::value
                  && "error: static assertion failed: Error: Derived class does not implement the 'minusOpt' method");
    getDerived()->minusOpt(std::move(data));
}

template<typename T, typename Derived, std::int32_t OPT>
template<typename>
constexpr void TypeWrapper<T, Derived, OPT>::modOpt(const TypeWrapper& data) {
    using namespace __type_wrapper_details;
    static_assert(HAS_CLASS_METHOD_modOpt<Derived, const TypeWrapper&>::value
                  && "error: static assertion failed: Error: Derived class does not implement the 'modOpt' method");
    getDerived()->modOpt(data);
}

template<typename T, typename Derived, std::int32_t OPT>
template<typename>
constexpr void TypeWrapper<T, Derived, OPT>::modOpt(TypeWrapper&& data) {
    using namespace __type_wrapper_details;
    static_assert(HAS_CLASS_METHOD_modOpt<Derived, TypeWrapper&&>::value
                  && "error: static assertion failed: Error: Derived class does not implement the 'modOpt' method");
    getDerived()->modOpt(std::move(data));
}

template<typename T, typename Derived, std::int32_t OPT>
template<typename>
constexpr void TypeWrapper<T, Derived, OPT>::andOpt(const TypeWrapper& data) {
    using namespace __type_wrapper_details;
    static_assert(HAS_CLASS_METHOD_andOpt<Derived, const TypeWrapper&>::value
                  && "error: static assertion failed: Error: Derived class does not implement the 'andOpt' method");
    getDerived()->andOpt(data);
}

template<typename T, typename Derived, std::int32_t OPT>
template<typename>
constexpr void TypeWrapper<T, Derived, OPT>::andOpt(TypeWrapper&& data) {
    using namespace __type_wrapper_details;
    static_assert(HAS_CLASS_METHOD_andOpt<Derived, TypeWrapper&&>::value
                  && "error: static assertion failed: Error: Derived class does not implement the 'andOpt' method");
    getDerived()->andOpt(std::move(data));
}

template<typename T, typename Derived, std::int32_t OPT>
template<typename>
constexpr void TypeWrapper<T, Derived, OPT>::orOpt(const TypeWrapper& data) {
    using namespace __type_wrapper_details;
    static_assert(HAS_CLASS_METHOD_andOpt<Derived, const TypeWrapper&>::value
                  && "error: static assertion failed: Error: Derived class does not implement the 'orOpt' method");
    getDerived()->orOpt(data);
}

template<typename T, typename Derived, std::int32_t OPT>
template<typename>
constexpr void TypeWrapper<T, Derived, OPT>::orOpt(TypeWrapper&& data) {
    using namespace __type_wrapper_details;
    static_assert(HAS_CLASS_METHOD_andOpt<Derived, TypeWrapper&&>::value
                  && "error: static assertion failed: Error: Derived class does not implement the 'orOpt' method");
    getDerived()->orOpt(std::move(data));
}

template<typename T, typename Derived, std::int32_t OPT>
template<typename>
constexpr void TypeWrapper<T, Derived, OPT>::xorOpt(const TypeWrapper& data) {
    using namespace __type_wrapper_details;
    static_assert(HAS_CLASS_METHOD_andOpt<Derived, const TypeWrapper&>::value
                  && "error: static assertion failed: Error: Derived class does not implement the 'xorOpt' method");
    getDerived()->xorOpt(data);
}

template<typename T, typename Derived, std::int32_t OPT>
template<typename>
constexpr void TypeWrapper<T, Derived, OPT>::xorOpt(TypeWrapper&& data) {
    using namespace __type_wrapper_details;
    static_assert(HAS_CLASS_METHOD_andOpt<Derived, TypeWrapper&&>::value
                  && "error: static assertion failed: Error: Derived class does not implement the 'xorOpt' method");
    getDerived()->xorOpt(std::move(data));
}

template<typename T, typename Derived, std::int32_t OPT>
template<typename>
constexpr void TypeWrapper<T, Derived, OPT>::leftShiftOpt(const TypeWrapper& data) {
    using namespace __type_wrapper_details;
    static_assert(HAS_CLASS_METHOD_leftShiftOpt<Derived, const TypeWrapper&>::value
                  && "error: static assertion failed: Error: Derived class does not implement the 'leftShiftOpt' method");
    getDerived()->leftShiftOpt(data);
}

template<typename T, typename Derived, std::int32_t OPT>
template<typename>
constexpr void TypeWrapper<T, Derived, OPT>::leftShiftOpt(TypeWrapper&& data) {
    using namespace __type_wrapper_details;
    static_assert(HAS_CLASS_METHOD_leftShiftOpt<Derived, TypeWrapper&&>::value
                  && "error: static assertion failed: Error: Derived class does not implement the 'leftShiftOpt' method");
    getDerived()->leftShiftOpt(std::move(data));
}

template<typename T, typename Derived, std::int32_t OPT>
template<typename>
constexpr void TypeWrapper<T, Derived, OPT>::rightShiftOpt(const TypeWrapper& data) {
    using namespace __type_wrapper_details;
    static_assert(HAS_CLASS_METHOD_rightShiftOpt<Derived, const TypeWrapper&>::value
                  && "error: static assertion failed: Error: Derived class does not implement the 'rightShiftOpt' method");
    getDerived()->rightShiftOpt(data);
}

template<typename T, typename Derived, std::int32_t OPT>
template<typename>
constexpr void TypeWrapper<T, Derived, OPT>::rightShiftOpt(TypeWrapper&& data) {
    using namespace __type_wrapper_details;
    static_assert(HAS_CLASS_METHOD_rightShiftOpt<Derived, TypeWrapper&&>::value
                  && "error: static assertion failed: Error: Derived class does not implement the 'rightShiftOpt' method");
    getDerived()->rightShiftOpt(std::move(data));
}

template<typename T, typename Derived, std::int32_t OPT>
template<typename>
constexpr void TypeWrapper<T, Derived, OPT>::multOpt(const TypeWrapper& data) {
    using namespace __type_wrapper_details;
    static_assert(HAS_CLASS_METHOD_andOpt<Derived, const TypeWrapper&>::value
                  && "error: static assertion failed: Error: Derived class does not implement the 'multOpt' method");
    getDerived()->multOpt(data);
}

template<typename T, typename Derived, std::int32_t OPT>
template<typename>
constexpr void TypeWrapper<T, Derived, OPT>::multOpt(TypeWrapper&& data) {
    using namespace __type_wrapper_details;
    static_assert(HAS_CLASS_METHOD_andOpt<Derived, TypeWrapper&&>::value
                  && "error: static assertion failed: Error: Derived class does not implement the 'multOpt' method");
    getDerived()->multOpt(std::move(data));
}

template<typename T, typename Derived, std::int32_t OPT>
template<typename>
constexpr void TypeWrapper<T, Derived, OPT>::divisionOpt(const TypeWrapper& data) {
    using namespace __type_wrapper_details;
    static_assert(HAS_CLASS_METHOD_andOpt<Derived, const TypeWrapper&>::value
                  && "error: static assertion failed: Error: Derived class does not implement the 'divisionOpt' method");
    getDerived()->divisionOpt(data);
}

template<typename T, typename Derived, std::int32_t OPT>
template<typename>
constexpr void TypeWrapper<T, Derived, OPT>::divisionOpt(TypeWrapper&& data) {
    using namespace __type_wrapper_details;
    static_assert(HAS_CLASS_METHOD_andOpt<Derived, TypeWrapper&&>::value
                  && "error: static assertion failed: Error: Derived class does not implement the 'divisionOpt' method");
    getDerived()->divisionOpt(std::move(data));
}

template<typename T, typename Derived, std::int32_t OPT>
const T& TypeWrapper<T, Derived, OPT>::load() const {
    return m_data;
}

} //! namespace atom::types

#endif //! ATOM_TYPE_WRAPPER_H
