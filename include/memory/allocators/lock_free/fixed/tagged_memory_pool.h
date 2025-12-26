#ifndef ATOM_LOCK_FREE_STATIC_TAGGED_MEMORY_POOL_H
#define ATOM_LOCK_FREE_STATIC_TAGGED_MEMORY_POOL_H

#include <array>
#include <atomic>
#include <type_traits>

#include <cstdint>
#include <cstddef>
#include <cassert>

namespace atom::memory::allocator::lock_free::fixed {

namespace __impl_details {

constexpr std::size_t SelectorToSlotIndex(std::uint32_t virtIndex) noexcept;

constexpr std::uint32_t SlotIndexToSelector(std::size_t slotIndex) noexcept;

} //! namespace __impl_details

template<typename Id, std::size_t N, std::size_t Capacity>
class TaggedMemoryPool final {
public:
    class TaggedPointer final {
    public:
        using TagType = std::uint32_t;
        using SelectorType = std::uint32_t;



        ~TaggedPointer() = default;
        TaggedPointer(const TaggedPointer&) = default;
        TaggedPointer(TaggedPointer&&) = default;
        TaggedPointer& operator=(const TaggedPointer&) = default;
        TaggedPointer& operator=(TaggedPointer&&) = default;

        [[nodiscard]] bool is_null() const noexcept {
            return m_selector == 0;
        }

        [[nodiscard]] explicit operator bool() const noexcept {
            return !is_null();
        }

        const TagType& tag() const noexcept {
            return m_tag;
        }

        TagType& tag() noexcept {
            return m_tag;
        }

        const SelectorType& selector() const noexcept {
            return m_selector;
        }

        const SelectorType& selector() noexcept {
            return m_selector;
        }

        [[nodiscard]] const void* deref() const noexcept {
            const auto index = __impl_details::SelectorToSlotIndex(m_selector);
            assert(index < Capacity);
            return m_slots[index];
        }

        [[nodiscard]] void* deref() noexcept {
            const auto index = __impl_details::SelectorToSlotIndex(m_selector);
            assert(index < Capacity);
            return m_slots[index];
        }

        template<typename T>
        [[nodiscard]] const T* interpretAs() const noexcept {
            return reinterpret_cast<const T*>(deref());
        }

        template<typename T>
        [[nodiscard]] T* interpretAs() noexcept {
            return reinterpret_cast<T*>(deref());
        }

    private:
        friend class TaggedMemoryPool<Id, N, Capacity>;

        explicit TaggedPointer(std::size_t slotIndex = 0, TagType tag = 0)
            : m_selector(__impl_details::SlotIndexToSelector(slotIndex))
            , m_tag(tag)
        {}

        SelectorType m_selector{0};
        TagType m_tag{0};
    };

private:
    static_assert(sizeof(TaggedPointer) <= 64);
    static_assert(std::is_trivially_copyable_v<TaggedPointer>);
    static_assert(std::atomic<TaggedPointer>::is_always_lock_free);

    struct Slot {
        std::array<std::byte, N> buffer{};
    };

    static inline std::array<Slot, Capacity> m_slots;

public:
    TaggedPointer allocate() {
        return TaggedPointer{};
    }

    void deallocate(TaggedPointer ptr) {

    }
};

namespace __impl_details {

constexpr std::size_t SelectorToSlotIndex(std::uint32_t virtIndex) noexcept {
    const std::uint32_t l1 = virtIndex & 0xFFFu;            // 12 bits
    const std::uint32_t l2 = (virtIndex >> 12) & 0x3FFu;    // 10 bits
    const std::uint32_t l3 = (virtIndex >> 22) & 0x3FFu;    // 10 bits

    constexpr std::size_t L1_MAX = 1u << 12;   // 4096
    constexpr std::size_t L2_MAX = 1u << 10;   // 1024

    return static_cast<std::size_t>(l3) * (L2_MAX * L1_MAX) +
           static_cast<std::size_t>(l2) * L1_MAX +
           l1;
}

constexpr std::uint32_t SlotIndexToSelector(std::size_t slotIndex) noexcept {
    constexpr std::size_t L1_MAX = 1u << 12;   // 4096
    constexpr std::size_t L2_MAX = 1u << 10;   // 1024

    const std::uint32_t l1 = static_cast<std::uint32_t>(slotIndex % L1_MAX);
    const std::size_t t = slotIndex / L1_MAX;

    const std::uint32_t l2 = static_cast<std::uint32_t>(t % L2_MAX);
    const std::uint32_t l3 = static_cast<std::uint32_t>(t / L2_MAX);

    return (static_cast<std::uint32_t>(l3) << 22) |
           (static_cast<std::uint32_t>(l2) << 12) |
           l1;
}

} //! namespace __impl_details

} //! namespace atom::memory::allocator::lock_free::fixed

#endif //! ATOM_LOCK_FREE_STATIC_TAGGED_MEMORY_POOL_H
