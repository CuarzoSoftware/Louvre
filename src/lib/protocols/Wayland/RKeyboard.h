#ifndef RKEYBOARD_H
#define RKEYBOARD_H

#include <LResource.h>

class Louvre::Protocols::Wayland::RKeyboard : public LResource
{
public:
    RKeyboard(GSeat *gSeat, Int32 id);
    ~RKeyboard();

    struct LastEventSerials
    {
        UInt32 leave = 0;
        UInt32 enter = 0;
        UInt32 modifiers = 0;
        UInt32 key = 0;
    };

    GSeat *seatGlobal() const;
    const LastEventSerials &serials() const;

    // Since 1
    bool keymap(UInt32 format, Int32 fd, UInt32 size);
    bool enter(UInt32 serial, RSurface *rSurface, wl_array *keys);
    bool leave(UInt32 serial, RSurface *rSurface);
    bool key(UInt32 serial, UInt32 time, UInt32 key, UInt32 state);
    bool modifiers(UInt32 serial, UInt32 modsDepressed, UInt32 modsLatched, UInt32 modsLocked, UInt32 group);

    // Since 4
    bool repeatInfo(Int32 rate, Int32 delay);

LPRIVATE_IMP(RKeyboard)
};

#endif // RKEYBOARD_H
