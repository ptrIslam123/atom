#ifndef ATOM_SYNCHRONIZER_FICTITOUS_MUTEX_H
#define ATOM_SYNCHRONIZER_FICTITOUS_MUTEX_H

namespace atom::concurrence::synchronizer {

/**
 * @brief A fictitious mutex class that mimics the behavior of a standard mutex.
 *
 * This class provides a minimal interface for a mutex, but does not actually
 * perform any locking or synchronization. It is intended for testing or
 * placeholder purposes where a real mutex is not required.
 *
 * @note This class is not thread-safe and does not provide real synchronization.
 */
struct FictitiousMutex final {
    using native_handle = void; ///< Native handle type (not used in this implementation).

    FictitiousMutex(const FictitiousMutex&) = delete; ///< Deleted copy constructor.
    FictitiousMutex& operator=(const FictitiousMutex&) noexcept = delete; ///< Deleted copy assignment operator.

    /**
     * @brief Default constructor.
     */
    explicit FictitiousMutex() = default;

    /**
     * @brief Default destructor.
     */
    ~FictitiousMutex() = default;

    /**
     * @brief Locks the mutex (no-op in this implementation).
     */
    void lock() {}

    /**
     * @brief Unlocks the mutex (no-op in this implementation).
     */
    void unlock() {}

    /**
     * @brief Attempts to lock the mutex (always succeeds in this implementation).
     * @return Always returns `true`.
     */
    bool try_lock() { return true; }
};

/**
 * @brief A fictitious shared mutex class that mimics the behavior of a standard shared mutex.
 *
 * This class provides a minimal interface for a shared mutex, but does not actually
 * perform any locking or synchronization. It is intended for testing or
 * placeholder purposes where a real shared mutex is not required.
 *
 * @note This class is not thread-safe and does not provide real synchronization.
 */
struct SharedFictitiousMutex final {
    using native_handle = void; ///< Native handle type (not used in this implementation).

    SharedFictitiousMutex(const SharedFictitiousMutex&) = delete; ///< Deleted copy constructor.
    SharedFictitiousMutex& operator=(const SharedFictitiousMutex&) = delete; ///< Deleted copy assignment operator.

    /**
     * @brief Default constructor.
     */
    explicit SharedFictitiousMutex() = default;

    /**
     * @brief Default destructor.
     */
    ~SharedFictitiousMutex() = default;

    /**
     * @brief Locks the mutex in exclusive mode (no-op in this implementation).
     */
    void lock() {}

    /**
     * @brief Unlocks the mutex in exclusive mode (no-op in this implementation).
     */
    void unlock() {}

    /**
     * @brief Attempts to lock the mutex in exclusive mode (always succeeds in this implementation).
     * @return Always returns `true`.
     */
    bool try_lock() { return true; }

    /**
     * @brief Locks the mutex in shared mode (no-op in this implementation).
     */
    void lock_shared() {}

    /**
     * @brief Unlocks the mutex in shared mode (no-op in this implementation).
     */
    void unlock_shared() {}

    /**
     * @brief Attempts to lock the mutex in shared mode (always succeeds in this implementation).
     * @return Always returns `true`.
     */
    bool try_lock_shared() { return true; }
};

} //! namespace atom::concurrence::synchronizer

#endif //! ATOM_SYNCHRONIZER_FICTITOUS_MUTEX_H
