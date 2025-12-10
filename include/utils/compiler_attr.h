#ifndef ATOM_COMPILER_ATTR_H
#define ATOM_COMPILER_ATTR_H

/**
 * @file atom_compiler_attr.h
 * @brief Cross-platform compiler attributes abstraction layer
 * @details Provides unified interface for compiler-specific attributes across
 *          different compilers (MSVC, GCC, Clang) and C++ standards.
 *
 * @defgroup compiler_attributes Compiler Attributes
 * @brief Macros for compiler-specific attributes and extensions
 *
 *
 * @brief Force function inlining regardless of optimization settings
 * @details Use for small, performance-critical functions that should always be inlined
 * @example
 * FORCE_INLINE int add(int a, int b) {...}
 *
 *
 * @brief Prevent function from being inlined
 * @details Use for large functions or when debugging requires function boundaries
 * @example
 * NO_INLINE void debug_log(const char* message) {...}
 *
 *
 * @brief Pack structure with 1-byte alignment
 * @param ... Structure definition to pack
 * @details Essential for network protocols, hardware registers, and memory-mapped I/O
 * @warning Packed structures may have performance penalties on some architectures
 * @example
 * PACK_STRUCT(struct NetworkHeader {
 *     uint16_t type;
 *     uint32_t length;
 * });
 *
 *
 * @brief Hint that a branch is likely to be taken (C++17)
 * @details Helps compiler optimize for the common case
 * @example
 * if (ptr != nullptr) LIKELY {
 *     // Main execution path
 * }
 *
 *
 * @brief Hint that a branch is unlikely to be taken (C++17)
 * @details Helps compiler optimize error handling paths
 * @example
 * if (error_condition) UNLIKELY {
 *     // Rare error handling
 * }
 *
 *
 * @brief Expression-based branch prediction
 * @param x Boolean expression to evaluate
 * @return The expression value with branch prediction hint
 * @example
 * if (LIKELY_EXPR(ptr != nullptr)) {
 *     // Optimized path
 * }
 *
 *
 * @brief Expression-based unlikely branch prediction
 * @param x Boolean expression to evaluate
 * @return The expression value with branch prediction hint
 * @example
 * if (UNLIKELY_EXPR(value < 0)) {
 *     handle_error();
 * }
 *
 *
 * @brief Warn if return value is ignored (C++17)
 * @details Prevents bugs from ignoring important return values
 * @example
 * NODISCARD int allocate_memory(); // Warning if ignored
 *
 *
 * @brief Mark function as rarely executed (cold)
 * @details Optimizes function for size rather than speed
 * @example
 * COLD_FUNC void error_recovery() {...}
 *
 *
 * @brief Mark function as frequently executed (hot)
 * @details Optimizes function for maximum performance
 * @example
 * HOT_FUNC int process_packet(Packet* pkt) {...}
 *
 *
 * @brief Mark function as not returning
 * @details Informs compiler that function never returns
 * @example
 * NORETURN void fatal_error(const char* msg) { exit(1); }
 *
 *
 * @brief Mark function/variable as deprecated
 * @param msg Deprecation message shown in warnings
 * @example
 * DEPRECATED("Use new_api() instead") void old_api();
 *
 *
 * @brief Suppress unused variable warnings
 * @example
 * void func(int used_param, int UNUSED unused_param) {}
 *
 *
 * @brief Force symbol emission even if unused
 * @details Prevents linker from removing unused symbols
 * @example
 * USED const int important_constant = 42;
 *
 *
 * @brief Specify custom alignment
 * @param n Alignment boundary in bytes
 * @example
 * ALIGNED(64) char cache_line[64]; // For cache optimization
 **/

