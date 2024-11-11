#ifndef SEAT_H
#define SEAT_H

#include <LSeat.h>

using namespace Louvre;

class Seat : public LSeat
{
public:
    Seat(const void *params);

    void enabledChanged() override;
    void nativeInputEvent(void *event) override;
    void outputUnplugged(LOutput *output) override;
    void inputDevicePlugged(LInputDevice *device) override;
    bool configureOutputsRequest(LClient *client, const std::vector<OutputConfiguration> &confs) override;

    void configureInputDevices() noexcept;
    void configureInputDevice(LInputDevice *device) noexcept;

    // Last swipe dx
    Float32 dx = 0.f;
    Float32 swipeMargin = 275.f;
};

#endif // SEAT_H
