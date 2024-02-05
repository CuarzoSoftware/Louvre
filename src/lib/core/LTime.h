#ifndef LTIME_H
#define LTIME_H

#include <LNamespaces.h>

/**
 * @brief Time utilities
 */
class Louvre::LTime
{
public:
    /// @cond OMIT
    LTime() = delete;
    /// @endcond

    /**
     * @brief Serial.
     *
     * This method returns a new positive integer number each time it is called, incrementally.
     */
    static UInt32 nextSerial();

    /**
     * @brief Milliseconds
     *
     * Monotonic time with a granularity of milliseconds and an undefined base.
     */
    static UInt32 ms();

    /**
     * @brief Microseconds
     *
     * Monotonic time with a granularity of microseconds and an undefined base.
     */
    static UInt32 us();

    /**
     * @brief Nanoseconds
     *
     * Monotonic time with nanosecond granularity and undefined base.
     */
    static timespec ns();
};

#endif // LTIME_H