#if defined(_MSC_VER)

    // Microsoft Visual C++ Compiler
    #define FORCE_INLINE __forceinline
    #define NO_INLINE __declspec(noinline)
    #define PACK_STRUCT(...) __pragma(pack(push, 1)) __VA_ARGS__ __pragma(pack(pop))

    // Branch prediction
    #if __cplusplus >= 201703L
        #define LIKELY [[likely]]
        #define UNLIKELY [[unlikely]]
    #else
        #define LIKELY
        #define UNLIKELY
    #endif

    // Expression-based branch prediction (MSVC has limited support)
    #if defined(_MSC_VER) && _MSC_VER >= 1926
        #define LIKELY_EXPR(x) [[likely]] (x)
        #define UNLIKELY_EXPR(x) [[unlikely]] (x)
    #else
        #define LIKELY_EXPR(x) (x)
        #define UNLIKELY_EXPR(x) (x)
    #endif

    // NODISCARD with SAL annotations
    #if __cplusplus >= 201703L
        #define NODISCARD [[nodiscard]]
    #else
        #include <sal.h>
        #define NODISCARD _Check_return_
    #endif

    // MSVC-specific attributes
    #define COLD_FUNC __declspec(noinline) // Cold functions shouldn't be inlined
    #define HOT_FUNC __forceinline          // Hot functions should be inlined
    #define NORETURN __declspec(noreturn)
    #define DEPRECATED(msg) __declspec(deprecated(msg))
    #define UNUSED
    #define USED
    #define WEAK_SYMBOL
    #define ALIGNED(n) __declspec(align(n))

    // MSVC doesn't have direct equivalents for these
    #define PURE_FUNC
    #define CONST_FUNC
    #define SECTION(name)

#elif defined(__GNUC__) || defined(__clang__)

    // GCC and Clang compilers
    #define FORCE_INLINE __attribute__((always_inline)) inline
    #define NO_INLINE __attribute__((noinline))
    #define PACK_STRUCT(...) __VA_ARGS__ __attribute__((packed))

    // Branch prediction
    #if __cplusplus >= 201703L
        #define LIKELY [[likely]]
        #define UNLIKELY [[unlikely]]
        #define LIKELY_EXPR(x) (x) [[likely]]
        #define UNLIKELY_EXPR(x) (x) [[unlikely]]
    #else
        #define LIKELY
        #define UNLIKELY
        #define LIKELY_EXPR(x) (__builtin_expect(!!(x), 1))
        #define UNLIKELY_EXPR(x) (__builtin_expect(!!(x), 0))
    #endif

    // NODISCARD
    #if __cplusplus >= 201703L
        #define NODISCARD [[nodiscard]]
    #else
        #define NODISCARD __attribute__((warn_unused_result))
    #endif

    // GCC/Clang attributes
    #define COLD_FUNC __attribute__((cold))
    #define HOT_FUNC __attribute__((hot))
    #define PURE_FUNC __attribute__((pure))
    #define CONST_FUNC __attribute__((const))
    #define NORETURN __attribute__((noreturn))
    #define DEPRECATED(msg) __attribute__((deprecated(msg)))
    #define UNUSED __attribute__((unused))
    #define USED __attribute__((used))
    #define WEAK_SYMBOL __attribute__((weak))
    #define ALIGNED(n) __attribute__((aligned(n)))
    #define SECTION(name) __attribute__((section(name)))

#else
    #pragma message("Unknown compiler - using default empty definitions")

    // Default empty definitions for unknown compilers
    #define FORCE_INLINE inline
    #define NO_INLINE
    #define PACK_STRUCT(...) __VA_ARGS__

    #define LIKELY
    #define UNLIKELY
    #define LIKELY_EXPR(x) (x)
    #define UNLIKELY_EXPR(x) (x)
    #define NODISCARD

    #define COLD_FUNC
    #define HOT_FUNC
    #define PURE_FUNC
    #define CONST_FUNC
    #define NORETURN
    #define DEPRECATED(msg)
    #define UNUSED
    #define USED
    #define WEAK_SYMBOL
    #define ALIGNED(n)
    #define SECTION(name)
#endif

// Cross-platform C++20 features
#if __cplusplus >= 202002L
    #define NODISCARD_MSG(msg) [[nodiscard(msg)]]
    #define NO_UNIQUE_ADDRESS [[no_unique_address]]
#else
    #define NODISCARD_MSG(msg) NODISCARD
    #define NO_UNIQUE_ADDRESS
#endif

// Fallthrough macro
#if defined(__cplusplus) && __cplusplus >= 201703L
    #define FALLTHROUGH [[fallthrough]]
#elif defined(__GNUC__) && __GNUC__ >= 7
    #define FALLTHROUGH __attribute__((fallthrough))
#else
    #define FALLTHROUGH
#endif

#include <type_traits>
#define IS_NOEXCEPT_CONSTRUCIBLE(T) noexcept(std::is_nothrow_constructible<T>::value)
#define IS_NOEXCEPT_DESTRUCTIBLE(T) noexcept(std::is_nothrow_destructible<T>::value)
#define IS_NOEXCEPT_CONSTR_AND_DESTR(T) noexcept(std::is_nothrow_constructible<T>::value && \
                                                std::is_nothrow_destructible<T>::value)

#endif // ATOM_COMPILER_ATTR_H
