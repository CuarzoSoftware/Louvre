#ifndef SEATGLOBAL_H
#define SEATGLOBAL_H

#include <LResource.h>

class Louvre::Protocols::Wayland::SeatGlobal : public LResource
{
public:
    SeatGlobal(LCompositor *compositor,
                  wl_client *client,
                  const wl_interface *interface,
                  Int32 version,
                  UInt32 id,
                  const void *implementation,
                  wl_resource_destroy_func_t destroy);

    ~SeatGlobal();

    // Events
    void sendCapabilities(UInt32 capabilities);
    void sendName(const char *name);

    KeyboardResource *keyboardResource() const;
    PointerResource  *pointerResource() const;
    DataDeviceResource *dataDeviceResource() const;

    LPRIVATE_IMP(SeatGlobal)
};

#endif // SEATGLOBAL_H
