#ifndef LWAYLANDSEATGLOBAL_H
#define LWAYLANDSEATGLOBAL_H

#include <LResource.h>

using namespace Louvre;

class LWaylandKeyboardResource;
class LWaylandPointerResource;

class LWaylandSeatGlobal : public LResource
{
public:
    LWaylandSeatGlobal(LCompositor *compositor,
                  wl_client *client,
                  const wl_interface *interface,
                  Int32 version,
                  UInt32 id,
                  const void *implementation,
                  wl_resource_destroy_func_t destroy);

    ~LWaylandSeatGlobal();

    struct LastKeyboardEventSerials
    {
        UInt32 leave = 0;
        UInt32 enter = 0;
        UInt32 modifiers = 0;
        UInt32 key = 0;
    };

    struct LastPointerEventSerials
    {
        UInt32 leave = 0;
        UInt32 enter = 0;
        UInt32 button = 0;
    };

    // Events
    void sendCapabilities(UInt32 capabilities);
    void sendName(const char *name);

    const std::list<LWaylandKeyboardResource*> &keyboardResources();
    const LastKeyboardEventSerials &keyboardSerials() const;

    const std::list<LWaylandPointerResource*> &pointerResources();
    const LastPointerEventSerials &pointerSerials() const;

    LPRIVATE_IMP(LWaylandSeatGlobal)
};

#endif // LWAYLANDSEATGLOBAL_H
