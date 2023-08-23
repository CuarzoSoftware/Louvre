#ifndef SEAT_H
#define SEAT_H

#include <LSeat.h>

using namespace Louvre;

class Seat : public LSeat
{
public:
    Seat(LSeat::Params *params);

    void seatEnabled() override;
    void backendNativeEvent(void *event) override;

    // Last swipe dx
    Float32 dx = 0.f;
    Float32 swipeMargin = 275.f;
};

#endif // SEAT_H
