#ifndef LTIME_H
#define LTIME_H

#include <LNamespaces.h>

/*!
 * @brief Time utilities
 */
class Louvre::LTime
{
public:
    /// @cond OMIT
    LTime(const LTime&) = delete;
    LTime& operator= (const LTime&) = delete;
    /// @endcond

    /*!
     * @brief Milliseconds
     *
     * Time with a granularity of milliseconds and an undefined base.
     */
    static UInt32 ms();

    /*!
     * @brief Nanoseconds
     *
     * Time with nanosecond granularity and undefined base.
     */
    static timespec ns();

private:
    /// @cond OMIT
    LTime() = delete;
    /// @endcond
};

#endif // LTIME_H
