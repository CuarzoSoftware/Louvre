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
};

#endif // SEAT_H
