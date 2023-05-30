#ifndef SEATGLOBAL_H
#define SEATGLOBAL_H

#include <LResource.h>

class Louvre::Protocols::Wayland::GSeat : public LResource
{
public:
    GSeat(wl_client *client,
          const wl_interface *interface,
          Int32 version,
          UInt32 id,
          const void *implementation,
          wl_resource_destroy_func_t destroy);

    ~GSeat();

    // Events
    void sendCapabilities(UInt32 capabilities);
    void sendName(const char *name);

    RKeyboard *keyboardResource() const;
    RPointer  *pointerResource() const;
    RDataDevice *dataDeviceResource() const;

    LPRIVATE_IMP(GSeat)
};

#endif // SEATGLOBAL_H
