#ifndef LTIME_H
#define LTIME_H

#include <LNamespaces.h>

/*!
 * @brief Time utilities
 */
class Louvre::LTime
{
public:
    LTime(const LTime&) = delete;
    LTime& operator= (const LTime&) = delete;

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
    LTime() = delete;
};

#endif // LTIME_H
