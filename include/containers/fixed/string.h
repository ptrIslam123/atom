#ifndef ATOM_CONTAINER_FIXED_STRING_H
#define ATOM_CONTAINER_FIXED_STRING_H

#include "include/containers/fixed/array.h"
#include "include/sfinae/char_traits.h"
#include "include/utils/compiler_attr.h"
#include "include/algorithm/algorithm.h"

#include <limits>
#include <cstring>

namespace atom::containers::fixed {

template<typename CharT, std::size_t N>
class BasicString final {
public:
    using ContainerType = Array<CharT, N>;
    using Iterator = typename ContainerType::Iterator;
    using ConstIterator = typename ContainerType::ConstIterator;
    using ReverseIterator = typename ContainerType::ReverseIterator;
    using ReverseConstIterator = typename ContainerType::ReverseConstIterator;
    using ElementType = typename ContainerType::ElementType;
    using ReferenceType = typename ContainerType::ReferenceType;
    using ConstReferenceType = typename ContainerType::ConstReferenceType;
    using PointerType = typename ContainerType::PointerType;
    using ConstPointerType = typename ContainerType::ConstPointerType;
    using SizeType = typename ContainerType::SizeType;
    using VersionTagType = typename ContainerType::VersionTagType;
    using TraitsType = CharTraits<CharT>;

    static constexpr SizeType npos = std::numeric_limits<SizeType>::max();

    BasicString(std::nullptr_t) = delete;
    BasicString& operator=(std::nullptr_t) = delete;

    FORCE_INLINE constexpr explicit BasicString() = default;
    template<typename InputIterator>
    FORCE_INLINE BasicString(InputIterator first, InputIterator last) { (void)append(first, last); }
    FORCE_INLINE explicit BasicString(CharT symbol) { (void)append(symbol); }
    FORCE_INLINE explicit BasicString(CharT symbol, SizeType count) { (void)append(symbol, count); }
    FORCE_INLINE BasicString(const CharT* src, SizeType count) { (void)append(src, count); }
    FORCE_INLINE BasicString(const CharT* src) { (void)append(src); }

    FORCE_INLINE BasicString(const BasicString& other) { (void)append(other); }
    FORCE_INLINE BasicString(BasicString&& other) { (void)append(std::move(other)); }
    FORCE_INLINE BasicString(std::initializer_list<const CharT> data) {
        (void)append(data.begin(), data.end());
    }

    FORCE_INLINE BasicString& operator=(const CharT* src) {
        clear();
        return append(src);
    }
    FORCE_INLINE BasicString& operator=(CharT symbol) {
        clear();
        return append(symbol);
    }
    FORCE_INLINE BasicString& operator=(std::initializer_list<const CharT> data) {
        clear();
        return append(data);
    }
    FORCE_INLINE BasicString& operator=(const BasicString& other) {
        m_container = other.m_container;
        return *this;
    }
    FORCE_INLINE BasicString& operator=(BasicString&& other) {
        m_container = std::move(other.m_container);
        return *this;
    }
    FORCE_INLINE BasicString& operator+=(const BasicString& str) { return append(str); }
    FORCE_INLINE BasicString& operator+=(BasicString&& str) { return append(std::move(str)); }

    FORCE_INLINE ~BasicString() = default;

    template<typename InputIterator>
    FORCE_INLINE BasicString& append(InputIterator first, InputIterator last) {
        (void)insert(endConstIter(), first, last);
        return *this;
    }
    FORCE_INLINE BasicString& append(const BasicString& str) {
        (void)insert(endConstIter(), str);
        return *this;
    }
    FORCE_INLINE BasicString& append(BasicString&& str) {
        (void)insert(endConstIter(), std::move(str));
        return *this;
    }
    FORCE_INLINE BasicString& append(std::initializer_list<const CharT> data) {
        (void)insert(endConstIter(), data.begin(), data.end());
        return *this;
    }
    FORCE_INLINE BasicString& append(const CharT* src) {
        (void)insert(endConstIter(), src);
        return *this;
    }
    FORCE_INLINE BasicString& append(const CharT* src, SizeType count) {
        (void)insert(endConstIter(), src, count);
        return *this;
    }
    FORCE_INLINE BasicString& append(CharT symbol) {
        (void)insert(endConstIter(), symbol);
        return *this;
    }
    BasicString& append(CharT symbol, SizeType count) {
        auto it = endConstIter();
        while (count != 0) {
            (void)insert(it, symbol);
            --count;
        }
        return *this;
    }

    template<typename InputIterator>
    FORCE_INLINE Iterator insert(ConstIterator pos, InputIterator first, InputIterator last) {
        return m_container.insert(pos, first, last);
    }
    FORCE_INLINE Iterator insert(ConstIterator pos, const BasicString& str) {
        return m_container.insert(pos, str.firstConstIter(), str.endConstIter());
    }
    FORCE_INLINE Iterator insert(ConstIterator pos, BasicString&& str) {
        return m_container.insert(pos, str.firstConstIter(), str.endConstIter());
    }
    FORCE_INLINE Iterator insert(ConstIterator pos, std::initializer_list<const CharT> data) {
        return m_container.insert(pos, data.begin(), data.end());
    }
    FORCE_INLINE Iterator insert(ConstIterator pos, const CharT* src) {
        //! TODO: TraitsType::length(src)); may be incorrect with wide char types
        return insert(pos, src, std::strlen(src));
    }
    FORCE_INLINE Iterator insert(ConstIterator pos, const CharT* src, SizeType count) {
        return m_container.insert(pos, src, src + count);
    }
    FORCE_INLINE Iterator insert(ConstIterator pos, CharT symbol) {
        return m_container.insert(pos, symbol);
    }

