#ifndef ATOM_SFINAE_HAS_CLASS_OPERATORS_H
#define ATOM_SFINAE_HAS_CLASS_OPERATORS_H

#include <utility>
#include <type_traits>

template<typename T, typename RetType, typename Arg>
struct has_operator_plus {
private:
    template<typename U>
    static auto test(int) -> std::enable_if_t<
        std::is_same_v<RetType, decltype(std::declval<U>().operator+(std::declval<Arg>()))>,
        std::true_type>;

    template<typename>
    static std::false_type test(...);

public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

template<typename T, typename RetType, typename Arg>
inline constexpr bool has_operator_plus_v = has_operator_plus<T, RetType, Arg>::value;



template<typename T, typename RetType, typename Arg>
struct has_operator_minus {
private:
    template<typename U>
    static auto test(int) -> std::enable_if_t<
        std::is_same_v<RetType, decltype(std::declval<U>().operator-(std::declval<Arg>()))>,
        std::true_type>;

    template<typename>
    static std::false_type test(...);

public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

template<typename T, typename RetType, typename Arg>
inline constexpr bool has_operator_minus_v = has_operator_minus<T, RetType, Arg>::value;



template<typename T, typename RetType = void>
struct has_operator_prefix_plus_plus {
private:
    template<typename U>
    static auto test(int) -> std::enable_if_t<
        std::is_same_v<RetType, decltype(++std::declval<U&>())>,
        std::true_type>;

    template<typename>
    static std::false_type test(...);

public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

template<typename T, typename RetType>
inline constexpr bool has_operator_prefix_plus_plus_v =
    has_operator_prefix_plus_plus<T, RetType>::value;



template<typename T, typename RetType>
struct has_operator_postfix_plus_plus {
private:
    template<typename U>
    static auto test(int) -> std::enable_if_t<
        std::is_same_v<RetType, decltype(std::declval<U&>()++)>,
        std::true_type>;

    template<typename>
    static std::false_type test(...);

public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

template<typename T, typename RetType = void>
inline constexpr bool has_operator_postfix_plus_plus_v =
    has_operator_postfix_plus_plus<T, RetType>::value;



template<typename T, typename RetType>
struct has_operator_prefix_minus_minus {
private:
    template<typename U>
    static auto test(int) -> std::enable_if_t<
        std::is_same_v<RetType, decltype(--std::declval<U&>())>,
        std::true_type>;

    template<typename>
    static std::false_type test(...);

public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

template<typename T, typename RetType>
inline constexpr bool has_operator_prefix_minus_minus_v =
    has_operator_prefix_minus_minus<T, RetType>::value;



template<typename T, typename RetType = void>
struct has_operator_postfix_minus_minus {
private:
    template<typename U>
    static auto test(int) -> std::enable_if_t<
        std::is_same_v<RetType, decltype(std::declval<U&>()--)>,
        std::true_type>;

    template<typename>
    static std::false_type test(...);

public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

template<typename T, typename RetType = void>
inline constexpr bool has_operator_postfix_minus_minus_v =
    has_operator_postfix_minus_minus<T, RetType>::value;



template<typename T, typename RetType, typename RightOperand>
struct has_operator_plus_any {
private:
    template<typename U>
    static auto test(int) -> std::enable_if_t<
        std::is_same_v<RetType, decltype(std::declval<U>() + std::declval<RightOperand>())>,
        std::true_type>;

    template<typename>
    static std::false_type test(...);

public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

template<typename T, typename RetType, typename RightOperand>
inline constexpr bool has_operator_plus_any_v =
    has_operator_plus_any<T, RetType, RightOperand>::value;



template<typename T, typename RetType, typename RightOperand>
struct has_operator_minus_any {
private:
    template<typename U>
    static auto test(int) -> std::enable_if_t<
        std::is_same_v<RetType, decltype(std::declval<U>() - std::declval<RightOperand>())>,
        std::true_type>;

    template<typename>
    static std::false_type test(...);

public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

template<typename T, typename RetType, typename RightOperand>
inline constexpr bool has_operator_minus_any_v =
    has_operator_minus_any<T, RetType, RightOperand>::value;



template<typename T, typename Arg>
struct has_operator_less {
private:
    template<typename U>
    static auto test(int) -> decltype(
        std::declval<U>() < std::declval<Arg>(),
        std::true_type{});

    template<typename>
    static std::false_type test(...);

public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

template<typename T, typename Arg>
inline constexpr bool has_operator_less_v =
    has_operator_less<T, Arg>::value;



template<typename T, typename Arg>
struct has_operator_less_eq {
private:
    template<typename U>
    static auto test(int) -> decltype(
        std::declval<U>() <= std::declval<Arg>(),
        std::true_type{});

    template<typename>
    static std::false_type test(...);

public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

template<typename T, typename Arg>
inline constexpr bool has_operator_less_eq_v =
    has_operator_less_eq<T, Arg>::value;



template<typename T, typename Arg>
struct has_operator_greater {
private:
    template<typename U>
    static auto test(int) -> decltype(
        std::declval<U>() > std::declval<Arg>(),
        std::true_type{});

    template<typename>
    static std::false_type test(...);

public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

template<typename T, typename Arg>
inline constexpr bool has_operator_greater_v =
    has_operator_greater<T, Arg>::value;



template<typename T, typename Arg>
struct has_operator_greater_eq {
private:
    template<typename U>
    static auto test(int) -> decltype(
        std::declval<U>() >= std::declval<Arg>(),
        std::true_type{});

    template<typename>
    static std::false_type test(...);

public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

template<typename T, typename Arg>
inline constexpr bool has_operator_greater_eq_v =
    has_operator_greater_eq<T, Arg>::value;



template<typename T, typename Arg>
struct has_operator_eq {
private:
    template<typename U>
    static auto test(int) -> decltype(
        std::declval<U>() == std::declval<Arg>(),
        std::true_type{});

    template<typename>
    static std::false_type test(...);

public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

template<typename T, typename Arg>
inline constexpr bool has_operator_eq_v =
    has_operator_eq<T, Arg>::value;



template<typename T, typename Arg>
struct has_operator_not_eq {
private:
    template<typename U>
    static auto test(int) -> decltype(
        std::declval<U>() != std::declval<Arg>(),
        std::true_type{});

    template<typename>
    static std::false_type test(...);

public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

template<typename T, typename Arg>
inline constexpr bool has_operator_not_eq_v =
    has_operator_not_eq<T, Arg>::value;



template<typename T, typename RetType>
struct has_operator_deref {
private:
    template<typename U>
    static auto test(int) -> std::enable_if_t<
        std::is_same_v<RetType, decltype(*std::declval<U>())>,
        std::true_type>;

    template<typename>
    static std::false_type test(...);

public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

template<typename T, typename RetType>
inline constexpr bool has_operator_deref_v =
    has_operator_deref<T, RetType>::value;



template<typename T, typename RetType>
struct has_operator_square_brackets {
private:
    template<typename U>
    static auto test(int) -> std::enable_if_t<
        std::is_same_v<RetType, decltype(std::declval<U>().operator[](0))>,
        std::true_type>;

    template<typename>
    static std::false_type test(...);

public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

template<typename T, typename RetType>
inline constexpr bool has_operator_square_brackets_v =
    has_operator_square_brackets<T, RetType>::value;

#endif //! ATOM_SFINAE_HAS_CLASS_OPERATORS_H
