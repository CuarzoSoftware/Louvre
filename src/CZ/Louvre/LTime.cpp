#include <CZ/Louvre/LTime.h>
#include <time.h>

using namespace Louvre;

static timespec ts;



UInt32 Louvre::LTime::ms() noexcept
{
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<UInt32>(ts.tv_sec) * 1000 + static_cast<UInt32>(ts.tv_nsec) / 1000000;
}

UInt32 LTime::us() noexcept
{
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<UInt64>(ts.tv_sec) * 1000000 + static_cast<UInt64>(ts.tv_nsec) / 1000;
}

timespec LTime::ns() noexcept
{
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts;
}
