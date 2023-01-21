#include <LTime.h>
#include <time.h>

using namespace Louvre;

UInt32 Louvre::LTime::ms()
{
    timespec endTime;
    clock_gettime(CLOCK_MONOTONIC_RAW, &endTime);
    return endTime.tv_sec*1000 + endTime.tv_nsec/1000000;
}

timespec LTime::us()
{
    timespec endTime;
    clock_gettime(CLOCK_MONOTONIC_RAW, &endTime);
    return endTime;
}
