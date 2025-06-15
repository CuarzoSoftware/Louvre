#ifndef RKEYBOARD_H
#define RKEYBOARD_H

#include <LResource.h>
#include <CZ/CZWeak.h>

class Louvre::Protocols::Wayland::RKeyboard final : public LResource
{
public:

    GSeat *seatRes() const noexcept
    {
        return m_seatRes;
    }

    /******************** REQUESTS ********************/

#if LOUVRE_WL_SEAT_VERSION >= 3
    static void release(wl_client *client, wl_resource *resource) noexcept;
#endif

    /******************** EVENTS ********************/

    // Since 1
    void keymap(UInt32 format, Int32 fd, UInt32 size) noexcept;
    void enter(const LKeyboardEnterEvent &event, RSurface *surfaceRes, wl_array *keys) noexcept;
    void leave(const LKeyboardLeaveEvent &event, RSurface *surfaceRes) noexcept;
    void key(const LKeyboardKeyEvent &event) noexcept;
    void modifiers(const LKeyboardModifiersEvent &event) noexcept;

    // Since 4
    bool repeatInfo(Int32 rate, Int32 delay) noexcept;

private:
    friend class Louvre::Protocols::Wayland::GSeat;
    RKeyboard(GSeat *seatRes, Int32 id) noexcept;
    ~RKeyboard() noexcept;
    CZWeak<GSeat> m_seatRes;
};

#endif // RKEYBOARD_H
