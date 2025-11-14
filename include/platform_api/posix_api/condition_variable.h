#ifndef ATOM_PLATFORM_API_POSIX_COND_VAR_H
#define ATOM_PLATFORM_API_POSIX_COND_VAR_H

#include "include/platform_api/posix_api/futex.h"

#include <chrono>

namespace atom::platform::posix {

class ConditionalVariable final {
public:
    ConditionalVariable(const ConditionalVariable& ) = delete;
    ConditionalVariable(ConditionalVariable&& ) = delete;
    ConditionalVariable& operator=(const ConditionalVariable& ) = delete;
    ConditionalVariable& operator=(ConditionalVariable&& ) = delete;

    ConditionalVariable() = default;
    ~ConditionalVariable() = default;

    template<typename Predicate>
    void wait(Predicate p) {
        if (p()) {
            return;
        }
        std::uint32_t current = __atomic_fetch_add(&m_counter, 1, __ATOMIC_RELAXED) + 1;
        FutexWait(m_counter, current);

        check:
        if (p()) {
            return;
        }

        current = __atomic_load_n(&m_counter, __ATOMIC_RELAXED);
        FutexWait(m_counter, current);
        goto check;
    }

    void notifyAny() {
        (void)__atomic_fetch_sub(&m_counter, 1, __ATOMIC_RELAXED);
        FutexWakeUpAny(m_counter);
    }

    void notifyAll() {
        std::uint32_t count = __atomic_exchange_n(&m_counter, 0, __ATOMIC_RELAXED);
        FutexWakeUpAny(m_counter);
    }

private:    
    std::uint32_t m_counter{0};
};

} //! namespace atom::platform::posix

#endif //! ATOM_PLATFORM_API_POSIX_COND_VAR_H