    FORCE_INLINE Iterator erase(ConstIterator pos) {
        return m_container.erase(pos);
    }
    FORCE_INLINE Iterator erase(ConstIterator first, ConstIterator last) {
        return m_container.erase(first, last);
    }

    FORCE_INLINE void pushBack(CharT symbol) { m_container.pushBack(symbol); }
    FORCE_INLINE void popBack() { m_container.popBack(); }
    FORCE_INLINE void clear() { m_container.clear(); }

    /////////
    FORCE_INLINE Iterator find(const BasicString& str, SizeType pos = 0) {}
    FORCE_INLINE Iterator find(const CharT* src, SizeType pos, SizeType count) {}
    FORCE_INLINE Iterator find(const CharT* src, SizeType pos = 0) {}
    FORCE_INLINE Iterator find(CharT symbol, SizeType pos = 0) {}

    FORCE_INLINE ConstIterator find(const BasicString& str, SizeType pos = 0) const {}
    FORCE_INLINE ConstIterator find(const CharT* src, SizeType pos, SizeType count) const  {}
    FORCE_INLINE ConstIterator find(const CharT* src, SizeType pos = 0) const {}
    FORCE_INLINE ConstIterator find(CharT symbol, SizeType pos = 0) const {}

    FORCE_INLINE bool contains(const BasicString& str) const noexcept {
        return find(str) != endConstIter();
    }
    FORCE_INLINE bool contains(const CharT* str) const noexcept {
        return find(str) != endConstIter();
    }
    FORCE_INLINE bool contains(const CharT* str, SizeType count) const noexcept {
        return find(str, count) != endConstIter();
    }
    FORCE_INLINE bool contains(CharT symbol) noexcept {
        return find(symbol) != endConstIter();
    }

    bool compare(const BasicString& str) const noexcept {
        return compare(str.data(), size());
    }
    bool compare(const CharT* str) const noexcept {
        // TraitsType::compare(const CharT* lhs, const CharT* rhs)
    }
    bool compare(const CharT* str, SizeType count) const noexcept {
        // TraitsType::compare(const CharT* lhs, const CharT* rhs)
    }

    FORCE_INLINE bool operator==(const BasicString& str) const noexcept {
        return compare(str);
    }

    BasicString subString(SizeType pos = 0, SizeType count = npos) const {
        ASSERTION(pos < size(), std::runtime_error, "")
        auto first = firstConstIter() + pos;
        auto last = first + algorithm::Clamp(count, SizeType{0}, size() - pos);
        return BasicString{first, last};
    }

    FORCE_INLINE BasicString copy() const noexcept {
        return *this;
    }

    FORCE_INLINE ConstReferenceType atUnsafe(SizeType index) const noexcept {
        return m_container.atUnsafe(index);
    }
    FORCE_INLINE ReferenceType atUnsafe(SizeType index) noexcept {
        return m_container.atUnsafe(index);
    }

    FORCE_INLINE ConstReferenceType operator[](SizeType index) const {
        return m_container.operator[](index);
    }
    FORCE_INLINE ReferenceType operator[](SizeType index) {
        return m_container.operator[](index);
    }

    FORCE_INLINE Iterator firstIter() noexcept {
        return m_container.firstIter();
    }
    FORCE_INLINE Iterator lastIter() noexcept {
        return m_container.lastIter();
    }
    FORCE_INLINE Iterator endIter() noexcept {
        return m_container.endIter();
    }
    FORCE_INLINE ConstIterator firstConstIter() const noexcept {
        return m_container.firstConstIter();
    }
    FORCE_INLINE ConstIterator lastConstIter() const noexcept {
        return m_container.lastConstIter();
    }
    FORCE_INLINE ConstIterator endConstIter() const noexcept {
        return m_container.endConstIter();
    }
    FORCE_INLINE ReverseIterator firstReverseIter() noexcept {
        return m_container.firstReverseIter();
    }
    FORCE_INLINE ReverseIterator lastReverseIter() noexcept {
        return m_container.lastReverseIter();
    }
    FORCE_INLINE ReverseIterator endReverseIter() noexcept {
        return m_container.endReverseIter();
    }
    FORCE_INLINE ReverseConstIterator firstReverseConstIter() const noexcept {
        return m_container.firstReverseConstIter();
    }
    FORCE_INLINE ReverseConstIterator lastReverseConstIter() const noexcept {
        return m_container.lastReverseConstIter();
    }
    FORCE_INLINE ReverseConstIterator endReverseConstIter() const noexcept {
        return m_container.endReverseConstIter();
    }

    FORCE_INLINE ConstReferenceType firstElement() const {
        return m_container.firstElement();
    }
    FORCE_INLINE ReferenceType firstElement() {
        return m_container.firstElement();
    }
    FORCE_INLINE ConstReferenceType lastElement() const {
        return m_container.lastElement();
    }
    FORCE_INLINE ReferenceType lastElement() {
        return m_container.lastElement();
    }

    FORCE_INLINE constexpr SizeType size() const noexcept {
        return m_container.size();
    }
    FORCE_INLINE constexpr SizeType capacity() const noexcept {
        return m_container.capacity();
    }

    FORCE_INLINE constexpr bool isEmpty() const noexcept {
        return m_container.isEmpty();
    }
    FORCE_INLINE constexpr bool isFull() const noexcept {
        return m_container.isFull();
    }

    FORCE_INLINE ConstPointerType data() const noexcept { return m_container.data(); }
    FORCE_INLINE PointerType data() noexcept { return m_container.data(); }

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
