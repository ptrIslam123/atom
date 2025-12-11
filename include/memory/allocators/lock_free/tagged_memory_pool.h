#ifndef ATOM_LOCK_FREE_STATIC_TAGGED_MEMORY_POOL_H
#define ATOM_LOCK_FREE_STATIC_TAGGED_MEMORY_POOL_H

#include <array>
#include <atomic>
#include <type_traits>

#include <cstdint>
#include <cstddef>
#include <cassert>

namespace atom::memory::allocator::lock_free::fixed {

template<std::size_t N, std::size_t Capacity>
class TaggedMemoryPool final {
public:
    class TaggedPointer final {
    public:
        static_assert(sizeof(TaggedPointer) <= 64);
        static_assert(sizeof(TaggedPointer) == 8);
        static_assert(alignof(TaggedPointer) == 8);
        static_assert(std::is_trivially_copyable_v<TaggedPointer>);
        static_assert(std::atomic<TaggedPointer>::is_always_lock_free);

        TaggedPointer() = default;
        ~TaggedPointer() = default;
        TaggedPointer(const TaggedPointer&) = default;
        TaggedPointer(TaggedPointer&&) = default;
        TaggedPointer& operator=(const TaggedPointer&) = default;
        TaggedPointer& operator=(TaggedPointer&&) = default;

        void const * deref() const noexcept {
            return (static_cast<std::uint8_t*>(this) - sizeof(*this));
        }

        void* deref() noexcept {
            return (static_cast<std::uint8_t*>(this) - sizeof(*this));
        }

        const std::uint64_t& getTag() const noexcept {
            return m_tag;
        }

        std::uint64_t& getTag() noexcept {
            return m_tag;
        }

    private:
        std::uint64_t m_tag{0};
    };

    TaggedPointer allocate() {
        // todo
        assert(false && "TODO");
        return {};
    }

    void deallocate(TaggedPointer&& ptr) {
        // todo
        assert(false && "TODO");
    }

private:
    struct Slot {
        TaggedPointer tag;
        std::array<std::byte, N> buffer{0};
    };

    std::array<Slot, Capacity> m_slots;
};

} //! namespace atom::memory::allocator::lock_free::fixed

#endif //! ATOM_LOCK_FREE_STATIC_TAGGED_MEMORY_POOL_H
