#ifndef KEYBOARDRESOURCE_H
#define KEYBOARDRESOURCE_H

#include <LResource.h>

class Louvre::Protocols::Wayland::RKeyboard : public LResource
{
public:
    RKeyboard(GSeat *seatGlobal, Int32 id);
    ~RKeyboard();

    struct LastEventSerials
    {
        UInt32 leave = 0;
        UInt32 enter = 0;
        UInt32 modifiers = 0;
        UInt32 key = 0;
    };

    // Events
    void sendRepeatInfo(Int32 rate, Int32 delay);
    void sendKeymap(Int32 fd, UInt32 size);
    void sendLeave(LSurface *surface);
    void sendEnter(LSurface *surface);
    void sendModifiers(UInt32 depressed, UInt32 latched, UInt32 locked, UInt32 group);
    void sendKey(UInt32 key, UInt32 state);

    GSeat *seatGlobal() const;
    const LastEventSerials &serials() const;

LPRIVATE_IMP(RKeyboard)
};

#endif // KEYBOARDRESOURCE_H
