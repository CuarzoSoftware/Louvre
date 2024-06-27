#ifndef SEAT_H
#define SEAT_H

#include <LSeat.h>

using namespace Louvre;

class Seat final : public LSeat
{
public:
    using LSeat::LSeat;
    void enabledChanged() override;
};

#endif // SEAT_H
