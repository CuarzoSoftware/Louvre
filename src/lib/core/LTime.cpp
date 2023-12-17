#include <LTime.h>
#include <time.h>

using namespace Louvre;

static timespec ts;
static UInt32 serial;

UInt32 LTime::nextSerial()
{
    return ++serial;
}

UInt32 Louvre::LTime::ms()
{
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<UInt32>(ts.tv_sec) * 1000 + static_cast<UInt32>(ts.tv_nsec) / 1000000;
}

UInt32 LTime::us()
{
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<UInt64>(ts.tv_sec) * 1000000 + static_cast<UInt64>(ts.tv_nsec) / 1000;
}

timespec LTime::ns()
{
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts;
}
