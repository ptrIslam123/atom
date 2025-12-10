#ifndef ATOM_SFINAE_TYPE_LIST_H
#define ATOM_SFINAE_TYPE_LIST_H

#include <cstddef>

namespace atom::sfinae {

template<typename ... Ts>
struct TypeList;

template<typename Head, typename ... Tail>
struct TypeList<Head, Tail ... > {
    using HeadType = Head;
    using TailType = TypeList<Tail ... >;
};

template<>
struct TypeList<> {};

template<typename Head, typename ... Tail>
struct TypeList<TypeList<Head, Tail ... >> {
    using HeadType = Head;
    using TailType = TypeList<Tail ... >;
};

template<>
struct TypeList<TypeList<>> {};

/// sizeof
template<typename T>
struct Sizeof;

template<typename Head, typename ... Tail>
struct Sizeof<TypeList<Head, Tail ...>> {
    static constexpr std::size_t Value = 1 + Sizeof<TypeList<Tail ...>>::Value;
};

template<>
struct Sizeof<TypeList<>> {
    static constexpr std::size_t Value = 0;
};


/// At
template<std::size_t Index, typename T>
struct At;

template<std::size_t Index, typename Head, typename ... Tail>
struct At<Index, TypeList<Head, Tail ...>> {
    static_assert(Index < (1 + sizeof...(Tail)), "Index out of bounds");
    using Type = typename At<Index - 1, TypeList<Tail ...>>::Type;
};

template<typename Head, typename ... Tail>
struct At<0, TypeList<Head, Tail ... >> {
    using Type = Head;
};

template<std::size_t Index>
struct At<Index, TypeList<>> {
    static_assert(Index == 0, "Cannot get type from empty TypeList or index out of bounds");
    using Type = void;
};

// // InsertAt
// template<typename T, std::size_t Index, typename List>
// struct InsertAt;

// // Специализация для пустого списка (Index должен быть 0)
// template<typename T>
// struct InsertAt<T, 0, TypeList<>> {
//     using Type = TypeList<T>;
// };

// // Специализация для вставки в начало (Index = 0, список не пустой)
// template<typename T, typename Head, typename... Tail>
// struct InsertAt<T, 0, TypeList<Head, Tail...>> {
//     using Type = TypeList<T, Head, Tail...>;
// };

// // Рекурсивный случай (Index > 0)
// template<typename T, std::size_t Index, typename Head, typename... Tail>
// struct InsertAt<T, Index, TypeList<Head, Tail...>> {
//     using Type = TypeList<Head, typename InsertAt<T, Index - 1, TypeList<Tail...>>::Type>;
// };

} //! namespace atom::sfinae

#endif //! ATOM_SFINAE_TYPE_LIST_H
