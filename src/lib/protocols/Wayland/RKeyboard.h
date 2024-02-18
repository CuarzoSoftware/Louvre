#ifndef RKEYBOARD_H
#define RKEYBOARD_H

#include <LResource.h>

class Louvre::Protocols::Wayland::RKeyboard : public LResource
{
public:
    RKeyboard(GSeat *gSeat, Int32 id);
    ~RKeyboard();

    GSeat *seatGlobal() const;

    // Since 1
    bool keymap(UInt32 format, Int32 fd, UInt32 size);
    bool enter(const LKeyboardEnterEvent &event, RSurface *rSurface, wl_array *keys);
    bool leave(const LKeyboardLeaveEvent &event, RSurface *rSurface);
    bool key(const LKeyboardKeyEvent &event);
    bool modifiers(const LKeyboardModifiersEvent &event);

    // Since 4
    bool repeatInfo(Int32 rate, Int32 delay);

    LPRIVATE_IMP_UNIQUE(RKeyboard)
};

#endif // RKEYBOARD_H
