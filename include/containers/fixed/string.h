#ifndef ATOM_CONTAINER_FIXED_STRING_H
#define ATOM_CONTAINER_FIXED_STRING_H

#include "include/containers/fixed/array.h"
#include "include/sfinae/char_traits.h"
#include "include/utils/compiler_attr.h"

namespace atom::containers::fixed {

template<typename CharT, std::size_t N>
class BasicString final {
public:
    using ContainerType = Array<CharT, N>;
    using Iterator = ContainerType::Iterator;
    using ConstIterator = ContainerType::ConstIterator;
    using ReverseIterator = ContainerType::ReverseIterator;
    using ReverseConstIterator = ContainerType::ReverseConstIterator;
    using ElementType = ContainerType::ElementType;
    using ReferenceType = ContainerType::ReferenceType;
    using ConstReferenceType = ContainerType::ConstReferenceType;
    using PointerType = ContainerType::PointerType;
    using ConstPointerType = ContainerType::ConstPointerType;
    using SizeType = ContainerType::SizeType;
    using VersionTagType = ContainerType::VersionTagType;
    using TraitsType = CharTraits<CharT>;

    BasicString(std::nullptr_t) = delete;
    BasicString& operator=(std::nullptr_t) = delete;

    constexpr explicit BasicString() = default;
    BasicString(CharT symbol) { (void)append(symbol); }
    BasicString(CharT symbol, SizeType count) { (void)append(symbol, count); }
    BasicString(const CharT* src, SizeType count) { (void)append(src, count); }
    BasicString(const CharT* src) { (void)append(src); }

    BasicString(const BasicString& other) { (void)append(other); }
    BasicString(BasicString&& other) { (void)append(std::move(other)); }
    BasicString(std::initializer_list<const CharT> data) {
        (void)append(data.begin(), data.end());
    }

    BasicString& operator=(const CharT* src) { return append(src); }
    BasicString& operator=(CharT symbol) { return append(symbol); }
    BasicString& operator=(std::initializer_list<const CharT> data) { return append(data); }
    BasicString& operator=(const BasicString& other) { return append(other); }
    BasicString& operator=(BasicString&& other) { return append(other); }

    ~BasicString() { clear(); }



    BasicString& append(const BasicString& str) {}
    BasicString& append(BasicString&& str) {}
    BasicString& append(const BasicString& str, SizeType pos, SizeType count) {}
    BasicString& append(BasicString&& str, SizeType pos, SizeType count) {}
    BasicString& append(std::initializer_list<const CharT> data) {}
    BasicString& append(const CharT* src) {}
    BasicString& append(const CharT* src, SizeType count) {}
    BasicString& append(CharT symbol) { m_container.pushBack(symbol); }
    void pushBack(CharT symbol) { m_container.pushBack(symbol); }
    void popBack() { m_container.popBack(); }
    void clear() { m_container.clear(); }

    SizeType find(const BasicString& str, SizeType pos = 0) {}
    SizeType find(const CharT* src, SizeType count, SizeType pos = 0) {}
    SizeType find(const CharT* src, SizeType pos = 0) {}
    SizeType find(CharT symbol, SizeType pos = 0) {}

    ConstReferenceType atUnsafe(SizeType index) const noexcept {
        return m_container.atUnsafe(index);
    }
    ReferenceType atUnsafe(SizeType index) noexcept { return m_container.atUnsafe(index); }

    ConstReferenceType operator[](SizeType index) const { return m_container.operator[](index); }
    ReferenceType operator[](SizeType index) { return m_container.operator[](index); }

    constexpr SizeType size() const noexcept { return m_container.size(); }
    constexpr SizeType capacity() const noexcept { return m_container.capacity(); }

    constexpr bool isEmpty() const noexcept { return m_container.isEmpty(); }
    constexpr bool isFull() const noexcept { return m_container.isFull(); }

private:
    ContainerType m_container{};
};

template<std::size_t N>
using String = BasicString<char, N>;

template<std::size_t N>
using WString = BasicString<wchar_t, N>;

template<std::size_t N>
using U8String = BasicString<char8_t, N>;

template<std::size_t N>
using U16String = BasicString<char16_t, N>;

template<std::size_t N>
using U32String = BasicString<char32_t, N>;


} //! namespace atom::containers::fixed

#endif //! ATOM_CONTAINER_FIXED_STRING_H
